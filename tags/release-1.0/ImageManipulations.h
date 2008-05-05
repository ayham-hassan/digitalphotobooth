/*
 * ImageManipulations.h
 * 
 * 	 @authors -	David M. Winiarski - dmw1407@rit.edu
 */

#include <unistd.h>
#include <glib.h>
 

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
gboolean image_resize(char * inImage, char * outImage, char * imageDim, GError *error);

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
gboolean create_oil_blob_image(char * inImage, char * outImage, GPid * id, GError *error);

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
gboolean create_charcoal_image(char * inImage, char * outImage, GPid * id, GError *error);

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
gboolean create_textured_image(char * inImage, char * texImage, char * outImage, GPid * id, GError *error);
