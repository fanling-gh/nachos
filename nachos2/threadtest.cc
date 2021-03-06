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
#include "Table.h"
#include "BoundedBuffer.h"
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
// 通过gdb debug得出：table应设为全局变量，否则会使currentThread丢失，原因尚未得知
Table *table = new Table(threadNum * itemNum);
BoundedBuffer *buffer = new BoundedBuffer(15);
int data[] = {1,3,4,13,12,17,18,23,19,20,13,33,27,43,26,21,16,14,10,29};

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void SimpleThread(int which)
{
    int num;

    for (num = 0; num < 5; num++)
    {
        //        std::cout << "*** thread " << which << " looped " << num << " times\n";
        printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

// 这里实现了使用semaphore的锁与条件变量形成的互斥，其他函数尚未实现
void Test1(int which)
{
    threadLock->Acquire();
    while (currentItemNum != 0)
    {
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
void Test2(int which)
{
    for (int i = 0; i < itemNum; ++i)
    {
        printf("*** Inserting item No.%d in thread %d\n", i + 1, which);
        Generate_nItems(1, dList);
    }
    printf("*** Removing item in thread %d\n", which);
    Remove_nItems(itemNum, dList);
}

//
void Test3(int which)
{
    printf("*** Inserting item in thread %d\n", which);
    Generate_nItems(itemNum, dList);
    currentThread->Yield();
    for (int i = 0; i < itemNum; i++)
    {
        printf("*** Removing item No.%d in thread %d\n", i + 1, which);
        Remove_nItems(1, dList);
    }
}

// Table test
void tableTest(int which)
{
    int indexs[threadNum];

    // 插入
    //srand(static_cast<unsigned>(time(0)));
    for (int i = 0; i < threadNum; ++i)
    {
        void *object = new int((Random() % max_key));
        indexs[i] = table->Alloc(object);
        printf("*** Object:%d stored at index[%d] in thread %d\n", *(int*)(object), indexs[i], which);
        currentThread->Yield();
    }
    // 获取
    for (int i = 0; i < threadNum; ++i)
    {
        printf("*** Get object:%d stored at index[%d] in thread %d\n", (int)table->Get(indexs[i]), indexs[i], which);
        currentThread->Yield();
    }
    // 释放
    for (int i = 0; i < threadNum; ++i)
    {
        table->Release(indexs[i]);
        printf("*** Release object stored at index[%d] in thread %d\n", indexs[i], which);
        currentThread->Yield();
    }
}

// BoundedBuffer test
void bufferTest(int which)
{
    // odd profucer, even consumer
    if (which % 2) 
    {
		printf("***producer[thread %d]", which);
        printf("\n\tBefore write:\n");
		buffer->printBuffer();
		//int size = Random() % 10;
        int size = 2;
		buffer->Write(data, size);
        printf("\tAfter write:\n");
        buffer->printBuffer();
    }
    else
    {
        printf("***consumer[thread %d]", which);
        printf("\n\tBefore read:\n");
        buffer->printBuffer();
		//int size = Random() % 10;
        int size = which;
		int *read = new int(size);
        buffer->Read(read, size);
        printf("\tread: ");
        for(int i = 0; i < size; ++i)
            printf("%d ", read[i]);
        printf("\n\tAfter read:\n");
        buffer->printBuffer();
        printf("***consumer[thread %d] finished reading\n", which);
    }
}   

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void Test(VoidNoArgFunctionPtr functionPtr) { functionPtr(); }

char msg[30] = "forked thread ";
void Test(VoidFunctionPtr functionPtr)
{
    DEBUG('t', "Entering threadTest");
    Thread *t;
    char num[5];
    for (int i = 0; i < threadNum; ++i)
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

void ThreadTest()
{
    switch (testnum)
    {
    case 1: // default test
        Test(SimpleThread);
        break;
    case 2: // test dllist-driver.cc
        Test(Driver_test);
        break;
    case 3: // test exp1-1
        Test(Test1);
        break;
    case 4: // test exp1-2
        Test(Test2);
        break;
    case 5: // test exp1-3
        Test(Test3);
        break;
    case 6: // test Table
        Test(tableTest);
        break;
    case 7: // test BoundedBuffer
        Test(bufferTest);
        break;
    default:
        printf("No test specified.\n");
        break;
    }
}
