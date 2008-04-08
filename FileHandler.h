/******************************************************************************
 * FileHandler.c
 * 
 * 	 @authors -	David M. Winiarski - dmw1407@rit.edu
 ******************************************************************************/
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_SIZE 255

#define ERROR_VAL -1
#define OK_VAL 0

extern int errno;
/******************************************************************************
 *
 *  Function:       printImage
 *  Description:    This function will print the image provided. 
 *  Inputs:         toPrint - String of image to be printed.  
 *  Outputs:       	retVal - Returns -1 if the image was not printed succesfully.
							 Returns 0 if the image was printed successfully.  
 *  Routines Called: execvp, waitpid
 *
 *****************************************************************************/
int printImage(char * toPrint);
