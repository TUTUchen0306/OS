/*
Student No.: 0816173
Student Name: 陳盈圖
Email: st993201.cs08@nycu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<queue>
#include<sys/wait.h>
#include<sys/time.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<fstream>
#define INF 0x3f3f3f3f
#define Bubble 1
#define Merge 2

using namespace std;

struct jobs{
	int left, right;
	int mll, mlr, mrl, mrr;
	int left_length, right_length;
	int job_type;
};

sem_t dis_sem;
sem_t work_sem;
sem_t test_sem;
sem_t new_sem;
sem_t need_sem;
sem_t nw_sem;

int n;
int job_pointer;
int job_amount;
int ptr;
int a[1000010];
int num[1000010];
int arr_checker[20];
int job_checker[20];
struct jobs jobs_queue[20];
queue<int> jq;

void Bub(int job_num);
void Mrg(int job_num);

void *Dispatcher(void *args){
	for(int pt = 15; pt > 0; pt--){
		sem_wait(&dis_sem);
		if(pt < 8)
			sem_wait(&need_sem);
		sem_post(&work_sem);
	}
	for(int i = 1; i <= ptr; i++)
		sem_wait(&dis_sem);
	for(int i = 1; i <= ptr; i++)
		sem_post(&work_sem);
	return NULL;
}

void *Worker(void *args){
	while(1){
		sem_wait(&work_sem);
		if(job_pointer == 0) break;
		sem_wait(&test_sem);
		if(job_pointer == 0) break;
		job_pointer--;
		int job_ind = jq.front();
		jq.pop();
		sem_post(&test_sem);
		if(jobs_queue[job_ind].job_type == Bubble) Bub(job_ind);
		else Mrg(job_ind);
		arr_checker[job_ind] = 1;
		sem_post(&new_sem);
		sem_post(&dis_sem);
	}
	return NULL;
}

void *New_job(void *args){
	while(1){
		sem_wait(&new_sem);
		sem_wait(&nw_sem);
		for(int i = 15; i > 1; i -= 2){
			if(arr_checker[i] && arr_checker[i-1]){
				if(!job_checker[i/2]){
					struct jobs jmp;
					jmp.mll = jobs_queue[i-1].left;
					jmp.mlr = jobs_queue[i-1].right;
					jmp.mrl = jobs_queue[i].left;
					jmp.mrr = jobs_queue[i].right;
					jmp.left = jmp.mll;
					jmp.right = jmp.mrr;
					jmp.left_length = jmp.mlr - jmp.mll + 1;
					jmp.right_length = jmp.mrr - jmp.mrl + 1;
					jmp.job_type = Merge;
					jobs_queue[i/2] = jmp;
					job_checker[i/2] = 1;
					jq.push(i/2);
					sem_post(&need_sem);
				}
			}
		}
		sem_post(&nw_sem);
	}
}

void Bub(int job_num){
	struct jobs jmp = jobs_queue[job_num];
	for(int i = jmp.left; i <= jmp.right; i++){
		for(int j = jmp.left; j < jmp.right-(i - jmp.left); j++){
			if(num[j] > num[j+1]){
				int tmp = num[j];
				num[j] = num[j+1];
				num[j+1] = tmp;
			}
		}
	}
}

void Mrg(int job_num){
	struct jobs jmp = jobs_queue[job_num];
	int l_arr[jmp.left_length+10], r_arr[jmp.right_length+10];
	for(int i = 0; i < jmp.left_length; i++) l_arr[i] = num[jmp.mll+i];
	for(int i = 0; i < jmp.right_length; i++) r_arr[i] = num[jmp.mrl+i];
	l_arr[jmp.left_length] = INF;
	r_arr[jmp.right_length] = INF;
	int l_ind = 0, r_ind = 0;
	for(int i = jmp.left; i <= jmp.right; i++){
		if(l_arr[l_ind] <= r_arr[r_ind]){
			num[i] = l_arr[l_ind];
			l_ind++;
		}
		else{
			num[i] = r_arr[r_ind];
			r_ind++;
		}
	}
}

void init(){
	while(jq.size()) jq.pop();
	for(int i = 1; i <= n; i++)
		num[i] = a[i];
	job_pointer = 15;
	job_amount = 8;
	int last = n % 8;
	int base = n / 8;
	memset(jobs_queue, 0, sizeof(jobs_queue));
	memset(arr_checker, 0, sizeof(arr_checker));
	memset(job_checker, 0, sizeof(job_checker));
	for(int i = 8; i <= 15; i++){
		jq.push(i);
		job_checker[i] = 1;
		int now_i = i - 7;
		if(now_i <= last){
			jobs_queue[i].left = ((now_i-1)*base+1+(now_i-1));
			jobs_queue[i].right = (now_i*base+(now_i-1)+1);
			jobs_queue[i].job_type = Bubble;
		}
		else{
			jobs_queue[i].left = ((now_i-1)*base+last+1);
			jobs_queue[i].right = (now_i*base+last);
			jobs_queue[i].job_type = Bubble;
		}
	}
	sem_init(&work_sem, 0, 0);
	sem_init(&test_sem, 0, 1);
	sem_init(&new_sem, 0, 0);
	sem_init(&need_sem, 0, 0);
	sem_init(&nw_sem, 0, 1);
}

int main(){
	ifstream fin;
	fin.open("input.txt");
	fin >> n;
	for(int i = 1; i <= n; i++)
		fin >> a[i];

	for(int pt = 1; pt <= 8; pt++){
		ptr = pt;
		char fptr[13] = "output_?.txt";
		fptr[7] = (char)('0' + pt);
		ofstream fout;
		fout.open(fptr);
		struct timeval start, end;
		gettimeofday(&start, 0);
		init();
		sem_init(&dis_sem, 0, pt);
		pthread_t p_dis;
		pthread_t p_new;
		pthread_t ptt[10];
		for(int i = 1; i <= pt; i++)
			pthread_create(&ptt[i], NULL, Worker, NULL);
		pthread_create(&p_dis, NULL, Dispatcher, NULL);
		pthread_create(&p_new, NULL, New_job, NULL);
		
		for(int i = 1; i <= pt; i++)
			pthread_join(ptt[i], NULL);
		pthread_join(p_dis, NULL);

		printf("worker thread #%d, ", pt);
		for(int i = 1; i <= n; i++)
			fout << num[i] << ' ';
		fout << "\n";

		gettimeofday(&end, 0);
		double sec = end.tv_sec - start.tv_sec;
		double usec = end.tv_usec - start.tv_usec;
		double spend_time = (sec*1000.0+(usec/1000.0));
		printf("elapsed %f ms\n", spend_time);
	}
	return 0;
}
