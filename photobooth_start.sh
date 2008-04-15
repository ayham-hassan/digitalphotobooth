#!/bin/bash

./photobooth &
sleep 5
joy2key "Digital Photo Booth" -dev /dev/input/js0 -X -buttons c -thresh 0 0 0 0 &

