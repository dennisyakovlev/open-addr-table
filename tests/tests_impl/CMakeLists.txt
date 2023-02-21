set(sources
    ${CMAKE_CURRENT_SOURCE_DIR}/locks/test_acquire.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/locks/test_deviation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/locks/test_exclusion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/locks/test_recursive_lock.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thourough/test_linear_probe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thourough/test_permutations.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thourough/test_rehash.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_block.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_unordered_map_req.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_umaplru.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/test_iterator.cpp
)

add_executable(${tests} ${sources} main.cpp)

target_link_libraries(${tests} Threads::Threads gtest tests_support)
