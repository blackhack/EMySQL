#include <iostream>
#include <Database.h>

int main()
{
    try
    {
        Database db("192.168.2.145", "root", "lula2013", "mysql", 1);

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