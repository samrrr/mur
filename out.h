#include <conio.h>
#include <stdio.h>
#include <malloc.h>
#include <string>
#include <vector>
#include <deque>

using namespace std;

class UUUte
{

public:
	UUUte(){
		for (int i = 0; i < 40; i++)
			s.push_front(":");
	}
	deque<string> s;
	void put(string _s)
	{
		s.push_front(_s);
		s.pop_back();
	}
	void set(int i,string _s)
	{
		s[i] = _s;
	}

};
extern UUUte out;