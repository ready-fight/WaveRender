cmake_minimum_required(VERSION 3.24)

project(WaveRender LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(WaveRender
    src/WaveRender/main.cpp

    src/Engine/Core/Application.cpp
    src/Engine/Core/Application.hpp
    src/Engine/Core/Log.cpp
    src/Engine/Core/Log.hpp
    src/Engine/Core/Types.hpp

    src/Engine/Renderer/Renderer.hpp
    src/Engine/Renderer/RenderTypes.hpp

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
)

target_link_libraries(WaveRender PRIVATE
    user32
)

if(MSVC)
    target_compile_options(WaveRender PRIVATE
        /W4
        /permissive-
        /EHsc
        /MP
    )
endif()