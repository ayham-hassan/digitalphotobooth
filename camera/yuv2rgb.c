#include "fourcc.h"
#include "yuv2rgb.h"

static int yuv420_to_rgbmodel(VidFrame *src,VidFrame *dest,unsigned int rgbModel[]);
static int yuyv_to_rgbmodel(VidFrame *src,VidFrame *dest,unsigned int rgbModel[]);

static int initialized=0;

/** Refer from xawtv & camstream source code. 

    Reference file: 	camstrea:: lib/ccvt/ccvt_c2.c
**/

#define CLIP 320
int cb[256];
int cr[256];
int cg1[256];
int cg2[256];
int clip[256+2*CLIP];

#define R(Y,V) clip[CLIP + Y + cr[V]];
#define G(Y,U,V) clip[CLIP + Y - (cg1[V] + cg2[U])]
#define B(Y,U) clip[CLIP + Y +cb[U]]

static int conv_init(){
	int i;
	for (i=0;i<256;i++){
		cb[i] = ((i-128) * 454)>>8; 
                cr[i] = ((i-128) * 359)>>8;
                cg1[i] = ((i-128) * 183)>>8;
		cg2[i] = ((i-128) * 88)>>8; 
	}
	
	for (i = 0; i < CLIP; i++)
		clip[i] = 0;
	for (; i < CLIP + 256; i++)
		clip[i] = i - CLIP;
	for (; i < 2 * CLIP + 256; i++)
		clip[i] = 255;

	return 0;
}

int yuv420_to_rgb24(VidFrame *src,VidFrame *dest){
	if (!initialized){
		initialized = 1;
		conv_init();
	}
	
	int bufsize =  vidFourccCalcFrameSize(dest->format,src->size.width,src->size.height);
	if (bufsize > vidFrameGetBufferLength(dest) ){
		vidFrameResizeBuffer(dest,bufsize);	
	}
	
	unsigned int rgbModel[3] = {0,1,2}; /* RGB */
	return yuv420_to_rgbmodel(src,dest,rgbModel);
}

int yuv420_to_bgr24(VidFrame *src,VidFrame *dest){
	if (!initialized){
		initialized = 1;
		conv_init();
	}

	int bufsize =  vidFourccCalcFrameSize(dest->format,src->size.width,src->size.height);	
	
	if (bufsize > vidFrameGetBufferLength(dest) ){
		vidFrameResizeBuffer(dest,bufsize);	
	}	
	unsigned int rgbModel[3] = {2,1,0}; /* RGB */
	return yuv420_to_rgbmodel(src,dest,rgbModel);
}

/**
 * yuv420_to_rgbmodel:
 * @param frame The source video frame
 * @param dest The destination buffer. (It must be large enough to hold the final image)
 * @param rgbModel: An array to describe the order of 'R','G','B" color model.
 *  Convert a YUV420/YU12 pixel into RGB24|BGR24
 * 
 */

static int yuv420_to_rgbmodel(VidFrame *src,VidFrame *dest,unsigned int rgbModel[]){ 
	unsigned char *d,*_d;
	unsigned char *y,*u,*v;
	unsigned char *_u,*_v;
	int i,j;
	unsigned int channel[3];
	
	int w=vidFrameGetWidth(src);
	int h=vidFrameGetHeight(src);

	dest->bytesperline = w * 3;

	d = vidFrameGetImageData(dest);

	y = vidFrameGetImageData(src);
	u = y +w*h;
	v = u+(w*h)/4;
	
	for (i=0;i<h;i++) {
		_u = u; _v = v; _d = d;
		for (j=0;j<w;j+=2) {
			channel[0] = R(*y,*v);
			channel[1] = G(*y,*u,*v);
			channel[2] = B(*y,*u);
			*(d++) = channel[ rgbModel[0] ];
			*(d++) = channel[ rgbModel[1]];
			*(d++)  = channel[rgbModel[2]];
			y++;
			
			channel[0] = R(*y,*v);
			channel[1] = G(*y,*u,*v);
			channel[2] = B(*y,*u);
			
			*(d++) = channel[ rgbModel[0] ];
			*(d++) = channel[ rgbModel[1]];
			*(d++)  = channel[rgbModel[2]];
			y++;u++;v++;
		}
		
		if ( !(i % 2) ){
			u = _u; v = _v;
		}
	}
	
	return 0;
}

int yuyv_to_rgb24(VidFrame *src,VidFrame *dest){
	if (!initialized){
		initialized = 1;
		conv_init();
	}
	
	
	int bufsize =  vidFourccCalcFrameSize(dest->format,src->size.width,src->size.height);
	if (bufsize > vidFrameGetBufferLength(dest) ){
		vidFrameResizeBuffer(dest,bufsize);	
	}
	
	unsigned int rgbModel[3] = {0,1,2}; /* RGB */
	return yuyv_to_rgbmodel(src,dest,rgbModel);
}

int yuyv_to_bgr24(VidFrame *src,VidFrame *dest){
	if (!initialized){
		initialized = 1;
		conv_init();
	}
		
	int bufsize =  vidFourccCalcFrameSize(dest->format,src->size.width,src->size.height);
	
	if (bufsize > vidFrameGetBufferLength(dest) ){
		vidFrameResizeBuffer(dest,bufsize);	
	}
	
	unsigned int rgbModel[3] = {2,1,0}; /* RGB */
	return yuyv_to_rgbmodel(src,dest,rgbModel);
}

/**
 * yuyv_to_rgbmodel:
 * @param frame The source video frame
 * @param dest The destination buffer. (It must be large enough to hold the final image)
 * @param rgbModel: An array to describe the order of 'R','G','B" color model.
 *  Convert a YUV420/YU12 pixel into RGB24|BGR24
 *
 *  YUYV Pixel Format: [Y0 U0 Y1 V0 ] [ Y2 U2 Y3 V2 ] .... 
 */

static int yuyv_to_rgbmodel(VidFrame *src,VidFrame *dest,unsigned int rgbModel[]) {
	int w=vidFrameGetWidth(src);
	int h=vidFrameGetHeight(src);
	
	unsigned int channel[3];
	unsigned char *s,*d;
	unsigned char y,u,v;
	int i,j;
	
	dest->bytesperline = w * 3;
	
	d = vidFrameGetImageData(dest);
	s = vidFrameGetImageData(src);
	
	for (i=0;i<h;i++) {
		for (j=0;j<w;j+=2) {
			y = s[0]; u=s[1]; v= s[3];
			
			channel[0] = R( y , v);
			channel[1] = G( y,u,v);
			channel[2] = B(y,u);
			
			*(d++) = channel[ rgbModel[0] ];
			*(d++) = channel[ rgbModel[1]];
			*(d++)  = channel[rgbModel[2]];
			
			y = s[2];
			
			channel[0] = R( y , v);
			channel[1] = G( y,u,v);
			channel[2] = B(y,u);
			
			*(d++) = channel[ rgbModel[0] ];
			*(d++) = channel[ rgbModel[1]];
			*(d++)  = channel[rgbModel[2]];
			s+=4;
		}
		
	}
	
	return 0;
}
