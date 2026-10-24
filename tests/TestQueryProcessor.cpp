
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

TEST_CASE("Russian uppercase and lowercase words match")
{
    QueryProcessor processor;

    const auto words = processor.process("ДОГОВОР договор");

    REQUIRE(words.size() == 1);
    CHECK(words.at("договор") == 2);
}

TEST_CASE("Russian Yo is converted to lowercase")
{
    QueryProcessor processor;

    const auto words = processor.process("ЁЛКА ёлка");

    REQUIRE(words.size() == 1);
    CHECK(words.at("ёлка") == 2);
}

TEST_CASE("English uppercase and lowercase words match")
{
    QueryProcessor processor;

    const auto words = processor.process("CONTRACT contract");

    REQUIRE(words.size() == 1);
    CHECK(words.at("contract") == 2);
}

TEST_CASE("English and Russian words are processed together")
{
    QueryProcessor processor;

    const auto words =
        processor.process("CONTRACT ДОГОВОР contract договор");

    REQUIRE(words.size() == 2);
    CHECK(words.at("contract") == 2);
    CHECK(words.at("договор") == 2);
}
