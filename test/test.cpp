#include <iostream>
#include "..\\memtracer\\include\\memory_tracer.h"

void* operator new(size_t size)
{
    return memtracer::MemoryTracer<>::get_instance()->add_allocation(size);
}

void operator delete(void* p)
{
	memtracer::MemoryTracer<>::get_instance()->remove_allocation(p);
}

class TestClass
{
public:
    TestClass()
    {
        std::cout << "Test Class Constructor" << std::endl;
    }

    ~TestClass()
    {
        std::cout << "Test Class Destructor" << std::endl;
    }

    void TestClassFunction()
    {
        std::cout << "TestClassFunction" << std::endl;
    }
};

void ThreadTest()
{
    std::cout << "Thread Test Function" << std::endl;
}

int main()
{
    memtracer::MemoryTracer<>::get_instance()->start();

    TestClass* a = new TestClass();

    memtracer::MemoryTracer<>::get_instance()->take_snapshot();

    delete a;

    memtracer::MemoryTracer<>::get_instance()->take_snapshot();

    memtracer::MemoryTracer<>::get_instance()->stop();
}
