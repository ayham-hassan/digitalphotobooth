#include <stdlib.h>
#include <linux/videodev.h>
#include <stdio.h>
#include "frame.h"
#include "drv-v4l2.h"
#include "cam.h"
#include "jpeglib.h"


/* Initializes the camera and returns a V4L2Capture pointer
 */
V4L2Capture *open_camera(){
  V4L2Capture *capture = v4l2CaptureOpen("/dev/video0");
  VidSize _resolution;
  _resolution.width = LR_WIDTH;
  _resolution.height = LR_HEIGHT;
  v4l2CaptureSetImageFormat(capture, (fourcc_t)YUYV, &_resolution);
  v4l2CaptureSetFPS(capture, FPS);
  //v4l2CaptureStartStreaming(capture,0,4);

  return capture;
}

/* Closes the video stream and releases resources
 *  capture - A pointer to the Video4Linux capture object
 */
void close_camera(V4L2Capture *capture){
  v4l2CaptureStopStreaming(capture);
  v4l2CaptureRelease(&capture);
}

/* Capture a single frame from the video stream. This frame is in RGB24 format.
 *  capture - A pointer to the Video4Linux capture object
 *  @return a VidFrame object with data in RGB24 format
 */
VidFrame *getFrame(V4L2Capture *capture){
  //capture frame
  VidFrame *myFrame = v4l2CaptureQueryFrame(capture);
  
  //Convert the frame to RGB:
  //Find the input format
  fourcc_t inputFormat = vidFrameGetFormat(myFrame);
  
  //output format (24-bit RGB)
  fourcc_t outputFormat = V4L2_PIX_FMT_RGB24;
  
  //converter object
  VidConv *converter = vidConvFind(inputFormat, outputFormat);
  
  //new rgb frame
  VidFrame *rgbFrame = vidFrameCreate();
  
  //do conversion
  if( !converter ){
    fprintf(stderr, "Couldn't find a valid converter.\n");
    exit(1);
  } else {
    if( vidConvProcess(converter, myFrame, rgbFrame) ){
      fprintf(stderr, "Error while converting frame format.\n");
      exit(1);
    }
  }

  return rgbFrame;
}

/* Write a Video4Linux2 frame to a JPEG image.
 *  frame - A pointer to the Video4Linux2 frame struct
 *  filename - C string specifying filename to save to
 *  quality - integer in the range [0, 100] specifying JPEG quality parameter
 *  @return 0 if the process was successful, nonzero if unsuccessful
 */
int write_jpg(VidFrame *frame, char *filename, int quality){
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;

  FILE *outFile;
  JSAMPROW row_pointer[1];
  int rowStride;
  JSAMPLE *imageData;

  //First we need to convert the frame to RGB
  //Find the input format
  fourcc_t inputFormat = vidFrameGetFormat(frame);
  //output format (24-bit RGB)
  fourcc_t outputFormat = V4L2_PIX_FMT_RGB24;
  VidConv *converter;
  VidFrame *rgbFrame;
  
  if( inputFormat != outputFormat ){
    //converter object
    converter = vidConvFind(inputFormat, outputFormat);
    //new rgb frame
    rgbFrame = vidFrameCreate();
    //do conversion
    if( !converter ){
      fprintf(stderr, "Couldn't find a valid converter.\n");
      exit(1);
    } else {
      if( vidConvProcess(converter, frame, rgbFrame) ){
        fprintf(stderr, "Error while converting frame format.\n");
        exit(1);
      }
    }
  } else { //input format is already rgb24
    rgbFrame = frame;
  }

  imageData = (JSAMPLE *) vidFrameGetImageData(rgbFrame);

  cinfo.err = jpeg_std_error(&jerr);
    
  jpeg_create_compress(&cinfo);

  if( (outFile = fopen(filename, "wb")) == NULL ){
    fprintf(stderr, "Can't create file %s. \n", filename);
    return(1);
  }
  jpeg_stdio_dest(&cinfo, outFile);

  cinfo.image_width = rgbFrame->size.width;
  cinfo.image_height = rgbFrame->size.height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 90, TRUE);
  jpeg_start_compress(&cinfo, TRUE);

  rowStride = vidFrameGetRowStride(rgbFrame);
  while(cinfo.next_scanline < cinfo.image_height){
    row_pointer[0] = &imageData[cinfo.next_scanline * rowStride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);
  fclose(outFile);
  jpeg_destroy_compress(&cinfo);

  return 0;

}


/* Using a Video4Linux2 capture object, write a high-resolution JPEG image.
 * Image size is defined in cam.h, HR_WIDTH and HR_HEIGHT
 *  capture - A pointer to the Video4Linux capture object. This object should
 *            not currently be in a streaming state.
 *  filename - C string specifying filename to save to
 *  quality - integer in the range [0, 100] specifying JPEG quality parameter
 *  @return 0 if the process was successful, nonzero otherwise
 */
int capture_hr_jpg(V4L2Capture *capture, VidSize *lowRes, char *fileName, 
                   int quality){
  int retVal = 0, counter = 0;
  VidSize highResolution;

  highResolution.width = HR_WIDTH;
  highResolution.height = HR_HEIGHT;
  v4l2CaptureSetResolution(capture, &highResolution);

  v4l2CaptureStartStreaming(capture, 0, 4);

  //It takes a few frames for the camera to equalize color and brightness
  VidFrame *highFrame = v4l2CaptureQueryFrame(capture);
  for(counter = 0; counter < 1; counter++){
    highFrame = v4l2CaptureQueryFrame(capture);
  }

  //Using jpeglib
  retVal = write_jpg(highFrame, fileName, 85);

  v4l2CaptureStopStreaming(capture);
  v4l2CaptureSetImageFormat(capture, (fourcc_t)YUYV, lowRes);

  return retVal;
}
