#include <stdio.h>
#include <stdlib.h>
#include "ConnectionNode.h"

void InsertConnectionNode(struct ConnectionNode* head, int value)
{
    struct ConnectionNode* it = head;
    struct ConnectionNode* new_node = malloc(sizeof(struct ConnectionNode));
    new_node->next = NULL;
    new_node->value = value;
    
    if(head != NULL)
    {
        while(it->next != NULL)
            it = it->next;
        
        it->next = new_node;
        return;
    }

    it = new_node;
}


struct ConnectionNode* FindConnectionNode(struct ConnectionNode* head, int value)
{
    struct ConnectionNode* temp = head;

    while(temp != NULL && temp->value != value)
        temp = temp->next;

    return temp;
}

void FreeConnectionNode(struct ConnectionNode* head)
{
    struct ConnectionNode* temp;
    while(head)
    {
        temp = head;
        head = head->next;
        free(temp);
    }
}