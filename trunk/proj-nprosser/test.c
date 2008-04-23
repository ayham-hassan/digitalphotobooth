#include "cam.h"
#include "drv-v4l2.h"
#include <stdio.h>
#include <time.h>

int main(){
  V4L2Capture *capture = open_camera();
  VidSize resolution;
  VidFrame *frame;
  clock_t startTime, stopTime;
  char fname[] = { 't', 'e', 's', 't', '0', '.', 'j', 'p', 'g', '\0'};

  v4l2CaptureGetResolution(capture, &resolution);
  printf("Resolution is %dx%d \n", resolution.width, resolution.height);
  //v4l2CaptureSetFPS(capture, 30);
  printf("Frame Rate is %f \n", v4l2CaptureGetFPS(capture));

  v4l2CaptureStartStreaming(capture, 0, 4);

  printf("Start streaming... \n");

  int i = 0;
  for(i; i<100; i++){
    frame = getFrame(capture);
  }

  printf("Stop streaming... \n");
  startTime = clock();

  printf("Capture image... \n");
  if( capture_hr_jpg(capture, /*&resolution,*/ "testImg.jpg", 85) ){
    fprintf(stderr, "Error 0x8cd7643a has occurred. \n");
    return 1;
  }
  stopTime = clock();
  printf("Done... \n");
  printf("Time = %f sec\n", ((double)(stopTime - startTime))/CLOCKS_PER_SEC);

  for(i = 0; i<10; i++){
    frame = getFrame(capture);
    fname[4] = i + 0x30;
    write_jpg(frame, fname, 85);
  }
    

  close_camera(capture);

  return 0;
}
