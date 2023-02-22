#include <tests_build/Funcs.h>

int main(int argc, char const *argv[])
{
    build_asm_pause();
    build_libcall();
    build_pthread();
    build_syscall();

    return 0;
}
