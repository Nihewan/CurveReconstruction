#pragma once

template<typename T, int(*cmp)(const T &x, const T &y)>
class CBinaryHeap
{
public:        
	int N, n;
	T *a;        
public:
	CBinaryHeap(int i_n) : N(i_n), n(0){ 
		a = (T*)malloc(N*sizeof(T));
		memset(a, 0, N*sizeof(T));
	}
	~CBinaryHeap(){ free(a); }
	void clear(){ n = 0; }
	bool empty() {return n == 0;}
	void insert(T t){ /* n<N */ a[n++] = t; up(n-1); }
	int	 find(T t)
	{
		for (int i = 0; i < n; ++i) {
			if (a[i] == t)
				return i;
		}
		return -1;
	}
	void del(int pos){ /* 0<=pos<n */ n--; modify(pos, a[n]); }
	void modify(int pos, T t){ /* 0<=pos<n */
		int tcmp = cmp(t, a[pos]);
		a[pos] = t;
		if (tcmp < 0) up(pos);
		else if (tcmp > 0) down(pos);
	}
	T top(){ /* n!=0 */ return a[0]; }

	void up(int pos)
	{
		int j;
		T t;
		while (pos > 0)
		{
			j = (pos-1)/2;
			if (cmp(a[pos], a[j]) < 0)
			{
				t = a[pos];
				a[pos] = a[j];
				a[j] = t;
				pos = j;
			}
			else break;
		}
	}
	void down(int pos)
	{
		int j, k;
		T t;
		for(;;)
		{
			j = pos;
			k = pos*2+1;
			if (k < n && cmp(a[j], a[k]) > 0) j = k;
			k = pos*2+2;
			if (k < n && cmp(a[j], a[k]) > 0) j = k;
			if (j == pos) break;
			t = a[pos];
			a[pos] = a[j];
			a[j] = t;
			pos = j;
		}
	}
};