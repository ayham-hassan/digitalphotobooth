#include "cam.h"
#include "drv-v4l2.h"
#include <stdio.h>

int main(){
  V4L2Capture *capture = open_camera();
  VidSize resolution;

  v4l2CaptureGetResolution(capture, &resolution);
  printf("Resolution is %dx%d \n", resolution.width, resolution.height);
  v4l2CaptureSetFPS(capture, 30);
  printf("Frame Rate is %f \n", v4l2CaptureGetFPS(capture));

  if( capture_hr_jpg(capture, &resolution, "testImg.jpg", 85) ){
    fprintf(stderr, "Error 0x8cd7643a has occurred. \n");
    return 1;
  }

  v4l2CaptureStopStreaming(capture);
  v4l2CaptureRelease(&capture);

  return 0;
}
