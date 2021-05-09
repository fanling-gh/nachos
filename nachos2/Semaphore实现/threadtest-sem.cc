// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "dllist.h"
#include "synch.h"
#include <ctime>
// testnum is set in main.cc
int testnum = 1;
// threadNum is set in main.cc
int threadNum = 2;
// itemNum is set in main.cc
int itemNum = 2;

DLList *dList = new DLList();
static int currentItemNum = 0;
extern void Driver_test();
extern void Generate_nItems(const int &N, DLList *list);
extern void Remove_nItems(const int &N, DLList *list);
Lock *threadLock = new Lock("lock used to mutex different threads");
Lock *dllistLock = new Lock("lock for dllist operations");
Condition *cIsEmpty = new Condition("condition: is dllist empty?");

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;

    for (num = 0; num < 5; num++) {
//        std::cout << "*** thread " << which << " looped " << num << " times\n";
        printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

// 这里实现了使用semaphore的锁与条件变量形成的互斥，其他函数尚未实现
void
Test1(int which)
{
    threadLock->Acquire();
    while (currentItemNum != 0) {
        cIsEmpty->Wait(threadLock);
    }

    printf("*** Inserting items in thread %d\n", which);
    Generate_nItems(itemNum, dList);
    currentItemNum += itemNum;

    currentThread->Yield(); // Yield here

    printf("*** Removing items in thread %d\n", which);
    Remove_nItems(itemNum, dList);
    currentItemNum -= itemNum;
    if (currentItemNum == 0)
        cIsEmpty->Signal(threadLock);

    threadLock->Release();
}

//
void
Test2(int which)
{
    for(int i = 0; i < itemNum; ++i)
    {
        printf("*** Inserting item No.%d in thread %d\n", i + 1, which);
        Generate_nItems(1, dList);
    }
    printf("*** Removing item in thread %d\n", which);
    Remove_nItems(itemNum, dList);
}

//
void
Test3(int which)
{
    printf("*** Inserting item in thread %d\n", which);
    Generate_nItems(itemNum, dList);
    currentThread->Yield();
    for(int i = 0; i < itemNum; i++){
        printf("*** Removing item No.%d in thread %d\n", i + 1, which);
        Remove_nItems(1, dList);
    }
}


// Table test
void
tableTest(int which)
{
    // int size = threadNum * itemNum;
    // int indexs[size];
    // Table *table(size);

    // // 插入
    // srand(static_cast<unsigned>(time(0)));
    // for (int i = 0; i < size; ++i)
    // {
    //     void *object = (void*)(rand() % max_key);
    //     indexs[i] = table->Alloc(object);
    //     printf("*** Object:%d stored at index[%d] in thread %d\n", (int)*object, indexs[i], which);
    //     currentThread->Yield();
    // }
    // // 获取
    // for (int i = 0; i < size; ++i)
    // {
    //     printf("*** Get object:%d stored at index[%d] in thread %d\n", (int)table->Get(indexs[i]), indexs[i], which);
    //     currentThread->Yield();
    // }
    // // 释放
    // for (int i = 0; i < size; ++i)
    // {
    //     table->Release(indexs[i]);
    //     printf("*** Release object stored at index[%d] in thread %d\n", indexs[i], which);
    //     currentThread->Yield();
    // }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
Test(VoidNoArgFunctionPtr functionPtr){ functionPtr(); }


char msg[30] = "forked thread ";
void
Test(VoidFunctionPtr functionPtr)
{
    DEBUG('t', "Entering threadTest");
    Thread *t;
    char num[5];
    for(int i = 0; i < threadNum; ++i)
    {
        sprintf(num, "%d", i + 1);
        t = new Thread(strcat(msg, num));
		t->Fork(functionPtr, i + 1);
    }
}


//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
        case 1:
            Test(SimpleThread); break;
        case 2:
            Test(Driver_test); break;
        case 3:
            Test(Test1); break;
        case 4:
            Test(Test2); break;
        case 5:
            Test(Test3); break;
        case 6:
            Test(tableTest); break;
        default:
            printf("No test specified.\n");
            break;
    }
}

