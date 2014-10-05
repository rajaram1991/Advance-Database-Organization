										README - RECORD MANAGER.
CONTENTS:
1.OverView of Design.
2.OverView of Implementation.
3.Functional Descriptions.

/***************************************************************************************************
 * OverView of Design
 * *************************************************************************************************/
Following are the structures used and its role.
RM_TableData - used to create and interact for a given table name.Its also used to store Buffer pool pointer of a table in its management data.
Schema - Used to maintain a schema for a given table.
Scan_Help -This structure is used to handle all the necessity details for a scan to take place.
Scan_Node - Its a linked list node to maintain active scans. Its also used to map current scan with scan_help mentioned above.

We create a linked list to maintain and handle two or more scans for a given table. We also maintain the schema of current table in memory.

Scan_Node contains all the details of the scan such as , track of last page number , last slot number ,  total records in a page and current page number.

Tuple Representation in pAgeFile :
|A,B,C|D,E,F

A,B,C - Tuple 1
D,E,F - Tuple 2

Note: mgmtinfo of RM_TableData is used to store the pointer of BM_BufferPool .

/***************************************************************************************************
 * OverView of Implementation
 * *************************************************************************************************/
When a table needs to be created , a pagefile for the table is created. Corresponding pagefile is accessed using a Buffer Pool. All the operations of the table such as insert , delete , update and scan are carried out using this buffer pool.Schema of the active table is cached in Memory 'Global schema'.
Buffer pool is initialized with 5 Page frames and FIFO as it's replacement strategy.

When a scan is initiated, its inserted into scan_node of the linked list. This gives a good concurrency to handle multiple scans at the same time.

We manage memory using tombstone technique. Once a record has been deleted , we mark tombstone in that address so that it no longer be considered for record manipulation.Once the pagefile has been reorganized , the tombstone will be removed and free space is given back to the file.

We also mark or set the page header for corresponding slots to '1' if that slot record has been deleted.

Note: We implemented TOMBSTONE CONCEPT by marking the space thats been freed using '>'. So those addresses with value '>' wont be considered for successive operation.
    


/***************************************************************************************************
 * Functional description.
 * *************************************************************************************************/
/*Version: 1.0.0
 * Method Name: getRecordSize
 * Arguments: Schema *schema
 * Returns: Size of the record
 * Purpose: To get the size of a record in a given schema
 * Algorithm: 1) Find the number of attributes from the schema
 * 				2) Find the datatype for each attribute and sum the size occupied by it in bytes.
 * 				3) Return the total size occupied by all the attribute datatypes in byte */
 
 
/*Version: 1.0.0
 * Method Name: createSchema
 * Arguments: int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys
 * Returns: The created schema
 * Purpose: To create a schema
 * Algorithm: 1) Find the sum of the total size occupied the schema
 * 				2) Allocate a schema with the calculated size
 * 				3) Assign the values to the schema and return the pointer */
 
 /*Version: 1.0.0
 * Method Name: freeSchema
 * Arguments: Schema *schema
 * Returns: Status
 * Purpose: To free a schema's memory
 * Algorithm: 1) Free the space occupied by each schema*/

 
/*Version: 1.0.0
 * Method Name: createRecord
 * Arguments: Record **record, Schema *schema
 * Returns: Status of record creation
 * Purpose: To create a record
 * Algorithm: 1)  */

/*Version: 1.0.0
 * Method Name: freeRecord
 * Arguments: Record *record
 * Returns: record freeing status
 * Purpose: To free the memory space occupied by a record
 * Algorithm: 1) Free the memory occupied by the given record pointer*/

/*Version: 1.0.0
 * Method Name: getAttr
 * Arguments: Record *record, Schema *schema, int attrNum, Value **value
 * Returns: Status of getAttr method
 * Purpose: To get a attribute.
 * Algorithm:1) Get the value of record->data at attrNum.
 * 				2) Get the datatype of the corresponding value
 * 				3) Set a temporary Value
 * 				4) Copy the address of the temporaryValue to Value*/
/*Version: 1.0.0
 * Method Name: setAttr
 * Arguments: Record *record, Schema *schema, int attrNum, Value *value
 * Returns: Status of setAttr method
 * Purpose: To set a attribute.
 * Algorithm: 1) Get the value from the parameter.
 * 				2) Go to the record element at attrNum
 * 				3) Set the set or update the value at the position
 * 				4) Make primary key check and not null check - Extra credits*/


//********************************* handling records in a table
/* Name: insertRecord
 * Behavior:Would insert a new record in the available page and slot , assign that to 'record' parameter.
 * Version: 1.0.0 */

 /* Name: deleteRecord
 * Behavior:Would delete a record based on the RID parameter.
 * Version: 1.0.0 */

/* Name: updateRecord
 * Behavior:Would update  a  existing record with a new value on the slot and page that has been passed .
 * Version: 1.0.0 */

/* Name: getRecord
 * Behavior:Would fetch an  existing record value based on RID parameter and assing the value to record plus assign that RID to record->RID.
 * Version: 1.0.0 */


//********************************* scans
/* Name: startscan
 * Behavior:Initializes a scan and load the active scan to a linked list.
 * Version: 1.0.0 */


/* Name: next
 * Behavior:Scan continously till a matching tuple is found.
 * Version: 1.0.0 */
1) Divide scan into two sections
2) One with expression =NULL
3)Scan thorough all pages and all slot and return that record to the client.
4)If expression is not null
5)Switch to the required operator.
6) get the column value from the table and compare with the scan criteria.
7) if match is found  , return that record to the client.
8) if not Loop until match is found.
9) If the scan is done for whole page file , return nO more tuples.

/* Name: closeScan
 * Behavior:Closes an current scan
 * Version: 1.0.0 */

RC closeScan (RM_ScanHandle *scan)
{
    Scan_Help *scanhelp=search_scandata(RM_Scan_stptr,scan);//get the current scan helper from linked list.
    free(scanhelp);// free up the scan.
    delete_Scan_node(&RM_Scan_stptr,scan);
    return RC_OK;
}

/* Name: initRecordManager
 * Behavior:Would initializes record manager
 * Version: 1.0.0 */
 

/* Name: shutdownRecordManager
 * Behavior:Would shutdown an existing record manager.
 * Version: 1.0.0 */

/* Name: createTable
 * Behavior:Creates a table in the page file.
 * Version: 1.0.0 */
1) Creates a table in the underlying page file.
2) Stores information about the schema


/* Name: openTable
 * Behavior:Opens the table from a page file.
 * Version: 1.0.0 */
1) Table is opened before inserting or scanning operations are performed

/* Name: closeTable
 * Behavior:Closes the table.
 * Version: 1.0.0 */
1)Changes are written to the page file before closing the table.

/* Name: deleteTable
 * Behavior:Deletes the table from a page file.
 * Version: 1.0.0 */

/* Name: getNumTuples
 * Behavior:Returns the count of tuples in the table.
 * Version: 1.0.0 */



