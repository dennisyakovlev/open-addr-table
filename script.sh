#!/bin/sh

if [ "$1" -eq  "1" ]; then
    sed -i "s/FAST_TESTS \(true\|false\)/FAST_TESTS true/" config.h
else
    sed -i "s/FAST_TESTS \(true\|false\)/FAST_TESTS false/" config.h
fi

