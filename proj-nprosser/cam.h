#include "frame.h"
#include "drv-v4l2.h"

#ifndef V4L2_JPG_H
#define V4L2_JPG_H

//Define the image size for high-resolution image capture
#define HR_WIDTH 1600
#define HR_HEIGHT 1200
/* Raw image format supported by QuickCam Pro 9000
 * This code is defined in fourcc.c
 */
#define YUYV 0x56595559
/* Low resolution video should be 640x480 */
#define LR_WIDTH  640
#define LR_HEIGHT 480
#define FPS 20

/* Initializes the camera and returns a V4L2Capture pointer
 */
V4L2Capture *open_camera();

/* Closes the video stream and releases resources
 *  capture - A pointer to the Video4Linux capture object
 */
void close_camera(V4L2Capture *capture);

/* Capture a single frame from the video stream. This frame is in RGB24 format.
 *  capture - A pointer to the Video4Linux capture object
 *  @return a VidFrame object with data in RGB24 format
 */
VidFrame *getFrame(V4L2Capture *capture);

/* Write a Video4Linux2 frame to a JPEG image.
 *  frame - A pointer to the Video4Linux2 frame struct
 *  filename - C string specifying filename to save to
 *  quality - integer in the range [0, 100] specifying JPEG quality parameter
 *  @return 0 if the process was successful, nonzero if unsuccessful
 */
int write_jpg(VidFrame *frame, char *fileName, int quality);

/* Using a Video4Linux2 capture object, write a high-resolution JPEG image.
 * Image size is defined in cam.h, HR_WIDTH and HR_HEIGHT
 *  capture - A pointer to the Video4Linux capture object
 *  lowRes - VidSize object holding the original video size
 *  filename - C string specifying filename to save to
 *  quality - integer in the range [0, 100] specifying JPEG quality parameter
 *  @return 0 if the process was successful, nonzero otherwise
 */
int capture_hr_jpg(V4L2Capture *capture, VidSize *lowRes, char *fileName, 
                   int quality);

#endif
