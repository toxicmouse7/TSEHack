cmake_minimum_required(VERSION 3.30)
project(showdown_hack LANGUAGES CXX)

set(TARGET_SCRIPT_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/custom.lua")
set(TARGET_SCRIPT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/generated/features/target_script.hpp")
set(UNLOCK_SCRIPT_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/unlock_script.lua")
set(UNLOCK_SCRIPT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/generated/features/unlock_script.hpp")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${TARGET_SCRIPT_SOURCE}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${UNLOCK_SCRIPT_SOURCE}")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated/features")
file(READ "${TARGET_SCRIPT_SOURCE}" TARGET_SCRIPT)
file(READ "${UNLOCK_SCRIPT_SOURCE}" UNLOCK_SCRIPT)
configure_file(src/features/target_script.hpp.in "${TARGET_SCRIPT_HEADER}" @ONLY)
configure_file(src/features/unlock_script.hpp.in "${UNLOCK_SCRIPT_HEADER}" @ONLY)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(showdown_hack SHARED
        src/dll/entry.cpp
        src/dll/runtime.cpp
        src/hooks/lua_api.cpp
        src/hooks/lua_hooks.cpp
        src/game/lua_game_state_reader.cpp
        src/features/target_assist.cpp
        src/overlay/overlay.cpp
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/backends/imgui_impl_dx11.cpp
        external/imgui/backends/imgui_impl_win32.cpp
)

find_package(minhook CONFIG REQUIRED)
target_include_directories(showdown_hack PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated" src external/imgui)
target_link_libraries(showdown_hack PRIVATE minhook::minhook d3d11 dxgi)

if (MSVC)
    target_compile_options(showdown_hack PRIVATE /W4 /permissive-)
endif()
