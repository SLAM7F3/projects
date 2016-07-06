// ==========================================================================
// Header file for mover class
// ==========================================================================
// Last modified on 9/29/08; 12/5/08; 2/13/09
// ==========================================================================

#ifndef MOVER_H
#define MOVER_H

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "track/track.h"

class mover
{
  public:

   enum MoverType 
   {
      VEHICLE=0, ROI=1, UAV=2, KOZ=3
   };

// Initialization, constructor and destructor functions:

   mover(MoverType t,int i);
   mover(const mover& m);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~mover();
   mover& operator= (const mover& m);
   friend std::ostream& operator<< (std::ostream& outstream,const mover& m);

// Set and get member functions:

   MoverType get_MoverType() const;
   void set_previously_encountered_flag(bool flag);
   bool get_previously_encountered_flag() const;
   int get_ID() const;
   void set_relative_size(double size);
   double get_relative_size() const;
   void set_avg_time_duration(double t);
   double get_avg_time_duration() const;
   void set_RGB_color(colorfunc::RGB curr_RGB);
   colorfunc::RGB get_RGB_color() const;
   void set_annotation_label(std::string label);
   std::string get_annotation_label() const;
   void set_track_ptr(track* t_ptr);
   track* get_track_ptr();
   const track* get_track_ptr() const;
   void set_orig_track_ptr(track* t_ptr);
   track* get_orig_track_ptr();
   const track* get_orig_track_ptr() const;
      
  private: 

   MoverType type;
   bool previously_encountered_flag;
   int ID;
   double relative_size;
   double avg_time_duration;  // secs
   colorfunc::RGB RGB_color;
   std::string annotation_label;
   track *track_ptr, *orig_track_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const mover& m);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline mover::MoverType mover::get_MoverType() const
{
   return type;
}

inline void mover::set_previously_encountered_flag(bool flag)
{
   previously_encountered_flag=flag;
}

inline bool mover::get_previously_encountered_flag() const
{
   return previously_encountered_flag;
}

inline int mover::get_ID() const
{
   return ID;
}

inline void mover::set_relative_size(double size) 
{
   relative_size=size;
}

inline double mover::get_relative_size() const
{
   return relative_size;
}

inline void mover::set_avg_time_duration(double t)
{
   avg_time_duration=t;
}

inline double mover::get_avg_time_duration() const
{
   return avg_time_duration;
}

inline void mover::set_RGB_color(colorfunc::RGB curr_RGB) 
{
   RGB_color=curr_RGB;
}

inline colorfunc::RGB mover::get_RGB_color() const
{
   return RGB_color;
}

inline void mover::set_annotation_label(std::string label)
{
   annotation_label=label;
}

inline std::string mover::get_annotation_label() const
{
   return annotation_label;
}

inline void mover::set_track_ptr(track* t_ptr)
{
   track_ptr=t_ptr;
}

inline track* mover::get_track_ptr()
{
   return track_ptr;
}

inline const track* mover::get_track_ptr() const
{
   return track_ptr;
}

inline void mover::set_orig_track_ptr(track* t_ptr)
{
   orig_track_ptr=t_ptr;
}

inline track* mover::get_orig_track_ptr()
{
   return orig_track_ptr;
}

inline const track* mover::get_orig_track_ptr() const
{
   return orig_track_ptr;
}


#endif  // mover.h



