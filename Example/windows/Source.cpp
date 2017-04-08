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
#include <Database.h>

int main()
{
    try
    {
        Database db("localhost", "user", "password", "mysql", 1);

        // This functions should be used only for no returning queries like insert, update, etc...
        // show tables will return but in this case is used only for demostration
        db.DirectExecuteQuery("show tables"); // Executes the query NOW

        db.AsynExecuteQuery("show tables"); // Executes the query in the near future, order of execution is no guaranted,
                                            // can be really fast, speacilly when mysql server is in another host.

        // ResultQuery is used in returning queries like show, select, etc...
        // It will return a DatabaseResult object, this is a std::vector<std::string>
        DatabaseResult result = db.ResultQuery("show tables");

        std::cout << "Amount of rows = " << result.size() << std::endl;

        for (auto &result_string : result)
            std::cout << result_string << std::endl;
    }
    catch (DatabaseException e)
    {
        std::cerr << "MySql error: " << e.what() << std::endl;
    }

    system("PAUSE");

    return 0;
}