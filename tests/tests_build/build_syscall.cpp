#include <sys/mman.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <unistd.h>

#include <tests_build/Funcs.h>

void
build_syscall()
{
    mmap(nullptr, 0, PROT_READ | PROT_WRITE, MAP_SHARED, 0, 0);
    munmap(nullptr, 0);
    msync(nullptr, 0, MS_SYNC);
    mremap(nullptr, 0, 0, 0);

    access("", F_OK);
    open("", O_RDWR, 0);
    close(0);
    ftruncate64(0, 0);

    sysconf(-1);
}
