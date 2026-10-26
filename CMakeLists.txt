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

add_subdirectory(Vulkan2D)
add_subdirectory(extern/spdlog)
add_subdirectory(extern/entt)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.hpp)
add_executable(Mellangan ${SOURCES})
target_include_directories(Mellangan PRIVATE src)
target_link_libraries(Mellangan PRIVATE Vulkan2D SDL3::SDL3 spdlog::spdlog EnTT::EnTT)

if (MSVC)
    target_compile_options(Mellangan PRIVATE /W4 /permissive- /utf-8)
else ()
    target_compile_options(Mellangan PRIVATE -Wall -Wextra)
endif ()

add_custom_command(TARGET Mellangan POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res $<TARGET_FILE_DIR:Mellangan>/res)
if (WIN32)
    add_custom_command(TARGET Mellangan POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:Mellangan> $<TARGET_FILE_DIR:Mellangan>
            COMMAND_EXPAND_LISTS)
endif ()
