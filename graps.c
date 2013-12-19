/* graps.c
 *
 * Copyright (c)2005 Sunil Mohan Ranta <smr [at] smr.co.in>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/* TODO : some problem in resizing
 * TODO : write into video/system memory (ram) instead of hdd
 *
 */

/* grapsrc format. file should be present as ~/.grapsrc
 *
 * capture_enabled=1
 * capture_delay=40
 * capture_path=./frames/
 * showfps_enabled=1
 * showfps_delay=1000
 * flip_frame=0
 * quiet=0
 * rawvideo_stream=0
 * resize_enable=0
 * grapsctl=1
 */

#define _GNU_SOURCE

#include "graps.h"

#include <dlfcn.h>

#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glx.h>

static void (*real_glXSwapBuffers)(Display*,GLXDrawable) = NULL;

/* flag for first call */
static int graps_firsttime = 1;

/* initialize graps
 */
void graps_init(){

	int viewport[4];

	int shmid;
	key_t key;
	char *shm;

	graps_firsttime = 0;

	/* initialize shared memory */
	key = SHMKEY;
	if( (shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0 ){
		perror("shmget");
		printf("try running ipcclean\n");
		exit(1);
	}
	if( (shm = shmat(shmid, NULL, 0)) == (char *) -1 ){
		perror("shmat"); exit(1);
	}
	grap = (struct grapsStruct *)shm;
	grap->shm = shm;

	/* default settings */
	grap->data = NULL;
	grap->frame_count = 1;
	grap->fps_frame_count = 1;
	grap->fp = NULL;
	grap->capture  = 1;
	grap->capture_everyframe = 0;
	grap->capture_delay = 40;
	strcpy(grap->capture_path, "./frames/" );
	grap->showfps = 0;
	grap->showfps_delay = 1000;
	grap->flip_frame = 0;
	grap->rawvideo_stream = 1;
	grap->resize_enable = 0;
	grap->frame_ready = 0;
	grap->grapsctl = 1;

	/* get viewport size */
	glGetIntegerv(GL_VIEWPORT, viewport); /* x,y,width,height */
	grap->width = viewport[2];
	grap->height = viewport[3];
	//grap->data = (unsigned char *)malloc( grap->width * grap->height * 3 );  // ?? when do i free it ?
	grap->data = (unsigned char *)shm + sizeof( struct grapsStruct );

	/* read grapsrc */
	graps_readConfig();

	/* mesg */
	graps_mesg();
	
	/* set timers */
	gettimeofday( &grap->oldtime, NULL );
	grap->oldtime.tv_sec -= 1;
	grap->oldtime2 = grap->oldtime;

	/* get real glXSwapBuffers function from libGL.so*/
	if( !real_glXSwapBuffers ){
		char *error;
		void *handle = dlopen( "libGL.so", RTLD_LAZY);
		if (!handle) {
			fprintf(stderr, "dlopen: %s\n", dlerror());
			exit(EXIT_FAILURE);
		}
		real_glXSwapBuffers = dlsym(handle, "glXSwapBuffers");
		if ((error = dlerror()) != NULL) {
			fprintf(stderr, "dlsym: %s\n", error);
			exit(EXIT_FAILURE);
		}
	}

}


/* graps main
 * grab frame
 */
void graps(){

	int viewport[4];
	long int timediff;

	/* time since last capture */
	gettimeofday( &grap->curtime, NULL );
	timediff = ( grap->curtime.tv_sec -  grap->oldtime.tv_sec ) *1000 + (  grap->curtime.tv_usec -  grap->oldtime.tv_usec )/1000.0;

	/* capture time */
	if( grap->capture && grap->data != NULL && ( grap->capture_everyframe || timediff > grap->capture_delay ) ){
		
		/* check for resizing */
		if( grap->resize_enable ){
			glGetIntegerv(GL_VIEWPORT, viewport);
			if( viewport[2] != grap->width || viewport[3] != grap->height ){
				printf("resizing %d,%d -- %d,%d\n", grap->width, grap->height, viewport[2], viewport[3] );
				grap->width = viewport[2];
				grap->height = viewport[3];
				//grap->data = (unsigned char *)realloc( grap->data, grap->width * grap->height * 3 );
			}
		}

		// TODO : wait for frame writing by grapsctl
		//while( grap->frame_ready ){
		//	printf("frame reading delay\n");
		//	usleep( 40 );
		//}

		/* read frame data */
		glReadPixels( 0, 0, grap->width, grap->height, GL_RGB, GL_UNSIGNED_BYTE, grap->data );

		/* write frame data */
		if( grap->grapsctl ){
			grap->frame_ready = 1;
		}else{
			graps_write( grap->data );
		}

		grap->oldtime = grap->curtime;
		++grap->frame_count;

	}/* END : capture time */

	/* fps counter */
	++grap->fps_frame_count;
	timediff = ( grap->curtime.tv_sec -  grap->oldtime2.tv_sec ) *1000 + (  grap->curtime.tv_usec -  grap->oldtime2.tv_usec )/1000.0;
	if( grap->showfps && timediff > grap->showfps_delay ){
		printf("\rfps : %.2f   ", grap->fps_frame_count * 1000.0 / timediff);
		fflush( stdout );
		grap->fps_frame_count = 0;
		grap->oldtime2 = grap->curtime;
	}
}

/* intercepted glXSwapBuffers routine
 */
void glXSwapBuffers( Display *dpy, GLXDrawable drawable ){

	/* initialize graps */
	if( graps_firsttime )
		graps_init();

	/* grab frame */
	graps();
	
	/* call actual glXSwapBuffers routine*/
	real_glXSwapBuffers(dpy,drawable);
}

