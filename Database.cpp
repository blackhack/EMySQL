/*
 * EMySQL - Light and simple MySQL wrapper, with multithread queries support.
 * Copyright (C) 2017 Blackhack
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "Database.h"
#include <chrono>

Database::Database(std::string db_host, std::string db_user, std::string db_password, std::string db_database, uint32_t threads)
{
    _db_host = db_host;
    _db_user = db_user;
    _db_password = db_password;
    _db_database = db_database;

    _connection = mysql_init(nullptr);
    if (!_connection)
        throw DatabaseException(mysql_error(_connection));

    if (!mysql_real_connect(_connection, _db_host.c_str(), _db_user.c_str(), _db_password.c_str(), (_db_database == "" ? nullptr : _db_database.c_str()), 0, nullptr, 0))
    {
        std::string errInfo = mysql_error(_connection);
        mysql_close(_connection);
        throw DatabaseException(errInfo);
    }

    for (uint32_t i = 0; i < threads; ++i)
    {
        std::shared_ptr<DatabaseThread> th = std::make_shared<DatabaseThread>(_db_host, _db_user, _db_password, _db_database);
        th->StartThread();
        _threadSet.insert(th);
    }
}

Database::~Database()
{
    mysql_close(_connection);

    for (auto& _thread : _threadSet)
        _thread->StopThread();

    for (auto& _thread : _threadSet)
        _thread->join();

    _threadSet.clear();
}

void Database::EscapeString(std::string& str)
{
    char *to = new char[(strlen(str.c_str()) * 2) + 1];
    mysql_real_escape_string(_connection, to, str.c_str(), (uint32_t)strlen(str.c_str()));
    str = to;
    delete[] to;
}

void Database::DirectExecuteQuery(std::string query, bool escape)
{
    if (escape)
        EscapeString(query);

    if (mysql_query(_connection, query.c_str()))
        throw DatabaseException(mysql_error(_connection));

    // Needed in case of someone using a returning query in this function
    MYSQL_RES* res = mysql_store_result(_connection);
    mysql_free_result(res);
}

void Database::AsynExecuteQuery(std::string query, bool escape)
{
    std::shared_ptr<DatabaseThread> bestThread = nullptr;

    //Select the Thread with less queries in queue
    for (auto& thread : _threadSet)
    {
        if (!bestThread)
            bestThread = thread;
        else if (bestThread->GetQueriesCount() > thread->GetQueriesCount())
            bestThread = thread;
    }

    if (bestThread)
        bestThread->AddQuery(query, escape);
    else
        DirectExecuteQuery(query, escape); // DirectExecutes if there is not threads running
}

DatabaseResult Database::ResultQuery(std::string query, bool escape)
{
    if (escape)
        EscapeString(query);

    if (mysql_query(_connection, query.c_str()))
        throw DatabaseException(mysql_error(_connection));

    MYSQL_RES *res;
    MYSQL_ROW row;
    DatabaseResult _result;

    res = mysql_use_result(_connection);

    while (row = mysql_fetch_row(res))
        _result.push_back(row[0]);

    mysql_free_result(res);

    return _result;
}


DatabaseThread::DatabaseThread(std::string db_host, std::string db_user, std::string db_password, std::string db_database)
    : _database(db_host, db_user, db_password, db_database, 0)
{

}

DatabaseThread::~DatabaseThread()
{
    delete _thread;
}

void DatabaseThread::AddQuery(std::string query, bool escape)
{
    std::lock_guard<std::mutex> lock(_lock);
    _queries.insert(std::pair<std::string, bool>(query, escape));
}

void DatabaseThread::StartThread()
{
    _run = true;
    _thread = new std::thread(&DatabaseThread::run, this);
}

void DatabaseThread::StopThread()
{
    _run = false;
}

void DatabaseThread::run()
{
    while (_run)
    {
            std::unique_lock<std::mutex> lock(_lock);
            std::multimap<std::string, bool> exeQueries(std::move(_queries));
            _queries.clear();
            lock.unlock();

            for (std::multimap<std::string, bool>::iterator itr = exeQueries.begin(); itr != exeQueries.end();)
            {
                try
                {
                    _database.DirectExecuteQuery(itr->first, itr->second);
                }
                catch (DatabaseException e)
                {
                    std::cerr << "Asyncronous MySQL error: " << e.what() << std::endl;
                    std::cerr << "Query: " << itr->first << std::endl;
                    std::cerr << "REMEMBER: Asyncronous errors cannot be handle, please fix the error\n";
                }

                itr = exeQueries.erase(itr);
            }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void DatabaseThread::join()
{
    _thread->join();
}
