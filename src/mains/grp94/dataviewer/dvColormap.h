// ==========================================================================
// Last updated on 6/8/04
// ==========================================================================

/********************************************************************
 *
 * Name: dvColormap.h
 *
 * Author: Luke Skelly
 *
 * Description:
 * fancy wrapper for colormaps_opengl.cpp
 *
 **********************************************************************/

#ifndef DVCOLORMAP
#define DVCOLORMAP

#include "colormaps_opengl.h"
#include "dvTracker.h"
#define DEFAULT_N_COLORS 65536 // 16bit
#define CM_DATA_TYPE unsigned char
const CM_DATA_TYPE CM_MIN=0;
const CM_DATA_TYPE CM_MAX=255;

class dvColormap
{

  public:

   dvColormap(colormap_t name=jet, long n_colors=DEFAULT_N_COLORS);
   long getSize();
   void getColor(long c, CM_DATA_TYPE * a);
   void operator = (const dvColormap & d);
   void prevColormap();
   void nextColormap();
   void setColormap(colormap_t name);
   colormap_t getColormap() const;
   bool isChanged(long id);

  private:

   long n_colors;
   CM_DATA_TYPE * colors;
   colormap_t cmname;
   dvTracker tracker;
};

#endif
