/*
 * ImageManipulations.c
 * 
 * 	 @authors -	David M. Winiarski - dmw1407@rit.edu
 */

#include "ImageManipulations.h"

char cnvCmd[8] = "convert";

/******************************************************************************
 *
 *  Function:       resizeImages
 *  Description:    This function will create a two smaller copies of the
 *					original photo.
 *  Inputs:         inImage - the image to resize
 *                  outImage - the resized image
 *                  imageDim - the image dimensions 
 *                  error - place to store error information
 *  Outputs:        TRUE on success, FALSE if error is set.
 *  Routines Called: g_spawn_sync
 *
 *****************************************************************************/
gboolean image_resize(char * inImage, char * outImage, char * imageDim, GError *error)
{
	/* Setup argument strings. */
	char resize[8] = "-resize";	
	/* Setup the arguments for the commands. */
	char * args[6];
	args[0] = cnvCmd;
	args[1] = inImage;
	args[2] = resize;
	args[3] = imageDim;
	args[4] = outImage;
	args[5] = '\0';

    return g_spawn_sync (NULL, args, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL, NULL, &error);
}
  /******************************************************************************
 *
 *  Function:       create_oil_blob_image
 *  Description:    This function will create an oil blob version of the provided
 *					image.
 *  Inputs:         inImage - the image to paint, 
 *                  outImage - the oil painted image
 *                  id - the ID of the spawned process
 *                  error - place to store error information
 *  Outputs:        TRUE on success, FALSE if error is set.
 *  Routines Called: g_spawn_async
 *
 *****************************************************************************/
gboolean create_oil_blob_image(char * inImage, char * outImage, GPid * id, GError *error)
{
	/* Setup argument strings. */
	char paint[7] = "-paint";
	char oilAmt[2] = "2";	

	/* Setup the arguments for the commands. */	
	char * args[6];
	args[0] = cnvCmd;
	args[1] = inImage;
	args[2] = paint;
	args[3] = oilAmt;
	args[4] = outImage;
	args[5] = '\0';

    /* Spawn a new process, to be run asynchronously. */
	return g_spawn_async ( NULL, args, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, id, &error);
}

/******************************************************************************
 *
 *  Function:       create_charcoal_image
 *  Description:    This function will create a charcoal version of the provided
 *					image.
 *  Inputs:         inImage - the image to paint, 
 *                  outImage - the oil painted image
 *                  id - the ID of the spawned process
 *                  error - place to store error information
 *  Outputs:        TRUE on success, FALSE if error is set.
 *  Routines Called: g_spawn_async
 *
 *****************************************************************************/
gboolean create_charcoal_image(char * inImage, char * outImage, GPid * id, GError *error)
{
	/* Setup local vars. */
	char charcoal[10] = "-charcoal";
	char cclAmt[2] = "4";
	
	/* Setup the arguments for the commands. */	
	char * args[6];
	args[0] = cnvCmd;
	args[1] = inImage;
	args[2] = charcoal;
	args[3] = cclAmt;
	args[4] = outImage;
	args[5] = '\0';

    /* Spawn a new process, to be run asynchronously. */
	return g_spawn_async ( NULL, args, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, id, &error);
}

/******************************************************************************
 *
 *  Function:       create_textured_image
 *  Description:    This function will create a texturized version of the provided
 *					image.
 *  Inputs:         inImage - the image to paint, 
 *                  outImage - the oil painted image
 *                  texImage - the texture to apply to the image
 *                  id - the ID of the spawned process
 *                  error - place to store error information  
 *  Outputs:        TRUE on success, FALSE if error is set.
 *  Routines Called: g_spawn_async
 *
 *****************************************************************************/
gboolean create_textured_image(char * inImage, char * texImage, char * outImage, GPid * id, GError *error)
{	/* Setup local vars. */
	char cmpCmd[10] = "composite";
	char tile[6] = "-tile";
	char compose[9] = "-compose";
	char hardlight[10] = "Hardlight";
	
	/* Setup the arguments for the commands. */	
	char * args[8];	args[0] = cmpCmd;
	args[1] = texImage;
	args[2] = inImage;
	args[3] = tile;
	args[4] = compose;
	args[5] = hardlight;
	args[6] = outImage;
	args[7] = '\0';

    /* Spawn a new process, to be run asynchronously. */
	return g_spawn_async ( NULL, args, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, id, &error);	
}


