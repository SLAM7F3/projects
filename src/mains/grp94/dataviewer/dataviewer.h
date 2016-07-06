/********************************************************************
 *
 *
 * Name: dataviewer.h
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * OpenGL data viewer for xyz data
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 * $Log: dataviewer.h,v $
 * Revision 1.2  2003/05/19 15:25:55  jadams
 * Fixed to now work with gcc3
 *
 * 
 * 38    2/28/03 12:49p Skelly
 * v2.6.2
 * 
 * 37    2/27/03 5:13p Skelly
 * removal of "dual buffer" stuff
 * 
 * 36    2/24/03 4:17p Skelly
 * 
 * 35    2/15/03 4:12p Skelly
 * added full octree support
 * 
 * 34    2/09/03 5:35p Skelly
 * 
 * 33    1/21/03 6:43p Skelly
 * v2.5.2
 * 
 * 31    1/09/03 6:21p Skelly
 * Data Dump
 * Lock View Position
 * Jump To Position
 * v2.5
 * 
 * 30    12/09/02 8:07p Skelly
 * added dataviewer light
 * 
 * 29    12/03/02 3:08p Skelly
 * 2.4.6
 * 
 * 28    11/25/02 11:31a Skelly
 * 
 * 27    11/21/02 7:33p Skelly
 * 
 * 26    11/08/02 1:25p Skelly
 * v 2.4.3
 * 
 * 25    10/20/02 12:26p Skelly
 * 
 * 24    10/18/02 7:42p Skelly
 * v2.4.1
 * 
 * 23    10/14/02 6:20p Skelly
 * v2.4.0
 * Complete Reorganization of code into C++ classes
 * main stuff is still working, but some options disabled
 * 
 * 22    10/12/02 12:17p Skelly
 * 2.3.1
 * 
 * 21    9/10/02 1:22p Skelly
 * measure tool added
 * 
 * 20    9/09/02 6:07p Skelly
 * 
 * 19    8/28/02 8:32p Skelly
 * 
 * 18    8/26/02 11:51a Skelly
 * fixed zero range problem
 * 
 * 17    8/21/02 10:27a Skelly
 * v2.2
 * 
 * 16    8/20/02 5:36p Skelly
 * Joe Do's GLUI
 * 
 * Revision 1.5  2002/07/17 15:49:59  jadams
 * moved 50. --> to 50
 *
 * Revision 1.4  2002/07/11 21:41:45  jadams
 * C++ compatibility changes, mostly explicit casts
 *
 * Revision 1.3  2002/06/28 18:42:15  jadams
 * *** empty log message ***
 *
 * Revision 1.2  2002/06/25 18:40:42  jadams
 * added help with the h key
 *
 * Revision 1.1  2002/06/25 18:12:48  jadams
 * *** empty log message ***
 *
 * Revision 1.3  2002/06/25 18:00:18  jadams
 * Incorporated all of lukes new changes to make version 2
 *
 * have a good .pr file for tmake (we should be able to cross compile!)
 *
 * Revision 1.2  2002/04/03 17:46:59  jadams
 * added colormap functions so you no longger need to read the colormap file
 *
 * Revision 1.1.1.1  2002/04/01 20:21:37  jadams
 * Luke's kickin' 3D data viewer
 *
 *
 **********************************************************************/

#ifndef DATAVIEWER
#define DATAVIEWER

#define OCTREE

#define GLUI_CONTROLS
#ifdef DATAVIEWER_JIGSAW
#undef GLUI_CONTROLS
#define DATAVIEWER_LIGHT
#define DATAVIEWER_VERSION "2.6.3"
#define JIGSAW_CONSTRAIN_MAX_LOW 10.0
#define JIGSAW_CONSTRAIN_MAX_HIGH 30.0
#define JIGSAW_DEFAULT_X 20.0
#define JIGSAW_DEFAULT_Y 20.0
#define JIGSAW_DEFAULT_Z 40.0
#elif defined(DATAVIEWER_LIGHT)
#undef GLUI_CONTROLS
#define DATAVIEWER_VERSION "2.6.3 LE"
#else
#define DATAVIEWER_VERSION "2.6.3"
#endif

#include "arch_dependent_headers.h"
//#include "usage.h"
#include "dvDataSequence.h"
#include "dvDataTreeNode.h"
#include "Frustum.h"
#include "dvDataPointsViewProperties.h"

// INCLUDE GLUT LIB
#ifdef __APPLE__
	#include <GLUT/glut.h>
#elif defined _WIN32 || WIN32
	#include <glut.h>
#else
	#include <GL/glut.h>
#endif
//#include <GL/glut.h>
/* this define is to get rid of compiler warnings about  unused paramters*/

void JumpToData(void);
void LookToData(void);

void initDataViewer(); 
void setpoints(dvDataSequence * dp,dvDataPointsViewProperties * vp);                     

//void setUpDirection(int v);

/* prints help */
//void print_help(void);


void startDataViewer();
#endif


