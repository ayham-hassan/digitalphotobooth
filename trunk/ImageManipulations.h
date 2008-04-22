/*
 * ImageManipulations.h
 * 
 *  Version:
 * 	  	$Id$
 * 
 *  Revisions:
 * 		$Log$
 * 
 * 	 @authors -	David M. Winiarski - dmw1407@rit.edu
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <glib.h>

#define MAX_SIZE 255

#define ERROR_VAL -1
#define OK_VAL 0

gboolean image_resize(char * inImage, char * outImage, char * imageDim, GError *error);
 
/******************************************************************************
 *
 *  Function:       resizeImages
 *  Description:    This function will create a two smaller copies of the
 *					original photo.
 *  Inputs:           
 *  Outputs:         
 *  Routines Called: 
 *
 *****************************************************************************/
int resizeImages(char * inImage, char * smImage, char * lgImage);

/******************************************************************************
 *
 *  Function:       createOilBlobImage
 *  Description:    This function will create an oil blob version of the provided
					image.
 *  Inputs:           
 *  Outputs:         
 *  Routines Called: 
 *
 *****************************************************************************/
int createOilBlobImage(char * inImage, char * oilBlobImage);

/******************************************************************************
 *
 *  Function:       createCharcoalImage
 *  Description:    This function will create a charcoal version of the provided
					image.
 *  Inputs:           
 *  Outputs:         
 *  Routines Called: 
 *
 *****************************************************************************/
int createCharcoalImage(char * inImage, char * charcoalImage);

/******************************************************************************
 *
 *  Function:       createTexturedImage
 *  Description:    This function will create a texturized version of the provided
					image.
 *  Inputs:           
 *  Outputs:         
 *  Routines Called: 
 *
 *****************************************************************************/
int createTexturedImage(char * inImage, char * texImage, char * texturedImage);


