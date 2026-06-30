#pragma once

#include "Engine/Renderer/D3D12/D3D12CommandContext.hpp"
#include "Engine/Renderer/D3D12/D3D12DescriptorAllocator.hpp"
#include "Engine/Renderer/D3D12/D3D12GpuResource.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include <array>

namespace Wave
{
    class D3D12Renderer final : public Renderer
    {
    public:
        D3D12Renderer(Window& window, const RenderSettings& settings);
        ~D3D12Renderer() override;

        void BeginFrame() override;
        void EndFrame() override;

    private:
        void CreateFactory();
        void CreateDevice();
        void CreateCommandQueue();
        void CreateSwapChain();
        void CreateRenderTargets();
        void CreateCommandObjects();
        void CreateSyncObjects();

        Microsoft::WRL::ComPtr<IDXGIAdapter1> SelectHardwareAdapter() const;

        D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferRtv() const;

        void TransitionCurrentBackBuffer(D3D12_RESOURCE_STATES after);
        void MoveToNextFrame();
        void WaitForGpu();

    private:
        Window& m_Window;
        RenderSettings m_Settings;

        Microsoft::WRL::ComPtr<IDXGIFactory6> m_Factory;
        Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_GraphicsQueue;
        Microsoft::WRL::ComPtr<IDXGISwapChain3> m_SwapChain;

        D3D12DescriptorAllocator m_RtvAllocator;
        std::array<D3D12DescriptorAllocation, SwapChainBufferCount> m_BackBufferRtvs = {};
        std::array<D3D12GpuResource, SwapChainBufferCount> m_BackBuffers = {};

        D3D12CommandContext m_GraphicsContext;

        Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
        std::array<u64, SwapChainBufferCount> m_FenceValues = {};
        HANDLE m_FenceEvent = nullptr;

        u32 m_FrameIndex = 0;
    };
}