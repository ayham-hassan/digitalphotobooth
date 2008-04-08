CC=gcc
CFLAGS=-c -Wall
LDFLAGS=-O2
SOURCES=cam.c drv-v4l2.c fourcc.c frame.c test.c utils.c yuv2rgb.c
INCLUDE=/usr/lib/libjpeg.a
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=test

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) $(INCLUDE)
	$(CC) $(LDFLAGS) $(OBJECTS) $(INCLUDE) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o

realclean: clean
	rm $(EXECUTABLE)
