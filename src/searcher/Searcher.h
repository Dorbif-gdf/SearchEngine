
#pragma once

#include <pqxx/pqxx>

#include <map>
#include <string>
#include <vector>

struct SearchResult
{
    std::string filename;
    std::string path;
    long long relevance;
};

class Searcher
{
public:
    explicit Searcher(pqxx::connection& connection)
        : connection_(connection)
    {
    }

    std::vector<SearchResult> search(
        const std::map<std::string, int>& queryWords)
    {
        std::vector<SearchResult> results;

        if (queryWords.empty())
        {
            return results;
        }

        pqxx::read_transaction transaction(connection_);

        std::string query = R"(
            SELECT
                d.filename,
                d.path,
                SUM(dw.frequency)::BIGINT AS relevance
            FROM documents d
            JOIN document_words dw
                ON dw.document_id = d.id
            JOIN words w
                ON w.id = dw.word_id
            WHERE w.word IN (
        )";

        bool firstWord = true;

        for (const auto& [word, frequency] : queryWords)
        {
            if (!firstWord)
            {
                query += ", ";
            }

            query += transaction.quote(word);
            firstWord = false;
        }

        query += R"(
            )
            GROUP BY d.id, d.filename, d.path
            HAVING COUNT(DISTINCT w.word) = )";

        query += std::to_string(queryWords.size());

        query += R"(
            ORDER BY relevance DESC, d.filename ASC
            LIMIT )";

        query += "10";

        const pqxx::result rows = transaction.exec(query);

        for (const auto& row : rows)
        {
            results.push_back({
                row["filename"].as<std::string>(),
                row["path"].as<std::string>(),
                row["relevance"].as<long long>()
                });
        }

        return results;
    }

private:
    pqxx::connection& connection_;
};
