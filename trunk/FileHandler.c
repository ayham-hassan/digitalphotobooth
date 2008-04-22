/******************************************************************************
 * FileHandler.c
 *
 * 	 @authors -	David M. Winiarski - dmw1407@rit.edu
 ******************************************************************************/

#include <unistd.h>
#include "FileHandler.h"

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
int printImage(char * toPrint)
{
	int pid, childStatus, retVal;

	/* Setup the argument strings. */
	char cmd[4] = "lpr";
	char o[3] = "-o";
	char fit[8] = "fitplot";
	
	/* Set the arguments. */	
	char * theArgs[5];
	theArgs[0] = cmd;
	theArgs[1] = o;
	theArgs[2] = fit;
	theArgs[3] = toPrint;
	theArgs[4] = '\0';

	/* Call ImageMagick to do the resizing. */
	pid = fork();
	if(pid < 0)
	{ 
		/* Couldn't fork... ERROR! */
		retVal = -1;
	} 
	else if(pid == 0)
	{ 
		/* This is the child process. */
		execvp(cmd, theArgs);
		exit(1);
	}
	else
	{
		/* We are the parent process... wait for the first cmd. */
		waitpid(pid,&childStatus,0);
		if (WIFEXITED(childStatus) != 0)
		{
			retVal = OK_VAL;	
		}
		else
		{
			retVal = ERROR_VAL;
		}
	}
	
	return retVal;
}

