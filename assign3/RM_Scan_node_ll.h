#ifndef RM_SCAN_NODE_LL_H_INCLUDED
#define RM_SCAN_NODE_LL_H_INCLUDED

#include "record_mgr.h"
#include "buffer_mgr.h"
#include "dt.h"
#include <stdio.h>
#include <stdlib.h>



//Scan helper Structure
typedef struct Scan_Help
{
    int _page;
    int _slot;
    int _totalrecordlength;
    int _totalrecordsinpage;
    int _totalnumPages;
    BM_PageHandle *ph;

}Scan_Help;

typedef struct Scan_Node
{
    RM_ScanHandle *scanHandle;
    Scan_Help *scanhelp;
    struct Scan_Node *nextScanNode;
} Scan_Node,*Scan_Node_ptr;

/*********Linked List Operations *************/
bool insert_Scan_node(Scan_Node_ptr *stnode , RM_ScanHandle *scanHandle , Scan_Help *scanhelp);
bool delete_Scan_node(Scan_Node_ptr *stnode , RM_ScanHandle *scanHandle );
Scan_Help *search_scandata(Scan_Node_ptr stnode , RM_ScanHandle *scanHandle);

#endif // RM_SCAN_NODE_LL_H_INCLUDED
