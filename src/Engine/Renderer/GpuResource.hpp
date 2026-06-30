#pragma once

#include "Engine/Core/Types.hpp"

namespace Wave
{
    enum class ResourceUsage : u8
    {
        Unknown,
        BackBuffer,
        VertexBuffer,
        IndexBuffer,
        ConstantBuffer,
        StructuredBuffer,
        Texture2D,
        RenderTarget,
        DepthStencil
    };

    struct GpuResourceDesc
    {
        ResourceUsage Usage = ResourceUsage::Unknown;
        u64 SizeInBytes = 0;
        const wchar_t* DebugName = L"Unnamed GPU Resource";
    };
}