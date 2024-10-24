#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include "string_parser.h"
#include "command.h"

int global_file;

int main(int argc, char * argv[]){
	//Read program workload from specified input file
	if (argc == 3){
		if(strcmp(argv[1], "-f") == 0){
			
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
		
    		global_file = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

        	while ((nread = getline(&line, &len, stream)) != -1) {
        		//
        		//
        		//
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
