#pragma once

#include <sstream>
#include <stdexcept>

#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace Wave
{
    inline void ThrowIfFailed(HRESULT result, const char* message)
    {
        if (FAILED(result))
        {
            std::ostringstream stream;
            stream << message << " HRESULT=0x" << std::hex << static_cast<unsigned long>(result);
            throw std::runtime_error(stream.str());
        }
    }
}