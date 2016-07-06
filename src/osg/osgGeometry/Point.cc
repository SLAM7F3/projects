// Note added on 9/29/09: Not sure if Point::set_posn() is ever used
// or needed.  Use Graphical::set_UVW_coords() instead...

// ==========================================================================
// POINT class member function definitions
// ==========================================================================
// Last modified on 4/14/10; 12/29/10; 11/6/11
// ==========================================================================

#include <osg/Geode>
#include <osg/LineWidth>
#include <string>
#include "osg/osgGraphicals/AnimationController.h"
#include "math/basic_math.h"
#include "osg/osgGeometry/Point.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::pair;
using std::string;

namespace osgGeometry
{

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

   void Point::allocate_member_objects()
      {
      }		       

   void Point::initialize_member_objects()
      {
         Graphical_name="Point";
         n_text_messages=1;
         for (unsigned int m=0; m<get_n_text_messages(); m++)
         {
            text_refptr.push_back(static_cast<osgText::Text*>(NULL));
         }
         set_dim_dependent_colors();
      }		       

   void Point::set_dim_dependent_colors()
      {
         if (get_ndims()==2)
         {
//            set_permanent_color(colorfunc::get_OSG_color(
//               colorfunc::brightcyan));
            set_permanent_color(colorfunc::get_OSG_color(
               colorfunc::brightpurple));
            set_selected_color(colorfunc::get_OSG_color(
               colorfunc::orange));
         }
         else if (get_ndims()==3)
         {
//            set_permanent_color(colorfunc::get_OSG_color(colorfunc::red));
            set_permanent_color(colorfunc::get_OSG_color(colorfunc::white));
            set_selected_color(colorfunc::get_OSG_color(
               colorfunc::brightpurple));
         }
      }		       

   Point::Point(const int p_ndims,int id,AnimationController* AC_ptr):
      Geometrical(p_ndims,id,AC_ptr)
      {	
//          cout << "inside Point constructor" << endl;
         initialize_member_objects();
         allocate_member_objects();
      }		       

   Point::~Point()
      {
      }

// ---------------------------------------------------------------------
// Overload << operator

   ostream& operator<< (ostream& outstream,const Point& p)
      {
         outstream << "inside Point::operator<<, this = " << &p << endl;
         outstream << "Uhat = " << p.Uhat << endl;
         outstream << "Vhat = " << p.Vhat << endl;
         outstream << "What = " << p.What << endl;

         outstream << static_cast<const Geometrical&>(p) << endl;
         return outstream;
      }

// ==========================================================================
// Drawing member functions
// ==========================================================================

   osg::Geode* Point::generate_drawable_geode(
      int passnumber,double crosshairs_size,double crosshairs_text_size,
      bool draw_text_flag,bool earth_feature_flag)
      {
//         cout << "inside Point::generate_drawable_geode()" << endl;

         get_UVW_dirs(get_initial_t(),passnumber,Uhat,Vhat,What);

         on_earth_flag=earth_feature_flag;
         geode_refptr = new osg::Geode();
         geode_refptr->addDrawable(generate_drawable_geom(crosshairs_size));

         if (draw_text_flag)
         {
            geode_refptr->addDrawable(
               generate_crosshairsnumber_text(crosshairs_text_size));
         }

         osg::LineWidth* linewidth = new osg::LineWidth();
         float line_thickness=1.0;
//         float line_thickness=3;	// viewgraphs

         if (get_ndims()==3)
         {
            line_thickness=5.0f;
         }
         linewidth->setWidth(line_thickness);
         osg::StateSet* stateset_ptr=geode_refptr->getOrCreateStateSet();
         stateset_ptr->setAttributeAndModes(
            linewidth,osg::StateAttribute::ON);

         return geode_refptr.get();
      }

// ---------------------------------------------------------------------
// Member function generate_drawable_geom instantiates an
// osg::Geometry consisting of ndims perpendicular lines.  If
// ndims==2,the crosshairs' y value is set to a tiny negative value so
// that it lies in front of an image with canonical y=0 values.
// (Recall x points towards the horizontal right while z points
// vertically upwards for OpenGL images.)

   osg::Geometry* Point::generate_drawable_geom(double crosshairs_size)
      {
//         cout << "inside Point::generate_drawable_geom()" << endl;

         osg::Geometry* geom_ptr=new osg::Geometry;
         const int n_vertices=2*get_ndims();

         vertices_refptr = new osg::Vec3Array(n_vertices);
         geom_ptr->setVertexArray(vertices_refptr.get());
         set_crosshairs_coords(crosshairs_size);

         color_array_refptr = new osg::Vec4Array(n_vertices);
         geom_ptr->setColorArray(color_array_refptr.get());
         geom_ptr->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

         geom_ptr->addPrimitiveSet(new osg::DrawArrays(
            osg::PrimitiveSet::LINES,0,n_vertices));
         return geom_ptr;
      }

// ---------------------------------------------------------------------
// Member function set_crosshairs_coords takes in a pointer to an
// array of 2*ndims osg::Vec3s.  It sets these vectors to equal the
// crosshairs tip locations in the XZ plane for ndims=2.

   void Point::set_crosshairs_coords(double crosshairs_size)
      {
//         cout << "inside Point::set_crosshairs_coords()" << endl;
//         cout << "Uhat = " << Uhat << " Vhat = " << Vhat
//              << " What = " << What << endl;
//         cout << "on_earth_flag = " << on_earth_flag << endl;

         osg::Vec3 origin(0,0,0);
         if (on_earth_flag) 
         {
            crosshairs_size *= ellipsoid_magnification_factor;
            const float SMALL_POS=0.1*ellipsoid_magnification_factor;
            origin=osg::Vec3(SMALL_POS*What.get(0),SMALL_POS*What.get(1),
                             SMALL_POS*What.get(2));
         }

         osg::Vec3 Udir(
            crosshairs_size*Uhat.get(0),crosshairs_size*Uhat.get(1),
            crosshairs_size*Uhat.get(2));
         osg::Vec3 Vdir(
            crosshairs_size*Vhat.get(0),crosshairs_size*Vhat.get(1),
            crosshairs_size*Vhat.get(2));
         osg::Vec3 Wdir(
            crosshairs_size*What.get(0),crosshairs_size*What.get(1),
            crosshairs_size*What.get(2));

         if (vertices_refptr->size()==4)	// ndims==2
         {

// In order for crosshairs to lie slightly in front of a 2D image
// which is located in the y=0 plane, we set its y value to a tiny
// negative value:

            const float TINY_NEG=-1E-4;
            origin=osg::Vec3(0.0f,TINY_NEG,0.0f);

            vertices_refptr->at(0)=origin-Udir;
            vertices_refptr->at(1)=origin+Udir;
            vertices_refptr->at(2)=origin-Wdir;
            vertices_refptr->at(3)=origin+Wdir;
         }
         else if (vertices_refptr->size()==6)	// ndims==3
         {
            vertices_refptr->at(0)=origin-Udir;
            vertices_refptr->at(1)=origin+Udir;
            vertices_refptr->at(2)=origin-Vdir;
            vertices_refptr->at(3)=origin+Vdir;
            vertices_refptr->at(4)=origin-Wdir;
            vertices_refptr->at(5)=origin+Wdir;
         }
         else
         {
            cout << "Error in Point::set_crosshairs_coords()" << endl;
            cout << "vertices_refptr->size() = "
                 << vertices_refptr->size() << endl;
            exit(-1);
         }
         dirtyDisplay();
      }

// ---------------------------------------------------------------------
// Member function set_crosshairs_color sets the color of the current
// object plus its attendant text label based upon the input RGBA
// information.

   void Point::set_color(const osg::Vec4& color)
      {
         set_crosshairs_color(color);
      }
   
   void Point::set_crosshairs_color(const osg::Vec4& color)
      {
//         cout << "inside Point::set_crosshairs_color()" << endl;

         if (text_refptr[0] != NULL) text_refptr[0]->setColor(color);
         set_curr_color(color);     

//         dirtyDisplay();
      }

// ---------------------------------------------------------------------
// Member function generate_crosshairsnumber_text generates an
// osg::Group with a text label for the input integer ID.

   osgText::Text* Point::generate_crosshairsnumber_text(double text_size)
      {
//         cout << "inside Point::generate_crosshairsnumber_text()" << endl;
         
         if (on_earth_flag) text_size *= ellipsoid_magnification_factor;

         text_refptr[0] = new osgText::Text;
         reset_text_label();

// For reasons we do not understand as of 7/21/05, we cannot read font
// information for two different windows (which we need when running
// the main fusion program) from the same font file.  So we have
// copied the arial.ttf file onto arial_2D.ttf and arial_3D.ttf.  We
// execute an osgText::readFontFile call to these two different copies
// of the same font file depending upon the value of input parameter
// ndims...

         string font_filename="fonts/arial"+ndims_label+".ttf";
         osgText::Font* font = osgText::readFontFile(font_filename);
   
         text_refptr[0]->setFont(font);
         text_refptr[0]->setAxisAlignment(osgText::Text::SCREEN);

         text_refptr[0]->setCharacterSize(text_size);
         set_crosshairsnumber_text_posn(text_size);

         if (on_earth_flag && get_ndims()==3) 
         {
            const float SMALL_POS=text_size/16*11.0;
            text_refptr[0]->setPosition(osg::Vec3(
               SMALL_POS*What.get(0),SMALL_POS*What.get(1),
               SMALL_POS*What.get(2)));
         }

         return text_refptr[0].get();
      }

// ---------------------------------------------------------------------
// Member function set_crosshairsnumber_text_posn

   void Point::set_crosshairsnumber_text_posn(double text_size)
      {
         const float TINY_NEG=-1E-4;
         if (!text_refptr[0].valid()) return;

         if (get_ndims()==2)
         {
            text_refptr[0]->setPosition(osg::Vec3(
               0.3*text_size,TINY_NEG,0.3*text_size));

// On 4/14/10, we empirically found that we need to offset crosshair
// IDs for photos shot in portrait mode which are rotated by 90 so
// that they do not interfere with the crosshairs themselves:

//            text_refptr[0]->setPosition(osg::Vec3(
//               text_size,TINY_NEG,0.3*text_size));
         }
         else if (get_ndims()==3)
         {
            text_refptr[0]->setPosition(osg::Vec3(
               0.3*text_size,0.3*text_size,0.3*text_size));

// Next line prevents text size from growing too large and text
// appearing fuzzy as one zooms in towards it:

//      text_refptr[0]->setCharacterSizeMode(
//         osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
         }
      }

// ---------------------------------------------------------------------
// Member function reset_text_label resets the text label to equal the
// Point's current integer ID.

   void Point::reset_text_label()
      {
         set_text_label(0,stringfunc::number_to_string(ID));
      }

   void Point::reset_text_label(string label)
      {
         set_text_label(0,label);
      }

// ==========================================================================
// Point manipulation methods
// ==========================================================================

   void Point::set_posn(double curr_t,int pass_number,const threevector& V)
      {
//         cout << "inside Point::set_posn()" << endl;

         set_UVW_coords(curr_t,pass_number,V);

         threevector UVW;
         get_UVW_coords(curr_t,pass_number,UVW);
//         cout << "UVW = " << UVW << endl;
      }

} // osgGeometry namespace
