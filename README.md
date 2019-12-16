# tree

Analogue of `tree -a` linux command.

### Compilation
```bash
gcc tree.c tree_impl.c tree_output.c -o tree # program
g++ test.cpp -o autotest -std=c++17 -lstdc++fs # autotest
```

### Description
```shell
foo@bar:~$ ./tree sample_dir
sample_dir
├── file1
├── file2
├── .hidden_file
└── subdirectory
    └── file3

1 directory, 4 files
```
The program takes arbitrary number of arguments and has no additional options. Given no arguments, `tree` recursively lists all files and subdirectories in the current directory. If arguments are provided, `tree` lists content of the given directories in sequence. After listing, `tree` prints the total number of found files and directories.