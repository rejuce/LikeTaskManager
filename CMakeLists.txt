cmake_minimum_required(VERSION 3.16)

project(LikeTaskManager
    VERSION 1.0.0
    DESCRIPTION "Qt-based system performance monitor for Linux"
    LANGUAGES CXX
)

# --------------------------------------------------------------------------
# Options / settings
# --------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
# LinguistTools is optional: translation generation is skipped if unavailable.
find_package(Qt6 COMPONENTS LinguistTools)

# Qwt — no CMake config is shipped, so locate it manually.
# Point CMake at a Qwt install with -DQWT_ROOT=<path> if it is not found.
find_path(QWT_INCLUDE_DIR
    NAMES qwt_plot.h
    HINTS ${QWT_ROOT} /usr/local/qwt-6.3.0 /usr/local /usr
    PATH_SUFFIXES include include/qwt
)
find_library(QWT_LIBRARY
    NAMES qwt
    HINTS ${QWT_ROOT} /usr/local/qwt-6.3.0 /usr/local /usr
    PATH_SUFFIXES lib lib/x86_64-linux-gnu
)
if(NOT QWT_INCLUDE_DIR OR NOT QWT_LIBRARY)
    message(FATAL_ERROR
        "Qwt not found. Install Qwt 6.x or pass -DQWT_ROOT=<Qwt install prefix>.")
endif()

# libnethogs — built from source, installed to /usr/local
find_library(NETHOGS_LIBRARY
    NAMES libnethogs nethogs
    HINTS /usr/local /usr
    PATH_SUFFIXES lib lib/x86_64-linux-gnu
)
find_path(NETHOGS_INCLUDE_DIR
    NAMES libnethogs.h
    HINTS /usr/local /usr
    PATH_SUFFIXES include
)
if(NOT NETHOGS_LIBRARY OR NOT NETHOGS_INCLUDE_DIR)
    message(FATAL_ERROR
        "libnethogs not found. Build libnethogs from source or pass its prefix.")
endif()

# --------------------------------------------------------------------------
# Target
# --------------------------------------------------------------------------
add_executable(${PROJECT_NAME}
    main.cpp
    ltm.cpp
    ltm.h
    ltm.ui
    ltm_cpu.cpp
    ltm_ram.cpp
    ltm_disk.cpp
    ltm_network.cpp
    cpuitemwidget.cpp
    cpuitemwidget.h
    cpustatreader.cpp
    cpustatreader.h
    cpustats.cpp
    cpustats.h
    ramitemwidget.cpp
    ramitemwidget.h
    ramstatreader.cpp
    ramstatreader.h
    diskitemwidget.cpp
    diskitemwidget.h
    diskstatreader.cpp
    diskstatreader.h
    networkitemwidget.cpp
    networkitemwidget.h
    networkstatreader.cpp
    networkstatreader.h
    processstatreader.cpp
    processstatreader.h
    processgroup.cpp
    processgroup.h
    processsorttree.cpp
    processsorttree.h
    perfitemwidget.cpp
    perfitemwidget.h
    StatReader.h
    images.qrc
    qdarkstyle/style.qrc
)

target_compile_definitions(${PROJECT_NAME} PRIVATE QT_DEPRECATED_WARNINGS)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QWT_INCLUDE_DIR}
    ${NETHOGS_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    ${QWT_LIBRARY}
    ${NETHOGS_LIBRARY}
)

# Keep symbols visible so backtrace_symbols() in the crash handler works
target_link_options(${PROJECT_NAME} PRIVATE -rdynamic)

# --------------------------------------------------------------------------
# Translations
# --------------------------------------------------------------------------
if(Qt6LinguistTools_FOUND)
    qt6_add_translations(${PROJECT_NAME}
        TS_FILES LikeTaskManager_de_AT.ts
    )
else()
    message(WARNING "Qt6LinguistTools not found - skipping translation generation")
endif()

# --------------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------------
option(LTM_BUILD_TESTS "Build unit tests" ON)
if(LTM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# --------------------------------------------------------------------------
# Install
# --------------------------------------------------------------------------
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /opt/LikeTaskManager/bin)
