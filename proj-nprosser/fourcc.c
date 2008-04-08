#include "fourcc.h"

#include <linux/ioctl.h>
#include <sys/time.h>
#include <linux/videodev.h>
//#include <linux/videodev2.h>
#include <linux/types.h>

struct {
  int code;
  const char *name;
  int numerator;
  int denominator;
  int bpp;
} fourcc_table[] = {
  {.code = 0x47504a4d,.name = "MJPEG",.numerator = -1,.denominator = 1},
  {.code = 0x56595559,	.name = "YUYV/YUY2",	.numerator = 2,.denominator = 1},

  //0x32315559
  {.code = V4L2_PIX_FMT_YUV420,.name = "YUV420/YU12",	.numerator = 3,.denominator = 2,.bpp=12},
 	
  // Raw Philips Web Cam Format?
  {.code = 0x32435750, .name = "PWC2",.numerator = -1,.denominator = 1}, 
			
  {.code = V4L2_PIX_FMT_RGB24, .name = "RGB24",.numerator = 3,.denominator = 1},
  {.code = V4L2_PIX_FMT_BGR24, .name = "BGR24",.numerator = 3,.denominator = 1},	
	
	
  {.code = V4L2_PIX_FMT_BGR32,  .name = "BGR32",.numerator = 4,.denominator = 1,.bpp =32},
  {.code = V4L2_PIX_FMT_RGB32, .name = "RGB32",.numerator = 4,.denominator = 1,.bpp=32},
	
  {.code = V4L2_PIX_FMT_UYVY, .name = "UYVV",.numerator = 2,.denominator = 1,.bpp=16},
  {.code = V4L2_PIX_FMT_YUV422P, .name = "YUV422P",.numerator = 2,.denominator = 1,.bpp=16},
	
  {.code = V4L2_PIX_FMT_GREY,  .name = "GREY",.numerator = 1,.denominator = 1,.bpp=8},
  {.code = V4L2_PIX_FMT_HI240,  .name = "HI240",.numerator = 1,.denominator = 1,.bpp=8},
  {.code = V4L2_PIX_FMT_RGB555, .name = "RGB555",.numerator = 2,.denominator = 1,.bpp=16},
  {.code = V4L2_PIX_FMT_RGB555X, .name = "RGB555X",.numerator = 2,.denominator = 1,.bpp=16},
  {.code = V4L2_PIX_FMT_RGB565, .name = "RGB565",.numerator = 2,.denominator = 1,.bpp=16},
  {.code = V4L2_PIX_FMT_RGB565X, .name = "RGB565X",.numerator = 2,.denominator = 1,.bpp=16},
  {.code = V4L2_PIX_FMT_YUV411P, .name = "YUV411P",.numerator = 3,.denominator = 2,.bpp=12},
  {.code = V4L2_PIX_FMT_YVU420, .name = "YVU420",	.numerator = 3,.denominator = 2,.bpp=12},
  {.code = V4L2_PIX_FMT_YUV410, .name = "YUV410/YUV410P",.numerator = 0,.denominator = 1,.bpp=9},
  {.code = V4L2_PIX_FMT_YVU410, .name = "YVU410",.numerator = 0,.denominator = 1,.bpp=9},
  //{.code = , .name = "",.numerator = 0,.denominator = 1,.bpp=}, //unknwon size	
	
  //{.code = , .name = "",.numerator = -1,.denominator = 1},
	
	 
  {0,0,0,0}	
};


/**
 *  @Return A string of the given code's name or null if it is unknown.
 */

const char* vidFourccToString(fourcc_t code){
  const char *res=0;
  int i=0;
	
  //	for ( i=0;i<fourcc_table_size;i++){
  while (fourcc_table[i].code!=0){	
    if (fourcc_table[i].code == code){
      res = fourcc_table[i].name;
      break; 	
    }
    i++;
  } 
	
  return res;	
}

int vidFourccCalcFrameSize(fourcc_t code,int width,int height){
  int i=0;
  int size=-1;
	
  //for ( i=0;i<fourcc_table_size;i++){
  while (fourcc_table[i].code!=0){
    if (fourcc_table[i].code == code){
      if (fourcc_table[i].numerator>0)
        size = width * height * fourcc_table[i].numerator / fourcc_table[i].denominator;
      //			if (fourcc_table[i].func)
      //				size = fourcc_table[i].func(width,height);
      break; 	
    }
    i++;
  } 
  return size;		
}
