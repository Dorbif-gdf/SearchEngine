
#pragma once

#include <boost/locale.hpp>
#include <boost/locale/encoding_utf.hpp>

#include <map>
#include <string>

class QueryProcessor
{
public:
    std::map<std::string, int> process(const std::string& query) const
    {
        std::map<std::string, int> words;

        // Создаём локаль с поддержкой Unicode.
        boost::locale::generator generator;
        const std::locale unicodeLocale = generator("en_US.UTF-8");

        // Приводим текст к нижнему регистру.
        const std::string lowerQuery =
            boost::locale::to_lower(query, unicodeLocale);

        // Преобразуем UTF-8 в широкую строку.
        const std::wstring text =
            boost::locale::conv::utf_to_utf<wchar_t>(lowerQuery);

        std::wstring currentWord;

        for (wchar_t ch : text)
        {
            if (isLetter(ch))
            {
                currentWord += ch;
            }
            else
            {
                addWord(words, currentWord);
                currentWord.clear();
            }
        }

        addWord(words, currentWord);

        return words;
    }

private:
    static bool isLetter(wchar_t ch)
    {
        return
            (ch >= L'a' && ch <= L'z') ||
            (ch >= L'\u0430' && ch <= L'\u044F') ||
            ch == L'\u0451';
    }

    static void addWord(
        std::map<std::string, int>& words,
        const std::wstring& word)
    {
        if (word.size() < 3 || word.size() > 32)
        {
            return;
        }

        const std::string utf8Word =
            boost::locale::conv::utf_to_utf<char>(word);

        ++words[utf8Word];
    }
};
