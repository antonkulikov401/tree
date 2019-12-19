#include <locale.h>
#include <unistd.h>
#include <stdio.h>
#include "tree_output.h"

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
