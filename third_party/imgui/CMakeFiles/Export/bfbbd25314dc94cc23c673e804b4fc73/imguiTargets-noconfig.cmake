#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "imgui::imgui" for configuration ""
set_property(TARGET imgui::imgui APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(imgui::imgui PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libimgui.a"
  )

list(APPEND _cmake_import_check_targets imgui::imgui )
list(APPEND _cmake_import_check_files_for_imgui::imgui "${_IMPORT_PREFIX}/lib/libimgui.a" )

# Import target "imgui::backend_glfw_opengl2" for configuration ""
set_property(TARGET imgui::backend_glfw_opengl2 APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(imgui::backend_glfw_opengl2 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libimgui_backend_glfw_opengl2.a"
  )

list(APPEND _cmake_import_check_targets imgui::backend_glfw_opengl2 )
list(APPEND _cmake_import_check_files_for_imgui::backend_glfw_opengl2 "${_IMPORT_PREFIX}/lib/libimgui_backend_glfw_opengl2.a" )

# Import target "imgui::backend_glfw_opengl3" for configuration ""
set_property(TARGET imgui::backend_glfw_opengl3 APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(imgui::backend_glfw_opengl3 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libimgui_backend_glfw_opengl3.a"
  )

list(APPEND _cmake_import_check_targets imgui::backend_glfw_opengl3 )
list(APPEND _cmake_import_check_files_for_imgui::backend_glfw_opengl3 "${_IMPORT_PREFIX}/lib/libimgui_backend_glfw_opengl3.a" )

# Import target "imgui::backend_sdl2_opengl2" for configuration ""
set_property(TARGET imgui::backend_sdl2_opengl2 APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(imgui::backend_sdl2_opengl2 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libimgui_backend_sdl2_opengl2.a"
  )

list(APPEND _cmake_import_check_targets imgui::backend_sdl2_opengl2 )
list(APPEND _cmake_import_check_files_for_imgui::backend_sdl2_opengl2 "${_IMPORT_PREFIX}/lib/libimgui_backend_sdl2_opengl2.a" )

# Import target "imgui::backend_sdl2_opengl3" for configuration ""
set_property(TARGET imgui::backend_sdl2_opengl3 APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(imgui::backend_sdl2_opengl3 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libimgui_backend_sdl2_opengl3.a"
  )

list(APPEND _cmake_import_check_targets imgui::backend_sdl2_opengl3 )
list(APPEND _cmake_import_check_files_for_imgui::backend_sdl2_opengl3 "${_IMPORT_PREFIX}/lib/libimgui_backend_sdl2_opengl3.a" )

# Import target "imgui::backend_sdl2_sdlrenderer2" for configuration ""
set_property(TARGET imgui::backend_sdl2_sdlrenderer2 APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(imgui::backend_sdl2_sdlrenderer2 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libimgui_backend_sdl2_sdlrenderer2.a"
  )

list(APPEND _cmake_import_check_targets imgui::backend_sdl2_sdlrenderer2 )
list(APPEND _cmake_import_check_files_for_imgui::backend_sdl2_sdlrenderer2 "${_IMPORT_PREFIX}/lib/libimgui_backend_sdl2_sdlrenderer2.a" )

# Import target "imgui::backend_glut_opengl2" for configuration ""
set_property(TARGET imgui::backend_glut_opengl2 APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(imgui::backend_glut_opengl2 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libimgui_backend_glut_opengl2.a"
  )

list(APPEND _cmake_import_check_targets imgui::backend_glut_opengl2 )
list(APPEND _cmake_import_check_files_for_imgui::backend_glut_opengl2 "${_IMPORT_PREFIX}/lib/libimgui_backend_glut_opengl2.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
