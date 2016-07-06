// ==========================================================================
// FEATURE class member function definitions
// ==========================================================================
// Last modified on 6/21/07; 10/14/07; 10/21/07; 8/19/09; 1/21/13
// ==========================================================================

#include <osgText/Font>
#include <osg/Geode>
#include <osg/LineWidth>
#include <string>
#include "osg/osgGraphicals/AnimationController.h"
#include "math/basic_math.h"
#include "astro_geo/Clock.h"
#include "osg/osgFeatures/Feature.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::pair;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Feature::allocate_member_objects()
{
}		       

void Feature::initialize_member_objects()
{
   Graphical_name="Feature";
   set_dim_dependent_colors();
   n_text_messages=2;
   for (unsigned int m=0; m<get_n_text_messages(); m++)
   {
      text_refptr.push_back(static_cast<osgText::Text*>(NULL));
   }
}		       

void Feature::set_dim_dependent_colors()
{
//   cout << "inside Feature::set_dim_dependent_colors()" << endl;

   if (get_ndims()==2)
   {
      set_permanent_color(colorfunc::get_OSG_color(	// conventional
         colorfunc::brightpurple));
//      set_permanent_color(colorfunc::get_OSG_color(	// movies/viewgraphs
//         colorfunc::green));
//         colorfunc::yellow));
//         colorfunc::red));
//         colorfunc::white));
      set_selected_color(colorfunc::get_OSG_color(	// conventional
         colorfunc::red));
   }
   else if (get_ndims()==3)
   {
      set_permanent_color(colorfunc::get_OSG_color(colorfunc::white));
						      // conventional
      set_selected_color(colorfunc::get_OSG_color(
                            colorfunc::brightpurple));	// conventional
//      set_permanent_color(colorfunc::get_OSG_color(colorfunc::red)); 
				// viewgraph
//      set_selected_color(colorfunc::get_OSG_color(
//                            colorfunc::red));	     // viewgraph
   }
}		       

Feature::Feature(const int p_ndims,int id,AnimationController* AC_ptr):
   osgGeometry::Point(p_ndims,id,AC_ptr), Annotator(p_ndims,id)
{	
//   cout << "inside Feature constructor" << endl;
   initialize_member_objects();
   allocate_member_objects();
}		       

Feature::~Feature()
{
//   cout << "inside Feature destructor" << endl;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Feature& f)
{
   outstream << "inside Feature::operator<<" << endl;
   outstream << static_cast<const osgGeometry::Point&>(f) << endl;
   return(outstream);
}

// ==========================================================================
// Drawing methods
// ==========================================================================

osg::Geode* Feature::generate_drawable_geode(
   int passnumber,double crosshairs_size,double crosshairs_text_size,
   bool earth_feature_flag)
{
//   cout << "inside Feature::generate_drawable_geode()" << endl;
//   cout << "earth_feature_flag = " << earth_feature_flag << endl;

   bool draw_text_flag=true;
   geode_refptr=osgGeometry::Point::generate_drawable_geode(
      passnumber,crosshairs_size,crosshairs_text_size,
      draw_text_flag,earth_feature_flag);
   geode_refptr->addDrawable(
      generate_nimages_appearance_text(crosshairs_text_size));
   return geode_refptr.get();
}

// ---------------------------------------------------------------------
// Member function generate_nimages_appearance_text generates an
// osg::Group with a text label for the number of images in which the
// Feature is not erased.

osgText::Text* Feature::generate_nimages_appearance_text(double text_size)
{
//   cout << "inside Feature::generate_nimages_appearance_text()" 
//        << endl;
   
   if (on_earth_flag) text_size *= ellipsoid_magnification_factor;

   text_refptr[1] = new osgText::Text;

// For reasons we do not understand as of 7/21/05, we cannot read font
// information for two different windows (which we need when running
// the main fusion program) from the same font file.  So we have
// copied the arial.ttf file onto arial_2D.ttf and arial_3D.ttf.  We
// execute an osgText::readFontFile call to these two different copies
// of the same font file depending upon the value of input parameter
// ndims...

   string font_filename="fonts/arial"+ndims_label+".ttf";
   osgText::Font* font = osgText::readFontFile(font_filename);
   
   text_refptr[1]->setFont(font);
   text_refptr[1]->setAxisAlignment(osgText::Text::SCREEN);
   text_refptr[1]->setCharacterSize(text_size);
   set_nimages_appearance_text_posn(text_size);

   return text_refptr[1].get();
}

// ---------------------------------------------------------------------
// Member function set_nimages_appearance_text_posn

void Feature::set_nimages_appearance_text_posn(double text_size)
{
   const float TINY_NEG=-1E-4;

   if (get_ndims()==2)
   {
      text_refptr[1]->setPosition(osg::Vec3(
         -1.2*text_size,TINY_NEG,-1.0*text_size));
   }
   else if (get_ndims()==3)
   {
      text_refptr[1]->setPosition(osg::Vec3(
         -0.9*text_size,-0.9*text_size,0.3*text_size));
   }
}

