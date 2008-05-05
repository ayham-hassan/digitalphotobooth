#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/ioctl.h>
#include <linux/videodev.h>
#include <linux/videodev2.h>
#include <linux/types.h>

#include "frame.h"
#include "fourcc.h"
#include "yuv2rgb.h"
#include "utils.h"

/* Image Format Converter */
VidConv converters[] = {
  {
  name: "YUV420 to RGB24 Converter",
  input: V4L2_PIX_FMT_YUV420,
  output: V4L2_PIX_FMT_RGB24,
  convert: yuv420_to_rgb24
  },
  {
  name: "YUV420 to BGR24 Converter",
  input: V4L2_PIX_FMT_YUV420,
  output: V4L2_PIX_FMT_BGR24,
  convert: yuv420_to_bgr24
  },
	
  {
  name: "YUYV to RGB24 Converter",
  input: V4L2_PIX_FMT_YUYV,
  output: V4L2_PIX_FMT_RGB24,
  convert: yuyv_to_rgb24
  },
	
  {
  name: "YUYV to BGR24 Converter",
  input: V4L2_PIX_FMT_YUYV,
  output: V4L2_PIX_FMT_BGR24,
  convert: yuyv_to_bgr24
  },
  {0,0,0,0}	
};

/* V4L Frame Function Call */

VidFrame *vidFrameCreate(){
  VidFrame * frame = malloc(sizeof(VidFrame));
  memset(frame,0,sizeof(VidFrame));
  frame->refcount = 1; 
  return frame;
}

void vidFrameRelease(VidFrame **frame){
  if ((*frame)->refcount>1){
    rvtk_log(RVTK_ERROR,"Release a frame with refcount > 1 \n");	
  }
	
  if  ( (*frame)->data !=0 ){
    free((*frame)->data);		
  }

  if  ( (*frame)->name !=0 ){
    free((*frame)->name);		
  }
	
  free((*frame));
  *frame = 0;
}

void vidFrameRef(VidFrame *frame){
  frame->refcount++;	
}

void vidFrameUnref(VidFrame **frameptr){
  if (frameptr==0 || *frameptr ==0 )
    return;
		
  if ( --(*frameptr)->refcount == 0) {
    rvtk_log(RVTK_DEBUG,"A frame's refcount dropped to zero\n");
    vidFrameRelease(frameptr);	
  } 
}

void vidFrameModify(VidFrame **frameptr){
  VidFrame *frame = *frameptr;
	
  if (frame->refcount > 1 || frame->readonly) {
		
    vidFrameUnref(frameptr);
    *frameptr = vidFrameClone(frame);
		   
  }
}

void vidFrameCache(VidFrame **frameptr){
  VidFrame *frame = *frameptr;
	
  if (frame->refcount > 1 || frame->readonly){
		
    vidFrameUnref(frameptr);
    *frameptr = vidFrameCreate(0);
    vidFrameCopy(frame,*frameptr,0);
  }
}

int vidFrameChangeFormat(VidFrame *frame,fourcc_t format,int width,int height){
  if (frame->refcount > 1 ){
    rvtk_log(RVTK_ERROR , "Change a frame with refcount = %d\n",frame->refcount);
    return -1; 
  } else if (frame->readonly) {
    rvtk_log(RVTK_ERROR,"Change a read only frame\n");
    return -1;
  } 	
	
  int bufsize =  vidFourccCalcFrameSize(format,width,height);
	
  if (bufsize > vidFrameGetBufferLength(frame) ){
    vidFrameResizeBuffer(frame,bufsize);	
  }	
	
  frame->format = format;
  frame->size.width = width;
  frame->size.height = height;
  return 0;
}

int vidFrameGetWidth(VidFrame *frame) {return frame->size.width;}

int vidFrameGetHeight(VidFrame *frame) {return frame->size.height;}

int vidFrameGetImageLength(VidFrame *frame) {return frame->imagesize;}

int vidFrameGetBufferLength(VidFrame *frame) { return frame->buflen;}

unsigned char* vidFrameGetImageData(VidFrame *frame) {return frame->data;}

fourcc_t vidFrameGetFormat(VidFrame *frame) { return frame->format;}

int vidFrameGetRowStride(VidFrame *frame) { return frame->bytesperline;}

int vidFrameResizeBuffer(VidFrame *frame,int length){
  frame->data = realloc(frame->data,length);
  frame->buflen = length;
  return length;	
}

/**
 * 
 *  @parm deep Perform a deep copy. If FALSE, it will simply allocate a buffer without copy the data.
 */

void vidFrameCopy(VidFrame *src,VidFrame *dest,int deep){
  dest->size = src->size;
  dest->bytesperline = src->bytesperline; 
  dest->imagesize = src->imagesize;

  if (vidFrameGetImageLength(src) > vidFrameGetBufferLength(dest)) 
    vidFrameResizeBuffer(dest,vidFrameGetImageLength(src));
	
  if (deep)
    memcpy(dest->data , src->data,vidFrameGetImageLength(src));
}

/**
 *  @return A cloned frame with refcount equal to 1.
 */

VidFrame* vidFrameClone(VidFrame *frame){
  VidFrame *new_frame = vidFrameCreate();
	
  vidFrameCopy(frame,new_frame,1);
	
  return new_frame;	
}

VidConv* vidConvFind(fourcc_t input,fourcc_t output){
  VidConv * res=0;
  int i=0;
  while (converters[i].input!=0){
    if (converters[i].input == input &&
        converters[i].output == output ){
      res = &converters[i];
      break;
    }
    i++;	
  }
  return res;
}

int vidConvProcess(VidConv *conv,VidFrame *src,VidFrame *dest){
  int res = -1;
  dest->format = conv->output;
  dest->size = src->size;
  dest->imagesize = vidFourccCalcFrameSize(conv->output,dest->size.width,dest->size.height);
  if (dest->imagesize < 0){
    const char *name = vidFourccToString(conv->output);
    if (name)
      fprintf(stderr,"Unknown image size for format [%s]\n",name);
    else
      fprintf(stderr,"Unknown image size for format [%08x]\n",conv->output);
    return res; 	
  }

  res = conv->convert(src,dest);		
  return res;			
}
