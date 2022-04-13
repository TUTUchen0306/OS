#include<bits/stdc++.h>
#define n 250000
using namespace std;

int a[n+10];
ofstream fout;
ofstream fans;

int main(){
	fout.open("input.txt");
	fans.open("hw3_ans.txt");
	fout << n << '\n';
	for(int i = 0; i < n; i++)
		a[i] = i;
	random_shuffle(a, a+n);
	for(int i = 0; i < n; i++)
		fout << a[i] << ' ';
	sort(a, a+n);
	for(int i = 0; i < n; i++)
		fans << a[i] << ' ';
	fans << '\n';
	return 0; 

}