set(sources
    # test_linear_probe.cpp
    test_attempt.cpp
    main.cpp
)

add_executable(${thourough_tests_exe} ${sources})

target_link_libraries(${thourough_tests_exe} tests_support Threads::Threads gtest)
