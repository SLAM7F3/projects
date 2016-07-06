/********************************************************************
 *
 *
 * Name: dvDataTreeNode.h
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * A data container to organize the data into a tree.
 * A fancier alternative to dvDataPoints.
 * 
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *********************************************************************/

#ifndef DVDATATREENODE
#define DVDATATREENODE
#include <vector>
#include "Frustum.h"
#include <ctime>
#include "dvColormap.h"
#include "dvDataCollection.h"
#include "dvDataPointsViewProperties.h"
const int TREE_ORDER=8;
typedef long INDEX_TYPE;
using namespace std;
#define MIN(a,b) (((a)<(b))?(a):(b)) 
#define MAX(a,b) (((a)>(b))?(a):(b)) 
class dvDataTreeNode:public dvDataCollection{
public:
	dvDataTreeNode();
	~dvDataTreeNode();
	dvDataTreeNode(float * points, int npoints, int stride, int poffset, dvDataPointsViewProperties * p,int updir);
	dvDataTreeNode(vector<float> min, vector<float> max, float * points, CM_DATA_TYPE * colors, int stride, int poffset, dvDataPointsViewProperties * p,int updir, vector<long> & indices);
	virtual int glDraw(bool force_redraw,clock_t deadline,CFrustum * f);
	virtual int glDraw(vector<float> & camera_position,bool force_redraw=true,clock_t deadline=0,CFrustum * f=NULL);
	void getFirstPoint(float &x, float &y, float &z) const;
	virtual void setConstraint(dvDataConstraint* d);
	virtual void setViewProperties(dvDataPointsViewProperties * p);
	dvDataPointsViewProperties* getViewProperties();
	bool isChanged();
	virtual void resetConstraint();
protected:
	void branch(vector<float> min, vector<float> max, float * points, CM_DATA_TYPE * colors, int stride, int poffset, dvDataPointsViewProperties * p,int updir, vector<long> & indices);
	vector<float> minbox;
	vector<float> maxbox;
	dvDataPointsViewProperties* view_properties;
	long stride;
private:
	CM_DATA_TYPE * colors;
};	
#endif

