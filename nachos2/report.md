我们的报告写在这里，请孩子们写下必要思路与文字说明。使用到的图片请上传到`nachos2/pic/`文件夹下
===

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
Lock 类定义
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
    bool status;      
    List* queue;
    Thread* holdingThread;
};
```
在Lock类的定义中，我们增加了，status（表示该资源是否被占有），queue（被堵塞的线程的队列），holdingThread（当前占有该资源的线程）。

Lock类具体函数实现
```C++
void Lock::Acquire()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    while(status == false)
    {
         queue->Append((void *)currentThread); 
         currentThread->Sleep();
    }
    status = false;
    holdingThread = currentThread;
    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
}

void Lock::Release() 
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	{   // make thread ready
	   scheduler->ReadyToRun(thread);
    }
    status = true;
    (void) interrupt->SetLevel(oldLevel);
}

bool Lock::isHeldByCurrentThread()
{
    return (currentThread == holdingThread);
}
```
在Lock函数的实现过程中，我们通过“interrupt->SetLevel()”函数来保证操作的原子性。同时通过“currentThread->Sleep()”函数来挂起线程，即将线程置入堵塞队列；通过“scheduler->ReadyToRun()”来唤醒一个线程。

Conditon 类定义
```C++
class Condition {
  public:
    Condition(char* debugName);		// initialize condition to 
					// "no one waiting"
    ~Condition();			// deallocate the condition
    char* getName() { return (name); }
    
    void Wait(Lock *conditionLock); 	// these are the 3 operations on 
					// condition variables; releasing the 
					// lock and going to sleep are 
					// *atomic* in Wait()
    void Signal(Lock *conditionLock);   // conditionLock must be held by
    void Broadcast(Lock *conditionLock);// the currentThread for all of 
					// these operations

  private:
    char* name;
    List* queue;
    // plus some other stuff you'll need to define
};

```
在Codition类中同样增加了“queue”来保存被挂起的线程。

Coditon类具体函数实现
```C++
void Condition::Wait(Lock* conditionLock) 
{ 
     assert(conditionLock->isHeldByCurrentThread());
    // still need off the interupt
    queue->Append((void *)currentThread);
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    conditionLock->Release();
    currentThread->Sleep();
    (void) interrupt->SetLevel(oldLevel);
    conditionLock->Acquire();// restart to require lock 
}
void Condition::Signal(Lock* conditionLock)
{
    Thread *thread;
    assert(conditionLock->isHeldByCurrentThread());
        thread = (Thread *)queue->Remove();
        if (thread != NULL) {   // make thread ready
            scheduler->ReadyToRun(thread);}
}
void Condition::Broadcast(Lock* conditionLock) 
{
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
由于采用Mesa语义，所以Signal()，Broadcast()函数在唤醒被阻塞队列中的线程后继续执行。
### 2.1.2用`Semaphore`实现锁机制和条件变量

### 2.1.3用锁机制和条件变量修改双向有序链表

#### 2.1.3.1使用`Thread::Sleep`测试

#### 2.1.3.2使用`Semaphore`测试

## 2.2实现一个线程安全的表结构

## 2.2实现一个大小受限的缓冲区

# 3.遇到问题与解决

# 4.实验小结
