#include "Table.h"
#include <assert.h>

Table::Table(int size)
{
    this->size = remain = size;
    table = new void*[size]();
    lock = new Lock("Table lock");
}

Table::~Table()
{
    delete lock;
    delete[] table;
}

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

void* Table::Get(int index)
{
    // 先检测index是否合法
    assert(index >= 0 && index < size);
    lock->Acquire();
    void *res = table[index];
    lock->Release();
    return res;
}

void Table::Release(int index)
{
    // index处置空
    assert(index >= 0 && index < size);
    lock->Acquire();
    table[index] = NULL;
    lock->Release();
}