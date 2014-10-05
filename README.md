Advance-Database-Organization
=============================

Database - Buffer Manager, Storage Manager, Record Manager 

										README - B+Tree.
CONTENTS:
1.OverView of Design.
2.OverView of Implementation.
3.Functional Descriptions.

/***************************************************************************************************
 * OverView of Design
 * *************************************************************************************************/
B+-tree index is implemented in this assignment. The index is backed up by a page file and pages of the index are accessed through the buffer manager. Each node  occupies one page.  A B+-tree stores pointer to records (RID) index by a keys of a given datatype.

Index Manager Functions:
These functions are used to initialize the index manager and shut it down, freeing up all acquired resources.

B+-tree Functions:
These functions are used to create, open, close, delete a b-tree index. 

Key Functions:
These functions are used to find, insert, and delete keys in/from the B+-tree. FindKey returns the RID for the entry with the search key in the b-tree. If the key does not exist this function returns RC_IM_KEY_NOT_FOUND. insertKey inserts a new key and record pointer pair into the index. It returns error code RC_IM_KEY_ALREADY_EXISTS if this key is already stored in the b-tree. deleteKey removes a key and also the corresponding record pointer from the index. It returns RC_IM_KEY_NOT_FOUND if the key is not in the index. For deletion it is up to the client whether this is handled as an error.

Debug functions:
printTree is used to create a string representation of the b+-tree. Each node of a b-tree is represented as one line in the string representation.RIDs are represented as PageNumber.Slot.

Utility Functions:
These functions are support functions used to handle B+-Tree functions like pin/unpin, merge, inserKeyToNode,etc..

/***************************************************************************************************
 * OverView of Implementation
 * *************************************************************************************************/

    


/***************************************************************************************************
 * Functional description.
 * *************************************************************************************************/
// create, destroy, open, and close an btree index

/*Version: 1.0.0
 * Method Name: initIndexManager 
 * Arguments: *mgmtData
 * Behavior: Intializes Index Manager
 
 
 
/*Version: 1.0.0
 * Method Name: shutdownIndexManager
 * Behaviour: Shutdowns Index Manager */

 
 /*Version: 1.0.0
 * Method Name: createBtree
 * Arguments: char *idxId, DataType keyType, int n
 * Behavior: Creates a B+-Tree
 * Algorithm: 1) Initializes B+-Tree
              2) Stops if order of 'n' is too high to fit into a page 
	      3) Updates the page when new root is generated */

 
/*Version: 1.0.0
 * Method Name: openBtree
 * Arguments: TreeHandle **tree, char *idxId
 * Behavior: Before a client can access a b+-tree index it is opened
 * Algorithm: 1) Setup Buffer Manager
	      2) Reads page and prepares schema
	      3) Unpin after reading */

/*Version: 1.0.0
 * Method Name: closeBtree
 * Arguments: BTreeHandle *tree
 * Behavior: To free the memory space occupied by B+ Tree
 * Algorithm: 1) Free the memory occupied by B+-Tree */ 

/*Version: 1.0.0
 * Method Name: deleteBtree 
 * Arguments: char *idxId
 * Behavior: Removes the corresponding page file. 
 * Algorithm: 1) Deletes B+-Tree */


//********************************* // access information about a b-tree

/* Method Name: getNumNodes
 * Arguments: BTreeHandle *tree, int *result
 * Behavior: Gets the number of nodes in B+-Tree
 * Version: 1.0.0 */

 /* Name: getNumEntries
 * Arguments: BTreeHandle *tree, int *result
 * Behavior: Gets the total number of keys in B+-Tree.
 * Version: 1.0.0 */

/* Name: getKeyType 
 * Arguments: BTreeHandle *tree, DataType *result
 * Behavior: Get the Datatype of Key
 * Version: 1.0.0 */


//********************************* index access
/* Name: findKey
 * Arguments: BTreeHandle *tree, Value *key, RID *result 
 * Behavior: Search for the key in the B+-Tree
 * Version: 1.0.0 */


/* Name: insertKey
  Arguments: BTreeHandle *tree, Value *key, RID rid
 * Behavior: Inserts key and record pointer pair into the index
 * Version: 1.0.0 
* Algorithm : 1)Returns error code RC_IM_KEY_ALREADY_EXISTS if the key already exists.
2)Returns RC_IM_KEY_NOT_FOUND if the key is not in the index.*/
 

/* Name: deleteKey
 * Arguments: BTreeHandle *tree, Value *key
 * Behavior:Deletes key from the B+-Tree
 * Version: 1.0.0 */

/* Name: openTreeScan
 * Arguments: BTreeHandle *tree, BT_ScanHandle **handle
 * Behavior: Scans through all entries in B+-Tree
 * Version: 1.0.0 */


/* Name: nextEntry 
 * Arguments: BT_ScanHandle *handle, RID *result 
 * Behavior: Finds the next entry in the B+-Tree  
 * Version: 1.0.0 
 * Algorithm : 1) Returns RC_IM_NO_MORE_ENTRIES if there are no more entries to be returned */


/* Name: closeTreeScan 
 * Arguments: BT_ScanHandle *handle
 * Behavior: Closes the tres scan.
 * Version: 1.0.0 
 *Algorithm : 1)Changes are written to the page file before closing the table. */


*****************************// debug and test functions

/* Name: printTree 
 *Arguments: BTreeHandle *tree
 * Behavior: Creates a string representation of a b-tree
 * Version: 1.0.0 */

*****************************// Utility Functions

1) static void unpinNode(BTreeHandle *treeHandle, PageNumber pNum)
2) insertKeyToNode(BTreeHandle *tree, NodeStruct *node, long long ptr, Value *key)
3) instantiateNode(BTreeHandle *tree)
4) insertKeyToParent(BTreeHandle *tree, PageNumber leftPn, PageNumber rightPn, Value key)
5) splitter(BTreeHandle *tree, PageNumber lpn, bool isLeaf)
6) delKeyHelper(BTreeHandle *tree, NodeStruct *node, PageNumber ptr, Value *key)
7) pageNumber getAdjacentNode(BTreeHandle *tree, PageNumber pn)
8) deleteElement(BTreeHandle *tree, PageNumber fromPn, Value *key)
9) distribute(BTreeHandle *tree, PageNumber lpn, PageNumber rpn)
10) merge(BTreeHandle *tree, PageNumber lpn, PageNumber rpn)
11) printTreeHelper(BTreeHandle *treeHandle, char *outputBuffer)

