#pragma once

#include "Engine/Core/Types.hpp"

namespace Wave
{
    enum class BufferUsage : u8
    {
        Unknown,
        Vertex,
        Index,
        Constant,
        Structured,
        Upload,
        Readback
    };

    struct BufferDesc
    {
        BufferUsage Usage = BufferUsage::Unknown;
        u64 SizeInBytes = 0;
        u32 StrideInBytes = 0;
        const wchar_t* DebugName = L"Unnamed Buffer";
    };
}