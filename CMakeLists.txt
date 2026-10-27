cmake_minimum_required(VERSION 3.21)
file(STRINGS VERSION MELLANGAN_VERSION LIMIT_COUNT 1)
project(Mellangan VERSION ${MELLANGAN_VERSION} LANGUAGES C CXX)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS VERSION)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Static CRT everywhere so a release needs no VC++ redistributable.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

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
add_subdirectory(extern/Vulkan2D)
add_subdirectory(extern/spdlog)
add_subdirectory(extern/entt)

add_library(stb_vorbis STATIC extern/stb/stb_vorbis.c)
target_include_directories(stb_vorbis PUBLIC extern/stb)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.hpp src/*.c src/*.h)
file(GLOB_RECURSE C_SOURCES CONFIGURE_DEPENDS src/*.c)
# The precompiled header is C++ only.
set_source_files_properties(${C_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
add_executable(Mellangan ${SOURCES})
# Generated header so a version bump only recompiles the files that show the version.
configure_file(src/Version.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/generated/Version.hpp @ONLY)
target_include_directories(Mellangan PRIVATE src ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_link_libraries(Mellangan PRIVATE Vulkan2D SDL3::SDL3 spdlog::spdlog EnTT::EnTT stb_vorbis)
# Only heavy, stable third-party headers: project headers change often and would invalidate the PCH.
target_precompile_headers(Mellangan PRIVATE src/pch.hpp)

if (WIN32)
    target_compile_definitions(Mellangan PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_libraries(Mellangan PRIVATE dbghelp)
endif ()
if (MSVC)
    # PDBs in every config so crash traces resolve to file:line.
    target_compile_options(Mellangan PRIVATE /W4 /permissive- /utf-8 /Zi /MP)
    target_link_options(Mellangan PRIVATE /DEBUG /OPT:REF /OPT:ICF
            $<$<NOT:$<CONFIG:Debug>>:/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup>)
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
