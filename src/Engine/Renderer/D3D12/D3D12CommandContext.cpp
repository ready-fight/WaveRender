#include "D3D12CommandContext.hpp"

#include <stdexcept>

namespace Wave
{
    void D3D12CommandContext::Initialize(
        ID3D12Device* device,
        D3D12_COMMAND_LIST_TYPE type,
        u32 frameCount,
        const wchar_t* debugName)
    {
        m_Type = type;
        m_CommandAllocators.resize(frameCount);

        for (u32 i = 0; i < frameCount; ++i)
        {
            ThrowIfFailed(
                device->CreateCommandAllocator(type, IID_PPV_ARGS(&m_CommandAllocators[i])),
                "Failed to create command allocator.");
        }

        ThrowIfFailed(
            device->CreateCommandList(
                0,
                type,
                m_CommandAllocators[0].Get(),
                nullptr,
                IID_PPV_ARGS(&m_CommandList)),
            "Failed to create command list.");

        if (debugName)
        {
            m_CommandList->SetName(debugName);
        }

        ThrowIfFailed(m_CommandList->Close(), "Failed to close initial command list.");
    }

    void D3D12CommandContext::Reset(u32 frameIndex, ID3D12PipelineState* initialPipelineState)
    {
        if (frameIndex >= m_CommandAllocators.size())
        {
            throw std::runtime_error("Invalid frame index for command context reset.");
        }

        ThrowIfFailed(
            m_CommandAllocators[frameIndex]->Reset(),
            "Failed to reset command allocator.");

        ThrowIfFailed(
            m_CommandList->Reset(m_CommandAllocators[frameIndex].Get(), initialPipelineState),
            "Failed to reset command list.");
    }

    void D3D12CommandContext::Close()
    {
        ThrowIfFailed(m_CommandList->Close(), "Failed to close command list.");
    }

    void D3D12CommandContext::TransitionResource(
        ID3D12Resource* resource,
        D3D12_RESOURCE_STATES before,
        D3D12_RESOURCE_STATES after)
    {
        if (before == after)
        {
            return;
        }

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = resource;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = before;
        barrier.Transition.StateAfter = after;

        m_CommandList->ResourceBarrier(1, &barrier);
    }

    void D3D12CommandContext::CopyBuffer(
        ID3D12Resource* destination,
        ID3D12Resource* source,
        u64 sizeInBytes)
    {
        m_CommandList->CopyBufferRegion(
            destination,
            0,
            source,
            0,
            sizeInBytes);
    }

    ID3D12GraphicsCommandList* D3D12CommandContext::GetCommandList() const
    {
        return m_CommandList.Get();
    }
}