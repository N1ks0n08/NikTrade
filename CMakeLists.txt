cmake_minimum_required(VERSION 3.10)

project(NikTrade)

set(CMAKE_CXX_STANDARD 20)

find_package(fmt CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(ZeroMQ CONFIG REQUIRED)
find_package(cppzmq CONFIG REQUIRED)
find_package(flatbuffers CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS lockfree)

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
    src/core/backtest_engines/macd_vwapBacktester.cpp

    src/core/net/zmq_subscriber.cpp
    src/core/net/python_launcher.cpp
    src/core/net/zmq_control_client.cpp

    src/utils/file_logger.cpp

    src/ui/core/init.cpp
    src/ui/windows/banner_window.cpp
    src/ui/windows/dataDisplay_window.cpp
    src/ui/windows/cryptoDataDisplay_window.cpp
    src/ui/windows/cryptoChartDisplay_window.cpp
    src/ui/plots/price_plot.cpp
    src/ui/plots/sma_plot.cpp
    src/ui/plots/ema_plot.cpp
    src/ui/plots/vwap_plot.cpp
    src/ui/plots/rsi_plot.cpp
    src/ui/plots/macd_plot.cpp
    src/ui/backtest_plots/sma_crossover_plot.cpp
    src/ui/backtest_plots/macd_vwapBacktester_plot.cpp

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
target_link_libraries(NikTrade PRIVATE libzmq libzmq-static)
target_link_libraries(NikTrade PRIVATE cppzmq cppzmq-static)
target_link_libraries(NikTrade PRIVATE flatbuffers::flatbuffers)
target_link_libraries(NikTrade PRIVATE Boost::lockfree)

# ------------------- Copy Python scripts and resources -------------------
# This ensures build/ has python/ and resources/ copied for testing
add_custom_command(TARGET NikTrade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/python
        $<TARGET_FILE_DIR:NikTrade>/python
)
add_custom_command(TARGET NikTrade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:NikTrade>/resources
)

# ------------------- Copy Binance FlatBuffers Python module -------------------
add_custom_command(TARGET NikTrade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/flatbuffers/Binance
        $<TARGET_FILE_DIR:NikTrade>/python/Binance
)

# ------------------- Install rules -------------------
install(TARGETS NikTrade DESTINATION bin)
install(DIRECTORY python DESTINATION bin)
install(DIRECTORY resources DESTINATION bin)
# Install Binance FlatBuffers Python module
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/core/flatbuffers/Binance
        DESTINATION bin/python
        FILES_MATCHING PATTERN "*.py"
)
