set(sources
    test_exclusion.cpp
    test_fairness.cpp
    main.cpp
)

add_executable(${new_lock_tests_exe} ${sources})

target_link_libraries(${new_lock_tests_exe} gtest Threads::Threads)
target_include_directories(${new_lock_tests_exe} PRIVATE ${include})
target_include_directories(${new_lock_tests_exe} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../locks)
