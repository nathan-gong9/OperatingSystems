#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


void sigusr1_handler(int sig) {

}

int main(int argc, char * argv[]){
	//Read program workload from specified input file
	if (argc == 3){
		if(strcmp(argv[1], "-f") == 0){
			
			//Read the file line by line
			FILE *workload;
        	char *line = NULL;
        	size_t len = 0;
        	ssize_t nread;

        	if (argc != 3) {
            	fprintf(stderr, "Usage: %s <file>\n", argv[0]);
            	exit(EXIT_FAILURE);
        	}

        	workload = fopen(argv[2], "r");
        	if (workload == NULL) {
            	perror("fopen");
            	exit(EXIT_FAILURE);
        	}
		
    		int max_args = 10;
    		int num_lines = 0;
    		
    		while ((nread = getline(&line, &len, workload)) != -1) {
        		num_lines++;
        	}
        	
        	rewind(workload);
        	
        	//This is the start of actually executing/reading the commands
        	
        	sa.sa_handler = sigusr1_handler;
			sigemptyset(&sa.sa_mask);
			sa.sa_flags = 0;
			sigaction(SIGUSR1, &sa, NULL);
        	
        	pid_t *commands = (pid_t*) malloc(num_lines * sizeof(pid_t));
        	int line_number = 0;
        	
        	while ((nread = getline(&line, &len, workload)) != -1) {
        		char *args[max_args];
        		char *token = strtok(line, " ");
				int count = 0;
		
				// Tokenize the line into arguments
				while (token != NULL && count < max_args) {
					args[count] = token;
					token = strtok(NULL, " \n");
					count++;
				}
				
				args[count] = NULL;
				
				commands[line_number] = fork();
				
				if(commands[line_number] < 0){
					char error[] = "Fork failed\n";
					write(STDOUT_FILENO, error, sizeof(error));
					exit(EXIT_FAILURE);
				}
				
				else if (commands[line_number] == 0){
				
					sigset_t sigset;
					sigemptyset(&sigset);
					sigaddset(&sigset, SIGUSR1);
					sigprocmask(SIG_BLOCK, &sigset, NULL);
					int sig;
					sigwait(&sigset, &sig);
					
					int wait_signal = sigwait();
					
					if(wait_signal != 0){
						char error[] = "Sigwait failed\n";
						write(STDOUT_FILENO, error, sizeof(error));
						exit(EXIT_FAILURE);
					}
					int exec = execvp(args[0], args);
				
					if (exec == -1) {
                		char error[] = "Execvp failed\n";
						write(STDOUT_FILENO, error, sizeof(error));
						exit(EXIT_FAILURE);
            		}
					char success[] = "Execvp success\n";
					write(STDOUT_FILENO, success, sizeof(success));
					exit(EXIT_FAILURE);
					exit(-1);
				}
				
				line_number++;
        	}
        	
        	while(wait(NULL) > 0);

        	free(line);
        	fclose(workload);
        	free(commands);
        	exit(EXIT_SUCCESS);
		}
		else{
			char error[] = "Wrong amount of parameters\n";
			write(STDOUT_FILENO, error, sizeof(error));
			exit(EXIT_FAILURE);
		}
	}
}
