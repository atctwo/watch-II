/******************************************************************************

Welcome to GDB Online.
GDB online is an online compiler and debugger tool for C, C++, Python, Java, PHP, Ruby, Perl,
C#, VB, Swift, Pascal, Fortran, Haskell, Objective-C, Assembly, HTML, CSS, JS, SQLite, Prolog.
Code, Compile, Run and Debug online from anywhere in world.

*******************************************************************************/
#include <stdio.h>

//w2_imu
class A
{
    public:
        virtual int thing() = 0;
};

//w2_imu_mpu6050
class B : public A
{
    public:
        int thing() override;
};

int B::thing()
{
    return 7;
}

//w2
class C
{
    public:
        C()
        {
            //something
        }
        A* obj;
};

//w2_esp32
class D : public C
{
    public:
    
        D() : C()
        {
            obj = new B();
        }
};


int main()
{
    D d;
    printf("%d", d.obj->thing());

    return 0;
}
