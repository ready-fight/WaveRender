#include "FileSystem.hpp"

#include <fstream>
#include <stdexcept>

namespace Wave
{
    std::vector<u8> FileSystem::ReadBinaryFile(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);

        if (!file)
        {
            throw std::runtime_error("Failed to open binary file: " + path.string());
        }

        const std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<u8> data(static_cast<size_t>(size));

        if (!file.read(reinterpret_cast<char*>(data.data()), size))
        {
            throw std::runtime_error("Failed to read binary file: " + path.string());
        }

        return data;
    }
}