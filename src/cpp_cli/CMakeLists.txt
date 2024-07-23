if(BUILD_APPS)
    add_executable(cpp_cli main.cpp)
    target_link_libraries(cpp_cli PRIVATE phone_sharedlib pthread)
endif ()