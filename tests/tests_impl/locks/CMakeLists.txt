set(sources
    test_exclusion.cpp
    test_fairness.cpp
    main.cpp
)

add_executable(${lock_tests_exe} ${sources})

target_link_libraries(${lock_tests_exe} tests_support Threads::Threads gtest)
