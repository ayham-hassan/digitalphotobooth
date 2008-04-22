CC=gcc
CFLAGS=-c -Wall $(shell pkg-config gtk+-2.0 libglade-2.0 --cflags)
LDFLAGS=-O2 -export-dynamic $(shell pkg-config gtk+-2.0 libglade-2.0 --libs)
SOURCES=proj-nprosser/cam.c proj-nprosser/drv-v4l2.c proj-nprosser/frame.c proj-nprosser/yuv2rgb.c proj-nprosser/fourcc.c proj-nprosser/utils.c ImageManipulations.c FileHandler.c photobooth.c
INCLUDE=/usr/lib/libjpeg.a
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=photobooth

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) $(INCLUDE)
	$(CC) $(LDFLAGS) $(OBJECTS) $(INCLUDE) -o $@
	sed '/response_id/d' photobooth.glade > photobooth2.glade
	gtk-builder-convert photobooth2.glade photobooth.xml
	rm photobooth2.glade

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	
rebuild: realclean $(EXECUTABLE)

clean:
	rm -f *.o
	rm -f proj-nprosser/*.o

realclean: clean
	rm -f $(EXECUTABLE)
	rm -f photobooth.xml
