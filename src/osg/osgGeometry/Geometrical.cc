// ==========================================================================
// Geometrical class member function definitions
// ==========================================================================
// Last updated on 9/9/11; 10/12/11; 3/26/13
// ==========================================================================

#include <string>
#include <osg/Geode>
#include "osg/osgGraphicals/AnimationController.h"
#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "math/constant_vectors.h"
#include "osg/osgGeometry/Geometrical.h"
#include "math/statevector.h"
#include "math/threevector.h"
#include "time/timefuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Geometrical::allocate_member_objects()
{
}		       

void Geometrical::initialize_member_objects()
{
   Graphical_name="Geometrical";
   reference_origin=Zero_vector;
   on_earth_flag=false;
   blinking_flag=false;
   blinking_color_calculated=false;
   single_blink_duration=0.25;	// secs
   max_blink_period=3.0;	// secs
   multicolor_flag=false;
   
//   ellipsoid_magnification_factor=100;
   ellipsoid_magnification_factor=1000;
//   ellipsoid_magnification_factor=10000;

   selected_color=colorfunc::get_OSG_color(colorfunc::green);
//   selected_color=colorfunc::get_OSG_color(colorfunc::purple);
   
   right_neighbor_ID=left_neighbor_ID=-1;

   n_text_messages=0;
   text_character_size=1.0;

   permanent_text_color=colorfunc::get_OSG_color(colorfunc::white);
//   permanent_text_color=colorfunc::get_OSG_color(colorfunc::red);

   track_ptr=NULL;

   x_dir=osg::Vec3(1,0,0);
}		       

Geometrical::Geometrical():
   Graphical()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

Geometrical::Geometrical(
   const int p_ndims,int id,AnimationController* AC_ptr):
   Graphical(p_ndims,id,AC_ptr)
{	
//    cout << "inside Geometrical constructor" << endl;
   initialize_member_objects();
   allocate_member_objects();
}		       

Geometrical::~Geometrical()
{

/*
   cout << "inside Geometrical destructor" << endl;
   if (vertices_refptr.valid())
   {
      cout << "vertices_refptr.get() = " << vertices_refptr.get() << endl;
      cout << "vertices_refptr->referenceCount() = "
           << vertices_refptr->referenceCount() << endl;
   }
   if (color_array_refptr.valid())
   {
      cout << "color_array_refptr.get() = " 
           << color_array_refptr.get() << endl;
      cout << "color_array_refptr->referenceCount() = "
           << color_array_refptr->referenceCount() << endl;
   }
   if (geode_refptr.valid())
   {
      cout << "geode_refptr.get() = " 
           << geode_refptr.get() << endl;
      cout << "geode_refptr->referenceCount() = "
           << geode_refptr->referenceCount() << endl;
   }
   for (int i=0; i<text_refptr.size(); i++)
   {
      cout << "i = " << i 
           << " text_refptr[i].get() = " << text_refptr[i].get()
           << " text_refptr[i]->referenceCount() = "
           << text_refptr[i]->referenceCount() << endl;
   }
   cout << "At end of Geometrical destructor" << endl;
*/

}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Geometrical& f)
{
   outstream << static_cast<const Graphical&>(f) << endl;
//   outstream << "coords_list = " << *(f.coords_list_ptr) << endl;
   return outstream;
}

// ---------------------------------------------------------------------
// Member function copy_size() transfers scale, size and text size
// parameters from *input_Geometrical_ptr to *this.

void Geometrical::copy_size(
   double curr_t,int curr_passnumber,const Geometrical* input_Geometrical_ptr)
{
//   cout << "inside Geometrical::copy_size()" << endl;

   threevector scale;
   input_Geometrical_ptr->get_scale(curr_t,curr_passnumber,scale);
   set_scale(curr_t,curr_passnumber,scale);
   
   set_size(input_Geometrical_ptr->get_size());
   for (unsigned int m=0; m<input_Geometrical_ptr->get_n_text_messages(); m++)
   {
      set_text_size(m,input_Geometrical_ptr->get_text_size(m));
   }
}

// ==========================================================================
// Color manipulation member functions
// ==========================================================================

// Member function set_curr_color 

void Geometrical::set_curr_color(const colorfunc::Color& c,double alpha)
{
//   cout << "inside Geometrical::set_curr_color(), alpha = " << alpha << endl;
//   cout << "inside Geometrical::set_curr_color(), c = " << c << endl;

// FAKE FAKE:  Sat Dec 17 at 10 am...for testing only we hardwire alpha=0.5

//   alpha=1.0;
//   alpha=0.8;
//   alpha=0.5;
//   alpha=0.3;
   set_curr_color(colorfunc::get_OSG_color(c,alpha));
}

void Geometrical::set_curr_color(const osg::Vec4& color)
{
   curr_color=color;
/*
   if (get_name()=="Polygon")
   {
      cout << "inside Geometrical::set_curr_color(vec4)" << endl;
      cout << "Graphical name = " << get_name() 
           << " ID = " << get_ID() << endl;
      cout << "this = " << this << endl;

      cout << "curr_color = " << curr_color.r() << " "
           << curr_color.g() << " " << curr_color.b() << " "
           << curr_color.a() << endl;
      cout << "color_array_refptr.valid() = "
           << color_array_refptr.valid() << endl;
      cout << "color_array_refptr.size() = "
           << color_array_refptr->size() << endl;
      outputfunc::enter_continue_char();
   }
*/

   if (color_array_refptr.valid())
   {
      for (int n=0; n<int(color_array_refptr->size()); n++)
      {
//         cout << "n = " << n << endl;
//         cout << "r = " << curr_color.r() 
//              << " g = " << curr_color.g()
//              << " b = " << curr_color.b()
//              << " a = " << curr_color.a() << endl;
         color_array_refptr->at(n)=curr_color;
      }
      dirtyDisplay();
   }
}

// ---------------------------------------------------------------------
// Dummy member function set_color which should be overloaded for
// certain specific classes like SignPost, Cylinder and OBSFRUSTUM.

void Geometrical::set_color(const colorfunc::Color& c)
{
   cout << "inside Geometrical::set_color(colorfunc::Color) dummy method" 
        << endl;
}

void Geometrical::set_color(const osg::Vec4& color)
{
   cout << "inside Geometrical::set_color(osg::Vec4 color) dummy method" 
        << endl;
}

void Geometrical::set_colors(const vector<osg::Vec4>& colors)
{
   cout << 
      "inside Geometrical::set_colors(const vector<osg::Vec4>& colors) dummy method" 
        << endl;
}

// ---------------------------------------------------------------------
osg::Vec4& Geometrical::get_curr_blinking_color()
{
//   cout << "inside Geometrical::get_curr_blinking_color()" << endl;
//   cout << "Geometrical name = " << get_name() << endl;
//   cout << "blinking_flag = " << blinking_flag << endl;
   
   if (!blinking_flag) return permanent_color;

   if (!blinking_color_calculated) compute_blinking_color(permanent_color);

   double delta_t=timefunc::elapsed_timeofday_time()-blinking_start_time;
//   cout << "elapsed timeofday = " << timefunc::elapsed_timeofday_time() 
//        << endl;
//   cout << "blinking_start_time = " << blinking_start_time << endl;
//   cout << "delta_t = " << delta_t << endl;
//   cout << "max_blink_period = " << max_blink_period << endl;

   if (delta_t > max_blink_period)
   {
      blinking_flag=false;
      return permanent_color;
   }
   
   int n_blink=static_cast<int>(delta_t/single_blink_duration);
//   cout << "single_blink_duration = " << single_blink_duration << endl;
//   cout << "n_blink = " << n_blink << endl;
//   cout << "blinking_color = " << endl;
//   osgfunc::print_Vec4(blinking_color);
//   cout << "permanent color = " << endl;
//   osgfunc::print_Vec4(permanent_color);

   if (is_odd(n_blink))
   {
      return blinking_color;
   }
   else
   {
      return permanent_color;
   }
}

// ---------------------------------------------------------------------
// Member function time_to_switch_multicolors_to_blinking_color() is a
// variant of the preceding get_curr_blinking_color() method.  It
// returns false if a multi-colored Geometrical (such as a PolyLine
// locally colored according to speed) should NOT be recolored.  It
// returns true if the Geometrical should have its multiple colors
// reset to the global blinking_color.  The latter can be retrieved
// via a subsequent call to get_blinking_color().

bool Geometrical::time_to_switch_multicolors_to_blinking_color()
{
//   cout << "inside Geometrical::time_to_switch_multicolors_to_blinking_color()" << endl;
//   cout << "Geometrical name = " << get_name() << endl;
   
//   cout << "blinking_flag = " << blinking_flag << endl;

   if (!blinking_flag) return false;

   osg::Vec4 base_color=permanent_color;
   if (!blinking_color_calculated) compute_blinking_color(base_color);

   double delta_t=timefunc::elapsed_timeofday_time()-blinking_start_time;
   if (delta_t > max_blink_period)
   {
      blinking_flag=false;
      return false;
   }
   
   int n_blink=static_cast<int>(delta_t/single_blink_duration);
   return (is_odd(n_blink));
}

// ---------------------------------------------------------------------
void Geometrical::compute_blinking_color(const osg::Vec4& base_color)
{
//   cout << "inside Geometrical::compute_blinking_color()" << endl;
   double h,s,v;
   colorfunc::RGB_to_hsv(
      base_color.r(),base_color.g(),base_color.b(),h,s,v);
   if (v > 0.5)
   {
      v *= 0.5;
   }
   else
   {
      v *= 2.0;
   }

   double new_r,new_g,new_b;
   colorfunc::hsv_to_RGB(h,s,v,new_r,new_g,new_b);
   blinking_color=osg::Vec4(new_r,new_g,new_b,base_color.a());
   blinking_color_calculated=true;
//   cout << "blinking_color = " << endl;
//   osgfunc::print_Vec4(blinking_color);
//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function dirtyDisplay loops over all Drawables within the
// Geometrical's geode member and dirties their Display Lists.  This
// forces each drawable to be updated in the next draw update.  Call
// this method whenever some internal Geometrical aspect (e.g. color,
// size) has been changed.

void Geometrical::dirtyDisplay()
{
//   cout << "inside Geometrical::dirtyDisplay()" << endl;
//   cout << "geode_refptr.valid() = " << geode_refptr.valid() << endl;
   if (geode_refptr.valid())
   {
//      cout << "num drawables = " << geode_refptr->getNumDrawables() << endl;
      for (unsigned int n=0; n<geode_refptr->getNumDrawables(); n++)
      {
         geode_refptr->getDrawable(n)->dirtyDisplayList();
         geode_refptr->getDrawable(n)->dirtyBound();
      }
   } // geode_refptr.valid() conditional
}

// ==========================================================================
// Text member functions
// ==========================================================================

bool Geometrical::get_text_refptr_valid(int i)
{
//   cout << "inside Geometrical::get_text_refptr_valid()" << endl;
   if (text_refptr.size()==0) 
   {
      return false;
   }
   else
   {
      return text_refptr[i].valid();
   }
}

void Geometrical::set_text_label(int i,string label)
{
//   cout << "inside Geometrical::set_text_label(), i = " << i 
//        << " label = " << label  << endl;
//   cout << "get_name() = " << get_name() << endl;
   if (text_refptr.size() > 0 && text_refptr[i].valid())
   {
      text_refptr[i]->setText(label);
   }
}

string Geometrical::get_text_label(int i)
{
//   cout << "inside Geometrical::get_text_label()" << endl;
   if (text_refptr.size() > 0 && text_refptr[i].valid())
   {
      return text_refptr[i]->getText().createUTF8EncodedString();
   }
   else
   {
      return "";
   }
}

// ---------------------------------------------------------------------
void Geometrical::set_text_posn(int i,const threevector& posn)
{
//   cout << "inside Geometrical::set_text_posn(), i = " << i
//        << " posn = " << posn << endl;
//   cout << "text_refptr.size() = " << text_refptr.size() << endl;

   if (text_refptr.size() > 0 && text_refptr[i].valid())
   {
      text_refptr[i]->setPosition(
         osg::Vec3(posn.get(0),posn.get(1),posn.get(2)));
   }
}

// ---------------------------------------------------------------------
threevector Geometrical::get_text_posn(int i)
{
//   cout << "inside Geometrical::get_text_posn(), i = " << i
//        << " posn = " << posn << endl;
//   cout << "text_refptr.size() = " << text_refptr.size() << endl;

   if (text_refptr.size() > 0 && text_refptr[i].valid())
   {
      osg::Vec3 posn=text_refptr[i]->getPosition();
      return threevector(posn);
   }
   else
   {
      return threevector(NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
   }
}

// ---------------------------------------------------------------------
void Geometrical::set_text_size(int i,double size)
{
//   cout << "inside Geometrical::set_text_size()" << endl;
   if (text_refptr.size() > 0 && text_refptr[i].valid())
   {
      text_refptr[i]->setCharacterSize(size);
   }
}

// ---------------------------------------------------------------------
void Geometrical::change_text_size(int i,double factor)
{
   if (text_refptr.size() > 0 && text_refptr[i].valid())
   {
      text_refptr[i]->setCharacterSize(
         text_refptr[i]->getCharacterHeight()*factor);
      text_refptr[i]->setMaximumWidth(
         text_refptr[i]->getMaximumWidth()*factor);
   }
}

void Geometrical::change_text_size(osgText::Text* text_ptr,double factor)
{
//   cout << "inside Geometrical::change_text_size()" << endl;
//   cout << "text_ptr = " << text_ptr << " factor = " << factor << endl;

   if (text_ptr != NULL) 
   {
      text_ptr->setCharacterSize(text_ptr->getCharacterHeight()*factor);
      text_ptr->setMaximumWidth(text_ptr->getMaximumWidth()*factor);

//      cout << "char size = " << text_ptr->getCharacterHeight() << endl;
//      cout << "max text width = " << text_ptr->getMaximumWidth() << endl;
   }
}		       

// ---------------------------------------------------------------------
double Geometrical::get_text_size(int i) const
{
//   cout << "inside Geometrical::get_text_size(), i = " << i << endl;

   if (text_refptr.size() > 0 && text_refptr[i].valid())
   {
      return text_refptr[i]->getCharacterHeight();
   }
   else
   {
      return NEGATIVEINFINITY;
   }
}

// ---------------------------------------------------------------------
void Geometrical::set_text_direction(int i,const threevector& text_dir)
{
   if (text_refptr.size() > 0 && text_refptr[i].valid())
   {
      text_Q.makeRotate(
         x_dir,osg::Vec3(text_dir.get(0),text_dir.get(1),text_dir.get(2)));
      text_refptr[i]->setRotation(text_Q);
   }
}

// ---------------------------------------------------------------------
void Geometrical::set_text_rotation(int i,double chi)
{
   if (text_refptr.size() > 0 && text_refptr[i].valid())
   {
      osg::Quat q(chi,osg::Vec3(
         z_hat.get(0),z_hat.get(1),z_hat.get(2)));
      text_refptr[i]->setRotation(q);
   }
}

// ---------------------------------------------------------------------
// Member function set_text_color performs the actual setting of the
// text line labeled by integer i with the input color (which may
// correspond to the permanent_text_color, selected_text_color, etc).

void Geometrical::set_text_color(int i,colorfunc::Color text_color)
{
   set_text_color(i,colorfunc::get_OSG_color(text_color));
}

void Geometrical::set_text_color(int i,const osg::Vec4& color)
{
//   cout << "inside Geometrical::set_text_color, i = " << i << endl;
//   cout << "text_refptr.size() = " << text_refptr.size() << endl;
   
//   cout << "color = " << endl;
//   osgfunc::print_Vec4(color);

   if (i < int(text_refptr.size()))
   {
      if (text_refptr[i].valid())
      {
         text_refptr[i]->setColor(color);
      }
   }
}

void Geometrical::set_text_color(const osg::Vec4& color)
{
   for (unsigned int i=0; i<get_n_text_messages(); i++)
   {
      set_text_color(i,color);
   }
}

// ---------------------------------------------------------------------
// Member function initialize_text

void Geometrical::initialize_text(int i)
{
//   cout << "inside Geometrical::initialize_text()" << endl;
//   cout << "this = " << this << endl;
   
// Important note!  In early January 2007, we discovered the very hard
// and painful way that the following line leads to a
// very-difficult-to-detect memory error:

//   text_refptr[i]->setFont("fonts/times.ttf");

// According to the OSG user's list circa October 2006, the
// setFont(string) command can sometimes return NULL osgText::Font*
// pointers.  This results in unpredictable and irreproducible program
// segmentation faults.

// To circumvent such disasters, we call setFont(string) just once
// within GeometricalsGroup and pass its pointer into text_refptr below:

   if (font_refptr.valid())
   {
      text_refptr[i]->setFont(font_refptr.get()); 
   }
   else
   {
      cout << "Trouble in Geometrical::initialize_text()" << endl;
      cout << "font_refptr.valid() = FALSE !!" << endl;
      cout << "Geometrical.get_name() = " << get_name() << endl;
   }

// Uncomment out next two lines for long/lat text displays on blue
// marble:

//   text_refptr[i]->setAxisAlignment(osgText::Text::SCREEN);
//   text_refptr[i]->setCharacterSizeMode(osgText::Text::SCREEN_COORDS);

//   text_refptr[i]->setCharacterSizeMode(
//      osgText::Text::
//      OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);

   text_refptr[i]->setAlignment(osgText::Text::CENTER_CENTER);
//   text_refptr[i]->setAlignment(osgText::Text::CENTER_BOTTOM);

   text_refptr[i]->setBackdropType(osgText::Text::OUTLINE);

// Set maximum width of text box to force long text strings to wrap
// around into several smaller, more readable lines:

//   text_refptr[i]->setMaximumWidth(50*get_text_character_size());

   set_text_color(i,permanent_text_color);
}

// ==========================================================================
// ActiveMQ broadcast member functions
// ==========================================================================

void Geometrical::broadcast_KOZ_bbox(
   int ID,double t,const bounding_box& bbox,
   const threevector* grid_origin_ptr,Messenger* Messenger_ptr)
{
//   cout << "inside Geometrical::broadcast_KOZ_bbox()" << endl;
//   cout << "this = " << this << endl;

// Recall that ActiveMQ messages consist of a single command string
// along with an STL vector of key-value string pair properties:

   string command,key,value;
   vector<Messenger::Property> properties;
   command="SEND_KOZ_BBOX";	
 
   key="Type";
   value="KOZ_bbox";
   properties.push_back(Messenger::Property(key,value));

   key="ID";
   value=stringfunc::number_to_string(ID);
   properties.push_back(Messenger::Property(key,value));

   key="Time";
   value=stringfunc::number_to_string(t);
   properties.push_back(Messenger::Property(key,value));

   key="Xlo";
   value=stringfunc::number_to_string(
      bbox.get_xmin()-grid_origin_ptr->get(0));
   properties.push_back(Messenger::Property(key,value));

   key="Ylo";
   value=stringfunc::number_to_string(
      bbox.get_ymin()-grid_origin_ptr->get(1));
   properties.push_back(Messenger::Property(key,value));

   key="Zlo";
   value=stringfunc::number_to_string(
      bbox.get_zmin()-grid_origin_ptr->get(2));
   properties.push_back(Messenger::Property(key,value));

   key="Xhi";
   value=stringfunc::number_to_string(
      bbox.get_xmax()-grid_origin_ptr->get(0));
   properties.push_back(Messenger::Property(key,value));

   key="Yhi";
   value=stringfunc::number_to_string(
      bbox.get_ymax()-grid_origin_ptr->get(1));
   properties.push_back(Messenger::Property(key,value));

   key="Zhi";
   value=stringfunc::number_to_string(
      bbox.get_zmax()-grid_origin_ptr->get(2));
   properties.push_back(Messenger::Property(key,value));

   key="Priority";
   value=stringfunc::number_to_string(1.0);
   properties.push_back(Messenger::Property(key,value));

//   for (int p=0; p<properties.size(); p++)
//   {
//      Messenger::Property curr_property(properties[p]);
//      cout << p << "  "
//           << "key = " << curr_property.first
//           << " value = " << curr_property.second
//           << endl;
//   }

   Messenger_ptr->broadcast_subpacket(command,properties);
}
