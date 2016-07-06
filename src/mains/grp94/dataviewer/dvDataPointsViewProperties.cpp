/********************************************************************
 *
 *
 * Name: dvDataPointsViewProperties.cpp
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
#include "dvDataPointsViewProperties.h"
dvDataPointsViewProperties::dvDataPointsViewProperties(){
	point_size=2;
}
void dvDataPointsViewProperties::modPointSize(int mod)
{
	point_size+=mod;
	if(point_size<=0) point_size=1;
	tracker.touch();
}
long dvDataPointsViewProperties::getPointSize() const
{
	return point_size;
}
void dvDataPointsViewProperties::setPointSize(long p)
{
	point_size=p;
	tracker.touch();
}
bool dvDataPointsViewProperties::isChanged(long id)
{
	return !tracker.isTracked(id); // value has changed if not tracked!
}