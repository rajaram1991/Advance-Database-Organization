#include "storage_mgr.h"
#include "buffer_mgr_stat.h"
#include "buffer_mgr.h"
#include "dberror.h"
#include "test_helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


// var to store the current test's name
char *testName;

// check whether two the content of a buffer pool is the same as an expected content
// (given in the format produced by sprintPoolContent)
#define ASSERT_EQUALS_POOL(expected,bm,message)			\
  do {									\
    char *real;								\
    real = sprintPoolContent(bm);					\
    if (strcmp((expected),real) != 0)					\
      {									\
	printf("[%s-%s-L%i-%s] FAILED: expected <%s> but was <%s>: %s\n",TEST_INFO, expected, real, message); \
	free(real);							\
	exit(1);							\
      }									\
    printf("[%s-%s-L%i-%s] OK: expected <%s> and was <%s>: %s\n",TEST_INFO, expected, real, message); \
    free(real);								\
  } while(0)



// test and helper methods
static void testCreatingAndRandomReadingDummyPages (void);
static void createDummyPages(BM_BufferPool *bm, int num);
static void readAndCheckDummyPage(BM_BufferPool *bm, int pageNum);

static void testCreatingAndReadingDummyPages (void);
static void checkDummyPages(BM_BufferPool *bm, int num);

static void testReadPage (void);

static void testFIFO (void);

static void testLRU (void);

static void testError (void);

// main method
int
main (void)
{
  initStorageManager();
  testName = "";

  //testCreatingAndRandomReadingDummyPages();
  //testCreatingAndReadingDummyPages();
  //testReadPage();
  //testFIFO();
  //testLRU();
  testError();
}

#define NUM_DUMMY_PAGES 100

// create pages 100 with content "Page X" and perform 10000 random reads of these pages and check that the correct pages are read
void
testCreatingAndRandomReadingDummyPages (void)
{
  int i;
  BM_BufferPool *bm = MAKE_POOL();
  testName = "Creating and Dummy Pages and reading them in random order";

  CHECK(createPageFile("testbuffer.bin"));
  createDummyPages(bm, 100);

  CHECK(initBufferPool(bm, "testbuffer.bin", 10, RS_FIFO, NULL));

  srand(time(0));
  for(i = 0; i < 10000; i++)
    {
      int page = rand() % NUM_DUMMY_PAGES;
      readAndCheckDummyPage(bm, page);
    }

  CHECK(shutdownBufferPool(bm));
  CHECK(destroyPageFile("testbuffer.bin"));

  free(bm);
  TEST_DONE();
}


void
createDummyPages(BM_BufferPool *bm, int num)
{
  int i;
  BM_PageHandle *h = MAKE_PAGE_HANDLE();

  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));

  for (i = 0; i < num; i++)
    {
      CHECK(pinPage(bm, h, i));
      sprintf(h->data, "%s-%i", "Page", h->pageNum);
      CHECK(markDirty(bm, h));
      CHECK(unpinPage(bm,h));
    }

  CHECK(shutdownBufferPool(bm));

  free(h);
}

void
readAndCheckDummyPage(BM_BufferPool *bm, int pageNum)
{
  int i;
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  char *expected = malloc(sizeof(char) * 512);

  CHECK(pinPage(bm, h, pageNum));

  sprintf(expected, "%s-%i", "Page", h->pageNum);
  ASSERT_EQUALS_STRING(expected, h->data, "check read page dummy page content");
  CHECK(unpinPage(bm,h));

  free(expected);
  free(h);
}


// create pages n with content "Page X" and read them back to check whether the content is right
void
testCreatingAndReadingDummyPages (void)
{
  BM_BufferPool *bm = MAKE_POOL();
  testName = "Creating and Reading Back Dummy Pages";

  CHECK(createPageFile("testbuffer.bin"));

  createDummyPages(bm, 22);
  checkDummyPages(bm, 20);

  createDummyPages(bm, 1000);
  checkDummyPages(bm, 1000);

  CHECK(destroyPageFile("testbuffer.bin"));

  free(bm);
  TEST_DONE();
}


void
checkDummyPages(BM_BufferPool *bm, int num)
{
  int i;
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  char *expected = malloc(sizeof(char) * 512);

  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));

  for (i = 0; i < num; i++)
    {
      CHECK(pinPage(bm, h, i));

      sprintf(expected, "%s-%i", "Page", h->pageNum);
      ASSERT_EQUALS_STRING(expected, h->data, "reading back dummy page content");

      CHECK(unpinPage(bm,h));
    }

  CHECK(shutdownBufferPool(bm));

  free(expected);
  free(h);
}

// simple test checking a single page
void
testReadPage ()
{
  BM_BufferPool *bm = MAKE_POOL();
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  testName = "Reading a page";

  CHECK(createPageFile("testbuffer.bin"));
  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));

  CHECK(pinPage(bm, h, 0));
  CHECK(pinPage(bm, h, 0));

  CHECK(markDirty(bm, h));

  CHECK(unpinPage(bm,h));
  CHECK(unpinPage(bm,h));

  CHECK(forcePage(bm, h));

  CHECK(shutdownBufferPool(bm));
  CHECK(destroyPageFile("testbuffer.bin"));

  free(bm);
  free(h);

  TEST_DONE();
}


// testing the FIFO page replacement strategy
void
testFIFO ()
{
  // expected results
  const char *poolContents[] = {
    "[0 0],[-1 0],[-1 0]" ,
    "[0 0],[1 0],[-1 0]",
    "[0 0],[1 0],[2 0]",
    "[3 0],[1 0],[2 0]",
    "[3 0],[4 0],[2 0]",
    "[3 0],[4 1],[2 0]",
    "[3 0],[4 1],[5x0]",
    "[6x0],[4 1],[5x0]",
    "[6x0],[4 1],[0x0]",
    "[6 0],[4 1],[0 0]"
  };
  const int requests[] = {0,1,2,3,4,4,5,6,0};
  const int numLinRequests = 5;
  const int numChangeRequests = 3;

  int i;
  BM_BufferPool *bm = MAKE_POOL();
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  testName = "Testing FIFO page replacement";

  CHECK(createPageFile("testbuffer.bin"));
  createDummyPages(bm, 100);
  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));

  // reading some pages linearly with direct unpin and no modifications
  for(i = 0; i < numLinRequests; i++)
    {
      pinPage(bm, h, requests[i]);
      unpinPage(bm, h);
      ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content");
    }

  // pin one page and test remainder
  i = numLinRequests;
  pinPage(bm, h, requests[i]);
  ASSERT_EQUALS_POOL(poolContents[i],bm,"pool content after pin page");

  // read pages and mark them as dirty
  for(i = numLinRequests + 1; i < numLinRequests + numChangeRequests + 1; i++)
    {
      pinPage(bm, h, requests[i]);
      markDirty(bm, h);
      unpinPage(bm, h);
      ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content");
    }

  // flush buffer pool to disk
  i = numLinRequests + numChangeRequests + 1;
    //ASSERT_EQUALS_POOL(poolContents[i],bm,"unpin last page");

    //i++;
    forceFlushPool(bm);

    ASSERT_EQUALS_POOL(poolContents[i],bm,"pool content after flush");
    h->pageNum = 4;
    unpinPage(bm, h);

    //h->pageNum = 4;
    //unpinPage(bm, h);
  // check number of write IOs
  ASSERT_EQUALS_INT(3, getNumWriteIO(bm), "check number of write I/Os");
  ASSERT_EQUALS_INT(8, getNumReadIO(bm), "check number of read I/Os");

  CHECK(shutdownBufferPool(bm));
  CHECK(destroyPageFile("testbuffer.bin"));

  free(bm);
  free(h);
  TEST_DONE();
}

// test the LRU page replacement strategy
void
testLRU (void)
{
  // expected results
  const char *poolContents[] = {
    // read first five pages and directly unpin them
    "[0 0],[-1 0],[-1 0],[-1 0],[-1 0]" ,
    "[0 0],[1 0],[-1 0],[-1 0],[-1 0]",
    "[0 0],[1 0],[2 0],[-1 0],[-1 0]",
    "[0 0],[1 0],[2 0],[3 0],[-1 0]",
    "[0 0],[1 0],[2 0],[3 0],[4 0]",
    // use some of the page to create a fixed LRU order without changing pool content
    "[0 0],[1 0],[2 0],[3 0],[4 0]",
    "[0 0],[1 0],[2 0],[3 0],[4 0]",
    "[0 0],[1 0],[2 0],[3 0],[4 0]",
    "[0 0],[1 0],[2 0],[3 0],[4 0]",
    "[0 0],[1 0],[2 0],[3 0],[4 0]",
    // check that pages get evicted in LRU order
    "[0 0],[1 0],[2 0],[5 0],[4 0]",
    "[0 0],[1 0],[2 0],[5 0],[6 0]",
    "[7 0],[1 0],[2 0],[5 0],[6 0]",
    "[7 0],[1 0],[8 0],[5 0],[6 0]",
    "[7 0],[9 0],[8 0],[5 0],[6 0]"
  };
  const int orderRequests[] = {3,4,0,2,1};
  const int numLRUOrderChange = 5;
  const int numChangeRequests = 3;

  int i;
  int snapshot = 0;
  BM_BufferPool *bm = MAKE_POOL();
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  testName = "Testing LRU page replacement";

  CHECK(createPageFile("testbuffer.bin"));
  createDummyPages(bm, 100);
  CHECK(initBufferPool(bm, "testbuffer.bin", 5, RS_LRU, NULL));

  // reading first five pages linearly with direct unpin and no modifications
  for(i = 0; i < 5; i++)
    {
      pinPage(bm, h, i);
      unpinPage(bm, h);
      ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading in pages");
    }

  // read pages to change LRU order
  for(i = 0; i < numLRUOrderChange; i++)
    {
      pinPage(bm, h, orderRequests[i]);
      unpinPage(bm, h);
      ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");
    }

  // replace pages and check that it happens in LRU order
  for(i = 0; i < 5; i++)
    {
      pinPage(bm, h, 5 + i);
      unpinPage(bm, h);
      ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");
    }

  // check number of write IOs
  ASSERT_EQUALS_INT(0, getNumWriteIO(bm), "check number of write I/Os");
  ASSERT_EQUALS_INT(10, getNumReadIO(bm), "check number of read I/Os");


  CHECK(shutdownBufferPool(bm));
  CHECK(destroyPageFile("testbuffer.bin"));

  free(bm);
  free(h);
  TEST_DONE();
}

// test error cases
void
testError (void)
{
  int i;
  BM_BufferPool *bm = MAKE_POOL();
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  testName = "Testing LRU page replacement";

   CHECK(createPageFile("testbuffer.bin"));

  // pin until buffer pool is full and request additional page
  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));
  CHECK(pinPage(bm, h, 0));
  CHECK(pinPage(bm, h, 1));
  CHECK(pinPage(bm, h, 2));
  ASSERT_ERROR(pinPage(bm, h, 3), "try to pin page when pool is full of pinned pages");
  CHECK(shutdownBufferPool(bm));

  // try to ready page with negative page number
  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));
  ASSERT_ERROR(pinPage(bm, h, -1), "try to pin page with negative page number");
  CHECK(shutdownBufferPool(bm));

  // try to use uninitialized buffer pool
  ASSERT_ERROR(shutdownBufferPool(bm), "shutdown buffer pool that is not open");
  ASSERT_ERROR(forceFlushPool(bm), "flush buffer pool that is not open");
  ASSERT_ERROR(pinPage(bm, h, 1), "pin page in buffer pool that is not open");
  ASSERT_ERROR(initBufferPool(bm, "xxx.bin", 3, RS_FIFO, NULL), "try to init buffer pool for non existing page file");

  // try to unpin, mark, or force page that is not in pool
  CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));
  ASSERT_ERROR(unpinPage(bm, h), "unpin page not in buffer pool");
  ASSERT_ERROR(forcePage(bm, h), "unpin page not in buffer pool");
  ASSERT_ERROR(markDirty(bm, h), "mark page dirty that is not in buffer pool");
  CHECK(shutdownBufferPool(bm));

  // done remove page file
  CHECK(destroyPageFile("testbuffer.bin"));

  free(bm);
  free(h);
  TEST_DONE();
}
