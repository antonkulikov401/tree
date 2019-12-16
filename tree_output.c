#include <sys/stat.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include "tree_impl.h"
#include "tree_output.h"

void set_style(file_type type) {
	if (type == dir) printf("\033[01;34m");
	if (type == lnk) printf("\033[01;36m");
	if (type == pi) printf("\033[40;33m");
	if (type == exec) printf("\033[01;32m");
	if (type == blk) printf("\033[40;33;01m");
	if (type == sock) printf("\033[01;35m");
	if (type == sticky_ow) printf("\033[30;42m");
	if (type == ow) printf("\033[34;42m");
	if (type == sticky) printf("\033[37;44m");
}

void reset_style(file_type type) {
	if (type != reg) printf("\033[00m");
}

void resolve_link(const char* path, file_info* file, dir_info** info) {
	char link[PATH_MAX];
	char buff[PATH_MAX];
	memset(buff, 0, PATH_MAX);
	strcpy(buff, path);
	strcat(buff, "/");
	strcat(buff, file->name);
	ssize_t len = readlink(buff, link, PATH_MAX);
	struct stat st;
	stat(buff, &st);
	set_style(get_type(st));
	printf("%.*s", (int)len, link);
	reset_style(get_type(st));
	update_dir_info(info, get_type(st));
}

void print_file(const char* path, file_info* file, const char* prefix,
	const char* pipe, dir_info** info) {

	printf("%s%s", prefix, pipe);
	set_style(file->type);
	printf("%s", file->name);
	reset_style(file->type);
	if (file->type == lnk) {
		printf(" -> ");
		resolve_link(path, file, info);
	}
	else update_dir_info(info, file->type);
	if (!is_dir(file->type)) printf("\n");
}

void list_dir(const char* path, char* prefix, dir_info* info) {
	file_info* files;
	ssize_t dir_size = get_dir_content(path, &files);
	if (dir_size == -1) {
		printf(" [error opening dir]\n");
		return;
	}
	printf("\n");
	char* pipe = "├── ";
	char* prefix_extension = "│   ";
	for (size_t i = 0; i < dir_size; ++i) {
		if (i == dir_size - 1) {
			pipe = "└── ";
			prefix_extension = "    ";
		}
		print_file(path, files + i, prefix, pipe, &info);
		if (is_dir(files[i].type)) {
			char new_path[PATH_MAX];
			strcpy(new_path, path);
			strcat(new_path, "/");
			strcat(new_path, files[i].name);
			char new_prefix[PATH_MAX];
			strcpy(new_prefix, prefix);
			strcat(new_prefix, prefix_extension);
			list_dir(new_path, new_prefix, info);
		}
	}
	free(files);
}

void print_tree(const char* path, dir_info* info) {
	struct stat st;
	lstat(path, &st);
	set_style(get_type(st));
	printf("%s", path);
	reset_style(get_type(st));
	list_dir(path, "", info);
}
