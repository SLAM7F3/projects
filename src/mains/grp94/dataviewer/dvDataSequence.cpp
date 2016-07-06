/********************************************************************
 *
 *
 * Name: dvDataSequence.cpp
 *
 *
 * Author: Luke Skelly
 *
 * Description:
 * a container class for dvData which can be stepped through
 *
 * --------------------------------------------------------------
 *    $Revision: 1.2 $
 * ---------------------------------------------------------------
 *
 *
 *
 **********************************************************************/
#include "dvDataSequence.h"
#include "base_file.h"

// CLK_TCK is obsolete, gcc3 does not support it
#ifndef CLK_TCK
#define CLK_TCK CLOCKS_PER_SEC
#endif

dvDataSequence::dvDataSequence()
{
   fps=0.0;
   stopped=1;
   gltime=(float)clock();
   frame_num=0;
   newframe=false;
}

void dvDataSequence::setFrame(int n)
{
   n%=size();
   if(n<0) n+=size();
   if(frame_num!=n)
   {
      frame_num=n;
      newframe=true;
   }
}

int dvDataSequence::getFrame() const
{
   return frame_num;
}

void dvDataSequence::modFrame(long n)
{
   setFrame(frame_num+n);
}

int dvDataSequence::glDraw(bool force_redraw,clock_t deadline,CFrustum * f)
{
   int i=0;
   clock_t now;
   int ret;
   bool local_changed=isChanged();
   if(local_changed || force_redraw)
   {
      force();
   }
   if(finished_draw && finished_force_pos>=size()) return 1;
   if(stopped){
      ret=dvDataCollection::glDraw(force_redraw || local_changed,deadline,f);
      if(ret==1) finished_force_pos=size();
      newframe=false;
   }
   else if(fps<0.0)
   {
      if(finished_force[frame_num]==false) 
      {
         finished_force_pos++;
      }
      ret=getData(frame_num)->glDraw(!finished_force[frame_num],deadline,f);
      newframe=false;
      finished_force[frame_num]=true;
   }
   else
   {
      if(finished_force[frame_num]==false) 
      {
         finished_force_pos++;
      }
      ret=getData(frame_num)->glDraw(!finished_force[frame_num],deadline,f);
      newframe=false;
      finished_force[frame_num]=true;
      if(clock()>gltime)
      {
         modFrame(1);		
         gltime=clock()+CLK_TCK/fps;
      }
   }
   return finished_draw=ret;
}
int dvDataSequence::glDraw(vector<float> & camera_position,bool force_redraw,clock_t deadline,CFrustum * f)
{
   int i=0;
   clock_t now;
   int ret;
   bool local_changed=isChanged();
   if(local_changed || force_redraw)
   {
      force();
   }
   if(finished_draw && finished_force_pos>=size()) return 1;
   if(stopped){
      ret=dvDataCollection::glDraw(camera_position,force_redraw,deadline,f);
      if(ret==1) finished_force_pos=size();
      newframe=false;
   }
   else if(fps<0.0)
   {
      if(finished_force[frame_num]==false) 
      {
         finished_force_pos++;
      }
      ret=getData(frame_num)->glDraw(camera_position,!finished_force[frame_num],deadline,f);
      newframe=false;
      finished_force[frame_num]=true;
   }
   else
   {
      if(finished_force[frame_num]==false) 
      {
         finished_force_pos++;
      }
      ret=getData(frame_num)->glDraw(camera_position,!finished_force[frame_num],deadline,f);
      newframe=false;
      finished_force[frame_num]=true;
      if(clock()>gltime)
      {
         modFrame(1);		
         gltime=clock()+CLK_TCK/fps;
      }
   }
   return finished_draw=ret;
}

void dvDataSequence::setFPS(float f)
{
   if(f==0.0 || (f==fps && stopped==0)) stopped=1;
   else
   {
      fps=f;
      stopped=0;
   }
   newframe=true;
}

float dvDataSequence::getFPS()
{
   return fps;
}

void dvDataSequence::modFPS(float mod)
{
   fps*=mod;
}
void dvDataSequence::data_dump_sequence(const char * filename_base)
{
   char filename[1024];
   int i;
   base_file file;
   if(stopped){
      printf("Dumping data\n");
      sprintf(filename,"%s.fxyz",filename_base);
      file.set_filename(filename);
      file.open_output_overwrite_binary();
      dvDataCollection::data_dump(file.fp);
      file.close();
   }
   else if(fps<0.0)
   {
      int fnum=frame_num;
      printf("Dumping frame number %d\n",fnum);
      sprintf(filename,"%s.%04d.fxyz",filename_base,fnum);
      file.set_filename(filename);
      file.open_output_overwrite_binary();
      getData(fnum)->data_dump(file.fp);
      file.close();
   }
   else
   {
      printf("Dumping data sequence\n");
      for(i=0;i<size();i++)
      {
         sprintf(filename,"%s.%04d.fxyz",filename_base,i);
         file.set_filename(filename);
         file.open_output_overwrite_binary();
         getData(i)->data_dump(file.fp);
         file.close();
      }
   }
}
void dvDataSequence::getSuggestedCameraLocationPoint(float & x, float & y, float & z) const
{
   if(stopped){
      dvDataCollection::getSuggestedCameraLocationPoint(x,y,z);
   }
   else
   {
      collection[frame_num]->getSuggestedCameraLocationPoint(x,y,z);
   }
}
void dvDataSequence::getSuggestedCameraLookPoint(float & x, float & y, float & z) const
{
   if(stopped){
      dvData::getSuggestedCameraLookPoint(x,y,z);
   }
   else
   {
      collection[frame_num]->getSuggestedCameraLookPoint(x,y,z);
   }
}
bool dvDataSequence::isChanged()
{
   if(!stopped && fps>0.0 && clock()>gltime)
   {
      modFrame(1);		
      gltime=clock()+CLK_TCK/fps;
   }
   bool f=newframe;
   newframe=false;
   return dvDataCollection::isChanged() || f;
}
