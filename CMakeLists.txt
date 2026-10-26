cmake_minimum_required(VERSION 3.21)
project(Mellangan VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (WIN32 AND NOT DEFINED ENV{VULKAN_SDK})
    file(GLOB _sdks LIST_DIRECTORIES true "C:/VulkanSDK/*")
    if (_sdks)
        list(SORT _sdks COMPARE NATURAL ORDER DESCENDING)
        list(GET _sdks 0 _sdk)
        set(ENV{VULKAN_SDK} "${_sdk}")
        message(STATUS "VULKAN_SDK not set, using ${_sdk}")
    endif ()
endif ()

# Shaders are compiled offline with the SDK's slangc, so Vulkan2D doesn't need to build Slang from source.
set(VK2D_DISABLE_SHADERS ON CACHE BOOL "" FORCE)
add_subdirectory(Vulkan2D)
add_subdirectory(extern/spdlog)
add_subdirectory(extern/entt)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.hpp)
add_executable(Mellangan ${SOURCES})
target_include_directories(Mellangan PRIVATE src)
target_link_libraries(Mellangan PRIVATE Vulkan2D SDL3::SDL3 spdlog::spdlog EnTT::EnTT)
# Only heavy, stable third-party headers: project headers change often and would invalidate the PCH.
target_precompile_headers(Mellangan PRIVATE
        <algorithm> <array> <cmath> <cstdint> <memory> <string> <string_view> <vector>
        <SDL3/SDL.h> <VK2D/VK2D.h> <entt/entt.hpp> <spdlog/spdlog.h> <spdlog/fmt/fmt.h>)

if (WIN32)
    target_compile_definitions(Mellangan PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_libraries(Mellangan PRIVATE dbghelp)
endif ()
if (MSVC)
    # PDBs in every config so crash traces resolve to file:line.
    target_compile_options(Mellangan PRIVATE /W4 /permissive- /utf-8 /Zi)
    target_link_options(Mellangan PRIVATE /DEBUG /OPT:REF /OPT:ICF)
else ()
    target_compile_options(Mellangan PRIVATE -Wall -Wextra)
endif ()

find_program(SLANGC slangc HINTS "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/bin" REQUIRED)
file(GLOB SHADER_SOURCES CONFIGURE_DEPENDS res/shaders/*.slang)
foreach (src ${SHADER_SOURCES})
    get_filename_component(name ${src} NAME_WE)
    set(spv ${CMAKE_CURRENT_BINARY_DIR}/shaders/${name}.spv)
    add_custom_command(OUTPUT ${spv}
            COMMAND ${SLANGC} ${src} -target spirv -profile glsl_450 -entry PixelShader -stage fragment -o ${spv}
            DEPENDS ${src}
            COMMENT "slangc ${name}.slang"
            VERBATIM)
    list(APPEND SHADER_BINARIES ${spv})
endforeach ()
add_custom_target(shaders DEPENDS ${SHADER_BINARIES})
add_dependencies(Mellangan shaders)

add_custom_command(TARGET Mellangan POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res $<TARGET_FILE_DIR:Mellangan>/res
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_BINARIES} $<TARGET_FILE_DIR:Mellangan>/res/shaders)
