// TestDelegate3.cpp : 定义控制台应用程序的入口点。
//

#include "Delegate.hpp"
#include <cstdio>
#include <cstdlib>
#include <cassert>


Delegate<int, float&> d;

void fun1(int a, float& b)
{
	b += 1.0f;
}

class MyObj {
public:
	void memfn1(int a, float& b)
	{
		b += 1.3f;
	}
};

MyObj myobj;

int main()
{
	for (int i = 0; i < 1000; ++i) {
		d.add(&fun1);
		d.add(&myobj, &MyObj::memfn1);
	}

	int a = 1;
	float b = 0.1f;
	for (int i = 0; i < 1000; ++i) {
		d(a, b);
	}

	for (int i = 0; i < 1000; ++i) {
		d.remove(&fun1);
		d.remove(&myobj, &MyObj::memfn1);
	}

	std::printf("a = %d, b = %f\n", a, b);
    return 0;
}

