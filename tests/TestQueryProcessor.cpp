
#include <catch2/catch_test_macros.hpp>

#include "searcher/QueryProcessor.h"

TEST_CASE("Query is converted to lowercase")
{
    QueryProcessor processor;

    const auto words = processor.process("ДОГОВОР");

    REQUIRE(words.size() == 1);
    CHECK(words.contains("договор"));
}

TEST_CASE("Short words are skipped")
{
    QueryProcessor processor;

    const auto words = processor.process("ДОГОВОР, ПРАВА!");

    REQUIRE(words.size() == 2);
    CHECK(words.contains("договор"));
    CHECK(words.contains("права"));
}

TEST_CASE("Punctuation separates words")
{
    QueryProcessor processor;

    const auto words = processor.process("и в договор");

    REQUIRE(words.size() == 1);
    CHECK(words.contains("договор"));
}

TEST_CASE("Repeated words are counted")
{
    QueryProcessor processor;

    const auto words = processor.process("договор договор права");

    REQUIRE(words.size() == 2);
    CHECK(words.at("договор") == 2);
    CHECK(words.at("права") == 1);
}
