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
        assert(conditionLock->isHeldByCurrentThread()); // 使用断言强制要求当前进程是控制着该锁的
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


