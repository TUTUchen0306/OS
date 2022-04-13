/*
Student No.: 0816173
Student Name: 陳盈圖
Email: st993201@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include<iostream>
#include<string.h>
#include<fstream>
#include<queue>
#include<map>
#include<sys/wait.h>
#include<sys/time.h>
#define pip pair<int, pair<int, unsigned int> >
#define F first
#define S second
using namespace std;


struct lru{
	unsigned int page_number;
	struct lru *prev;
	struct lru *next;

	lru(unsigned int page_number, struct lru *prev, struct lru *next){
		this->page_number = page_number;
		this->prev = prev, this->next = next;
	}
};

map<unsigned int, struct lru *> mp_lru;
struct lru *lru_head;
struct lru *lru_last;

priority_queue<pip, vector<pip>, greater<pip> > pq_lfu;
map<unsigned int, unsigned int> mp_lfu;

int frame_num[4] = {64, 128, 256, 512};

int hit_lru[4], miss_lru[4];
int hit_lfu[4], miss_lfu[4]; 

void LRU(int p, const char *file_name){
	ifstream fin;
	fin.open(file_name);
	unsigned int p_num;
	int cnt = 0;
	while(fin >> p_num){
		if(lru_head == NULL){
			miss_lru[p]++;
			cnt++;
			lru_head = new lru(p_num, NULL, NULL);
			lru_last = lru_head;
			mp_lru[p_num] = lru_head;
		}
		else{
			if(mp_lru.find(p_num) != mp_lru.end()){
				hit_lru[p]++;
				struct lru *tmp = mp_lru[p_num];
				if(tmp == lru_head) continue;
				if(tmp == lru_last) lru_last = tmp->prev;
				if(tmp->prev) tmp->prev->next = tmp->next;
				if(tmp->next) tmp->next->prev = tmp->prev;
				lru_head->prev = tmp;
				tmp->next = lru_head;
				lru_head = tmp;
			}
			else{
				miss_lru[p]++;
				if(cnt == frame_num[p]){
					struct lru *tmp = lru_last;
					mp_lru.erase(lru_last->page_number);
					lru_last = lru_last->prev;
					lru_last->next = NULL;

					tmp->page_number = p_num;
					lru_head->prev = tmp;
					tmp->next = lru_head;
					tmp->prev = NULL;
					lru_head = tmp;
					mp_lru[p_num] = tmp;
				}
				else{
					cnt++;
					struct lru *tmp = new lru(p_num, NULL, lru_head);
					lru_head->prev = tmp;
					lru_head = tmp;
					mp_lru[p_num] = tmp;
				}
			}
		}
	}
	double ratio = (double)miss_lru[p] / (double)(hit_lru[p] + miss_lru[p]);
	printf("%d\t%d\t\t%d\t\t%.10f\n", frame_num[p], hit_lru[p], miss_lru[p], ratio);
}

void LFU(int p, const char *file_name){
	ifstream fin;
	fin.open(file_name);
	unsigned int p_num;
	int cnt = 0;
	int times = 0;
	while(fin >> p_num){
		times++;
		if(mp_lfu.find(p_num) != mp_lfu.end()){
			hit_lfu[p]++;
			mp_lfu[p_num]++;
			pq_lfu.push(make_pair(mp_lfu[p_num], make_pair(times, p_num)));
		}
		else{
			miss_lfu[p]++;
			if(cnt == frame_num[p]){
				while(true){
					pip tmp = pq_lfu.top();
					pq_lfu.pop();
					if(tmp.F != mp_lfu[tmp.S.S]) continue;
					else{
						mp_lfu.erase(tmp.S.S);
						pq_lfu.push(make_pair(1, make_pair(times, p_num)));
						mp_lfu[p_num] = 1;
						break;
					}
				}
			}
			else{
				cnt++;
				pq_lfu.push(make_pair(1, make_pair(times, p_num)));
				mp_lfu[p_num] = 1;
			}
		}
	}
	double ratio = (double)miss_lfu[p] / (double)(hit_lfu[p] + miss_lfu[p]);
	printf("%d\t%d\t\t%d\t\t%.10f\n", frame_num[p], hit_lfu[p], miss_lfu[p], ratio);
}

void init(){
	mp_lru.clear();
	mp_lfu.clear();
	lru_head = NULL;
	lru_last = NULL;
	while(pq_lfu.size()) pq_lfu.pop();
}

int main(int argc, char const *argv[]){
	struct timeval start, end;
	printf("LFU policy:\n");
	printf("Frame\tHit\t\tMiss\t\tPage fault ratio\n");
	gettimeofday(&start, 0);
	for(int p = 0; p < 4; p++){
		init();
		LFU(p, argv[1]);
	}
	gettimeofday(&end, 0);
	double sec = end.tv_sec - start.tv_sec;
	double usec = end.tv_usec - start.tv_usec;
	double spend_time = (sec*1000+(usec/1000.0)) / 1000.0;
	printf("Total elapsed time %4f sec\n\n", spend_time);

	printf("LRU policy:\n");
	printf("Frame\tHit\t\tMiss\t\tPage fault ratio\n");
	gettimeofday(&start, 0);
	for(int p = 0; p < 4; p++){
		init();
		LRU(p, argv[1]);
	}
	gettimeofday(&end, 0);
	sec = end.tv_sec - start.tv_sec;
	usec = end.tv_usec - start.tv_usec;
	spend_time = (sec*1000+(usec/1000.0)) / 1000.0;
	printf("Total elapsed time %4f sec\n\n", spend_time);
	return 0;
}