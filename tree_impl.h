#ifndef TREE_IMPL_H
#define TREE_IMPL_H

#include "tree_structures.h"

ssize_t get_dir_content(const char* path, file_info** files);
int is_dir(file_type type);
void update_dir_info(dir_info** info, file_type type);

#endif
