// ==========================================================================
// Header file for GEOMETRICALSGROUP class
// ==========================================================================
// Last modified on 2/10/11; 5/17/11; 10/12/11
// ==========================================================================

#ifndef GEOMETRICALSGROUP_H
#define GEOMETRICALSGROUP_H

#include <string>
#include <osgText/Font>
#include <osg/Geode>
#include <osg/PositionAttitudeTransform>
#include "color/colorfuncs.h"
#include "osg/Custom2DManipulator.h"
#include "osg/Custom3DManipulator.h"
#include "osg/osgGeometry/Geometrical.h"
#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "track/tracks_group.h"

class AnimationController;
class Clock;
class Ellipsoid_model;
class genmatrix;

class GeometricalsGroup : public GraphicalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   GeometricalsGroup(const int p_ndims,Pass* PI_ptr,threevector* GO_ptr=NULL);
   GeometricalsGroup(const int p_ndims,Pass* PI_ptr,
                     AnimationController* AC_ptr,threevector* GO_ptr=NULL);
   GeometricalsGroup(const int p_ndims,Pass* PI_ptr,Clock* clock_ptr,
                     Ellipsoid_model* EM_ptr,threevector* GO_ptr=NULL);
   virtual ~GeometricalsGroup();

// Scenegraph node insertion & removal member functions

   void attach_bunched_geometries_to_OSGsubPAT(
      const threevector& reference_vertex,int subPAT_number=0);
   bool remove_bunched_geometries_from_OSGsubPAT();

// Set & get methods:

   void set_Geometricals_updated_flag(bool flag);
   bool get_Geometricals_updated_flag() const;
   void set_ladar_height_data_flag(bool flag);
   bool get_ladar_height_data_flag() const;

   void set_permanent_colorfunc_color(colorfunc::Color perm_color);
   colorfunc::Color get_permanent_colorfunc_color() const;
   void set_selected_colorfunc_color(colorfunc::Color selected_color);
   colorfunc::Color get_selected_colorfunc_color() const;

   void set_multicolor_flags(bool flag);
   bool get_multicolor_flags() const;
   void set_n_text_messages(int n);
   int get_n_text_messages() const;
   osg::Geode* get_geode_ptr();
   const osg::Geode* get_geode_ptr() const;
   osg::PositionAttitudeTransform* get_PAT_ptr();
   const osg::PositionAttitudeTransform* get_PAT_ptr() const;
   Geometrical* get_Geometrical_ptr(int n) const;
   Geometrical* get_ID_labeled_Geometrical_ptr(int ID) const;
   void set_common_geometrical_size(double size);
   double get_common_geometrical_size() const;
   void set_package_subdir(std::string subdir);
   void set_package_filename_prefix(std::string prefix);
   void set_tracks_group_ptr(tracks_group* tg_ptr);
   
// CustomManipulator set & get methods:

   osgGA::CustomManipulator* get_CM_ptr();
   const osgGA::CustomManipulator* get_CM_ptr() const;
   osg::ref_ptr<osgGA::CustomManipulator>& get_CM_refptr();
   const osg::ref_ptr<osgGA::CustomManipulator>& get_CM_refptr() const;

   void set_CM_2D_ptr(osgGA::Custom2DManipulator* CM_ptr);   
   osgGA::Custom2DManipulator* get_CM_2D_ptr();
   const osgGA::Custom2DManipulator* get_CM_2D_ptr() const;

   void set_CM_3D_ptr(osgGA::Custom3DManipulator* CM_ptr);   
   osgGA::Custom3DManipulator* get_CM_3D_ptr();
   const osgGA::Custom3DManipulator* get_CM_3D_ptr() const;

// Geometrical appearance alteration methods:

   void change_size(double factor);
   void change_size(double geometrical_factor,double text_scale_factor);
   void change_size(
      double geom_X_factor,double geom_Y_factor,double geom_Z_factor,
      double text_scale_factor);
   void set_size(double size);
   void set_size(double geometrical_size,double text_scale_size);
   void set_size(double geom_X_size,double geom_Y_size,double geom_Z_size,
                 double text_size,int n_repeats=1);
   double compute_altitude_dependent_size(
      double zmin,double zmax,double size_min=0.5,double size_max=100);
   double compute_altitude_dependent_size(
      double zmin,double zintermediate,double zmax,
      double size_min,double size_intermediate,double size_max);

// Geometrical coloring member functions:

   virtual void reset_colors();
   void reset_text_color(int i,const osg::Vec4& color);
   bool blink_Geometrical(int Geometrical_ID,double max_blink_period=10);
   bool blink_Geometricals(const std::vector<int>& Geometrical_IDs,
      double max_blink_period=10);
   void set_colors(colorfunc::Color permanent_color,
   		   colorfunc::Color selected_color);
   void update_colors();

// Geometrical tracking member functions:

   void follow_selected_Geometrical(
      double min_height_above_Geometrical=200);


  protected:

   bool ladar_height_data_flag;
   int n_text_messages;
   double common_geometrical_size;
   std::string package_subdir,package_filename_prefix;
   colorfunc::Color permanent_colorfunc_color,selected_colorfunc_color;
   osg::ref_ptr<osg::Geode> geode_refptr;
   osg::ref_ptr<osg::PositionAttitudeTransform> PAT_refptr;
   osg::ref_ptr<osgText::Font> font_refptr;
   tracks_group* tracks_group_ptr;

   bool import_package_params(
      int framenumber,double& frustum_sidelength,genmatrix& P);
   double compute_altitude_dependent_alpha(
      double zmin,double zmax,double alpha_max);

  private:

   osg::ref_ptr<osgGA::CustomManipulator> CM_refptr;

   bool Geometricals_updated_flag;
   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const GeometricalsGroup& A);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void GeometricalsGroup::set_Geometricals_updated_flag(bool flag)
{
   Geometricals_updated_flag=flag;
}

inline bool GeometricalsGroup::get_Geometricals_updated_flag() const
{
   return Geometricals_updated_flag;
}

inline void GeometricalsGroup::set_ladar_height_data_flag(bool flag)
{
   ladar_height_data_flag=flag;
}

inline bool GeometricalsGroup::get_ladar_height_data_flag() const
{
   return ladar_height_data_flag;
}

inline void GeometricalsGroup::set_permanent_colorfunc_color(
   colorfunc::Color perm_color)
{
   permanent_colorfunc_color=perm_color;
}

inline colorfunc::Color GeometricalsGroup::get_permanent_colorfunc_color() 
   const
{
   return permanent_colorfunc_color;
}

inline void GeometricalsGroup::set_selected_colorfunc_color(
   colorfunc::Color selected_color)
{
   selected_colorfunc_color=selected_color;
}

inline colorfunc::Color GeometricalsGroup::get_selected_colorfunc_color() 
   const
{
   return selected_colorfunc_color;
}

inline void GeometricalsGroup::set_multicolor_flags(bool flag)
{
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      get_Geometrical_ptr(n)->set_multicolor_flag(flag);
   }
}

inline bool GeometricalsGroup::get_multicolor_flags() const
{
//   std::cout << "inside GeometricalsGroup::get_multicolor_flags()" 
//             << std::endl;
   bool multicolor_flags=false;
   if (get_n_Graphicals() > 0) multicolor_flags=true;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      if (!get_Geometrical_ptr(n)->get_multicolor_flag())
      {
         multicolor_flags=false;
      }
   }
   return multicolor_flags;
}

// --------------------------------------------------------------------------
inline void GeometricalsGroup::set_n_text_messages(int n)
{
   n_text_messages=n;
}

inline int GeometricalsGroup::get_n_text_messages() const
{
   return n_text_messages;
}

// --------------------------------------------------------------------------
inline osg::Geode* GeometricalsGroup::get_geode_ptr()
{
   return geode_refptr.get();
}

inline const osg::Geode* GeometricalsGroup::get_geode_ptr() const
{
   return geode_refptr.get();
}

// --------------------------------------------------------------------------
inline osg::PositionAttitudeTransform* GeometricalsGroup::get_PAT_ptr()
{
   return PAT_refptr.get();
}

inline const osg::PositionAttitudeTransform* GeometricalsGroup::get_PAT_ptr() 
   const
{
   return PAT_refptr.get();
}

// --------------------------------------------------------------------------
inline Geometrical* GeometricalsGroup::get_Geometrical_ptr(int n) const
{
   return dynamic_cast<Geometrical*>(get_Graphical_ptr(n));
}

inline Geometrical* GeometricalsGroup::get_ID_labeled_Geometrical_ptr(int ID) 
   const
{
   return dynamic_cast<Geometrical*>(get_ID_labeled_Graphical_ptr(ID));
}

// --------------------------------------------------------------------------
inline void GeometricalsGroup::set_common_geometrical_size(double size)
{
   common_geometrical_size=size;
}

inline double GeometricalsGroup::get_common_geometrical_size() const
{
   return common_geometrical_size;
}

// --------------------------------------------------------------------------
inline void GeometricalsGroup::set_package_subdir(std::string subdir)
{
   package_subdir=subdir;
}

inline void GeometricalsGroup::set_package_filename_prefix(std::string prefix)
{
   package_filename_prefix=prefix;
}

// --------------------------------------------------------------------------
inline void GeometricalsGroup::set_tracks_group_ptr(tracks_group* tg_ptr)
{
   tracks_group_ptr=tg_ptr;
}

#endif // GeometricalsGroup.h



