set(lib_sources
    CustomString.h
    Funcs.cpp
    Funcs.h
    SpecialHash.cpp
    SpecialHash.h
    Vars.h
)

add_library(libtests STATIC
    ${lib_sources}
)

target_include_directories(libtests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
