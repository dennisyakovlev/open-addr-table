set(sources
    test_linear_probe.cpp
    main.cpp
)

add_executable(${thourough_tests_exe} ${sources})

target_link_libraries(${thourough_tests_exe} gtest Threads::Threads libtests)
target_include_directories(${thourough_tests_exe} PRIVATE ${include})
