#include "D3D12Buffer.hpp"

#include <cstring>
#include <stdexcept>

namespace Wave
{
    void D3D12Buffer::Create(
        ID3D12Device* device,
        const BufferDesc& desc,
        D3D12_HEAP_TYPE heapType,
        D3D12_RESOURCE_STATES initialState)
    {
        m_Desc = desc;

        D3D12_HEAP_PROPERTIES heapProperties = {};
        heapProperties.Type = heapType;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProperties.CreationNodeMask = 1;
        heapProperties.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Alignment = 0;
        resourceDesc.Width = desc.SizeInBytes;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        Microsoft::WRL::ComPtr<ID3D12Resource> resource;

        ThrowIfFailed(
            device->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                initialState,
                nullptr,
                IID_PPV_ARGS(&resource)),
            "Failed to create D3D12 buffer.");

        GpuResourceDesc gpuDesc = {};
        gpuDesc.Usage = ResourceUsage::StructuredBuffer;
        gpuDesc.SizeInBytes = desc.SizeInBytes;
        gpuDesc.DebugName = desc.DebugName;

        m_Resource.Attach(resource, initialState, gpuDesc);
    }

    void D3D12Buffer::UploadData(const void* data, u64 sizeInBytes)
    {
        if (sizeInBytes > m_Desc.SizeInBytes)
        {
            throw std::runtime_error("Upload data is larger than destination buffer.");
        }

        D3D12_RANGE readRange = {};
        readRange.Begin = 0;
        readRange.End = 0;

        void* mappedMemory = nullptr;

        ThrowIfFailed(
            GetResource()->Map(0, &readRange, &mappedMemory),
            "Failed to map upload buffer.");

        std::memcpy(mappedMemory, data, static_cast<size_t>(sizeInBytes));

        GetResource()->Unmap(0, nullptr);
    }

    ID3D12Resource* D3D12Buffer::GetResource() const
    {
        return m_Resource.Get();
    }

    D3D12_GPU_VIRTUAL_ADDRESS D3D12Buffer::GetGpuVirtualAddress() const
    {
        return GetResource()->GetGPUVirtualAddress();
    }

    D3D12_RESOURCE_STATES D3D12Buffer::GetState() const
    {
        return m_Resource.GetState();
    }

    void D3D12Buffer::SetState(D3D12_RESOURCE_STATES state)
    {
        m_Resource.SetState(state);
    }

    D3D12_VERTEX_BUFFER_VIEW D3D12Buffer::GetVertexBufferView() const
    {
        D3D12_VERTEX_BUFFER_VIEW view = {};
        view.BufferLocation = GetGpuVirtualAddress();
        view.SizeInBytes = static_cast<UINT>(m_Desc.SizeInBytes);
        view.StrideInBytes = m_Desc.StrideInBytes;
        return view;
    }

    const BufferDesc& D3D12Buffer::GetDesc() const
    {
        return m_Desc;
    }
}