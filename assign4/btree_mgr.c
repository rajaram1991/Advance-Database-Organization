#include "btree_mgr.h"
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct Btree{
	struct Value val;
	struct RID rid;
}Btree;

int entrycount;

Btree **btree;

RC initIndexManager (void *mgmtData)
{
    return RC_OK;
}
RC shutdownIndexManager ()
{
    return RC_OK;
}
RC createBtree (char *idxId, DataType keyType, int n)
{
    char filename[100]={'\0'};
    strcat(filename,idxId);
    strcat(filename,".bin");
    createPageFile(filename);
    //need to do something with schema.
    //BM_PageHandle *ph=MAKE_PAGE_HANDLE();
    //BM_BufferPool *bm=MAKE_POOL();
    //initBufferPool(bm,filename,3,RS_FIFO,NULL);
    return RC_OK;
}
RC openBtree (BTreeHandle **tree, char *idxId)
{
    char filename[100]={'\0'};
    strcat(filename,idxId);
    strcat(filename,".bin");
    BTreeHandle *tree0=(BTreeHandle *)malloc(sizeof(BTreeHandle));
    tree0->idxId=idxId;
    tree0->keyType=DT_INT;
    BM_BufferPool *bm=MAKE_POOL();
    initBufferPool(bm,filename,4,RS_FIFO,NULL);
    tree0->mgmtData=bm;
    tree[0]=tree0;

    return RC_OK;
}
RC closeBtree (BTreeHandle *tree)
{
    BM_BufferPool *bm=(BM_BufferPool *)tree->mgmtData;
    shutdownBufferPool(bm);
    free(bm);
    free(tree);
}
RC deleteBtree (char *idxId)
{
    char filename[100]={'\0'};
    strcat(filename,idxId);
    strcat(filename,".bin");
    destroyPageFile(filename);
}

RC getNumNodes (BTreeHandle *tree, int *result, BM_BufferPool *const bm)
{
	*result=bm->numPages;
	return RC_OK;
}

RC getNumEntries (BTreeHandle *tree, int *result)
{
    //Prassana return entry count
	//*result=entry;
	//return RC_OK;
}

RC getKeyType (BTreeHandle *tree, DataType *result)
{
	int i;
	for(i=0;i<entrycount;i++)
	{
		result[i]=tree->keyType;
	}

	return RC_OK;
}
