
#pragma once

#include <pqxx/pqxx>

#include <map>
#include <stdexcept>
#include <string>

class DatabaseIndexer
{
public:
    explicit DatabaseIndexer(pqxx::connection& connection)
        : connection_(connection)
    {
    }

    void saveDocument(
        const std::string& path,
        const std::string& filename,
        const std::map<std::string, int>& frequencies)
    {
        pqxx::work transaction(connection_);

        const auto documentResult = transaction.exec(
            "INSERT INTO documents (path, filename) VALUES (" +
            transaction.quote(path) + ", " +
            transaction.quote(filename) + ") "
            "ON CONFLICT (path) DO UPDATE SET "
            "filename = EXCLUDED.filename, "
            "indexed_at = CURRENT_TIMESTAMP "
            "RETURNING id;"
        );

        const long long documentId =
            documentResult[0][0].as<long long>();

        transaction.exec(
            "DELETE FROM document_words WHERE document_id = " +
            std::to_string(documentId) + ";"
        );

        for (const auto& [word, frequency] : frequencies)
        {
            const auto wordResult = transaction.exec(
                "INSERT INTO words (word) VALUES (" +
                transaction.quote(word) + ") "
                "ON CONFLICT (word) DO NOTHING "
                "RETURNING id;"
            );

            long long wordId = 0;

            if (!wordResult.empty())
            {
                wordId = wordResult[0][0].as<long long>();
            }
            else
            {
                const auto existingWord = transaction.exec(
                    "SELECT id FROM words WHERE word = " +
                    transaction.quote(word) + ";"
                );

                if (existingWord.empty())
                {
                    throw std::runtime_error("Failed to get word ID");
                }

                wordId = existingWord[0][0].as<long long>();
            }

            transaction.exec(
                "INSERT INTO document_words "
                "(document_id, word_id, frequency) VALUES (" +
                std::to_string(documentId) + ", " +
                std::to_string(wordId) + ", " +
                std::to_string(frequency) + ");"
            );
        }

        transaction.commit();
    }

private:
    pqxx::connection& connection_;
};
