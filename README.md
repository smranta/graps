graps
=====

graps : capture OpenGL Frames graps is a tool for grabing frames from openGL application, without editing the application.
graps can be used for making video from an opengl application. graps can also be useful for monitoring fps of opengl application.  graps writes the glReadPixel data to file on each glXSwapBuffers call. graps uses the power of LD_PRELOAD to run without modifying the application.  graps is dirty fraps alternative for screen and realtime video capturing and fps monitoring (hence benchmarking).

How to Record Video
-------------------

in ~/grapsrc set rawvideo_stream=1

run opengl application using run.sh
```
$ sh run.sh opengl-app
```

record video
```
$ sh recordvid.sh frame-width frame-height fps output-file
```

IMPORTANT : capture_delay should not be set more than time required for
writing the video. Otherwise disable grapsctl.

IMPORTANT : maximum frame size allowed is 1024x1024x3(rgb). You can increase it from source (graps.h)
You might also need to increase max shared memory (SHMMAX).


How to Record Frames to Images
------------------------------

in ~/grapsrc set rawvideo_stream=0

run opengl application using run.sh
```
$ sh run.sh opengl-app
```

to start recording frames run
```
$ ./grapsctl
```


Running without grapsctl
------------------------

in ~/grapsrc set grapsctl=0

To record run
```
$ sh run.sh opengl-app
```

NOTE : if grapsctl is disabled and you want to record video,
you need to run mencoder on videopipe first (see recordvid.sh).


How it works
------------
It intercepts glXSwapBuffer calls, and saves the framebuffer to file.
If grapsctl is enabled, then framebuffer is copied to shared memory,
and grapsctl binary writes frame from shared memory to file.



Example
-------
run glxgears
```
$ sh run.sh glxgears
```

record
```
$ sh revordvid.sh 300 300 25 glxgears.avi
```


Install
-------
copy graps folder to install directory ( say /opt/graps )
set GRAPSPATH to graps install directory


Changelog
---------
08.09.2007 :
- recording to video [ using mencoder ]. Thanks to Mineral for hints.
- grapsctl, separated frame capturing and writing to image/video.
- bug fixes : ppm header (thanks matt) and width*width (thanks Mineral).
- graps v0.2 release.

