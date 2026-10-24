#include <iostream>
#include <string>

#include <pqxx/pqxx>

#include "config/Config.h"
#include "searcher/Searcher.h"
#include "searcher/QueryProcessor.h"
#ifdef _WIN32
#include <windows.h>
#endif

int main()
{

#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    try
    {
        const Config config("config/config.ini");

        const std::string connection_string =
            "host=" + config.get("database.host") +
            " port=" + config.get("database.port") +
            " dbname=" + config.get("database.name") +
            " user=" + config.get("database.user") +
            " password=" + config.get("database.password");

        pqxx::connection connection(connection_string);

        Searcher searcher(connection);
        QueryProcessor queryProcessor;

        std::cout << "Enter a search query (type exit to quit).\n";

        std::string query;

        while (true)
        {
            std::cout << "> ";

            if (!std::getline(std::cin, query))
            {
                break;
            }

            if (query == "exit")
            {
                break;
            }

            const auto queryWords = queryProcessor.process(query);

            if (queryWords.empty())
            {
                std::cout
                    << "Enter at least one word of 3 to 32 characters.\n";
                continue;
            }

            if (queryWords.size() > 4)
            {
                std::cout
                    << "Error: no more than 4 distinct words are allowed.\n";
                continue;
            }

            const auto results = searcher.search(queryWords);

            if (results.empty())
            {
                std::cout << "No results found.\n";
                continue;
            }

            for (const auto& result : results)
            {
                std::cout << "File: " << result.filename << '\n';
                std::cout << "Path: " << result.path << '\n';
                std::cout << "Relevance: "
                    << result.relevance << "\n\n";
            }
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "Searcher error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}