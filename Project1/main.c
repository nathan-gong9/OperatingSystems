#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "string_parser_h_"

int global_file;

int main(int argc, char * argv[]){
	
	//What to do if we're executing the pseudo-shell in file mode
	if(argv[1] = "-f" && argc == 3){
			
		//Read the file line by line
		FILE *stream;
        char *line = NULL;
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
		
		FILE *output = fopen("output.txt", "r");
		if (file == NULL) {
        	printf("Error opening file!\n");
        	return 1;
    	}
    	global_file = "output.txt";

        while ((nread = getline(&line, &len, stream)) != -1) {
            printf("Retrieved line of length %zd:\n", nread);
            fwrite(line, nread, 1, stdout);
            executeLine(line)
            
            
        }

        free(line);
        fclose(stream);
        fclose(output);
        exit(EXIT_SUCCESS);
	}
	
	//What to do if we execute the pseudo-shell in interactive mode
	else if (argc == 1){
		global_file = STDOUT_FILENO;
	
		FILE *stream;
        char *line = NULL;
        size_t len = 0;
        ssize_t nread;
        int count = 0;

        stream = fopen(argv[count], "r");
        if (stream == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        
        while ((nread = getline(&line, &len, stream)) != -1 && count < argc) {
            printf("Retrieved line of length %zd:\n", nread);
            fwrite(line, nread, 1, stdout);
            
        }
        
        free(line);
        fclose(stream);
        exit(EXIT_SUCCESS);
	}
}

void executeLine(char *line, FILE *output){
	//Go through the commands on the line by using the delimiters
    command_line commands = str_filler(line, " ; ");
    count = 0;
    char* cmd = commands.command_list[count];
    commands.num_token = count_token(line, " : ");
    if(commands.num_token == 0){
    	commands  = str_filler(line, ";");
    	cmd = commands.command_list[count];
    }
    count++;
    
    char command[50];
    char arg1[50];
    char arg2[50];
 
    while (count <= commands.num_token) {
    	printf(" % s\n", cmd);
    	
    	int argCount = sscanf(cmd, "%s %s %s", cmd, arg1, arg2);
    	
        //identify the command and what to do with it.
        if (strcmp(cmd, "ls") == 0) {
        	if(arg1 != NULL)
        		write(global_file, "Error! Unsupported parameters for command: %s\n", command);
        	else
        		listDir();
    	} else if (strcmp(cmd, "pwd") == 0) {
    		if(arg1 != NULL)
    			write(global_file, "Error! Unsupported parameters for command: %s\n", command)
    		else
        		showCurrentDir();
    	} else if (strcmp(cmd, "mkdir") == 0) {
    		if(arg1 != NULL && arg2 == NULL)
    			makeDir(arg1);
    		else
    			write(global_file, "Error! Unsupported parameters for command: %s\n", command)
    	} else if (strcmp(cmd, "cd") == 0) {
    		if(arg1 != NULL && arg2 == NULL)
    			changeDir(arg1);
    		else
    			write(global_file, "Error! Unsupported parameters for command: %s\n", command)
    	} else if (strcmp(cmd, "cp") == 0) {
    		if(arg1 != NULL && arg2 != NULL)
        		copyFile(arg1, arg2);
        	else
        		write(global_file, "Error! Unsupported parameters for command: %s\n", command)
    	} else if (strcmp(cmd, "mv") == 0) {
    		if(arg1 != NULL && arg2 != NULL)
        		moveFile(arg1, arg2);
        	else
        		write(global_file, "Error! Unsupported parameters for command: %s\n", command)
    	} else if (strcmp(cmd, "rm") == 0) {
    		if(arg1 != NULL && arg2 == NULL)
        		deleteFile(arg1);
        	else
        		write(global_file, "Error! Unsupported parameters for command: %s\n", command)
    	} else if (strcmp(cmd, "cat") == 0 && arg1 != NULL) {
    		if(arg1 != NULL && arg2 == NULL)
        		displayFile(arg1);
        	else
        		write(global_file, "Error! Unsupported parameters for command: %s\n", command)
    	} else {
        	write(global_file, "Error! Unrecognized command: %s \n", cmd);
    	}
        
        //Exit if the exit command is entered
        if(cmd == "exit"){
        	exit;
        }
        	
        //iterate through command_list	
        cmd = commands.command_list[count++];
    }
    
    free_command_line(commands);
}

