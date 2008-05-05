
INSTALL = /usr/bin/install -c
INSTALLDATA = /usr/bin/install -c -m 644

prefix = /usr/local
# Prefix for each installed program,
# normally empty or `g'.
binprefix =

# The directory to install tar in.
bindir = $(prefix)/bin

# The directory to install the info files in.
datadir = $(prefix)/share/photobooth

CC=gcc
CFLAGS=-c -Wall $(shell pkg-config gtk+-2.0 libglade-2.0 --cflags)
LDFLAGS=-O2 -export-dynamic $(shell pkg-config gtk+-2.0 libglade-2.0 --libs)

SOURCES=camera/cam.c camera/drv-v4l2.c camera/frame.c camera/yuv2rgb.c camera/fourcc.c camera/utils.c usb-drive.c ImageManipulations.c FileHandler.c photobooth.c
INCLUDE=/usr/lib/libjpeg.a
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=photobooth

all: $(SOURCES) $(EXECUTABLE) photobooth.xml
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(INCLUDE) -o $@
	
photobooth.xml: photobooth.glade
	sed '/response_id/d' photobooth.glade > photobooth2.glade
	gtk-builder-convert photobooth2.glade photobooth.xml
	rm photobooth2.glade

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	
rebuild: realclean all

clean:
	rm -f *.o
	rm -f camera/*.o

realclean: clean
	rm -f $(EXECUTABLE)
	rm -f photobooth.xml
	
install: all
	$(INSTALL) photobooth $(bindir)/$(binprefix)photobooth
	$(INSTALL) photobooth_start.sh $(bindir)/$(binprefix)photobooth_start
	$(INSTALLDATA) -t $(datadir) photobooth.xml banner_image.jpg texture_fabric.gif
