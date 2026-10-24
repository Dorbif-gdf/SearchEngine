
#pragma once

#include "../WordNormalizer.h"

#include <map>
#include <string>

class QueryProcessor
{
public:
    std::map<std::string, int> process(
        const std::string& query) const
    {
        return WordNormalizer::processUtf8(query);
    }
};
