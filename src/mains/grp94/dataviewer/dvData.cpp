// ==================================================================
// Last updated on 6/6/04
// ==================================================================

/********************************************************************
 *
 * Name: dvData.cpp
 *
 * Author: Luke Skelly
 *
 * Description:
 * base class for all types of data (points, lines, polys, etc.)
 **********************************************************************/

#include <cmath>
#include <iostream>
#include "basic_math.h"
#include "colormaps_opengl.h"
#include "dvData.h"

dvData::dvData()
{
   finished_draw=0;
   colormap=NULL;
   color_code=0;
   data_constraint=NULL;
   lookx=looky=lookz=locx=locy=locz=0.0f;
}

long dvData::getNumberOfElements() const
{
   return 0;
}

int dvData::glDraw(bool force_redraw,clock_t deadline,CFrustum * f){return 0;}

int dvData::glDraw(vector<float> & camera_position,bool force_redraw,
                   clock_t deadline,CFrustum * f)
{
   printf("wrong glDraw!\n");return 0;
}

void dvData::getSuggestedCameraLookPoint(float &x, float &y, float &z) const
{
   x=lookx;y=looky;z=lookz;
}

void dvData::getSuggestedCameraLocationPoint(float &x, float &y, float &z) const
{
   x=locx;y=locy;z=locz;
}

float dvData::distance(float x, float y, float z)
{
   return sqrt((x-lookx)*(x-lookx)+(y-looky)*(y-looky)+(z-lookz)*(z-lookz));
}

void dvData::setSuggestedCameraLookPoint(float x, float y, float z)
{
   lookx=x;looky=y;lookz=z;
}

void dvData::setSuggestedCameraLocationPoint(float x, float y, float z)
{
   locx=x;locy=y;locz=z;
}

void dvData::setColormap(dvColormap* c_ptr)
{
   colormap=c_ptr;
}

void dvData::setConstraint(dvDataConstraint* d)
{
   data_constraint=d;
}

dvDataConstraint* dvData::getConstraint()
{
   return data_constraint;
}

dvColormap* dvData::getColormap() const
{
   return colormap;
}

void dvData::clear()
{}

// Since int mod can equal -1, we need to set the color code modulo 4
// in order to avoid negative color code values!

void dvData::modColorCode(int mod)
{
   setColorCode(getColorCode()+mod);
}

void dvData::setColorCode(int c)
{
   color_code=c;
   colormap->setColormap((colormap_t) modulo(colormap->getColormap(),4));
}

long dvData::getColorCode() const
{
   return color_code;
}

void dvData::data_dump(FILE * file)
{}

bool dvData::isChanged()
{
   long id=(long)this;
   bool cchange=colormap->isChanged(id);
   bool conchange=data_constraint->isChanged(id,color_code);
   bool pchange=data_constraint->isChanged(id,3);
   return cchange || conchange || pchange;
}

void dvData::resetConstraint()
{
}
