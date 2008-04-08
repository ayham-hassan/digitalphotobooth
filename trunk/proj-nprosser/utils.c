#include "utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

int rvtk_log(RvtkLogLevel level,const char *format,...){
	int n;
	
	va_list ap;

	va_start(ap,format);
	
	n = vfprintf(stderr,format,ap);
		
	va_end(ap);
	
	return n;
}