# Thourough Tests

More robust checking compared to unit tests. Tests failing in this directory will be harder or not possible to debug. If a test(s) fails, look at the printed input to the test and manually rewrite test in another file inside say main function. Then debug there.

## test_linear_probe

Closer test of insert, erase, and find operations on the linear probing algorithms.

## test_permutations

Like test_linear_probe, but will permutate **one** of insert or erase order so that all possible combination are tested.
