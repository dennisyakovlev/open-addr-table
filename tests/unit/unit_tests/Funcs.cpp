#include <stdio.h>
#include <ftw.h>

#include <unit_tests/Funcs.h>

int
unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = remove(fpath);

    if (rv)
        perror(fpath);

    return rv;
}

int
RemoveCreateDir(const char* path)
{
    int res = 0;
    if (nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS))
    {
        perror("ftw");
        res = -1;
    }

    if (mkdir(path, S_IRWXU | S_IRUSR | S_IWUSR))
    {
        perror("mkdir");
        res = -1;
    }

    return res;
}
