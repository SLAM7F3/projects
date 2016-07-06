Data Viewer 3D v2.6.2
MIT Lincoln Laboratory Group 94
by Luke Skelly
Joe Adams
Joe Do (initial GUI)
Ross Anderson (MacOS version)

RUNTIME REQUIREMENTS:
OpenGL Library
Glut Library

PROGRAM DESCRIPTION: Data Viewer 3D is designed to look at 3D Laser Radar
Data, but can be used to look at any 3D data.  It currently only supports
point clouds.  A fourth dimension (p) is also used for color/intensity
information or for filtering.  The viewer can also be used to look at a
(time) sequence of 3D data.  A sequence can be loaded using a .fif file or
by separating each frame of the sequence into a sequence of files.

The data is viewed through a virtual camera.  Control over the camera is
covered in another section.  Each data point is given a color.  The
colormap direction (default = z = up) can be set to any of the 4 dimensions
(x,y,z,p).  The limits of the colormap are initially set to the min and max
value in that dimension, but can be changed at any time.  If a point's
value in that dimension is not within these limits, the point will not be
drawn.  These limits carry over after you change the direction of the
colormap. There is a variable called ZFAR that specifies the maximum
distance from the camera a point will be drawn (Default = 3000).

This version of the viewer requires that all the data be loaded into system
memory.  If the system runs out of memory, the program will crash.  After
loaded into memory, the data is organized into a tree structure.  While
moving the viewing camera or changing data view properties the viewer will
attempt to draw everything, but if it runs out of time it will give up.
This time limit is based on a variable called MINIMUM_REFRESH_RATE (Default
= 10).  If the viewer is in a static state, then all the data will be drawn
eventually.  The order the data is drawn is based on the distance from the
viewing camera.

TO RUN PROGRAM:

DataViewer3D [options] xyzfiles

	xyzfiles - list names of xyz files (.xyz, .xyzp, .3dp)
			wildcards are allowed

options
	-movie f
		display data as movie with f "frames" per draw
	-fif fiffile
		use a .fif file for "frame" info
	-double
		read input file as single precision floating point data, default=float
	-poffset offset
		initial "p" offset ("p" data index offset from xyz data), default=0
	-stride
		number of data indexes per xyz point (xyzpph stride=6, xyzp stride=4), minimum=default=4

SUPPORTED FILE FORMATS: xyzfile format (.xyz, .xyzp) (binary ieee floating
point): x y z p[0] p[1] .. p[n] x y z p[0] p[1] .. p[n]...

where x y z is the location of the point and p[x] is the "p" with offset x
and a stride of n+4

also supports harris file format (.3dp) types 1 (float float float int) and
1000 (float * n)

fif (Frame Information File) format (binary 32bit integer):
n0n1n2n3n4n5n6n7n8...

where nx is the number of points for frame x


MOVING THE CAMERA:

Translation: 
Forward/Backward - translates the camera in the direction the
camera is facing {Middle/Right Mouse Button} 
Strafe - translates the camera
left/right {Left/Right Arrow} Fly - translates the camera up from the
camera's perspective (not necessarily along z axis) {Up/Down Arrow} 
Jump To Origin - camera jumps to (0,0,0) or to origin specified by file format
(ex. .3dp = sensor position) {'o' key} 
Jump To Data - camera jumps to first data point (spatially, not in file) {'j' key} 
Look At Data - camera points to "spin point" {'k' key} 
Speed - the speed the camera will translate
(meters/(screen refresh)) {'Insert' or 'Delete' key} holding shift key will
increase speed of translation by a factor

Rotation: Rotate Left/Right - rotates camera about z-axis at "spin point"
(x,y) location {'r' and 't' key} Spin Mode - toggles in/out of "spin mode"
{'s' key}

GUI Specific: To rotate about camera position drag left mouse button in
window in "normal mode" To rotate about "spin point" drag left mouse button
in window in "spin mode" 

Jump To Position - camera jumps to the next position user left clicks on
with mouse

Set Spin Point - sets "spin point" to the next position user left clicks on
with mouse

XY Translation - translates camera along XY Plane Z Translation -
translates camera along Z axis

CHANGING THE COLORMAP: Change Colormap - changes the colormap (Rainbow,
Black and White, etc.) {'n' key} Colormap Direction - changes the direction
of the colormap (default = z) {'c' key} Page Up/Page Down will
increase/decrease max value for colormap (not implemented in GUI) Home/End
will increase/decrease min value for colormap (not implemented in GUI)
holding shift key will increase the change in min/max value for the
colormap by a factor (not implemented in GUI)

CHANGING VIEW PROPERTIES:
+/- Point Size - increases/decreases the point size (units are in screen pixels) {'+' and '-' keys}
Pthresh - the current minimum threshold for the fourth dimension {'[' and ']' keys will increase/decrease value}
FOV - Field of View of Camera when in perspective view (Degrees)

QUERYING INFORMATION:
Query Position - The position of the next point the user left clicks on will be printed in GUI
	For the keyboard: Holding down the CTRL key while clicking with the left mouse button on an object will print its
	position in the command line window
Query Distance - The distance between the start and end points of a left drag on the window will be printed in the GUI
	For the keyboard: To measure distance in 3D space, while holding the ALT key left click and hold on start position
	then drag and release on the end position
'p' key will print the probability of detection threshold
'l' key will print camera orientation

VIEWING SEQUENCES OF DATA:
PLAY - if stopped or paused, will begin playing sequence
PAUSE - if playing, will pause sequence
STOP - if playing or paused, will draw entire sequence and remain static
RESTART - '`' will restart sequence back to beginning {'~' key}
the '1' key will step through frames forward when paused
the '2' key will step through frames backward when paused
fps - Sequence Frames Per Second

OTHER GUI NOTES:
Press the "Hide GUI" Button to remove GUI
The buttons on the GUI work as toggles
The translate controls have ctrl (slower) and shift (faster) modifiers
The dual axis translate control has an Alt modifier to select a single axis
Once a spinner control has been selected (Speed, Pthresh), the up/down 
arrows can be used to modify

To update the values of eye, look, up, etc.  press the REFRESH DISPLAY
button  (or use auto refresh)

ADVANCED OPTIONS:

u key will rotate the axis orientations (REMOVED FOR NOW)
x key will force the camera to not change elevation when moving forward/backward
the ~ key will print how fast the program can draw to the screen in frames per second (toggle)
F1 allows you to type in Probability of detection.  (Must focus onto command line window)
F2 allows you to type in the Field of View (Degrees)
F3 allows you to type in ZMin (points closer than this distance from camera will not be drawn)
F4 allows you to type in ZMax (points beyond this distance away from camera will not be drawn)
F5 allows you to type in eye position (ex.  "4 100 500" will move you 4 units left, 100 units up,
						and 500 units forward)
F6 allows you to type in look vector
F7 allows you to type in FPS (frames per second)  if < 0 all frames will be drawn, set to 0 to pause
F8 allows you to type in MINIMUM_REFRESH_RATE (screen refreshes/second) if set to 0, all data will always be drawn
F12 toggles orthographic view (removes perspective)
F11 changes orthographic width and height
the '3' key locks camera to sensor position and stare at data centroid (.3dp) (for use during sequence viewing)
the '4' key locks camera to sensor position and stare at spin point (for use during sequence viewing)

COMPILING:

Required Libraries:
OpenGL (3D rendering) - http://www.opengl.org/
Glut (OpenGL Toolkit) - http://www.opengl.org/developers/documentation/glut/index.html
GLUI (for GUI Controls) - http://www.cs.unc.edu/~rademach/glui/
Boost (for smart pointers) - http://www.boost.org/

Options:
colormaps_opengl.h - #define _jsa_colormap_use_all_colormaps_ to use all colormaps instead of just the main 2 (increases compilation time)
dataviewer.h - #define GLUI_CONTROLS to use the GLUI GUI controls (requires glui library)
PREDEFINE DATAVIEWER_LIGHT for extremely simplistic controls (designed for use in a helicopter)
PREDEFINE DATAVIEWER_JIGSAW for DATAVIEWER_LIGHT with JIGSAW specific controls

Platform Specific:
Most platform specific settings are specified in arch_dependent_headers.h with the exception of glut.  The glut include is done in dataviewer.h.

WIN32 Specific:
In Visual Studio, you must link Setargv.obj for wildcard expansion

Known Bugs:
Pressing keys of the same pair (forward/backward) can give unpredictable results
There is no exception handling (notably bad with memory allocation)
GUI textboxes and spinners are not always synced with actual values if the keyboard is used (using auto refresh may help some)

Change Log:
Version 2.6.2
02/28/2003	Added option to type in MINIMUM_REFRESH_RATE
02/27/2003	Query Position, Query Distance, Set Spin Point, and Jump to Position now search for closest point if you miss
		Removed double depth buffer and dual buffer style, thus making the code simpler and faster
		Fixed pthresh change when color direction is not p
02/25/2003	Fixed double depth buffer issue
		Converted Octrees to general trees
		Fixed opengl quirk of overwriting color buffer during depth buffer copy

Version 2.6
02/20/2003	Added option to step through sequence backwards
		Re-added option to fix z value for camera (key x)
02/19/2003	Fixed Redraw issues with multiple files and fps change
02/17/2003	Maximum Redraw Rate
02/15/2003	Accumulated Redraw with Double Buffering
02/14/2003	Accumulated Redraw
01/24/2003	Octree support added
		Minimum Redraw Rate
		Axis orientation change removed

Version 2.5.2
01/13/2003	Constraint modification speed is no longer dependent on data but instead fixed
		Re-added znear zfar inputs (F3 and F4) which allows user to change accuracy of zbuffer

Version 2.5.1
01/09/2003	Added Set Spin Point Button to GUI and now keep spin point constant during spin mode otherwise
		Fixed Problems with Spin Mode when looking near up/down
		Pthresh changes with keyboard should now be in sync with GUI
		Added spin mode toggle button for GUI

Version 2.5
01/08/2003	The Default for .xyz/.xyzp files will now be single precision floating-point
		Re-Added Data Dump Option
		Added Jump To Position Button in GUI
		Fixed Jump To Data Option with .xyzp files

Version 2.4.7
01/02/2003	Added Option to lock camera to sensor position in .3dp sequence
		Added Option to make colormap absolute throughout sequence
12/31/2002  reduced memory requirements by changing colormap data type
		fixed big endian read data issue
		fixed issue with reading .3dp files that are less than 2KiB*

Version 2.4.6
12/03/2002	.3dp files now use sensor position as default camera location
		fixed a bug in one of the translation controls
		dynamic scenes now autoplay (reduces initial render time)
11/22/2002	Added GLUI Controls
		Many minor fixes
11/08/2002  Can now open multiple files in one view
		Changed order of program arguments
10/20/2002  Significant performance increase (~4x) by removing branches in draw routine
10/16/2002  Source has been completely reorganized, much more C++, some options have been removed (Mostly GLUI stuff)
		Re-implemented spin mode

Version 2.3
09/10/2002  Added measure tool
		GUI Changes
09/09/2002  Added point position query by mouse
		GUI Changes
08/26/2002	Fixed Zero Range colormap problem
		Option to change Up Axis to Z direction
		Added Frames per Second Benchmark

Version 2.2
08/20/2002	Spin Vector can be seen from any y position
		Joe Do's GLUI interface
08/19/2002  Fixed Endian problems
		Added colormap cycle

Version 2.1
08/16/2002  Added support for .3dp file format (types 1 and 1000)
		Changed default to look at data and color axis = y (height)
08/08/2002  Added Look at Data key
08/07/2002	Added Jump Key
		Added ability to type in look vector
		Added data dump
		Changed default to jump and look at data, and ZFAR to 3000
07/27/2002  Added "p" switch (choose p to be anything along data stride)
07/26/2002  Added "p" colormap orientation
		Generalized xyz data file input (float, stride)
		Changed program parameter format
		Added jet colormap interpolation
		Fixed aspect ratio to match window ratio
07/08/2002	Removed "black pixels"
07/03/2002	Added Orthographic View

Version 2.0
06/15/2002	Added ability to type in eye location
06/14/2002	Added ability to type in Field of View, ZMin, ZMax,
		Probability of Detection Threshold
05/10/2002	Added Colormap orientation change
		Added movie restart
04/03/2002	Added movie support for fif files
03/28/2002	Added movie option
03/27/2002	Fixed spin mode problems
03/08/2002	Redo of look up/down using polar coords
03/07/2002	Fixed look up/down bug
03/01/2002	Converted to Binary file format
02/19/2002	Added Force fixed altitude
		Added auto rotate
02/18/2002	Added secondary keys
		Added look up/down
		Added Spin Mode
02/06/2002	Added dynamic probability of detection threshold
02/05/2002

* KiB = KibiBytes or Base-2 KiloBytes (2048 bytes)
