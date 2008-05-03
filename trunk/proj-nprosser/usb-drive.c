/*
 * usb-drive.c
 *
 * Author: Nathan Prosser  ntp0166@rit.edu
 *
 */

#include "usb-drive.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>


/* This is a list of files that are expected to be in /media */
static char *expectedFiles[] = { ".", "..", "floppy0", "floppy", "cdrom0", 
                                 "cdrom", ".hal-mtab", ".hal-mtab-lock" };

/* isExpectedFilename
 * Checks to see if a given filename is within the list of expected files in
 *  /media.
 *
 * Static function is only available to other functions within this file.
 */
static int isExpectedFilename( char *filename ){
  int i = 0;
  int counter = 0;

  for( i = 0; i < 8; i++ ){
    /* strcmp returns 0 if two strings are equal */
    if( !strcmp( filename, expectedFiles[ i ] ) ){
      counter ++;
    }
  }

  return counter;
}

/* toUpperCase()
 * Convert a null-terminated C string to all uppercase letters.
 *
 * Static function is only available to other functions within this file.
 */
static char *toUpperCase( char *string ){
  int i = 0;
  char *output = string;

  while( string[i] != '\0' ){
    if( (string[i] >= 'a') && (string[i] <= 'z') ){
      output[i] = string[i] - 32;
    } else {
      output[i] = string[i];
    }
    i++;
  }
      
  return output;
}

/* reverse:  reverse string s in place 
 *
 * Credit: Kernighan, Ritchie, "The C Programming Language", p. 58
 */
static void reverse(char s[]){
  int c, i, j;

  for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}

/* itoa:  convert n to characters in s
 *
 * Credit: Kernighan, Ritchie, "The C Programming Language", p. 60
 */
static void itoa(int n, char s[]){
  int i, sign;

  if ((sign = n) < 0)  /* record sign */
    n = -n;          /* make n positive */
  i = 0;
  do {       /* generate digits in reverse order */
    s[i++] = n % 10 + '0';   /* get next digit */
  } while ((n /= 10) > 0);     /* delete it */
  if (sign < 0)
    s[i++] = '-';
  s[i] = '\0';
  reverse(s);
} 


/* incrementFileName()
 * Expects a filename in the form IMG####.JPG Increments the number portion
 *  and returns the result.
 *
 * Static function is only available to other functions within this file.
 */
static void incrementFileName( char *inFileName, char *outFileName ){
  int num, i;
  char number[4];

  /* Extract the number portion */
  for(i = 0; i < 4; i++){
    number[i] = inFileName[i + 3];
  }

  /* Convert to integer and increment */
  num = atoi(number);
  num++;

  if( num < 10000 ){
    /* Put together the output filename */
    memcpy( outFileName, "\0\0\0\0\0\0\0\0\0\0\0\0", 12 );
    strncpy( outFileName, "IMG", 3 );
    itoa( num, number );
    strncat( outFileName, "000", (4 - strlen(number)) );
    strncat( outFileName, number, strlen(number) );
    strncat( outFileName, ".JPG", 4);
  } else {
    strncpy( outFileName, "", 1 );
  }

}

/* getUSBDriveName()
 * Finds the absolute path of a USB flash drive. This function makes several
 *  assumptions. First is that it looks in /media, which is where Ubuntu
 *  automounts flash drives. Second is that it assumes only one external drive
 *  will be inserted at a time. Third, it uses a list of hard-coded devices
 *  which are expected to be in /media, and are not flash drives. It does not
 *  matter if any of these are not present, but if additional ones are present
 *  (eg a CD-ROM), that may be detected instead of the flash drive.
 *
 * If a usb device is not found, a null pointer is returned.
 */
void getUSBDriveName( char *fileName ){
  DIR *media;
  struct dirent *ep;
  char *fn = 0;

  printf("getUSBDriveName(): ");

  media = opendir( "/media" );

  /* List the files in /media and check whether each is an expected file */
  if( media != NULL ){
    while( (ep = readdir( media )) ){
      if( !isExpectedFilename( ep -> d_name ) ){
        fn = ep -> d_name;
      }
    }
  } else {
    fprintf(stderr, "Couldn't open /media \n");
  }

  /* Append "/media/" to the filename */
  if(fn){
    strcpy( fileName, "/media/" );
    strcat( fileName, fn );
  }

  printf("%s \n", fileName);
  
  closedir(media);
}

/* unmountUSBDrive()
 * Unmounts the USB drive. This function uses the getUSBDriveName function to
 *  determine the mountpoint for the USB drive.
 *
 * Returns 0 when the drive has been unmounted. Returns nonzero if there was a
 *  problem.
 */
int unmountUSBDrive(){
  /*   pid_t mypid; */
  /*   char *driveName = getUSBDriveName(); */
  /*   char *arg */

  /*   mypid = fork(); */

  /*   if( !mypid ){ /\* Child process *\/ */

  printf("Drive unmounting not yet implemented. Remove at own risk!\n");

  return 0;
}

/* writeFileToUSBDrive()
 * Writes a file to the currently connected USB drive. This function uses the
 *  getUSBDriveName function to determine the mountpoint for the USB drive.
 *  If a folder named "DigitalPhotoBooth" does not yet exist on the root of the
 *  drive, it will be created and the specified file will be placed in it. If
 *  it does exist, a check will be performed to see whether the default output
 *  filename (IMG0001.JPG) is already present. The number at the end of the
 *  filename will be incremented until it does not match an already existing
 *  filename.
 *  
 * Parameters:
 *  fileName: the name of the file (current directory or path) to write
 *
 * Returns 0 if successful, nonzero if an error occurred.
 */
int writeFileToUSBDrive(char *fileName){
  /* The USB drive mount point */
  char usbDriveName[ 100 ] = "";
  getUSBDriveName( usbDriveName );

  /* Stuff having to do with searching through directories */
  DIR *usbDrive;
  struct dirent *ep;
  
  /* Input file (source) and output file (destination) */
  FILE *inFile;
  FILE *outFile;
  /* Output file name */
  char outFileName[ 12 ] = "IMG0001.JPG";
  char outFileName2[ 12 ] = "";
  
  /* The sub-directory name to create on the USB stick */
  char *dirname = malloc(20 * sizeof(char));
  strcpy(dirname, "DigitalPhotoBooth");
  int dirExists = 0;

  int retVal = 0;
  
  /* Storage for data bytes being copied */
  unsigned char data;

  /* User:RWX, Group:RWX, Other:RWX */
  mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;

  printf("writeFileToUSBDrive(): ");

  if( usbDriveName != NULL ){ /* USB drive not detected by getUSBDriveName */

    /* Open the USB drive for listing */
    usbDrive = opendir( usbDriveName );

    /* Figure out what the final sub-directory name is */
    if( usbDrive != NULL ){ /* USB drive could not be opened */
      while( (ep = readdir( usbDrive )) ){
        if( !strcmp( ep->d_name, dirname ) ){ /* "if ep->d_name == dirname" */
          dirExists ++;
        } /* if */
      }

      /* Close the directory listing */
      closedir( usbDrive );
    
      /* Catenate all the stuff together for the path to the output file */
      strcat(usbDriveName, "/");
      strcat(usbDriveName, dirname);
      strcat(usbDriveName, "/");

      if( !dirExists ){
        printf("mkdir making %s \n", usbDriveName);

        /* Make the directory on the USB drive */
        retVal = mkdir(usbDriveName, mode);
      } else {
        printf("Directory already exists. \n");
      
        /* Check whether the default filename exists and adjust as necessary */
        usbDrive = opendir( usbDriveName );
        if( usbDrive != NULL ){
          while( (ep = readdir( usbDrive )) ){
            if( !strcmp( toUpperCase( ep->d_name ), outFileName ) ){
              incrementFileName( outFileName, outFileName2 );
              strncpy( outFileName, outFileName2, 11 );
              rewinddir( usbDrive );
            }
          }
        }
        retVal = 0;
      }

      /* Catenate the output file name */
      strcat(usbDriveName, outFileName);

      printf("writing %s \n", usbDriveName);

      inFile = fopen(fileName, "rb");
      outFile = fopen(usbDriveName, "wb");

      /* Copy the input file to the output file */
      int errFlag = 0;
      if( (inFile != NULL) && (outFile != NULL) ){
        errFlag |= ferror(inFile);
        errFlag |= ferror(outFile);
        while( !feof(inFile) && !errFlag ){
          errFlag = 0; /* This should be fresh every time */
          data = fgetc(inFile);
          errFlag = ferror(inFile);
          if( !errFlag ){
            fputc(data, outFile); 
            errFlag |= ferror(outFile);
          }
        }
      }

      fclose(inFile);
      fclose(outFile);


    } else {
      retVal = -1;
      fprintf(stderr, "Error: USB drive could not be read. \n");
    } 

    
  } /* end of chech for whether usb drive was found */
  else {
    retVal = -1;
    fprintf(stderr, "Error: USB drive not found. \n");
  }

  return retVal;
}
