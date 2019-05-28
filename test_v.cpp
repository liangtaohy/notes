#include <iostream>

#define GETVPTR(obj) \
	(long *)*((long *)obj + 0)

class Base1 {
public:
	Base1(int x, int y):x(x),y(y) {}
public:
	int x;
	int y;
};

class Base2 {
};

class Base4 {
public:
	virtual void say() {
		std::cout << "Base4 say hello" << std::endl;
	}

	virtual void fuck() {
		std::cout << "Base4 say WTF" << std::endl;
	}
};

class Base5 {
public:
	virtual void hi() {
		std::cout << "Base5 say hi" << std::endl;
	}
};

class Base3 {
public:
	Base3():x(0),y(0) {}
	Base3(int x, int y): x(x),y(y) {}

	virtual int incr() {
		return x++;
	}
	
	virtual int get() {
		return x + y;
	}
protected:
	int x;
	int y;
};

class Derive1: public Base4
{
};

class Derive2: public Base4, Base5
{
};

class Derive3: public Base3
{
public:
	virtual int get() {
		return x - y;
	}
};

typedef void(*vfunPtr)();

int main(void)
{
	Derive1 * d1 = new Derive1();
	Derive2 d2;
	Base3* b1 = new Base3();
	Base3* b2 = new Derive3();

	std::cout << "pointer size: " << sizeof(void *) << std::endl;
	std::cout << "int size: " << sizeof(int) << std::endl;
	std::cout << "Base1 size: " << sizeof(Base1) << std::endl;
	std::cout << "Base2 (empty class) size: " << sizeof(Base2) << std::endl;
	std::cout << "Base3 size: " << sizeof(Base3) << std::endl;
	std::cout << "Base4 (empty,virtual class) size: " << sizeof(Base4) << std::endl;
	std::cout << "Derive1 from Base4 size: " << sizeof(Derive1) << std::endl;
	std::cout << "Derive2 from Base4, Base5 size: " << sizeof(Derive2) << std::endl;

	std::cout << "vtable pointer: " << (long *)d1 << "; value=" << (long *)*((long *)d1 + 0) << std::endl;
	long * vptr = GETVPTR(d1);
	vfunPtr fun = (vfunPtr)*((long *)*(long *)d1);
	fun();
	vfunPtr fun2 = (vfunPtr)*(vptr + 1);
	fun2();
	return 0;
}