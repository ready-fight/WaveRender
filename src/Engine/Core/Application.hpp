#pragma once

#include "Engine/Platform/Window.hpp"

#include <memory>

namespace Wave
{
    class Application
    {
    public:
        Application();
        ~Application();

        int Run();

    private:
        std::unique_ptr<Window> m_Window;
        bool m_IsRunning = true;
    };
}