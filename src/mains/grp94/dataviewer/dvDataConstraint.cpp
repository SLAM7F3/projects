/********************************************************************
 *
 *
 * Name: dvDataConstraint.cpp
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
#include "dvDataConstraint.h"
dvDataConstraint::dvDataConstraint()
{
	fastmod=0;
	modspeed=4;
}
void dvDataConstraint::modMin(long dim, float mod)
{
	checkSize(dim);
	constraint_min[dim]+=.1*mod*modspeed*(1+fastmod*4);
	trackers[dim].touch();
}
void dvDataConstraint::modMax(long dim, float mod)
{
	checkSize(dim);
	constraint_max[dim]+=.1*mod*modspeed*(1+fastmod*4);
	trackers[dim].touch();
}
void dvDataConstraint::setMin(long dim, float val)
{
	checkSize(dim);
	if(constraint_min[dim]!=val) trackers[dim].touch();
	constraint_min[dim]=val;
}
void dvDataConstraint::setMax(long dim, float val)
{
	checkSize(dim);
	if(constraint_min[dim]!=val) trackers[dim].touch();
	constraint_max[dim]=val;
}
float dvDataConstraint::getMax(long dim)
{
	checkSize(dim);
	return constraint_max[dim];
}
float dvDataConstraint::getMin(long dim)
{
	checkSize(dim);
	return constraint_min[dim];
}
bool dvDataConstraint::isChanged(long id,long dim)
{
	checkSize(dim);
	return !trackers[dim].isTracked(id); // value has changed if not tracked!
}
void dvDataConstraint::fastColor(int yes)
{
	fastmod=yes;
}
void dvDataConstraint::checkSize(long d)
{
	if(d>=constraint_min.size())
	{
		constraint_min.resize(d+1);
		constraint_max.resize(d+1);
		trackers.resize(d+1);
	}
}
long dvDataConstraint::size() const
{
	return constraint_min.size();
}