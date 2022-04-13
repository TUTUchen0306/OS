/*
Student No.: 0816173
Student Name: 陳盈圖
Email: st993201.cs08@nctu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not supposed to be posted to a public server, such as a public GitHub repository or a public web page.
*/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/mman.h>

struct block {
	size_t size;
	long long free;
	struct block *prev;
	struct block *next;
};

int start = 0;
void* head;
struct block *header;

void* malloc(size_t sz){
	size_t n_sz = 0;
	while(sz > n_sz) n_sz += 32;
	sz = n_sz; 
	if(!start){
		start = 1;
		head = mmap(NULL, 20000, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
		struct block *fh;
		header = head;
		fh = (head+32+sz);
		header->size = sz, fh->size = 20000 - sz - 32 - 32;
		header->free = 0, fh->free = 1;
		header->prev = NULL, fh->prev = header;
		header->next = fh, fh->next = NULL;
		return (head+32);
	}
	else{
		if(sz == 0){
			int cnt = 0;
			struct block *hd = header;
			while(hd != NULL){
				if(hd->free == 1){
					if(hd->size > cnt)
						cnt = hd->size;
				}
				hd = hd->next;
			}
			char buff[10] = "";
			sprintf(buff, "%d", cnt);
			char out_s[] = "Max Free Chunk Size = ";
			write(STDOUT_FILENO, out_s, sizeof(out_s));
			write(STDOUT_FILENO, buff, sizeof(buff));
			write(STDOUT_FILENO, "\n", strlen("\n"));
			munmap(head, 20000);
			return NULL;
		}
		else{
			struct block *hd = header;
			struct block *good_rec = header;
			size_t the_rest = 20000;
			while(hd != NULL){
				if(hd->free == 1){
					if(hd->size >= sz){
						if(hd->size - sz < the_rest){
							the_rest = hd->size - sz;
							good_rec = hd;
						}
					}
				}
				hd = hd->next;
			}
			if(the_rest > 0){
				struct block *tmp = ((void*)good_rec+32+sz);
				struct block *h_next = good_rec->next;
				tmp->size = good_rec->size - sz - 32, good_rec->size = sz;
				tmp->free = 1, good_rec->free = 0;
				tmp->prev = good_rec;
				if(h_next != NULL) h_next->prev = tmp;
				tmp->next = h_next, good_rec->next = tmp;
				return (void*)good_rec+32;
			}
			else if(the_rest == 0){
				good_rec->free = 0;
				return (void*)good_rec+32;
			}
			
		}
	}
}

void free(void* p){
	struct block *hd = p-32;
	struct block *hd_next = hd->next;
	struct block *hd_prev = hd->prev;
	if(hd_prev == NULL){
		hd->free = 1;
		if(hd_next == NULL) return;
		if(hd_next->free == 1){
			struct block *tmp_next = hd_next->next;
			hd->size = header->size + hd_next->size + 32;
			hd->next = tmp_next;
			tmp_next->prev = hd;
		}
		return;
	}
	else if(hd_next == NULL){
		hd->free = 1;
		if(hd_prev == NULL) return;
		if(hd_prev->free == 1){
			hd_prev->size = hd_prev->size + 32 + hd->size;
			hd_prev->next = NULL;
		}
		return;
	}
	else{
		hd->free = 1;
		if(hd_next->free == 1 && hd_prev->free == 1){
			hd_prev->next = hd_next->next;
			if(hd_next->next)
				hd_next->next->prev = hd_prev;
			hd_prev->size = hd_prev->size + 32 + hd->size + 32 + hd_next->size;
		}
		else if(hd_next->free == 1 && hd_prev->free == 0){
			hd->next = hd_next->next;
			if(hd_next->next)
				hd_next->next->prev = hd;
			hd->size = hd->size + 32 + hd_next->size;
		}
		else if(hd_next->free == 0 && hd_prev->free == 1){
			hd->prev->next = hd_next;
			hd_next->prev = hd_prev;
			hd_prev->size = hd_prev->size + 32 + hd->size;
		}
		return;
	}
}