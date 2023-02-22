#include <stdio.h>

#include <ftw.h>

#include <tests_build/Funcs.h>

int
f(const char *a, const struct stat *b, int c, struct FTW *d)
{
    return -1;    
}

void
build_libcall()
{
    remove("");

    nftw("", f, 0, 0);
}
