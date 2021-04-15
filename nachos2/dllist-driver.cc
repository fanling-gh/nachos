#include "dllist.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

extern int itemNum;
// generates N items with random keys
// inserts them into a doubly-linked list
void Generate_nItems(const int &N, DLList *list)
{
	static int delta = 0;
	++delta;
    srand(static_cast<unsigned>(time(0) + delta));
    int nKey;
    for(int i = 0; i < N; ++i)
    {

        int *nItem = new int();
        nKey = rand() % (max_key + 1);
        *nItem = nKey % 127;
        std::cout << "Insert item: " << *nItem << ", key: " << nKey << std::endl;
        list->SortedInsert(nItem, nKey);
}
}

// removes N items starting from the head of the list 
// prints out the removed items to the console
void Remove_nItems(const int &N, DLList *list)
{
    void *nRes;
    int nKey;
    for(int i = 0; i < N; ++i)
    {
        nRes = list->Remove(&nKey);
        if(nRes)
            std::cout << "Remove item: " << *(int*)nRes << ", key: " << nKey << std::endl;
        else
        {
            std::cout << "have removed " << i+1 << " items, the list is empty now." << std::endl;
            break;
        }
    }
}

void Driver_test()
{
    DLList *dList = new DLList;
    Generate_nItems(2 * itemNum, dList);
    Remove_nItems(itemNum, dList);
}
