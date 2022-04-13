#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/wait.h>
#include<sys/time.h>

int main(){
	unsigned int n;
	unsigned int seg_id;
	unsigned int* seg_p, *tmp_p;
	pid_t pid;
	printf("Input the matrix dimension: ");
	scanf("%u", &n);
	unsigned int** mat_ab = (unsigned int**)malloc(sizeof(unsigned int*)*(n));
	for(int i = 0; i < n; i++) mat_ab[i] = (unsigned int*)malloc(sizeof(unsigned int)*(n));
	for(int i = 0; i < n; i++)
		for(int j = 0; j < n; j++)
			mat_ab[i][j] = i * n + j;

	for(int i = 1; i <= 16; i++){
		struct timeval start, end;
		gettimeofday(&start, 0);
		int last = n % i;
		int lines = n / i;
		seg_id = shmget(0, n*n*sizeof(unsigned int), IPC_CREAT | 0600);
		if(seg_id < 0){
			fprintf(stderr, "shmget failed\n");
			exit(1);
		}
		seg_p = (unsigned int*) shmat(seg_id, NULL, 0);
		if(seg_p == (unsigned int*) -1){
			fprintf(stderr, "shmat failed\n");
			exit(1);
		}
		tmp_p = seg_p;
		for(int j = 0; j < i; j++){
			if((pid = fork()) < 0){
				fprintf(stderr, "fork error\n");
				exit(1);
			}
			else if(pid == 0){
				if(j < last){
					for(int k = 0; k < lines+1; k++){
						for(int l = 0; l < n; l++){
							unsigned int cnt = 0;
							for(int m = 0; m < n; m++){
								cnt += mat_ab[(j*lines+j)+k][m] * mat_ab[m][l];
							}
							*(tmp_p+((j*lines+j)+k)*n+l) = cnt;
						}
					}
				}
				else{
					for(int k = 0; k < lines; k++){
						for(int l = 0; l < n; l++){
							unsigned int cnt = 0;
							for(int m = 0; m < n; m++){
								cnt += mat_ab[(j*lines+last)+k][m] * mat_ab[m][l];
							}
							*(tmp_p+((j*lines+last)+k)*n+l) = cnt;
						}
					}
				}
				exit(0);
			}
		}
		for(int j = 0; j < i; j++){
			wait(NULL);
		}
	    printf("\n");
		unsigned int sum = 0;
		for(int j = 0; j < n*n; j++)
			sum += *(tmp_p++);
		
		gettimeofday(&end, 0);
		double sec = end.tv_sec - start.tv_sec;
		double usec = end.tv_usec - start.tv_usec;
		double spend_time = (sec*1000+(usec/1000.0)) / 1000.0;
		printf("Multiplying matrices using %d process\n", i);
		printf("Elapsed time: %f sec, Checksum: %u\n", spend_time, sum);
		shmdt(seg_p);
		shmctl(seg_id, IPC_RMID, NULL);
	}
	return 0;
}