

set(SUPPORTED_BACKENDS "glfw_opengl2;glfw_opengl3;sdl2_sdlrenderer2;sdl2_opengl2;sdl2_opengl3;glut_opengl2")
set(HAS_IMGUI_EMSCRIPTEN "")
set(HAS_IMGUI_DAWN "")
set(HAS_IMGUI_ALLEGRO "")
set(HAS_IMGUI_GLFW "ON")
set(HAS_IMGUI_GLUT "ON")
set(HAS_IMGUI_OPENGL "ON")
set(HAS_IMGUI_SDL2 "ON")
set(HAS_IMGUI_SDL3 "")
set(HAS_IMGUI_VULKAN "")
set(HAS_IMGUI_DIRECTX_9_X64 "")
set(HAS_IMGUI_DIRECTX_9_X86 "")
set(HAS_IMGUI_DIRECTX_10_X64 "")
set(HAS_IMGUI_DIRECTX_10_X86 "")
set(HAS_IMGUI_DIRECTX_11_X64 "")
set(HAS_IMGUI_DIRECTX_11_X86 "")
set(HAS_IMGUI_DIRECTX_12_X64 "")
set(HAS_IMGUI_DIRECTX_12_X86 "")
set(WITH_FREETYPE "")

if (HAS_IMGUI_GLFW)
    find_package(glfw3 REQUIRED)
endif ()

if (HAS_IMGUI_OPENGL)
    find_package(OpenGL REQUIRED)
endif ()

if (HAS_IMGUI_SDL2)
    find_package(SDL2 REQUIRED)
endif ()

if (HAS_IMGUI_SDL3)
    find_package(SDL3 REQUIRED)
endif ()

if (HAS_IMGUI_VULKAN)
    find_package(Vulkan REQUIRED)
endif ()

if (HAS_IMGUI_ALLEGRO)
    find_package(Allegro REQUIRED)
endif ()

if (HAS_IMGUI_GLUT)
    find_package(GLUT QUIET)
    if (NOT GLUT_FOUND)
        find_package(FreeGLUT REQUIRED)
    endif ()
endif ()

if (HAS_IMGUI_DAWN)
    find_package(Dawn REQUIRED)
endif ()

if (WITH_FREETYPE AND NOT HAS_IMGUI_EMSCRIPTEN)
    find_package(freetype REQUIRED CONFIG)
endif ()

include("${CMAKE_CURRENT_LIST_DIR}/imguiTargets.cmake")
