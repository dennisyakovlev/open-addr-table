set(sources
    # test_lock_req.cpp
    # test_recursive_lock.cpp
    # test_umaplru.cpp
    test_unordered_map_req.cpp
    main.cpp
)

add_executable(${unit_tests_exe} ${sources})

target_link_libraries(${unit_tests_exe} tests_support Threads::Threads gtest)
