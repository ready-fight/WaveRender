#pragma once

#include "Engine/Core/Types.hpp"

namespace Wave
{
    struct WindowDesc
    {
        u32 Width = 1280;
        u32 Height = 720;
        const wchar_t* Title = L"WaveRender";
    };
}