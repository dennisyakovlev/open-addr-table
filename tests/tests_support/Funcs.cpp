#include <stdio.h>
#include <ftw.h>

#include <tests_support/Funcs.h>

int
unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = remove(fpath);

    if (rv)
    {
        perror(fpath);
    }

    return rv;
}

int
RemoveRecursive(const char* path)
{
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}
