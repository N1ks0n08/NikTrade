cmake_minimum_required(VERSION 3.10)

project(NikTrade)

set(CMAKE_CXX_STANDARD 20)

find_package(fmt CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_subdirectory(third_party/imgui)

include_directories(src/)

add_executable(NikTrade 
    src/main.cpp
    src/core/data_loader.cpp
    src/core/tech_indicators/sma.cpp
    src/core/tech_indicators/ema.cpp
    src/core/tech_indicators/rsi.cpp
    src/core/tech_indicators/macd.cpp
    src/core/tech_indicators/vwap.cpp
    src/core/backtest_engines/sma_crossover.cpp
    src/core/backtest_engines/macd_vwap_optimizer.cpp

    src/ui/core/init.cpp
    src/ui/windows/main_window.cpp
    src/ui/windows/dataDisplay_window.cpp
    src/ui/plots/price_plot.cpp
    src/ui/plots/sma_plot.cpp
    src/ui/plots/ema_plot.cpp
    src/ui/plots/vwap_plot.cpp
    src/ui/plots/macd_plot.cpp
    src/ui/backtest_plots/sma_crossover_plot.cpp

    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/imgui_demo.cpp
    third_party/imgui/backends/imgui_impl_glfw.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
    third_party/implot/implot.cpp
    third_party/implot/implot_items.cpp
)

if(MSVC)
    set_target_properties(NikTrade PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )
endif()

target_include_directories(NikTrade PRIVATE third_party/implot)

target_link_libraries(NikTrade PRIVATE fmt::fmt)
target_link_libraries(NikTrade PRIVATE glfw)
target_link_libraries(NikTrade PRIVATE glad::glad)
target_link_libraries(NikTrade PRIVATE imgui)
target_link_libraries(NikTrade PRIVATE cpr::cpr)
target_link_libraries(NikTrade PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(NikTrade PRIVATE opengl32)
target_link_directories(NikTrade PRIVATE third_party/implot)
