# Thourough Tests

More robust checking compared to unit tests. Tests failing in this directory will be harder or not possible to debug. 

If a test(s) fails, you can attempt to debug it normally. Otherwise look at the printed input to the test and manually rewrite test in another file then debug there. To do this follow

* switch to top level directory that contains LICENSE
* create a file, say test.cpp
* execute `./run.sh` to build necessary libraries
* build target file with `g++ -Icode/include -Itests test.cpp -Lbuild_out/tests/tests_support -ltests_support` or your compilers equivalent command.
    * -I is include path
    * -L is linker search path
    * -l is library to link with. note the actual name is libtests_support.a

## test_linear_probe.cpp

Closer test of insert, erase, and find operations on the linear probing algorithms.

## test_permutations.cpp

Like test_linear_probe, but will permutate **one** of insert or erase order so that all possible combination are tested.

## test_rehash.cpp

When the container is rehashed elements will move be moved around. Check to see when elements are moved around that they still exist.
