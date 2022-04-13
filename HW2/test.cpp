#include<iostream>

using namespace std;

#define TEST 100

unsigned int a[TEST+10][TEST+10];
unsigned int ans[TEST+10][TEST+10];

int main(){
	for(int i = 0; i < TEST; i++)
		for(int j = 0; j < TEST; j++)
			a[i][j] = TEST * i + j;

	for(int i = 0; i < TEST; i++){
		for(int j = 0; j < TEST; j++){
			unsigned int cnt = 0;
			for(int k = 0; k < TEST; k++){
				cnt += a[i][k] * a[k][j];
			}
			ans[i][j] = cnt;
		}
	}
	unsigned int res = 0;
	for(int i = 0; i < TEST; i++)
		for(int j = 0; j < TEST; j++)
			res += ans[i][j];
	cout << res << '\n';
	return 0;
}