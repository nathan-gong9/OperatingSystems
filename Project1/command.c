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
	write(global_file, cwd, strlen(cwd));
	
	DIR *dir;
	dir = opendir(cwd);
	struct dirent *read_dir;
	
	write(global_file, "\n", strlen("\n"));
	while((read_dir = readdir(dir)) != NULL){
			write(global_file, read_dir->d_name, strlen(read_dir->d_name));
			write(global_file, " ", strlen(" "));
	}
	write(global_file, "\n", strlen("\n"));
	closedir(dir);
}

void showCurrentDir(){ /*for the pwd command*/
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	write(global_file, "\n", strlen("\n"));
	write(global_file, cwd, strlen(cwd));
	write(global_file, "\n", strlen("\n"));
}

void makeDir(char *dirName){ /*for the mkdir command*/
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	int mkdirCheck = 0;

	strcat(cwd, "/");
	strcat(cwd, dirName);
	
	DIR *dir;
	dir = opendir(cwd);
	struct dirent *read_dir;
	
	while((read_dir = readdir(dir)) != NULL){
		if(strcmp(read_dir->d_name, dirName) == 0)
			mkdirCheck = 1;
	}
	closedir(dir);
	if(mkdirCheck == 0){
		write(global_file, "\n", sizeof("\n"));
		write(global_file, cwd, strlen(cwd));
		write(global_file, "\n", sizeof("\n"));
		mkdir(cwd, 0755);
	}
	else{
		write(global_file, "\n", sizeof("\n"));
		write(global_file, "Directory already exists!", strlen("Directory already exists!"));
		write(global_file, "\n", sizeof("\n"));
	}
}

void changeDir(char *dirName){ /*for the cd command*/
	if (chdir(dirName) == 0) {
        write(global_file, dirName, strlen(dirName));
    } else {
    	char* error = "Error! Directory does not exist\n";
        write(global_file, error, strlen(error));
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
        return;
    }

    int dest_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1) {
        char* error = "Error opening file\n";
        write(global_file, error, strlen(error));
        return;
    }

    char buffer[1024];
    ssize_t bytes_read, bytes_written;

    while ((bytes_read = read(source_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written == -1) {
            char* error = "Error writing to destination\n";
        	write(global_file, error, strlen(error));
        	return;
        }
    }

    if (bytes_read == -1) {
        char* error = "Error writing to destination\n";
        write(global_file, error, strlen(error));
        return;
    }

    close(source_fd);
    close(dest_fd);
}

void moveFile(char *sourcePath, char *destinationPath){ /*for the mv command*/
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));

	strcat(cwd, "/");
	printf("cwd: %s\n", cwd);
	
	char cwd2[1024];
	getcwd(cwd2, sizeof(cwd2));

	strcat(cwd2, "/");
	
	char* source;
	char* destination;
	char* fileName;
	
	source = strcat(cwd, sourcePath);
	char* substringSource = strchr(sourcePath, '/');
	if(substringSource == NULL)
		fileName = sourcePath;
	else{
		substringSource++;
		fileName = substringSource;
	}
	
		
	char character2 = '.';
	char* substringDestination = strchr(destinationPath, character2);
	
	if(substringDestination != NULL){
		destination = strcat(cwd2, destinationPath);
	}
	else
		destination = strcat(cwd2, strcat(strcat(destinationPath, "/"), fileName));

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
	printf("cwd: %s\n", cwd);
	strcat(cwd, filename);
	unlink(cwd);
}

void displayFile(char *filename){ /*for the cat command*/
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	write(global_file, cwd, strlen(cwd));
	
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
				write(global_file, "\n", sizeof("\n"));
				do {
					bytes = read(file_id, buffer, sizeof(buffer));
					write(global_file, buffer, bytes);
				} while (bytes > 0);
				write(global_file, "\n", sizeof("\n"));
				
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