#define FUSE_USE_VERSION 30
#include<fuse.h>
#include<string.h>
#include<iostream>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<vector>
#include<stdlib.h>

using namespace std;

struct t1{
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
};

t1 n_file[1000];
char *content[1000];
int file_cnt = 0;

char *m_path[1000];
int path_cnt = 0;

char *m_f[1000];
int f_cnt = 0;

char *m_d[1000];
int d_cnt = 0;

vector<int> p[100], d[100];

long long octtoll(char* tmp){
    long long sz = 0;
    for(int i = 0; i < strlen(tmp); i++){
        if(tmp[i] == ' ') break;
        sz *= 8;
        sz += (tmp[i] - '0');
    }
    return sz;
}

int my_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
        filler(buffer, ".", NULL, 0);
        filler(buffer, "..", NULL, 0);
        if(strcmp(path, "/") == 0){
                for(int i = 0; i < d[path_cnt].size(); i++)
                        filler(buffer, m_d[d[path_cnt][i]], NULL, 0);
                for(int i = 0; i < p[path_cnt].size(); i++)
                        filler(buffer, m_f[p[path_cnt][i]], NULL, 0);
        }
        else{
            for(int i = 0; i < path_cnt; i++){
                if(strcmp(path, m_path[i]) == 0){
                    for(int j = 0; j < d[i].size(); j++)
                            filler(buffer, m_d[d[i][j]], NULL, 0);
                    for(int j = 0; j < p[i].size(); j++)
                            filler(buffer, m_f[p[i][j]], NULL, 0);
                }
            }
        }
        return 0;
}
int my_getattr(const char *path, struct stat *st){
        if(strcmp(path, "/") == 0){
                st->st_mode = S_IFDIR | 0444;
                return 0;
        }
        for(int i = 0; i < file_cnt; i++){
                if(strcmp(path, n_file[i].name) == 0){
                        st->st_uid = octtoll(n_file[i].uid);
                        st->st_gid = octtoll(n_file[i].gid);
                        st->st_mtime = octtoll(n_file[i].mtime);
                        if(n_file[i].typeflag == '5'){
                                mode_t md = octtoll(n_file[i].mode);
                                st->st_mode = S_IFDIR | md;
                                st->st_size = octtoll(n_file[i].size);
                        }
                        else{
                                mode_t md = octtoll(n_file[i].mode);
                                st->st_mode = S_IFREG | md;
                                st->st_size = octtoll(n_file[i].size);
                        }
                        return 0;
                }
        }
        return -2;
}
int my_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi){
    char *select = (char*) malloc(sizeof(char) * 10000);
    for(int i = 0; i < file_cnt; i++){
        if(strcmp(path, n_file[i].name) == 0){
            long long sz = octtoll(n_file[i].size);
            for(int j = 0; j < sz; j++) select[j] = content[i][j];
            if(offset + size > sz) size = sz - offset;
            if(size < 0) size = 0;
            memcpy(buffer, select + offset, size);
            return size;
        }
    }
    return -1;
}
static struct fuse_operations op;

int main(int argc, char *argv[]){
    int fd = open("test.tar", O_RDONLY);
    int lp = 0;
    int rec_file = 0;
    bool last = false;
    while(true){
        t1 aaa;
        int ret = read(fd, &aaa, 512);
        if((aaa.typeflag == '0' || aaa.typeflag == '5') && isdigit(aaa.size[0])){
            if(last) last = false;
            bool the_same = false;
            for(int i = 0; i < file_cnt; i++){
                if(strcmp(aaa.name, n_file[i].name) == 0){
                    long long now_time = octtoll(n_file[i].mtime);
                    long long a_time = octtoll(aaa.mtime);
                    if(now_time > a_time){
                        last = true;
                        break;
                    }
                    n_file[i] = aaa;
                    the_same = true;
                    rec_file = i;
                    lp = 0;
                    break;
                }
            }
            if(last) continue;
            if(the_same) continue;
            rec_file = file_cnt;
            content[file_cnt] = (char*)malloc(sizeof(char) * 10000);
            n_file[file_cnt++] = aaa;
            lp = 0;
        }
        else{
            if(last) continue;
            char tmp[520] = "";
            memcpy(tmp, &aaa, 512);
            for(int i = 0; i < ret; i++){
                content[rec_file][lp++] = tmp[i];
            }
        }
        if(ret < 512) break;
    }
    for(int i = 0; i < file_cnt; i++){
        int nl = strlen(n_file[i].name);
        for(int j = nl-1; j >= 0; j--){
            n_file[i].name[j+1] = n_file[i].name[j];
        }
        n_file[i].name[0] = '/';
        if(n_file[i].typeflag == '5'){
            n_file[i].name[nl] = '\0';
            m_path[path_cnt] = (char*)malloc(sizeof(char) * 1000);
            m_path[path_cnt++] = n_file[i].name;
        }
    }

    for(int i = 0; i < file_cnt; i++){
        if(n_file[i].typeflag == '0'){
            int nl = strlen(n_file[i].name);
            m_f[f_cnt] = (char*)malloc(sizeof(char) * 1000);
            int ind = 0;
            char tmp[100] = "";
            char tmp_p[1000] = "";
            for(int j = nl-1; j >= 0; j--){
                if(n_file[i].name[j] == '/'){
                    ind = j;
                    break;
                }
            }
            for(int j = 0; j < nl; j++){
                if(j < ind) tmp_p[j] = n_file[i].name[j];
                else if(j > ind) tmp[j-ind-1] = n_file[i].name[j];
            }
            m_f[f_cnt] = strdup(tmp);
            if(strlen(tmp_p) == 0) p[path_cnt].push_back(f_cnt);
            else{
                for(int j = 0; j < path_cnt; j++){
                    if(strcmp(m_path[j], tmp_p) == 0)
                        p[j].push_back(f_cnt);
                }
            }
            f_cnt++;
        }
        else if(n_file[i].typeflag == '5'){
            int nl = strlen(n_file[i].name);
            m_d[d_cnt] = (char*)malloc(sizeof(char) * 1000);
            int ind = 0;
            char tmp[100] = "";
            char tmp_d[1000] = "";
            for(int j = nl-1; j >= 0; j--){
                if(n_file[i].name[j] == '/'){
                    ind = j;
                    break;
                }
            }
            for(int j = 0; j < nl; j++){
                if(j < ind) tmp_d[j] = n_file[i].name[j];
                else if(j > ind) tmp[j-ind-1] = n_file[i].name[j];
            }
            m_d[d_cnt] = strdup(tmp);
            if(strlen(tmp_d) == 0) d[path_cnt].push_back(d_cnt);
            else{
                for(int j = 0; j < path_cnt; j++){
                    if(strcmp(m_path[j], tmp_d) == 0)
                        d[j].push_back(d_cnt);
                }
            }
            d_cnt++;
        }
    }
    memset(&op, 0, sizeof(op));
    op.getattr = my_getattr;
    op.readdir = my_readdir;
    op.read = my_read;
    return fuse_main(argc, argv, &op, NULL);
}