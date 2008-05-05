/******************************************************************************
 * FileHandler.c
 * 
 * 	 @authors -	David M. Winiarski - dmw1407@rit.edu
 ******************************************************************************/
#include <unistd.h>
#include <glib.h>

/******************************************************************************
 *
 *  Function:       printImage
 *  Description:    This function will print the image provided. 
 *  Inputs:         toPrint - String of image to be printed
 *                  id - the ID of the spawned process
 *                  error - place to store error information 
 *  Outputs:       	TRUE on success, FALSE if error is set.
 *  Routines Called: g_spawn_sync
 *
 *****************************************************************************/
gboolean printImage(char * toPrint, GError *error);

 /******************************************************************************
 *
 *  Function:       fs_sync
 *  Description:    This function will call the 'sync' unix command.  This 
 *                  will cause the file system to sync pending writes.
 *  Outputs:       	TRUE on success, FALSE if error is set.
 *  Routines Called: g_spawn_sync
 *
 *****************************************************************************/
gboolean fs_sync(GError *error);
