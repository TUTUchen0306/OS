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
#include<unistd.h>

// void ign_handler(){
// 	while(waitpid(-1, NULL, WNOHANG) > 0);
// }

int main(){
	signal(SIGCHLD, SIG_IGN);
	// signal(SIGCHLD, ign_handler);
	char string[256];
	while(1){
		printf(">");

		if(fgets(string, sizeof(string), stdin) == 0)
			return 0;

		char s[] = " \n";
		char *token;
		char *rec[256] = {0};
		int cnt = 0, pipecnt = 0;
		int nowait = 0;
		token = strtok(string, s);
		rec[cnt++] = token;

		while(token != NULL){
			token = strtok(NULL, s);
			if(token != NULL && strcmp(token, "&") == 0){
				nowait = 1;
				continue;
			}
			rec[cnt++] = token;
		}

		pid_t pid = fork();
		if(pid < 0){
			perror("Fork failed\n");
			exit(-1);
		}
		else if(pid == 0){
			if(execvp(rec[0], rec) == -1){
				fprintf(stdout, "%s error\n", rec[0]);
				exit(1);
			}
		}
		else{
			if(nowait == 0){
				waitpid(pid, NULL, 0);
			}
		}
	}
	return 0;
}