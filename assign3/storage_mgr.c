#include "storage_mgr.h"
#include "dberror.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include  <sys/stat.h>
//Read Any Block

/********************************************************************************
*                                   Implementation                                       *
*********************************************************************************/
void initStorageManager (void)
{
//Dont do anything as we do not have any global data structure to initialise

}

/*CreatePageFile: This method could be used to create an empty file with size as big as the size of a page block*/
RC createPageFile (char *fileName)
{
    FILE *fp;
    /*To check if the file already exists*/
    if(access(fileName, 0)!=-1)
    {
        printf("This file is already present\n\n");
        return RC_OK; //appropriate error code is not present
    }
    fp=fopen(fileName,"wb"); //opens the file
    if(fp==NULL)
    {
        printf("Could not create file\n\n");
    }
    else
    {
        printf("The file, %s has been successfully created\n\n", fileName);
//        char c[PAGE_SIZE];
        int  i;
        //One page block would be a single line of a file.
        for(i=0; i<PAGE_SIZE; i++)
        {
            fprintf(fp,"%c",'\0'); //Writing a file as long as the PAGE_SIZE and filling it with 0s
        }
        fclose(fp);
    }
    return RC_OK;
}

/*openPageFile: Opens a file to read or write*/
RC openPageFile (char *fileName, SM_FileHandle *fHandle)
{
    FILE *fp;
    //Check if the file with the provided filename exists or not
    if(access(fileName, 0)!=-1)
    {
        fp = fopen(fileName,"rb+");
        if(fp==NULL)
        {
            printf("Sorry, no such file exists");
        }
        struct stat st;
        //Calculate the total number of pages that the file has
        if (stat(fileName, &st) == 0)
        {
            unsigned long int numberOfPageBlocks;
            numberOfPageBlocks = (st.st_size)/PAGE_SIZE;
            fHandle->totalNumPages =(st.st_size)%PAGE_SIZE > 0? numberOfPageBlocks+1:numberOfPageBlocks;
        }
        //Initiatinng the basic structure
        fHandle->fileName=fileName;
        fHandle->curPagePos=0;
        fHandle->mgmtInfo = fp;
        return RC_OK;
    }
    else
    {
        return RC_FILE_NOT_FOUND;
    }
}

/*closePageFile: Closes the file that we give as input and frees the memory*/
RC closePageFile (SM_FileHandle *fHandle)
{
    if(fHandle==NULL)
    {
        return RC_FILE_HANDLE_NOT_INIT;
    }
    if(fclose(fHandle->mgmtInfo)==0)
    {
        printf("The file named %s has been successfully closed",fHandle->fileName);
        return RC_OK;
    }
    else
    {
        printf("Error occurred in closing the file");
        return RC_FILE_NOT_FOUND;
    }
}

/*DestroyPageFile: To delete the file completely from secondary memory*/
RC destroyPageFile (char *fileName)
{
    //closePageFile(fHandle);
    if(remove(fileName)==0)
    {
//        printf("File named %s has been successfully destroyed/deleted\n\n", fileName);
        return RC_OK;
    }
    else
    {
        printf("Error occurred in closing the file\n\n");
        RC_message="Error occurred in closing the file";
        return RC_FILE_NOT_FOUND;
    }
}
///////////Read//////////////////////////////////////////////////////////////
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    RC ret=RC_OK;//default status
    if(fHandle->mgmtInfo==NULL||pageNum >fHandle->totalNumPages )//Check for existence
    {
        ret=RC_READ_NON_EXISTING_PAGE;//Return specifed RC value
    }
    else
    {
        fseek(fHandle->mgmtInfo,PAGE_SIZE*(pageNum),SEEK_SET);//Move the file pointer to the desried postion

        fread(memPage,PAGE_SIZE,1,fHandle->mgmtInfo);//read the content to mempage  passed.

        fHandle->curPagePos=pageNum;//Set the current page position as soon as page is read.


    }

    return ret;//return final status.
}
//Get the current block position at that instant
int getBlockPos (SM_FileHandle *fHandle)
{
    int _pos=0;
    if(fHandle!=NULL)
    {
        _pos=fHandle->curPagePos;
    }
    else
    {
        RC_message="Error in the system";
        printf("%s",RC_message);
    }

    return _pos;
}
//Read first block of the file.
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    RC ret=RC_OK;//default status
    FILE *file;//File pointer
    file=fHandle->mgmtInfo;

    if(file==NULL )//Check for existence
    {
        printError(RC_READ_NON_EXISTING_PAGE);
        ret=RC_READ_NON_EXISTING_PAGE;
    }
    else
    {
        fseek(file,0 ,SEEK_SET);//Move the file pointer to the desried postion
        rewind(file);//Move the file descriptor to First page
        fread(memPage,PAGE_SIZE,1,file);
        fHandle->curPagePos=1;//Set the first page position
    }

    return ret;//return final status.

}
//Read last page of the file.
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    RC ret=RC_OK;//default status
    FILE *file;//File pointer
    file = fHandle->mgmtInfo;//Easy use of variable
    if(file==NULL )//Check for existence
    {
        ret=RC_READ_NON_EXISTING_PAGE;
    }
    else
    {
        fseek(file,PAGE_SIZE*((fHandle->totalNumPages-1)) ,SEEK_SET);//Move the file pointer to the desried postion
        fread(memPage,PAGE_SIZE,1,file);//read the content to mempage as passed.
        fHandle->curPagePos=fHandle->totalNumPages;//Set the Last  page position as soon as page is read.
    }

    return ret;//return final status.

}
//Reading current block
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    RC ret=RC_OK;//default status
    FILE *file;//File pointer

    file=fHandle->mgmtInfo;
    if(file==NULL|| fHandle->curPagePos <0 || fHandle->curPagePos > fHandle->totalNumPages )//Check for existence
    {
        ret=RC_READ_NON_EXISTING_PAGE;
    }
    else
    {
        fseek(file,PAGE_SIZE *((fHandle->curPagePos)-1),SEEK_CUR);//Move the file pointer to the desried postion

        fread(memPage,PAGE_SIZE,1,file);//read the content of current page  to mempage
    }

    return ret;//return final status.
}
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    RC ret=RC_OK;//default status
    FILE *file;//File pointer
    file=fHandle->mgmtInfo;
    if(file==NULL|| (fHandle->curPagePos)-2 <0 )//Check for existence
    {
        ret=RC_READ_NON_EXISTING_PAGE;
    }
    else
    {
        fseek(file,PAGE_SIZE *((fHandle->curPagePos)-2),SEEK_SET);//Move the file pointer to the desried postion

        fread(memPage,PAGE_SIZE,1,file);//read the content to mempage as passed.
        fHandle->curPagePos=(fHandle->curPagePos)-1;//Set the previous page position as soon as page is read.

    }

    return ret;//return final status.

}
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    RC ret=RC_OK;//default status
    FILE *file;//File pointer

    file=fHandle->mgmtInfo;// Boxing to Local variable for easy access and use.

    if(file==NULL|| (fHandle->curPagePos)+1 > fHandle->totalNumPages )//Check for existence
    {
        ret=RC_READ_NON_EXISTING_PAGE;//return if non existing page is tried to read.
    }
    else
    {
        fseek(file,1,SEEK_CUR);//Move the file pointer to the desried postion

        fread(memPage,PAGE_SIZE,1,file);//read the content to mempage as passed.

        fHandle->curPagePos=(fHandle->curPagePos)+1;//Set the next page position as soon as page is read.
    }

    return ret;//return final status.
}


/********************************************************************************
*                                 Write                                         *
*********************************************************************************/
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    /*  check if the file write is files*/
    if(NULL == fHandle->mgmtInfo)
    {
        printf("\n  Error with the file!!!\n");
        return RC_WRITE_FAILED;
    }
    /* get the current position of the file pointer*/
    fseek(fHandle->mgmtInfo,PAGE_SIZE * pageNum ,0);
    /* write contents to the file*/
    fwrite(memPage,PAGE_SIZE,1,fHandle->mgmtInfo);
    /* Close the file*/
    //closePageFile(fHandle);
    //printf("\n File stream closed through fclose()\n");
    return RC_OK;
}
/***************************************************************************************
* 							Append													  *
***************************************************************************************/
RC appendEmptyBlock (SM_FileHandle *fHandle)
{
    int i,totalnumpages;// check for the total number of pages
    /* add one page and print '\0' in the new empty block.*/
    FILE *fp=fHandle->mgmtInfo;
    totalnumpages=fHandle->totalNumPages;
    if(fp!=NULL)
    {
        fseek(fp,PAGE_SIZE * totalnumpages ,0);
        for(i=0; i<PAGE_SIZE; i++)
        {
            fprintf(fp,"%c",'\0'); //Writing a file as long as the PAGE_SIZE and filling it with 0s
        }
        fHandle->totalNumPages+=1;
        return RC_OK;
    }
    else
    {
        return RC_FILE_NOT_FOUND;
    }
}
/***************************************************************************************
* 							write current block						   				   *
***************************************************************************************/
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    if(NULL==fHandle->mgmtInfo)
    {
        printf("No such file exitst");
        return RC_FILE_NOT_FOUND;
    }
    fwrite(memPage,PAGE_SIZE,1,fHandle->mgmtInfo);
    return RC_OK;
}
/***************************************************************************************
* 							ensure capacity													  *
***************************************************************************************/
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle)
{
    int spaceCreationFlag;
    int availablePageBlocks;
    int i;
    struct stat st;
    if (stat(fHandle->fileName, &st) == 0)
    {
        availablePageBlocks = (st.st_size)/PAGE_SIZE;
        /* add one extra page to the file.*/
        availablePageBlocks =(st.st_size)%PAGE_SIZE > 0? availablePageBlocks+1:availablePageBlocks;
        spaceCreationFlag = availablePageBlocks>numberOfPages?0:1;
        if(spaceCreationFlag==1)
        {
            int capacity =(numberOfPages-availablePageBlocks)*PAGE_SIZE;
            for(i=0; i<capacity; i++)
            {
                //c[i]='0';
                fprintf(fHandle->mgmtInfo,"%c",'\0'); //Writing a file as long as the PAGE_SIZE and filling it with 0s
            }
        }
        else
        {
            printf("There is enough capacity already");
        }
        return RC_OK;
    }
    else
    {
        printf("Unable to locate the specified file");
        return RC_FILE_NOT_FOUND;
    }
}

