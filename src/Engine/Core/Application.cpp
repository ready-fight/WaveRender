#include "Application.hpp"

#include "Engine/Core/Log.hpp"

namespace Wave
{
    Application::Application()
    {
        Log::Info("WaveRender starting.");
    }

    Application::~Application()
    {
        Log::Info("WaveRender shutting down.");
    }

    int Application::Run()
    {
        Log::Info("Engine skeleton initialized.");
        Log::Info("Renderer backend not initialized yet.");

        return 0;
    }
}