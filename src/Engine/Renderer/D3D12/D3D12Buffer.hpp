#pragma once

#include "Engine/Renderer/Buffer.hpp"
#include "Engine/Renderer/D3D12/D3D12GpuResource.hpp"

namespace Wave
{
    class D3D12Buffer final
    {
    public:
        D3D12Buffer() = default;

        D3D12Buffer(const D3D12Buffer&) = delete;
        D3D12Buffer& operator=(const D3D12Buffer&) = delete;

        D3D12Buffer(D3D12Buffer&&) = default;
        D3D12Buffer& operator=(D3D12Buffer&&) = default;

        void Create(
            ID3D12Device* device,
            const BufferDesc& desc,
            D3D12_HEAP_TYPE heapType,
            D3D12_RESOURCE_STATES initialState);

        void UploadData(const void* data, u64 sizeInBytes);

        ID3D12Resource* GetResource() const;
        D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const;

        D3D12_RESOURCE_STATES GetState() const;
        void SetState(D3D12_RESOURCE_STATES state);

        D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;

        const BufferDesc& GetDesc() const;

    private:
        D3D12GpuResource m_Resource;
        BufferDesc m_Desc = {};
    };
}