#!/bin/bash

sed '/response_id/d' photobooth.glade > photobooth2.glade
gtk-builder-convert photobooth2.glade photobooth.xml
rm photobooth2.glade
gcc -Wall -g -o photobooth main.c -export-dynamic `pkg-config gtk+-2.0 libglade-2.0 --cflags --libs`
