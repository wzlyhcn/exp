// TestDelegate3.cpp : 定义控制台应用程序的入口点。
//

#include "Delegate.hpp"
#include <cstdio>
#include <cstdlib>
#include <cassert>


Delegate<int, float&> d;

class MyObj : public Delegatable {
public:
	MyObj(int id) :
		m_id(id)
	{
	}

	void memfn1(int a, float& b)
	{
		std::printf("MyObj::memfn1() - id = %d\n", m_id);
		b += 1.3f;
	}

	void memfn2(int a, float& b)
	{
		std::printf("MyObj::memfn2() - id = %d\n", m_id);
		b -= 1.3f;
	}
private:
	int m_id;
};

MyObj myobj1(1), myobj2(2);

int main()
{
	d.bind(&myobj1, &MyObj::memfn1);
	d.bind(&myobj1, &MyObj::memfn2);

	assert(d.exists(&myobj1, &MyObj::memfn1) == true);
	assert(d.exists(&myobj1, &MyObj::memfn2) == true);

	d.bind(&myobj2, &MyObj::memfn1);
	d.bind(&myobj2, &MyObj::memfn2);

	assert(d.exists(&myobj2, &MyObj::memfn1) == true);
	assert(d.exists(&myobj2, &MyObj::memfn2) == true);

	int a = 1;
	float b = 0.1f;
	for (int i = 0; i < 10; ++i) {
		d(a, b);
	}

	myobj1.unbindAll();

	assert(d.exists(&myobj1, &MyObj::memfn1) == false);
	assert(d.exists(&myobj1, &MyObj::memfn2) == false);

	myobj2.unbindAll();

	assert(d.exists(&myobj2, &MyObj::memfn1) == false);
	assert(d.exists(&myobj2, &MyObj::memfn2) == false);

	std::printf("a = %d, b = %f\n", a, b);
#if _WIN32
	std::system("pause");
#endif
	return 0;
}

