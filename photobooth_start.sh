#!/bin/bash

./photobooth &
sleep 2
joy2key "Digital Photo Booth" -dev /dev/input/js0 -X -buttons 0 0 0 0 0 c -thresh 0 0 0 0 &

