set(sources
    # test_basefile.cpp
    # test_files.cpp
    # test_lock_req.cpp
    # test_recursive_lock.cpp
    # test_umaplru.cpp
    # test_unordered_map_req.cpp
    test_unordered_map_extra.cpp
    main.cpp
)

set(unit_sources
    unit_tests/Vars.h
    unit_tests/CustomString.h
    unit_tests/Funcs.h
    unit_tests/Funcs.cpp
    unit_tests/SpecialHash.h
    unit_tests/SpecialHash.cpp
)

add_executable(${unit_tests_exe} ${sources} ${unit_sources})

target_link_libraries(${unit_tests_exe} gtest Threads::Threads)
target_include_directories(${unit_tests_exe} PRIVATE ${include})
target_include_directories(${unit_tests_exe} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../unit)
