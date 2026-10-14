
#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include <vector>

class FileScanner
{
public:
    std::vector<std::filesystem::path> findFiles(
        const std::filesystem::path& rootDirectory,
        const std::string& extension) const
    {
        namespace fs = std::filesystem;

        std::vector<fs::path> files;

        if (!fs::exists(rootDirectory) ||
            !fs::is_directory(rootDirectory))
        {
            return files;
        }

        std::string normalizedExtension = extension;

        std::transform(
            normalizedExtension.begin(),
            normalizedExtension.end(),
            normalizedExtension.begin(),
            [](unsigned char character)
            {
                return static_cast<char>(std::tolower(character));
            });

        if (!normalizedExtension.empty() &&
            normalizedExtension.front() != '.')
        {
            normalizedExtension.insert(0, ".");
        }

        for (const auto& entry :
            fs::recursive_directory_iterator(rootDirectory))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }

            std::string fileExtension =
                entry.path().extension().string();

            std::transform(
                fileExtension.begin(),
                fileExtension.end(),
                fileExtension.begin(),
                [](unsigned char character)
                {
                    return static_cast<char>(std::tolower(character));
                });

            if (fileExtension == normalizedExtension)
            {
                files.push_back(fs::absolute(entry.path()).lexically_normal());
            }
        }

        std::sort(files.begin(), files.end());

        return files;
    }
};
