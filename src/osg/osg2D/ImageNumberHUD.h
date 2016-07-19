// ==========================================================================
// Header file for imagenumberHUD class which displays the current
// video image number as well as its mode (play, pause, reverse, etc)
// ==========================================================================
// Last modified on 5/20/11; 12/31/11; 4/2/14; 7/19/16
// ==========================================================================

#ifndef IMAGENUMBERHUD_H
#define IMAGENUMBERHUD_H

#include "osg/GenericHUD.h"

class AnimationController;

class ImageNumberHUD : public GenericHUD
{
  public:

   ImageNumberHUD(
      AnimationController* AC_ptr,
      bool display_movie_number=true,bool display_movie_state=true,
      bool display_movie_world_time=false,
      bool display_movie_elapsed_time=false,
      bool display_movie_nframes=false);
      
   void showFrame();

// Set & get member functions:

   void set_display_movie_number_flag(bool flag);
   void set_display_movie_state_flag(bool flag);
   void set_display_movie_world_time_flag(bool flag);
   void set_display_movie_elapsed_time_flag(bool flag);
   void set_display_movie_nframes_flag(bool flag);
   void set_display_UTC_flag(bool flag);
   void set_AnimationController_ptr(AnimationController* AC_ptr);

   void push_back_label(std::string curr_label);

  protected:

  private:

   bool m_display_movie_state;
   bool m_display_movie_number;
   bool m_display_movie_world_time;
   bool m_display_movie_elapsed_time;
   bool m_display_movie_nframes;
   bool display_UTC_flag;
   AnimationController* AnimationController_ptr;
   std::vector<std::string> labels;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void ImageNumberHUD::set_display_movie_number_flag(bool flag)
{
   m_display_movie_number=flag;
}

inline void ImageNumberHUD::set_display_movie_state_flag(bool flag)
{
   m_display_movie_state=flag;
}

inline void ImageNumberHUD::set_display_movie_world_time_flag(bool flag)
{
   m_display_movie_world_time=flag;
}

inline void ImageNumberHUD::set_display_movie_elapsed_time_flag(bool flag)
{
   m_display_movie_elapsed_time=flag;
}

inline void ImageNumberHUD::set_display_movie_nframes_flag(bool flag)
{
   m_display_movie_nframes=flag;
}

inline void ImageNumberHUD::set_display_UTC_flag(bool flag)
{
   display_UTC_flag=flag;
}

inline void ImageNumberHUD::set_AnimationController_ptr(
   AnimationController* AC_ptr)
{
   AnimationController_ptr=AC_ptr;
}

inline void ImageNumberHUD::push_back_label(std::string curr_label)
{
   labels.push_back(curr_label);
}


#endif 
