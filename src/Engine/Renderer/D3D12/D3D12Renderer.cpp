#include "D3D12Renderer.hpp"

#include "Engine/Core/FileSystem.hpp"
#include "Engine/Core/Log.hpp"

#include <filesystem>
#include <string>

#ifndef WAVE_SHADER_DIR
#define WAVE_SHADER_DIR "shaders"
#endif

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
        CreatePipelineObjects();
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
        m_GraphicsContext.Reset(m_FrameIndex, m_PipelineState.Get());

        TransitionCurrentBackBuffer(D3D12_RESOURCE_STATE_RENDER_TARGET);

        const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetCurrentBackBufferRtv();
        ID3D12GraphicsCommandList* commandList = m_GraphicsContext.GetCommandList();

        commandList->RSSetViewports(1, &m_Viewport);
        commandList->RSSetScissorRects(1, &m_ScissorRect);

        commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

        const float clearColor[] = {0.04f, 0.06f, 0.10f, 1.0f};
        commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

        commandList->SetGraphicsRootSignature(m_RootSignature.Get());
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->DrawInstanced(3, 1, 0, 0);
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

    void D3D12Renderer::CreatePipelineObjects()
    {
        const std::filesystem::path shaderDir = WAVE_SHADER_DIR;

        const std::vector<u8> vertexShader = FileSystem::ReadBinaryFile(shaderDir / "TriangleVS.cso");
        const std::vector<u8> pixelShader = FileSystem::ReadBinaryFile(shaderDir / "TrianglePS.cso");

        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        rootSignatureDesc.NumParameters = 0;
        rootSignatureDesc.pParameters = nullptr;
        rootSignatureDesc.NumStaticSamplers = 0;
        rootSignatureDesc.pStaticSamplers = nullptr;
        rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

        HRESULT serializeResult = D3D12SerializeRootSignature(
            &rootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            &signatureBlob,
            &errorBlob);

        if (FAILED(serializeResult))
        {
            std::string errorMessage = "Failed to serialize root signature.";

            if (errorBlob)
            {
                errorMessage += " ";
                errorMessage += static_cast<const char*>(errorBlob->GetBufferPointer());
            }

            ThrowIfFailed(serializeResult, errorMessage.c_str());
        }

        ThrowIfFailed(
            m_Device->CreateRootSignature(
                0,
                signatureBlob->GetBufferPointer(),
                signatureBlob->GetBufferSize(),
                IID_PPV_ARGS(&m_RootSignature)),
            "Failed to create root signature.");

        m_RootSignature->SetName(L"WaveRender Empty Root Signature");

        D3D12_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
        rasterizerDesc.FrontCounterClockwise = FALSE;
        rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rasterizerDesc.DepthClipEnable = TRUE;
        rasterizerDesc.MultisampleEnable = FALSE;
        rasterizerDesc.AntialiasedLineEnable = FALSE;
        rasterizerDesc.ForcedSampleCount = 0;
        rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        D3D12_BLEND_DESC blendDesc = {};
        blendDesc.AlphaToCoverageEnable = FALSE;
        blendDesc.IndependentBlendEnable = FALSE;
        blendDesc.RenderTarget[0].BlendEnable = FALSE;
        blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable = FALSE;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        depthStencilDesc.StencilEnable = FALSE;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = {};
        pipelineDesc.pRootSignature = m_RootSignature.Get();
        pipelineDesc.VS = {vertexShader.data(), vertexShader.size()};
        pipelineDesc.PS = {pixelShader.data(), pixelShader.size()};
        pipelineDesc.BlendState = blendDesc;
        pipelineDesc.SampleMask = UINT_MAX;
        pipelineDesc.RasterizerState = rasterizerDesc;
        pipelineDesc.DepthStencilState = depthStencilDesc;
        pipelineDesc.InputLayout = {};
        pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pipelineDesc.NumRenderTargets = 1;
        pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        pipelineDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
        pipelineDesc.SampleDesc.Count = 1;
        pipelineDesc.SampleDesc.Quality = 0;

        ThrowIfFailed(
            m_Device->CreateGraphicsPipelineState(&pipelineDesc, IID_PPV_ARGS(&m_PipelineState)),
            "Failed to create graphics pipeline state.");

        m_PipelineState->SetName(L"WaveRender Triangle PSO");

        m_Viewport.TopLeftX = 0.0f;
        m_Viewport.TopLeftY = 0.0f;
        m_Viewport.Width = static_cast<float>(m_Settings.BackBufferWidth);
        m_Viewport.Height = static_cast<float>(m_Settings.BackBufferHeight);
        m_Viewport.MinDepth = 0.0f;
        m_Viewport.MaxDepth = 1.0f;

        m_ScissorRect.left = 0;
        m_ScissorRect.top = 0;
        m_ScissorRect.right = static_cast<LONG>(m_Settings.BackBufferWidth);
        m_ScissorRect.bottom = static_cast<LONG>(m_Settings.BackBufferHeight);

        Log::Info("Triangle graphics pipeline created.");
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