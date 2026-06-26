#pragma once

#include "Engine/Core/Types.hpp"

#include <memory>

namespace Wave
{
    struct WindowDesc
    {
        u32 Width = 1280;
        u32 Height = 720;
        const wchar_t *Title = L"WaveRender";
    };

    class Window
    {
      public:
        virtual ~Window() = default;

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        virtual void PollEvents() = 0;
        virtual bool ShouldClose() const = 0;

        virtual void *GetNativeHandle() const = 0;
        virtual u32 GetWidth() const = 0;
        virtual u32 GetHeight() const = 0;

        static std::unique_ptr<Window> Create(const WindowDesc &desc);

      protected:
        Window() = default;
    };
} // namespace Wave