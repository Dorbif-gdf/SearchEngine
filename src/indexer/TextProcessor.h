
#pragma once

#include "../WordNormalizer.h"

#include <fstream>
#include <map>
#include <stdexcept>
#include <string>

class TextProcessor
{
public:
    std::map<std::string, int> countWords(
        const std::string& filename) const
    {
        std::ifstream file(filename, std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error(
                "Unable to open file: " + filename
            );
        }

        std::string text;
        std::string line;

        while (std::getline(file, line))
        {
            text += line;
            text += '\n';
        }

        if (file.bad())
        {
            throw std::runtime_error(
                "Error reading file: " + filename
            );
        }

        return WordNormalizer::processUtf8(text);
    }
};
