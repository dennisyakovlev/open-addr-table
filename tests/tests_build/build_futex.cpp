#include <cstdint>
#include <linux/futex.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <tests_build/Funcs.h>

void
build_futex()
{    
    uint32_t n = 0; 
    syscall(SYS_futex, reinterpret_cast<uint32_t*>(&n), FUTEX_WAIT, n, NULL, 0, 0);
    syscall(SYS_futex, reinterpret_cast<uint32_t*>(&n), FUTEX_WAKE, INT32_MAX, NULL, 0, 0);
}
