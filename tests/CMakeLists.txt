# --------------------------------------------------------------------------
# Unit tests (QTest)
# --------------------------------------------------------------------------
# Shared object library with the sources under test, so test executables
# are independent of the GUI application target.
find_package(Qt6 REQUIRED COMPONENTS Test)

add_library(ltm_testlib STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../cpustats.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../ramstatreader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../diskstatreader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../networkstatreader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../processstatreader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../processgroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../processsorttree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../perfitemwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../cpuitemwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../ramitemwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../diskitemwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../networkitemwidget.cpp
)

target_compile_definitions(ltm_testlib PRIVATE QT_DEPRECATED_WARNINGS)

target_include_directories(ltm_testlib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${QWT_INCLUDE_DIR}
    ${NETHOGS_INCLUDE_DIR}
)

target_link_libraries(ltm_testlib PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Test
    ${QWT_LIBRARY}
    ${NETHOGS_LIBRARY}
)

# --------------------------------------------------------------------------
# Per-module test executables — one per module
# --------------------------------------------------------------------------
function(ltm_add_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE ltm_testlib)
    add_test(NAME ${name} COMMAND ${name})
    # GUI-based tests need an offscreen platform
    set_tests_properties(${name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        TIMEOUT 120
    )
endfunction()

# Auto-register every t_*.cpp that exists in this directory, so new
# per-module tests can be added incrementally.
file(GLOB LTM_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/t_*.cpp)
foreach(src ${LTM_TEST_SOURCES})
    get_filename_component(tname ${src} NAME_WE)
    ltm_add_test(${tname} ${src})
endforeach()
