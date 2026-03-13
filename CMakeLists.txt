cmake_minimum_required(VERSION 3.20)
project(CCTool VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 可执行文件输出到 Binaries/（MSVC 多配置生成器需要单独指定每个配置）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Binaries)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Binaries)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Binaries)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/Binaries)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/Binaries)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/Binaries)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/Binaries)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/Binaries)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/Binaries)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

set(SOURCES
    Sources/Main.cpp
    Sources/MainWindow.cpp
    Sources/Ui/LeftPanel.cpp
    Sources/Ui/RepoTab.cpp
    Sources/Ui/TalkView.cpp
    Sources/Ui/RichTextEditor.cpp
    Sources/Ui/HistoryPanel.cpp
    Sources/Ui/SessionPickerDialog.cpp
    Sources/Core/CCProcess.cpp
    Sources/Core/CCSessionReader.cpp
    Sources/Core/StreamSimulator.cpp
    Sources/Data/Database.cpp
)

set(HEADERS
    Sources/MainWindow.h
    Sources/Ui/LeftPanel.h
    Sources/Ui/RepoTab.h
    Sources/Ui/TalkView.h
    Sources/Ui/RichTextEditor.h
    Sources/Ui/HistoryPanel.h
    Sources/Ui/SessionPickerDialog.h
    Sources/Core/CCProcess.h
    Sources/Core/CCSessionReader.h
    Sources/Core/StreamSimulator.h
    Sources/Data/Database.h
    Sources/Data/RepoInfo.h
    Sources/Data/SessionInfo.h
    Sources/Data/Message.h
)

qt_add_executable(CCTool
    ${SOURCES}
    ${HEADERS}
    Resources/App.qrc
)

target_include_directories(CCTool PRIVATE Sources)

target_link_libraries(CCTool
    PRIVATE
    Qt6::Widgets
    Qt6::Sql
)

set_target_properties(CCTool PROPERTIES
    WIN32_EXECUTABLE TRUE
)
