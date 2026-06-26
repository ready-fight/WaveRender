#pragma once

#include "Engine/Platform/Window.hpp"
#include "Engine/Renderer/RenderTypes.hpp"

#include <memory>

namespace Wave
{
    class Renderer
    {
    public:
        virtual ~Renderer() = default;

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        static std::unique_ptr<Renderer> Create(Window& window, const RenderSettings& settings);

    protected:
        Renderer() = default;
    };
}