#include "BoundedBuffer.h"

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
