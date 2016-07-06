// ==========================================================================
// ArmySymbol class member function definitions
// ==========================================================================
// Last updated on 2/7/07; 8/13/09; 3/31/11
// ==========================================================================

#include <iostream>
#include <string>
#include <osg/Geode>
#include "osg/osgAnnotators/ArmySymbol.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ArmySymbol::allocate_member_objects()
{
}		       

void ArmySymbol::initialize_member_objects()
{
   Graphical_name="ArmySymbol";
   symbol_type=0;
   selected_face_displacement=0.1;
}		       

ArmySymbol::ArmySymbol(double w,double l,double h,int id):
   Box(w,l,h,id)
{	
   allocate_member_objects();
   initialize_member_objects();
}		       

ArmySymbol::~ArmySymbol()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const ArmySymbol& b)
{
   outstream << "inside ArmySymbol::operator<<" << endl;
   return(outstream);
}

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_drawable_group() instantiates an OSG Group
// containing an OSG Box shape and a selected face rectangle.

osg::Group* ArmySymbol::generate_drawable_group()
{
   osg::Group* group_ptr=Box::generate_drawable_group();

   reset_symbol_image();

// Not sure if next 2 lines actually do anything...

   set_permanent_color(colorfunc::red);
   set_curr_color(colorfunc::red,0.5);

   return group_ptr;
}

// ---------------------------------------------------------------------
// Member function reset_symbol_image()

void ArmySymbol::reset_symbol_image()
{
//   cout << "inside ArmySymbol::reset_symbol_image()" << endl;
   
   string image_filename=get_image_filename_corresponding_to_symbol_type();
//   cout << "image_filename = " << image_filename << endl;

   osg::StateSet* face_stateset_ptr=selected_face_ptr->
      generate_texture_and_stateset(image_filename);
//   cout << "face_stateset_ptr = " << face_stateset_ptr << endl;
   
   osg::Geode* face_geode_ptr=dynamic_cast<osg::Geode*>(
      get_group_ptr()->getChild(1));
//   cout << "face_geode_ptr = " << face_geode_ptr << endl;
   
   face_geode_ptr->setStateSet(face_stateset_ptr);
}

// ---------------------------------------------------------------------
// Member function get_image_filename_corresponding_to_symbol_type()
// returns the name of a hardwired ArmySymbol image based upon the
// current value of symbol_type:

string ArmySymbol::get_image_filename_corresponding_to_symbol_type()
{
   string osg_data_dir(getenv("OSG_FILE_PATH"));
   osg_data_dir += "/army_symbols/";

   string image_filename;
   switch (symbol_type)
   {
      case 0:
         image_filename=osg_data_dir+"blue_symbol1.png";
         break;
      case 1:
         image_filename=osg_data_dir+"blue_symbol2.png";
         break;
      case 2:
         image_filename=osg_data_dir+"blue_symbol3.png";
         break;
      case 3:
         image_filename=osg_data_dir+"blue_symbol4.png";
         break;
      case 4:
         image_filename=osg_data_dir+"blue_symbol5.png";
         break;
      case 5:
         image_filename=osg_data_dir+"red_symbol1.png";
         break;
      case 6:
//         image_filename=osg_data_dir+"Top-arrow-256.png";
         image_filename=osg_data_dir+"red_arrow.png";
         break;
      case 7:
         image_filename=osg_data_dir+"grey_arrow.png";
//         image_filename=osg_data_dir+"yellow_arrow.png";
//         image_filename=osg_data_dir+"cyan_arrow.png";
         break;
      default:
         image_filename=osg_data_dir+"red_symbol2.png";
         break;
   }

//   cout << "image_filename = " << image_filename << endl;
   return image_filename;
}
