#include "frame.h"
#include "drv-v4l2.h"

#ifndef V4L2_JPG_H
#define V4L2_JPG_H

//Define the image size for high-resolution image capture
#define HR_WIDTH 1600
#define HR_HEIGHT 1200

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