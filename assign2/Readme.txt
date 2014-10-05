                                            README - BUFFER MANAGER.
CONTENTS:
1.OverView of Design.
2.OverView of Implementation.
3.Functional Descriptions.
4.Addtional Tests.

/***************************************************************************************************
 * OverView of Design
 * *************************************************************************************************/
Following are the structures used and its role.
BM_BufferPool - used to create a pool for a given file.
BufferPool_Node - LL Node to Insert Delete and search.It also serves to map the pool and its frames.
Buffer_page_Dtl -This structure is used to contain the page frames to a buffer pool.

We create a linked list to maintain the active buffer pools.In each linked list node , we contain buffer pool pointer and buffer page dtl pointer so that it servers as map between
frame and the pool.Other details with respect to frames such as pgnum , dirty flag fixcount are also present in buffer page dtl.

Note: mgmtinfo of BM_BufferPool is used to store the pointer of SM_FileHandle .

For a short idea of design , please refer to the "Simple Design Layout.jpg" present along in the assing2 folder.

/***************************************************************************************************
 * OverView of Implementation
 * *************************************************************************************************/
When a buffer pool is created, it inserted into the buffer node of a linked list.
At any instant , Linkedlist nodes will contain all active buffer pools.
If two or more pools are created for the same file ,  we contain two different buffer pool pointers and share the same page frames.
On shutdown operation , we delete the node that contains the active pool.

FIFO technique has been done using sorted weights.
LRU technique has been done using time stamp.

Note:1) We have implemented LFU Page Replace Technique.
     2) We have made the application thread safe using Mutex synchronization.


/***************************************************************************************************
 * Functional description.
 * *************************************************************************************************/

/* Name: markDirty
 * Expected arguments: BM_BufferPool *const bm, BM_PageHandle *const page
 * Behavior: Would mark a particular page from a particular BufferPool as dirty
 * Version: 1.0.0 */
1) Get a page handle.
2) Mark the frame/page at the given handle as dirty.
3) Return appropriate message.

/* Name: unpinPage
 * Expected arguments: BM_BufferPool *const bm, BM_PageHandle *const page
 * Behavior: Would check if a page is dirty. If dirty, this function would write the change, back to the file
 * 			 and unpin it from the buffer pool. If clean, this function would simply write the change back to
 * 			 the file and unpin the page from buffer pool.
 * Version: 1.0.0 */
1) Check if the page to be unpinned has fixcounts==0
2) If not throw an error
3) If fixcounts==0, check if isDirty==TRUE
4) If isDirty==TRUE, forcePage
5) Free the page from the memory reference

/* Name: forcePage
 * Expected arguments: BM_BufferPool *const bm, BM_PageHandle *const page
 * Behavior:
 * Version: 1.0.0 */
1) Get the frame to be written to the file on disk.
2) Write the page contents to the file on disk
3) Return appropriate message

/* Name: pinPage
 * Expected arguments: BM_BufferPool *const bm, BM_PageHandle *const page, covnst PageNumber pageNum
 * Behavior: Would perform the applicable page replacement strategy on the buffer pool and pin the page from
 * 			 file at the appropriate position.
 * Version: 1.0.0 */
1) Check if any of the frames is empty.
2) If an empty page is found, use that for the replacement strategy. If not check if the page is already present in buffer.
3) If found add appropriate replcementWeight/timestamp to each page.
4) Get the appropriate replacement page by invoking the corresponding strategy
5) Replace that page with the new page and update the replacement parameters.


/****************************************************************************************************************
 ********************************** Custom - Page Replacement methods********************************************
 ****************************************************************************************************************/

/* Name: applyFifo
 * Expected arguments: BM_BufferPool
 * Behavior: Would apply FIFO as page replacement strategy on the buffer pool and return us the appropriate
 * 			 page to be replaced.
 * Returns: Success/failure flag
 * Version: 1.0.0 */
1) Add replacement weights whenever a page is added into the frame.
2) The page added first would have the least replacement weight and the page added last would have the highest weight.
3) Repeat this process in such a manner that the maximum weight of the page equals the number of frames.
4) Replace the page with the least weight which also means it is the first-in page.



/* Name: applyLRU
 * Expected arguments: BM_BufferPool
 * Behavior: Would apply LRU as page replacement strategy on the buffer pool and return us the appropriate
 * 			 page to be replaced.
 * Returns: Success/failure flag.
 * Version: 1.0.0 */
1) Add timestamps whenever a page is added into the frame or accessed.
2) The page added first would have the oldest timestamp and the page added last would have the latest timestamp.
3) Replace the page with the oldest timestamp which also means that it is the Least Recently Used page.


/* Name: applyLFU
 * Expected arguments: BM_BufferPool
 * Behavior: Would apply LFU as page replacement strategy on the buffer pool and return us the appropriate
 * 			 page to be replaced.
 * Returns: Success/failure flag.
 * Version: 1.0.0 */
1) Add replacement weights(frequency) whenever a page is added into the frame or accessed.
2) The page least accessed would have the least replacement weight and the page most accessed would have the highest weight.
3) When ever a new page is added, replacement weight(frequency of access) of that page should be neutralised.
4) Replace the least frequently acced page.



/****************************************************************************************************************
 ********************************** Custom - Interface Helper Functions********************************************
 ****************************************************************************************************************/
/* Name: sortWeights
 * Expected arguments: Buffer_page_Dtl
 * Behavior: Would apply Quick-sort on the array.
 * Returns: Lowest weighted page number of the list.
 * Version: 1.0.0 */
1) Collect all the pages with fix count 0.
2) Sort them based on their replacement parameters.
3) Return the appropriate replacement page.
/****************************************************************************************************************
 ********************************** Pool Handling**********
 ****************************************************************************************************************/


/* Name: InitBufferPool
 * Behavior:Would initialize a new buffer pool and if buffer pool exists , it share with new pool handler
 * Version: 1.0.0 */
 1)Dynamic Allocation of a linked list node.
 2) Initialize the page frames depending on the buffer pool num pages.
 3) Insert the active pool and page frames into the LL node.


/* Name: ShutDown BufferPool
 * Behavior:Would shutdown an existing buffer pool.
 * Version: 1.0.0 */

 1)Get the buffer page dtl for the pool to be shutdowned.
 2) Check for pinned pages.If true , exit the operation with error.
 3) If false , check for any dirty page frames.If present. Write them to the disc.
 4) Free the resources associated with the pool.


/* Name: ForceFlush BufferPool
 * Behavior:Write the data to disk forecefully with fix count =0
 * Version: 1.0.0 */

 1)Get the buffer page dtl for the current pool
 2) Check for any dirty page frames.If present and has got fixcount=0 Write them to the disc.

 /******************************************Statistics interfaces******************************/
/* Name: getFrameContents
 * Behavior:Would return an array pointer with array size equals total number of frames in buffer pool. Each array value equals page numbers.
 * Version: 1.0.0 */
 1) Get all the pages frames for the given buffer pool.
 2) Read the page number of corresponding frames , put them in an array and return to the client.


/* Name: getDirtyFlags
 * Behavior:Would return an array pointer with array size equals total number of frames in buffer pool. Each array value equals true or false w.r.t corresponding page.
 * Version: 1.0.0 */
 1) Get all the pages frames for the given buffer pool.
 2) Read the dirty flage of corresponding frames , put them in an array and return to the client.

/* Name: getNumReadIO
 * Behavior:Would return total number of reads that has taken place.
 * Version: 1.0.0 */
1)Get the buffer node containg the buffer pool.
2) Return the total read IO of the pool from buffer node.

/* Name: getNumWriteIO
 * Behavior:Would return total number of writes that has taken place.
 * Version: 1.0.0 */
1)Get the buffer node containg the buffer pool.
2) Return the total write IO of the pool from buffer node.

/* Name: getFixCounts
 * Behavior:Would return an array pointer with array size equals total number of frames in buffer pool. Each array value equals fixcount of a frame.
 * Version: 1.0.0 */
 1) Get all the pages frames for the given buffer pool.
 2) Read the fixcounts of corresponding frames , put them in an array and return to the client.


/****************************************************************************************************************
 ********************************** Additional Tests********************************************
 ****************************************************************************************************************/
In the file , we test_assign2_1.c ,  we have created a new thread and invoked testFIFO in that thread.
In order to test thread safe , please remove the commented code and execute the file.
