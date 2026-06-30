find_program(DXC_EXECUTABLE
    NAMES dxc.exe dxc
)

if(NOT DXC_EXECUTABLE)
    message(FATAL_ERROR "DXC compiler not found. Open the project from Visual Studio Developer PowerShell or add dxc.exe to PATH.")
endif()

message(STATUS "Using DXC: ${DXC_EXECUTABLE}")

set(WAVE_SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")

file(MAKE_DIRECTORY "${WAVE_SHADER_OUTPUT_DIR}")

function(wave_add_hlsl_shader target_name source_file entry_point shader_profile output_name)
    set(output_file "${WAVE_SHADER_OUTPUT_DIR}/${output_name}.cso")

    set(debug_args
        "$<$<CONFIG:Debug>:-Zi>"
        "$<$<CONFIG:Debug>:-Qembed_debug>"
        "$<$<CONFIG:Debug>:-Od>"
        "$<$<CONFIG:Debug>:-DWR_SHADER_DEBUG=1>"
    )

    set(release_args
        "$<$<CONFIG:Release>:-O3>"
        "$<$<CONFIG:Release>:-DWR_SHADER_RELEASE=1>"
    )

    add_custom_command(
        OUTPUT "${output_file}"
        COMMAND "${DXC_EXECUTABLE}"
            -T "${shader_profile}"
            -E "${entry_point}"
            -HV 2021
            -WX
            -Fo "${output_file}"
            ${debug_args}
            ${release_args}
            "${source_file}"
        DEPENDS "${source_file}"
        COMMENT "Compiling HLSL ${source_file} -> ${output_file}"
        VERBATIM
        COMMAND_EXPAND_LISTS
    )

    add_custom_target(${target_name}
        DEPENDS "${output_file}"
    )

    set_property(TARGET ${target_name} PROPERTY FOLDER "Shaders")
endfunction()