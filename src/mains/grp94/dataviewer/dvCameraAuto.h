/********************************************************************
 *
 *
 * Name: dvCameraAuto.h
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
#ifndef DVCAMERAAUTO
#define DVCAMERAAUTO
#include "dvCamera.h"
class dvCameraAuto:public dvCamera{
public:
	dvCameraAuto();
	void step();
	void setSpeed(float s);
	void run(int yes);
	void walk(int dir);
	void stop();
	void strafe(int dir);
	void fly(int dir);
	void spin(int dir);
	void modSpeed(int mod);
	float getSpeed();
	void staticZ(bool on);
	void toggleStaticZ();
protected:
	int spinning,walking,running,flying,strafing;
	float movespeed;
	int speedmod;
	bool staticz;
};
#endif
