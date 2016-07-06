/********************************************************************
 *
 *
 * Name: dvCamera.cpp
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
#include "dvCamera.h"
#include <cmath>



#include <cstdio> // for debugging

dvCamera::dvCamera()
{
	moveAbsolute(0.0,0.0,0.0);
	lookAbsolute(0.0,PI/2);
	fov=40.;
	znear=.1;
	zfar=3000.;
	perspective=~0;
	W=800;
	H=600;
	ortho_width=ortho_height=100;
}
void dvCamera::glSwapColorBuffers()
{
	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_FRONT);
	glDepthMask(GL_FALSE);
	glCopyPixels(0,0,W,H,GL_COLOR);
	glDepthMask(GL_TRUE);
	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_FRONT);
}

bool glGetWorldCoord(double mx, double my, double & wx, double & wy, double & wz,
							   GLint * viewport, GLdouble * mvmatrix,GLdouble * projmatrix){
	/*  note viewport[3] is height of window in pixels  */
    GLint realy = viewport[3] - (GLint) my - 1;
	GLfloat z;
	glReadPixels(mx, realy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT,&z);
	if(z<1.0){
		gluUnProject ((GLdouble) mx, (GLdouble) realy, (GLdouble)z,
		mvmatrix, projmatrix, viewport, &wx, &wy, &wz);
		return true;
	}
	return false;
}
bool dvCamera::glGetWorldCoord(double mx, double my, double & wx, double & wy, double & wz){
	GLfloat z;
	GLint viewport[4];
	GLdouble mvmatrix[16], projmatrix[16];
	GLint realy;  /*  OpenGL y coordinate position  */
	//GLdouble wx, wy, wz;  /*  returned world x, y, z coords  */
	glGetIntegerv (GL_VIEWPORT, viewport);
    glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
    glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
	if(::glGetWorldCoord(mx,my,wx,wy,wz,viewport,mvmatrix,projmatrix)) return true;
	else{
		long border=1;
		long i,xpos,ypos;
		while(border<W && border<H)
		{
			for(i=-border;i<=border;i++)
			{
				xpos=mx+i;
				if(xpos>=0 && xpos<W)
				{
					ypos=my-border;
					if(ypos>=0 && ypos<H)
					{
						if(::glGetWorldCoord(xpos,ypos,wx,wy,wz,viewport,mvmatrix,projmatrix)) return true;
					}
					ypos=my+border;
					if(ypos>=0 && ypos<H)
					{
						if(::glGetWorldCoord(xpos,ypos,wx,wy,wz,viewport,mvmatrix,projmatrix)) return true;
					}
				}
			}
			for(i=-border+1;i<border;i++)
			{
				ypos=my+i;
				if(ypos>=0 && ypos<H)
				{
					xpos=mx-border;
					if(xpos>=0 && xpos<W)
					{
						if(::glGetWorldCoord(xpos,ypos,wx,wy,wz,viewport,mvmatrix,projmatrix)) return true;
					}
					xpos=mx+border;
					if(xpos>=0 && xpos<W)
					{
						if(::glGetWorldCoord(xpos,ypos,wx,wy,wz,viewport,mvmatrix,projmatrix)) return true;
					}
				}
			}
			border++;
		}
		return false;
	}
	return false;
}
void dvCamera::moveAbsolute(float x, float y, float z)
{
	posx=x;posy=y;posz=z;
	changed=true;
}
void dvCamera::moveRelative(float x, float y, float z)
{
	posx+=x;posy+=y;posz+=z;
	changed=true;
}

void dvCamera::moveWalk(float speed)
{
	moveRelative(lookx*speed,looky*speed,lookz*speed);
}

void dvCamera::moveForward(float speed)
{
	moveRelative(lookx*speed,looky*speed,0.);
}

/*
///////////////////////////////// STRAFE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
/////
/////	Strafes left/right at a speed
/////	from DigiBen		digiben@gametutorials.com
/////
///////////////////////////////// STRAFE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
*/
void dvCamera::moveStrafe(float speed)
{
	 // Strafing is quite simple if you understand what the cross product is.
    // If you have 2 vectors (say the up vVector and the view vVector) you can
    // use the cross product formula to get a vVector that is 90 degrees from the 2 vectors.
    // For a better explanation on how this works, check out the OpenGL "Normals" tutorial at our site.

    // Initialize a variable for the cross product result
    float vCrossx,vCrossy,vCrossz;

    // Get the view vVector of our camera and store it in a local variable
    float vViewVectorx=lookx;
    float vViewVectory=looky;
    float vViewVectorz=lookz;
    // Here we calculate the cross product of our up vVector and view vVector

    // The X value for the vVector is:  (V1.y * V2.z) - (V1.z * V2.y)
    vCrossx = ((upy * vViewVectorz) - (upz * vViewVectory));

    // The Y value for the vVector is:  (V1.z * V2.x) - (V1.x * V2.z)
    vCrossy = ((upz * vViewVectorx) - (upx * vViewVectorz));

    // The Z value for the vVector is:  (V1.x * V2.y) - (V1.y * V2.x)
    vCrossz = ((upx * vViewVectory) - (upy * vViewVectorx));

    // Now we want to just add this new vVector to our position and view, as well as
    // multiply it by our speed factor.  If the speed is negative it will strafe the
    // opposite way.

    // Add the resultant vVector to our position
    moveRelative(vCrossx * speed,vCrossy*speed,vCrossz * speed);
}
void dvCamera::moveFly(float speed)
{
	moveRelative(upx*speed,upy*speed,upz*speed);
}
/*
//////////////////////////////// UPDATE VECTOR \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\ 
/////
/////	Used to update the look and up vector
/////
///////////////////////////////// UPDATE VECTOR \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\/
*/
void updatevector(float *x, float *y, float *z, float *r, float *theta, float *phi)
{
	*x=(*r)*sin(*phi)*cos(*theta);
	*y=(*r)*sin(*phi)*sin(*theta);
	*z=(*r)*cos(*phi);
}
void dvCamera::lookAbsolute(float pan, float tilt)
{
	float one=1.0;
	lookphi=tilt;
	looktheta=pan;
	updatevector(&lookx,&looky,&lookz,&one,&looktheta,&lookphi);
	lookphi-=PI/2;
	updatevector(&upx,&upy,&upz,&one,&looktheta,&lookphi);
	lookphi+=PI/2;
	changed=true;
}
void dvCamera::lookRelativeMouse(float x, float y)
{
	mouseToAngle(x,y);
	lookRelative(x,y);
}
void dvCamera::lookRelative(float pan, float tilt)
{
	lookphi+=tilt;
	looktheta+=pan;
	lookAbsolute(looktheta,lookphi);
}

void dvCamera::lookAt(float x, float y, float z)
{
	if(upz<0) lookAbsolute(
           atan2(y-posy,x-posx)+PI,
           -atan2(sqrt((x-posx)*(x-posx)+(y-posy)*(y-posy)),z-posz));

	else lookAbsolute(
           atan2(y-posy,x-posx),
           atan2(sqrt((x-posx)*(x-posx)+(y-posy)*(y-posy)),z-posz));
}

void dvCamera::spin(float x, float y)
{
	float tempx=spinx-posx,tempy=spiny-posy,tempz=spinz-posz;
	float d=sqrt(tempx*tempx+tempy*tempy+tempz*tempz);
	lookAt(spinx,spiny,spinz);
	moveWalk(d);
	lookRelative(x,y);
	moveWalk(-d);
}

void dvCamera::mouseToAngle(float &x, float &y)
{
	x*=fov/(float)W*2./40.;
	y*=fov/(float)H*2./40.;
}
void dvCamera::spinMouse(float x, float y)
{
	mouseToAngle(x,y);
	spin(x,y);
}
void dvCamera::setFOV(float f)
{
	fov=f;
	glSetPerspective();
}
void dvCamera::glLook()
{
	gluLookAt(posx,posy,posz,lookx+posx,looky+posy,lookz+posz,upx,upy,upz);
}
void dvCamera::glSetPerspective()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if(perspective){
		gluPerspective(  fov,                         /* field of view in degree */
			aspect_ratio,                             /* aspect ratio */
			znear,  zfar);                 /* Z near *//* Z far */
	}
	else{
		glOrtho(-ortho_width/2,ortho_width/2,-ortho_height/2,ortho_height/2,znear,zfar);
	}
    glMatrixMode(GL_MODELVIEW);
	changed=true;
}
void dvCamera::glReshape(int w, int h)
{
	W=w;H=h;
	aspect_ratio=(double)W/double(H);
    glViewport(0, 0, W, H);
	glSetPerspective();
}

void dvCamera::getPosition(float * p) const
{
	p[0]=posx;
	p[1]=posy;
	p[2]=posz;
}
void dvCamera::getPosition(vector<float> & p) const
{
	p.resize(3);
	p[0]=posx;
	p[1]=posy;
	p[2]=posz;
}
void dvCamera::getLook(float * lk) const 
{
	lk[0]=lookx;
	lk[1]=looky;
	lk[2]=lookz;
}
void dvCamera::getLookAt(float * lk) const
{
	lk[0]=lookx+posx;
	lk[1]=looky+posy;
	lk[2]=lookz+posz;
}
void dvCamera::getLookAngles(float * a) const
{
	a[0]=looktheta;
	a[1]=lookphi;
}
void dvCamera::setOrthographicDimensions(float width, float height)
{
	ortho_width=width;
	ortho_height=height;
	glSetPerspective();
}
void dvCamera::togglePerspective()
{
	perspective=~perspective;
	glSetPerspective();
}
void dvCamera::setSpinPoint(float x, float y, float z)
{
	spinx=x;spiny=y;spinz=z;
}
void dvCamera::getSpinPoint(float &x, float &y, float &z) const
{
	x=spinx;
	y=spiny;
	z=spinz;
}
void dvCamera::setZNear(float zt)
{
	znear=zt;
	glSetPerspective();
}
void dvCamera::setZFar(float zt)
{
	zfar=zt;
	glSetPerspective();
}
bool dvCamera::isChanged()
{
	if(changed==true)
	{
		changed=false;
		return true;
	}
	return false;
}
