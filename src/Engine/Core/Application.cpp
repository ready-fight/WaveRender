#include "Application.hpp"

#include "Engine/Core/Log.hpp"
#include "Engine/Renderer/RenderTypes.hpp"

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

        RenderSettings renderSettings = {};
        renderSettings.BackBufferWidth = windowDesc.Width;
        renderSettings.BackBufferHeight = windowDesc.Height;
        renderSettings.EnableValidation = true;

        m_Renderer = Renderer::Create(*m_Window, renderSettings);
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
                break;
            }

            m_Renderer->BeginFrame();
            m_Renderer->EndFrame();
        }

        Log::Info("Main loop ended.");

        return 0;
    }
}