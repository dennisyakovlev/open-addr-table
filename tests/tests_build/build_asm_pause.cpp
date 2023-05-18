#include <files/spin_lock.h>

#include <tests_build/Funcs.h>

void
build_asm_pause()
{    
    SPIN_PAUSE();
}
