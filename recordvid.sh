#!/bin/sh

# record.sh
#
# Copyright (c)2005 Sunil Mohan Ranta <smr [at] smr.co.in>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

echo "syntax : sh record.sh frame-width frame-height fps output-video"
echo "default frame-width=300, frame-height=300, fps=25, output-video=output.avi"

grapspath=$GRAPSPATH

if [ "x$grapspath" == "x" ] ; then
	grapspath="."
fi

#read -p "Frame Width [defaul=300] : " width
#read -p "Frame Height [default=300] : " height
#read -p "Video FPS [default=25] : " fps

width=$1
height=$2
fps=$3
output=$4

if [ "x$width" == "x" ] ; then
	width=300
fi
if [ "x$height" == "x" ] ; then
	height=300
fi
if [ "x$fps" == "x" ] ; then
	fps=25
fi
if [ "x$output" == "x" ] ; then
	output="output.avi"
fi

mkfifo /tmp/grapsvideopipe

(mencoder /tmp/grapsvideopipe -rawvideo w=$width:h=$height:fps=$fps:format=RGB24 -demuxer rawvideo -o $output -ovc lavc -lavcopts vcodec=mpeg4:vbitrate=800 & ) >/dev/null 2>&1

$grapspath/grapsctl

rm /tmp/grapsvideopipe

ipcclean >/dev/null 2>&1

