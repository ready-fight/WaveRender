#include "Window.hpp"

#include "Engine/Platform/Win32/Win32Window.hpp"

#include <memory>

namespace Wave
{
    std::unique_ptr<Window> Window::Create(const WindowDesc& desc)
    {
        return std::make_unique<Win32Window>(desc);
    }
}