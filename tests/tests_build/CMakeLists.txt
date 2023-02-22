set(sources
    build_asm_pause.cpp
    build_libcall.cpp
    build_syscall.cpp
    build_pthread.cpp
)

message(VERBOSE "adding build tests")

add_executable(${build_tests} main.cpp ${sources})
target_link_libraries(${build_tests} Threads::Threads)
target_include_directories(${build_tests} PUBLIC ${include} ${CMAKE_CURRENT_SOURCE_DIR}/..)
