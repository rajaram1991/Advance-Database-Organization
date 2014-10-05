#include "buffer_mgr.h"
#include "buffer_node_ll.h"
#include "storage_mgr.h"
#include "dt.h"
//#include "pthread.h"
#include <stdio.h>
#include <stdlib.h>

/**GLOBAL REGION Variables**/
static Nodeptr BP_StNode_ptr=NULL;//Linked List Start Ptr
//Mutex Objects for each method
//static pthread_mutex_t work_mutex_init=PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t work_mutex_forceflush=PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t work_mutex_shutdown=PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t work_mutex_pin=PTHREAD_MUTEX_INITIALIZER;

static long double univtime=-32674;
/**Static Prototypes **/
static Buffer_page_Dtl *initializebufferdetails(const int numPages);
static char *initializePageframes(const int numPages);
static Buffer_page_Dtl *sortWeights(BM_BufferPool *const bm, BufferPool_Node *bf_node);
/***************************************Pool handling*************************************/

/* Name: InitBufferPool
 * Behavior:Would initialize a new buffer pool and if buffer pool exists , it share with new pool handler
 * Version: 1.0.0 */
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,const int numPages, ReplacementStrategy strategy,void *stratData)
{
    RC ret=RC_OK;
    Buffer_page_Dtl *bf_pg_dtl;
    BufferPool_Node *_buffernode;
    BM_BufferPool *tempPool;
    SM_FileHandle *sh;
//    pthread_mutex_lock(&work_mutex_init); //1st Incoming thread hold a lock.
    _buffernode=checkActiveBufferPools(BP_StNode_ptr,pageFileName);
    if(_buffernode==NULL)
    {
        sh=(SM_FileHandle *)malloc(sizeof(SM_FileHandle));
        if(sh==NULL)
        {
            return RC_FILE_HANDLE_NOT_INIT;
        }
        openPageFile(pageFileName,sh);
        bm->pageFile=pageFileName;
        bm->numPages=numPages;
        bm->strategy=strategy;
        bm->mgmtData=sh;

        bf_pg_dtl=initializebufferdetails(numPages);
        ret=bf_pg_dtl!=NULL?RC_OK:RC_FILE_HANDLE_NOT_INIT;
        if(insert_node(&BP_StNode_ptr,bm,bf_pg_dtl)==FALSE)
        ret=RC_FILE_HANDLE_NOT_INIT;
    }
    else
       {
            tempPool=_buffernode->buffer_pool_ptr;
            bm->pageFile=pageFileName;
            bm->numPages=numPages;
            bm->strategy=strategy;
            bm->mgmtData=tempPool->mgmtData;
            bf_pg_dtl=_buffernode->buffer_page_dtl;
            if(insert_node(&BP_StNode_ptr,bm,bf_pg_dtl)==FALSE)
            ret=RC_FILE_HANDLE_NOT_INIT;
       }
//      pthread_mutex_unlock(&work_mutex_init);//1st Incoming thread release a lock.
return ret;
}
/* Name: ShutDown BufferPool
 * Behavior:Would shutdown an existing buffer pool.
 * Version: 1.0.0 */
RC shutdownBufferPool(BM_BufferPool *const bm)
{
    RC ret=RC_WRITE_FAILED;
    int i ,*pgnums, *fixounts;
    bool ispinned=FALSE;
    Buffer_page_Dtl *pg_dtl;
    BufferPool_Node *bufnode;
    char *frame;
    int currentaccessed;
//    pthread_mutex_lock(&work_mutex_shutdown);//1st Incoming thread hold a lock.
    fixounts=getFixCounts(bm);
    pgnums=getFrameContents(bm);
    for(i=0 ; i < bm->numPages ; i++)
    {
        if(fixounts[i] > 0)
        {
            ispinned=TRUE;//Condition to check for pinned pages before shut down.
        }
    }
    free(fixounts);
    if(ispinned==FALSE)
    {
        ret=RC_OK;
        bufnode=search_data(BP_StNode_ptr,bm);//Get the active buffer pool from collection of pools.
        currentaccessed=getfilebeingused(BP_StNode_ptr,bm->pageFile);//Check if the same pool is being shared.
        if(bufnode!=NULL)
        {
            pg_dtl=bufnode->buffer_page_dtl;
            if(pg_dtl!=NULL)
            for(i=0;i<bm->numPages;i++)
            {
                frame=pg_dtl[i].pageframes;
                if(pg_dtl[i].isdirty==TRUE)
                {
                   ret=writeBlock(pg_dtl[i].pagenums,bm->mgmtData,frame)==RC_OK?RC_OK:RC_WRITE_FAILED;//Write the content with dirty to disk.
                }
                currentaccessed == 1 ?free(frame):' ';
            }
            currentaccessed == 1 ?free(pg_dtl):' ';
            pg_dtl=NULL;
            delete_node(&BP_StNode_ptr,bm);
        }
    currentaccessed == 1 ?closePageFile(bm->mgmtData):' ';
    currentaccessed == 1 ?free(bm->mgmtData):' ';

  }
//  pthread_mutex_unlock(&work_mutex_shutdown);//1st Incoming thread release a lock.
    free(pgnums);
    return ret;
}
/* Name: ForceFlush BufferPool
 * Behavior:Write the data to disk forecefully with fix count =0
 * Version: 1.0.0 */
RC forceFlushPool(BM_BufferPool *const bm)
{
    RC ret=RC_OK;
    int i;
    Buffer_page_Dtl *pg_dtl;
    BufferPool_Node *bufnode;
    char *frame;
//    pthread_mutex_lock(&work_mutex_forceflush);
    bufnode=search_data(BP_StNode_ptr,bm);
    if(bufnode!=NULL)
        {
            pg_dtl=bufnode->buffer_page_dtl;
            for(i=0;i<bm->numPages;i++)
            {
                frame=pg_dtl[i].pageframes;
                if(pg_dtl[i].isdirty==TRUE && pg_dtl[i].fixcounts==0)//Writes the data with fixcount=0
                {
                   ret=writeBlock(pg_dtl[i].pagenums,bm->mgmtData,frame)==RC_OK?RC_OK:RC_WRITE_FAILED;
                   pg_dtl[i].isdirty=FALSE;
                   bufnode->numwriteIO++;
                }
            }
        }
//    pthread_mutex_unlock(&work_mutex_forceflush);
    return ret;
}
/******************************************Statistics interfaces******************************/
/* Name: getFrameContents
 * Behavior:Would return an array pointer with array size equals total number of frames in buffer pool. Each array value equals page numbers.
 * Version: 1.0.0 */
PageNumber *getFrameContents (BM_BufferPool *const bm)
{
    PageNumber i,*pgnum=NULL;
    Buffer_page_Dtl *bf_pg_dtl;
    Nodeptr _bufferNode=search_data(BP_StNode_ptr,bm);
    if(_bufferNode!=NULL)
    {
        pgnum=(PageNumber *)calloc(bm->numPages,sizeof(PageNumber));
        bf_pg_dtl=_bufferNode->buffer_page_dtl;
        if(bf_pg_dtl!=NULL){
        for(i=0;i < bm->numPages;i++)
        {
            pgnum[i]=bf_pg_dtl[i].pagenums;
        }
        }else
          {
              free(bf_pg_dtl);
              bf_pg_dtl=NULL;
          }

    }
    return pgnum;
}

/* Name: getDirtyFlags
 * Behavior:Would return an array pointer with array size equals total number of frames in buffer pool. Each array value equals true or false w.r.t corresponding page.
 * Version: 1.0.0 */
bool *getDirtyFlags (BM_BufferPool *const bm)
{
    int i;
    bool  *dirtyflags;
    Buffer_page_Dtl *bf_pg_dtl;
    Nodeptr _bufferNode=search_data(BP_StNode_ptr,bm);
    if(_bufferNode!=NULL)
    {
        dirtyflags=(bool *)calloc(bm->numPages,sizeof(bool));
        bf_pg_dtl=_bufferNode->buffer_page_dtl;
        if(bf_pg_dtl!=NULL){
        for(i=0;i < bm->numPages;i++)
        {
            dirtyflags[i]=bf_pg_dtl[i].isdirty;
        }
        }else
          {
              free(bf_pg_dtl);
              bf_pg_dtl=NULL;
          }
    }
return dirtyflags;
}
/* Name: getNumReadIO
 * Behavior:Would return total number of reads that has taken place.
 * Version: 1.0.0 */
int getNumReadIO (BM_BufferPool *const bm)
{
    int  nOreadIO=0;
    Nodeptr _bufferNode=search_data(BP_StNode_ptr,bm);
    if(_bufferNode!=NULL)
    {
        nOreadIO=_bufferNode->numreadIO;
    }

    return nOreadIO;

}
/* Name: getNumReadIO
 * Behavior:Would return total number of writes that has taken place.
 * Version: 1.0.0 */
int getNumWriteIO (BM_BufferPool *const bm){
    int  nOwriteIO=0;
    Nodeptr _buffernode=search_data(BP_StNode_ptr,bm);
    if(_buffernode!=NULL)
    {
        nOwriteIO=_buffernode->numwriteIO;
    }
    return nOwriteIO;
}

/* Name: getFixCounts
 * Behavior:Would return an array pointer with array size equals total number of frames in buffer pool. Each array value equals fixcount of a frame.
 * Version: 1.0.0 */
int *getFixCounts (BM_BufferPool *const bm)
{
int i,*fixcounts;
Buffer_page_Dtl *bf_pg_dtl;
Nodeptr _buffernode=search_data(BP_StNode_ptr,bm);
    if(_buffernode!=NULL)
      {
          fixcounts=(int *)calloc(bm->numPages,sizeof(int));
          bf_pg_dtl=_buffernode->buffer_page_dtl;
          if(bf_pg_dtl!=NULL){
          for(i=0 ; i < bm->numPages ; i++)
          {
              fixcounts[i]=bf_pg_dtl[i].fixcounts;
          }
          }
          else
          {
              free(bf_pg_dtl);
              bf_pg_dtl=NULL;
          }

      }
return fixcounts;
}
/********Helper Functions*********/
/* Name: initializebufferdetails
 * Behavior:Initialize the page details that needs to be mapped with current pool.
 * Version: 1.0.0 */
static Buffer_page_Dtl *initializebufferdetails(const int numPages)
{
    int i;
    Buffer_page_Dtl *temppagedtl=NULL;
    temppagedtl=(Buffer_page_Dtl *)calloc(numPages,sizeof(Buffer_page_Dtl));//dynamic allocation.
    if(temppagedtl!=NULL)
    {
        for(i=0;i < numPages;i++)
        {
            (temppagedtl+i)->pageframes=initializePageframes(numPages);
            temppagedtl[i].fixcounts=0;
            temppagedtl[i].isdirty=FALSE;
            temppagedtl[i].pagenums=NO_PAGE;
        }
    }
    return temppagedtl;
}
/* Name: initializePageframes
 * Behavior:Initialize the page frames that needs to be mapped with individual page details.
 * Version: 1.0.0 */
static char *initializePageframes(const int numPages)
{
    int i;
    char *data;
    data=(char *)malloc(PAGE_SIZE);
    if(data!=NULL)
    {
        for(i=0;i < PAGE_SIZE ; i++)
            data[i]='\0';
    }
    return data;
}
/***************************************************************************************************
 * Interface Access Pages
 * *************************************************************************************************/

/* Name: markDirty
 * Expected arguments: BM_BufferPool *const bm, BM_PageHandle *const page
 * Behavior: Would mark a particular page from a particular BufferPool as dirty
 * Version: 1.0.0 */
RC markDirty(BM_BufferPool * const bm, BM_PageHandle * const page) {
	int i;
	BufferPool_Node *pg_node;
	Buffer_page_Dtl *bf_pg_dtl;
	pg_node = search_data(BP_StNode_ptr, bm);
	bf_pg_dtl = pg_node->buffer_page_dtl;
	for (i = 0; i < bm->numPages; i++) {
		if (((bf_pg_dtl + i)->pagenums) == page->pageNum) {
			(bf_pg_dtl + i)->isdirty = TRUE;
			return RC_OK;
		}
	}
return RC_WRITE_FAILED; /*Should have been markdirty:failure*/
}

/* Name: unpinPage
 * Expected arguments: BM_BufferPool *const bm, BM_PageHandle *const page
 * Behavior: Would check if a page is dirty. If dirty, this function would write the change back to the file
 * 			 and unpin it from the buffer pool. If clean, this function would simply write the change back to
 * 			 the file and unpin the page from buffer pool.
 * Version: 1.0.0
 * Algorithm:   1) Check if the page to be unpinned has fixcounts==0
 * 				2) If not throw an error that "this page has been currently accessed"
 * 				3) If fixcounts==0, check if isDirty==TRUE
 * 				4) If isDirty==TRUE, forcePage
 * 				5) Free the page from the memory reference*/
RC unpinPage(BM_BufferPool * const bm, BM_PageHandle * const page) {

	int totalPages = bm->numPages, i, k;
	BufferPool_Node *bf_node;
	bf_node = search_data(BP_StNode_ptr, bm);
	Buffer_page_Dtl *pg_dtl;
	pg_dtl = bf_node->buffer_page_dtl;
	for (i = 0; i < totalPages; i++) {
		if ((pg_dtl + i)->pagenums == page->pageNum) {
			if ((pg_dtl + i)->isdirty == TRUE) {
			}
			(pg_dtl + i)->fixcounts -= 1;
			return RC_OK;
		}
	}

	return RC_WRITE_FAILED; //should be unpin failed
}

/* Name: forcePage
 * Expected arguments: BM_BufferPool *const bm, BM_PageHandle *const page
 * Behavior:
 * Version: 1.0.0 */
RC forcePage(BM_BufferPool * const bm, BM_PageHandle * const page) {
	BufferPool_Node *bf_node;
	bf_node = search_data(BP_StNode_ptr, bm);
	if(bf_node!=NULL)
    {

	if (bm->mgmtData!= NULL) {
		writeBlock(page->pageNum, bm->mgmtData, page->data);
		bf_node->numwriteIO++;
	} else {
		return RC_FILE_NOT_FOUND;
	}
    }
	return RC_OK;
}

/* Name: pinPage
 * Expected arguments: BM_BufferPool *const bm, BM_PageHandle *const page, covnst PageNumber pageNum
 * Behavior: Would perform the applicable page replacement strategy on the buffer pool and pin the page from
 * 			 file at the appropriate position.
 * Version: 1.0.0
 * Algorithm:   1) Check if the page is already present in buffer
 * 				2) Add a replcementWeight to each page.
 * 				2) Get the appropriate replacement page by invoking the corresponding strategy
 * 				2) Replace that page with the new page.
 * 				3) The newly added page should have the maximum weight. */
RC pinPage(BM_BufferPool * const bm, BM_PageHandle * const page,
		const PageNumber pageNum) {
	int replacementFrameNum, totalPages, i;
	int statusFlag;
	ReplacementStrategy strategy = bm->strategy;
	Buffer_page_Dtl *lowestPossiblePage;
	totalPages = bm->numPages;
	BufferPool_Node *bf_node;
//	pthread_mutex_lock(&work_mutex_pin);//Hold the mutex lock for thread in pin block.
	bf_node = search_data(BP_StNode_ptr, bm);
	Buffer_page_Dtl *pg_dtl;
	pg_dtl = bf_node->buffer_page_dtl;
	RC writeVal=1, readVal=1;
	int emptyFlag = 1;
	if (pg_dtl != NULL) {

		/*This for loop checks if the page is already present in the buffer*/
		for (i = 0; i < totalPages; i++) {
			/*In order to make sure that all the elements of the array are empty, I am not breaking the loop*/
			if ((pg_dtl + i)->pagenums > -1) { //I assume that pagenums will be a mandatory field for any pg_dtl.
				emptyFlag = 0;
				/*To check if the page is already present in buffer.*/
				if ((pg_dtl + i)->pagenums == pageNum) {
                    (pg_dtl + i)->timeStamp=univtime++;//timestamp used in LRU
                    page->pageNum = pageNum;
					page->data = (pg_dtl + i)->pageframes;
					(pg_dtl + i)->fixcounts+=1;
//					pthread_mutex_unlock(&work_mutex_pin);//Unlock the mutex lock for thread in pin block.
					return RC_OK; // If there is a hit, return OK. There is no need to pin it since it is already present.
				}

			}
		} //end of 1st for loop

		/*This loop gets the first lowestPossible page and assigns it.*/
		for (i = 0; i < totalPages; i++) {
			if ((pg_dtl + i)->pagenums == -1){
				lowestPossiblePage = ((pg_dtl + i)); //After the loop, lowestPossiblePage would be last page for which pg_dtl was empty
				emptyFlag = 1;
				break;
			}
		}//end of 2nd for loop

	} else { /*This also means that the page detail has not been initialized at all since the array itself is null. This block might never be reached*/
		lowestPossiblePage = (pg_dtl + 0); // I assign the first frame itself as the lowestPossiblePage which is fit to be replaced.
		lowestPossiblePage->replacementWeight =
				lowestPossiblePage->replacementWeight + 1;

		(pg_dtl + i)->timeStamp=univtime++;
		emptyFlag = 1;
	}

	/*Even if there is one empty frame, emptyFlag would be 1. And we could make use of that page.*/
	if (emptyFlag == 1) {
		page->pageNum = pageNum;
		page->data = lowestPossiblePage->pageframes;
		/*If you find a free frame, just write the contents to that frame*/
		writeVal = RC_OK;
		/*This if condition is not mandatory. Infact this might never become true*/
		if (lowestPossiblePage->isdirty == TRUE) {
			writeVal = writeBlock(pageNum, bm->mgmtData,
					lowestPossiblePage->pageframes);
					bf_node->numwriteIO++;
		}
		if(readBlock(pageNum, bm->mgmtData,lowestPossiblePage->pageframes)==RC_READ_NON_EXISTING_PAGE)
				{
				    readVal=appendEmptyBlock(bm->mgmtData);//If page requested in pin not available in the file , do append it.
				}
				else
                    readVal=RC_OK;
        bf_node->numreadIO++;
		lowestPossiblePage->fixcounts += 1;
		lowestPossiblePage->pagenums = pageNum;

	} else { /*If no frame is free, call the appropriate page replacement algorithm to do the job*/
		if (strategy == RS_FIFO) {
			statusFlag = applyFIFO(bm, page, pageNum);
		} else if (strategy == RS_LRU) {
			statusFlag = applyLRU(bm, page, pageNum);
		} else if (strategy == RS_CLOCK) {
//			statusFlag = applyCLOCK(bm, page, pageNum);
		} else if (strategy == RS_LRU_K) {
//			statusFlag = applyLRU_k(bm, page, pageNum);
		} else {
			return RC_WRITE_FAILED; // should probably return "Invalid strategy"
		}
	} //end of outer else block

	/*Status flag indicates the success of the page replacement algorithm when no frame is free
	 * writeVal indicates the success of writelock call when a frame is dirty*/
//   	 pthread_mutex_unlock(&work_mutex_pin);
	if (statusFlag == 1 || (writeVal == RC_OK && readVal == RC_OK)) {
		return RC_OK;
	} else {
		return RC_WRITE_FAILED;
	}
}

/****************************************************************************************************************
 ********************************** Custom - Page Replacement methods********************************************
 ****************************************************************************************************************/

/* Name: applyFifo
 * Expected arguments: BM_BufferPool
 * Behavior: Would apply FIFO as page replacement strategy on the buffer pool and return us the appropriate
 * 			 page to be replaced.
 * Returns: Success/failure flag
 * Version: 1.0.0
 * Algorithm:   1) Copy the weights array from buffer_page_dtl to a temporary array
 * 				2) Sort it to get the lowest weighted page number(first added element)
 * 				3) Check if it is dirty
 * 				4) If its not dirty, return the page number
 * 				5) If it is dirty, pop that from the array and search again*/
//int applyFIFO(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum , Buffer_page_Dtl pagedetail){
int applyFIFO(BM_BufferPool * const bm, BM_PageHandle * const page,
		const PageNumber pageNum) {
	BufferPool_Node *bf_node;
	bf_node = search_data(BP_StNode_ptr, bm);
	Buffer_page_Dtl *lowestPossiblePage=NULL;
	lowestPossiblePage = sortWeights(bm, bf_node);

	if(lowestPossiblePage==NULL)
    {
        return 0;
    }
	char *replacementAddress;
	replacementAddress = lowestPossiblePage->pageframes;

	RC writeVal = RC_OK;
	RC readVal = RC_OK;
	if (lowestPossiblePage->isdirty == TRUE) {
		writeVal = writeBlock(lowestPossiblePage->pagenums, bm->mgmtData, replacementAddress);
		lowestPossiblePage->isdirty=FALSE;
		bf_node->numwriteIO++;
	}
	readVal = readBlock(pageNum, bm->mgmtData, replacementAddress);
	if(readVal==RC_READ_NON_EXISTING_PAGE)
    {
        readVal =appendEmptyBlock(bm->mgmtData);
        readBlock(pageNum, bm->mgmtData, replacementAddress);
    }
    bf_node->numreadIO++;
	page->pageNum  = pageNum;
	page->data = lowestPossiblePage->pageframes;
    lowestPossiblePage->pagenums = pageNum;
    lowestPossiblePage->fixcounts+=1;
	lowestPossiblePage->replacementWeight = lowestPossiblePage->replacementWeight + 1;
	if (readVal == RC_OK && writeVal == RC_OK)
		return 1; //success flag
	else
		return 0;
}

/* Name: applyLRU
 * Expected arguments: BM_BufferPool
 * Behavior: Would apply LRU as page replacement strategy on the buffer pool and return us the appropriate
 * 			 page to be replaced.
 * Returns: Success/failure flag.
 * Version: 1.0.0 */
int applyLRU(BM_BufferPool * const bm, BM_PageHandle * const page,
		const PageNumber pageNum) {
	BufferPool_Node *bf_node;
	bf_node = search_data(BP_StNode_ptr, bm);
	Buffer_page_Dtl *lowestPossiblePage;
	lowestPossiblePage = sortWeights(bm, bf_node);
	char *replacementAddress;
	replacementAddress = lowestPossiblePage->pageframes;

	RC writeVal = RC_OK;
	RC readVal = RC_OK;
	if (lowestPossiblePage->isdirty == TRUE) {
		writeVal = writeBlock(page->pageNum, bm->mgmtData, replacementAddress);
		lowestPossiblePage->isdirty=FALSE;
		bf_node->numwriteIO++;
	}
	readVal = readBlock(pageNum, bm->mgmtData, replacementAddress);
	if(readVal==RC_READ_NON_EXISTING_PAGE)
    {
        readVal =appendEmptyBlock(bm->mgmtData);
        readVal = readBlock(pageNum, bm->mgmtData, replacementAddress);
    }
	page->pageNum  = pageNum;
	page->data = lowestPossiblePage->pageframes;
    lowestPossiblePage->pagenums = pageNum;
    lowestPossiblePage->fixcounts+=1;
	lowestPossiblePage->replacementWeight =
    lowestPossiblePage->replacementWeight + 1;
	lowestPossiblePage->timeStamp =(long double)univtime++;
    bf_node->numreadIO++;
	if (readVal == RC_OK && writeVal == RC_OK)
		return 1; //success flag
	else
		return 0;
}

/* Name: applyLFU
 * Expected arguments: BM_BufferPool
 * Behavior: Would apply LFU as page replacement strategy on the buffer pool and return us the appropriate
 * 			 page to be replaced.
 * Returns: Success/failure flag.
 * Version: 1.0.0 */
int applyLFU(BM_BufferPool * const bm, BM_PageHandle * const page,
		const PageNumber pageNum) {

	BufferPool_Node *bf_node;
	bf_node = search_data(BP_StNode_ptr, bm);
	Buffer_page_Dtl *lowestPossiblePage=NULL;
	lowestPossiblePage = sortWeights(bm, bf_node);
	if(lowestPossiblePage==NULL)
    {
        return 0;
    }
	char *replacementAddress;
	replacementAddress = lowestPossiblePage->pageframes;


	RC writeVal = RC_OK;
	RC readVal = RC_OK;
	if (lowestPossiblePage->isdirty == TRUE) {
		writeVal = writeBlock(lowestPossiblePage->pagenums, bm->mgmtData, replacementAddress);
		lowestPossiblePage->isdirty=FALSE;
		bf_node->numwriteIO++;
	}
	readVal = readBlock(pageNum, bm->mgmtData, replacementAddress);
	if(readVal==RC_READ_NON_EXISTING_PAGE)
    {
        readVal =appendEmptyBlock(bm->mgmtData);
    }
    bf_node->numreadIO++;
	page->pageNum  = pageNum;
	page->data = lowestPossiblePage->pageframes;
    lowestPossiblePage->pagenums = pageNum;
    lowestPossiblePage->fixcounts+=1;
	lowestPossiblePage->replacementWeight = lowestPossiblePage->replacementWeight + 1;
	if (readVal == RC_OK && writeVal == RC_OK)
		return 1; //success flag
	else
		return 0;
}

/* Name: sortWeights
 * Expected arguments: Buffer_page_Dtl
 * Behavior: Would apply Quick-sort on the array.and  Returns the best frame to the pool depending on strategy.
 * Version: 1.0.0 */

Buffer_page_Dtl *sortWeights(BM_BufferPool * const bm, BufferPool_Node *bf_node) {
	int i;
	int totalPagesInBlock = bm->numPages;
	Buffer_page_Dtl *bf_page_dtl = bf_node->buffer_page_dtl;
	Buffer_page_Dtl *bf_page_dtl_with_zero_fixcount[totalPagesInBlock];
	int count = 0;

	/*Get the list of all bf_page_dtl where fixcount == 0*/
	for (i = 0; i < totalPagesInBlock; i++) {
		bf_page_dtl_with_zero_fixcount[i] = NULL;
		}

	for (i = 0; i < totalPagesInBlock; i++) {
		if ((bf_page_dtl[i].fixcounts) == 0) {
			bf_page_dtl_with_zero_fixcount[count] = (bf_page_dtl+i);
			count++;
		}
	}

	/*Macro to determine the size of the newly created pointer array*/
    #define sizeofa(array) sizeof array / sizeof array[ 0 ]
    int sizeOfPagesWithFixcountZero = sizeofa(bf_page_dtl_with_zero_fixcount);

	/*Implementation of a modified Quick sort algorithm to find the page with lowest replacementWeight*/
	Buffer_page_Dtl *next_bf_page_dtl;
	Buffer_page_Dtl *replacement_bf_page_dtl ;
	replacement_bf_page_dtl = bf_page_dtl_with_zero_fixcount[0];
	for (i = 0; i < sizeOfPagesWithFixcountZero; i++) {
		next_bf_page_dtl = bf_page_dtl_with_zero_fixcount[i];
		if(next_bf_page_dtl!=NULL){
			if(bm->strategy == RS_FIFO||bm->strategy==RS_LFU){
				if ((replacement_bf_page_dtl->replacementWeight) > (next_bf_page_dtl->replacementWeight))
						replacement_bf_page_dtl = next_bf_page_dtl;
			}
			else if(bm->strategy==RS_LRU){
				/*If time stamp of the current page is newer than the time stamp of the next page, assign next page as the replaceable page*/
	            if(replacement_bf_page_dtl->timeStamp > next_bf_page_dtl->timeStamp)
				{
					replacement_bf_page_dtl = next_bf_page_dtl;
				}
			}
		}
	}
return replacement_bf_page_dtl;
}
