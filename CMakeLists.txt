cmake_minimum_required(VERSION 3.21)
file(STRINGS VERSION MELLANGAN_VERSION LIMIT_COUNT 1)
if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    # MoltenVK's official release requires macOS 12.
    set(CMAKE_OSX_DEPLOYMENT_TARGET 12.0 CACHE STRING "")
endif ()
project(Mellangan VERSION ${MELLANGAN_VERSION} LANGUAGES C CXX)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS VERSION)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
# Static CRT everywhere so a release needs no VC++ redistributable.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
get_property(_multi GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (NOT _multi AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif ()
# SDL is linked in statically so a release is just the executable and res/.
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)

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
if (APPLE)
    # The app links MoltenVK directly and embeds it; the loader option is for local runs with validation layers.
    option(MELLANGAN_VULKAN_LOADER "Link the system Vulkan loader instead of the bundled MoltenVK" OFF)
    find_package(Vulkan REQUIRED)
    if (NOT MELLANGAN_VULKAN_LOADER)
        file(GLOB _mvk_dirs LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/build-deps/moltenvk-*")
        find_library(MOLTENVK_LIBRARY MoltenVK PATHS ${_mvk_dirs} NO_DEFAULT_PATH REQUIRED)
        set_target_properties(Vulkan::Vulkan PROPERTIES IMPORTED_LOCATION "${MOLTENVK_LIBRARY}")
    endif ()
endif ()
add_subdirectory(extern/Vulkan2D)
add_subdirectory(extern/spdlog)
add_subdirectory(extern/entt)

add_library(stb_vorbis STATIC extern/stb/stb_vorbis.c)
target_include_directories(stb_vorbis PUBLIC extern/stb)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.hpp src/*.c src/*.h)
file(GLOB_RECURSE C_SOURCES CONFIGURE_DEPENDS src/*.c)
# The precompiled header is C++ only.
set_source_files_properties(${C_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

if (WIN32)
    configure_file(res/Mellangan.rc.in ${CMAKE_CURRENT_BINARY_DIR}/generated/Mellangan.rc @ONLY)
    list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/generated/Mellangan.rc)
endif ()

if (APPLE)
    set(_icns ${CMAKE_CURRENT_BINARY_DIR}/generated/Mellangan.icns)
    set(_iconset ${CMAKE_CURRENT_BINARY_DIR}/generated/Mellangan.iconset)
    add_custom_command(OUTPUT ${_icns}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${_iconset}
            COMMAND sips -z 16 16 ${CMAKE_CURRENT_SOURCE_DIR}/res/icon.png --out ${_iconset}/icon_16x16.png
            COMMAND sips -z 32 32 ${CMAKE_CURRENT_SOURCE_DIR}/res/icon.png --out ${_iconset}/icon_16x16@2x.png
            COMMAND sips -z 32 32 ${CMAKE_CURRENT_SOURCE_DIR}/res/icon.png --out ${_iconset}/icon_32x32.png
            COMMAND sips -z 64 64 ${CMAKE_CURRENT_SOURCE_DIR}/res/icon.png --out ${_iconset}/icon_32x32@2x.png
            COMMAND sips -z 128 128 ${CMAKE_CURRENT_SOURCE_DIR}/res/icon.png --out ${_iconset}/icon_128x128.png
            COMMAND sips -z 256 256 ${CMAKE_CURRENT_SOURCE_DIR}/res/icon.png --out ${_iconset}/icon_128x128@2x.png
            COMMAND sips -z 256 256 ${CMAKE_CURRENT_SOURCE_DIR}/res/icon.png --out ${_iconset}/icon_256x256.png
            COMMAND iconutil -c icns ${_iconset} -o ${_icns}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/icon.png
            VERBATIM)
    set_source_files_properties(${_icns} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    list(APPEND SOURCES ${_icns})
endif ()

add_executable(Mellangan ${SOURCES})
if (APPLE)
    configure_file(res/Info.plist.in ${CMAKE_CURRENT_BINARY_DIR}/generated/Info.plist @ONLY)
    set_target_properties(Mellangan PROPERTIES
            MACOSX_BUNDLE ON
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/generated/Info.plist
            OUTPUT_NAME "Mellangan"
            BUILD_WITH_INSTALL_RPATH ON
            INSTALL_RPATH "@executable_path/../Frameworks")
else ()
    set_target_properties(Mellangan PROPERTIES OUTPUT_NAME "MellanganApp")
endif ()
# Generated header so a version bump only recompiles the files that show the version.
configure_file(src/Version.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/generated/Version.hpp @ONLY)
target_include_directories(Mellangan PRIVATE src ${CMAKE_CURRENT_BINARY_DIR}/generated)
find_package(Threads REQUIRED)
target_link_libraries(Mellangan PRIVATE Vulkan2D SDL3::SDL3 spdlog::spdlog EnTT::EnTT stb_vorbis Threads::Threads)
# Only heavy, stable third-party headers: project headers change often and would invalidate the PCH.
target_precompile_headers(Mellangan PRIVATE src/pch.hpp)

if (WIN32)
    target_compile_definitions(Mellangan PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif ()
if (MSVC)
    target_compile_options(Mellangan PRIVATE /W4 /permissive- /utf-8 /MP)
    target_link_options(Mellangan PRIVATE /OPT:REF /OPT:ICF
            $<$<CONFIG:Debug>:/DEBUG>
            $<$<NOT:$<CONFIG:Debug>>:/SUBSYSTEM:WINDOWS>
            "/MANIFESTINPUT:${CMAKE_CURRENT_SOURCE_DIR}/res/Mellangan.manifest")
else ()
    target_compile_options(Mellangan PRIVATE -Wall -Wextra)
endif ()
if (UNIX AND NOT APPLE)
    target_link_options(Mellangan PRIVATE -rdynamic)
endif ()

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set(_inc "$<TARGET_PROPERTY:Mellangan,INCLUDE_DIRECTORIES>")
    set(_def "$<TARGET_PROPERTY:Mellangan,COMPILE_DEFINITIONS>")
    set(_flags "\"-I$<JOIN:${_inc},\",\"-I>\"$<$<BOOL:${_def}>:,\"-D$<JOIN:${_def},\",\"-D>\">")
    set(_entries "")
    foreach (src ${SOURCES})
        if (NOT src MATCHES "\\.(c|cpp)$")
            continue ()
        endif ()
        if (src MATCHES "\\.c$")
            set(_lang "\"clang\",\"-std=c11\"")
        else ()
            set(_lang "\"clang++\",\"-std=c++20\",\"-include\",\"${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp\"")
        endif ()
        list(APPEND _entries "{\"directory\":\"${CMAKE_CURRENT_BINARY_DIR}\",\"file\":\"${src}\",\"arguments\":[${_lang},${_flags},\"-c\",\"${src}\"]}")
    endforeach ()
    list(JOIN _entries ",\n" _entries)
    file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/compile_commands.json CONTENT "[\n${_entries}\n]\n" CONDITION $<CONFIG:Release>)
endif ()

if (APPLE)
    file(GLOB _slang_bins LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/build-deps/slang-*/bin")
endif ()
find_program(SLANGC slangc HINTS "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/bin" ${_slang_bins} REQUIRED)
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

if (APPLE)
    set(_res $<TARGET_BUNDLE_CONTENT_DIR:Mellangan>/Resources/res)
    set(_embed "")
    if (NOT MELLANGAN_VULKAN_LOADER)
        set(_embed COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_BUNDLE_CONTENT_DIR:Mellangan>/Frameworks
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MOLTENVK_LIBRARY} $<TARGET_BUNDLE_CONTENT_DIR:Mellangan>/Frameworks
                COMMAND codesign --force --sign - $<TARGET_BUNDLE_CONTENT_DIR:Mellangan>/Frameworks/libMoltenVK.dylib)
    endif ()
    add_custom_command(TARGET Mellangan POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${_res}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_BINARIES} ${_res}/shaders
            ${_embed}
            COMMAND codesign --force --sign - $<TARGET_BUNDLE_DIR:Mellangan>
            VERBATIM)
else ()
    add_custom_command(TARGET Mellangan POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res $<TARGET_FILE_DIR:Mellangan>/res
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_BINARIES} $<TARGET_FILE_DIR:Mellangan>/res/shaders)
endif ()
