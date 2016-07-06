// ==========================================================================
// Last updated on 6/6/04
// ==========================================================================

/********************************************************************
 *
 *
 * Name: dataviewer.c
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * OpenGL data viewer for xyz data
 * uses GLUT which can be found in the Mesa-devel package
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 * $Log: dataviewer.cpp,v $
 * Revision 1.2  2003/05/19 15:25:55  jadams
 * Fixed to now work with gcc3
 *
 * 
 * 82    2/28/03 12:49p Skelly
 * min_draw_rate default changed to 10
 * 
 * 81    2/27/03 5:13p Skelly
 * removal of "dual buffer" stuff
 * 
 * 80    2/25/03 3:03p Skelly
 * double zbuffer continued redraw support
 * 
 * 79    2/24/03 4:17p Skelly
 * 
 * 78    2/23/03 4:50p Skelly
 * cross_platform stuff
 * 
 * 77    2/23/03 2:53p Skelly
 * readded static Z option
 * 
 * 76    2/20/03 6:34p Skelly
 * 
 * 75    2/20/03 3:28p Skelly
 * octree support with double buffer
 * 
 * 74    2/16/03 8:29a Skelly
 * now continues drawing with double buffer thanks to Octree
 * 
 * 73    2/15/03 4:12p Skelly
 * added full octree support
 * 
 * 72    2/09/03 5:35p Skelly
 * 
 * 71    1/21/03 6:43p Skelly
 * re-added zmin zfar input
 * 
 * 70    1/09/03 7:12p Skelly
 * Added Spin Mode Toggle Button
 * 
 * 69    1/09/03 7:05p Skelly
 * Added set spin point button (GLUI)
 * 
 * 68    1/09/03 6:21p Skelly
 * Data Dump
 * Lock View Position
 * Jump To Position
 * v2.5
 * 
 * 67    12/09/02 8:07p Skelly
 * added dataviewer light
 * 
 * 66    12/03/02 3:08p Skelly
 * 2.4.6
 * 
 * 65    11/25/02 11:31a Skelly
 * 61    11/08/02 1:25p Skelly
 * v 2.4.3
 * 
 * 59    10/18/02 7:42p Skelly
 * v2.4.1
 * 
 * 58    10/16/02 1:32p Skelly
 * Re-implemented spin mode
 * 
 * 57    10/14/02 6:20p Skelly
 * v2.4.0
 * Complete Reorganization of code into C++ classes
 * main stuff is still working, but some options disabled
 * 
 * 56    10/12/02 12:17p Skelly
 * 2.3.1
 * 
 * 50    9/10/02 1:22p Skelly
 * measure tool added
 * 
 * 49    9/09/02 7:23p Skelly
 * added position query by mouse
 * 
 * 46    8/26/02 11:51a Skelly
 * fixed zero range problem
 * 
 * 43    8/21/02 10:27a Skelly
 * v2.2
 * 
 * 42    8/20/02 5:36p Skelly
 * Joe Do's GLUI
 * 
 * Revision 1.9  2002/08/08  lskelly
 * Added look at data key
 *
 * Revision 1.8  2002/08/07  lskelly
 * Added jump key
 * Changed some defaults
 *
 * Revision 1.7  2002/07/26  lskelly
 * Generalized xyzp format
 * Added colormap scale based on "p"
 *
 * Revision 1.6  2002/07/25  lskelly
 * Removed "black pixels"
 * Added Orthographic projection
 *
 * Revision 1.5  2002/07/17 15:49:58  jadams
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
#include "arch_dependent_headers.h"
#include <ctime>
#include "dataviewer.h"
#include <cmath>
#ifdef GLUI_CONTROLS
#include <glui.h>
#endif
#include "base_file.h"
#include "dvCameraAuto.h"
#include "dvDataSequence.h"
#include "dvColormap.h"
#include "dvDataPointsViewProperties.h"

#ifdef WIN32 || _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // for Sleep()
#define usleep(x) (Sleep(x/1000)) // Sleep() takes milliseconds
#else 
#include <unistd.h>
#endif


dvCameraAuto camera;
dvDataConstraint data_constraint;
dvDataPointsViewProperties* datapointsprop;
dvDataSequence* datapoints;
dvColormap cmap;
int buffer_number=0;
int buffer_count=0;
const int NUMBER_OF_BUFFERS=1;
#define BUFSIZE 256
int spinning=0,spin=0;
int rotating;
int maxcmchange,mincmchange;
int pthreshchange;
int lastx, lasty;
int upflip;
GLdouble bodyWidth;
int newModel;
int scaling;
double *pvertices;
int nframes;
int *pointsperframe;
time_t last_time_t;
float max_draws_per_second=30;
float min_draws_per_second=10;
int main_window;
int gltime=clock();
int printglfps=0;
int glframes=0;
int updir=2; // default z is up
int lock_sensor_view=0;
int lock_sensor_position=0;

int measure=0;
double measurex,measurey,measurez;
#ifdef GLUI_CONTROLS
//*******************GLUI live variables
//joe do 08/20/2002
float view_rotate[16]={1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
float obj_pos[]={0.0,0.0,0.0};
float prev_obj_pos[]={0.0,0.0,0.0};
float obj_rot[]={0.0,0.0,0.0};
float xy_aspect;
float thetarad,phirad;
int counter=0;
int autorefresh=0;
int glui_query_position=0;
int glui_query_distance=0;
int glui_jump_to_point=0;
int glui_set_spin_point=0;
GLUI_Panel* glui_extra_panel;
GLUI_StaticText* glui_extra_text;

// glui_live_variables
float fov=40.;
float pthresh=0.0;
float eye[3]={0,0,0};
float theta=0.0,phi=0.;
float anglerad[2]={0,0};
float speedmod=0.0;
float fps=4;

GLUI *glui;
//ids for GLUI callback function
const int PLUS_ID=0;
const int MINUS_ID=1;
//const int APTHRESHCHANGE_ID=2;
//const int QPTHRESHCHANGE_ID=3;

const int UPFLIP_ID=6;
const int SPIN_ID=7;
const int SPINVECTORSHOW_ID=8;
const int ORIGIN_ID=9;
const int SNAPAXIS_ID=10;
const int TILDE_ID=11;
const int JUMP_ID=16;
const int LOOK_AT_DATA_ID=18;
const int CHANGE_COLOR_AXIS_ID=19;
const int NEXT_COLORMAP_ID=20;

const int FWALKING_ID=100;
const int BWALKING_ID=101;
const int FPSCHANGE999_ID=102;
const int FPSCHANGE1001_ID=103;
const int T_ID=104;
const int R_ID=105;
const int SPEEDMODCHANGEMINUS_ID=106;

const int SETPERSPECTIVE_ID=200;
const int FOV_ID=201;
const int ZNEAR_ID=202;
const int ZFAR_ID=203;

const int SPEEDMODCHANGEPLUS_ID=301;
const int STRAFELEFT_ID=302;
const int STRAFERIGHT_ID=303;
const int STRAFEUP_ID=304;
const int STRAFEDOWN_ID=305;

const int XYTRANS_ID=400;
const int HIDE_ID=401;
const int ZTRANS_ID=402;
const int SPEEDMOD_ID=403;
const int REFRESH_ID=404;
const int PTHRESH_ID=405;
const int MOVIE_PLAY_ID=406;
const int MOVIE_STOP_ID=407;
const int MOVIE_PAUSE_ID=408;
const int MOVIE_RESTART_ID=409;
const int FPS_ID=410;
const int QUERY_POSITION_ID=411;
const int QUERY_DISTANCE_ID=412;
const int JUMP_TO_POSITION_ID=413;
const int SET_SPIN_POINT_ID=414;
char IDMAP[]={'+','-','a','q','f','b','/','s','z','o','x','`',',','.',
              't','r','j',127,'k','c','n'};
char IDMAPHOLD[]={'f','b',',','.','t','r',127};
int IDMAPHOLDSTATUS[100];
int IDMAPSPEC[]={GLUT_KEY_F12,GLUT_KEY_F7,GLUT_KEY_F2,GLUT_KEY_F3,GLUT_KEY_F4};
int IDMAPSPECHOLD[]={'!',GLUT_KEY_INSERT,GLUT_KEY_LEFT,GLUT_KEY_RIGHT,GLUT_KEY_UP,GLUT_KEY_DOWN};
int IDMAPSPECHOLDSTATUS[100];
//***********end of GLUI variables
#endif //GLUI_CONTROLS
/*//////////////////////// setIdleFunc \\\\\\\\\\\\\\\\\\\\\\\
/////			wrapper setIdleFunc
////////////////////////// setIdleFunc \\\\\\\\\\\\\\\\\\\\\\\*/

void setIdleFunc(void (*func)(void))
{
#ifdef GLUI_CONTROLS
   GLUI_Master.set_glutIdleFunc(func);
#else
   glutIdleFunc(func);
#endif
}

/*
///////////////////////////////// DRAW MESH \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
/////
/////	Does all the drawing...
/////	Draws the points and spin vector
/////
///////////////////////////////// DRAW MESH \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
*/

void drawmesh(void)
{
   GLint v;
   glGetIntegerv(GL_DRAW_BUFFER,&v);
   //printf("Layer = %d\n",wglGetCurrentDC());
   time_t now_t=clock();
   float timediff=(now_t-last_time_t)/(float)CLOCKS_PER_SEC;
   float timeremain=1/(float)max_draws_per_second-timediff;
   if (timeremain>0)
   {
      usleep((unsigned long)(timeremain*1000000));
   }

   glLoadIdentity();
   camera.glLook();
   bool camchange=camera.isChanged();
   bool dchange=datapoints->isChanged() || datapointsprop->isChanged(0);
   bool clear=(camchange || dchange);
   if (clear)
   {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   }
   else
   {
   }
   CFrustum f;
   f.CalculateFrustum();
   vector<float> camloc(3);
   camera.getPosition(camloc);

   if (min_draws_per_second==0) datapoints->glDraw(camloc,clear,0,&f);
   else datapoints->glDraw(
      camloc,clear,clock()+CLOCKS_PER_SEC/min_draws_per_second,&f);

   camera.glSwapColorBuffers();
	
   if (++glframes>=30){
      if (printglfps){
         printf("%f\r",30/(float)(clock()-gltime)*(float)CLOCKS_PER_SEC);
         /*	 If the stream argument is NULL, fflush flushes 
                 all open output streams.	
                 -JSA */

         fflush(stdout);
      }
      gltime=clock();
      glframes=0;
   }

#ifdef GLUI_CONTROLS
   if (autorefresh){
      glui->sync_live();
   }
#endif
   if (lock_sensor_position)
   {
      float lpos[3];
      camera.getSpinPoint(lpos[0],lpos[1],lpos[2]);
      JumpToData();
      camera.lookAt(lpos[0],lpos[1],lpos[2]);
   }
   if (lock_sensor_view)
   {
      JumpToData();
      LookToData();
   }
   last_time_t=clock();
}

void setPerspective()
{
   camera.glSetPerspective();
}
/*
///////////////////////////////// RESHAPE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
/////
/////	Takes care of any window reshaping
/////
///////////////////////////////// RESHAPE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
*/

void myReshape(int w, int h)
{
   camera.glReshape(w,h);
}

/*
///////////////////////////////// getWorldCoord \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
/////
/////	Inputs: mx, my (mouse coordinates from screen)
/////	Outputs:	wx, wy, wz (real world coordinates)
/////
///////////////////////////////// MOUSE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
*/
bool getWorldCoord(double mx, double my, double & wx, double & wy, double & wz){
   return camera.glGetWorldCoord(mx,my,wx,wy,wz);
/*	GLfloat z;
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint realy;  //  OpenGL y coordinate position  
	//GLdouble wx, wy, wz;  //  returned world x, y, z coords  
	glGetIntegerv (GL_VIEWPORT, viewport);
        glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
        glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
	//  note viewport[3] is height of window in pixels  
        realy = viewport[3] - (GLint) my - 1;
	glReadPixels(mx, realy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT,&z);
	if (z<1.0){
        gluUnProject ((GLdouble) mx, (GLdouble) realy, (GLdouble)z,
        mvmatrix, projmatrix, viewport, wx, wy, wz);
        return true;
	}
	else{
        return false;
	}*/
}

void checkRun()
{
   if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
   {
      camera.run(1);
      data_constraint.fastColor(1);
   }
   else
   {
      camera.run(0);
      data_constraint.fastColor(0);
   }
}

/*

///////////////////////////////// MOUSE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
/////
/////	Takes care of mouse buttons
/////	walking and looking
/////
///////////////////////////////// MOUSE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
*/

void mouse(int button, int state, int x, int y)
{
#ifdef GLUI_CONTROLS
   if (glui_query_position)
   {
      double wx,wy,wz;
      getWorldCoord(x,y,wx,wy,wz);
      glui_extra_panel->set_name("Query Position");
      char t[1024];
      sprintf((char*)&t,"%f %f %f",wx,wy,wz);
      glui_extra_text->set_text(t);
      glui_query_position=0;
      return;
   }
   if (glui_query_distance)
   {
      double wx,wy,wz;
      getWorldCoord(x,y,wx,wy,wz);
      if (glui_query_distance==1)
      {
         measurex=wx;
         measurey=wy;
         measurez=wz;
         glui_query_distance=2;
         rotating=0;
      }
      else if (state==GLUT_UP)
      {
         glui_extra_panel->set_name("Measure");
         char t[1024];
         sprintf((char*)&t,"%f %f %f",wx-measurex,wy-measurey,wz-measurez);
         glui_extra_text->set_text(t);
         glui_query_distance=0;
      }
      rotating=spinning=0;
      return;

   }
   if (glui_jump_to_point)
   {
      double wx,wy,wz;
      bool ret;
      ret=getWorldCoord(x,y,wx,wy,wz);
      glui_extra_panel->set_name("Jump To Position");
      char t[1024];
		
      glui_jump_to_point=0;
      if (ret)
      {
         camera.moveAbsolute((float)wx,(float)wy,(float)wz);
         sprintf((char*)&t,"%f %f %f",wx,wy,wz);
         glui_extra_text->set_text(t);
      }
      else
      {
         float fx,fy,fz;
         datapoints->getSuggestedCameraLocationPoint(fx,fy,fz);
         sprintf((char*)&t,"%f %f %f",fx,fy,fz);
         glui_extra_text->set_text(t);
         camera.moveAbsolute(fx,fy,fz);
      }
		
      return;
   }
   if (glui_set_spin_point)
   {
      double wx,wy,wz;
      bool ret=getWorldCoord(x,y,wx,wy,wz);
      if (ret)
      {
         camera.setSpinPoint(wx,wy,wz);
      }
      else
      {
         float fx,fy,fz;
         datapoints->getSuggestedCameraLookPoint(fx,fy,fz);
         camera.setSpinPoint(fx,fy,fz);
      }
      glui_set_spin_point=0;
   }
#endif // GLUI_CONTROLS
   if (button == GLUT_LEFT_BUTTON)
   {
      if (state==GLUT_DOWN){
         if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
         {
            double wx,wy,wz;
            bool ret=getWorldCoord(x,y,wx,wy,wz);
            if (ret)
            {
               printf ("World coords (%f, %f, %f)\n",wx, wy, wz);
            }
            rotating=0;
         }
         else if (glutGetModifiers() == GLUT_ACTIVE_ALT)
         {
            bool ret=getWorldCoord(x,y,measurex,measurey,measurez);
            if (ret) measure=1;
            else if (measure==1){
               measure=0;
               double wx, wy, wz;
               ret=getWorldCoord(x,y,wx,wy,wz);
               if (ret){
                  wx-=measurex;
                  wy-=measurey;
                  wz-=measurez;
                  printf ("Difference (%f, %f, %f)\nDistance = %f\n",wx, wy, wz, sqrt(wx*wx+wy*wy+wz*wz));
               }
            }
            rotating=0;
         }
         else{
            if (spin)
            {
               rotating=0;
               spinning=1;
            }
            else rotating=1;
            lastx=x;
            lasty=y;
         }
			
      }
      else
      {
         rotating=spinning=0;
         if (glutGetModifiers() == GLUT_ACTIVE_ALT && measure==1){
            double wx, wy, wz;
            bool ret=getWorldCoord(x,y,wx,wy,wz);
            if (ret){
               wx-=measurex;
               wy-=measurey;
               wz-=measurez;
               printf ("Difference (%f, %f, %f)\nDistance = %f\n",wx, wy, wz, sqrt(wx*wx+wy*wy+wz*wz));
            }
         }
      }

   }
   if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
   {
      camera.walk(1);
   }
   if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
   {
      camera.walk(0);
   }
   if (button == GLUT_MIDDLE_BUTTON && state == GLUT_UP)
   {
      camera.walk(0);
   }
   if (button == GLUT_MIDDLE_BUTTON && state==GLUT_DOWN)
   {
      camera.walk(-1);
   }
   checkRun();
}

/*
///////////////////////////////// keyboardspec \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
/////
/////	Takes care of any special keyboard events
/////
///////////////////////////////// keyboardspec \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
*/

void keyboardspec(int key, int x, int y)
{
   float f;
   float nx,ny,nz;
   int r;

   if (key==GLUT_KEY_LEFT) camera.strafe(1);           
   else if (key==GLUT_KEY_RIGHT) camera.strafe(-1);    
   else if (key==GLUT_KEY_UP) camera.fly(1);           
   else if (key==GLUT_KEY_DOWN) camera.fly(-1);    
   else if (key==GLUT_KEY_PAGE_UP)
   {
      maxcmchange=1;
   }
   else if (key==GLUT_KEY_PAGE_DOWN)
   {
      maxcmchange=-1;
   }
   else if (key==GLUT_KEY_HOME)
   {
      mincmchange=1;
   }
   else if (key==GLUT_KEY_END)
   {
      mincmchange=-1;
   }
   else if (key==GLUT_KEY_INSERT)
   {
      camera.modSpeed(1);
   }
   else if (key==GLUT_KEY_F1)
   {
      printf("Probability of Detection Threshold: ");
      r=scanf("%f",&f);
      if (r==1)
      {
         //datapoints->setPoffsetConstrainMin(f);
         data_constraint.setMin(3,f);
      }
#ifdef GLUI_CONTROLS
      pthresh=f;
      glui->sync_live();
#endif
   }
   else if (key==GLUT_KEY_F2)
   {
      printf("FOV: ");
      r=scanf("%f",&f);
      if (r==1)
      {
         camera.setFOV(f);
         printf("FOV=%lf\n",f);
      }
#ifdef GLUI_CONTROLS
      fov=f;
      glui->sync_live();
#endif
   }
   else if (key==GLUT_KEY_F4)
   {
      printf("ZFAR: ");
      r=scanf("%f",&f);
      if (r==1)
      {
         camera.setZFar(f);
      }
   }
   else if (key==GLUT_KEY_F3)
   {
      printf("ZNEAR: ");
      r=scanf("%f",&f);
      if (r==1)
      {
         camera.setZNear(f);
      }
   }
   else if (key==GLUT_KEY_F5)
   {
      printf("Camera Location \"x y z\": ");
      r=scanf("%f %f %f",&nx,&ny,&nz);
      if (r==3)
      {
         camera.moveAbsolute(nx,ny,nz);
      }
   }
   else if (key==GLUT_KEY_F6)
   {
      printf("Look At \"x y z\": ");
      r=scanf("%f %f %f",&nx,&ny,&nz);
      if (r==3)
      {
         camera.lookAt(nx,ny,nz);
      }
   }

   else if (key==GLUT_KEY_F7){
      printf("FPS : ");
      r=scanf("%f",&nx);
      if (r==1)
      {
         datapoints->setFPS(nx);
      }
#ifdef GLUI_CONTROLS
      fps=nx;
      glui->sync_live();
#endif
   }
   else if (key==GLUT_KEY_F8){
      printf("MINIMUM_REFRESH_RATE: ");
      r=scanf("%f",&nx);
      if (r==1)
      {
         min_draws_per_second=nx;
      }
   }
   else if (key==GLUT_KEY_F11)
   {
      printf("ORTHO_SIZE w h: ");
      float width,height;
      r=scanf("%f %f",&width,&height);
      if (r==2)
      {
         camera.setOrthographicDimensions(width,height);
      }
   }
   else if (key==GLUT_KEY_F12)
   {
      camera.togglePerspective();
   }
   checkRun();
   glutPostRedisplay();
}


/*
///////////////////////////////// keyboardspecup \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
/////
/////	Takes care of any special keyboard up events (special keys are released)
/////
///////////////////////////////// keyboardspecup \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
*/

void keyboardspecup(int key, int x, int y)
{
   if (key==GLUT_KEY_LEFT || key==GLUT_KEY_RIGHT) camera.strafe(0);
   else if (key==GLUT_KEY_UP || key==GLUT_KEY_DOWN) camera.fly(0);
   else if (key==GLUT_KEY_HOME || key==GLUT_KEY_END) mincmchange=0;
   else if (key==GLUT_KEY_PAGE_UP || key==GLUT_KEY_PAGE_DOWN) maxcmchange=0;
   else if (key==GLUT_KEY_INSERT)
   {
      camera.modSpeed(0);
#ifdef GLUI_CONTROLS
      speedmod=camera.getSpeed();
      glui->sync_live();
#endif
   }
}
/*
///////////////////////////////// keyboard \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
/////
/////	Takes care of any keyboard events
/////
///////////////////////////////// keyboard \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
*/

void keyboard(unsigned char key, int x, int y)
{
   if (key=='+' || key=='=') datapointsprop->modPointSize(1);
   if (key=='-' || key=='_') datapointsprop->modPointSize(-1);    
   // 127=DELETE KEY
   if (key==127)
   {
      camera.modSpeed(-1);
   }
   else if (key=='d' || key=='D'){
      printf("Enter filename base: ");
      char filename[1024];
      scanf("%s",filename);
      datapoints->data_dump_sequence(filename);
   }
   else if (key=='i' || key=='I') camera.modSpeed(-1);
   else if (key=='[' || key=='{' || key=='a') pthreshchange=-1;
   else if (key==']' || key=='}') pthreshchange=1;
   else if (key=='f' || key=='F') camera.walk(1);
   else if (key=='b' || key=='B') camera.walk(-1);
   //   else if (key=='/' || key=='?') upflip*=upflip;
   else if (key=='s' || key=='S') spin=~spin;
   else if (key=='o' || key=='O') camera.moveAbsolute(0.0,0.0,0.0);
   else if (key=='c') datapoints->modColorCode(1);
   else if (key=='C') datapoints->modColorCode(-1);

   else if (key=='`') datapoints->setFrame(0);

   else if (key=='t' || key=='T') camera.spin(1);
   else if (key=='r' || key=='R') camera.spin(-1);
   else if (key=='5')
   {
      camera.spin(1);
      camera.walk(1);
      camera.setSpeed(0.5);
   }
   else if (key=='x') camera.toggleStaticZ();
   else if (key=='l')
   {
      float look[3];
      camera.getLook(look);
      float angles[2];
      camera.getLookAngles(angles);
      printf("look=%lf %lf %lf\ntheta=%lf (%lf)\nphi=%lf(%lf)\n",
             look[0],look[1],look[2],angles[0],angles[0]/3.14*180.,
             angles[0],angles[1]);
   }
//    else if (key=='h') print_help();
   else if (key=='e')
   {
      float eye[3];
      camera.getPosition((float*)&eye);
      printf("position=%lf %lf %lf\nrange=%lf\n",
             eye[0],eye[1],eye[2],sqrt(pow(eye[0],2)+pow(eye[1],2)
                                       +pow(eye[2],2)));
   }
   else if (key=='p') printf("Probability threshold = %E\n",
                            data_constraint.getMin(3));
//    else if (key==',') fpschange=.999;
//    else if (key=='.') fpschange=1.001;
   else if (key=='j') JumpToData();
   else if (key=='k') LookToData();
   else if (key=='n')
   {
      cmap.nextColormap();
      datapoints->setColormap(&cmap);  // this is a crappy fix
   }
   else if (key=='N')
   {
      cmap.prevColormap();
      datapoints->setColormap(&cmap);  // this is a crappy fix

   }
   else if (key=='~') printglfps=~printglfps;

   else if (key=='1') datapoints->modFrame(1);
   else if (key=='2') datapoints->modFrame(-1);
   else if (key=='3') lock_sensor_view=~lock_sensor_view;
   else if (key=='4') lock_sensor_position=~lock_sensor_position;
   else if (key=='q' || key=='Q') exit(-1);	// Quit dataviewer program!

   checkRun();
   glutPostRedisplay();
}


/*
///////////////////////////////// keyboardup \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
/////
/////	Takes care of any keyboard up events (keys are released)
/////
///////////////////////////////// keyboardup \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
*/

void keyboardup(unsigned char key,int x, int y)
{
   if (key==127)
   {
      camera.modSpeed(0);
#ifdef GLUI_CONTROLS
      speedmod=camera.getSpeed();
      glui->sync_live();
#endif
   }
   else if (key=='b' || key=='B' || key=='f'|| key=='F') camera.walk(0);
   else if (key=='i' || key=='I' || key=='d' || key=='D') camera.modSpeed(0);
   else if (key=='r' || key=='R' || key=='t' || key=='T') camera.spin(0);
   else if (key=='[' || key=='{' || key=='a' || key==']' || key=='}' 
            || key=='q' || key=='Q') pthreshchange=0;
//    else if (key==',' || key=='.') fpschange=1.0;
}


/*
///////////////////////////////// MOTION \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
/////
/////	Takes care of any mouse movement events.  Updates look vector
/////
///////////////////////////////// MOTION \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
*/

void motion(int x, int y)
{
   if (lastx==x && lasty==y) return;
   if (rotating==1)
   {
      camera.lookRelativeMouse(-((float)(x-lastx)),-((float)(y-lasty)));
        
   }
   if (spinning)
   {
      camera.spinMouse(-((float)(x-lastx)),-((float)(y-lasty)));
   }
   lastx=x;
   lasty=y;
   glutPostRedisplay();
}


/*
///////////////////////////////// CHECK MOVE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
/////
/////	This is the function that is called when idle.  Updates any change in vectors
/////
///////////////////////////////// CHECK MOVE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
*/

void checkmove(void)
{
//	printf("checkmove\n");
#ifdef GLUI_CONTROLS
   //************************************************
   //******************joe do 08/20/2002
   //sync up GLUI variables with display
   glutSetWindow(main_window);	
   //**********************************
#endif // GLUI_CONTROLS
   camera.step();
   /* if (nframes>1)
      {
      fps*=fpschange;
      glutPostRedisplay();
      }*/
   if (maxcmchange!=0)
   {
      data_constraint.modMax(datapoints->getColorCode(),maxcmchange);
      glutPostRedisplay();
   }
   if (mincmchange!=0)
   {
      data_constraint.modMin(datapoints->getColorCode(),mincmchange);
      glutPostRedisplay();
   }
   if (pthreshchange!=0)
   {
      data_constraint.modMin(3,pthreshchange);
#ifdef GLUI_CONTROLS
      pthresh=data_constraint.getMin(3);
      glui->sync_live();
#endif
      glutPostRedisplay();
   }
/*	if (fpschange!=0)
	{
		datapoints->modFPS(fpschange);
	}*/
#ifdef GLUI_CONTROLS
   camera.getPosition((float*)&eye);
   camera.getLookAngles((float*)&anglerad);
   theta=anglerad[0]/PI*180.;
   phi=anglerad[1]/PI*180.;
#endif
   glutPostRedisplay();
}


#ifdef GLUI_CONTROLS
/*
///////////////////////////////// CONTROL_CB \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
/////
/////	Takes care of any buttons from GLUI.
/////	Control callback function performs action when a control is
/////	activated.  This code does the same things as the keyboard code.
/////
///////////////////////////////// CONTROL_CB \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
*/

void control_cb(int control)
{
   switch(control)
   {
      case XYTRANS_ID:
         camera.moveStrafe(prev_obj_pos[0]-obj_pos[0]);
         camera.moveForward(obj_pos[1]-prev_obj_pos[1]);
         prev_obj_pos[0]=obj_pos[0];
         prev_obj_pos[1]=obj_pos[1];
         break;
      case ZTRANS_ID:
         camera.moveRelative(0.0f,0.0f,obj_pos[2]-prev_obj_pos[2]);
         prev_obj_pos[2]=obj_pos[2];
         break;
      case HIDE_ID:
         glui->hide();
         setIdleFunc(checkmove);
         break;
      case REFRESH_ID:
         glui->sync_live();
         break;
      case FOV_ID:
         camera.setFOV(fov);
         break;
      case PTHRESH_ID:
         data_constraint.setMin(3,pthresh);
         break;
      case SPEEDMOD_ID:
         camera.setSpeed(speedmod);
         break;

      case MOVIE_PLAY_ID:
         datapoints->setFPS(fps);
         break;
      case MOVIE_PAUSE_ID:
         datapoints->setFPS(-fps);
         break;
      case MOVIE_STOP_ID:
         datapoints->setFPS(0);
         break;
      case MOVIE_RESTART_ID:
         datapoints->setFrame(0);
         break;
      case FPS_ID:
         datapoints->setFPS(fps);
         break;

      case QUERY_POSITION_ID:
         glui_query_position=1;
         break;
      case QUERY_DISTANCE_ID:
         glui_query_distance=1;
         break;
      case JUMP_TO_POSITION_ID:
         glui_jump_to_point=1;
         break;
      case SET_SPIN_POINT_ID:
         glui_set_spin_point=1;
         break;
      default:
         if (control<100) keyboard(IDMAP[control],0,0);
         else if (control<200){
            int offset=100;
            if (IDMAPHOLDSTATUS[control-offset]==0) keyboard(IDMAPHOLD[control-offset],0,0);
            else keyboardup(IDMAPHOLD[control-offset],0,0);
            IDMAPHOLDSTATUS[control-offset]=~IDMAPHOLDSTATUS[control-offset];
         }
         else if (control<300) keyboardspec(IDMAPSPEC[control-300],0,0);
         else if (control<400){
            int offset=300;
            if (IDMAPSPECHOLDSTATUS[control-offset]==0)	keyboardspec(IDMAPSPECHOLD[control-offset],0,0);
            else keyboardspecup(IDMAPSPECHOLD[control-offset],0,0);
            IDMAPSPECHOLDSTATUS[control-offset]=~IDMAPSPECHOLDSTATUS[control-offset];
         }
   }
}
//end control_cb
#endif //GLUI_CONTROLS


/*
///////////////////////////////// SETPOINTS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
/////
/////	changes points to draw to xyzp array
/////
///////////////////////////////// SETPOINTS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
*/

void setpoints(dvDataSequence * dp,dvDataPointsViewProperties * vp)
{
   datapoints=dp;
   datapointsprop=vp;
   datapoints->setColormap(&cmap);
   datapoints->setConstraint(&data_constraint);
   datapoints->resetConstraint();
   datapoints->setColorCode(2);
}

void JumpToData(void)
{
   float x,y,z;
   datapoints->getSuggestedCameraLocationPoint(x,y,z);
   camera.moveAbsolute(x,y,z);
}

void LookToData(void)
{
   float x,y,z;
   datapoints->getSuggestedCameraLookPoint(x,y,z);
   camera.lookAt(x,y,z);
}

/*
///////////////////////////////// INIT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
/////
/////	Initializes viewer to:
/////		draw nothing
/////
///////////////////////////////// INIT \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
*/

void initDataViewer()
{
//    fps=20;
   rotating=0;
   upflip=1;
   bodyWidth = 3.0;
   newModel = 1;
   glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
//	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
   main_window=glutCreateWindow("3D Data Viewer");
   glutIgnoreKeyRepeat(1);
   glutReshapeFunc(myReshape);
   glutSpecialFunc(keyboardspec);
   glutSpecialUpFunc(keyboardspecup);
   glutKeyboardFunc(keyboard);
   glutMouseFunc(mouse);
   glutKeyboardUpFunc(keyboardup);
   glutMotionFunc(motion);
   setIdleFunc(checkmove);
   glutDisplayFunc(drawmesh);
   setPerspective();
   camera.moveAbsolute(0.0,0.0,0.0);
   camera.lookAbsolute(0.0,3.14/2);
   glEnable(GL_DEPTH_TEST);
   glEnableClientState(GL_COLOR_ARRAY);
   glEnableClientState(GL_VERTEX_ARRAY);
   glLineWidth(5.0);
#ifdef GLUI_CONTROLS
   //************************************************
   //*********************GLUI CODE Joe Do 08/20/2002
   //this code add controls to the GUI
   glui=GLUI_Master.create_glui("GUI Control",0,500,1);
   glui->set_main_gfx_window( main_window );
	
	//for the controls, the parameters are usually name,control variation (if any),live variables (if any),callback_id (if any), and callback_func
   GLUI_Translation *trans_xy=glui->add_translation("XY Translation",GLUI_TRANSLATION_XY,obj_pos,XYTRANS_ID,control_cb);
   trans_xy->set_speed(.5);
   GLUI_Translation *trans_z=glui->add_translation("Z Translation",GLUI_TRANSLATION_Z,obj_pos+2,ZTRANS_ID,control_cb);
   trans_z->set_speed(.5);
   glui->add_button("Spin Mode",SPIN_ID,control_cb);
   //button paramaters are usually name,callback_id,and callback_func
   GLUI_Panel *camera_move_panel = glui->add_panel("Camera Move");
   glui->add_button_to_panel(camera_move_panel,"Strafe Left",STRAFELEFT_ID,control_cb);
   glui->add_button_to_panel(camera_move_panel,"Strafe Right",STRAFERIGHT_ID,control_cb);
   glui->add_button_to_panel(camera_move_panel,"Strafe Up",STRAFEUP_ID,control_cb);
   glui->add_button_to_panel(camera_move_panel,"Strafe Down",STRAFEDOWN_ID,control_cb);
   glui->add_button_to_panel(camera_move_panel,"Walk Forward",FWALKING_ID,control_cb);
   glui->add_button_to_panel(camera_move_panel,"Walk Backward",BWALKING_ID,control_cb);
   glui->add_button_to_panel(camera_move_panel,"Rotate Left",T_ID,control_cb);
   glui->add_button_to_panel(camera_move_panel,"Rotate Right",R_ID,control_cb);

   speedmod=camera.getSpeed();
   GLUI_Spinner *speed_spinner = glui->add_spinner("Speed:", GLUI_SPINNER_FLOAT, &speedmod,SPEEDMOD_ID,control_cb );
	

   glui->add_column(true);
   GLUI_Panel *camera_options_panel = glui->add_panel("Camera Options");
   glui->add_button_to_panel(camera_options_panel,"Jump To Origin",ORIGIN_ID,control_cb);
   glui->add_button_to_panel(camera_options_panel,"Jump To Data",JUMP_ID,control_cb);
   glui->add_button_to_panel(camera_options_panel,"Jump To Position",JUMP_TO_POSITION_ID,control_cb);
   glui->add_button_to_panel(camera_options_panel,"Look At Data",LOOK_AT_DATA_ID,control_cb);
   glui->add_button_to_panel(camera_options_panel,"Set Spin Point",SET_SPIN_POINT_ID,control_cb);
   //glui->add_button_to_panel(camera_options_panel,"Lock Y Axis",SNAPAXIS_ID,control_cb);
   glui->add_spinner_to_panel(camera_options_panel,"FOV:", GLUI_EDITTEXT_FLOAT, &fov,FOV_ID,control_cb);

   GLUI_Panel *viewport_options_panel = glui->add_panel("Viewport Options");
   glui->add_button_to_panel(viewport_options_panel,"+ POINT SIZE",PLUS_ID,control_cb);
   glui->add_button_to_panel(viewport_options_panel,"- POINT SIZE",MINUS_ID,control_cb);

   GLUI_Panel *colormap_options_panel = glui->add_panel("Colormap Options");
   glui->add_button_to_panel(colormap_options_panel,"Colormap Direction",CHANGE_COLOR_AXIS_ID,control_cb);
   glui->add_button_to_panel(colormap_options_panel,"Change Colormap",NEXT_COLORMAP_ID,control_cb);

	//glui->add_button("UPFLIP",UPFLIP_ID,control_cb);
   GLUI_Spinner *pthresh_spinner = glui->add_spinner("Pthresh:", GLUI_SPINNER_FLOAT, &pthresh,PTHRESH_ID,control_cb );
   pthresh_spinner->set_speed(.15);

	
   glui->add_button("Hide GUI",HIDE_ID,control_cb);
   glui->add_button("Quit",0,(GLUI_Update_CB)exit);

	//add column seperator 
   glui->add_column(true);

   GLUI_Panel *movie_panel = glui->add_panel("Movie Controls");
   glui->add_button_to_panel(movie_panel,"Play",MOVIE_PLAY_ID,control_cb);
   glui->add_button_to_panel(movie_panel,"Pause",MOVIE_PAUSE_ID,control_cb);
   glui->add_button_to_panel(movie_panel,"Stop",MOVIE_STOP_ID,control_cb);
   glui->add_button_to_panel(movie_panel,"Restart",TILDE_ID,control_cb);
   GLUI_Spinner *fps_spinner=glui->add_spinner_to_panel(movie_panel,"fps",GLUI_SPINNER_FLOAT,&fps,FPS_ID,control_cb);
   fps_spinner->set_float_limits(1.0,100.0);
   //eye values panel
   //add panels and controls into panels
   GLUI_Panel *eye_panel = glui->add_panel("Eye Values" );
   GLUI_EditText *eyex_edittext = glui->add_edittext_to_panel(eye_panel,"EyeX:", GLUI_EDITTEXT_FLOAT, (float*)&eye );
   GLUI_EditText *eyey_edittext = glui->add_edittext_to_panel(eye_panel,"EyeY:", GLUI_EDITTEXT_FLOAT, (float*)&eye+1 );
   GLUI_EditText *eyez_edittext = glui->add_edittext_to_panel(eye_panel,"EyeZ:", GLUI_EDITTEXT_FLOAT, (float*)&eye+2 );
   eyex_edittext->disable();
   eyez_edittext->disable();
   eyey_edittext->disable();

   //up values panel
   //add panels and controls into panels

   //look values
   GLUI_Panel *look_panel = glui->add_panel("Look Values" );
   GLUI_EditText *thetarad_edittext = glui->add_edittext_to_panel(look_panel,"Theta:", GLUI_EDITTEXT_FLOAT, &theta);
   GLUI_EditText *phirad_edittext = glui->add_edittext_to_panel(look_panel,"Phi:", GLUI_EDITTEXT_FLOAT, &phi);	
   thetarad_edittext->disable();
   phirad_edittext->disable(); 
   glui->add_button("REFRESH DISPLAY",REFRESH_ID,control_cb);
   glui->add_checkbox("Auto Refresh",&autorefresh);
   glui->add_button("Query Position",QUERY_POSITION_ID,control_cb);
   glui->add_button("Query Distance",QUERY_DISTANCE_ID,control_cb);
   glui_extra_panel=glui->add_panel("");
   glui_extra_text=glui->add_statictext_to_panel(glui_extra_panel," ");
   glui->add_column(true);
   //***********************************end of GLUI
#endif //GLUI_CONTROLS
}

void startDataViewer()
{
   initDataViewer();
   JumpToData();
   LookToData();
   float fx,fy,fz;
   datapoints->getSuggestedCameraLookPoint(fx,fy,fz);
   camera.setSpinPoint(fx,fy,fz);
   glutMainLoop();
}
