#include <stdio.h>
#include <stdlib.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define N 12

extern char **environ;

char *allowed[N] = {"cp","touch","mkdir","ls","pwd","cat","grep","chmod","diff","cd","exit","help"};

//returns 1 if the command is allowed, 0 otherwise
int isAllowed(const char*cmd) {
	for (int i=0; i<N; i++) {
		if (strcmp(cmd,allowed[i])==0) {
			return 1;
		}
	}
	return 0;
}

int main() {
    char line[256];

    while (1) {
		//The command is initialized to NULL
		char command[256];

		//The number of arguments is limited to 20, and the last argument is NULL
		char *argv[21];
		
		fprintf(stderr,"rsh>");
		if (fgets(line,256,stdin)==NULL) continue;
		if (strcmp(line,"\n")==0) continue;
		line[strlen(line)-1]='\0';

		//make the argument array
		char lineCopy[256];
		strcpy(lineCopy, line);
		int i=0;
		char *token = strtok(lineCopy, " ");
		while(token != NULL && i<20) {
			argv[i++] = token;
			token = strtok(NULL, " ");
		}

		if(i>=20) {
			i++;
			argv[i]=NULL;
		} else {
			argv[i]=NULL;
		}
		
		//the command should be the first argument
		strcpy(command, argv[0]);

		// Initialize spawn attributes
		posix_spawnattr_t attr;
		posix_spawnattr_init(&attr);

		if(isAllowed(command) != 1) {
			printf("NOT ALLOWED\n");
		} else {
			//Spawn process for first nine commands
			if(isAllowed(command) && strcmp(command,"cd")!=0 && strcmp(command,"exit")!=0 && strcmp(command,"help")!=0) {
				pid_t pid;
				int status;

				// Spawn a new process
				if (posix_spawnp(&pid, command, NULL, &attr, argv, environ) != 0) {
					perror("posix_spawn");
					exit(EXIT_FAILURE);
				}

				// Wait for the spawned process to terminate
				if (waitpid(pid, &status, 0) == -1) {
					perror("waitpid failed");
					exit(EXIT_FAILURE);
				}

				// Destroy spawn attributes
				posix_spawnattr_destroy(&attr);

			}

			//Execute cd command
			if(isAllowed(command) && strcmp(command,"cd")==0) {
				int count = 0;
				while (argv[count] != NULL && count<20) {
					count++;
				}

				if(count>2) {
					printf("-rsh: cd: too many arguments\n");
				} else {
					chdir(argv[1]);
				}
			}

			//Execute help command, simply print the allowed commands
			if(isAllowed(command) && strcmp(command,"help")==0) {
				printf("The allowed commands are:\n");
				for (int i=0; i<N; i++) {
					printf("%s\n", allowed[i]);
				}
			}

			//Execute exit command, simply return 0
			if(isAllowed(command) && strcmp(command,"exit")==0) {
				return 0;
			}
		}	
	}
}
