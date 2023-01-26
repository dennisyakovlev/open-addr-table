find_package(Threads REQUIRED)

set(tests_include ${CMAKE_CURRENT_SOURCE_DIR} CACHE FILEPATH "testing lib include dir")

add_subdirectory(tests_support)
add_subdirectory(tests_impl)
