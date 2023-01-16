#!/bin/bash

cd build
make
if [ $? -ne 0 ]; then
    exit 1
fi
cd -

# declare -a files=("unit/UNIT_TESTS" "locks/LOCK_TESTS")
# declare -a files=("unit/UNIT_TESTS")
declare -a files=("locks/LOCK_TESTS")

for file in "${files[@]}"; do
    build/tests/"$file"
    printf "\n"
done

ls | egrep "^.{16}$" | xargs -I{} rm {}
