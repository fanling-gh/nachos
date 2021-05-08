#include "BoundedBuffer.h"
// #include "synch.h"

BoundedBuffer::BoundedBuffer(int maxsize)
{
    this->maxsize = maxsize;
    buffer = new int[maxsize];
    lock = new Lock("Buffer lock");
    full = new Condition();
    empty = new Condition();
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
    lock->Acquire();
    for (int i = 0; i < size; ++i)
    {
        while (count == 0)
            empty->Wait(lock);
        readData[i] = buffer[out];
        out = (out + 1) % maxsize;
        --count;
        full->Signal(lock);
    }
    lock->Release();
}

void BoundedBuffer::Write(void *data, int size)
{
    int *writeData = (int*)data;
    lock->Acquire();
    for (int i = 0; i < size; ++i)
    {
        while (count == maxsize)
            full->Wait(lock);
        buffer[in] = writeData[i];
        in = (in + 1) % maxsize;
        ++count;
        empty->Signal(lock);
    }
    lock->Release();
}
