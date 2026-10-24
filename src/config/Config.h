
#pragma once

#include <fstream>
#include <map>
#include <stdexcept>
#include <string>

class Config
{
public:
    explicit Config(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error(
                "Unable to open configuration file: " + filename
            );
        }

        std::string line;
        std::string section;
        std::size_t lineNumber = 0;

        while (std::getline(file, line))
        {
            ++lineNumber;

            // Remove UTF-8 BOM if it is present at the start of the file.
            if (lineNumber == 1 &&
                line.size() >= 3 &&
                static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB &&
                static_cast<unsigned char>(line[2]) == 0xBF)
            {
                line.erase(0, 3);
            }

            line = trim(line);

            if (line.empty() || line[0] == ';' || line[0] == '#')
            {
                continue;
            }

            if (line.front() == '[')
            {
                if (line.size() < 3 || line.back() != ']')
                {
                    throw std::runtime_error(
                        "Invalid section at line "
                        + std::to_string(lineNumber)
                    );
                }

                section = trim(line.substr(1, line.size() - 2));

                if (section.empty())
                {
                    throw std::runtime_error(
                        "Empty section name at line "
                        + std::to_string(lineNumber)
                    );
                }

                continue;
            }

            const std::size_t separator = line.find('=');

            if (separator == std::string::npos)
            {
                throw std::runtime_error(
                    "Missing '=' at line "
                    + std::to_string(lineNumber)
                );
            }

            if (section.empty())
            {
                throw std::runtime_error(
                    "Setting outside a section at line "
                    + std::to_string(lineNumber)
                );
            }

            const std::string key =
                trim(line.substr(0, separator));

            const std::string value =
                trim(line.substr(separator + 1));

            if (key.empty())
            {
                throw std::runtime_error(
                    "Empty setting name at line "
                    + std::to_string(lineNumber)
                );
            }

            const std::string fullKey = section + "." + key;

            if (values_.contains(fullKey))
            {
                throw std::runtime_error(
                    "Duplicate setting: " + fullKey
                );
            }

            values_[fullKey] = value;
        }

        if (file.bad())
        {
            throw std::runtime_error(
                "Error reading configuration file: " + filename
            );
        }
    }

    std::string get(const std::string& key) const
    {
        const auto it = values_.find(key);

        if (it == values_.end())
        {
            throw std::runtime_error(
                "Missing setting: " + key
            );
        }

        if (it->second.empty() && key != "database.password")
        {
            throw std::runtime_error(
                "Empty setting value: " + key
            );
        }

        return it->second;
    }

private:
    static std::string trim(const std::string& value)
    {
        const auto first = value.find_first_not_of(" \t\r\n");

        if (first == std::string::npos)
        {
            return {};
        }

        const auto last = value.find_last_not_of(" \t\r\n");

        return value.substr(first, last - first + 1);
    }

    std::map<std::string, std::string> values_;
};
