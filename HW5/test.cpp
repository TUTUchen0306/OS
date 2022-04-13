#include<iostream>
#include<queue>
#define pii pair<int, int>
#define F first
#define S second

using namespace std;

priority_queue<pii, vector<pii>, greater<pii> >pq;

int main(){
	pq.push(make_pair(1, 2));
	pq.push(make_pair(3, 1));
	pq.push(make_pair(2, 4));
	pq.push(make_pair(1, 5));
	while(pq.size()){
		cout << pq.top().F << ' ' << pq.top().S << '\n';
		pq.pop();
	}
} 