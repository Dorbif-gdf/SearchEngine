
#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <set>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

class FileScanner
{
public:
    std::vector<std::filesystem::path> findFiles(
        const std::filesystem::path& rootDirectory,
        const std::string& extension) const
    {
        namespace fs = std::filesystem;

        std::error_code error;

        if (!fs::exists(rootDirectory, error) || error)
        {
            throw std::runtime_error(
                "Root directory does not exist: "
                + rootDirectory.string()
            );
        }

        if (!fs::is_directory(rootDirectory, error) || error)
        {
            throw std::runtime_error(
                "Root path is not a directory: "
                + rootDirectory.string()
            );
        }

        const std::set<std::string> extensions =
            parseExtensions(extension);

        std::vector<fs::path> files;

        fs::recursive_directory_iterator iterator(
            rootDirectory,
            fs::directory_options::skip_permission_denied,
            error
        );

        const fs::recursive_directory_iterator end;

        if (error)
        {
            throw std::runtime_error(
                "Cannot scan root directory: "
                + rootDirectory.string()
            );
        }

        while (iterator != end)
        {
            const fs::directory_entry entry = *iterator;

            error.clear();

            if (entry.is_regular_file(error) && !error)
            {
                const std::string fileExtension =
                    toLower(entry.path().extension().string());

                if (extensions.contains(fileExtension))
                {
                    std::error_code pathError;

                    fs::path absolutePath =
                        fs::absolute(entry.path(), pathError);

                    if (!pathError)
                    {
                        files.push_back(
                            absolutePath.lexically_normal()
                        );
                    }
                }
            }

            error.clear();
            iterator.increment(error);

            if (error)
            {
                error.clear();
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
