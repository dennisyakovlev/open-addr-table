#include <stdio.h>

#include <ftw.h>

#include <tests_build/Funcs.h>

void
build_libcall()
{
    remove("");

    nftw("", nullptr, 0, 0);
}
