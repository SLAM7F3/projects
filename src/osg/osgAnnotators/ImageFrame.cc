// ==========================================================================
// ImageFrame class member function definitions
// ==========================================================================
// Last updated on 9/19/06; 1/21/07; 10/13/07
// ==========================================================================

#include <iostream>
#include <vector>
#include <osgText/Font>
#include <osg/Geode>
#include <osg/Node>
#include <string>
#include "osg/osgGraphicals/AnimationController.h"
#include "color/colorfuncs.h"
#include "osg/osgAnnotators/ImageFrame.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"
#include "osg/osg2D/Movie.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ImageFrame::allocate_member_objects()
{
}		       

void ImageFrame::initialize_member_objects()
{
   Graphical_name="ImageFrame";
   set_size(1.0);
   linesegments_group_ptr=NULL;
}		       

ImageFrame::ImageFrame(Pass* PI_ptr,int id,AnimationController* AC_ptr):
   Geometrical(3,id,AC_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();

   linesegments_group_ptr=new LineSegmentsGroup(
      3,PI_ptr,AnimationController_ptr);
   linesegments_group_ptr->generate_canonical_segments(4);
   linesegments_group_ptr->set_width(5);
   linesegments_group_ptr->get_LineSegment_ptr(0)->set_draw_arrow_flag();
//   linesegments_group_ptr->get_LineSegment_ptr(1)->set_draw_arrow_flag();
   linesegments_group_ptr->get_LineSegment_ptr(3)->set_draw_arrow_flag();
}		       

ImageFrame::~ImageFrame()
{
   delete linesegments_group_ptr;
}


// ---------------------------------------------------------------------
void ImageFrame::set_axes_labels(
   string horiz_label,string vert_label,
   double text_scalefactor,double max_text_width)
{
   label_vertical_axis(horiz_label,text_scalefactor,max_text_width);
   label_horizontal_axis(vert_label,text_scalefactor,max_text_width);
}

// ---------------------------------------------------------------------
void ImageFrame::label_vertical_axis(string label,double text_scalefactor,
                                     double max_text_width)
{
   osgText::Text* vertaxis_text_ptr=linesegments_group_ptr->
      get_LineSegment_ptr(3)->get_text_ptr();

   vertaxis_text_ptr->setCharacterSize(0.2*text_scalefactor); // SPASE
//   vertaxis_text_ptr->setCharacterSize(2*text_scalefactor); // RH2/AK

//   vertaxis_text_ptr->setAlignment(osgText::Text::LEFT_CENTER);
//   vertaxis_text_ptr->setAlignment(osgText::Text::CENTER_BOTTOM);

//   vertaxis_text_ptr->setAxisAlignment(osgText::Text::XY_PLANE);
//   vertaxis_text_ptr->setAxisAlignment(osgText::Text::REVERSED_XY_PLANE);
//   vertaxis_text_ptr->setAxisAlignment(osgText::Text::XZ_PLANE);
//   vertaxis_text_ptr->setAxisAlignment(osgText::Text::REVERSED_XZ_PLANE);
//   vertaxis_text_ptr->setAxisAlignment(osgText::Text::YZ_PLANE);
//   vertaxis_text_ptr->setAxisAlignment(osgText::Text::REVERSED_YZ_PLANE);
   vertaxis_text_ptr->setAxisAlignment(osgText::Text::SCREEN);
   vertaxis_text_ptr->setText(label);
   vertaxis_text_ptr->setColor(colorfunc::get_OSG_color(colorfunc::white));
   osg::Vec3 vert_text_position(1.1,0,0);
   vertaxis_text_ptr->setPosition(vert_text_position);
   vertaxis_text_ptr->setMaximumWidth(max_text_width);
}

// ---------------------------------------------------------------------
void ImageFrame::label_horizontal_axis(string label,double text_scalefactor,
                                       double max_text_width)
{
   osgText::Text* horizaxis_text_ptr=linesegments_group_ptr->
      get_LineSegment_ptr(0)->get_text_ptr();

   horizaxis_text_ptr->setCharacterSize(0.2*text_scalefactor); // SPASE
//   horizaxis_text_ptr->setCharacterSize(2*text_scalefactor); // RH2/AK
   horizaxis_text_ptr->setAxisAlignment(osgText::Text::SCREEN);
   horizaxis_text_ptr->setText(label);
   horizaxis_text_ptr->setColor(colorfunc::get_OSG_color(colorfunc::white));
   osg::Vec3 horiz_text_position(1.1,0,0);
   horizaxis_text_ptr->setPosition(horiz_text_position);
   horizaxis_text_ptr->setMaximumWidth(max_text_width);
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const ImageFrame& f)
{
   outstream << "inside ImageFrame::operator<<" << endl;
   outstream << static_cast<const Geometrical&>(f) << endl;
   return(outstream);
}

// ---------------------------------------------------------------------
// Member function transform_linesegments computes the world space
// corner locations of the image frame.  It then scales, rotates and
// translates each line segment within linesegments_group so that they
// border the image frame corresponding to time curr_t and pass
// pass_number.  It also stretches the U and V borders so that their
// vector arrow heads poke out beyond the image plane's rectangular
// boundary.

void ImageFrame::transform_linesegments(
   double curr_t,int passnumber,Movie* movie_ptr)
{

// First compute worldspace corner locations of imageframe line
// segments:

   double Uscale,Vscale,Wscale;
   threevector Uhat,Vhat,What;
   movie_ptr->get_UVW_scales(curr_t,passnumber,Uscale,Vscale,Wscale);
   movie_ptr->get_UVW_dirs(curr_t,passnumber,Uhat,Vhat,What);

   vector<threevector> V1,V2;
   V1.push_back(movie_ptr->get_frame_origin());
   V2.push_back(V1.back()+Uscale*Uhat);
   
   V1.push_back(V2.back());
   V2.push_back(V1.back()+Vscale*Vhat);

   V1.push_back(V2.back());
   V2.push_back(V1.back()-Uscale*Uhat);

   V1.push_back(V2.back());
   V2.push_back(V1.back()-Vscale*Vhat);

   const double arrow_stretch_factor=0.15;
   
   for (int n=0; n<4; n++)
   {
      LineSegment* LineSegment_ptr=linesegments_group_ptr->
         get_LineSegment_ptr(n);
      LineSegment_ptr->set_curr_color(colorfunc::white);
      LineSegment_ptr->set_permanent_color(colorfunc::white);

      if (n==0)
      {
         V2[n] += arrow_stretch_factor*(V2[n]-V1[n]);
      }

//      if (n==1)
//      {
//         V2[n] += arrow_stretch_factor*(V2[n]-V1[n]);
//      }

      if (n==3)
      {
         threevector v1=V2[n];
         threevector v2=V1[n];
         v2 += arrow_stretch_factor*(v2-v1);
         V1[n]=v1;
         V2[n]=v2;
      }

      LineSegment_ptr->set_scale_attitude_posn(curr_t,passnumber,V1[n],V2[n]);
   } // loop over index n labeling imageframe segments

}
