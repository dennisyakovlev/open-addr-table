#!/bin/bash

cd build
make
if [ $? -ne 0 ]; then
    exit 1
fi
cd -

files=(unit/UNIT_TESTS locks/LOCK_TESTS thourough/THOUROUGH_TESTS)

for file in "${files[@]}"; do
    build/tests/"$file"
    printf "\n"
done

ls | egrep "^.{16}$" | xargs -I{} rm {}
