set(sources
    Correctness.cpp
    Correctness.h
    QueueFairness.cpp
    QueueFairness.h
    main.cpp
)

add_executable(${lock_tests_exe} ${sources})

target_link_libraries(${lock_tests_exe} Threads::Threads)
target_include_directories(${lock_tests_exe} PRIVATE ${include})
