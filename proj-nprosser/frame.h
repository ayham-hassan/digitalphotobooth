#ifndef FRAME_H_
#define FRAME_H_

#include <sys/time.h>
#include "fourcc.h"

#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

/// Screen Size

typedef struct {
  /// The width 
  int width;
  /// The height
  int height;
} VidSize;

/// Video Frame
typedef struct {
  /// A frame may be named.
  char *name;
	
  /// A read only frame is used togather with mmap operation. 
  int readonly;
	
  /// The size of frame
  VidSize size;	

  /// The image type(fourcc) 
  fourcc_t format;
	
  /// Row stride
  int bytesperline;
	
  /// Excepted image size of current format. 
  int imagesize;
	
  /// The size of data/buffer allocated to hold image. It should be larger than or equal to imagesize
  int buflen; 
  
  /// Pointer to image data.
  unsigned char *data;
	
  /// the last modified time.
  struct timeval	timestamp;
	
  /// refcount
  int refcount;
} VidFrame;

/// Allocate and initialize a V4L2Frame structure
VidFrame *vidFrameCreate();

/// Increases the reference count of frame
void vidFrameRef(VidFrame *frame);

/// Decreases the reference count of frame. When its reference count drops to 0, the frame is finalized
void vidFrameUnref(VidFrame **frame);

/// Release a V4L2Frame.
void vidFrameRelease(VidFrame **frame);

/// Declare to modify the content of frame. The original data should be preserved.
void vidFrameModify(VidFrame **frame);

/// Declare to use the frame as a cache. The original data do not preserved.
void vidFrameCache(VidFrame **frame);

/// Change the frame format and allocate image buffer.
int vidFrameChangeFormat(VidFrame *frame,fourcc_t format,int width,int height);

/// Get the width of a V4L2Frame
int vidFrameGetWidth(VidFrame *frame);

/// Get the height of a V4L2Frame
int vidFrameGetHeight(VidFrame *frame) ;

/// Get the image Length of the frame
int vidFrameGetImageLength(VidFrame *frame);

/// Get the buffer length of the frame (should be larger or equal to Image Length)
int vidFrameGetBufferLength(VidFrame *frame);

/// Get the pointer to image data
unsigned char* vidFrameGetImageData(VidFrame *frame);

/// Get image format
fourcc_t vidFrameGetFormat(VidFrame *frame);

/// Resize the buffer length of image data. 
int vidFrameResizeBuffer(VidFrame *frame,int length);

/// Get the Row stride of the frame. Remark: It could be zero for some kind of image format.
int vidFrameGetRowStride(VidFrame *frame);

void vidFrameCopy(VidFrame *src,VidFrame *dest,int deep);

/// Clone a frame by create a deep copy of the object.  
VidFrame* vidFrameClone(VidFrame *frame);

/* Image Format Convertor */

typedef int (*v4l2ConvFunc) (VidFrame *src,VidFrame *dest);

/// Image format converter 

typedef struct {
  /// Name of the converter
  char *name;
  /// The input image format(fourcc)
  fourcc_t input;
  /// The output image format(fourcc))
  fourcc_t output;
	
  /// The callback function to handle the convertion
  v4l2ConvFunc convert;	
} VidConv;

/// Find a converter to convert image format from input to output

VidConv* vidConvFind(fourcc_t input,fourcc_t output);

/// Execute the image converter.

int vidConvProcess(VidConv *conv,VidFrame *src,VidFrame *dest);

#ifdef __cplusplus
} /* extern "C" */
#endif /* defined(__cplusplus) */


#endif /*FRAME_H_*/
