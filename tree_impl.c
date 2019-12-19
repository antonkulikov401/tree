#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include "tree_impl.h"

ssize_t get_dir_size(const char* path) {
	ssize_t count = 0;
	struct dirent* dp;
	DIR* dir = opendir(path);
	if (dir == NULL) return -1;
	while ((dp = readdir(dir)) != NULL)
		if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, ".."))
			++count;
	closedir(dir);
	return count;
}

file_type get_type(struct stat st) {
	if (S_ISDIR(st.st_mode) &&
		st.st_mode & S_ISVTX &&
		st.st_mode & S_IWOTH) return sticky_ow;
	if (S_ISDIR(st.st_mode) && st.st_mode & S_ISVTX) return sticky;
	if (S_ISDIR(st.st_mode) && st.st_mode & S_IWOTH) return ow;
	if (S_ISDIR(st.st_mode)) return dir;
	if (S_ISLNK(st.st_mode)) return lnk;
	if (S_ISFIFO(st.st_mode)) return pi;
	if (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)) return blk;
	if (S_ISSOCK(st.st_mode)) return sock;
	if (st.st_mode & S_IXUSR) return exec;
	return reg;
}

int is_dir(file_type type) {
	return type == dir || type == sticky_ow || type == sticky || type == ow;
}

void update_dir_info(dir_info** info, file_type type) {
	if (is_dir(type)) (*info)->dir_count++;
	else (*info)->file_count++;
}

ssize_t get_dir_content(const char* path, file_info** files) {
	ssize_t dir_size = get_dir_size(path);
	if (dir_size == -1) return -1;
	DIR* dir = opendir(path);
	if (dir == NULL) return -1;
	struct dirent* dp = NULL;
	*files = (file_info*)malloc(dir_size * sizeof(file_info));
	if (*files == NULL) return -1;
	for(int i = 0; (dp = readdir(dir)) != NULL; ++i) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
			--i;
			continue;
		}
		char file_path[PATH_MAX];
		sprintf(file_path, "%s/%s", path, dp->d_name);
		strcpy((*files)[i].name, dp->d_name);
		struct stat st;
		int err = lstat(file_path, &st);
		(*files)[i].type = get_type(st);
		if (err == -1) return -1;
	}
	qsort(*files, dir_size, sizeof(file_info),
		(int(*)(const void*, const void*))strcoll);
	closedir(dir);
	return dir_size;
}
