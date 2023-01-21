#ifndef UNIT_TESTS_FUNCS
#define UNIT_TESTS_FUNCS

/**
 * @brief Remove file or directory and all contents.
 * 
 * @param path path to remove
 * @return int 0 for success, non-zero on failure
 */
int
RemoveRecursive(const char* path);

#endif
