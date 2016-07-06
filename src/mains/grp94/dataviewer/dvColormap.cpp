// ==========================================================================
// Last updated on 6/6/04
// ==========================================================================

/********************************************************************
 *
 * Name: dvColormap.cpp
 * Author: Luke Skelly
 *
 * Description:
 * fancy wrapper for colormaps_opengl.cpp
 **********************************************************************/

#include "basic_math.h"
#include "dvColormap.h"

dvColormap::dvColormap(colormap_t name,long n)
{
   n_colors=n;
   cmname=name;
   colors=colormaps_func::colormap_interpolate(cmname,n,CM_MIN,CM_MAX);
}

long dvColormap::getSize()
{
   return n_colors;
}

void dvColormap::getColor(long c, CM_DATA_TYPE * a)
{
   if (colors==NULL)
   {
      a[0]=a[1]=a[2]=1.0;
   }
   else memcpy(a,colors+c*3,sizeof(CM_DATA_TYPE)*3);
}

void dvColormap::operator = (const dvColormap & d)
{
   if (colors!=NULL) free(colors);
   cmname=d.cmname;
   colors=(CM_DATA_TYPE*)malloc(d.n_colors*sizeof(CM_DATA_TYPE)*3);
   memcpy(colors,d.colors,sizeof(CM_DATA_TYPE)*3*d.n_colors);
   n_colors=d.n_colors;
}

void dvColormap::nextColormap()
{
   setColormap(static_cast<colormap_t>(
      modulo(int(getColormap())+1,colormaps_func::number_of_colormaps)));
//   setColormap(getColormap()+colormap_t(1));
}

void dvColormap::prevColormap()
{
   setColormap(static_cast<colormap_t>(
      modulo(int(getColormap())-1,colormaps_func::number_of_colormaps)));
//   setColormap(getColormap()+colormap_t(-1));
}

bool dvColormap::isChanged(long id)
{
   return !tracker.isTracked(id); // value has changed if not tracked!
}

void dvColormap::setColormap(colormap_t name)
{
   if (name!=cmname)
   {
      if (colors!=NULL) free(colors);
      cmname=name;
      colors=colormaps_func::colormap_interpolate(
         cmname,n_colors,CM_MIN,CM_MAX);
   }
   tracker.touch();
}

colormap_t dvColormap::getColormap() const
{
   return cmname;
}
