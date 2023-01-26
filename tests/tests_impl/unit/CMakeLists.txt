set(sources
    # test_lock_req.cpp
    # test_recursive_lock.cpp
    # test_umaplru.cpp
    test_unordered_map_req.cpp
    main.cpp
)

add_executable(${unit_tests_exe} ${sources})

target_link_libraries(${unit_tests_exe} gtest Threads::Threads libtests)
target_include_directories(${unit_tests_exe} PRIVATE ${include})
