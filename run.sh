#!/bin/sh

build_dir=build_out

mkdir -p "$build_dir"

#configure
cmake -B "$build_dir" -DTESTING=1
if [ $? -ne 0 ]; then
    >&2 echo "cmake configuration failed"
    exit 1
fi

# attempt to build build tests
cmake --build "$build_dir" --target BUILD_TESTS
if [ $? -ne 0 ]; then
    >&2 echo "build tests could not build"
    exit 1
fi

# attempt to build normal tests
cmake --build "$build_dir" --target TESTS
if [ $? -ne 0 ]; then
    >&2 echo "tests could not build"
    exit 1
fi

ctest --test-dir "$build_dir" --verbose
if [ $? -ne 0 ]; then
    >&2 echo "tests failed to run"
    exit 1
fi

cd "$build_dir"
ls | egrep "^.{16}$" | xargs -I{} rm {}
cd -
