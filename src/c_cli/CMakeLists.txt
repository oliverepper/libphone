if(BUILD_APPS)
    add_executable(c_cli main.c helper.h helper.c)
    target_link_libraries(c_cli PRIVATE phone_sharedlib)
endif ()
