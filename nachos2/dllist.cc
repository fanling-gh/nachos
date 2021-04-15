#include "dllist.h"
#include "system.h"
#include <cstring>

extern int testnum;

DLLElement::DLLElement(void *itemPtr, int sortKey)
{
    item = itemPtr;
    key = sortKey;
    next = prev = NULL;
}

// init the dllist
// DLList::DLList() : first(NULL), last(NULL){};
DLList::DLList()
{
    first = new DLLElement();
    last = new DLLElement();
    first = last = NULL;
}

DLList::~DLList()
{
    DLLElement *item = first, *temp;
    while(item)
    {
        temp = item->next;
        delete item;
        item = temp;
    }
}
// add to head of list (set key = min_key - 1)
void DLList::Prepend(void *item)
{
    if (IsEmpty())
    {
        DLLElement *new_item = new DLLElement(item, first->key - 1);
        new_item->next = first;
        first->prev = new_item;
        first = new_item;
    }
    else
    {
        DLLElement *new_item = new DLLElement(item, min_key);
        first = last = new_item;
    }
}
// add to tail of list (set key = max_key + 1)
void DLList::Append(void *item)
{
    if (IsEmpty())
    {
        DLLElement *new_item = new DLLElement(item, last->key + 1);;
        new_item->prev = last;
        last->next = new_item;
        last = new_item;
    }
    else
    {
        DLLElement *new_item = new DLLElement(item, min_key);
        first = last = new_item;
    }
}
// remove from head of list
// set *keyPtr to key of the removed item
// return item (or NULL if list is empty)
void *DLList::Remove(int *keyPtr)
{
    if (!IsEmpty())
        return NULL;
    void *res;
    DLLElement *elem = first;
    if(testnum == 5)
        currentThread->Yield();
    
    first = first->next;
    if(first)
    {
        if(testnum == 5)
            currentThread->Yield();
        first->prev = NULL;
    }
    else
        first = last = NULL;
    
    elem->next = NULL;
    if(testnum == 5)
        currentThread->Yield();
    *keyPtr = elem->key;
    res = elem->item;
    delete elem;
    return res;
}
// return true if list has elements
bool DLList::IsEmpty()
{
    return ((!first && !last) ? FALSE : TRUE);
}
// routines to put/get items on/off list in order (sorted by key)
void DLList::SortedInsert(void *item, int sortKey)
{
    DLLElement *new_item = new DLLElement(item, sortKey);
    if (!IsEmpty())
    {
        if(testnum == 4)
        {
            currentThread->Yield();
			first = last = new_item;
			currentThread->Yield();
        }
        else
            first = last = new_item;
        return;
    }
    DLLElement *elem = first;
    while(elem)
    {
        if(elem->key < sortKey)
            elem = elem->next;
        else
            break;
    }
    if(elem)
    {
        new_item->next = elem;
        if(elem != first)
        {
            new_item->prev = elem->prev;
            elem->prev->next = new_item;
            elem->prev = new_item;
        }
        else
        {
            elem->prev = new_item;
            first = new_item;
        }
    }
    else
    {
        last->next = new_item;
        new_item->prev = last;
        new_item->next = NULL;
        last = new_item;
    }
}
// remove first item with key==sortKey
void *DLList::SortedRemove(int sortKey)
{
    if (!IsEmpty())
        return NULL;
    DLLElement *elem = first;
    void *res;
    while (elem != NULL && elem->key != sortKey)
        elem = elem->next;
    if (elem != NULL)
    {
        if (elem  == first)
        {
            first = elem->next;
            if(first)
                first->prev = NULL;
            else
                last = NULL;
            elem->next = NULL;

        }
        else if(elem == last)
        {
            last = elem->prev;
            last->next = NULL;
            elem->prev = NULL;
        }
        else 
        {
            elem->prev->next = elem->next;
            elem->next->prev = elem->prev;
        }
        res = elem->item;
        delete elem;
    }
    return res;
}
