/********************************************************************
 *
 *
 * Name: dvDataPoints.h
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * draws points for openGL
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 *
 *
 **********************************************************************/
#ifndef DVDATAPOINTS
#define DVDATAPOINTS
#include "dvData.h"
#include "dvDataPointsViewProperties.h"
#include "dvColormap.h"
#include <vector>
using namespace std;
class dvDataPoints:public dvData{
public:
	dvDataPoints();
	~dvDataPoints();
	dvDataPoints(int stride,dvDataPointsViewProperties * p);
	dvDataPoints(int stride, int poffset,dvDataPointsViewProperties * p,int udir);
	dvDataPoints(float * points, long numpoints,dvDataPointsViewProperties * p);
	dvDataPoints(int stride, int poffset, float * points, long numpoints,dvDataPointsViewProperties * p);
	dvDataPoints(const dvDataPoints& dp);
	void clear();
	void add(float * pts, long numpoints);
	virtual int glDraw(bool force_redraw=true,clock_t deadline=0,CFrustum * f=NULL);
	void getCentroid(float &x, float &y, float &z) const;
	void getFirstPoint(float &x, float &y, float &z) const;
	void setUpDirection(int n);
	void setViewProperties(dvDataPointsViewProperties * p);
	virtual void setConstraint(dvDataConstraint* d);
	dvDataPointsViewProperties* getViewProperties();
	int getStride() const;
	dvDataPoints& operator = (const dvDataPoints & d);
	void printInfo() const;
	void dvDataPoints::data_dump(FILE * file);
	void modColorCode(int mod);
	void setColorCode(int c);
	
private:
	void updateColors();
	void updatePointInfo();
	void init(int stride, int poffset, float * points, long numpoints,dvDataPointsViewProperties * p,int udir);
	void rotateAxis(int n);
	vector<float *> points;
	vector<long> npoints;
	vector<long> points_to_draw;
	vector<CM_DATA_TYPE *> colors;
	vector<long *> indices;
	int stride;
	int poffset;
	vector<float> min;
	vector<float> max;
	vector<float>constrain_min;
	vector<float>constrain_max;
	vector<float>mean;
	long pointstodraw;
	int updir;
	long n_elements;
	dvDataPointsViewProperties* view_properties;
};
#endif
