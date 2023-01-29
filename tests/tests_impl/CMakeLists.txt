set(sources
    ${CMAKE_CURRENT_SOURCE_DIR}/thourough/test_linear_probe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thourough/test_attempt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_lock_req.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_unordered_map_req.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_recursive_lock.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_umaplru.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/locks/test_exclusion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/locks/test_fairness.cpp
)

add_executable(${tests} ${sources} main.cpp)

target_link_libraries(${tests} Threads::Threads gtest tests_support)
