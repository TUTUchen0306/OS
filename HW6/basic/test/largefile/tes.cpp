#include<iostream>
#include<fstream>
#include<string.h>

using namespace std;

int main(){
	ifstream fin;
	ofstream fout;
	fin.open("8mb.txt");
	fout.open("test.txt");
	string s;
	while(fin >> s){
		fout << s;
	}
	return 0;

}