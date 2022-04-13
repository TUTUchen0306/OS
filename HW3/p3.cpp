/*
Student No.: 0816129
Student Name: Yi-Ting Hung
Email: 12401002he@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not supposed to be posted to a public server, such as a public GitHub repository or a public web page.
*/
#include <iostream>
#include <vector>
#include <queue>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <fstream>

#define MAX 0x3f3f3f3f
using namespace std;

int n, small_seg, small_left;
vector<int> arr;
vector<int> copyArr;
vector<pair<int, int> > work;
int work_done[20];
queue<int> do_list;
int g_pt;
int work_push[20];
int job_count;

sem_t give_job;
sem_t Q;
sem_t new_job;
sem_t save_do;
sem_t work_fin;

struct node{
    char work_type;
    int work_num;
    int front;
    int end;
    int mid;
    node(){}
    node(char work_type, int work_num, int front, int end, int mid){
        this->work_type = work_type;
        this->work_num = work_num;
        this->front = front;
        this->end = end;
        this->mid = mid;
    }
};

node *work_node[20];

void merge(int front, int mid, int end){
    vector<int> LeftSub(copyArr.begin() + front, copyArr.begin() + mid + 1),
                     RightSub(copyArr.begin() + mid + 1, copyArr.begin() + end + 1);
    LeftSub.insert(LeftSub.end(), MAX);      // Add MAX at the end of LeftSub[]
    RightSub.insert(RightSub.end(), MAX);    // Add MAX at the end of RightSub[]
    int idxLeft = 0, idxRight = 0;
    for (int i = front; i <= end; i++) {
        
        if (LeftSub[idxLeft] <= RightSub[idxRight]) {
            copyArr[i] = LeftSub[idxLeft++];
        }
        else {
            copyArr[i] = RightSub[idxRight++];
        }
    }
}

void bubble(int front, int end){
    for (int i = front; i < end ; ++i){
        for(int j = front; j < front + end - i; ++j){
            if(copyArr[j+1] < copyArr[j]){
                int temp;
                temp = copyArr[j+1];
                copyArr[j+1] = copyArr[j];
                copyArr[j] = temp;
            }
        }
    }
}

void *dispatcher(void *args){
    //小肥可愛！<3
    //give jobs?
    for(int i = 1; i <= g_pt; ++i)
        sem_post(&give_job);

    for(int i = 1; i <= 15 - g_pt; ++i){
        sem_wait(&work_fin);
        if(g_pt + i > 8)
            sem_wait(&new_job);
        sem_post(&give_job);
    }
	for(int i = 1; i <= g_pt; ++i)
    	sem_post(&give_job);
}

void *to_do(void *args){
    while(true){
        sem_wait(&give_job);
        if(job_count == 0) break;
        //mutex get job
        sem_wait(&Q);
        if(job_count == 0) break;
        job_count--;
        int cur_job = do_list.front();
        do_list.pop();
        //test
        sem_post(&Q);
        //working
        // cout << "cur_job: " << cur_job << "\n";

        struct node *cur = work_node[cur_job];

        if(cur->work_type == 'b')
            bubble(cur->front, cur->end);
        else if(cur->work_type == 'm')
            merge(cur->front, cur->mid, cur->end);

        work_done[cur_job] = 1;

        //mutex check if generate new job
        sem_wait(&save_do);
        for(int i = 15; i > 1; i -= 2){
            if(work_done[i] && work_done[i-1]){
                // cout << "i & i-1 : " << i << ", " << i-1 << ", make i/2: " << i/2 << "\n";
                if(work_push[i/2] == 0){
                    work_push[i/2] = 1;
                    do_list.push(i/2);
                    sem_post(&new_job);
                }
            }
        }
        sem_post(&save_do);
        sem_post(&work_fin);
    }
}

void init(){
    job_count = 15;
    while(!do_list.empty()) do_list.pop();
    for(int i = 0; i < n; ++i) copyArr.push_back(arr[i]);
    memset(work_done, 0, sizeof(work_done));
    memset(work_push, 0, sizeof(work_push));
    for(int i = 8; i <= 15; ++i) {
        work_push[i] = 1;
        do_list.push(i);
    }

    sem_init(&give_job, 0, 0);
    sem_init(&Q, 0, 1);
    sem_init(&save_do, 0, 1);
    sem_init(&work_fin, 0, 0);
    sem_init(&new_job, 0, 0);

    //test
}

void make_works(){
    for(int i = 0; i < 8; ++i){
        int j = 0;
        if(i < small_left){
            node *newNode = new node('b', i+8, i*small_seg+i, i*small_seg+i+small_seg, 0);
            work_node[i+8] = newNode;
            work.push_back(make_pair(i*small_seg+i, i*small_seg+i+small_seg));
            // cout << "bble: " << i*small_seg+i << " " << i*small_seg+i+small_seg << "\n";
        }
        else{
            node *newNode = new node('b', i+8, i*small_seg+small_left, i*small_seg+small_left+small_seg-1, 0);
            work_node[i+8] = newNode;
            work.push_back(make_pair(i*small_seg+small_left, i*small_seg+small_left+small_seg-1));
            // cout << "bble: " << i*small_seg+small_left << " " << i*small_seg+small_left+small_seg-1 << "\n";
        }
    }
    for(int i = 0; i < 4; ++i){
        int front = work[i*2].first;
        int end = work[i*2+1].second;
        int mid_i = (i*2 + i*2+1) / 2;
        int mid = work[mid_i].second;
        node *newNode = new node('m', i+4, front, end, mid);
        work_node[i+4] = newNode;
        // cout << i+4 << ": " << front << " " << mid << " " << end << "\n";
    }
    for(int i = 0; i < 2; ++i){
        int front = work[i*4].first;
        int end = work[i*4+3].second;
        int mid_i = (i*4 + i*4+3) / 2;
        int mid = work[mid_i].second;
        node *newNode = new node('m', i+2, front, end, mid);
        work_node[i+2] = newNode;
        // cout << i+2 << ": " << front << " " << mid << " " << end << "\n";
    }

    int mid_i = n / 2;
    int mid = work[3].second;
    node *lastNode = new node('m', 1, work[0].first, work[7].second, mid);
    work_node[1] = lastNode;
    // cout << 1 << ": " << work[0].first << " " << mid << " " << work[7].second << "\n";
}

int main(){
	// I/O
	cin >> n;
	for(int i = 0; i < n; ++i){
        int tmp;
        cin >> tmp;
        arr.push_back(tmp);
    }
	small_seg = n/8;
	small_left = n%8;

    //make works

    make_works();

    // start
    for(int p_num = 1; p_num <= 8; ++p_num){
        init();
        struct timeval start, end;
        gettimeofday(&start, 0);
        // cout << "yeah boiiiiiiiiiiiiiiii\n";
        g_pt = p_num;
        pthread_t thread_dis;
        pthread_t threads[20];

        pthread_create(&thread_dis, NULL, dispatcher, NULL);

        for(int i = 1; i <= p_num; ++i){
            pthread_create(&threads[i], NULL, to_do, NULL);
        }
        
        for(int i = 1; i <= p_num; ++i){
            pthread_join(threads[i], NULL);
        }
        pthread_join(thread_dis, NULL);
        char s[10] = "x.txt";
        s[0] = char(p_num+'0');
        ofstream fout;
        fout.open(s);
        for(int i = 0; i < n; ++i) fout << copyArr[i] << " ";
        fout << "\n";
        gettimeofday(&end, 0);
        double sec = end.tv_sec - start.tv_sec;
        double usec = end.tv_usec - start.tv_usec;
        double spend_time = (sec*1000.0+(usec/1000.0));
        printf("worker thread #%d, ", p_num);
        printf("elapsed %f ms\n", spend_time);
    }

	return 0;
}