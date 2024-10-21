#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>

extern int global_file;

void listDir(){ /*for the ls command*/
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	
	DIR *dir;
	dir = opendir(cwd);
	struct dirent *read_dir;
	int num_objects = 0;
	
	while((read_dir = readdir(dir)) != NULL){
			num_objects++;
	}
	
	rewinddir(dir);
	
	if(num_objects > 2){
		while((read_dir = readdir(dir)) != NULL){
			write(global_file, read_dir->d_name, strlen(read_dir->d_name));
			write(global_file, " ", strlen(" "));
		}
		write(global_file, "\n", strlen("\n"));
	}
	else{
		return;
	}
	
	closedir(dir);
}

void showCurrentDir(){ /*for the pwd command*/
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	write(global_file, cwd, strlen(cwd));
	write(global_file, "\n", strlen("\n"));
}

void makeDir(char *dirName){ /*for the mkdir command*/
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	
	char newDir[1024];
	getcwd(newDir, sizeof(newDir));
	int mkdirCheck = 0;

	strcat(newDir, "/");
	strcat(newDir, dirName);
	
	DIR *dir;
	dir = opendir(cwd);
	struct dirent *read_dir;
	
	while((read_dir = readdir(dir)) != NULL){
		if(strcmp(read_dir->d_name, dirName) == 0)
			mkdirCheck = 1;
	}
	closedir(dir);
	if(mkdirCheck == 0){
		mkdir(newDir, 0755);
	}
	else{
		write(global_file, "Directory already exists!", strlen("Directory already exists!"));
		write(global_file, "\n", sizeof("\n"));
	}
}

void changeDir(char *dirName){ /*for the cd command*/
	if (chdir(dirName) != 0) {
    	char* error = "Error! Directory does not exist\n";
        write(global_file, error, strlen(error));
        write(global_file, "\n", sizeof("\n"));
        return;
    }
}

void copyFile(char *sourcePath, char *destinationPath){ /*for the cp command*/ 
    char source[1024];
	getcwd(source, sizeof(source));

	strcat(source, "/");
	strcat(source, sourcePath);
    
    char destination[1024];
    getcwd(destination, sizeof(destination));
    strcat(destination,"/");
    strcat(destination, destinationPath);
    
    int source_fd = open(source, O_RDONLY);
    if (source_fd == -1) {
        char* error = "File does not exist\n";
        write(global_file, error, strlen(error));
        write(global_file, "\n", sizeof("\n"));
        return;
    }

	struct stat file_stat;

    if (stat(destination, &file_stat) == 0) {
    	if(S_ISDIR(file_stat.st_mode)){
			char* fileName;
			char* substringSource = strchr(sourcePath, '/');
			if(substringSource == NULL)
				fileName = sourcePath;
			else{
				substringSource++;
				fileName = substringSource;
			}
			strcat(destination, "/");
			strcat(destination, fileName);
		}
    }
	
    int dest_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1) {
        char* error = "Error opening file\n";
        write(global_file, error, strlen(error));
        write(global_file, "\n", sizeof("\n"));
        return;
    }

    char buffer[1024];
    ssize_t bytes_read, bytes_written;

    while ((bytes_read = read(source_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written == -1) {
            char* error = "Error writing to destination\n";
        	write(global_file, error, strlen(error));
        	write(global_file, "\n", sizeof("\n"));
        	return;
        }
    }

    if (bytes_read == -1) {
        char* error = "Error writing to destination\n";
        write(global_file, error, strlen(error));
        write(global_file, "\n", sizeof("\n"));
        return;
    }

    close(source_fd);
    close(dest_fd);
}

void moveFile(char *sourcePath, char *destinationPath){ /*for the mv command*/
	char source[1024];
	getcwd(source, sizeof(source));

	strcat(source, "/");
	strcat(source, sourcePath);
    
    char destination[1024];
    getcwd(destination, sizeof(destination));
    strcat(destination,"/");
    strcat(destination, destinationPath);
    
    int source_fd = open(source, O_RDONLY);
    if (source_fd == -1) {
        char* error = "File does not exist\n";
        write(global_file, error, strlen(error));
        write(global_file, "\n", sizeof("\n"));
        return;
    }

	struct stat file_stat;

    if (stat(destination, &file_stat) == 0) {
    	if(S_ISDIR(file_stat.st_mode)){
			char* fileName;
			char* substringSource = strchr(sourcePath, '/');
			if(substringSource == NULL)
				fileName = sourcePath;
			else{
				substringSource++;
				fileName = substringSource;
			}
			strcat(destination, "/");
			strcat(destination, fileName);
		}
    }

	if (rename(source, destination) == -1) {
		char* error = "Source or Destination does not exist";
        write(global_file, error, strlen(error));
        return;
    }
}

void deleteFile(char *filename){ /*for the rm command*/
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));

	strcat(cwd, "/");
	strcat(cwd, filename);
	
	struct stat fileStat;
	if(stat(cwd, &fileStat) == 0){
		if(S_ISDIR(fileStat.st_mode)){
			char error[256];
			strcpy(error, "rm: cannot remove a directory: '");
			strcat(error, filename);
			strcat(error, "'\n");
			write(global_file, error, strlen(error));
		}
		else
			unlink(cwd);
	}
	else{
		char error[256];
		strcpy(error, "rm: cannot remove: '");
		strcat(error, filename);
		strcat(error, "': No such file or directory\n");
		write(global_file, error, strlen(error));
	}
}

void displayFile(char *filename){ /*for the cat command*/
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	
	DIR *dir;
	dir = opendir(cwd);
	struct dirent *read_dir;
	int count = 1;
	int did_open = 0;
	char buffer[1024];
	size_t bytes;
	
	//iterate through directory
	while((read_dir = readdir(dir)) != NULL){
		if(count != 1){
			
			if(strcmp(read_dir->d_name, filename) == 0){
				int file_id = open(read_dir->d_name, O_RDONLY);
				do {
					bytes = read(file_id, buffer, sizeof(buffer));
					write(global_file, buffer, bytes);
				} while (bytes > 0);
				//write(global_file, "\n", sizeof("\n"));
				
				close(file_id);
				did_open = 1;
			}
			
		}
		count++;
	}
	
	//print error message if no file exists in the directory
	if(did_open == 0){
		write(global_file, "\n", sizeof("\n"));
		char* error = "Error! File doesn't exist";
		write(global_file, error, strlen(error));
		return;
	}
	
	closedir(dir);
}