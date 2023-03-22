find_package(PkgConfig REQUIRED)
pkg_check_modules(PJPROJECT REQUIRED libpjproject)

configure_file(version.cpp.in phone_version.cpp @ONLY)

add_library(phone_version STATIC ${CMAKE_CURRENT_BINARY_DIR}/phone_version.cpp)

target_include_directories(phone_version PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(phone OBJECT phone.cpp phone_instance_t.cpp include/phone/account_t.h include/phone/call_t.h)

include(GenerateExportHeader)
generate_export_header(phone BASE_NAME "phone")

target_compile_options(phone PUBLIC ${PJPROJECT_CFLAGS})

target_include_directories(phone
        PUBLIC
        include
        ${CMAKE_CURRENT_BINARY_DIR}
        PRIVATE
        include/phone)
target_link_libraries(phone PRIVATE ${PJPROJECT_LINK_LIBRARIES} phone_version)

set(phone_PUBLIC_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/phone.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/phone_instance_t.h
        ${CMAKE_CURRENT_BINARY_DIR}/phone_export.h)

add_library(phone_sharedlib SHARED)
set_target_properties(phone_sharedlib PROPERTIES
    OUTPUT_NAME "phone"
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION 0
    PUBLIC_HEADER "${phone_PUBLIC_HEADERS}")
target_link_libraries(phone_sharedlib PUBLIC phone)
