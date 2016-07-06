/********************************************************************
 *
 *
 * Name: dvDataTreeLeaf.h
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * Data container for the bottom (leaf) of a DataTree.
 * Only used directly by dvDataTreeNode class.
 * 
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *********************************************************************/
#ifndef DVDATATREELEAF
#define DVDATAREELEAF

#include "dvDataTreeNode.h"
#include "dvDataPoints.h"
const long MAX_LEAF_SIZE=10000;
const long DEFAULT_STRIDE =4;
const long POSITION_LENGTH =3;
const long COLOR_LENGTH =3;
#define COLOR_TYPE GL_UNSIGNED_BYTE
const long COLOR_STRIDE =0;
class dvDataTreeLeaf:public dvDataTreeNode
{
public:
	dvDataTreeLeaf();
	dvDataTreeLeaf(vector<float> min, vector<float> max, float * points, CM_DATA_TYPE * colors, int stride, int poffset, dvDataPointsViewProperties * p,int updir, vector<long> & indices);
	virtual int glDraw(bool force_redraw,clock_t deadline,CFrustum * f);
	virtual int glDraw(vector<float> & camera_position,bool force_redraw,clock_t deadline,CFrustum * f);
	void setColormap(dvColormap * cm);
	void getCentroid(float &x, float &y, float &z) const;
	void getFirstPoint(float &x, float &y, float &z) const;
	int getStride() const;
	void rotateAxis(int n);
	void setConstraint(dvDataConstraint* d);
	void modColorCode(int mod);
	void setColorCode(int c);
	void printInfo() const;
	void data_dump(FILE * file);

private:
	void updateColors();
	void updatePointInfo();
	float * points;
	CM_DATA_TYPE * colors;
	long * indicestodraw;
	vector<long> indices;
	int poffset;
	vector<float> min;
	vector<float> max;

	vector<float>mean;

	long pointstodraw;

	int updir;

	bool updatecolor;

};
#endif
