#include "cam.h"
#include "drv-v4l2.h"
#include <stdio.h>

/* Raw image format supported by QuickCam Pro 9000
 * This code is defined in fourcc.c
 */
#define YUYV 0x56595559
/* Low resolution video should be 640x480 */
#define LR_WIDTH  640
#define LR_HEIGHT 480

int main(){
  V4L2Capture *capture = v4l2CaptureOpen("/dev/video0");
  VidSize resolution, _resolution;
  _resolution.width = LR_WIDTH;
  _resolution.height = LR_HEIGHT;
  v4l2CaptureSetImageFormat(capture, (fourcc_t)YUYV, &_resolution);

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
