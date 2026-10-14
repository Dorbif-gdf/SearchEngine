
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
        std::ifstream file(filename);

        if (!file.is_open())
        {
            throw std::runtime_error(
                "Не удалось открыть файл настроек: " + filename
            );
        }

        std::string line;
        std::string section;

        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == ';' || line[0] == '#')
            {
                continue;
            }

            if (line.front() == '[' && line.back() == ']')
            {
                section = line.substr(1, line.size() - 2);
                continue;
            }

            const std::size_t separator = line.find('=');

            if (separator == std::string::npos)
            {
                continue;
            }

            const std::string key = line.substr(0, separator);
            const std::string value = line.substr(separator + 1);

            values_[section + "." + key] = value;
        }
    }

    std::string get(const std::string& key) const
    {
        const auto it = values_.find(key);

        if (it == values_.end())
        {
            throw std::runtime_error(
                "Не найдена настройка: " + key
            );
        }

        return it->second;
    }

private:
    std::map<std::string, std::string> values_;
};
