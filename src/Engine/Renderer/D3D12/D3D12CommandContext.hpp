#pragma once

#include "Engine/Core/Types.hpp"
#include "Engine/Renderer/D3D12/D3D12Common.hpp"

#include <vector>

namespace Wave
{
    class D3D12CommandContext final
    {
    public:
        D3D12CommandContext() = default;

        D3D12CommandContext(const D3D12CommandContext&) = delete;
        D3D12CommandContext& operator=(const D3D12CommandContext&) = delete;

        void Initialize(
            ID3D12Device* device,
            D3D12_COMMAND_LIST_TYPE type,
            u32 frameCount,
            const wchar_t* debugName);

        void Reset(u32 frameIndex, ID3D12PipelineState* initialPipelineState = nullptr);
        void Close();

        void TransitionResource(
            ID3D12Resource* resource,
            D3D12_RESOURCE_STATES before,
            D3D12_RESOURCE_STATES after);

        void CopyBuffer(
            ID3D12Resource* destination,
            ID3D12Resource* source,
            u64 sizeInBytes);

        ID3D12GraphicsCommandList* GetCommandList() const;

    private:
        D3D12_COMMAND_LIST_TYPE m_Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_CommandAllocators;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
    };
}