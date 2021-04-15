#ifndef _DLLIST_H_
#define _DLLIST_H_

class DLLElement
{
public:
    DLLElement(void *itemPtr, int sortKey); // initialize a list element
    DLLElement(){}
    DLLElement *next; // next element on list
                      // NULL if this is the last
    DLLElement *prev; // previous element on list
                      // NULL if this is the first
    int key;          // priority, for a sorted list
    void *item;       // pointer to item on the list
};
class DLList
{
public:
    DLList();                 // initialize the list
    ~DLList();                // de-allocate the list
    void Prepend(void *item); // add to head of list (set key = min_key-1)
    void Append(void *item);  // add to tail of list (set key = max_key+1)
    void *Remove(int *keyPtr); // remove from head of list
                              // set *keyPtr to key of the removed item
                              // return item (or NULL if list is empty)
    bool IsEmpty();           // return true if list has elements
    // routines to put/get items on/off list in order (sorted by key)
    void SortedInsert(void* item, int sortKey);
    void *SortedRemove(int sortKey); // remove first item with key==sortKey
    // return NULL if no such item exists
private:
    DLLElement *first; // head of the list, NULL if empty
    DLLElement *last;  // last element of the list, NULL if empty
};

void Generate_nItems(const int &N, DLList *list);
void Remove_nItems(const int &N, DLList *list);

//the pid_max of os is 32768
const int min_key = 0;
const int max_key = 32767;

#endif
