cmake_minimum_required(VERSION 3.21)
project(AstraPDF VERSION 0.2.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.7 REQUIRED COMPONENTS Core Gui Widgets Pdf Network)

qt_add_executable(AstraPDF WIN32
    src/main.cpp
    src/MainWindow.h src/MainWindow.cpp
    src/PdfCanvas.h src/PdfCanvas.cpp
    src/LayoutManager.h src/LayoutManager.cpp
    src/CosInspector.h src/CosInspector.cpp
    src/ReaderSessionController.h src/ReaderSessionController.cpp
    src/UiEnhancer.h src/UiEnhancer.cpp
    src/BackgroundController.h src/BackgroundController.cpp
    src/InterfaceController.h src/InterfaceController.cpp
)

target_link_libraries(AstraPDF PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Pdf Qt6::Network)

if(MSVC)
    target_compile_options(AstraPDF PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(AstraPDF PRIVATE -Wall -Wextra -Wpedantic)
endif()
set_target_properties(AstraPDF PROPERTIES OUTPUT_NAME "AstraPDF")
