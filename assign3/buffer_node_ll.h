/******************Linked List DataStructure ***************/
#ifndef BUFFER_NODE_LL_H_INCLUDED
#define BUFFER_NODE_LL_H_INCLUDED

#include "buffer_mgr.h"
#include <stdio.h>
#include <stdlib.h>
#include "dt.h"
//LL Node to Insert Delete and search
//It also serves to map the pool and its frames.
typedef struct BufferPool_Node
{
    void *buffer_pool_ptr;
    void *buffer_page_dtl;//buffer_page_dtl refers &buffer_page_dtl[]
    int numreadIO;
    int numwriteIO;
    struct BufferPool_Node *nextBufferNode;
} BufferPool_Node,*Nodeptr;

//Buffer Page/Frame details.
typedef struct Buffer_page_Dtl
{
    char *pageframes;
    PageNumber pagenums;
    bool isdirty;
    int fixcounts;
    int replacementWeight;
    long double timeStamp;
} Buffer_page_Dtl;


/*********Linked List Operations *************/
bool insert_node(Nodeptr *stnode , void *buffer_pool_ptr , void * buffer_page_dtl);
bool delete_node(Nodeptr *nodeptr , void *buffer_pool_ptr );
void print_list (Nodeptr startptr);
BufferPool_Node *search_data(Nodeptr nodeptr, void * buffer_pool_ptr );
BufferPool_Node *checkActiveBufferPools(Nodeptr stnode,char *filename);
int getfilebeingused(Nodeptr stnode,char *filename);
#endif // BUFFER_NODE_LL_H_INCLUDED
