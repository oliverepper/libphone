cmake_minimum_required(VERSION 3.3)

project(phone VERSION 0.9.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN true)

if(NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/installed/pjproject/macos-arm64"
            CACHE STRING "Path to pjproject dependency")
endif()

include(GNUInstallDirs)

file(RELATIVE_PATH pathToLibs
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

list(APPEND CMAKE_MESSAGE_INDENT "🔧 ")
if(APPLE)
    message(DEBUG "Configuring for Apple platform")
    set(CMAKE_INSTALL_RPATH "@loader_path/${pathToLibs}")
elseif(UNIX)
    message(DEBUG "Configuring for UNIX")
    set(CMAKE_INSTALL_RPATH "$ORIGIN/${pathToLibs}")
endif()
message(DEBUG "CMAKE_INSTALL_RPATH: ${CMAKE_INSTALL_RPATH}")
list(POP_BACK CMAKE_MESSAGE_INDENT)

if (EXISTS "${CMAKE_SOURCE_DIR}/cmake/credentials.cmake")
    include(cmake/credentials.cmake)
else()
    message(FATAL_ERROR "please copy cmake/credentials.cmake.example to cmake/credentials.cmake")
endif()

add_subdirectory(src bin)

install(TARGETS phone_sharedlib cpp_cli)
