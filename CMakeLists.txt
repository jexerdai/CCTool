cmake_minimum_required(VERSION 3.20)
project(CCTool VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 可执行文件输出到项目根目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

set(SOURCES
    Src/Main.cpp
    Src/MainWindow.cpp
    Src/Ui/LeftPanel.cpp
    Src/Ui/RepoTab.cpp
    Src/Ui/OutputView.cpp
    Src/Ui/RichTextEditor.cpp
    Src/Ui/HistoryPanel.cpp
    Src/Core/CCProcess.cpp
    Src/Core/StreamSimulator.cpp
    Src/Data/Database.cpp
)

set(HEADERS
    Src/MainWindow.h
    Src/Ui/LeftPanel.h
    Src/Ui/RepoTab.h
    Src/Ui/OutputView.h
    Src/Ui/RichTextEditor.h
    Src/Ui/HistoryPanel.h
    Src/Core/CCProcess.h
    Src/Core/StreamSimulator.h
    Src/Data/Database.h
    Src/Data/RepoInfo.h
    Src/Data/SessionInfo.h
    Src/Data/Message.h
)

qt_add_executable(CCTool
    ${SOURCES}
    ${HEADERS}
    Resources/App.qrc
)

target_include_directories(CCTool PRIVATE Src)

target_link_libraries(CCTool
    PRIVATE
    Qt6::Widgets
    Qt6::Sql
)

set_target_properties(CCTool PROPERTIES
    WIN32_EXECUTABLE TRUE
)
