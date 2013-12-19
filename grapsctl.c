/* grapsctl.c
 *
 * Copyright (c)2005 Sunil Mohan Ranta <smr [at] smr.co.in>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "graps.h"

int main(){

	int shmid;
	key_t key;
	char *shm;//, *s;

	/* initialize shared memory */
	key = SHMKEY;
	if( (shmid = shmget(key, SHMSZ, 0666)) < 0 ){
		perror("shmget");
		printf("Probably you didnt run opengl application first using run.sh\n");
		return 0;
	}
	if( (shm = shmat(shmid, NULL, 0)) == (char *) -1 ){
		perror("shmat"); exit(1);
	}
	grap = (struct grapsStruct *)shm;
	grap->data2 = (unsigned char *)shm + sizeof( struct grapsStruct );

	/* mesg */
	graps_mesg();
	
	/* open videopipe if video mode */
	if( grap->rawvideo_stream ){
		grap->fp = fopen( "/tmp/grapsvideopipe", "w" );
		if( !grap->fp ){
			printf("Error : opening videopipe");
			return 0;
		}
	}
	
	// capture
	printf("capture started. ctrl-c to end\n" );
	while( 1 ){
		while( !grap->frame_ready )
			usleep( 10 );
		graps_write( grap->data2 );
		grap->frame_ready = 0;
	}

	/* close videopipe if video mode */
	if( grap->rawvideo_stream ){
		fclose( grap->fp );
	}

	return 0;
}
