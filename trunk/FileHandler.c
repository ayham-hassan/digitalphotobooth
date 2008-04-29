/******************************************************************************
 * FileHandler.c
 *
 * 	 @authors -	David M. Winiarski - dmw1407@rit.edu
 ******************************************************************************/

#include "FileHandler.h"

 /******************************************************************************
 *
 *  Function:       printImage
 *  Description:    This function will print the image provided. 
 *  Inputs:         toPrint - String of image to be printed
 *                  id - the ID of the spawned process
 *                  error - place to store error information 
 *  Outputs:       	TRUE on success, FALSE if error is set.
 *  Routines Called: g_spawn_async
 *
 *****************************************************************************/
gboolean printImage(char * toPrint, GError *error)
{
	/* Setup the argument strings. */
	char cmd[4] = "lpr";
	char o[3] = "-o";
	char fit[8] = "fitplot";
	
	/* Set the arguments. */	
	char * args[5];
	args[0] = cmd;
	args[1] = o;
	args[2] = fit;
	args[3] = toPrint;
	args[4] = '\0';

    /* Spawn a new process, to be run asynchronously. */
	return g_spawn_sync (NULL, args, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL, NULL, &error);	
}

