find_package(PkgConfig REQUIRED)

pkg_check_modules(PJPROJECT REQUIRED IMPORTED_TARGET libpjproject)
pkg_check_modules(OPUS REQUIRED IMPORTED_TARGET opus)

# configure git variables in version templates
find_package(Git REQUIRED)
execute_process(
        COMMAND ${GIT_EXECUTABLE} -C ${CMAKE_SOURCE_DIR} rev-parse HEAD
        RESULT_VARIABLE result
        OUTPUT_VARIABLE phone_GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE)
if(result)
    message(FATAL_ERROR "failed to get git hash: ${result}")
endif()

execute_process(
        COMMAND ${GIT_EXECUTABLE} -C ${CMAKE_SOURCE_DIR} describe --dirty
        RESULT_VARIABLE result
        OUTPUT_VARIABLE phone_GIT_DESCRIPTION
        OUTPUT_STRIP_TRAILING_WHITESPACE)
if(result)
    message(FATAL_ERROR "failed to get git description: ${result}")
endif()

configure_file(version_c.cpp.in version.cpp @ONLY)
configure_file(version_cpp.cpp.in version_cpp.cpp @ONLY)

add_library(phone_version OBJECT ${CMAKE_CURRENT_BINARY_DIR}/version.cpp ${CMAKE_CURRENT_BINARY_DIR}/version_cpp.cpp)

target_include_directories(phone_version
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR})

add_library(phone OBJECT
        phone.cpp
        phone_instance_t.cpp
        include/private/account_t.h
        include/private/call_t.h
        include/private/calls_t.h
        include/private/linked_list_iterator.h
        include/private/IfAddrs.h)

include(GenerateExportHeader)
generate_export_header(phone BASE_NAME "phone")

target_compile_options(phone PUBLIC ${PJPROJECT_CFLAGS} "-ffile-prefix-map=${CMAKE_SOURCE_DIR}=.")

if (APPLE)
        find_library(RESOLV_LIBRARY resolv)
endif()

include(FetchContent)

FetchContent_Declare(stunning
    GIT_REPOSITORY https://github.com/oliverepper/stunning.git
    GIT_TAG        main)

FetchContent_MakeAvailable(stunning)

target_include_directories(phone
        PUBLIC
        include
        ${CMAKE_CURRENT_BINARY_DIR}
        PRIVATE
        include/phone)

target_link_libraries(phone PRIVATE ${RESOLV_LIBRARY} stunning::stunning PkgConfig::PJPROJECT PkgConfig::OPUS)

set(phone_PUBLIC_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/phone.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/phone_instance_t.h
        ${CMAKE_CURRENT_BINARY_DIR}/phone_export.h)

add_library(phone_sharedlib SHARED dummy.cpp)
set_target_properties(phone_sharedlib PROPERTIES
    OUTPUT_NAME "phone"
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${phone_PUBLIC_HEADERS}")
target_link_libraries(phone_sharedlib PUBLIC phone phone_version)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/phone DESTINATION include)

if(BUILD_FRAMEWORK)
  set(phonekit_FRAMEWORK_PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PhoneKit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/phone.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/phone/version.h
    ${CMAKE_CURRENT_BINARY_DIR}/phone_export.h)

  add_library(PhoneKit SHARED
    dummy.cpp
    ${phonekit_FRAMEWORK_PUBLIC_HEADERS})

  set_target_properties(PhoneKit PROPERTIES
    FRAMEWORK TRUE
    FRAMEWORK_VERSION A
    MACOSX_FRAMEWORK_BUNDLE_NAME PhoneKit
    MACOSX_FRAMEWORK_IDENTIFIER de.oliver-epper.PhoneKit
    MACOSX_FRAMEWORK_BUNDLE_VERSION 1
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING "1.0"
    PUBLIC_HEADER "${phonekit_FRAMEWORK_PUBLIC_HEADERS}")

  target_link_libraries(PhoneKit PUBLIC phone phone_version)

  configure_file(modulemap.in module.modulemap @ONLY)

  target_sources(PhoneKit PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/module.modulemap)

  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/module.modulemap PROPERTIES
    MACOSX_PACKAGE_LOCATION Modules)

  install(TARGETS PhoneKit DESTINATION "${CMAKE_INSTALL_PREFIX}")

  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND NOT CMAKE_OSX_SYSROOT MATCHES "iphone")
    install(CODE "execute_process(COMMAND ln -sf Versions/Current/Modules \"${CMAKE_INSTALL_PREFIX}/PhoneKit.framework\")")
  endif()
endif()
