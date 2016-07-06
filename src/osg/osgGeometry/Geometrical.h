// ==========================================================================
// Header file for (pure virtual) Geometrical class
// ==========================================================================
// Last updated on 2/24/11; 9/9/11; 10/12/11
// ==========================================================================

#ifndef GEOMETRICAL_H
#define GEOMETRICAL_H

#include <iostream>
#include <vector>
#include <osgText/Font>
#include <osgText/Text>
#include "color/colorfuncs.h"
#include "osg/osgGraphicals/Graphical.h"
#include "messenger/Messenger.h"
#include "osg/osgfuncs.h"
#include "track/track.h"

class AnimationController;
class bounding_box;
class statevector;
class threevector;

class Geometrical : public Graphical
{

  public:

// Initialization, constructor and destructor functions:

   Geometrical();
   Geometrical(const int p_ndims,int id,AnimationController* AC_ptr=NULL);
   virtual ~Geometrical();
   friend std::ostream& operator<< (
      std::ostream& outstream,const Geometrical& f);

// Set & get member functions:

   void set_reference_origin(const threevector& v_ref);
   threevector& get_reference_origin();
   const threevector& get_reference_origin() const;

   void set_blinking_flag(bool flag);
   bool get_blinking_flag() const;
   void set_blinking_start_time(double t);
   void set_single_blink_duration(double t);
   void set_max_blink_period(double t);

   void set_multicolor_flag(bool flag);
   bool get_multicolor_flag() const;

   int get_n_vertices() const;
   void set_permanent_color(const colorfunc::Color& c,double alpha=1.0);
   void set_permanent_color(const osg::Vec4& color);
   osg::Vec4& get_permanent_color();
   const osg::Vec4& get_permanent_color() const;

   void set_permanent_text_color(const colorfunc::Color& c,double alpha=1.0);
   void set_permanent_text_color(const osg::Vec4& color);
   osg::Vec4& get_permanent_text_color();
   const osg::Vec4& get_permanent_text_color() const;

   void set_curr_color(const colorfunc::Color& c,double alpha=1.0);
   void set_curr_color(const osg::Vec4& curr_color);
   osg::Vec4& get_curr_color();
   const osg::Vec4& get_curr_color() const;
   osg::Vec4Array* get_color_array_ptr();

   void set_selected_color(const colorfunc::Color& c,double alpha=1.0);
   void set_selected_color(const osg::Vec4& s_color);
   osg::Vec4& get_selected_color();
   const osg::Vec4& get_selected_color() const;

   void set_marked_color(const colorfunc::Color& c,double alpha=1.0);
   void set_marked_color(const osg::Vec4& s_color);
   osg::Vec4& get_marked_color();
   const osg::Vec4& get_marked_color() const;

   void set_blinking_color(const colorfunc::Color& c,double alpha=1.0);
   void set_blinking_color(const osg::Vec4& b_color);
   osg::Vec4& get_blinking_color();
   const osg::Vec4& get_blinking_color() const;

   virtual void set_local_colors(const std::vector<osg::Vec4>& colors);
   std::vector<osg::Vec4>& get_local_colors();
   const std::vector<osg::Vec4>& get_local_colors() const;

   osg::Geode* get_geode_ptr();
   const osg::Geode* get_geode_ptr() const;

   void copy_size(double curr_t,int curr_passnumber,
                  const Geometrical* input_Geometrical_ptr);

   void set_right_neighbor_ID(int ID);
   void set_left_neighbor_ID(int ID);
   int get_right_neighbor_ID() const;
   int get_left_neighbor_ID() const;
   
   void set_track_ptr(track* t_ptr);
   track* get_track_ptr();
   const track* get_track_ptr() const;

// Color manipulation member functions:

   virtual void set_color(const colorfunc::Color& c);
   virtual void set_color(const osg::Vec4& color);
   virtual void set_colors(const std::vector<osg::Vec4>& colors);

   osg::Vec4& get_curr_blinking_color();
   bool time_to_switch_multicolors_to_blinking_color();
   void compute_blinking_color(const osg::Vec4& base_color);
   virtual void dirtyDisplay();

// Text member functions:

   void set_font_ptr(osgText::Font* f_ptr);
   osgText::Font* get_font_ptr();
   const osgText::Font* get_font_ptr() const;
   void set_n_text_messages(int n);
   unsigned int get_n_text_messages() const;
   osgText::Text* get_text_ptr(unsigned int m=0);
   const osgText::Text* get_text_ptr(unsigned int m=0) const;
   bool get_text_refptr_valid(int i);

   void set_text_label(int i,std::string label);
   std::string get_text_label(int i=0);
   void set_text_posn(int i,const threevector& posn);
   threevector get_text_posn(int i);
   void set_text_size(int i,double size);
   void change_text_size(int i,double factor);
   void change_text_size(osgText::Text* text_ptr,double factor);
   double get_text_size(int i) const;
   void set_text_color(int i,colorfunc::Color text_color);
   void set_text_color(int i,const osg::Vec4& color);
   void set_text_color(const osg::Vec4& color);
   void set_text_direction(int i,const threevector& text_dir);
   void set_text_rotation(int i,double chi);
   void initialize_text(int i);

// ActiveMQ broadcast member functions:

   void broadcast_KOZ_bbox(
      int ID,double t,const bounding_box& bbox,
      const threevector* grid_origin_ptr,Messenger* Messenger_ptr);
   void broadcast_progress(
      double curr_progress_frac,Messenger* Messenger_ptr);

  protected:

   bool on_earth_flag,blinking_flag,blinking_color_calculated;
   bool multicolor_flag;
   int n_text_messages;
   int right_neighbor_ID,left_neighbor_ID;
   double ellipsoid_magnification_factor;
   double text_character_size;
   double blinking_start_time,single_blink_duration,max_blink_period;
   threevector reference_origin;

   osg::ref_ptr<osgText::Font> font_refptr;
   osg::ref_ptr<osg::Vec3Array> vertices_refptr;
   osg::ref_ptr<osg::Vec4Array> color_array_refptr;
   osg::ref_ptr<osg::Geode> geode_refptr;
   std::vector<osg::ref_ptr<osgText::Text> > text_refptr;
   std::vector<osg::Vec4> local_colors;

   track* track_ptr;	// Just pointer, not actual object !

  private:

   osg::Vec3 x_dir;
   osg::Vec4 permanent_color,selected_color,blinking_color,marked_color;
   osg::Vec4 curr_color;
   osg::Vec4 permanent_text_color,selected_text_color,archived_text_color;

   osg::Quat text_Q;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Geometrical& c);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void Geometrical::set_reference_origin(const threevector& v_ref)
{
   reference_origin=v_ref;
}
   
inline threevector& Geometrical::get_reference_origin()
{
   return reference_origin;
}
   
inline const threevector& Geometrical::get_reference_origin() const
{
   return reference_origin;
}

inline void Geometrical::set_blinking_flag(bool flag)
{
   blinking_flag=flag;
}

inline bool Geometrical::get_blinking_flag() const
{
   return blinking_flag;
}

inline void Geometrical::set_blinking_start_time(double t)
{
   blinking_start_time=t;
}

inline void Geometrical::set_single_blink_duration(double t)
{
   single_blink_duration=t;
}

inline void Geometrical::set_max_blink_period(double t)
{
   max_blink_period=t;
}

inline void Geometrical::set_multicolor_flag(bool flag)
{
   multicolor_flag=flag;
}

inline bool Geometrical::get_multicolor_flag() const
{
   return multicolor_flag;
}

inline int Geometrical::get_n_vertices() const
{
//   std::cout << "inside Geometrical::get_n_vertices()" << std::endl;
//   std::cout << "vertices_refptr.valid() = "
//             << vertices_refptr.valid() << std::endl;

   if (vertices_refptr.valid())
   {
//      std::cout << " vertices_refptr->size() = " << vertices_refptr->size()
//                << std::endl;
      return vertices_refptr->size();
   }
   else
   {
      return -1;
   }
}

// ---------------------------------------------------------------------
inline void Geometrical::set_permanent_color(
   const colorfunc::Color& c,double alpha)
{
// FAKE FAKE:  Tues February 3 at 7:05 am

//   colorfunc::RGB orig_RGB=colorfunc::get_RGB_values(c);
//   colorfunc::HSV orig_hsv=colorfunc::RGB_to_hsv(orig_RGB);
//   orig_hsv.third *= 0.5;
//   colorfunc::RGB new_RGB=colorfunc::hsv_to_RGB(orig_hsv);
//   osg::Vec4 new_color(new_RGB.first,new_RGB.second,new_RGB.third,alpha);
//   set_permanent_color(new_color);

   set_permanent_color(colorfunc::get_OSG_color(c,alpha));
}

inline void Geometrical::set_permanent_color(const osg::Vec4& color)
{
//   std::cout << "inside Geometrical::set_permanent_color()" << std::endl;
//   std::cout << "color = " << std::endl;
//   osgfunc::print_Vec4(color);
   permanent_color=color;
}

inline osg::Vec4& Geometrical::get_permanent_color() 
{
   return permanent_color;
}

inline const osg::Vec4& Geometrical::get_permanent_color() const
{
   return permanent_color;
}

// ---------------------------------------------------------------------
inline void Geometrical::set_permanent_text_color(
   const colorfunc::Color& c,double alpha)
{
   set_permanent_text_color(colorfunc::get_OSG_color(c,alpha));
}

inline void Geometrical::set_permanent_text_color(const osg::Vec4& color)
{
//   std::cout << "inside Geometrical::set_permanent_text_color()" << std::endl;
//   std::cout << "color = " << std::endl;
//   osgfunc::print_Vec4(color);
   permanent_text_color=color;
}

inline osg::Vec4& Geometrical::get_permanent_text_color() 
{
   return permanent_text_color;
}

inline const osg::Vec4& Geometrical::get_permanent_text_color() const
{
   return permanent_text_color;
}

// ---------------------------------------------------------------------
inline osg::Vec4& Geometrical::get_curr_color() 
{
   return curr_color;
}

inline const osg::Vec4& Geometrical::get_curr_color() const
{
   return curr_color;
}

inline osg::Vec4Array* Geometrical::get_color_array_ptr() 
{
   return color_array_refptr.get();
}

inline void Geometrical::set_selected_color(
   const colorfunc::Color& c,double alpha)
{
   set_selected_color(colorfunc::get_OSG_color(c,alpha));
}

inline void Geometrical::set_selected_color(const osg::Vec4& color)
{
//   std::cout << "inside Geometrical::set_selected_color()" << std::endl;
//   std::cout << "color.r = " << color.r() << " color.g = " << color.g()
//             << " color.b = " << color.b() << std::endl;
   selected_color=color;
}

inline osg::Vec4& Geometrical::get_selected_color() 
{
   return selected_color;
}

inline const osg::Vec4& Geometrical::get_selected_color() const
{
   return selected_color;
}

inline void Geometrical::set_marked_color(
   const colorfunc::Color& c,double alpha)
{
   set_marked_color(colorfunc::get_OSG_color(c,alpha));
}

inline void Geometrical::set_marked_color(const osg::Vec4& color)
{
   marked_color=color;
}

inline osg::Vec4& Geometrical::get_marked_color() 
{
   return marked_color;
}

inline const osg::Vec4& Geometrical::get_marked_color() const
{
   return marked_color;
}

inline void Geometrical::set_blinking_color(
   const colorfunc::Color& c,double alpha)
{
   set_blinking_color(colorfunc::get_OSG_color(c,alpha));
}

inline void Geometrical::set_blinking_color(const osg::Vec4& color)
{
   blinking_color=color;
   blinking_color_calculated=true;
}

inline osg::Vec4& Geometrical::get_blinking_color()
{
   return blinking_color;
}

inline const osg::Vec4& Geometrical::get_blinking_color() const
{
   return blinking_color;
}

inline void Geometrical::set_local_colors(
   const std::vector<osg::Vec4>& colors)
{
//   std::cout << "inside Geometrical::set_local_colors()" << std::endl;
//   std::cout << "input colors.size() = " << colors.size() << std::endl;
   local_colors.clear();
   for (int n=0; n<int(colors.size()); n++)
   {
      local_colors.push_back(colors[n]);
   }
}

inline std::vector<osg::Vec4>& Geometrical::get_local_colors()
{
   return local_colors;
}

inline const std::vector<osg::Vec4>& Geometrical::get_local_colors() const
{
   return local_colors;
}


inline void Geometrical::set_font_ptr(osgText::Font* f_ptr)
{
   font_refptr=f_ptr;
}

inline osgText::Font* Geometrical::get_font_ptr()
{
   return font_refptr.get();
}

inline const osgText::Font* Geometrical::get_font_ptr() const
{
   return font_refptr.get();
}

inline void Geometrical::set_n_text_messages(int n)
{
   n_text_messages=n;
}

inline unsigned int Geometrical::get_n_text_messages() const
{
   return n_text_messages;
}

inline osgText::Text* Geometrical::get_text_ptr(unsigned int m)
{
   if (m < get_n_text_messages())
   {
      return text_refptr[m].get();
   }
   else
   {
      std::cout << "Error in Geometrical::get_text_ptr()!" << std::endl;
      std::cout << "m = " << m << " > n_text_messages = " 
                << get_n_text_messages() << std::endl;
      exit(-1);
   }
}

inline const osgText::Text* Geometrical::get_text_ptr(unsigned int m) const
{
   if (m < get_n_text_messages())
   {
      return text_refptr[m].get();
   }
   else
   {
      std::cout << "Error in Geometrical::get_text_ptr()!" << std::endl;
      std::cout << "m = " << m << " > n_text_messages = " 
                << get_n_text_messages() << std::endl;
      exit(-1);
   }
}

inline osg::Geode* Geometrical::get_geode_ptr()
{
   return geode_refptr.get();
}

inline const osg::Geode* Geometrical::get_geode_ptr() const
{
   return geode_refptr.get();
}

inline void Geometrical::set_right_neighbor_ID(int ID)
{
   right_neighbor_ID=ID;
}

inline void Geometrical::set_left_neighbor_ID(int ID)
{
   left_neighbor_ID=ID;
}

inline int Geometrical::get_right_neighbor_ID() const
{
   return right_neighbor_ID;
}

inline int Geometrical::get_left_neighbor_ID() const
{
   return left_neighbor_ID;
}

inline void Geometrical::set_track_ptr(track* t_ptr)
{
   track_ptr=t_ptr;
}

inline track* Geometrical::get_track_ptr()
{
   return track_ptr;
}

inline const track* Geometrical::get_track_ptr() const
{
   return track_ptr;
}

#endif // Geometrical.h



