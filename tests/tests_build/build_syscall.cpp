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
    auto res1 = mmap(nullptr, 0, PROT_READ | PROT_WRITE, MAP_SHARED, 0, 0);
    auto res2 = munmap(nullptr, 0);
    auto res3 = msync(nullptr, 0, MS_SYNC);
    auto res4 = mremap(nullptr, 0, 0, 0);

    auto res5 = access("", F_OK);
    auto res6 = open("", O_RDWR, 0);
    auto res7 = close(0);
    auto res8 = ftruncate64(0, 0);

    auto res9 = sysconf(-1);

    if (reinterpret_cast<void*>(&res1) ||
        reinterpret_cast<void*>(&res2) ||
        reinterpret_cast<void*>(&res3) ||
        reinterpret_cast<void*>(&res4) ||
        reinterpret_cast<void*>(&res5) ||
        reinterpret_cast<void*>(&res6) ||
        reinterpret_cast<void*>(&res7) ||
        reinterpret_cast<void*>(&res8) ||
        reinterpret_cast<void*>(&res9))
    {
        return;
    }
}
