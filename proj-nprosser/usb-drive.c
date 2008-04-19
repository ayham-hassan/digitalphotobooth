#include "usb-drive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

/* isExpectedFilename
 * Checks to see if a given filename is within the list of expected files in
 *  /media.
 */
int isExpectedFilename( char *filename ){
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
 */
char *usbDriveName(){
  DIR *media;
  struct dirent *ep;
  char *filename = NULL;
  char *fn = 0;
  int counter1 = 0, counter2 = 0;

  media = opendir( "/media" );

  if( media != NULL ){
    while( ep = readdir( media ) ){
      if( !isExpectedFilename( ep -> d_name ) ){
        fn = ep -> d_name;
      }
    }
  } else {
    fprintf(stderr, "Couldn't open /media \n");
  }

  if(fn){
    filename = malloc( (1 + strlen(fn) + strlen("/media")) * sizeof(char) );
    strcpy( filename, "/media/" );
    strcat( filename, fn );
  }

  printf("%s \n", filename);
  
  closedir(media);

  return filename;
}

