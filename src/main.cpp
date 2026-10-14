#include <iostream>
#include <cstddef>
#include <future>
#include <algorithm>
#include <map>
#include <string>
#include <thread>
#include <vector>
#include <sstream>
#include <cctype>

#include <pqxx/pqxx>

#include "config/Config.h"
#include "indexer/TextProcessor.h"
#include "indexer/FileScanner.h"
#include "indexer/DatabaseIndexer.h"
#include "indexer/ThreadPool.h"

#include "searcher/Searcher.h"
#include "searcher/QueryProcessor.h"

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

        std::cout << "Структура базы данных успешно создана!\n";

        {
            pqxx::read_transaction read_transaction(connection);

            const pqxx::result result = read_transaction.exec(
                "SELECT table_name "
                "FROM information_schema.tables "
                "WHERE table_schema = 'public' "
                "AND table_name IN "
                "('documents', 'words', 'document_words') "
                "ORDER BY table_name;"
            );

            std::cout << "\nТаблицы в базе данных:\n";

            for (const auto& row : result)
            {
                std::cout << "- "
                    << row["table_name"].c_str()
                    << '\n';
            }
        }

        FileScanner scanner;

        const auto files = scanner.findFiles(
            config.get("search.root_directory"),
            config.get("search.extensions")
        );

        std::cout << "\nНайдено текстовых файлов: "
            << files.size() << '\n';


        TextProcessor processor;
        DatabaseIndexer databaseIndexer(connection);

        const std::size_t threadCount = std::max(
            1u,
            std::min(
                std::thread::hardware_concurrency() == 0
                ? 2u
                : std::thread::hardware_concurrency(),
                static_cast<unsigned int>(
                    std::max<std::size_t>(1, files.size())
                    )
            )
        );

        ThreadPool pool(threadCount);

        std::cout << "\nКоличество рабочих потоков: "
            << threadCount << '\n';

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

        for (std::size_t i = 0; i < files.size(); ++i)
        {
            const Frequencies frequencies = results[i].get();

            databaseIndexer.saveDocument(
                files[i].string(),
                files[i].filename().string(),
                frequencies
            );

            std::cout << "Обработан файл: "
                << files[i].filename().string()
                << "; уникальных слов: "
                << frequencies.size() << '\n';
        }


        Searcher searcher(connection);

        std::cout << "\nВведите запрос для поиска "
            << "(для выхода введите exit):\n";

        std::string query;

        while (true)
        {
            std::cout << "> ";
            std::getline(std::cin, query);

            if (query == "exit")
            {
                break;
            }

            QueryProcessor queryProcessor;
            const auto queryWords = queryProcessor.process(query);

            if (queryWords.empty())
            {
                std::cout << "Введите хотя бы одно слово длиной от 3 до 32 символов.\n";
                continue;
            }

            if (queryWords.size() > 4)
            {
                std::cout << "Ошибка: запрос должен содержать не более 4 разных слов.\n";
                continue;
            }

            const auto results = searcher.search(queryWords);

            if (results.empty())
            {
                std::cout << "Ничего не найдено.\n";
                continue;
            }

            for (const auto& result : results)
            {
                std::cout << "Файл: " << result.filename << '\n';
                std::cout << "Путь: " << result.path << '\n';
                std::cout << "Релевантность: "
                    << result.relevance << "\n\n";
            }
        }


    }

    catch (const std::exception& error)
    {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }



    return 0;
}
