#pragma once

namespace Wave
{
    class Renderer
    {
    public:
        Renderer() = default;
        virtual ~Renderer() = default;

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
    };
}