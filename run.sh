#!/bin/sh

# run.sh
#
# Copyright (c)2005 Sunil Mohan Ranta <smr [at] smr.co.in>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# Changelog:
# Mon 01 May 2006 07:12:05 PM IST Created

if ! [ -a $GRAPSPATH/libgraps.so ] ; then
	if [ -a $PWD/libgraps.so ] ; then
		GRAPSPATH="$PWD"
	else
		echo "Error!"
		echo "libgraps.so not found in \$GRAPSPATH or \$PWD"
		echo
		echo "either set GRAPSPATH to the directory containing libgraps.so"
		echo "or copy libgraps.so to current working directory"
		exit
	fi
fi

if [ $# -lt 1 ] ; then
	echo "Error!"
	echo "Syntax : sh run.sh <application>"
	echo "ex : sh run.sh glxgears"
	exit
fi

LD_PRELOAD=$GRAPSPATH/libgraps.so $@

