/********************************************************************
 *
 *
 * Name: dvDataPointsViewProperties.h
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * Keeps track of info specific to Data Points (vs Lines, Triangles)
 * 
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *********************************************************************/
#ifndef DVDATAPOINTSVIEWPROP
#define DVDATAPOINTSVIEWPROP
#include "dvTracker.h"
class dvDataPointsViewProperties{
public:
	dvDataPointsViewProperties();
	void modPointSize(int n);
	long getPointSize() const;
	void setPointSize(long s);
	bool isChanged(long id);
private:
	long point_size;
	dvTracker tracker;
};
#endif
