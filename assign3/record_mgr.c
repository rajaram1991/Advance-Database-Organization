#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "RM_Scan_node_ll.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***STATIC VARIABLES***/
//static Scan_Help *scan_help;
static Scan_Node_ptr RM_Scan_stptr=NULL;//Linked list node to maintain Scan handle
static Schema Globalschema;
static char textdata[1000];//Used to share string values across modules from the database
/***STATIC HELPER PROTOTYPES***/
static PageNumber getPageNo(RM_TableData *rel,BM_BufferPool *bufferPool,BM_PageHandle *pagehandle,int *currentslotid);
//Optional methods for extra credits
extern RC checkPkConstraint();
extern RC checkNotNullConstraint();


/*********************************
 * Dealing with Schemas
 * *******************************/
/*Version: 1.0.0
 * Method Name: getRecordSize
 * Arguments: Schema *schema
 * Returns: Size of the record
 * Purpose: To get the size of a record in a given schema
 * Algorithm: 1) Find the number of attributes from the schema
 * 				2) Find the datatype for each attribute and sum the size occupied by it in bytes.
 * 				3) Return the total size occupied by all the attribute datatypes in byte */
int getRecordSize(Schema *schema) {
	int numAttr = schema->numAttr;
	DataType *dtP = schema->dataTypes;
	int *typeLength = schema->typeLength;
	int dataTypeSize = 0;
	int i;
	//Loop across all the datamembers
	for (i = 0; i < numAttr; i++) {
		DataType dt = *(dtP + i);
		if (dt == DT_INT) {
			dataTypeSize += sizeof(int);
		} else if (dt == DT_FLOAT) {
			dataTypeSize += sizeof(float);
		} else if (dt == DT_BOOL) {
			dataTypeSize += sizeof(bool);
		} else if (dt == DT_STRING) { /*In case it is a String, the size will be equal to the corresponding type length*/
			dataTypeSize += typeLength[i];
		}
	}
	return dataTypeSize+numAttr;
}

/*Version: 1.0.0
 * Method Name: createSchema
 * Arguments: int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys
 * Returns: The created schema
 * Purpose: To create a schema
 * Algorithm: 1) Find the sum of the total size occupied the schema
 * 				2) Allocate a schema with the calculated size
 * 				3) Assign the values to the schema and return the pointer */
Schema *createSchema(int numAttr, char **attrNames, DataType *dataTypes,
		int *typeLength, int keySize, int *keys) {
	Schema *schema = (Schema*) malloc(sizeof(Schema));
	schema->numAttr = numAttr;
	schema->attrNames = attrNames;
	schema->dataTypes = dataTypes;
	schema->typeLength = typeLength;
	schema->keySize = keySize;
	schema->keyAttrs = keys; /*I assume that keys and keyAttrs are the same*/
	return schema;
}

/*Version: 1.0.0
 * Method Name: freeSchema
 * Arguments: Schema *schema
 * Returns: Status
 * Purpose: To free a schema's memory
 * Algorithm: 1) Free the space occupied by each schema*/
RC freeSchema(Schema *schema) {
	free(schema);
	return RC_OK;

}
/**********************************************************
 * Dealing with records and attribute values
 * ********************************************************/
/*Version: 1.0.0
 * Method Name: createRecord
 * Arguments: Record **record, Schema *schema
 * Returns: Status of record creation
 * Purpose: To create a record
 * Algorithm: 1)  */
RC createRecord(Record **record, Schema *schema) {
	int numAttr = schema->numAttr;
	DataType *dtP = schema->dataTypes;
	int *typeLength = schema->typeLength;
	int i;
	int dataMemory = 0;
	char *data;//Addded by sri
	for (i = 0; i < numAttr; i++) {
		int dt = *(dtP + i);
		if (dt == DT_INT) {
			dataMemory += sizeof(int);
		} else if (dt == DT_FLOAT) {
			dataMemory += sizeof(float);
		} else if (dt == DT_BOOL) {
			dataMemory += sizeof(bool);
		} else if (dt == DT_STRING) { /*In case it is a String, the size will be equal to the corresponding type length*/
			dataMemory += *(typeLength + i);
		}
	}
	data= (char *)malloc(dataMemory + numAttr);
	for(i=0;i < dataMemory + numAttr ;i++)data[i]='\0';
	*record = (Record *)malloc(sizeof(Record));
	record[0]->data=data;//create a char with length equals length of the tuple.
	return RC_OK;
}
/*Version: 1.0.0
 * Method Name: freeRecord
 * Arguments: Record *record
 * Returns: record freeing status
 * Purpose: To free the memory space occupied by a record
 * Algorithm: 1) Free the memory occupied by the given record pointer*/
RC freeRecord(Record *record) {
	free(record);//free record pointer.
	return RC_OK;
}

/*Version: 1.0.0
 * Method Name: getAttr
 * Arguments: Record *record, Schema *schema, int attrNum, Value **value
 * Returns: Status of getAttr method
 * Purpose: To get a attribute.
 * Algorithm:1) Get the value of record->data at attrNum.
 * 				2) Get the datatype of the corresponding value
 * 				3) Set a temporary Value
 * 				4) Copy the address of the temporaryValue to Value*/
RC getAttr(Record *record, Schema *schema, int attrNum, Value **value) {
	int numAttr = schema->numAttr;
	DataType *dtP = schema->dataTypes;
	int *typeLength = schema->typeLength;
	char *data = record->data;
    Value *val = (Value*) malloc(sizeof(Value));
    textdata[100]='\0';
    char *temp=NULL;//Used to substring the data from record and convert it to required datatype.

	int offset = 1;
	if (attrNum < numAttr) {
		int i;
		//Loop through the whole tuple to get the required offset
		for (i = 0; i < attrNum; i++) {

			if (dtP[i] == DT_INT) {
				offset += sizeof(int);
			} else if (dtP[i]== DT_FLOAT) {
				offset += sizeof(float);
			} else if (dtP[i]== DT_BOOL) {
				offset += sizeof(bool);
			} else if (dtP[i]== DT_STRING) { /*In case it is a String, the size will be equal to the corresponding type length*/
				offset += typeLength[i];
			}
		}
		offset+=attrNum;
		int sizeOfValToRead = 0;
		int dt = *(dtP + i);/*The value of i would have incremented */
		/*Set the datatype of Value*/
		if (dt == DT_INT) {
			val->dt = DT_INT;
			sizeOfValToRead += sizeof(int);
			temp=malloc(sizeOfValToRead+1);
		} else if (dt == DT_FLOAT) {
			val->dt = DT_FLOAT;
			sizeOfValToRead += sizeof(float);
			temp=malloc(sizeOfValToRead+1);
		} else if (dt == DT_BOOL) {
			val->dt = DT_BOOL;
			sizeOfValToRead += sizeof(bool);
			temp=malloc(sizeOfValToRead+1);
		} else if (dt == DT_STRING) { /*In case it is a String, the size will be equal to the corresponding type length*/
			val->dt = DT_STRING;
			sizeOfValToRead += *(typeLength + i);
			temp=malloc(sizeOfValToRead+1);
			for(i=0;i <= sizeOfValToRead ; i++)
            {
                temp[i]='\0';
            }
		}


        strncpy(temp, data + offset, sizeOfValToRead); //Check if offset has to be divided by, say 2, because offset will be in bytes and you might want to get in chars
        temp[sizeOfValToRead]='\0';
		/*Set the data element of Value*/
		if (val->dt == DT_INT) {
			val->v.intV = atoi(temp);
		} else if (val->dt == DT_FLOAT) {
			val->v.floatV = (float) *temp;
		} else if (val->dt == DT_BOOL) {
			val->v.boolV = (bool) *temp;
		} else if (val->dt == DT_STRING) {
		    for(i=0;i < sizeOfValToRead;i++)
            {
            textdata[i]=temp[i];
            }
			val->v.stringV=textdata;
		}
		value[0]=val;
		free(temp);
		return RC_OK;
	}
	/*If attrNum is greater than numAttr - Set the error message*/
	RC_message = "attrNum is greater than the available number of attributes";
	return RC_RM_NO_MORE_TUPLES;
}

/*Version: 1.0.0
 * Method Name: setAttr
 * Arguments: Record *record, Schema *schema, int attrNum, Value *value
 * Returns: Status of setAttr method
 * Purpose: To set a attribute.
 * Algorithm: 1) Get the value from the parameter.
 * 				2) Go to the record element at attrNum
 * 				3) Set the set or update the value at the position
 * 				4) Make primary key check and not null check - Extra credits*/
RC setAttr(Record *record, Schema *schema, int attrNum, Value *value) {
	int numAttr = schema->numAttr,j,temp;
	DataType *dtP = schema->dataTypes;
	int *typeLength = schema->typeLength;
	//char *data = record->data;

	int offset = 1;
	if (attrNum < numAttr) {
		int i;
		for (i = 0; i <attrNum; i++) {
			int dt = *(dtP + i);
			if (dt == DT_INT) {
				offset += sizeof(int);
			} else if (dt == DT_FLOAT) {
				offset += sizeof(float);
			} else if (dt == DT_BOOL) {
				offset += sizeof(bool);
			} else if (dt == DT_STRING) { /*In case it is a String, the size will be equal to the corresponding type length*/
				offset += *(typeLength + i);
			}
		}
		offset+=attrNum;

		/*Since this is a pointer, modifying the local *data will in turn modify the particular record*/
        char *addrToModify ;
		if(attrNum==0)
        {
            addrToModify =record->data ;
            addrToModify[0]='|';//Used to sepearate different tuples in a page file.
            addrToModify++;
        }
        else
        {
            addrToModify=record->data+offset;
            (addrToModify-1)[0]=',';//Comma seperator for records in a tuple.
        }


		int dt = value->dt;
		/*Set the datatype of Value*/
		if (dt == DT_INT) {
			sprintf(addrToModify,"%d",value->v.intV);
			while(strlen(addrToModify)!=sizeof(int))
            {
                strcat(addrToModify,"0");
            }
//format integer to string
    for (i=0,j=strlen(addrToModify)-1 ; i < j;i++,j--)
    {
        temp=addrToModify[i];
        addrToModify[i]=addrToModify[j];
        addrToModify[j]=temp;
    }


		} else if (dt == DT_FLOAT) {
			sprintf(addrToModify,"%f",value->v.floatV);
			while(strlen(addrToModify)!=sizeof(float))
            {
                strcat(addrToModify,"0");
            }
//format float to string
    for (i=0,j=strlen(addrToModify)-1 ; i < j;i++,j--)
    {
        temp=addrToModify[i];
        addrToModify[i]=addrToModify[j];
        addrToModify[j]=temp;
    }
		} else if (dt == DT_BOOL) {
			sprintf(addrToModify,"%i",value->v.boolV);//Convert bool to string
		} else if (dt == DT_STRING) { /*In case it is a String, the size will be equal to the corresponding type length*/
			sprintf(addrToModify,"%s",value->v.stringV);
		}
		return RC_OK;
	}
	/*If attrNum is greater than numAttr - Set the error message*/
	RC_message = "attrNum is greater than the available number of Attributes";
	return RC_RM_NO_MORE_TUPLES;
}


/*********************************
 * Utility Functions
 * *******************************/
/*Version: 1.0.0
 * Method Name: checkPkConstraint
 * Arguments: RM_TableData *rel, Record *record, RID id
 * Returns: "proceed" message if there is no violation and "stop" message if there is a constraint violation.
 * Purpose: To check if there is any constraint violation to perform insert/delete/update operation
 * Algorithm:  */
RC checkPkConstraint(RM_TableData *rel, Record *record, RID id){
	RC_message = "proceed";
	if(record == NULL){
		/*Its a check for delete operation*/
		return RC_OK;
	}
	/*Its a check for insert or update operations*/
	return RC_OK;
}


/*Version: 1.0.0
 * Method Name: checkNotNullConstraint
 * Arguments: RM_TableData *rel, Record *record, RID id
 * Returns: "proceed" message if there is no violation and "stop" message if there is a constraint violation.
 * Purpose: To check if there is any constraint violation to perform insert/delete/update operation
 * Algorithm:  */
RC checkNotNullConstraint(RM_TableData *rel, Record *record){
	/*Its a check for insert or update operations - we need not bother if it is a delete*/
	RC_message = "proceed";
	return RC_OK;
}

//********************************* handling records in a table
/* Name: insertRecord
 * Behavior:Would insert a new record in the available page and slot , assign that to 'record' parameter.
 * Version: 1.0.0 */
RC insertRecord (RM_TableData *rel, Record *record)
{
    PageNumber _pgno;
    RID _id;
    int slotID=0;
    char *freespace=NULL;
    BM_PageHandle *page=MAKE_PAGE_HANDLE();
    BM_BufferPool *bm=(BM_BufferPool *)rel->mgmtData;
    _pgno=getPageNo(rel,bm,page,&slotID);//get the appropriate page number where slot is free to insert the record.
    pinPage(bm,page,_pgno);//pin that page
    freespace=page->data;
    freespace=freespace+strlen(page->data);
    strcpy(freespace,record->data);//write that record to the page.
    markDirty(bm,page);//mark dirty
    unpinPage(bm,page);//unpin
    _id.page=_pgno;//assign corresponding page and slot id.
    _id.slot=slotID;
    record->id=_id;
    free(page);
    return RC_OK;
}
/* Name: deleteRecord
 * Behavior:Would delete a record based on the RID parameter.
 * Version: 1.0.0 */
RC deleteRecord (RM_TableData *rel, RID id)
{
    int i ,totalrecordlength;
    char *spaceToBeCleared=NULL;
    BM_PageHandle *page=MAKE_PAGE_HANDLE();
    BM_BufferPool *bm=(BM_BufferPool *)rel->mgmtData;
    PageNumber _pgno=id.page;
    int slot=id.slot;
    totalrecordlength=getRecordSize(rel->schema);//gets the total record size that needs to be deleted.
    if(pinPage(bm,page,_pgno)==RC_OK)
    {
        spaceToBeCleared=page->data;
        spaceToBeCleared =spaceToBeCleared +totalrecordlength*slot;
        for(i=0;i<totalrecordlength;i++)
        {
            spaceToBeCleared [i]='>';//We implement TOMBSTONE concept to handle deletion and freespace. '>' will indicate that this memory will no longer be involve in record insertion. On page reorganization this memory will be freed up and given to the system.
        }
        markDirty(bm,page);
        unpinPage(bm,page);
    }
    free(page);
    return RC_OK;

}

/* Name: updateRecord
 * Behavior:Would update  a  existing record with a new value on the slot and page that has been passed .
 * Version: 1.0.0 */
RC updateRecord (RM_TableData *rel, Record *record)
{
    int totalrecordlength;
    char *spaceToBeUpdated;
    BM_PageHandle *page=MAKE_PAGE_HANDLE();
    BM_BufferPool *bm=(BM_BufferPool *)rel->mgmtData;
    RID id=record->id;
    PageNumber _pgno=id.page;//gets the appropriate page that needs to be updated.
    int slot=id.slot;//gets the appropriate slot that needs to be updated.
    totalrecordlength=getRecordSize(rel->schema);
    if(pinPage(bm,page,_pgno)==RC_OK)
    {
        spaceToBeUpdated=page->data;
        spaceToBeUpdated =spaceToBeUpdated +totalrecordlength*slot;//calculate the address that needs to be updated.
        strncpy(spaceToBeUpdated,record->data,totalrecordlength);// copy the contents.
        markDirty(bm,page);
        unpinPage(bm,page);
    }
    free(page);
    return RC_OK;

}

/* Name: getRecord
 * Behavior:Would fetch an  existing record value based on RID parameter and assing the value to record plus assign that RID to record->RID.
 * Version: 1.0.0 */
RC getRecord (RM_TableData *rel, RID id, Record *record)
{
    int totalrecordlength;
    char *spaceToBeReturned;
    BM_PageHandle *page=MAKE_PAGE_HANDLE();
    BM_BufferPool *bm=(BM_BufferPool *)rel->mgmtData;
    PageNumber _pgno=id.page;
    int slot=id.slot;
    totalrecordlength=getRecordSize(rel->schema);
    if(pinPage(bm,page,_pgno)==RC_OK)
    {
        spaceToBeReturned=page->data;
        spaceToBeReturned =spaceToBeReturned+totalrecordlength*slot;
        strncpy(record->data,spaceToBeReturned,totalrecordlength);
        record->id=id;
        markDirty(bm,page);
        unpinPage(bm,page);
    }
    free(page);
    return RC_OK;

}

//********************************* scans
/* Name: getRecord
 * Behavior:Initializes a scan and load the active scan to a linked list.
 * Version: 1.0.0 */
RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{
    scan->rel=rel;
    scan->mgmtData= cond;
    BM_BufferPool *bufferPool=(BM_BufferPool *)rel->mgmtData;
    SM_FileHandle *sh=(SM_FileHandle *)bufferPool->mgmtData;
    Scan_Help *scan_help=(Scan_Help *)malloc(sizeof(Scan_Help));//This is used to handle multiple scans for a same file where active scans are loaded in linked list.
    scan_help->_page=1;
    scan_help->_slot=1;
    scan_help->_totalnumPages=sh->totalNumPages;
    scan_help->_totalrecordsinpage=0;

    scan_help->_totalrecordlength=getRecordSize(rel->schema);
    scan_help->ph=MAKE_PAGE_HANDLE();
    insert_Scan_node(&RM_Scan_stptr,scan,scan_help);//Insert active scan to a linked list.
    return RC_OK;

}
/* Name: next
 * Behavior:Scan continously till a matching tuple is found.
 * Version: 1.0.0 */
RC next (RM_ScanHandle *scan, Record *record)
{
    int pagelength;
    Expr *_expr=(Expr *)scan->mgmtData,*secondaryexpr,*left,*right;
    Operator *scancriteria,*secondscancriteria;
    RM_TableData *_rel=scan->rel;
    Scan_Help *scan_help=search_scandata(RM_Scan_stptr,scan);
    RID _id;
    Value **columnvalue=(Value **)malloc(sizeof(Value *));//Exact column value thats needs to be checked in expression.
    *columnvalue=NULL;
    bool matchfound=FALSE;

//We divide scans into two. One with expression and other NULL
    if(_expr==NULL)
    {
        //1.Return all the tuples.
        //2.Once all the tuples returned... return no more tuples.
      if(scan_help->_page < scan_help->_totalnumPages)//Scan through all the pages of the table.
      {
          pinPage(_rel->mgmtData,scan_help->ph,scan_help->_page);
          pagelength=strlen(scan_help->ph->data);
          scan_help->_totalrecordsinpage=pagelength/scan_help->_totalrecordlength;
          if(scan_help->_slot < scan_help->_totalrecordsinpage)//scan through all the slots and get the record.
          {
              _id.page=scan_help->_page;
              _id.slot=scan_help->_slot;
              getRecord(_rel,_id,record);
              scan_help->_slot++;
          }
          else
          {
              scan_help->_page+=1;
              scan_help->_slot=1;
          }
          unpinPage(_rel->mgmtData,scan_help->ph);
          free(columnvalue[0]);
          free(columnvalue);
          return RC_OK;
      }
      else
      {
          free(columnvalue[0]);
          free(columnvalue);
          return RC_RM_NO_MORE_TUPLES;//Once done with the scans.. returns no more tuples to indicate end of the scan.
      }
    }
    else
    {
        //1.return those tuples that satisfy the condition alone.
        //2. Once done return no more tuples.
    scancriteria=_expr->expr.op;//Get the main scan criteria.
    switch(scancriteria->type)
      {
      case OP_COMP_EQUAL://do the above operation for equal criteria.
        left=scancriteria->args[0];
        right=scancriteria->args[1];
        while(scan_help->_page < scan_help->_totalnumPages)
      {
          pinPage(_rel->mgmtData,scan_help->ph,scan_help->_page);
          pagelength=strlen(scan_help->ph->data);
          scan_help->_totalrecordsinpage=pagelength/scan_help->_totalrecordlength;
          while(scan_help->_slot < scan_help->_totalrecordsinpage)
          {
              _id.page=scan_help->_page;
              _id.slot=scan_help->_slot;
              getRecord(_rel,_id,record);
              getAttr(record,_rel->schema,right->expr.attrRef,columnvalue);
              if(_rel->schema->dataTypes[right->expr.attrRef]==DT_INT){
              if(columnvalue[0]->v.intV == left->expr.cons->v.intV)//Check whether the scan condition and column value from page are equal for integers.
              {
                 scan_help->_slot++;
                 unpinPage(_rel->mgmtData,scan_help->ph);
                 matchfound=TRUE;//Indicate that a match has been found.
                 break;
              }
              }
              else if(_rel->schema->dataTypes[right->expr.attrRef]==DT_STRING)//Check whether the scan condition and column value from page are equal for integers.
                {
                    if(strcmp(columnvalue[0]->v.stringV , left->expr.cons->v.stringV)==0)
                {
                 scan_help->_slot++;
                 unpinPage(_rel->mgmtData,scan_help->ph);
                 matchfound=TRUE;
                 break;
                }
                }
                 else if(_rel->schema->dataTypes[right->expr.attrRef]==DT_FLOAT)//Check whether the scan condition and column value from page are equal for float values.
                {
                    if(columnvalue[0]->v.floatV == left->expr.cons->v.floatV)
                {
                 scan_help->_slot++;
                 unpinPage(_rel->mgmtData,scan_help->ph);
                 matchfound=TRUE;
                 break;
                }
                }

               scan_help->_slot++;
           }
          if(matchfound==TRUE)
            break;
          else
          {
           scan_help->_page+=1;
           scan_help->_slot=1;
           unpinPage(_rel->mgmtData,scan_help->ph);
          }
      }
      break;
      case OP_COMP_SMALLER:
        left=scancriteria->args[0];
        right=scancriteria->args[1];
        while(scan_help->_page < scan_help->_totalnumPages)
      {
          pinPage(_rel->mgmtData,scan_help->ph,scan_help->_page);
          pagelength=strlen(scan_help->ph->data);
          scan_help->_totalrecordsinpage=pagelength/scan_help->_totalrecordlength;
          while(scan_help->_slot < scan_help->_totalrecordsinpage)
          {
              _id.page=scan_help->_page;
              _id.slot=scan_help->_slot;
              getRecord(_rel,_id,record);
              getAttr(record,_rel->schema,right->expr.attrRef,columnvalue);
              if(_rel->schema->dataTypes[right->expr.attrRef]==DT_INT){
              if(columnvalue[0]->v.intV < left->expr.cons->v.intV)
              {
                 scan_help->_slot++;
                 unpinPage(_rel->mgmtData,scan_help->ph);
                 matchfound=TRUE;
                 break;
              }
              }
              scan_help->_slot++;
          }
          if(matchfound==TRUE)
            break;
          else
          {
           scan_help->_page+=1;//Increment the page.
           scan_help->_slot=1;//Reset the slot.
           unpinPage(_rel->mgmtData,scan_help->ph);
          }
      }
      break;
      case OP_BOOL_NOT://seperate case for boolean NOT !
          secondaryexpr=_expr->expr.op->args[0];
          secondscancriteria=secondaryexpr->expr.op;
          left=secondscancriteria->args[1];
          right=secondscancriteria->args[0];

        switch(secondscancriteria->type)

        {
        case OP_COMP_SMALLER:
      while(scan_help->_page < scan_help->_totalnumPages)
      {
          pinPage(_rel->mgmtData,scan_help->ph,scan_help->_page);
          pagelength=strlen(scan_help->ph->data);
          scan_help->_totalrecordsinpage=pagelength/scan_help->_totalrecordlength;
          while(scan_help->_slot < scan_help->_totalrecordsinpage)
          {
              _id.page=scan_help->_page;
              _id.slot=scan_help->_slot;
              getRecord(_rel,_id,record);
             getAttr(record,_rel->schema,right->expr.attrRef,columnvalue);
              if(_rel->schema->dataTypes[right->expr.attrRef]==DT_INT){
              if(columnvalue[0]->v.intV > left->expr.cons->v.intV)
              {
                 scan_help->_slot++;
                 unpinPage(_rel->mgmtData,scan_help->ph);
                 matchfound=TRUE;
                 break;
              }
              }
               scan_help->_slot++;
            }
          if(matchfound==TRUE)
            break;
          else
          {
           scan_help->_page+=1;
           scan_help->_slot=1;
           unpinPage(_rel->mgmtData,scan_help->ph);
          }
      }
      break;

      }
      break;

      }
      free(*columnvalue);//free the column value we created.
      free(columnvalue);////free the column value we created.
      if(matchfound==TRUE)
return RC_OK;//end the scan once the match is found.
else
    return RC_RM_NO_MORE_TUPLES;
 }
}

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


/*********HELPER METHODS**********/
//To get page number
PageNumber getPageNo(RM_TableData *rel,BM_BufferPool *bufferPool,BM_PageHandle *pagehandle,int *currentslotid)
{
    PageNumber _pgno=1;
    int pagelength,totalrecordlength;
    SM_FileHandle *sh=(SM_FileHandle *)bufferPool->mgmtData;
    totalrecordlength=getRecordSize(rel->schema);
    //Scan through all the pages to find an empty slot.
    while(_pgno < sh->totalNumPages)
    {
      pinPage(bufferPool,pagehandle,_pgno);
      pagelength=strlen(pagehandle->data);
      if(PAGE_SIZE-pagelength > totalrecordlength)//If found empty slot. get the slot id.
      {
          *currentslotid=pagelength/totalrecordlength;
          unpinPage(bufferPool,pagehandle);
          break;
      }
       unpinPage(bufferPool,pagehandle);
      _pgno++;
  }
  if(*currentslotid==0)//If not slot id is found , mean all pages are full , append the page file and return that page number.
  {
      pinPage(bufferPool,pagehandle,_pgno + 1);
      unpinPage(bufferPool,pagehandle);
  }
  return _pgno;
}

/**********************table and Manger*/

/* Name: initRecordManager
 * Behavior:Initializes and record manager.
 * Version: 1.0.0 */
RC initRecordManager (void *mgmtData)
{
    return RC_OK;//We dont initialize any global data structure.
}

/* Name: shutdownRecordManager
 * Behavior:Shut downs the current record manager freeing up all the resources.
 * Version: 1.0.0 */
RC shutdownRecordManager ()
{
    return RC_OK;
}

/* Name: CReateTable
 * Behavior:We create a new page file for a given table and store the schema in the first page.
 * Version: 1.0.0 */

RC createTable (char *name, Schema *schema)
{
    char filename[100]={'\0'};
    int i,pos=0;
    strcat(filename,name);
    strcat(filename,".bin");//We create the page file as  a binary file with "bin" extension.
    createPageFile(filename);
    BM_PageHandle *ph=MAKE_PAGE_HANDLE();
    BM_BufferPool *bm=MAKE_POOL();
    initBufferPool(bm,filename,1,RS_FIFO,NULL);
    pinPage(bm,ph,0);
    for(i=0;i < schema->numAttr;i++)
    {
      pos+=sprintf(ph->data+pos,"Numattr-%d,DataType[%d]-%d,Typelength[%d]=%d",schema->numAttr,i,schema->dataTypes[i],i,schema->typeLength[i]);//Write the schema by getting the offset.
    }
    markDirty(bm,ph);
    unpinPage(bm,ph);
    forceFlushPool(bm);
    shutdownBufferPool(bm);
    Globalschema=*schema;//This is used as an cache for a active table instead of reading Schema every time from file.
    return RC_OK;
}
/* Name: OpenTable
 * Behavior:Access the table by accessing the page file using buffer manager we created.
 * Version: 1.0.0 */

RC openTable (RM_TableData *rel, char *name)
{
    char filename[100]={'\0'};
    Schema *schema=(Schema *)malloc(sizeof(Schema));
    strcat(filename,name);
    strcat(filename,".bin");
    BM_BufferPool *bm=MAKE_POOL();//We access the page file using buffer manager.
    initBufferPool(bm,filename,4,RS_FIFO,NULL);//Create a new buffer pool with FIFO logic.
    rel->name=name;
    rel->mgmtData=bm;
    *schema=Globalschema;
    rel->schema=schema;
    return RC_OK;
}
/* Name: CloseTable
 * Behavior:We close the current table access by clearing up the buffer pool and Schema created.
 * Version: 1.0.0 */

RC closeTable (RM_TableData *rel)
{
    BM_BufferPool *bm=(BM_BufferPool *)rel->mgmtData;
    freeSchema(rel->schema);//Free up the schema.
    shutdownBufferPool(bm);//Shutdown the pool associated with that table.
    free(bm);
    return RC_OK;
}

/* Name: deleteTable
 * Behavior:We delete the current table by deleting the page file.
 * Version: 1.0.0 */
RC deleteTable (char *name)
{
    char filename[100]={'\0'};
    strcat(filename,name);
    strcat(filename,".bin");
    destroyPageFile(filename);//delete the page file
    return RC_OK;
}

/* Name: getNumTuples
 * Behavior:Get the count of touples for a given page.
 * Version: 1.0.0 */
int getNumTuples (RM_TableData *rel)
{
    int i,tuplesCount=0;
    PageNumber _pgno=1;
    int pagelength,totalrecordlength;
    BM_BufferPool *bufferPool=(BM_BufferPool *)rel->mgmtData;
    BM_PageHandle *pagehandle=MAKE_PAGE_HANDLE();//Create a page handle for this scan alone.
    SM_FileHandle *sh=(SM_FileHandle *)bufferPool->mgmtData;
    totalrecordlength=getRecordSize(rel->schema);
    while(_pgno < sh->totalNumPages)//Loop through all the page files.
    {
      pinPage(bufferPool,pagehandle,_pgno);
      pagelength=strlen(pagehandle->data);
      if(pagelength > 0)//If found empty slot. get the slot id.
      {
          for(i=0;i < PAGE_SIZE;i++)//Increment the count value in each page when a touple is found. Deleted record wont be taken into account.
          {
               tuplesCount=pagehandle->data[i]=='|'?tuplesCount+1:tuplesCount;
          }
      }
       unpinPage(bufferPool,pagehandle);
      _pgno++;
  }
    free(pagehandle);
    return tuplesCount;
}
