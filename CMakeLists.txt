cmake_minimum_required(VERSION 3.3)

project(phone VERSION 0.3.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN true)

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

include(cmake/credentials.cmake)

add_subdirectory(src bin)

install(TARGETS phone_sharedlib)
