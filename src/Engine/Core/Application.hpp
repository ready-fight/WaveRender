#pragma once

#include "Engine/Platform/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"

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
        std::unique_ptr<Renderer> m_Renderer;
        bool m_IsRunning = true;
    };
}