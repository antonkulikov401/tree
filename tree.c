#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <libgen.h>
#include <fcntl.h>
#include <locale.h>

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

void update_dir_info(dir_info** info, file_type type) {
	if (is_dir(type)) (*info)->dir_count++;
	else (*info)->file_count++;
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

int main(int argc, char* argv[]) {
	setlocale(LC_COLLATE, "");
	dir_info info = { 0, 0 };
	if (argc == 1) {
		char dir[PATH_MAX];
		getcwd(dir, PATH_MAX);
		print_tree(dir, &info);
	}
	else {
		for (int i = 1; i < argc; ++i)
			print_tree(argv[i], &info);
	}
	printf("\n%ld director%s, %ld file%s\n", info.dir_count,
		info.dir_count == 1 ? "y" : "ies", info.file_count,
		info.file_count == 1 ? "" : "s");
	return 0;
}
