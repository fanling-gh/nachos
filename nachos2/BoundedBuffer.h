// #include "synch.h"
#ifndef _BOUNDEDBUFFER_H_
#define _BOUNDEDBUFFER_H_

class Lock
{
public:
  Lock(const char *);
  void Acquire();
  void Release();
};
class Condition
{
public:
  Condition();
};
class Semaphore
{

};
class BoundedBuffer
{
public:
  // create a bounded buffer with a limit of 'maxsize' bytes
  BoundedBuffer(int maxsize);

  // read 'size' bytes from the bounded buffer, storing into 'data'.
  // ('size' may be greater than 'maxsize')
  void Read(void *data, int size);

  // write 'size' bytes from 'data' into the bounded buffer.
  // ('size' may be greater than 'maxsize')
  void Write(void *data, int size);

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
#endif 