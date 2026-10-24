
#include <catch2/catch_test_macros.hpp>

#include "indexer/FileScanner.h"

#include <filesystem>
#include <fstream>
#include <set>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

class TemporaryDirectory
{
public:
    TemporaryDirectory()
    {
        path_ = fs::temp_directory_path()
            / ("search_engine_scanner_test_"
                + std::to_string(counter_++));

        fs::create_directories(path_);
    }

    ~TemporaryDirectory()
    {
        std::error_code error;
        fs::remove_all(path_, error);
    }

    const fs::path& path() const
    {
        return path_;
    }

    void createFile(
        const fs::path& relativePath,
        const std::string& content = "test content")
    {
        const fs::path fullPath = path_ / relativePath;

        fs::create_directories(fullPath.parent_path());

        std::ofstream file(fullPath, std::ios::binary);
        REQUIRE(file.is_open());

        file << content;
        REQUIRE(file.good());
    }

private:
    fs::path path_;
    inline static unsigned int counter_ = 0;
};

TEST_CASE("FileScanner finds files recursively")
{
    TemporaryDirectory directory;

    directory.createFile("first.txt");
    directory.createFile("nested/second.txt");
    directory.createFile("nested/deeper/third.txt");
    directory.createFile("ignored.md");

    FileScanner scanner;
    const auto files = scanner.findFiles(directory.path(), ".txt");

    REQUIRE(files.size() == 3);

    for (const auto& file : files)
    {
        CHECK(file.is_absolute());
        CHECK(file.extension() == ".txt");
    }
}

TEST_CASE("FileScanner supports multiple extensions")
{
    TemporaryDirectory directory;

    directory.createFile("first.txt");
    directory.createFile("second.md");
    directory.createFile("third.cpp");

    FileScanner scanner;
    const auto files =
        scanner.findFiles(directory.path(), ".txt,.md");

    REQUIRE(files.size() == 2);

    std::set<std::string> foundExtensions;

    for (const auto& file : files)
    {
        foundExtensions.insert(file.extension().string());
    }

    CHECK(foundExtensions.contains(".txt"));
    CHECK(foundExtensions.contains(".md"));
}

TEST_CASE("FileScanner ignores extension case")
{
    TemporaryDirectory directory;

    directory.createFile("first.txt");
    directory.createFile("second.TXT");
    directory.createFile("third.TxT");
    directory.createFile("ignored.md");

    FileScanner scanner;
    const auto files = scanner.findFiles(directory.path(), ".txt");

    CHECK(files.size() == 3);
}

TEST_CASE("FileScanner trims spaces around extensions")
{
    TemporaryDirectory directory;

    directory.createFile("first.txt");
    directory.createFile("second.md");

    FileScanner scanner;
    const auto files =
        scanner.findFiles(directory.path(), " .txt , .md ");

    CHECK(files.size() == 2);
}

TEST_CASE("FileScanner rejects a missing directory")
{
    FileScanner scanner;

    const fs::path missingPath =
        fs::temp_directory_path()
        / "search_engine_missing_directory_928371";

    CHECK_THROWS_AS(
        scanner.findFiles(missingPath, ".txt"),
        std::runtime_error
    );
}

TEST_CASE("FileScanner rejects a regular file as root")
{
    TemporaryDirectory directory;
    directory.createFile("not_a_directory.txt");

    FileScanner scanner;

    CHECK_THROWS_AS(
        scanner.findFiles(
            directory.path() / "not_a_directory.txt",
            ".txt"
        ),
        std::runtime_error
    );
}

TEST_CASE("FileScanner rejects empty extensions")
{
    TemporaryDirectory directory;
    FileScanner scanner;

    CHECK_THROWS_AS(
        scanner.findFiles(directory.path(), ""),
        std::runtime_error
    );

    CHECK_THROWS_AS(
        scanner.findFiles(directory.path(), " ,  "),
        std::runtime_error
    );
}
