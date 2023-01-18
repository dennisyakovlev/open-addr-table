set(sources
    main.cpp
)

add_executable(${thourough_tests_exe} ${sources})

target_link_libraries(${thourough_tests_exe} gtest Threads::Threads)
target_include_directories(${thourough_tests_exe} PRIVATE ${include})
target_include_directories(${thourough_tests_exe} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../thourough)
