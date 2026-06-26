#include "Win32Window.hpp"

#include "Engine/Core/Log.hpp"

#include <Windows.h>

namespace
{
    constexpr const wchar_t* WindowClassName = L"WaveRenderWindowClass";

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
    {
        auto* window = reinterpret_cast<Wave::Win32Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        switch (message)
        {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
            // Resize handling will be wired into the renderer later.
            return 0;

        default:
            return DefWindowProcW(hwnd, message, wparam, lparam);
        }
    }
}

namespace Wave
{
    Win32Window::Win32Window(const WindowDesc& desc)
    {
        CreateNativeWindow(desc);
    }

    Win32Window::~Win32Window()
    {
        DestroyNativeWindow();
    }

    void Win32Window::PollEvents()
    {
        MSG message = {};

        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                m_ShouldClose = true;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    bool Win32Window::ShouldClose() const
    {
        return m_ShouldClose;
    }

    void* Win32Window::GetNativeHandle() const
    {
        return m_Handle;
    }

    u32 Win32Window::GetWidth() const
    {
        return m_Width;
    }

    u32 Win32Window::GetHeight() const
    {
        return m_Height;
    }

    void Win32Window::CreateNativeWindow(const WindowDesc& desc)
    {
        m_Width = desc.Width;
        m_Height = desc.Height;

        HINSTANCE instance = GetModuleHandleW(nullptr);

        WNDCLASSEXW windowClass = {};
        windowClass.cbSize = sizeof(WNDCLASSEXW);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = instance;
        windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        windowClass.lpszClassName = WindowClassName;

        RegisterClassExW(&windowClass);

        RECT rect = {};
        rect.left = 0;
        rect.top = 0;
        rect.right = static_cast<LONG>(m_Width);
        rect.bottom = static_cast<LONG>(m_Height);

        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        HWND hwnd = CreateWindowExW(
            0,
            WindowClassName,
            desc.Title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            rect.right - rect.left,
            rect.bottom - rect.top,
            nullptr,
            nullptr,
            instance,
            nullptr);

        if (!hwnd)
        {
            Log::Error("Failed to create Win32 window.");
            m_ShouldClose = true;
            return;
        }

        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        m_Handle = hwnd;

        Log::Info("Win32 window created.");
    }

    void Win32Window::DestroyNativeWindow()
    {
        if (m_Handle)
        {
            DestroyWindow(static_cast<HWND>(m_Handle));
            m_Handle = nullptr;
        }

        UnregisterClassW(WindowClassName, GetModuleHandleW(nullptr));
    }
}