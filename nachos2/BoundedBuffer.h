#include "synch.h"
#ifndef _BOUNDEDBUFFER_H_
#define _BOUNDEDBUFFER_H_

class BoundedBuffer
{
public:
  // create a bounded buffer with a limit of 'maxsize' bytes
  BoundedBuffer(int maxsize);
  ~BoundedBuffer();
  // read 'size' bytes from the bounded buffer, storing into 'data'.
  // ('size' may be greater than 'maxsize')
  void Read(void *data, int size);

  // write 'size' bytes from 'data' into the bounded buffer.
  // ('size' may be greater than 'maxsize')
  void Write(void *data, int size);

  void printBuffer();

private:
  int *buffer;
  int maxsize; // 缓冲区最大长度
  int in, out, count; // in、out分别用于指向写入和读出的位置索引，count记录当前缓冲区中的数据个数
  // 锁与条件变量实现
  Lock *lock; // 用于控制只允许一个线程使用缓冲区
  Condition *full, *empty; // 读写时出现读空与写满的情况时需要依靠条件变量进行阻塞当前线程
  // 信号量实现
 // Semaphore s, n, e;
};
#endif 
