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

  for( i; i < 8; i++ ){
    /* strcmp returns 0 if two strings are equal */
    if( !strcmp( filename, expectedFiles[ i ] ) ){
      counter ++;
    }
  }

  return counter;
}

/* usbDriveName()
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
char *getUSBDriveName(){
  DIR *media;
  struct dirent *ep;
  char *filename = NULL;
  char *fn = 0;
  int counter1 = 0, counter2 = 0;

  printf("getUSBDriveName(): ");

  media = opendir( "/media" );

  /* List the files in /media and check whether each is an expected file */
  if( media != NULL ){
    while( ep = readdir( media ) ){
      if( !isExpectedFilename( ep -> d_name ) ){
        fn = ep -> d_name;
      }
    }
  } else {
    fprintf(stderr, "Couldn't open /media \n");
  }

  /* Append "/media/" to the filename */
  if(fn){
    filename = malloc( (1 + strlen(fn) + strlen("/media")) * sizeof(char) );
    strcpy( filename, "/media/" );
    strcat( filename, fn );
  }

  printf("%s \n", filename);
  
  closedir(media);

  return filename;
}

/* unmountUSBDrive()
 * Unmounts the USB drive. This function uses the getUSBDriveName function to
 *  determine the mountpoint for the USB drive.
 *
 * Returns 0 when the drive has been unmounted. Returns nonzero if there was a
 *  problem.
 */
int unmountUSBDrive(){
  //pid_t mypid;
  //char *driveName = getUSBDriveName();
  //char *arg

  //mypid = fork();

  //if( !mypid ){ /* Child process */

  printf("Drive unmounting not yet implemented. Remove at own risk!\n");

  return 0;
}

/* writeFileToUSBDrive()
 * Writes a file to the currently connected USB drive. This function uses the
 *  getUSBDriveName function to determine the mountpoint for the USB drive.
 *  If a folder named "DigitalPhotoBooth" does not yet exist on the root of the
 *  drive, it will be created and the specified file will be placed in it. If
 *  it does exist, a number will be added to the end of the name, starting at
 *  0 and increasing until an unused one is found.
 *  
 * Parameters:
 *  inFileName: the name of the file (in the current directory) to write
 *  outFileName: the name of the file to write (this will be on the USB)
 *
 * Returns 0 if successful, nonzero if an error occurred.
 */
int writeFileToUSBDrive(char *inFileName, char *outFileName){
  /* The USB drive mount point */
  char *usbDriveName = getUSBDriveName();

  /* Stuff having to do with searching through directories */
  DIR *usbDrive = opendir( usbDriveName );
  struct dirent *ep;
  
  /* Input file (source) and output file (destination) */
  FILE *inFile;
  FILE *outFile;
  
  /* The sub-directory name to create on the USB stick */
  char *dirname = malloc(20 * sizeof(char));
  strcpy(dirname, "DigitalPhotoBooth");

  /* The first number to append (if needed) */
  int counter = 0, retVal = 0;
  
  /* Storage for data bytes being copied */
  unsigned char data;

  /* User:RWX, Group:RWX, Other:RWX */
  mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;

  printf("writeFileToUSBDrive(): ");

  /* Figure out what the final sub-directory name is */
  if( usbDrive != NULL ){
    while( ep = readdir( usbDrive ) ){
      if( !strcmp( ep->d_name, dirname ) ){ /* "if ep->d_name == dirname" */
        char suffix[2] = {( (char)(counter + 0x30) ), '\0'};
        strcat(dirname, suffix);
        counter++;
      } /* if */
    }
  } else {
    fprintf(stderr, "I just took a dump in my underwear. Yippee! \n");
  }

  /* Catenate all the stuff together for the path to the output file */
  strcat(usbDriveName, "/");
  strcat(usbDriveName, dirname);
  strcat(usbDriveName, "/");

  printf("mkdir making %s \n", usbDriveName);

  /* Make the directory on the USB drive */
  retVal = mkdir(usbDriveName, mode);

  /* Catenate the output file name */
  strcat(usbDriveName, outFileName);

  printf("writing %s \n", usbDriveName);

  inFile = fopen(inFileName, "rb");
  outFile = fopen(usbDriveName, "wb");

  if( (inFile != NULL) && (outFile != NULL) ){
    while( !feof(inFile) ){
      data = fgetc(inFile);
      fputc(data, outFile);
    }
  }

  fclose(inFile);
  fclose(outFile);

  return retVal;
}
