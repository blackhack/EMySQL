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

#include <iostream>
#include <my_global.h>
#include <mysql.h>
#include <thread>
#include <mutex>
#include <memory>
#include <atomic>
#include <set>
#include <vector>
#include <map>
#include <string>

class DatabaseThread;

typedef std::vector<std::string> DatabaseResult;

class Database
{
public:
    Database(std::string db_host, std::string db_user, std::string db_password, std::string db_database, uint32_t threads);
    ~Database();

    void DirectExecuteQuery(std::string query, bool escape = false);
    void AsynExecuteQuery(std::string query, bool escape = false);
    DatabaseResult ResultQuery(std::string query, bool escape = false);

private:

    MYSQL* _connection;
    std::string _db_host;
    std::string _db_user;
    std::string _db_password;
    std::string _db_database;
    std::set<std::shared_ptr<DatabaseThread> > _threadSet;
};

class DatabaseException : public std::runtime_error
{
public:
    DatabaseException(std::string info) : std::runtime_error(info) { }
};

class DatabaseThread
{
    friend class Database;
public:

    DatabaseThread(std::string db_host, std::string db_user, std::string db_password, std::string db_database);
    ~DatabaseThread();
    
private:

    uint32_t GetQueriesCount() { return (uint32_t)_queries.size(); }
    void AddQuery(std::string query, bool escape);

    void StartThread();
    void StopThread();

    void run();
    void join();

    Database _database;
    std::thread* _thread;
    std::atomic<bool> _run;
    std::mutex _lock;

    std::multimap<std::string, bool> _queries;
};