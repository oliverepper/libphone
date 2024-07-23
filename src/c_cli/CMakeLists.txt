if(BUILD_APPS)
    add_executable(c_cli main.c helper.h)
    target_link_libraries(c_cli PRIVATE phone_sharedlib)
endif ()