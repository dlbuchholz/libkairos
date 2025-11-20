cmake_minimum_required(VERSION 3.16)

project(libkairos LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# Define source files
set(SRC_DIR src)
file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.h")

# Include directories
set(INCLUDE_DIR include)
include_directories(${INCLUDE_DIR})

add_library(libkairos STATIC
  ${SRC_FILES}
)

target_include_directories(libkairos
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(libkairos PUBLIC Qt${QT_VERSION_MAJOR}::Core)

target_compile_definitions(libkairos PUBLIC LIBKAIROS_LIBRARY)

option(LIBKAIROS_BUILD_TESTS "Build unit tests for libkairos" ON)

if(LIBKAIROS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
