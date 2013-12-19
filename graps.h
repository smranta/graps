/* graps.h
 *
 * Copyright (c)2005 Sunil Mohan Ranta <smr [at] smr.co.in>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/* grapsrc format. file should be present as ~/.grapsrc
 *
 * capture_enabled=1
 * capture_delay=40
 * capture_everyframe=0
 * capture_path=./frames/
 * showfps_enabled=1
 * showfps_delay=1000
 * flip_frame=0
 * quiet=0
 * rawvideo_stream=0
 * resize_enable=0
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>
#include <time.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#define FRAME_SIZE_MAX 1024*1024*3
#define SHMSZ FRAME_SIZE_MAX + sizeof( struct grapsStruct )
#define SHMKEY	2771

/* data structure for graps */
static struct grapsStruct{

	struct timeval curtime, oldtime, oldtime2;
	unsigned char *data;
	unsigned char *data2;
	int frame_count;
	int fps_frame_count;
	FILE *fp;
	int capture_delay;
	int capture_everyframe;
	int capture;
	char capture_path[1024];
	int showfps;
	int showfps_delay;
	int flip_frame;
	int width;
	int height;
	int quiet;
	int rawvideo_stream;
	int resize_enable;
	int frame_ready;
	int grapsctl;
	char *shm;

} *grap;

ssize_t getline(char **lineptr, size_t *n, FILE *stream);

/* read grapsrc
 */
void graps_readConfig(){
	char *line = NULL;
	int read;
	size_t len;
	char grapsrc[1024];

	/* load graps config file : ~/.grapsrc */
	sprintf( grapsrc, "%s/.grapsrc", getenv("HOME") );
	grap->fp = fopen( grapsrc, "r" );
	if( grap->fp != NULL ){
		
		while( (read = getline(&line, &len, grap->fp)) != -1 ){
			if( line[read-1] == '\n' ) line[read-1] = '\0';
			
			if( line[0] == '#' || read == 1 )
				continue;
			
			if( strstr( line, "capture_enabled" ) != NULL ){
				grap->capture = atoi( &line[16] );
			}
			else if( strstr( line, "capture_everyframe" ) != NULL ){
				grap->capture_everyframe = atoi( &line[19] );
			}
			else if( strstr( line, "capture_delay" ) != NULL ){
				grap->capture_delay = atoi( &line[14] );
			}
			else if( strstr( line, "capture_path" ) != NULL ){
				strcpy( grap->capture_path, &line[13] );
			}
			else if( strstr( line, "showfps_enabled" ) != NULL ){
				grap->showfps = atoi( &line[16] );
			}
			else if( strstr( line, "showfps_delay" ) != NULL ){
				grap->showfps_delay = atoi( &line[14] );
			}
			else if( strstr( line, "flip_frame" ) != NULL ){
				grap->flip_frame = atoi( &line[11] );
			}
			else if( strstr( line, "quiet" ) != NULL ){
				grap->quiet = atoi( &line[6] );
			}
			else if( strstr( line, "rawvideo_stream" ) != NULL ){
				grap->rawvideo_stream = atoi( &line[16] );
			}
			else if( strstr( line, "resize_enable" ) != NULL ){
				grap->resize_enable = atoi( &line[14] );
			}
			else if( strstr( line, "grapsctl" ) != NULL ){
				grap->resize_enable = atoi( &line[8] );
			}
			else {
				printf("grapsrc : \"%s\" not recognised. ignored.\n", line );
			}
		}
		fclose( grap->fp );
	}else{
		printf("graps config file not found. using default values.\n");
	}

	if( grap->capture_everyframe ){
		grap->grapsctl = 0;
	}

	if( grap->rawvideo_stream ){
		grap->resize_enable = 0; // wrint to video stream doesn't allow resizing
		if( !grap->grapsctl ){
			grap->fp = fopen( "/tmp/grapsvideopipe", "w" );
			if( !grap->fp ){
				printf("Error : opening videopipe");
				exit( 0 );
				return;
			}
		}
	}

}

/* graps mesg
 */
void graps_mesg( void ){
	if( !grap->quiet ) printf("\
graps v0.1								\n\
Copyright 2006, smr <smr at smr.co.in>	\n\
										\n\
using :									\n\
    capture_enabled     : %d			\n\
    capture_delay       : %d			\n\
    capture_everyframe  : %d			\n\
    capture_path        : %s			\n\
    showfps_enabled     : %d			\n\
    showfps_delay       : %d			\n\
    flip_frame          : %d			\n\
    rawvideo_stream     : %d			\n\
    resize_enable       : %d			\n\
    grapsctl            : %d			\n\n\
", grap->capture, grap->capture_delay, grap->capture_everyframe,
 grap->capture_path, grap->showfps, grap->showfps_delay,
 grap->flip_frame, grap->rawvideo_stream, grap->resize_enable, grap->grapsctl );
}

/* flip the frame, as glRead gives inverted data */
void graps_flipFrame( unsigned char *data ){
	int i, j, j2, k;
	unsigned char t;

	for( i=0; i<grap->width; ++i ){
		for( j=0; j<grap->height/2; ++j ){
			j2 = grap->height - j;
			for( k=0; k<3; ++k ){
				t = data[ j * grap->width * 3 + i * 3 + k ];
				data[ j * grap->width * 3 + i * 3 + k ] = data[ j2 * grap->width *3 + i * 3 + k ];
				data[ j2 * grap->width * 3 + i * 3 + k ] = t;
			}
		}
	}
}

/* write frame data */
void graps_write( unsigned char *data ){
	char frameName[1024];

	/* flip the frame, as glRead gives inverted data */
	if( grap->flip_frame )
		graps_flipFrame( data );

	if( grap->rawvideo_stream ){
		fwrite( data, sizeof( unsigned char), grap->width*grap->height*3, grap->fp );
	}else{
		sprintf( frameName, "%s/frame%04d.ppm", grap->capture_path, grap->frame_count );
		grap->fp = fopen( frameName, "w");
		if( grap->fp == NULL ){
			printf("Error writing file %s. Make sure path is valid and quota is not full.\n", frameName );
			exit( 0 );
		}
		fprintf( grap->fp, "P6\n%d %d\n255\n", grap->width, grap->height );
		fwrite( data, sizeof( unsigned char), grap->width*grap->height*3, grap->fp );
		fclose( grap->fp );
	}
}

