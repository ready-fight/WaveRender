#pragma once

#include "Engine/Platform/Window.hpp"

namespace Wave
{
    class Win32Window final : public Window
    {
    public:
        explicit Win32Window(const WindowDesc& desc);
        ~Win32Window() override;

        void PollEvents() override;
        bool ShouldClose() const override;

        void* GetNativeHandle() const override;
        u32 GetWidth() const override;
        u32 GetHeight() const override;

    private:
        void CreateNativeWindow(const WindowDesc& desc);
        void DestroyNativeWindow();

    private:
        void* m_Handle = nullptr;
        u32 m_Width = 0;
        u32 m_Height = 0;
        bool m_ShouldClose = false;
    };
}