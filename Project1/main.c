#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "string_parser.h"
#include "command.h"

int global_file;

void executeLine(char* line);

int main(int argc, char * argv[]){
	printf("RUNNING");
	
	//What to do if we're executing the pseudo-shell in file mode
	if(strcmp(argv[1], "-f") == 0 && argc == 3){
			
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
            fwrite(line, nread, 1, stdout);
            executeLine(line);
            line = NULL;
        	line = malloc(1024 * sizeof(char));
        }

        free(line);
        fclose(stream);
        close(global_file);
        exit(EXIT_SUCCESS);
	}
	
	//What to do if we execute the pseudo-shell in interactive mode
	else if (argc == 1){
		printf("INTERACTIVE MODE");
		global_file = STDOUT_FILENO;
	
        char *line = NULL;
        
        size_t len = 0;
        ssize_t nread;
        
        printf(">>>");
        while ((nread = getline(&line, &len, stdin)) != -1) {
            fwrite(line, nread, 1, stdout);
            executeLine(line);
            line = NULL;
        	line = malloc(1024 * sizeof(char));
        	printf(">>>");
        	
        }
        free(line);
        exit(EXIT_SUCCESS);
	}
}

void executeLine(char *line){
	//Go through the commands on the line by using the delimiters
    command_line commands = str_filler(line, " ; ");
    int count = 0;
    char* cmd = commands.command_list[count];
    commands.num_token = count_token(line, " : ");
    if(commands.num_token == 0){
    	commands  = str_filler(line, ";");
    	cmd = commands.command_list[count];
    }
    count++;
    
    char *command = NULL;
    char *arg1 = NULL;
    char *arg2 = NULL; 
    char *dummy = NULL;

    command = malloc(100 * sizeof(char));
    arg1 = malloc(100 * sizeof(char));
    arg2 = malloc(100 * sizeof(char));
    dummy = malloc(100 * sizeof(char));
    
 
    while (count < commands.num_token) {  	
    	int argCount = sscanf(cmd, "%s %s %s %s", command, arg1, arg2, dummy);
    	char parameterMessage[] = "Error! Unsupported parameters for command: ";
    	char unrecognizedMessage[] = "Error! Unrecognized command: ";
    	char argumentMessage[] = "Too many parameters: ";
    	strcat(parameterMessage, command);
    	strcat(unrecognizedMessage, command);
    	strcat(argumentMessage, command);
    	
    	if(argCount > 3){
    		write(global_file, argumentMessage, strlen(argumentMessage));
    		break;
    	}
    	
        //identify the command and what to do with it.
        if (strcmp(command, "ls") == 0) {
        	if(arg1 != NULL){
        		write(global_file, parameterMessage, strlen(parameterMessage));
        	}
        	else
        		listDir();
    	} else if (strcmp(command, "pwd") == 0) {
    		if(arg1 != NULL)
    			write(global_file, parameterMessage, strlen(parameterMessage));
    		else
        		showCurrentDir();
    	} else if (strcmp(command, "mkdir") == 0) {
    		if(arg1 != NULL && arg2 == NULL)
    			makeDir(arg1);
    		else
    			write(global_file, parameterMessage, strlen(parameterMessage));
    	} else if (strcmp(command, "cd") == 0) {
    		if(arg1 != NULL && arg2 == NULL)
    			changeDir(arg1);
    		else
    			write(global_file, parameterMessage, strlen(parameterMessage));
    	} else if (strcmp(command, "cp") == 0) {
    		if(arg1 != NULL && arg2 != NULL)
        		copyFile(arg1, arg2);
        	else
        		write(global_file, parameterMessage, strlen(parameterMessage));
    	} else if (strcmp(command, "mv") == 0) {
    		if(arg1 != NULL && arg2 != NULL)
        		moveFile(arg1, arg2);
        	else
        		write(global_file, parameterMessage, strlen(parameterMessage));
    	} else if (strcmp(command, "rm") == 0) {
    		if(arg1 != NULL && arg2 == NULL)
        		deleteFile(arg1);
        	else
        		write(global_file, parameterMessage, strlen(parameterMessage));
    	} else if (strcmp(command, "cat") == 0 && arg1 != NULL) {
    		if(arg1 != NULL && arg2 == NULL)
        		displayFile(arg1);
        	else
        		write(global_file, parameterMessage, strlen(parameterMessage));
        } else if(strcmp(command, "exit") == 0) {
        	break;
    	} else {
        	write(global_file, unrecognizedMessage, strlen(unrecognizedMessage));
    	}
        	
        //iterate through command_list	
        count++;
        cmd = commands.command_list[count];
        
        free(command);
        free(arg1);
        free(arg2);
        
        command = NULL;
        arg1 = NULL;
        arg2 = NULL;
        
        command = malloc(100 * sizeof(char));
    	arg1 = malloc(100 * sizeof(char));
    	arg2 = malloc(100 * sizeof(char));
        
        
    }
    
    free_command_line(&commands);
    free(command);
    free(arg1);
    free(arg2);
}

