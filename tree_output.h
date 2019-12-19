#ifndef TREE_OUTPUT_H
#define TREE_OUTPUT_H

#include <unistd.h>
#include <sys/stat.h>
#include "tree_structures.h"

void print_tree(const char* path, dir_info* info);
file_type get_type(struct stat st);

#endif
