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

}

void BoundedBuffer::Write(void *data, int size)
{

}