#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "string_parser.h"
#include "command.h"

int global_file;

bool executeLine(char* line);

int main(int argc, char * argv[]){
	
	//What to do if we execute the pseudo-shell in interactive mode
	if (argc == 1){
		global_file = STDOUT_FILENO;
	
        char *line = NULL;
        line = malloc(1024 * sizeof(char));
        
        size_t len = 0;
        ssize_t nread;
        
        printf(">>> ");
        while ((nread = getline(&line, &len, stdin)) != -1) {
            if(executeLine(line)){
            	break;
            }
        	printf(">>> ");
        	
        }
        free(line);
        exit(EXIT_SUCCESS);
	}
	
	//What to do if we're executing the pseudo-shell in file mode
	else if (argc == 3){
		if(strcmp(argv[1], "-f") == 0){
			
			//Read the file line by line
			FILE *stream;
        	char *line = NULL;
        	line = malloc(1024 * sizeof(char));
        	size_t len = 0;
        	ssize_t nread;

        	if (argc != 3) {
            	fprintf(stderr, "Usage: %s <file>\n", argv[0]);
            	exit(EXIT_FAILURE);
        	}

        	stream = fopen(argv[2], "r");
        	if (stream == NULL) {
            	perror("fopen");
            	exit(EXIT_FAILURE);
        	}
		
    		global_file = open("testoutput.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

        	while ((nread = getline(&line, &len, stream)) != -1) {
        		if(executeLine(line)){
            		break;
            	}
        	}

        	free(line);
        	fclose(stream);
        	close(global_file);
        	exit(EXIT_SUCCESS);
		}
		else{
			exit(EXIT_FAILURE);
		}
	}
}

bool executeLine(char *line){
	//Go through the commands on the line by using the delimiters
    command_line commands = str_filler(line, ";");
    int count = 0;
    bool do_exit = false;
    
    while (count < commands.num_token - 1) {
    	char cmd[256];
    	strcpy(cmd, commands.command_list[count]);
    	char command[256];
    	char arg1[256];
    	char arg2[256];
    	char dummy[256];
    	
    	int argCount = sscanf(cmd, "%s %s %s %s", command, arg1, arg2, dummy);
    	
    	char parameterMessage[256], unrecognizedMessage[256];
    	strcpy(parameterMessage, "Error! Unsupported parameter for command: ");
    	strcpy(unrecognizedMessage, "Error! Unrecognized command: ");
    	strcat(parameterMessage, command);
    	strcat(unrecognizedMessage, command);
    	
    	
        //identify the command and what to do with it.
        if (strcmp(command, "ls") == 0) {
        	if(argCount == 1)
        		listDir();
        	else{
        		write(global_file, parameterMessage, strlen(parameterMessage));
        		write(global_file, "\n", sizeof("\n"));
        	}
    	} else if (strcmp(command, "pwd") == 0) {
    		if(argCount == 1)
    			showCurrentDir();
    		else{
        		write(global_file, parameterMessage, strlen(parameterMessage));
        		write(global_file, "\n", sizeof("\n"));
        	}
    	} else if (strcmp(command, "mkdir") == 0) {
    		if(argCount == 2)
    			makeDir(arg1);
    		else{
        		write(global_file, parameterMessage, strlen(parameterMessage));
        		write(global_file, "\n", sizeof("\n"));
        	}
    	} else if (strcmp(command, "cd") == 0) {
    		if(argCount == 2)
    			changeDir(arg1);
    		else{
        		write(global_file, parameterMessage, strlen(parameterMessage));
        		write(global_file, "\n", sizeof("\n"));
        	}
    	} else if (strcmp(command, "cp") == 0) {
    		if(argCount == 3)
        		copyFile(arg1, arg2);
        	else{
        		write(global_file, parameterMessage, strlen(parameterMessage));
        		write(global_file, "\n", sizeof("\n"));
        	}
    	} else if (strcmp(command, "mv") == 0) {
    		if(argCount == 3)
        		moveFile(arg1, arg2);
        	else{
        		write(global_file, parameterMessage, strlen(parameterMessage));
        		write(global_file, "\n", sizeof("\n"));
        	}
    	} else if (strcmp(command, "rm") == 0) {
    		if(argCount == 2)
        		deleteFile(arg1);
        	else{
        		write(global_file, parameterMessage, strlen(parameterMessage));
        		write(global_file, "\n", sizeof("\n"));
        	}
    	} else if (strcmp(command, "cat") == 0) {
    		if(argCount == 2)
        		displayFile(arg1);
        	else{
        		write(global_file, parameterMessage, strlen(parameterMessage));
        		write(global_file, "\n", sizeof("\n"));
        	}
        } else if(strcmp(command, "exit") == 0) {
        	if(argCount == 1){
        		do_exit = true;
        		break;
        	}
        	else{
        		write(global_file, parameterMessage, strlen(parameterMessage));
        		write(global_file, "\n", sizeof("\n"));
        	}
    	} else {
        	write(global_file, unrecognizedMessage, strlen(unrecognizedMessage));
        	write(global_file, "\n", sizeof("\n"));
    	}
        	
        //iterate through command_list	
        count++;
    }
    
    free_command_line(&commands);
    return do_exit;
}

