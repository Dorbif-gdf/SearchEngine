#include <iostream>
#include <algorithm>
#include <cstddef>
#include <future>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include <pqxx/pqxx>

#include "config/Config.h"
#include "indexer/TextProcessor.h"
#include "indexer/FileScanner.h"
#include "indexer/DatabaseIndexer.h"
#include "indexer/ThreadPool.h"

int main()
{
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

        pqxx::work transaction(connection);

        transaction.exec(R"(
            CREATE TABLE IF NOT EXISTS documents (
                id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
                path TEXT NOT NULL UNIQUE,
                filename TEXT NOT NULL,
                indexed_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP
            );
        )");

        transaction.exec(R"(
            CREATE TABLE IF NOT EXISTS words (
                id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
                word TEXT NOT NULL UNIQUE
            );
        )");

        transaction.exec(R"(
            CREATE TABLE IF NOT EXISTS document_words (
                document_id BIGINT NOT NULL
                    REFERENCES documents(id) ON DELETE CASCADE,
                word_id BIGINT NOT NULL
                    REFERENCES words(id) ON DELETE CASCADE,
                frequency INTEGER NOT NULL CHECK (frequency > 0),
                PRIMARY KEY (document_id, word_id)
            );
        )");

        transaction.exec(R"(
            CREATE INDEX IF NOT EXISTS idx_document_words_word_id
            ON document_words(word_id);
        )");

        transaction.commit();

        std::cout << "Database structure created successfully.\n";

        FileScanner scanner;

        const auto files = scanner.findFiles(
            config.get("search.root_directory"),
            config.get("search.extensions")
        );

        std::cout << "Text files found: " << files.size() << '\n';

        TextProcessor processor;
        DatabaseIndexer databaseIndexer(connection);

        const unsigned int hardwareThreads =
            std::thread::hardware_concurrency();

        const std::size_t availableThreads =
            hardwareThreads == 0 ? 2 : hardwareThreads;

        const std::size_t threadCount = std::max<std::size_t>(
            1,
            std::min(availableThreads,
                std::max<std::size_t>(1, files.size()))
        );

        ThreadPool pool(threadCount);

        std::cout << "Worker threads: " << threadCount << '\n';

        using Frequencies = std::map<std::string, int>;

        std::vector<std::future<Frequencies>> results;
        results.reserve(files.size());

        for (const auto& file : files)
        {
            results.push_back(pool.submit(
                [&processor, file]()
                {
                    return processor.countWords(file.string());
                }
            ));
        }

        std::size_t indexedFiles = 0;
        std::size_t failedFiles = 0;

        for (std::size_t i = 0; i < files.size(); ++i)
        {
            Frequencies frequencies;

            try
            {
                frequencies = results[i].get();
            }
            catch (const std::exception& error)
            {
                std::cerr
                    << "Failed to process file: "
                    << files[i].string()
                    << "; error: "
                    << error.what()
                    << '\n';

                ++failedFiles;
                continue;
            }

            databaseIndexer.saveDocument(
                files[i].string(),
                files[i].filename().string(),
                frequencies
            );

            std::cout
                << "Indexed: "
                << files[i].filename().string()
                << "; unique words: "
                << frequencies.size()
                << '\n';

            ++indexedFiles;
        }

        std::cout << "\nIndexing finished.\n"
            << "Successfully indexed: " << indexedFiles << '\n'
            << "Failed to process: " << failedFiles << '\n';

        if (failedFiles > 0)
        {
            std::cerr
                << "Indexing completed with errors.\n";

            return 1;
        }

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Indexer error: " << error.what() << '\n';
        return 1;
    }
}
