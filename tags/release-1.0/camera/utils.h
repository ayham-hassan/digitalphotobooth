#ifndef UTILS_H_
#define UTILS_H_

typedef enum  {
	RVTK_DEBUG=0,
	RVTK_WARN=1,
	RVTK_ERROR=2
} RvtkLogLevel;

int rvtk_log(RvtkLogLevel level,const char *format,...);

#endif /*UTILS_H_*/
