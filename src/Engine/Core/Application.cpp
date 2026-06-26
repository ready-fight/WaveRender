#include "Application.hpp"

#include "Engine/Core/Log.hpp"

namespace Wave
{
    Application::Application()
    {
        Log::Info("WaveRender starting.");

        WindowDesc windowDesc = {};
        windowDesc.Width = 1280;
        windowDesc.Height = 720;
        windowDesc.Title = L"WaveRender";

        m_Window = Window::Create(windowDesc);
    }

    Application::~Application()
    {
        Log::Info("WaveRender shutting down.");
    }

    int Application::Run()
    {
        Log::Info("Main loop started.");

        while (m_IsRunning)
        {
            m_Window->PollEvents();

            if (m_Window->ShouldClose())
            {
                m_IsRunning = false;
            }
        }

        Log::Info("Main loop ended.");

        return 0;
    }
}