INCLUDE(FetchContent)

FETCHCONTENT_DECLARE(
        fmtlib
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 9.1.0
)

FETCHCONTENT_MAKEAVAILABLE(fmtlib)

add_executable(cpp_cli main.cpp)
target_link_libraries(cpp_cli PRIVATE phone_sharedlib fmt)