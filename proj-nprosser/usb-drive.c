#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INFO_FILE "/home/nprosser/projects/hal-mtab"

int main(){
  FILE *infoFile;
  char *word = malloc(200 * sizeof(char));
  char *subWord = malloc(200 * sizeof(char));
  char *deviceName = malloc(200 * sizeof(char));
  int found = 0;

  infoFile = fopen(INFO_FILE, "r");

  if( infoFile == NULL ){
    fprintf(stderr, "Can't open %s for information on USB drives. \n", 
            INFO_FILE);
    exit(1);
  }

  while( fscanf(infoFile, "%s", word) != EOF ){
    strncpy(subWord, word, 6);
    if( !strncmp( subWord, "/media", 6 ) || found ){
      printf("Found! \n");
      deviceName = strncat( deviceName, word, 25 );
      deviceName = strncat( deviceName, " ", 1 );
      found = 1;
    }
    //printf("%s %s \n", word, subWord);
    printf("Device name: %s \n", deviceName);
  }

  printf("%s \n", word);

  free(word);
  free(subWord);
  free(deviceName);
  fclose(infoFile);

  return 0;
}
