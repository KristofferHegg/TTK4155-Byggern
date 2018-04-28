#include <cstdlib>
#include <string> 
#include <iostream>

#include "List.h"

using namespace std;

List::List(){
    head = NULL;
    curr = NULL;
    temp = NULL;
}

void List::AddNode(string addData){
    
    nodePtr n = new node;
    n -> next = NULL;
    n -> data = addData;

    if(head != NULL){
        curr = head;
        while(curr -> next != NULL){
            curr = curr -> next;
        }
        curr -> next = n;
    }else{
        head = n;
    }
    
    pc.printf("The player %s was added\r\n", addData);
}

void List::DeleteNode(string delData){
    nodePtr delPtr = NULL;
    temp = head;
    curr = head;
    while(curr != NULL && curr -> data != delData){
        temp = curr;
        curr = curr -> next;
    }
    if(curr == NULL){
        pc.printf("%s was not in the listr\n", delData);
        delete delPtr;
    }else{
        delPtr = curr;
        curr = curr -> next;
        temp -> next = curr;
        if(delPtr == head){
            head = head -> next;
            temp = NULL;
        }
        delete delPtr;
        pc.printf("The player %s was deleted\r\n", delData);
    }
}

void List::PrintList(){
    curr = head;
    while(curr != NULL){
        cout << curr -> data << '\r' <<endl;
        curr = curr -> next;
    }
}