实验二 线程与同步
===

**小组成员：**

| 学号 | 姓名 | 专业 | 分工|
| --- | --- | --- | --- |
| 13720182200490 | 李少鹏 | 计算机 | 2.2 && 2.3 |
| 22920182204231 | 林枫 | 计算机 | 2.1.1 && 2.1.3.1 |
| 22920182204356 | 张加辉 | 计算机 | 2.1.2 && 2.1.3.2 |

# 1.实验内容简述
本次实验的目的在于将nachos中的锁机制和条件变量的实现补充完整，并利用这些同步机制实现几个基础工具类。实验内容分三部分：实现锁机制和条件变量，并利用这些同步机制将实验一中所实现双向有序链表类修改成线程安全的；实现一个线程安全的表结构；实现一个大小受限的缓冲区.
# 2.实验步骤与代码

## 2.1实现锁机制和条件变量
### 2.1.1用`Thread::Sleep`实现锁机制和条件变量
1. `Lock` 类定义

```C++
class Lock {
  public:
    Lock(char* debugName);  		
    ~Lock();				
    char* getName() { return name; }	

    void Acquire(); 
    void Release(); 
    bool isHeldByCurrentThread();	

  private:
    char* name;				
    bool status;  //资源当前是否空闲 
    List* queue;  //堵塞的线程队列
    Thread* holdingThread; //当前占有该资源的线程
};
```
在`Lock`类的定义中，我们增加了，`status`（表示该资源是否未被占有），`queue`（被堵塞的线程的队列），`holdingThread`（当前占有该资源的线程）。

2. `Lock`类具体函数实现

```C++
void Lock::Acquire()
{
    /*获取资源*/
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    while(status == false)//当前资源已被占有
    {
         queue->Append((void *)currentThread); //将该线程加入堵塞队列
         currentThread->Sleep();  //将当前线程挂起
    }
    //若资源空闲
    status = false; //占有资源
    holdingThread = currentThread; 
    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
}

void Lock::Release() 
{
    /*释放资源*/
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts

    thread = (Thread *)queue->Remove();//将一个线程从等待队列中移除
    if (thread != NULL)	{   
	   scheduler->ReadyToRun(thread);//将一个线程置于就绪队列
    }
    status = true;
    (void) interrupt->SetLevel(oldLevel); // re-enable interrupts
}

bool Lock::isHeldByCurrentThread()
{
    /*判断当前线程是否是资源的占有者*/
    return (currentThread == holdingThread);
}
```
在Lock函数的实现过程中，我们通过`interrupt->SetLevel()`函数来保证操作的原子性。同时通过`currentThread->Sleep()`函数来挂起线程，即将线程置入堵塞队列；通过`scheduler->ReadyToRun()`来唤醒一个线程。

3 `Conditon` 类定义

```C++
class Condition {
  public:
    Condition(char* debugName);		// initialize condition to 					// "no one waiting"
    ~Condition();			// deallocate the condition
    char* getName() { return (name); }
   
   //下面三个操作需满足当前线程是占有该资源的线程
    void Wait(Lock *conditionLock); 					/
    void Signal(Lock *conditionLock);   
    void Broadcast(Lock *conditionLock);
					
  private:
    char* name;
    List* queue;//等待队列
    // plus some other stuff you'll need to define
};

```
在`Condition`类中同样增加了`queue`来保存被挂起的线程。

4. `Conditon`类具体函数实现

```C++
void Condition::Wait(Lock* conditionLock) 
{ 
    assert(conditionLock->isHeldByCurrentThread());
    // still need off the interupt
    queue->Append((void *)currentThread);//加入等待队列
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    conditionLock->Release(); //解锁，使其余线程可继续执行
    currentThread->Sleep();  //挂起该线程
    (void) interrupt->SetLevel(oldLevel);
    conditionLock->Acquire();// 被唤醒后，重新获得锁
}
void Condition::Signal(Lock* conditionLock)
{
    Thread *thread;
    assert(conditionLock->isHeldByCurrentThread());
        thread = (Thread *)queue->Remove();
        if (thread != NULL) {   //若等待队列非空，将线程从等待队列中移除
            scheduler->ReadyToRun(thread);}//唤醒该线程
}
void Condition::Broadcast(Lock* conditionLock) 
{
    /*广播，将等待队列中的每个线程唤醒*/
    Thread *thread;
    assert(conditionLock->isHeldByCurrentThread());
    // wake up all the thread
    while (!queue->IsEmpty()) {
        thread = (Thread *)queue->Remove();
        if (thread != NULL) {   // make thread ready
            scheduler->ReadyToRun(thread);
        }
    }
}
```
由于采用`Mesa`语义，所以`Signal()`，`Broadcast()`函数在唤醒被阻塞队列中的线程后继续执行。
### 2.1.2用`Semaphore`实现锁机制和条件变量
#### 用`Semaphore`实现加锁机制

用信号量实现锁的关键在于将信号量的初值设置为1，这样才能保证同一时间能够占用被锁住的资源的线程只有一个。  
以下展示主要修改的/增加的部分代码。

1. 对`synch.h`的修改

   ```cpp
   class Lock {
   public:
       ...
   
   private:
       char *name;
       Semaphore *sem; // 信号量，用于实现锁
       Thread* currentLockHoldingThread;  // 记录占用当前锁的线程     
   };
   ```

2. 对`synch.cc`的修改

   ```cpp
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

#### 用`Semaphore`实现条件变量

实现条件变量需要配合使用锁，锁的作用为实现互斥，条件变量的所有操作(`Wait`, `Signal`, `Broadcast`)都需要一个锁，同一个条件变量使用同一个锁。

1. 对`synch.h`的修改

   ``` Cpp
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

   ``` Cpp
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

### 2.1.3用锁机制和条件变量修改双向有序链表

修改`dllist.cc`和`threadtest.cc`，实现线程安全

#### 2.1.3.1使用`Thread::Sleep`测试

#### 2.1.3.2使用`Semaphore`测试

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

## 2.2实现一个线程安全的表结构

1. `Table.h`

   ```cpp
   class Table
   {
   public:
     // create a table to hold at most 'size' entries.
     Table(int size); // 指定参数size的类型为int
     ~Table(); // 析构函数的声明
     // allocate a table slot for 'object'.
     // return the table index for the slot or -1 on error.
     int Alloc(void *object);
   
     // return the object from table index 'index' or NULL on error.
     // (assert index is in range).  Leave the table entry allocated
     // and the pointer in place.
     void *Get(int index);
   
     // free a table slot
     void Release(int index);
   
   private:
     // Your code here.
     void** table;
     int size; // Table大小
     Lock* lock; // 互斥锁
     int remain; // 剩余可分配的entries数
   };
   ```

2. `Table.cc`

   实现其中的函数

   ```cpp
   // 构造函数
   Table::Table(int size)
   {
       this->size = remain = size;
       table = new void*[size]();
       lock = new Lock("Table lock");
   }
   // 析构函数
   Table::~Table()
   {
       delete lock;
       delete[] table;
   }
   // 将object存入table
   int Table::Alloc(void *object)
   {
       lock->Acquire();
       int index = -1;
       // 若无空间，则返回-1
       // 若还有空间，查找空的位置插入object，更新remain
       if (!remain)
           return index;
       else
       {
           while (table[++index] != NULL);
           table[index] = object;
           --remain;
       }
       lock->Release();
       return index;
   }
   // 获取指定index的object
   void* Table::Get(int index)
   {
       // 先检测index是否合法
       assert(index >= 0 && index < size);
       lock->Acquire();
       void *res = table[index];
       lock->Release();
       return res;
   }
   // 释放指定index的object
   void Table::Release(int index)
   {
       // index处置空
       assert(index >= 0 && index < size);
       lock->Acquire();
       table[index] = NULL;
       lock->Release();
   }
   ```

3. 对线程安全的表进行测试

   在`threadtest.cc`文件中添加`tableTest(int which)`函数，进行测试
   
   ```cpp
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
   ```
   
   ![](./pic/tableTest.png)

## 2.2实现一个大小受限的缓冲区

1. `BoundedBuffer.h`

   ```cpp
   class BoundedBuffer
   {
     public:
     ...
     
     private:
     int *buffer;
     int maxsize;
     int in, out, count;
     // 锁与条件变量实现
     Lock *lock;
     Condition *full, *empty;
     // 信号量实现
     Semaphore s, n, e;
   };
   ```

2. `BoundedBuffer.cc`

   ```cpp
   BoundedBuffer::BoundedBuffer(int maxsize)
   {
       this->maxsize = maxsize;
       buffer = new int[maxsize];
       lock = new Lock("Buffer lock");
       full = new Condition("Write full");
       empty = new Condition("Read empty");
       in = out = count = 0;
   }
   
   BoundedBuffer::~BoundedBuffer()
   {
       delete empty;
       delete full;
       delete lock;
       delete buffer;
   }
   
   void BoundedBuffer::Read(void *data, int size)
   {
       int *readData = (int*)data;
       for (int i = 0; i < size; ++i)
       {
       	lock->Acquire();
           while (count == 0)
               empty->Wait(lock);
           readData[i] = buffer[out];
           out = (out + 1) % maxsize;
           --count;
           full->Broadcast(lock);
       	lock->Release();
       }
   }
   
   void BoundedBuffer::Write(void *data, int size)
   {
       int *writeData = (int*)data;
       for (int i = 0; i < size; ++i)
       {
       	lock->Acquire();
           while (count == maxsize)
               full->Wait(lock);
           buffer[in] = writeData[i];
           in = (in + 1) % maxsize;
           ++count;
           empty->Signal(lock);
       	lock->Release();
       }
   }
   // 输出buffer，以便测试
   void BoundedBuffer::printBuffer()
   {
       int i = out, cnt = count;
       printf("\t\tthe buffer is: ");
       if(cnt == 0)
           printf("null");
       else
           while (cnt--)
           {
               printf("%d ", buffer[i]);
               i = (i + 1) % maxsize;
           }
   	printf("\n");
   }
   
   ```
   
3. 对有界缓冲区测试

   在`threadtest.cc`中添加`bufferTest(int which)`函数

   ```cpp
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
   ```

   ![](pic/bufferTest.png)

# 3.遇到问题与解决

# 4.实验小结