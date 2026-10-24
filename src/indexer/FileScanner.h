
#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <set>
#include <stdexcept>
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

        if (!fs::exists(rootDirectory))
        {
            throw std::runtime_error(
                "Root directory does not exist: "
                + rootDirectory.string()
            );
        }

        if (!fs::is_directory(rootDirectory))
        {
            throw std::runtime_error(
                "Root path is not a directory: "
                + rootDirectory.string()
            );
        }

        const std::set<std::string> extensions =
            parseExtensions(extension);

        std::vector<fs::path> files;

        for (const auto& entry :
            fs::recursive_directory_iterator(rootDirectory))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }

            const std::string fileExtension =
                toLower(entry.path().extension().string());

            if (extensions.contains(fileExtension))
            {
                files.push_back(
                    fs::absolute(entry.path()).lexically_normal()
                );
            }
        }

        std::sort(files.begin(), files.end());

        return files;
    }

private:
    static std::string toLower(std::string value)
    {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](unsigned char character)
            {
                return static_cast<char>(
                    std::tolower(character)
                    );
            }
        );

        return value;
    }

    static std::set<std::string> parseExtensions(
        const std::string& input)
    {
        std::set<std::string> extensions;
        std::size_t start = 0;

        while (start <= input.size())
        {
            const std::size_t separator =
                input.find(',', start);

            const std::size_t end =
                separator == std::string::npos
                ? input.size()
                : separator;

            std::string extension =
                input.substr(start, end - start);

            const auto first =
                extension.find_first_not_of(" \t\r\n");

            if (first != std::string::npos)
            {
                const auto last =
                    extension.find_last_not_of(" \t\r\n");

                extension =
                    extension.substr(first, last - first + 1);

                extension = toLower(extension);

                if (extension.front() != '.')
                {
                    extension.insert(0, ".");
                }

                extensions.insert(extension);
            }

            if (separator == std::string::npos)
            {
                break;
            }

            start = separator + 1;
        }

        if (extensions.empty())
        {
            throw std::runtime_error(
                "At least one file extension must be specified"
            );
        }

        return extensions;
    }
};
