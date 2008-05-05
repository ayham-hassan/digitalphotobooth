#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "fourcc.h"

/* TODO:
 *  Auto-detection through sysfs interface.
 *     /sys/class/video4linux/ * /dev
 * 
 */

#include "drv-v4l2.h"

#define DEBUG
#ifdef DEBUG
#include <stdio.h>
#define DPRINTF(...) fprintf(stderr,__VA_ARGS__);
#else
#define DPRINTF(...)
#endif

//#define BURST_MODE 

/////////////////////////
/* Static Functions ****/
/////////////////////////


/// Free null terminated array
static void free_strv(char **data){
  int i=0;
	
  if (data!=0){
    while (data[i]!=0){
      free(data[i++]);	
    }
    free(data);
  }
}

static struct {
  int cmd;
  char *name;
} ioctl_table[] = {
  {VIDIOC_QUERYCAP,"VIDIOC_QUERYCAP"},
  {VIDIOC_G_FMT,"VIDIOC_G_FMT"},
  {VIDIOC_S_FMT,"VIDIOC_S_FMT"},
  {VIDIOC_ENUMINPUT,"VIDIOC_ENUMINPUT"},
  {VIDIOC_S_INPUT,"VIDIOC_S_INPUT"},
  {VIDIOC_ENUMSTD,"VIDIOC_ENUMSTD"},
  {VIDIOC_S_STD,"VIDIOC_S_STD"},
  {VIDIOC_TRY_FMT,"VIDIOC_TRY_FMT"},
  {VIDIOC_G_PARM,"VIDIOC_G_PARM"},
  {VIDIOC_S_PARM,"VIDIOC_S_PARM"},
  {VIDIOC_QBUF,"VIDIOC_QBUF"},
  {VIDIOC_DQBUF,"VIDIOC_DQBUF"},
  {VIDIOC_REQBUFS,"VIDIOC_REQBUFS"},
  {VIDIOC_G_STD,"VIDIOC_G_STD"},
  {VIDIOC_S_STD,"VIDIOC_S_STD"},
  {VIDIOC_G_INPUT,"VIDIOC_G_INPUT"},
  {VIDIOC_S_INPUT,"VIDIOC_S_INPUT"},
  {VIDIOC_STREAMON,"VIDIOC_STREAMON"},
  {VIDIOC_STREAMOFF,"VIDIOC_STREAMOFF"},
  {0,0}
};

static const char*  v4l2_ioctl_name(int video_cmd){
  const char*  res=0;
  int i=0;
	
  //for (i=0;i<sizeof(ioctl_table) / sizeof(struct _ioctl);i++){
  while (ioctl_table[i].name != 0){
    if (ioctl_table[i].cmd == video_cmd){
      res = ioctl_table[i].name;
      break;
    }
    i++;
  }  

  return res;
}

static int capture_log(V4L2Capture *dev,char *format,...) {
  int n=0;
	
  if (dev->log){
    va_list ap;
    va_start(ap,format);
    fprintf(stderr,"[libv4l2] ");
    n = vfprintf(stderr,format,ap);
    va_end(ap);
  }
	
  return n;
}

static int v4l_ioctl(V4L2Capture *dev,int cmd,void *arg){
  int fd = dev->fd;
  int res = ioctl(fd,cmd,arg);
  const char* cmdstr;

  if (res<0 && dev->log) {
    cmdstr = v4l2_ioctl_name(cmd);
    if (cmdstr) {
      fprintf(stderr,"[libv4l2] ioctl(%s) : %s\n",cmdstr,strerror(errno));
    } else {
      fprintf(stderr,"[libv4l2] ioctl(%08x) : %s\n",cmd,strerror(errno));
    }
  }
  return res;
}

static void capture_clear_frames_buffer(V4L2Capture *dev){
  int i;
	
  if (dev->framesbuffer) {
    for (i=0;i<dev->frames;i++) {
      if (dev->framesbuffer[i].data!=0)
        free(	dev->framesbuffer[i].data);
    }
    free(dev->framesbuffer);
    dev->framesbuffer = 0;
    dev->frames = 0;
  }
}

static int capture_create_frames_buffer(V4L2Capture *dev,int size){
  capture_clear_frames_buffer(dev);
  VidFrame *frame;
  int i;
	
	
  dev->frames = size;
	
  dev->framesbuffer = malloc(sizeof(VidFrame) * size);
	
  memset(dev->framesbuffer,0,sizeof(VidFrame) * size);
	
  dev->curr_frame_idx = -1;
	
  for (i=0;i<dev->frames;i++){
    frame = &dev->framesbuffer[i];
    frame->size.width = dev->resolution.width;
    frame->size.height = dev->resolution.height;
    frame->format = dev->format;
    frame->imagesize = dev->bufsize;
    frame->bytesperline = dev->bytesperline;
  }
	
  return size;
}

/// Enqueue a frame
static int capture_enqueue(V4L2Capture *dev,int index){
  struct v4l2_buffer buffer;
  int res;
	
  //printf("%s::index = %d\n",__func__,index);
  memset (&buffer, 0, sizeof (buffer));
	
  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buffer.memory = V4L2_MEMORY_MMAP;
  buffer.index = index;//?
	
  res = v4l_ioctl(dev,VIDIOC_QBUF,&buffer);
	
  return res;	
}

/// Dequeue a frame (blocked I/O)

static int capture_dequeue(V4L2Capture *dev){
  struct v4l2_buffer buffer;
  int res;
  //printf("%s\n",__func__);
	
  memset (&buffer, 0, sizeof (buffer));

  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buffer.memory = V4L2_MEMORY_MMAP;
	
  res = v4l_ioctl(dev,VIDIOC_DQBUF,&buffer);
	
  return res;
}

/**
 *  @todo  implement vidFrameMmap() to avoid to access VidFrame
 *  member attributes directly 
 */

static int capture_mmap(V4L2Capture *dev,int nBuffer){
  int res;
  struct v4l2_requestbuffers reqbuf;
  unsigned int i;

  memset (&reqbuf, 0, sizeof (reqbuf));
  reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  reqbuf.memory = V4L2_MEMORY_MMAP;
  reqbuf.count = nBuffer;
	
  res = v4l_ioctl (dev, VIDIOC_REQBUFS, &reqbuf);
	
  if (res<0){
    capture_log(dev,"Video mmap-streaming is not supported\n");
    return res;
  } 
	
  //capture_log(dev,"Allocated %d frames for buffer reading\n",reqbuf.count); 
	
  capture_create_frames_buffer(dev,reqbuf.count);

  for (i=0; i < dev->frames ; i++){
    struct v4l2_buffer buffer;
    memset (&buffer, 0, sizeof (buffer));
		
    buffer.type = reqbuf.type;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = i;
    res = v4l_ioctl (dev, VIDIOC_QUERYBUF, &buffer);
    if (!res){
      dev->framesbuffer[i].buflen =buffer.length; /* remember for munmap() */
      dev->framesbuffer[i].readonly = 1; /* Do not allow to be modified by client */
#if 0			 
      dev->framesbuffer[i].data = mmap (NULL, buffer.bufsize,
                                        PROT_READ, /* required */
                                        MAP_SHARED,             /* recommended */
                                        dev->fd, buffer.m.offset);
#else
      dev->framesbuffer[i].data = mmap (NULL, buffer.length,
                                        PROT_READ | PROT_WRITE, /* required */
                                        MAP_SHARED,             /* recommended */
                                        dev->fd, buffer.m.offset);
#endif                                 
                                 
      if (dev->framesbuffer[i].data == MAP_FAILED){
        capture_log(dev,"mmap: %s\n",strerror(errno));
        res = -1;
        break;
      }                              
    } else {
      break;	
    }
  }
  return res;
}
/**
 *  @todo  implement vidFrameMunmap() to avoid to access VidFrame
 *  member attributes directly 
 */

static int capture_munmap(V4L2Capture *dev){
  int res=0;
  int i;
  for (i=0; i < dev->frames ; i++){
    munmap(dev->framesbuffer[i].data,dev->framesbuffer[i].buflen);
    dev->framesbuffer[i].data = 0;
    dev->framesbuffer[i].buflen = 0;
  }
  return res;
	
}

static int capture_refresh_norm(V4L2Capture *dev){
  int res;
  v4l2_std_id id;
	
  res = v4l_ioctl(dev,VIDIOC_G_STD,&id);
	
  int i = 0;
  if (dev->norm_list){
    while (dev->norm_list[i]!=0){ 
      if (dev->std_list[i] == id){
        dev->norm = i;
        break;
      }
      i++; 
    }
		
    if (dev->norm_list[i]==0)
      res = -1;
  }
		
  return res;
}

static int capture_refresh_image_format(V4L2Capture *dev){
  struct v4l2_format argp;
  int res;
  int width;
  int height;
	
  argp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  res = v4l_ioctl(dev,VIDIOC_G_FMT,&argp);
	
  if (!res){
    dev->format = argp.fmt.pix.pixelformat;
    dev->resolution.width = width = argp.fmt.pix.width;
    dev->resolution.height = height = argp.fmt.pix.height;
    dev->bufsize = argp.fmt.pix.sizeimage;
    dev->bytesperline = argp.fmt.pix.bytesperline;
  }
  return res;
}

static int capture_refresh_channel(V4L2Capture *dev){
  int input;
  int res;
	
  res = v4l_ioctl(dev,VIDIOC_G_INPUT,&input);
	
  if (!res){
    dev->channel = input;	
  }
  return res;
}

/** Internal function. Should be involved by v4l_dev_open only.
 *  Return no. of channel supported.
 */ 
 
static int capture_query_channel(V4L2Capture *dev){
  struct v4l2_input argp;
  int i=0;
  int fd = dev->fd;
  int res;
  int n;
	
  while (1){
    argp.index = i;
    res = ioctl(fd,VIDIOC_ENUMINPUT,&argp);
    //printf("res=%d\n",res);
    if (res)
      break;
    i++;
  }
	
  n = i;
		
  if (i){	
    dev->channel_list = realloc(dev->channel_list,sizeof(char*) * (i+1));
		
    for (i=0;i<n;i++){
      argp.index = i;
      res = v4l_ioctl(dev,VIDIOC_ENUMINPUT,&argp);
      //			v4l_log(dev,"%s::channel[%s] %s\n",
      //					__func__,
      //					argp.name,
      //					argp.type == V4L2_INPUT_TYPE_TUNER ? "has tuner" : "hasn't tuner");
      dev->channel_list[i] = strdup((char *)argp.name);		
    }
    dev->channel_list[i] = 0;
  }
	
  dev->nChannels = n;
	
  return n;
}

/**  Internal function. Should be involved by v4l2CaptureOpen only.
 * Return: no. of norm supported
 */

static int capture_query_norm(V4L2Capture *dev){
  struct v4l2_standard std;
	
  int i=0,n;
  int fd = dev->fd;
  int res;
	
  while (1){
    std.index = i;
    res = ioctl(fd,VIDIOC_ENUMSTD,&std);
    if (res)
      break;
    i++;
  }
	
  n = i;
		
  if (i){
    dev->norm_list = realloc(dev->norm_list,sizeof(char*) * (i+1));
    dev->std_list =  realloc(dev->std_list,sizeof(v4l2_std_id) * (i+1));
		
    for (i=0;i<n;i++){
      std.index = i;
      //	res = ioctl(fd,VIDIOC_ENUMINPUT,&argp);
			
      res = v4l_ioctl(dev,VIDIOC_ENUMSTD,&std);
		
      //capture_log(dev,"%s::norm[%s] id = %d\n",__func__,std.name,std.id);
					
      dev->norm_list[i] = strdup((char *)std.name);
      dev->std_list[i] = std.id;
					
    }
    dev->norm_list[i] = 0;
  }
	
  dev->nNorms = n;	
		
  return n;	
}

/**  Query no. of buffer type supported.
 *  Internal function. Should be involved by v4l_dev_open only.
 */

static int capture_query_buffer_type(V4L2Capture *dev){
  struct v4l2_format argp;
  int fd = dev->fd;
  int i=0,k=0;
  int res;

  memset(dev->buffer_type,0,sizeof(dev->buffer_type));
	
  dev->buffer_types = 0;
	
  argp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  res = ioctl(fd,VIDIOC_G_FMT,&argp);	
		
  for (i=0;i<V4L2_BUF_TYPE_PRIVATE;i++){
    argp.type = i;
    res = ioctl(fd,VIDIOC_TRY_FMT,&argp);
    //res = v4l_ioctl(dev,VIDIOC_TRY_FMT,&argp);
    //printf("res=%d\n",res);
    if (!res) {
      dev->buffer_type[i] = 1;
      k++;
    } 	
  }
	
  dev->buffer_types= k;
	
  return k;
}

/// Query the supported image formats(in fourcc) for Video Capture Stream.

static int capture_query_image_format(V4L2Capture *dev){
  int res;
  int i;
  int fd = dev->fd;
	
  int max = 20;
  struct v4l2_fmtdesc argp;

  dev->imageformat_list = malloc(sizeof(int) * (max+1));
  dev->imageformat_list[0] = 0;
	
  argp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE ;
  i = 0;
		
  while (1){
    argp.index = i;
    res = ioctl(fd,VIDIOC_ENUM_FMT,&argp);
    if (res){
      break;
    } else {
			
      dev->imageformat_list[i++] = argp.pixelformat;

      //			DPRINTF("Supported image format[%d] = %s \n",i-1,
      //				vidFourccToString(argp.pixelformat) ? vidFourccToString(argp.pixelformat) : "Unknown"
      //				);
      //DPRINTF("%08x\n",argp.pixelformat);

      if (i>=max){
        max+=10;
        dev->imageformat_list=realloc(dev->imageformat_list,sizeof(int) * (max+1));
      }									
    }
  }
  dev->imageformat_list[i]=0;
  //}
  return i;
}

static int capture_query_fps(V4L2Capture *capture,int *numerator,int *denominator){
	
  struct v4l2_streamparm argp;
  int res;
  argp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
  res = v4l_ioctl(capture,VIDIOC_G_PARM,&argp);
	
  if (!res){
    if ( argp.parm.capture.capability & V4L2_CAP_TIMEPERFRAME){
      *numerator = argp.parm.capture.timeperframe.numerator;
      *denominator = argp.parm.capture.timeperframe.denominator;
      //printf("V4L2_CAP_TIMEPERFRAME is supported\n");
      capture->fps = 1 / ( (double)*numerator / *denominator  );
    } else {
      capture_log(capture,"V4L2_CAP_TIMEPERFRAME is not supported\n");
      res = -1;
      capture->fps = -1;
    }	
	
  } else {
    *numerator = -1;
    *denominator = 1;
    capture->fps = -1;
		 	
  }
	
  return res;
}


////////////////////////
/* Public Functions ***/
////////////////////////

/**
 *  @param filename - The location of device.
 *  @Return Pointer to a initialized video capturing
 * structure, or NULL if failed to open the camera. 
 * 
 *  The function v4l2CaptureOpen opens and querys the properties
 * of a V4L2 video device. The result will be used to initialize
 * the newly allocated V4L2Capture. 
 * 
 *  Soon the video device is not used anymore , it should 
 *  be released by v4l2CaptureRelease
 * 
 * @see v4l2CaptureRelease. 
 * 
 */

V4L2Capture* v4l2CaptureOpen(const char *filename) {
  struct v4l2_capability argp;
  int fd;
  int res;
  V4L2Capture *capture =0;
	
  fd = open(filename,O_RDWR);
	
  if (fd!=-1){
    res = ioctl (fd,VIDIOC_QUERYCAP,&argp);
		
    if (!res) { //It is a V4L2 device
      capture = malloc(sizeof(V4L2Capture));
      memset(capture,0,sizeof(V4L2Capture));
      capture->fd = fd;
      capture->name = strdup((char *)argp.card);
      capture->driver = strdup((char *)argp.driver);
      capture->bus = strdup((char *)argp.bus_info);
      capture->location = strdup((char *)filename);
      capture->capabilities = argp.capabilities;
      capture->iomode = V4L2_CAP_READWRITE;
			
      v4l2CaptureSetLog(capture,1);
			
      capture_query_channel(capture);
      capture_query_norm(capture);
      capture_query_buffer_type(capture);
      capture_query_image_format(capture);
			
      capture_refresh_image_format(capture);
			
      capture->channel = -1;
      if (capture->nChannels)
        capture_refresh_channel(capture);
				
      capture->norm = -1;
      if (capture->nNorms)
        capture_refresh_norm(capture);
				
      int n,d;
			
      capture_query_fps(capture,&n,&d);	
				
    } else {
      close(fd);	
    }
  }
	
  return capture;
}

/**
 *  @param capture - video capture structure
 *  @return A newly grabbed video frame. It should not be released or modified by user.
 * 
 *  \todo time stamp
 */
VidFrame* v4l2CaptureQueryFrame(V4L2Capture* capture){
  VidFrame *frame=0;
  int n;
		
  if (capture->iomode == V4L2_CAP_STREAMING){
		
    capture->curr_frame_idx++;
		
    if (capture->curr_frame_idx >= capture->frames){
      capture->curr_frame_idx = 0;	
    }
		
    capture_dequeue(capture);
		
    frame= &capture->framesbuffer[capture->curr_frame_idx];
    //#ifndef BURST_MODE
    if (!capture->burst_mode){
      int next = capture->curr_frame_idx +1;
		
      if (next >= capture->frames)
        next=0;
		
      capture_enqueue(capture,next);
    } else {
      //#else
      capture_enqueue(capture,capture->curr_frame_idx);
    }
    //#endif		
		
  } else if (capture->iomode == V4L2_CAP_READWRITE) {
		
    if (!capture->frames){ 
      capture_create_frames_buffer(capture,2);	
    }
		
    capture->curr_frame_idx++;
    if (capture->curr_frame_idx >= capture->frames){
      capture->curr_frame_idx = 0;	
    }
		
    frame= &capture->framesbuffer[capture->curr_frame_idx]; 
		
    if (vidFrameGetBufferLength(frame) < capture->bufsize){
      //frame->data = realloc(frame->data,capture->bufsize);
      vidFrameResizeBuffer(frame,capture->bufsize);
    }
		
    n = read(capture->fd,frame->data,capture->bufsize);
		
    if (n!=capture->bufsize){
      capture_log(capture,"Excepted %d of bytes read but only %d retruned\n",
                  capture->bufsize,n); 						
      frame = 0;
    }
		
  } else { 
    capture_log(capture,"Unknown IO Mode\n");
  }
  return frame;
}

void v4l2CaptureRelease(V4L2Capture** capture){
	
  if (capture == 0 || *capture == 0)
    return;
	
  V4L2Capture *_cap = *capture;
	
	
  v4l2CaptureStopStreaming(_cap);
	
  if (_cap->fd){
    close(_cap->fd);
  }
	
  free(_cap->location);
  free(_cap->name);
  free(_cap->driver);
  free(_cap->bus);
		
  free_strv(_cap->channel_list);
	
  free_strv(_cap->norm_list);
  if (_cap->std_list)
    free(_cap->std_list);
	
  if (_cap->imageformat_list!=0) free(_cap->imageformat_list);
	
  capture_clear_frames_buffer(_cap);
	
  *capture = 0;
}

void v4l2CaptureSetLog(V4L2Capture *capture,int state){
  capture->log = state;
}

/// Start streaming I/O (Memory Mapping)
/** 
 * @param capture - video capturing structure
 * @param burst_mode - Turn on burst mode.
 * @param nBuffer - no. of buffer should be allocated. (The min value is 2.) 
 * @Return Non-zero value to indicate error
 * 
 * Streaming is an I/O method where only pointers to buffers
 * are exchanged between application and driver. Depend on 
 * driver's implementation, the max no. of buffers could be 
 * different. 
 * 
 * To retrieve a frame from device, a buffer must be enqueued
 * into capturing buffer and then dequeued when the user require 
 * to grab the frame.
 * 
 * Burst mode controls the way of enqueuing process. With 
 * burst mode, all buffers are enqueued in order to get 
 * the highest throughput. Otherwise, only a single buffer
 * is enqueued for each iteration of v4l2CaputreQueryFrame() .
 *  
 *  
 */

int v4l2CaptureStartStreaming(V4L2Capture *capture,int burst_mode,int nBuffer){
  int res = -1;
  if ( capture->iomode != V4L2_CAP_STREAMING &&
       capture->capabilities & V4L2_CAP_STREAMING ){
			
    if (nBuffer<2)
      nBuffer = 2;	
			
    res = capture_mmap(capture,nBuffer);
    if (!res) {
      capture->iomode = V4L2_CAP_STREAMING;
      capture->burst_mode = burst_mode;
      //#ifndef BURST_MODE
      if (!capture->burst_mode) {			
        int next = capture->curr_frame_idx +1;
        if (next >= capture->frames)
          next=0;
		
        capture_enqueue(capture,next);
      } else {
        //#else
        int i;
        for (i = 0; i<capture->frames;i++){
          capture_enqueue(capture,i);
        }
      }
      //#endif
      int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			
      res = v4l_ioctl(capture,VIDIOC_STREAMON,&type);

    }
  }
  return res;
}

/// Stop streaming mode
int v4l2CaptureStopStreaming(V4L2Capture *capture){
  int res = 0;
  int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			
  v4l_ioctl(capture,VIDIOC_STREAMOFF,&type);
		
  if (capture->iomode == V4L2_CAP_STREAMING){
    res = capture_munmap(capture);
    if (!res) {
      capture->iomode = V4L2_CAP_READWRITE;
    }	
  }
  return res;	
}

//////////////////////////////////////////////
/* Query and set properties functions */
//////////////////////////////////////////////

/**
 * Remark:
 * 	VIDIOC_ENUM_FRAMESIZES & VIDIOC_ENUM_FRAMEINTERVALS 
 * were introduced in 2.6.19 kernel. Until Feb 2007,
 * UVC is the only driver implemented the ioctl, and then 
 * it were introduced to PWC on March.
 *
 *  \todo Implemente VIDIOC_ENUM_FRAMESIZES for >=2.6.19 kernel.
 */

int v4l2CaptureQueryResolutionsList(V4L2Capture *capture,VidSize **sizes){
  int res;
  int n=0;
  int i=0,j,k;
	
  struct v4l2_format argp;
	
  int size[][2] = {
    {160,120},
    {320,240},
    {360,288},
    {640,480},
    {720,576},
    {800,600},
    {1024,768}
  }; 
  int count =  sizeof(size) / sizeof(int[2]);
  *sizes = 0;
	
  argp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  res = v4l_ioctl(capture,VIDIOC_G_FMT,&argp); 
	
  if (res){
    return n;	
  }

  i = sizeof(VidSize) * (count+1);
  *sizes = malloc(i);
  memset(*sizes,0,i); 	
	
  for (i=0;i<count ; i++){
    argp.fmt.pix.width = size[i][0];
    argp.fmt.pix.height = size[i][1];
		
    res = v4l_ioctl(capture,VIDIOC_TRY_FMT,&argp);
    if (!res){
      //printf("[%d x %d] is supported\n",argp.fmt.pix.width,argp.fmt.pix.height);
			
      for (j=0,k=0;j<n;j++){
        if ( (*sizes)[j].width == argp.fmt.pix.width &&
             (*sizes)[j].height == argp.fmt.pix.height){
          k = 1;
          break; 
        }
      }
			
      if (!k) {
        (*sizes)[n].width = argp.fmt.pix.width;
        (*sizes)[n].height = argp.fmt.pix.height;
        n++;	
      }
    }
  }
	
  return n;
}



inline const char* v4l2CaptureGetName(V4L2Capture *capture){
  return capture->name;
}

inline const char* v4l2CaptureGetLocation(V4L2Capture *capture){
  return capture->location;
}

const char* v4l2CaptureGetDriver(V4L2Capture *capture){
  return capture->driver;
}


int v4l2CaptureGetImageFormat(V4L2Capture *capture){
  return capture->format;
}


/**
 * @param capture The video capture structure
 * 
 * @param fourcc The fourcc code to be assigned for the device
 * @param size Pointer to v4lframesize for the input image's size. Pass a null pointer to use orignial size.
 * @Return Non-zero value to indicate error
 */

int v4l2CaptureSetImageFormat(V4L2Capture *capture,fourcc_t fourcc,VidSize *size){
  struct v4l2_format argp;
  int res;
	
  argp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  res = v4l_ioctl(capture,VIDIOC_G_FMT,&argp);
	
  if (!res){
    argp.fmt.pix.pixelformat = fourcc;
    if (size){
      argp.fmt.pix.width = size->width;
      argp.fmt.pix.height = size->height;
    }
    res = v4l_ioctl(capture,VIDIOC_S_FMT,&argp);
		
    capture_refresh_image_format(capture);	
		
  }
  return res;	
}

int v4l2CaptureAutoSetImageFormat(V4L2Capture *capture,fourcc_t output){
  int res = -1;
	
  if (capture->imageformat_list){
    int i=0;
    while (capture->imageformat_list[i]){
      int format = capture->imageformat_list[i];
			
      VidConv *converter = vidConvFind(format,output);
			
      if (converter){
        if (!v4l2CaptureSetImageFormat(capture,format,NULL)){
          res = capture_refresh_image_format(capture);
          break;	
        }
      } else {
        capture_log(capture,"Can't find converter for %s to %s\n",
                    vidFourccToString(format),vidFourccToString(output));
      }
      i++;
    }
  }
	
  return res;	
}



/// Get the current resolution 

int v4l2CaptureGetResolution(V4L2Capture *capture,VidSize *size){
  *size = capture->resolution;
  return 0; 
}

int v4l2CaptureSetResolution(V4L2Capture *capture,VidSize *size){
  int res ; 
  struct v4l2_format argp;
	
  argp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
  res = v4l_ioctl(capture,VIDIOC_G_FMT,&argp);
	
  if (!res){
    argp.fmt.pix.width = size->width;
    argp.fmt.pix.height = size->height;		
		
    res = v4l_ioctl(capture,VIDIOC_S_FMT,&argp);	
		
    if (!res){
      capture->resolution.width = argp.fmt.pix.width;
      capture->resolution.height = argp.fmt.pix.height;
      if (argp.fmt.pix.width != size->width ||
          argp.fmt.pix.height != size->height ){
        capture_log(capture,
                    "<Warning> Could not set frame size to [ %d x %d]\n",
                    size->width,
                    size->height);
      }
    }
  }
	
  return res;
}



double v4l2CaptureGetFPS(V4L2Capture *capture){
  return capture->fps;
}

int v4l2CaptureSetFPS(V4L2Capture *capture,int fps){
  struct v4l2_streamparm argp;
  int res;
  argp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
  res = v4l_ioctl(capture,VIDIOC_G_PARM,&argp);
	
  if (!res){
    if (! (argp.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)){
      capture_log(capture,"V4L2_CAP_TIMEPERFRAME is not supported\n");
      res = -1;
    } else {
      argp.parm.capture.timeperframe.numerator = 1;
      argp.parm.capture.timeperframe.denominator = fps;

      res = v4l_ioctl(capture,VIDIOC_S_PARM,&argp);
			
      if (!res){
        int n,d;
        res = capture_query_fps(capture,&n,&d);
      }
    }	
  }
	
  return res;
}

char** v4l2CaptureGetChannelsList(V4L2Capture *capture){
  return capture->channel_list;
}

char** v4l2CaptureGetNormsList(V4L2Capture *capture){
  return capture->norm_list;
}

int* v4l2CaptureGetImageFormatsList(V4L2Capture *capture){
  return capture->imageformat_list;
}

int v4l2CaptureSetChannel(V4L2Capture *capture,int index){
		
  int res = v4l_ioctl(capture,VIDIOC_S_INPUT,&index);
	
  if (!res){
    capture_refresh_channel(capture);
    if (capture->channel != index) {
      res = -1;	
    }
  }
	
  return res;
}

int v4l2CaptureGetChannel(V4L2Capture *capture){
  return capture->channel;
}

int v4l2CaptureSetNorm(V4L2Capture *capture,int index){
  int res=-1;
  if (index < capture->nNorms && index>=0){
    v4l2_std_id id = capture->std_list[index] ;
		
    res = v4l_ioctl(capture,VIDIOC_S_STD,&id);
		
    if (!res){
      capture_refresh_norm(capture);
			
      if ( capture->norm != index)
        res = -1;
    }
  }
  return res;
}

int v4l2CaptureGetNorm(V4L2Capture *capture){
  return capture->norm;
}

