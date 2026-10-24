#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include "config/Config.h"

namespace
{
    class TemporaryConfig
    {
    public:
        explicit TemporaryConfig(const std::string& content)
        {
            static int counter = 0;

            path_ = std::filesystem::temp_directory_path()
                / ("search_engine_test_config_"
                    + std::to_string(++counter) + ".ini");

            std::ofstream file(path_);

            if (!file)
            {
                throw std::runtime_error(
                    "Не удалось создать временный конфигурационный файл"
                );
            }

            file << content;
        }

        ~TemporaryConfig()
        {
            std::error_code error;
            std::filesystem::remove(path_, error);
        }

        std::string path() const
        {
            return path_.string();
        }

    private:
        std::filesystem::path path_;
    };
}

TEST_CASE("Config trims spaces around keys and values")
{
    TemporaryConfig file(
        "  [database]  \n"
        "  host = localhost  \n"
    );

    const Config config(file.path());

    REQUIRE(config.get("database.host") == "localhost");
}

TEST_CASE("Config reports missing settings")
{
    TemporaryConfig file(
        "[database]\n"
        "host=localhost\n"
    );

    const Config config(file.path());

    REQUIRE_THROWS_AS(
        config.get("database.password"),
        std::runtime_error
    );
}

TEST_CASE("Config rejects malformed lines")
{
    TemporaryConfig file(
        "[database]\n"
        "host localhost\n"
    );

    REQUIRE_THROWS_AS(
        Config(file.path()),
        std::runtime_error
    );
}

TEST_CASE("Config rejects duplicate settings")
{
    TemporaryConfig file(
        "[database]\n"
        "host=localhost\n"
        "host=127.0.0.1\n"
    );

    REQUIRE_THROWS_AS(
        Config(file.path()),
        std::runtime_error
    );
}

TEST_CASE("Config rejects empty required settings")
{
    TemporaryConfig file(
        "[database]\n"
        "host=\n"
    );

    const Config config(file.path());

    REQUIRE_THROWS_AS(
        config.get("database.host"),
        std::runtime_error
    );
}
    TEST_CASE("Config accepts UTF-8 BOM")
    {
        TemporaryConfig file(
            std::string("\xEF\xBB\xBF") +
            "[database]\n"
            "host=localhost\n"
        );

        const Config config(file.path());

        REQUIRE(config.get("database.host") == "localhost");
    }