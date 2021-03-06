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
// testnum is set in main.cc
int testnum = 1;
// threadNum is set in main.cc
int threadNum = 2;
// itemNum is set in main.cc
int itemNum = 2;

DLList *dList = new DLList();
extern void Driver_test();
extern void Generate_nItems(const int &N, DLList *list);
extern void Remove_nItems(const int &N, DLList *list);
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

// Share
void
Test1(int which)
{
    printf("*** Inserting items in thread %d\n", which);
    Generate_nItems(itemNum, dList);
    currentThread->Yield(); // Yield here
    printf("*** Removing items in thread %d\n", which);
    Remove_nItems(itemNum, dList);
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
        default:
            printf("No test specified.\n");
            break;
    }
}

