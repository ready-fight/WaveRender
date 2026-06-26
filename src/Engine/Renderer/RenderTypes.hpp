#pragma once

#include "Engine/Core/Types.hpp"

namespace Wave
{
    enum class GraphicsBackend : u8
    {
        D3D12
    };

    struct RenderSettings
    {
        u32 BackBufferWidth = 1280;
        u32 BackBufferHeight = 720;
        bool EnableValidation = true;
    };
}