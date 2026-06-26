#include "Log.hpp"

#include <iostream>

namespace Wave
{
    void Log::Info(std::string_view message)
    {
        std::cout << "[Info] " << message << '\n';
    }

    void Log::Warn(std::string_view message)
    {
        std::cout << "[Warn] " << message << '\n';
    }

    void Log::Error(std::string_view message)
    {
        std::cerr << "[Error] " << message << '\n';
    }
}