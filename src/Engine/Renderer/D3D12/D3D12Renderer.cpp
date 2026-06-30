#include "D3D12Renderer.hpp"

#include "Engine/Core/Log.hpp"

namespace Wave
{
    D3D12Renderer::D3D12Renderer(Window& window, const RenderSettings& settings)
        : m_Window(window)
        , m_Settings(settings)
    {
        CreateFactory();
        CreateDevice();
        CreateCommandQueue();
        CreateSwapChain();
        CreateRenderTargets();
        CreateCommandObjects();
        CreateSyncObjects();

        Log::Info("D3D12 renderer initialized.");
    }

    D3D12Renderer::~D3D12Renderer()
    {
        WaitForGpu();

        if (m_FenceEvent)
        {
            CloseHandle(m_FenceEvent);
            m_FenceEvent = nullptr;
        }

        Log::Info("D3D12 renderer destroyed.");
    }

    void D3D12Renderer::BeginFrame()
    {
        m_GraphicsContext.Reset(m_FrameIndex);

        TransitionCurrentBackBuffer(D3D12_RESOURCE_STATE_RENDER_TARGET);

        const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetCurrentBackBufferRtv();
        ID3D12GraphicsCommandList* commandList = m_GraphicsContext.GetCommandList();

        commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

        const float clearColor[] = {0.04f, 0.06f, 0.10f, 1.0f};
        commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    }

    void D3D12Renderer::EndFrame()
    {
        TransitionCurrentBackBuffer(D3D12_RESOURCE_STATE_PRESENT);

        m_GraphicsContext.Close();

        ID3D12CommandList* commandLists[] = {m_GraphicsContext.GetCommandList()};
        m_GraphicsQueue->ExecuteCommandLists(1, commandLists);

        ThrowIfFailed(m_SwapChain->Present(1, 0), "Failed to present swap chain.");

        MoveToNextFrame();
    }

    void D3D12Renderer::CreateFactory()
    {
        UINT factoryFlags = 0;

#if defined(_DEBUG)
        if (m_Settings.EnableValidation)
        {
            Microsoft::WRL::ComPtr<ID3D12Debug> debugController;

            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
                factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
                Log::Info("D3D12 debug layer enabled.");
            }
        }
#endif

        HRESULT result = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_Factory));

        if (FAILED(result) && factoryFlags != 0)
        {
            Log::Warn("Debug DXGI factory failed. Retrying without debug flags.");
            ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&m_Factory)), "Failed to create DXGI factory.");
            return;
        }

        ThrowIfFailed(result, "Failed to create DXGI factory.");
    }

    void D3D12Renderer::CreateDevice()
    {
        Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter = SelectHardwareAdapter();

        ThrowIfFailed(
            D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device)),
            "Failed to create D3D12 device.");

        m_Device->SetName(L"WaveRender D3D12 Device");
    }

    void D3D12Renderer::CreateCommandQueue()
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;

        ThrowIfFailed(
            m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_GraphicsQueue)),
            "Failed to create graphics command queue.");

        m_GraphicsQueue->SetName(L"WaveRender Graphics Queue");
    }

    void D3D12Renderer::CreateSwapChain()
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = m_Settings.BackBufferWidth;
        swapChainDesc.Height = m_Settings.BackBufferHeight;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = SwapChainBufferCount;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        swapChainDesc.Flags = 0;

        Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;

        ThrowIfFailed(
            m_Factory->CreateSwapChainForHwnd(
                m_GraphicsQueue.Get(),
                static_cast<HWND>(m_Window.GetNativeHandle()),
                &swapChainDesc,
                nullptr,
                nullptr,
                &swapChain),
            "Failed to create swap chain.");

        ThrowIfFailed(
            m_Factory->MakeWindowAssociation(static_cast<HWND>(m_Window.GetNativeHandle()), DXGI_MWA_NO_ALT_ENTER),
            "Failed to disable DXGI Alt+Enter handling.");

        ThrowIfFailed(swapChain.As(&m_SwapChain), "Failed to query IDXGISwapChain3.");

        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
    }

    void D3D12Renderer::CreateRenderTargets()
    {
        m_RtvAllocator.Initialize(
            m_Device.Get(),
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            SwapChainBufferCount);

        for (u32 i = 0; i < SwapChainBufferCount; ++i)
        {
            Microsoft::WRL::ComPtr<ID3D12Resource> backBuffer;

            ThrowIfFailed(
                m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)),
                "Failed to get swap chain back buffer.");

            GpuResourceDesc resourceDesc = {};
            resourceDesc.Usage = ResourceUsage::BackBuffer;
            resourceDesc.DebugName = L"WaveRender Back Buffer";

            m_BackBuffers[i].Attach(
                backBuffer,
                D3D12_RESOURCE_STATE_PRESENT,
                resourceDesc);

            m_BackBufferRtvs[i] = m_RtvAllocator.Allocate();

            m_Device->CreateRenderTargetView(
                m_BackBuffers[i].Get(),
                nullptr,
                m_BackBufferRtvs[i].CpuHandle);
        }
    }

    void D3D12Renderer::CreateCommandObjects()
    {
        m_GraphicsContext.Initialize(
            m_Device.Get(),
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            SwapChainBufferCount,
            L"WaveRender Graphics Command List");
    }

    void D3D12Renderer::CreateSyncObjects()
    {
        ThrowIfFailed(
            m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)),
            "Failed to create fence.");

        m_FenceValues[m_FrameIndex] = 1;

        m_FenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

        if (!m_FenceEvent)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), "Failed to create fence event.");
        }
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter1> D3D12Renderer::SelectHardwareAdapter() const
    {
        Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

        for (u32 adapterIndex = 0;
             m_Factory->EnumAdapterByGpuPreference(
                 adapterIndex,
                 DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                 IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND;
             ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc = {};
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
            {
                return adapter;
            }
        }

        throw std::runtime_error("No suitable D3D12 hardware adapter found.");
    }

    D3D12_CPU_DESCRIPTOR_HANDLE D3D12Renderer::GetCurrentBackBufferRtv() const
    {
        return m_BackBufferRtvs[m_FrameIndex].CpuHandle;
    }

    void D3D12Renderer::TransitionCurrentBackBuffer(D3D12_RESOURCE_STATES after)
    {
        D3D12GpuResource& backBuffer = m_BackBuffers[m_FrameIndex];
        const D3D12_RESOURCE_STATES before = backBuffer.GetState();

        m_GraphicsContext.TransitionResource(backBuffer.Get(), before, after);
        backBuffer.SetState(after);
    }

    void D3D12Renderer::MoveToNextFrame()
    {
        const u64 currentFenceValue = m_FenceValues[m_FrameIndex];

        ThrowIfFailed(
            m_GraphicsQueue->Signal(m_Fence.Get(), currentFenceValue),
            "Failed to signal fence.");

        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

        if (m_Fence->GetCompletedValue() < m_FenceValues[m_FrameIndex])
        {
            ThrowIfFailed(
                m_Fence->SetEventOnCompletion(m_FenceValues[m_FrameIndex], m_FenceEvent),
                "Failed to set fence completion event.");

            WaitForSingleObject(m_FenceEvent, INFINITE);
        }

        m_FenceValues[m_FrameIndex] = currentFenceValue + 1;
    }

    void D3D12Renderer::WaitForGpu()
    {
        if (!m_GraphicsQueue || !m_Fence || !m_FenceEvent)
        {
            return;
        }

        const u64 fenceValue = m_FenceValues[m_FrameIndex];

        ThrowIfFailed(
            m_GraphicsQueue->Signal(m_Fence.Get(), fenceValue),
            "Failed to signal fence during GPU wait.");

        ThrowIfFailed(
            m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent),
            "Failed to set fence event during GPU wait.");

        WaitForSingleObject(m_FenceEvent, INFINITE);

        ++m_FenceValues[m_FrameIndex];
    }
}