/********************************************************************
 *
 *
 * Name: dvDataConstraint.h
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * Bounding box for n dimensions.  Used by dataviewer to determine
 * color of data and when to update color info.
 * 
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *********************************************************************/
#ifndef DVDATACONSTRAINT
#define DVDATACONSTRAINT
#include <vector>
#include <cstdio>
#include "dvTracker.h"
using namespace std;
class dvDataConstraint{
public:
	dvDataConstraint();
	void modMin(long dim, float mod);
	void modMax(long dim, float mod);
	void setMin(long dim, float val);
	void setMax(long dim, float val);
	float getMax(long dim);
	float getMin(long dim);
	bool isChanged(long id,long dim);
	void fastColor(int yes);
	long size() const;
protected:
	void checkSize(long d);
	vector<float> constraint_min;
	vector<float> constraint_max;
	vector<dvTracker> trackers;
	float modspeed;
	float fastmod;
};
#endif
