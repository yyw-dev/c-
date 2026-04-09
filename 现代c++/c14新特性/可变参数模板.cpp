#include<iostream>
using namespace std;

void print()
{
	
}


template<typename T,typename... Types>
void print(const T& firstArg,const Types&... args)
{
	cout<<firstArg<<endl;
	print(args...);
}

int main()
{
	print(7.5,"hello",true,42);
	return 0;
}