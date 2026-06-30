#include "D3D12GpuResource.hpp"

namespace Wave
{
    void D3D12GpuResource::Attach(
        Microsoft::WRL::ComPtr<ID3D12Resource> resource,
        D3D12_RESOURCE_STATES initialState,
        GpuResourceDesc desc)
    {
        m_Resource = resource;
        m_State = initialState;
        m_Desc = desc;

        if (m_Resource && desc.DebugName)
        {
            m_Resource->SetName(desc.DebugName);
        }
    }

    ID3D12Resource* D3D12GpuResource::Get() const
    {
        return m_Resource.Get();
    }

    D3D12_RESOURCE_STATES D3D12GpuResource::GetState() const
    {
        return m_State;
    }

    void D3D12GpuResource::SetState(D3D12_RESOURCE_STATES state)
    {
        m_State = state;
    }

    const GpuResourceDesc& D3D12GpuResource::GetDesc() const
    {
        return m_Desc;
    }
}