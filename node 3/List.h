#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#include "mbed.h"
extern Serial pc;

#include <string>
using namespace std;

class List{
private:

    typedef struct node{
        string data;
        node* next;
    }* nodePtr;

    nodePtr head;
    nodePtr curr;
    nodePtr temp;

public:

    List();
    void AddNode(string addData);
    void DeleteNode(string delData);
    void PrintList(void);
};


#endif // LIST_H_INCLUDED