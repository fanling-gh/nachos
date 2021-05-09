# 用Semaphore实现互斥

## 用Semaphore实现加锁机制

---

用信号量实现锁的关键在于将信号量的初值设置为1，这样才能保证同一时间能够占用被锁住的资源的线程只有一个。  
以下展示主要修改的/增加的部分代码。

1. 对`synch.h`的修改

    ``` C
    class Lock {
    public:
        ...

    private:
        char *name;
        Semaphore *sem; // 信号量，用于实现锁
        Thread* currentLockHoldingThread;  // 记录占用当前锁的线程     
    };
    ```

2. 对`synch.cc`的修改（实现各函数）

    ``` C
    // 初始化锁使其处于可用状态
    Lock::Lock(char* debugName)
    {
        name = debugName; 
        sem = new Semaphore(debugName, 1);
         // 信号量设为1，确保只有一个线程能够占用锁
        currentThread = NULL;
    }

    // 占用锁
    void Lock::Acquire()
    {   
        sem->P();
        currentLockHoldingThread = currentThread; 
         // 确定当前占用锁的线程为此线程
    }

    // 释放锁
    void Lock::Release()
    {
        sem->V();
    }

    /*
        True if the current thread holds this lock. Useful for checking in
        Release, and in Condition variable operations.
    */
    bool Lock::isHeldByCurrentThread()
    {
        return (currentThread == currentLockHoldingThread);
    }
    ```

## 用Semaphore实现条件变量

---

实现条件变量需要配合使用锁，锁的作用为实现互斥，条件变量的所有操作(`Wait`, `Signal`, `Broadcast`)都需要一个锁，同一个条件变量使用同一个锁。

1. 对`synch.h`的修改

    ``` C
    class Condition {
    public:
        ...

    private:
        char *name;
        int numWaiting; // 在当前条件变量的等待队列中阻塞的线程数
        Semaphore *sem; // 用于阻塞线程的信号量
    };
    ```

2. 对`synch.cc`的修改

    ``` C
    // 构造函数
    Condition::Condition(char* debugName)
    { 
        name = debugName;
        sem = new Semaphore(debugName , 0); // 用于阻塞线程的信号量，初始化为0使调用Wait时必然阻塞
        currentThread = NULL; 
        numWaiting = 0; // 在阻塞队列中等待的线程数，使用这个变量能够使调用Signal的效果不积累
    }

    // 析构函数
    Condition::~Condition()
    {
        delete sem;
    }

    // Wait函数
    void Condition::Wait(Lock* conditionLock)
    { 
        // 在调用此函数之前必然要先conditionLock->Acquire()获取锁
        // 进入管程（拥有锁的控制）
        assert(conditionLock->isHeldByCurrentThread()); // 使用断言强制要求当前线程是控制着该锁的
        conditionLock->Release(); // 释放锁，允许其他线程进入管程
        numWaiting++; 
        sem->P(); // 线程被阻塞在这里

        // 解除阻塞（被唤醒）之后
        numWaiting--;
        conditionLock->Acquire(); // 重新获取管程的锁，禁止其他线程进入管程

        // 进入临界区（调用该函数之后）
    }

    // Signal函数
    void Condition::Signal(Lock* conditionLock)
    {
        assert(conditionLock->isHeldByCurrentThread());
        // 如果队列中没有线程，则什么也不做（与信号量V()的主要区别）
        if(numWaiting > 0){
            sem->V();
            numWaiting--; 
        }
        // Mesa语义：这个线程还会继续在管程中运行直到释放锁，随后被阻塞的线程才会获得锁与资源
    }

    // 广播函数：唤醒所有在此条件上阻塞的进程
    void Condition::Broadcast(Lock* conditionLock)
    {
        assert(conditionLock->isHeldByCurrentThread());
        for(int i = 0; i < numWaiting; i++){
            sem->V();
        }
    }
    ```

## 修改`DLList.cc`与`threadtest.cc`，做到线程安全

---

1. 对`DLList.cc`进行的修改：维护一个链表专用的锁`dllistLock`，对于所有需要修改链表数据（增加或减少）的操作，都使用

    ``` C
    dllistLock->Acquire();
    dllistLock->Release();
    ```

    将整个操作“包裹”起来，示例如下：

    ``` C
    void *DLList::Remove(int *keyPtr)
    {
        dllistLock->Acquire();
        if (!IsEmpty()) {
            cIsEmpty->Signal(dllistLock);
            dllistLock->Release();
            return NULL;
        }

        ......
        
        dllistLock->Release();
        return res;
        // 注意在所有return之前都要释放锁。
    }
    ```

    其他函数同理。

    如此一来，对链表的所有插入/删除操作都成为原子性的，不会再被其他线程打断。相当于我们的模拟线程切换行为  

    ``` C
    currentThread->Yield();  
    ```

    均无效化，因为其他线程得不到锁，都会被阻塞。

2. 对`threadtest.cc`的修改：维护一个用于线程之间的锁，和一个指示链表是否为空的条件变量

    ``` C
        Lock *threadLock = new Lock("lock used to mutex different threads");
        Condition *cIsEmpty = new Condition("condition: is dllist empty?");
    ```

    在我们的测试函数`Test1`中，由于所有线程共享同一个双向链表，所以各线程之间的切换会导致各个线程最后删除的item有可能并不是自己当初添加进去的item，而是别的线程添加的item。使用锁和条件变量可以解决这个问题：

    ``` C
    void
    Test1(int which)
    {
        threadLock->Acquire(); // 获取锁，形成各线程之间的互斥
        while (currentItemNum != 0) {
            cIsEmpty->Wait(threadLock); // 等待链表清空
        }

        printf("*** Inserting items in thread %d\n", which);
        Generate_nItems(itemNum, dList);
        currentItemNum += itemNum;

        currentThread->Yield(); // Yield here

        printf("*** Removing items in thread %d\n", which);
        Remove_nItems(itemNum, dList);
        currentItemNum -= itemNum;
        if (currentItemNum == 0)
            cIsEmpty->Signal(threadLock); // 链表清空后则发出信号，唤醒被阻塞的线程

        threadLock->Release();
    }
    ```

    这里我们使用锁实现互斥，并使用条件变量强制要求各线程在链表为空时才进行下一步操作：如果链表不为空就允许线程去添加删除的话，就必然会出现前面提到的问题。
