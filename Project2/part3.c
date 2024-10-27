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

#define TIME_SLICE 1 

pid_t *processes; 
int num_processes;    
int current_process = 0;  

void alarm_handler(int sig) {
    kill(processes[current_process], SIGSTOP);
    printf("Process %d stopped\n", processes[current_process]);

    current_process = (current_process + 1) % num_processes;

    kill(processes[current_process], SIGCONT);
    printf("Process %d continued\n", processes[current_process]);

    alarm(TIME_SLICE);
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
        	
        	workload = fopen(argv[2], "r");
        	if (workload == NULL) {
            	perror("fopen");
            	exit(EXIT_FAILURE);
        	}
		
    		num_processes = 0;
    		
    		while ((nread = getline(&line, &len, workload)) != -1) {
        		num_processes++;
        	}
        	
        	rewind(workload);
        	
        	//This is the start of actually executing/reading the commands
        	
        	struct sigaction sa;
			sa.sa_handler = alarm_handler;
			sigemptyset(&sa.sa_mask);
			sa.sa_flags = 0;
			sigaction(SIGALRM, &sa, NULL);
			
			sigset_t sigset;
			sigemptyset(&sigset);
			sigaddset(&sigset, SIGUSR1);
			sigprocmask(SIG_BLOCK, &sigset, NULL);
        	
        	processes = (pid_t*) malloc(num_processes * sizeof(pid_t));
        	int max_args = 10;
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
					char error[] = "Fork failed\n";
					write(STDOUT_FILENO, error, sizeof(error));
					exit(EXIT_FAILURE);
				}
				
				else if (pid == 0){
					int sig;
					sigwait(&sigset, &sig);

					int exec = execvp(args[0], args);
				
					exit(-1);
				}
				else{
					processes[line_number] = pid;
					line_number++;
				}
        	}
        	
        	
			for (int i = 0; i < num_processes; i++) {
				kill(processes[i], SIGUSR1);
			}
	
			for (int i = 1; i < num_processes; i++) {
				kill(processes[i], SIGSTOP);
			}

			alarm(TIME_SLICE);
	
			kill(processes[0], SIGCONT);
			printf("Process %d started\n", processes[0]);
	
			while (num_processes > 0) {
				int status;
				pid_t pid = waitpid(-1, &status, WNOHANG);  
	
				if (pid > 0) {
					printf("Process %d terminated\n", pid);
	
					int i;
					for (i = 0; i < num_processes; i++) {
						if (processes[i] == pid) break;
					}
					for (; i < num_processes - 1; i++) {
						processes[i] = processes[i + 1];
					}
					num_processes--;
	
					if (current_process >= num_processes) {
						current_process = 0;
					}
				}
			}
	
			free(processes);
			free(line);
        	fclose(workload);
			exit(EXIT_SUCCESS);
		}
		
		else{
			char error[] = "Not in file mode\n";
			write(STDOUT_FILENO, error, sizeof(error));
			exit(EXIT_FAILURE);
		}
	}
	else{
		char error[] = "Wrong amount of parameters\n";
		write(STDOUT_FILENO, error, sizeof(error));
		exit(EXIT_FAILURE);
	}
}
