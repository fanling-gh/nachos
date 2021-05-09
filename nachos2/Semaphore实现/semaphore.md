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

使用信号量实现条件变量较为棘手。

## 修改DLList使它为线程安全的
