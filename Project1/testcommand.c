#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>

int global_file = STDOUT_FILENO;

int testMain() {
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	write(global_file, cwd, strlen(cwd));
	
	DIR *dir;
	dir = opendir(cwd);
	struct dirent *read_dir;
	int count = 1;
	
	write(global_file, "\n", strlen("\n"));
	while((read_dir = readdir(dir)) != NULL){
		if(count != 0){
			write(global_file, read_dir->d_name, strlen(read_dir->d_name));
			write(global_file, "\n", strlen("\n"));
		}
		count++;
	}
	closedir(dir);
	
	return 0;
}
