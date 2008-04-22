/*
 * ImageManipulations.c
 * 
 *  Version:
 * 	  	$Id$
 * 
 *  Revisions:
 * 		$Log$
 * 
 * 	 @authors -	David M. Winiarski - dmw1407@rit.edu
 */

#include <glib.h>
#include "ImageManipulations.h"

extern int errno;
char cnvCmd[8] = {'c', 'o', 'n', 'v', 'e', 'r', 't', '\0'};

  /******************************************************************************
 *
 *  Function:         GetImageName
 *  Description:    This function will parse the provided .jpg image name, and 
 * 					find the root of the file name.
 *  Inputs:           
 *  Outputs:         
 *  Routines Called: 
 *
 *****************************************************************************/
 void getImageName(char * in, char * name)
 {
	int i, len, temp;
	/* Find the period in the sentence. */
	len = strlen(in);
	for (i = 0; i < len; i++)
	{
		if(in[i] == '.')
		{
			temp = i;
			break;
		}
	}	
	strncpy(name, in, temp);
 }
 
 
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
gboolean image_resize(char * inImage, char * outImage, char * imageDim, GPid *child_pid, GError *error)
{
	/* Setup local vars. */
	char resize[8] = "-resize";
	
	char * args[6];
	
	/* Setup the arguments for the commands. */
	args[0] = cnvCmd;
	args[1] = inImage;
	args[2] = resize;
	args[3] = imageDim;
	args[4] = outImage;
	args[5] = '\0';

    return g_spawn_async (NULL, args, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, child_pid, &error);
}
 
 
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
int resizeImages(char * inImage, char * smImage, char * lgImage)
 {
 
	/* Setup local vars. */
	int pid1, pid2, childStatus, retVal;
	char root[MAX_SIZE];
	char smSuffix[8] = { '_', 's', 'm', '.', 'j', 'p', 'g', '\0'};
	char lgSuffix[8] = { '_', 'l', 'g', '.', 'j', 'p', 'g', '\0'};
	
	char resize[8] = {'-', 'r', 'e', 's', 'i', 'z', 'e', '\0'};
	
	char smDim[8] = { '6', '4', '0', 'x', '4', '8', '0', '\0'};
	char lgDim[8] = { '8', '0', '0', 'x', '6', '0', '0', '\0'};
	
	char * smArgs[6];
	char * lgArgs[6];
	
	/* Clear memory and get the root of the image file name. */
	memset(root, '\0', MAX_SIZE);
	getImageName(inImage, root);
	
	
	/* Concatenate the new image name strings. */
	strcpy(smImage, root);
	strcpy(lgImage, root);
	strcat(smImage, smSuffix);
	strcat(lgImage, lgSuffix);

	/* Setup the arguments for the commands. */
	smArgs[0] = cnvCmd;
	lgArgs[0] = cnvCmd;
	smArgs[1] = inImage;
	lgArgs[1] = inImage;
	smArgs[2] = resize;
	lgArgs[2] = resize;
	smArgs[3] = smDim;
	lgArgs[3] = lgDim;
	smArgs[4] = smImage;
	lgArgs[4] = lgImage;	
	smArgs[5] = '\0';
	lgArgs[5] = '\0';

	/* Call ImageMagick to do the resizing. */
	pid1 = fork();
	if(pid1 < 0)
	{ 
		/* Couldn't fork... ERROR! */
		retVal = -1;
	} 
	else if(pid1 == 0)
	{ 
		/* This is the child process. */
		execvp(cnvCmd, smArgs);
		exit(1);
	}
	else
	{
		/* We are the parent process... wait for the first cmd. */
		waitpid(pid1,&childStatus,0);
		if (WIFEXITED(childStatus) != 0)
		{
			pid2 = fork();	
			if(pid2 < 0)
			{ 
				/* Couldn't fork... ERROR! */
				retVal = ERROR_VAL;
			} 
			else if(pid2 == 0)
			{ 
				/* This is the child process. */
				execvp(cnvCmd, lgArgs);
				exit(1);
			}
			else
			{
				waitpid(pid2,&childStatus,0);
				if (WIFEXITED(childStatus) != 0)
				{
					retVal = OK_VAL;
				}
				else
				{
					retVal = ERROR_VAL;
				}
			}
		}
		else
		{
			/* Errored... */
			retVal = ERROR_VAL;
		}
	}
	
	return retVal;	
 }

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
int createOilBlobImage(char * inImage, char * oilBlobImage)
{
	/* Setup local vars. */
	int pid1, childStatus, retVal;
	char root[MAX_SIZE];
	char oilSuffix[9] = { '_', 'o', 'i', 'l', '.', 'j', 'p', 'g', '\0'};	
	char paint[7] = {'-', 'p', 'a', 'i', 'n', 't', '\0'};
	char oilAmt[2] = { '2', '\0'};
	
	char * theArgs[6];	
	/* Clear memory and get the root of the image file name. */
	memset(root, '\0', MAX_SIZE);
	getImageName(inImage, root);

	/* Setup the output file string, and the command arguments. */
	strcpy(oilBlobImage, root);
	strcat(oilBlobImage, oilSuffix);

	theArgs[0] = cnvCmd;
	theArgs[1] = inImage;
	theArgs[2] = paint;
	theArgs[3] = oilAmt;
	theArgs[4] = oilBlobImage;
	theArgs[5] = '\0';

	/* Call ImageMagick to do the drawing. */
	pid1 = fork();
	if(pid1 < 0)
	{ 
		/* Couldn't fork... ERROR! */
		retVal = -1;
	} 
	else if(pid1 == 0)
	{ 
		/* This is the child process. */
		execvp(cnvCmd, theArgs);
		exit(1);
	}
	else
	{
		/* This is the parent process.. wait for child to finish. */
		waitpid(pid1,&childStatus,0);
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
int createCharcoalImage(char * inImage, char * charcoalImage)
{
	/* Setup local vars. */
	int pid1, childStatus, retVal;
	char root[MAX_SIZE];
	char cclSuffix[9] = { '_', 'c', 'c', 'l', '.', 'j', 'p', 'g', '\0'};	
	char charcoal[10] = {'-', 'c', 'h', 'a', 'r', 'c', 'o', 'a', 'l','\0'};
	char cclAmt[2] = { '4', '\0'};
	
	char * theArgs[6];	
	/* Clear memory and get the root of the image file name. */
	memset(root, '\0', MAX_SIZE);
	getImageName(inImage, root);

	/* Setup the output file string, and the command arguments. */
	strcpy(charcoalImage, root);
	strcat(charcoalImage, cclSuffix);

	theArgs[0] = cnvCmd;
	theArgs[1] = inImage;
	theArgs[2] = charcoal;
	theArgs[3] = cclAmt;
	theArgs[4] = charcoalImage;
	theArgs[5] = '\0';

	/* Call ImageMagick to do the drawing. */
	pid1 = fork();
	if(pid1 < 0)
	{ 
		/* Couldn't fork... ERROR! */
		retVal = -1;
	} 
	else if(pid1 == 0)
	{ 
		/* This is the child process. */
		execvp(cnvCmd, theArgs);
		exit(1);
	}
	else
	{
		/* This is the parent process.. wait for child to finish. */
		waitpid(pid1,&childStatus,0);
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
int createTexturedImage(char * inImage, char * texImage, char * texturedImage)
{
	/* Setup local vars. */
	int pid1, childStatus, retVal;
	char root[MAX_SIZE];
	char texSuffix[9] = { '_', 't', 'e', 'x', '.', 'j', 'p', 'g', '\0'};	
	char cmpCmd[10] = { 'c', 'o', 'm', 'p', 'o', 's', 'i', 't', 'e', '\0' };
	char tile[6] = {'-', 't', 'i', 'l', 'e','\0'};
	char compose[9] = {'-', 'c', 'o', 'm', 'p', 'o', 's', 'e', '\0' };
	char hardlight[10] = { 'H', 'a', 'r', 'd', 'l', 'i', 'g', 'h', 't', '\0'};
	
	char * theArgs[8];	
	/* Clear memory and get the root of the image file name. */
	memset(root, '\0', MAX_SIZE);
	getImageName(inImage, root);

	/* Setup the output file string, and the command arguments. */
	strcpy(texturedImage, root);
	strcat(texturedImage, texSuffix);

	theArgs[0] = cmpCmd;
	theArgs[1] = texImage;
	theArgs[2] = inImage;
	theArgs[3] = tile;
	theArgs[4] = compose;
	theArgs[5] = hardlight;
	theArgs[6] = texturedImage;
	theArgs[7] = '\0';

	/* Call ImageMagick to do the drawing. */
	pid1 = fork();
	if(pid1 < 0)
	{ 
		/* Couldn't fork... ERROR! */
		retVal = -1;
	} 
	else if(pid1 == 0)
	{ 
		/* This is the child process. */
		execvp(cmpCmd, theArgs);
		exit(1);
	}
	else
	{
		/* This is the parent process.. wait for child to finish. */
		waitpid(pid1,&childStatus,0);
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


