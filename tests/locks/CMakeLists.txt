set(sources
    test_exclusion.cpp
    test_fairness.cpp
    main.cpp
)

add_executable(${lock_tests_exe} ${sources})

target_link_libraries(${lock_tests_exe} gtest Threads::Threads)
target_include_directories(${lock_tests_exe} PRIVATE ${include})
target_include_directories(${lock_tests_exe} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../locks)
