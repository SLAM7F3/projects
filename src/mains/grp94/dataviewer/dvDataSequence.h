/********************************************************************
 *
 * Name: dvDataSequence.h
 *
 * Author: Luke Skelly
 *
 * Description:
 * a container class for dvData which can be stepped through
 **********************************************************************/

#ifndef DVDATASEQUENCE_H
#define DVDATASEQUENCE_H

#include "dvDataCollection.h"

class dvDataSequence:public dvDataCollection
{

  public:
   dvDataSequence();
   virtual int glDraw(bool force_redraw=true,clock_t deadline=0,CFrustum * f=NULL);
   virtual int glDraw(vector<float> & camera_position,bool force_redraw=true,clock_t deadline=0,CFrustum * f=NULL);

   void setFPS(float fps);
   void modFPS(float mod);
   float getFPS();
   void setFrame(int i);
   int getFrame() const;
   void modFrame(long n);
   void data_dump_sequence(const char * filename_base);
   virtual bool isChanged();
   virtual void getSuggestedCameraLocationPoint(float & x, float & y, float & z) const;
   virtual void getSuggestedCameraLookPoint(float & x, float & y, float & z) const;

  protected:

   float fps;
   int stopped;
   float gltime;
   long frame_num;
   bool newframe;
};

#endif // DVDATASEQUENCE_H
