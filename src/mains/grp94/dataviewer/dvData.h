// ==================================================================
// Last updated on 6/6/04
// ==================================================================

/********************************************************************
 *
 * Name: dvData.h
 * Author: Luke Skelly
 *
 * Description:
 * base class for all types of data (points, lines, polys, etc.)
 **********************************************************************/

#ifndef DVDATA_H
#define DVDATA_H

#include <ctime>
#include "dvColormap.h"
#include "dvDataConstraint.h"
#include "Frustum.h"

class dvData
{

  public:

   long getNumberOfElements() const;
   virtual int glDraw(bool force_redraw=true,clock_t deadline=0,
                      CFrustum * f=NULL);
   virtual int glDraw(vector<float> & camera_position,
                      bool force_redraw=true,clock_t deadline=0,
                      CFrustum * f=NULL);

   virtual void setColormap(dvColormap* c_ptr);
   dvColormap* getColormap() const;
   virtual void setConstraint(dvDataConstraint* d);
   dvDataConstraint* getConstraint();
   virtual void clear();
   virtual void getSuggestedCameraLookPoint(float &x, float &y, float &z) 
      const;
   virtual void getSuggestedCameraLocationPoint(float &x, float &y, float &z) 
      const;
   void setSuggestedCameraLookPoint(float x, float y, float z);
   void setSuggestedCameraLocationPoint(float x, float y, float z);
   virtual void resetConstraint();
   long getColorCode() const;
   virtual void modColorCode(int mod);
   virtual void setColorCode(int c);
   virtual void data_dump(FILE * file);
   virtual bool isChanged();
   float distance(float x, float y, float z);

  protected:

   dvData();
   dvColormap* colormap;
   dvDataConstraint* data_constraint;
   float lookx,looky,lookz;
   float locx,locy,locz;
   long color_code;
   long finished_draw;
  private:
	
};

#endif // DVDATA_H
