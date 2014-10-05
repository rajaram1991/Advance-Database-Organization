#include "buffer_node_ll.h"
#include <stdio.h>
#include <stdlib.h>

/*******Buffer Linked List implementation********/
//Create new node and insert to the linked list at last.
bool insert_node(Nodeptr *stnode ,  void *buffer_pool_ptr , void *buffer_page_handle)
{
    bool ret=FALSE;
    Nodeptr newptr;
    Nodeptr previousptr;
    Nodeptr currentptr;
   // static pthread_mutex_t insertnode=PTHREAD_MUTEX_INITIALIZER;
  //  pthread_mutex_lock(&insertnode);//Hold lock on the object for the first incoming thread.
    newptr =(Nodeptr)malloc(sizeof(BufferPool_Node));
    if(newptr!=NULL)
    {
        newptr->buffer_pool_ptr=buffer_pool_ptr;
        newptr->buffer_page_dtl=buffer_page_handle;
        newptr->numreadIO=0;
        newptr->numwriteIO=0;
        newptr->nextBufferNode=NULL;

        previousptr=NULL;
        currentptr =*stnode;

        while (currentptr!=NULL )
        {
            previousptr=currentptr;
            currentptr=currentptr->nextBufferNode;
        }
        if(previousptr==NULL)
        {
            *stnode=newptr;
        }
        else
        {
            previousptr->nextBufferNode=newptr;
        }
        ret=TRUE;
    }
   else
    {
        printf("Memory not available");
    }
//    pthread_mutex_unlock(&insertnode);//Release lock on the object for the first incoming thread.
    return ret;
}
//Used to print values in linked list.
void print_list(Nodeptr startptr)
{
    Nodeptr previousptr;
    Nodeptr currentptr;
    BM_BufferPool *buffnode;
    previousptr=NULL;
    currentptr=startptr;
    if(currentptr==NULL)
    {
        printf("List is empty");
    }
    else
    {
        while(currentptr!=NULL)
        {
            previousptr=currentptr;
            currentptr=currentptr->nextBufferNode;
            buffnode=(BM_BufferPool *)previousptr->buffer_pool_ptr;

            printf("%s ->  ",buffnode->pageFile);
        }
    }
}
//Delete the node from the linked list using given data.
bool delete_node(Nodeptr *nodeptr ,  void *buffer_pool_ptr )
{
    Nodeptr temptr;
    Nodeptr previousptr;
    Nodeptr currentptr;
//    static pthread_mutex_t deletenode=PTHREAD_MUTEX_INITIALIZER;
  //  pthread_mutex_lock(&deletenode);
    previousptr=NULL;
    currentptr=*nodeptr;


    while(currentptr!=NULL && currentptr->buffer_pool_ptr!=buffer_pool_ptr)
    {
        previousptr=currentptr;
        currentptr=currentptr->nextBufferNode;
    }

    if(currentptr!=NULL)
    {
        temptr=currentptr;

        if(previousptr== NULL)
        {
            *nodeptr=currentptr->nextBufferNode;
        }
        else
        {
            previousptr->nextBufferNode=currentptr->nextBufferNode;
        }
        free(temptr);
    }
    else
    {
        printf("Item not in the list");
    }
//    pthread_mutex_unlock(&deletenode);
    return TRUE;

}
//Get the corresponding node for the given buffer pool pointer.
BufferPool_Node *search_data(Nodeptr nodeptr,  void *buffer_pool_ptr )
{
    Nodeptr previousptr=NULL;
    Nodeptr currentptr=nodeptr;
//    static pthread_mutex_t searchnode=PTHREAD_MUTEX_INITIALIZER;
  //  pthread_mutex_lock(&searchnode);
    while(currentptr!=NULL)
    {
        if(currentptr->buffer_pool_ptr==buffer_pool_ptr)
        {
            break;
        }
        previousptr=currentptr;
        currentptr=currentptr->nextBufferNode;
    }
    if(currentptr==NULL)
    {
        printf("Item not available");
    }
//    pthread_mutexattr_destroy(&searchnode);
    return currentptr;
}
//Check number of pools for same files.
BufferPool_Node *checkActiveBufferPools(Nodeptr stnode,char *filename)
{
    Nodeptr previousptr=NULL;
    Nodeptr currentptr=stnode;
    BM_BufferPool *bufferpool;
    while(currentptr!=NULL)
    {
        bufferpool=(BM_BufferPool *)currentptr->buffer_pool_ptr;
        if(bufferpool->pageFile==filename)
        {
           break;
        }
        previousptr=currentptr;
        currentptr=currentptr->nextBufferNode;
    }
    return currentptr;
}
//Get the number of pools totally for a same file.
int getfilebeingused(Nodeptr stnode,char *filename)
{
    Nodeptr previousptr=NULL;
    Nodeptr currentptr=stnode;
    BM_BufferPool *bufferpool;
    int count=0;
    while(currentptr!=NULL)
    {
        bufferpool=(BM_BufferPool *)currentptr->buffer_pool_ptr;
        if(bufferpool->pageFile==filename)
        {
         count++;
        }
        previousptr=currentptr;
        currentptr=currentptr->nextBufferNode;
    }
    return count;
}
