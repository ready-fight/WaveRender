cmake_minimum_required(VERSION 3.24)

project(WaveRender LANGUAGES CXX)
include(cmake/ShaderCompile.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(WaveRender
    src/WaveRender/main.cpp

    src/Engine/Core/Application.cpp
    src/Engine/Core/Application.hpp
    src/Engine/Core/FileSystem.cpp
    src/Engine/Core/FileSystem.hpp
    src/Engine/Core/Log.cpp
    src/Engine/Core/Log.hpp
    src/Engine/Core/Types.hpp

    src/Engine/Renderer/Renderer.cpp
    src/Engine/Renderer/Renderer.hpp
    src/Engine/Renderer/RenderTypes.hpp
    src/Engine/Renderer/GpuResource.hpp
    src/Engine/Renderer/Buffer.hpp
    src/Engine/Renderer/D3D12/D3D12Buffer.cpp
    src/Engine/Renderer/D3D12/D3D12Buffer.hpp
    src/Engine/Renderer/D3D12/D3D12Common.hpp
    src/Engine/Renderer/D3D12/D3D12CommandContext.cpp
    src/Engine/Renderer/D3D12/D3D12CommandContext.hpp
    src/Engine/Renderer/D3D12/D3D12DescriptorAllocator.cpp
    src/Engine/Renderer/D3D12/D3D12DescriptorAllocator.hpp
    src/Engine/Renderer/D3D12/D3D12GpuResource.cpp
    src/Engine/Renderer/D3D12/D3D12GpuResource.hpp
    src/Engine/Renderer/D3D12/D3D12Renderer.cpp
    src/Engine/Renderer/D3D12/D3D12Renderer.hpp

    src/Engine/Platform/Window.cpp
    src/Engine/Platform/Window.hpp
    src/Engine/Platform/Win32/Win32Window.cpp
    src/Engine/Platform/Win32/Win32Window.hpp
)

target_include_directories(WaveRender PRIVATE
    src
)

target_compile_definitions(WaveRender PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    UNICODE
    _UNICODE
    WAVE_SHADER_DIR="${WAVE_SHADER_OUTPUT_DIR}"
)

target_link_libraries(WaveRender PRIVATE
    user32
    d3d12
    dxgi
    dxguid
)

wave_add_hlsl_shader(
    WaveShader_TriangleVS
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/Triangle.hlsl"
    VSMain
    vs_6_0
    TriangleVS
)

wave_add_hlsl_shader(
    WaveShader_TrianglePS
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/Triangle.hlsl"
    PSMain
    ps_6_0
    TrianglePS
)

add_custom_target(WaveShaders
    DEPENDS
        WaveShader_TriangleVS
        WaveShader_TrianglePS
)

add_dependencies(WaveRender WaveShaders)

if(MSVC)
    target_compile_options(WaveRender PRIVATE
        /W4
        /permissive-
        /EHsc
        /MP
    )
endif()