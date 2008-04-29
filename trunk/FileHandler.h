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
 *  Routines Called: g_spawn_async
 *
 *****************************************************************************/
gboolean printImage(char * toPrint, GError *error);
