/********************************************************************
 *
 *
 * Name: dvCamera.h
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * virtual camera for openGL
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 *
 *
 **********************************************************************/
#ifndef DVCAMERA
#define DVCAMERA
#include "arch_dependent_headers.h"
#include <vector>
#include <boost/shared_array.hpp>
using namespace std;
using namespace boost;
class dvCamera{
public:
	dvCamera();
	void moveAbsolute(float x, float y, float z);
	void moveRelative(float x, float y, float z);
	void moveWalk(float amount);
	void moveForward(float amount);
	void moveStrafe(float amount);
	void moveFly(float amount);
	void lookAbsolute(float pan, float tilt);
	void lookRelative(float pan, float tilt);
	void lookRelativeMouse(float x, float y);
	void spin(float x, float y);
	void spinMouse(float x, float y);
	void lookAt(float x, float y, float z);

	// Sets
	void setFOV(float f);
	void setOrthographicDimensions(float width, float height);
	void togglePerspective();
	void setSpinPoint(float x, float y, float z);
	void setZNear(float z);
	void setZFar(float z);

	// Gets
	void getPosition(float * p) const;
	void getPosition(vector<float> & p) const;
	void getLook(float * l) const;
	void getLookAt(float * lk) const;
	void getLookAngles(float * a) const;
	void dvCamera::getSpinPoint(float &x, float &y, float &z) const;
	bool isChanged();

	// openGL
	void glLook();
	void glSetPerspective();
	void glReshape(int w, int h);
	void glSwapColorBuffers();
	bool glGetWorldCoord(double mx, double my, double & wx, double & wy, double & wz);
protected:
	void mouseToAngle(float &x, float &y);

	float posx,posy,posz;
	float upx,upy,upz;
	float lookx,looky,lookz;
	float fov;
	float znear,zfar;
	float aspect_ratio;
	float ortho_width,ortho_height;
	float looktheta,lookphi,uptheta,upphi,lookr,upr;
	int upflip;
	float spinx,spiny,spinz;
	long perspective;
	int updir;
	float lookspeed;
	bool changed;
	int W,H;
};
#endif
