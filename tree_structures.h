#ifndef TREE_STRUCTURES_H
#define TREE_STRUCTURES_H

#include <sys/types.h>
#include <linux/limits.h>

typedef enum file_type { 
	reg, dir, lnk, pi, exec, blk, sock, sticky_ow, sticky, ow
} file_type;

typedef struct file_info {
	char name[PATH_MAX];
	file_type type;
} file_info;

typedef struct dir_info {
	size_t dir_count;
	size_t file_count;
} dir_info;

#endif
