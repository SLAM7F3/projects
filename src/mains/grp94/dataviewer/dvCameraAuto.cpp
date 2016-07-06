/********************************************************************
 *
 *
 * Name: dvCameraAuto.cpp
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * virtual camera for openGL with modifiers
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 *
 *
 **********************************************************************/
#include "dvCameraAuto.h"

dvCameraAuto::dvCameraAuto():dvCamera()
{
   stop();
   movespeed=1.0;
   running=1;
   staticz=false;
}

void dvCameraAuto::step()
{
   if(walking!=0)
   {
      if(staticz) moveForward((float)(5/100.)*
                              walking*running*movespeed);
      else moveWalk((float)(5/100.)*walking*running*movespeed);
      changed=true;
   }
   if(strafing!=0)
   {
      moveStrafe((float)(5/100.)*strafing*running*movespeed);
      changed=true;
   }
   if(flying!=0)
   {
      moveFly((float)(5/100.)*flying*running*movespeed);
      changed=true;
   }
   if(spinning)
   {
      dvCamera::spinMouse(movespeed*spinning*running,0.0f);
      changed=true;
   }
   if(speedmod==-1)
   {
      movespeed*=(float).99;
   }
   else if(speedmod==1)
   {
      movespeed*=(float)1.01;
   }
   if(movespeed <=.0001)
   {
      movespeed=(float).0001;
   }
}

void dvCameraAuto::setSpeed(float s)
{
   movespeed=s;
}

void dvCameraAuto::run(int yes)
{
   if(yes) running=50;
   else running=1;
}

void dvCameraAuto::walk(int dir)
{
   if(walking==0) walking=dir;
   else if((walking*dir) > 0) walking=0;
   else walking=dir;
}

void dvCameraAuto::stop()
{
   walking=strafing=flying=spinning=0;
}

void dvCameraAuto::strafe(int dir)
{
   if(strafing==0) strafing=dir;
   else if((strafing*dir) > 0) strafing=0;
   else strafing=dir;
}

void dvCameraAuto::fly(int dir)
{
   if(flying==0) flying=dir;
   else if((flying*dir) > 0) flying=0;
   else flying=dir;
}

void dvCameraAuto::spin(int dir)
{
   if(spinning==0) spinning=dir;
   else if((spinning*dir) > 0) spinning=0;
   else spinning=dir;
}

void dvCameraAuto::staticZ(bool on)
{
   staticz=on;
}

void dvCameraAuto::toggleStaticZ()
{
   staticz=!staticz;
}

void dvCameraAuto::modSpeed(int mod)
{
   speedmod=mod;
}

float dvCameraAuto::getSpeed()
{
   return movespeed;
}
