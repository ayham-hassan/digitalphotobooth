#!/bin/bash

sed '/response_id/d' photobooth.glade > photobooth2.glade
gtk-builder-convert photobooth2.glade photobooth.xml
rm photobooth2.glade
make
