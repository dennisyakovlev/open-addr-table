set(lib_sources
    SpecialHash.cpp
)

add_library(tests_support STATIC
    ${lib_sources}
)

target_include_directories(tests_support PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${include})
target_link_libraries(tests_support Threads::Threads gtest)

