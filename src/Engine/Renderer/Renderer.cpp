#include "Renderer.hpp"

#include "Engine/Renderer/D3D12/D3D12Renderer.hpp"

namespace Wave
{
    std::unique_ptr<Renderer> Renderer::Create(Window& window, const RenderSettings& settings)
    {
        return std::make_unique<D3D12Renderer>(window, settings);
    }
}