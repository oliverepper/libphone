find_package(PkgConfig REQUIRED)

pkg_check_modules(PJPROJECT REQUIRED libpjproject)

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
target_link_libraries(phone PRIVATE ${PJPROJECT_LINK_LIBRARIES} ${RESOLV_LIBRARY} stunning::stunning)

set(phone_PUBLIC_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/phone.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/phone_instance_t.h
        ${CMAKE_CURRENT_BINARY_DIR}/phone_export.h)

add_library(phone_sharedlib SHARED dummy.cpp)
set_target_properties(phone_sharedlib PROPERTIES
    OUTPUT_NAME "phone"
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION 0
    PUBLIC_HEADER "${phone_PUBLIC_HEADERS}")
target_link_libraries(phone_sharedlib PUBLIC phone phone_version)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/phone DESTINATION include)