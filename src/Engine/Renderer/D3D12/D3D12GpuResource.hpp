#pragma once

#include "Engine/Renderer/D3D12/D3D12Common.hpp"
#include "Engine/Renderer/GpuResource.hpp"

namespace Wave
{
    class D3D12GpuResource final
    {
    public:
        D3D12GpuResource() = default;

        D3D12GpuResource(const D3D12GpuResource&) = delete;
        D3D12GpuResource& operator=(const D3D12GpuResource&) = delete;

        D3D12GpuResource(D3D12GpuResource&&) = default;
        D3D12GpuResource& operator=(D3D12GpuResource&&) = default;

        void Attach(
            Microsoft::WRL::ComPtr<ID3D12Resource> resource,
            D3D12_RESOURCE_STATES initialState,
            GpuResourceDesc desc);

        ID3D12Resource* Get() const;
        D3D12_RESOURCE_STATES GetState() const;
        void SetState(D3D12_RESOURCE_STATES state);

        const GpuResourceDesc& GetDesc() const;

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;
        D3D12_RESOURCE_STATES m_State = D3D12_RESOURCE_STATE_COMMON;
        GpuResourceDesc m_Desc = {};
    };
}