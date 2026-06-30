#pragma once

#include "Engine/Core/Types.hpp"

#include <filesystem>
#include <vector>

namespace Wave
{
    class FileSystem final
    {
    public:
        static std::vector<u8> ReadBinaryFile(const std::filesystem::path& path);
    };
}