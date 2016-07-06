/********************************************************************
 *
 *
 * Name: dvDataPointCollection.h
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * container class for dvDataPoints
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 *
 *
 **********************************************************************/
#ifndef DVDATAPOINTCOLLECTION
#define DVDATAPOINTCOLLECTION
#include "dvDataCollection.h"
#include "dvDataPoints.h"
#include "dvColormap.h"
//typedef (void (dvDataPoints::*) (float m)) modfloat;
class dvDataPointCollection:public dvCollection<dvDataPoints>{
public:
	void modConstrainMax(float mod);
	void modConstrainMin(float mod);
	void setConstrainMin(float val);
	void setConstrainMax(float val);
	void setConstrainMin(int dim, float val);
	void setConstrainMax(int dim, float val);
	float getConstrainMax() const;
	float getConstrainMin() const;
	float getConstrainMax(int dim) const;
	float getConstrainMin(int dim) const;
	void modPoffsetConstrainMin(float mod);
	float getPoffsetConstrainMin() const;
	void modPointSize(int n);
	void cycleColorDirection();
	void setUpDirection(int n);
	void setPoffsetConstrainMin(float f);
	void setColormap(dvColormap * cm);
	void fastColor(int yes);
	void setViewPoint(float x, float y, float z);
	void ResetConstraints();
	void data_dump(const char * filename_base) const;
	//friend class dvDataPoints;
private:
	dvColormap colormap;
};	

#endif

