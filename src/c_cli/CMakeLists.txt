add_executable(c_cli main.c)
target_link_libraries(c_cli PRIVATE phone_sharedlib)