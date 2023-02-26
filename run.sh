#!/bin/sh

maybe_err()
{
    if [ $? -ne 0 ]; then
        >&2 echo "\n\n\t$1\n\n"
        exit 1
    fi
}

build_dir=build_out

mkdir -p "$build_dir"

#configure
cmake -B "$build_dir" -DTESTING=1 -DFAST_TESTS=true
maybe_err "cmake configuration failed"

# attempt to build build tests
cmake --build "$build_dir" --target BUILD_TESTS
maybe_err "build tests could not build"

# attempt to build normal tests
cmake --build "$build_dir" --target TESTS
maybe_err "tests could not build"

# run the tests
ctest --test-dir "$build_dir" --verbose
maybe_err "tests failed to run"
