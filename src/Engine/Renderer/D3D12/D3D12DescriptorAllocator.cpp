#include "D3D12DescriptorAllocator.hpp"

#include <stdexcept>

namespace Wave
{
    void D3D12DescriptorAllocator::Initialize(
        ID3D12Device* device,
        D3D12_DESCRIPTOR_HEAP_TYPE type,
        u32 capacity,
        D3D12_DESCRIPTOR_HEAP_FLAGS flags)
    {
        m_Type = type;
        m_Flags = flags;
        m_Capacity = capacity;
        m_AllocatedCount = 0;
        m_DescriptorSize = device->GetDescriptorHandleIncrementSize(type);

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.Type = type;
        heapDesc.NumDescriptors = capacity;
        heapDesc.Flags = flags;
        heapDesc.NodeMask = 0;

        ThrowIfFailed(
            device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_Heap)),
            "Failed to create descriptor heap.");
    }

    D3D12DescriptorAllocation D3D12DescriptorAllocator::Allocate()
    {
        if (m_AllocatedCount >= m_Capacity)
        {
            throw std::runtime_error("D3D12 descriptor heap exhausted.");
        }

        D3D12DescriptorAllocation allocation = {};
        allocation.Index = m_AllocatedCount;

        allocation.CpuHandle = m_Heap->GetCPUDescriptorHandleForHeapStart();
        allocation.CpuHandle.ptr += static_cast<SIZE_T>(allocation.Index) * static_cast<SIZE_T>(m_DescriptorSize);

        if (m_Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
        {
            allocation.GpuHandle = m_Heap->GetGPUDescriptorHandleForHeapStart();
            allocation.GpuHandle.ptr += static_cast<UINT64>(allocation.Index) * static_cast<UINT64>(m_DescriptorSize);
        }

        ++m_AllocatedCount;

        return allocation;
    }

    ID3D12DescriptorHeap* D3D12DescriptorAllocator::GetHeap() const
    {
        return m_Heap.Get();
    }

    u32 D3D12DescriptorAllocator::GetDescriptorSize() const
    {
        return m_DescriptorSize;
    }

    u32 D3D12DescriptorAllocator::GetCapacity() const
    {
        return m_Capacity;
    }

    u32 D3D12DescriptorAllocator::GetAllocatedCount() const
    {
        return m_AllocatedCount;
    }
}