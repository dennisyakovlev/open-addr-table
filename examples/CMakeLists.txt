add_executable(std_vector
    std_vector/main.cpp)
target_include_directories(std_vector PRIVATE ${include})

add_executable(two_sum
    two_sum/main.cpp)
target_include_directories(two_sum PRIVATE ${include})

add_executable(array_like
    array_like/main.cpp)
# target_include_directories(array_like PRIVATE ${include})
target_link_libraries(array_like tests_support)
