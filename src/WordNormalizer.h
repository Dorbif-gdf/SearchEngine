
#pragma once

#include <boost/locale/encoding_utf.hpp>

#include <map>
#include <string>

namespace WordNormalizer
{
    inline wchar_t toLower(wchar_t character)
    {
        if (character >= L'A' && character <= L'Z')
        {
            return character + (L'a' - L'A');
        }

        if (character >= L'\u0410' &&
            character <= L'\u042F')
        {
            return character + 0x20;
        }

        if (character == L'\u0401')
        {
            return L'\u0451';
        }

        return character;
    }

    inline bool isLetter(wchar_t character)
    {
        return
            (character >= L'a' && character <= L'z') ||
            (character >= L'\u0430' &&
                character <= L'\u044F') ||
            character == L'\u0451';
    }

    inline void saveWord(
        const std::wstring& word,
        std::map<std::string, int>& frequencies)
    {
        if (word.size() < 3 || word.size() > 32)
        {
            return;
        }

        const std::string utf8Word =
            boost::locale::conv::utf_to_utf<char>(word);

        ++frequencies[utf8Word];
    }

    inline std::map<std::string, int> countWords(
        const std::wstring& text)
    {
        std::map<std::string, int> frequencies;
        std::wstring currentWord;

        for (wchar_t character : text)
        {
            character = toLower(character);

            if (isLetter(character))
            {
                currentWord += character;
            }
            else
            {
                saveWord(currentWord, frequencies);
                currentWord.clear();
            }
        }

        saveWord(currentWord, frequencies);

        return frequencies;
    }

    inline std::map<std::string, int> processUtf8(
        const std::string& text)
    {
        const std::wstring wideText =
            boost::locale::conv::utf_to_utf<wchar_t>(text);

        return countWords(wideText);
    }
}
