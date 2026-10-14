
#pragma once

#include <boost/locale.hpp>

#include <fstream>
#include <locale>
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
                "Не удалось открыть файл: " + filename
            );
        }

        std::string text;
        std::string line;

        while (std::getline(file, line))
        {
            text += line;
            text += '\n';
        }

        boost::locale::generator generator;
        const std::locale locale = generator("ru_RU.UTF-8");

        const std::wstring wideText =
            boost::locale::conv::utf_to_utf<wchar_t>(text);

        const auto& characterType =
            std::use_facet<std::ctype<wchar_t>>(locale);

        std::map<std::string, int> frequencies;
        std::wstring currentWord;

        for (wchar_t character : wideText)
        {
            if (characterType.is(
                std::ctype_base::alpha, character))
            {
                currentWord += characterType.tolower(character);
            }
            else
            {
                saveWord(currentWord, frequencies);
            }
        }

        saveWord(currentWord, frequencies);

        return frequencies;
    }

private:
    static void saveWord(
        std::wstring& word,
        std::map<std::string, int>& frequencies)
    {
        if (word.length() >= 3 && word.length() <= 32)
        {
            const std::string utf8Word =
                boost::locale::conv::utf_to_utf<char>(word);

            ++frequencies[utf8Word];
        }

        word.clear();
    }
};
