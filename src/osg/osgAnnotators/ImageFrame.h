// ==========================================================================
// Header file for ImageFrame class
// ==========================================================================
// Last updated on 9/19/06; 1/21/07; 10/13/07; 1/2/08
// ==========================================================================

#ifndef ImageFrame_H
#define ImageFrame_H

#include <iostream>
#include <string>
#include "osg/osgGeometry/Geometrical.h"
#include "passes/PassesGroup.h"

class AnimationController;
class LineSegmentsGroup;
class Movie;

class ImageFrame : public Geometrical
{

  public:
    
// Initialization, constructor and destructor functions:

   ImageFrame(Pass* PI_ptr,int id,AnimationController* AC_ptr);
   virtual ~ImageFrame();
   friend std::ostream& operator<< (
      std::ostream& outstream,const ImageFrame& s);

   void set_horiz_axis_label(std::string label);
   void set_vert_axis_label(std::string label);
   LineSegmentsGroup* get_linesegments_group_ptr();
   const LineSegmentsGroup* get_linesegments_group_ptr() const;
   
   void set_axes_labels(
      std::string horiz_label,std::string vert_label,
      double text_scalefactor=1.0,double max_text_width=5.0);
   void label_vertical_axis(
      std::string label,double text_scalefactor=1.0,
      double max_text_width=5.0);
   void label_horizontal_axis(
      std::string label,double text_scalefactor=1.0,
      double max_text_width=5.0);
   void transform_linesegments(double curr_t,int passnumber,Movie* movie_ptr);

  protected:

  private:

   std::string horiz_axis_label,vert_axis_label;
   LineSegmentsGroup* linesegments_group_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ImageFrame& s);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void ImageFrame::set_horiz_axis_label(std::string label)
{
   horiz_axis_label=label;
}

inline void ImageFrame::set_vert_axis_label(std::string label)
{
   vert_axis_label=label;
}

inline LineSegmentsGroup* ImageFrame::get_linesegments_group_ptr()
{
   return linesegments_group_ptr;
}

inline const LineSegmentsGroup* ImageFrame::get_linesegments_group_ptr() 
   const
{
   return linesegments_group_ptr;
}

#endif // ImageFrame.h



