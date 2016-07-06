// ==========================================================================
// PowerPoint class member function definitions
// ==========================================================================
// Last updated on 8/24/07
// ==========================================================================

#include <iostream>
#include <string>
#include <osg/Geode>
#include "osg/osgAnnotators/PowerPoint.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PowerPoint::allocate_member_objects()
{
}		       

void PowerPoint::initialize_member_objects()
{
   Graphical_name="PowerPoint";
   selected_face_displacement=0.1;
}		       

PowerPoint::PowerPoint(double w,double l,double h,int id):
   Box(w,l,h,id)
{	
   allocate_member_objects();
   initialize_member_objects();
}		       

PowerPoint::~PowerPoint()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const PowerPoint& b)
{
   outstream << "inside PowerPoint::operator<<" << endl;
   return(outstream);
}

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_drawable_group instantiates an OSG Group
// containing an OSG Box shape and a selected face rectangle.

osg::Group* PowerPoint::generate_drawable_group()
{
   osg::Group* group_ptr=Box::generate_drawable_group();

// Load image from input file.  Then assign image to texture:

   string osg_data_dir(getenv("OSG_FILE_PATH"));
   osg_data_dir += "/powerpoint/";

// As of 8/24/07, we do not understand why we have to flip the
// following image in order for it to appear correctly oriented on the
// box face:

   string image_filename=osg_data_dir+"PowerPointIconWords.png";
//   cout << "image_filename = " << image_filename << endl;

   osg::StateSet* face_stateset_ptr=selected_face_ptr->
      generate_texture_and_stateset(image_filename);
   
   osg::Geode* face_geode_ptr=dynamic_cast<osg::Geode*>(
      group_ptr->getChild(1));
   face_geode_ptr->setStateSet(face_stateset_ptr);

// Not sure if next 2 lines actually do anything...

   set_permanent_color(colorfunc::red);
   set_curr_color(colorfunc::red,0.5);

   return group_ptr;
}
