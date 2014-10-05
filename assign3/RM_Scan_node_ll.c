#include "RM_Scan_node_ll.h"

#include <stdio.h>
#include <stdlib.h>

bool insert_Scan_node(Scan_Node_ptr *stnode , RM_ScanHandle *scanHandle , Scan_Help *scanhelp)
{
    bool ret;
    Scan_Node_ptr newnode;
    Scan_Node_ptr previousnode;
    Scan_Node_ptr currentnode;

    newnode=(Scan_Node *)malloc(sizeof(Scan_Node));

    if(newnode!=NULL)
    {
        newnode->scanHandle=scanHandle;
        newnode->scanhelp=scanhelp;
        newnode->nextScanNode=NULL;

        previousnode=NULL;
        currentnode=*stnode;

        while(currentnode!=NULL)
        {
            previousnode=currentnode;
            currentnode=currentnode->nextScanNode;
        }

        if(previousnode==NULL)
        {
            *stnode=newnode;
        }
        else
        {
            previousnode->nextScanNode=newnode;
        }

        ret=TRUE;
    }
    else
    {
        printf("Memory not available");
        ret=FALSE;
    }
    return ret;
}
bool delete_Scan_node(Scan_Node_ptr *stnode , RM_ScanHandle *scanHandle)
{
    bool ret;
    Scan_Node_ptr temptr;
    Scan_Node_ptr previousnode;
    Scan_Node_ptr currentnode;

    previousnode=NULL;
    currentnode=*stnode;

    while(currentnode!=NULL && currentnode->scanHandle!=scanHandle)
    {
        previousnode=currentnode;
        currentnode=currentnode->nextScanNode;
    }

    if(currentnode!=NULL)
    {
        temptr=currentnode;

        if(previousnode== NULL)
        {
            *stnode=currentnode->nextScanNode;
        }
        else
        {
            previousnode->nextScanNode=currentnode->nextScanNode;
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
Scan_Help *search_scandata(Scan_Node_ptr stnode , RM_ScanHandle *scanHandle)
{
    Scan_Node_ptr previousnode;
    Scan_Node_ptr currentnode;

    previousnode=NULL;
    currentnode=stnode;

    while(currentnode!=NULL && currentnode->scanHandle!=scanHandle)
    {
        previousnode=currentnode;
        currentnode=currentnode->nextScanNode;
    }

    return currentnode->scanhelp;
}
