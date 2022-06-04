/*
Student No.: 0816173
Student Name: Ying-Tu Chen
Email: st993201.cs08@nycu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#define MAX 512

void ign_handler(){
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main(){
	signal(SIGCHLD, SIG_IGN);
	// signal(SIGCHLD, ign_handler);
	char string[MAX];
	while(1){
		printf("\n> ");

		if(fgets(string, sizeof(string), stdin) == 0)
			return 0;

		char s[] = " \n";
		char *token;
		char *rec[MAX][MAX] = {0};
		int pipe_array[MAX][2];
		int cnt = 0, pipecnt = 0;
		int nowait = 0;

		token = strtok(string, s);
		rec[0][cnt++] = token;

		while(token != NULL){
			token = strtok(NULL, s);
			if(token != NULL && strcmp(token, "&") == 0){
				nowait = 1;
				continue;
			}
			else if(token != NULL && strcmp(token, "|") == 0){
				rec[pipecnt++][cnt] = NULL;
				cnt = 0;
				continue;
			}
			rec[pipecnt][cnt++] = token;
		}

		for(int i = 0; i < pipecnt; i++){
			if(pipe(pipe_array[i]) < 0){
				perror("pipe create error\n");
			}
		}

		for(int i = 0; i <= pipecnt; i++){
			pid_t pid = fork();
			if(pid < 0){
				perror("Fork failed\n");
				exit(-1);
			}
			else if(pid == 0){

				int rec_cnt = 0;
				int in = 0, out = 0;
				char *filename = NULL;

				while(rec[i][rec_cnt] != NULL){
					if(rec[i][rec_cnt] != NULL && strcmp(rec[i][rec_cnt], "<") == 0){
						in = 1;
						filename = rec[i][rec_cnt+1];
						rec[i][rec_cnt] = NULL;
						rec[i][rec_cnt+1] = NULL;
					}
					else if(rec[i][rec_cnt] != NULL && strcmp(rec[i][rec_cnt], ">") == 0){
						out = 1;
						filename = rec[i][rec_cnt+1];
						rec[i][rec_cnt] = NULL;
						rec[i][rec_cnt+1] = NULL;
					}
					rec_cnt++;
				}

				if(in == 1){
					int fd0 = open(filename, O_RDWR);
					if(fd0 < 0){
						perror("Cannot open the file\n");
						exit(0);
					}
					in = 0;
					dup2(fd0, STDIN_FILENO);
					close(fd0);
				}

				else if(out == 1){
					int fd1 = open(filename, O_CREAT | O_RDWR);
					out = 0;
					dup2(fd1, STDOUT_FILENO);
					close(fd1);
				}

				else{
					if(i != 0)
						dup2(pipe_array[i-1][0], STDIN_FILENO);
					if(i != pipecnt)
						dup2(pipe_array[i][1], STDOUT_FILENO);

					for(int j = 0; j < pipecnt; j++){
						close(pipe_array[j][0]);
						close(pipe_array[j][1]);
					}
				}

				if(execvp(rec[i][0], rec[i]) == -1){
					fprintf(stderr ,"%s error\n", rec[i][0]);
					exit(1);
				}
			}
			else{

				if(i != 0){
					close(pipe_array[i-1][0]);
					close(pipe_array[i-1][1]);
				}
				if(nowait == 0){
					waitpid(pid, NULL, 0);
				}
			}
		}
	}
	return 0;
}