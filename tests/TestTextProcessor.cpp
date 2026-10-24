
#include <catch2/catch_test_macros.hpp>

#include "indexer/TextProcessor.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

class TemporaryTextFile
{
public:
    explicit TemporaryTextFile(const std::string& content)
    {
        path_ = fs::temp_directory_path()
            / ("search_engine_test_"
                + std::to_string(counter_++)
                + ".txt");

        std::ofstream file(path_, std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error(
                "Could not create temporary test file"
            );
        }

        file.write(
            content.data(),
            static_cast<std::streamsize>(content.size())
        );

        if (!file)
        {
            throw std::runtime_error(
                "Could not write temporary test file"
            );
        }
    }

    ~TemporaryTextFile()
    {
        std::error_code error;
        fs::remove(path_, error);
    }

    const fs::path& path() const
    {
        return path_;
    }

private:
    fs::path path_;
    inline static unsigned int counter_ = 0;
};

TEST_CASE("TextProcessor counts repeated English words")
{
    TemporaryTextFile file("contract contract agreement");

    TextProcessor processor;
    const auto words = processor.countWords(file.path().string());

    REQUIRE(words.size() == 2);
    CHECK(words.at("contract") == 2);
    CHECK(words.at("agreement") == 1);
}

TEST_CASE("TextProcessor handles Russian uppercase and lowercase")
{
    const std::string russianWord =
        "\xD0\x94\xD0\x9E\xD0\x93\xD0\x9E\xD0\x92\xD0\x9E\xD0\xA0";

    const std::string lowerRussianWord =
        "\xD0\xB4\xD0\xBE\xD0\xB3\xD0\xBE\xD0\xB2\xD0\xBE\xD1\x80";

    TemporaryTextFile file(
        russianWord + " " + lowerRussianWord
    );

    TextProcessor processor;
    const auto words = processor.countWords(file.path().string());

    REQUIRE(words.size() == 1);
    CHECK(words.at(lowerRussianWord) == 2);
}

TEST_CASE("TextProcessor handles Russian Yo")
{
    const std::string upperYo = "\xD0\x81\xD0\x9B\xD0\x9A\xD0\x90";
    const std::string lowerYo = "\xD1\x91\xD0\xBB\xD0\xBA\xD0\xB0";

    TemporaryTextFile file(upperYo + " " + lowerYo);

    TextProcessor processor;
    const auto words = processor.countWords(file.path().string());

    REQUIRE(words.size() == 1);
    CHECK(words.at(lowerYo) == 2);
}

TEST_CASE("TextProcessor handles English and Russian together")
{
    const std::string russianUpper =
        "\xD0\x94\xD0\x9E\xD0\x93\xD0\x9E\xD0\x92\xD0\x9E\xD0\xA0";

    const std::string russianLower =
        "\xD0\xB4\xD0\xBE\xD0\xB3\xD0\xBE\xD0\xB2\xD0\xBE\xD1\x80";

    TemporaryTextFile file(
        "CONTRACT " + russianUpper + " contract " + russianLower
    );

    TextProcessor processor;
    const auto words = processor.countWords(file.path().string());

    REQUIRE(words.size() == 2);
    CHECK(words.at("contract") == 2);
    CHECK(words.at(russianLower) == 2);
}

TEST_CASE("TextProcessor skips words shorter than three characters")
{
    TemporaryTextFile file(
        "a an the \xD0\xB4\xD0\xBE\xD0\xB3\xD0\xBE\xD0\xB2\xD0\xBE\xD1\x80"
    );

    TextProcessor processor;
    const auto words = processor.countWords(file.path().string());

    REQUIRE(words.size() == 2);
    CHECK(words.contains("the"));
    CHECK(words.contains(
        "\xD0\xB4\xD0\xBE\xD0\xB3\xD0\xBE\xD0\xB2\xD0\xBE\xD1\x80"
    ));
}

TEST_CASE("TextProcessor reports missing files")
{
    TextProcessor processor;

    const fs::path missingPath =
        fs::temp_directory_path()
        / "search_engine_file_that_does_not_exist_928371.txt";

    CHECK_THROWS(
        processor.countWords(missingPath.string())
    );
}
