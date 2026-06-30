#pragma once

#include "Engine/Core/Types.hpp"
#include "Engine/Renderer/D3D12/D3D12Common.hpp"

#include <limits>

namespace Wave
{
    struct D3D12DescriptorAllocation
    {
        D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle = {};
        D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle = {};
        u32 Index = std::numeric_limits<u32>::max();

        bool IsValid() const
        {
            return Index != std::numeric_limits<u32>::max();
        }
    };

    class D3D12DescriptorAllocator final
    {
      public:
        D3D12DescriptorAllocator() = default;

        D3D12DescriptorAllocator(const D3D12DescriptorAllocator &) = delete;
        D3D12DescriptorAllocator &operator=(const D3D12DescriptorAllocator &) = delete;

        void Initialize(ID3D12Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 capacity,
                        D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

        D3D12DescriptorAllocation Allocate();

        ID3D12DescriptorHeap *GetHeap() const;
        u32 GetDescriptorSize() const;
        u32 GetCapacity() const;
        u32 GetAllocatedCount() const;

      private:
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_Heap;
        D3D12_DESCRIPTOR_HEAP_TYPE m_Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        D3D12_DESCRIPTOR_HEAP_FLAGS m_Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        u32 m_Capacity = 0;
        u32 m_AllocatedCount = 0;
        u32 m_DescriptorSize = 0;
    };
} // namespace Wave