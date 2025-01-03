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

int main(int argc, char * argv[]){
	//Read program workload from specified input file
	if (argc == 3){
		if(strcmp(argv[1], "-f") == 0){
			
			//Read the file line by line
			FILE *workload;
        	char *line = NULL;
        	size_t len = 0;
        	ssize_t nread;
        	
        	workload = fopen(argv[2], "r");
        	if (workload == NULL) {
            	perror("fopen");
            	exit(EXIT_FAILURE);
        	}
		
    		int max_args = 10;
    		int num_processes = 0;
    		
    		while ((nread = getline(&line, &len, workload)) != -1) {
        		num_processes++;
        	}
        	
        	rewind(workload);
        	
        	//This is the start of actually executing/reading the commands
			
			sigset_t sigset;
			sigemptyset(&sigset);
			sigaddset(&sigset, SIGUSR1);
			sigprocmask(SIG_BLOCK, &sigset, NULL);
        	
        	pid_t *processes = (pid_t*) malloc(num_processes * sizeof(pid_t));
        	int line_number = 0;
        	
        	while ((nread = getline(&line, &len, workload)) != -1) {
        		char *args[max_args];
        		char *token = strtok(line, " \n");
				int count = 0;
		
				// Tokenize the line into arguments
				while (token != NULL && count < max_args) {
					args[count] = token;
					token = strtok(NULL, " \n");
					count++;
				}
				
				args[count] = NULL;
				pid_t pid = fork();
				

				if(pid < 0){
					perror("Fork failed");
					exit(EXIT_FAILURE);
				}
				
				else if (pid == 0){
					int sig;
					sigwait(&sigset, &sig);

					int exec = execvp(args[0], args);
					
					if(exec == -1){
						perror("Command exec failed");
						exit(-1);
					}
				}
				else{
					processes[line_number] = pid;
					line_number++;
				}
        	}
        	
        	
        	for (int i = 0; i < num_processes; i++) {
				if(kill(processes[i], SIGUSR1) == -1){
					perror("SENDING SIGUSR1");
				}
			}
			
			
				
			sleep(1);
			for (int i = 0; i < num_processes; i++) {
				if(kill(processes[i], SIGSTOP) == -1){
					perror("Sending SIGSTOP");
				}
				else{
					printf("Process %d stopped\n", processes[i]);
				}
			}
			
				
			sleep(1);
			for (int i = 0; i < num_processes; i++) {
				if(kill(processes[i], SIGCONT) == -1){
					perror("Sending SIGCONT");
				}
				else{
					printf("Process %d continued\n", processes[i]);
				}
			}
			
			while(wait(NULL) > 0);
	
			free(processes);
			free(line);
        	fclose(workload);
			exit(EXIT_SUCCESS);
		}
		
		else{
			perror("Not in file mode");
			exit(EXIT_FAILURE);
		}
	}
	else{
		perror("Not enough args");
		exit(EXIT_FAILURE);
	}
}
