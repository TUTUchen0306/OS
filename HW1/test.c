#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main(){
	char string[256];
	char s[] = " \n";
	int cnt = 0;
	fgets(string, sizeof(string), stdin);
	char *token = strtok(string, s);
	char *rec[100];
	rec[cnt] = token;
	cnt++;
	while(token != NULL){
		// printf("%s\n", token);
		token = strtok(NULL, s);
		if(token == NULL) break;
		rec[cnt] = token;
		cnt++;
	}
	for(int i = 0; i < cnt; i++)
		printf("%s\n", rec[i]);
}