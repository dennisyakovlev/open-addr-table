# Examples

Some examples of usage. Some are obvious, some aren't.

## array_like

Since the container uses open addressing it can be used as array. Store words in file then load the words again. Running should be something like
```
./array_like write   # paragraph.txt appears
./array_like read    # read data from paragraph.txt
```

## std_vector

The mmap allocator can be used with a vector. This is contrived, since vector behaviour can be simulated (see array_like).

Can view the files on disk using `ls` to figure out what the corresponding files are then using `od` command.

## two_sum

Leetcode's [two sum problem](https://leetcode.com/problems/two-sum/) done using this unordered map. 
