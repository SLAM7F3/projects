// =========================================================================
// Header file for pure virtual Grid class which displays a 2D grid
// underneath a 3D set of points
// =========================================================================
// Last updated on 6/9/10; 11/4/11; 11/24/11
// =========================================================================

#ifndef GRID_H
#define GRID_H

#include <iostream>
#include <string>
#include <vector>
#include <osg/LineWidth>
#include <osg/StateSet>
#include <osgText/Text>
#include "osg/osgGeometry/Geometrical.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "math/threevector.h"

// class osg::Geometry;

class Grid : public Geometrical
{

  public:

   enum Distance_Scale
   {
      centimeter,meter,kilometer
   };

// Initialization, constructor and destructor functions:

   Grid(const int ndims,int ID=-1);
   virtual ~Grid();
   friend std::ostream& operator<< (
      std::ostream& outstream,const Grid& g);

// Set & get member functions:

   double get_delta_x();
   double get_delta_y();
   double get_z_plane();
   double get_change_x_by();
   double get_change_y_by();
   double get_change_z_by();
   double get_axis_char_label_size();
   double get_tick_char_label_size();
   double get_xsize() const;
   double get_ysize() const;
   void set_zsize(double zsize);
   
   threevector* get_world_origin_ptr();
   const threevector* get_world_origin_ptr() const;
   threevector get_world_origin();
   threevector get_world_middle();
   threevector get_world_maximum();

   void set_XYZ_extents(double min_x,double max_x,double min_y,double max_y,
                        double min_z);
   double get_min_grid_x() const;
   double get_max_grid_x() const;
   double get_min_grid_y() const;
   double get_max_grid_y() const;
   threevector get_corner(int c) const;
   void set_axes_labels(std::string xlabel,std::string ylabel);
   void set_xtick_value0(double x0);
   void set_ytick_value0(double x0);
   void set_delta_xy(double deltax,double deltay);
   void set_delta_x(double deltax);
   void set_delta_y(double deltay);
   void set_z_plane(double zplane);

   void set_change_x_by(double change_x);
   void set_change_y_by(double change_y);
   void set_change_z_by(double change_z);
   void set_axis_char_label_size(double size);
   void set_tick_char_label_size(double size);
   void set_thickness(float thickness);
   void set_rot_x_axis_label_flag(bool flag);
   void set_ticks_in_xy_plane_flag(bool flag);

   osg::Group* get_drawable_group_ptr();
   void set_HiresDataVisitor_ptr(HiresDataVisitor* HRDV_ptr);
   HiresDataVisitor* get_HiresDataVisitor_ptr();

   void set_root_ptr(osg::Group* r_ptr);
   osg::Group* get_root_ptr();

// Drawing member functions:

   osg::Geode* generate_drawable_geode();
   void fill_drawable_geode();
   virtual void update_grid()=0;
   virtual void update_grid_text_color();

   virtual void redraw_long_lat_lines(bool refresh_flag=false);
   virtual void destroy_dynamic_grid_lines();
   threevector get_north_hat();
   const threevector get_north_hat() const;

// User initiated grid manipulation member functions:

   void set_mask(int mask_value);
   void toggle_mask();
   void increase_delta_x();
   void decrease_delta_x();
   void increase_delta_y();
   void decrease_delta_y();
   void increase_z_plane();
   void decrease_z_plane();

   void set_world_origin_and_middle(double x,double y);

   bool point_lies_inside_grid_borders(const threevector& XYZ) const;

  protected:


   Distance_Scale x_distance_scale;
   Distance_Scale y_distance_scale;

// Extremal datagraph X and Y values:

   double xmin,xmax,ymin,ymax;
   double x_origin,y_origin,zplane;
   double xsize,ysize,zsize;

// Distance between x grid lines and y grid lines:

   double delta_x,delta_y;

   double axis_char_label_size;
   double tick_char_label_size;
   double xtick_value0,ytick_value0;
   std::string x_axis_label;
   std::string y_axis_label;

// Screen vs XY plane orientation for tick #'s:

   bool rot_x_axis_label_flag;
   bool ticks_in_xy_plane_flag;

   double xmiddle,ymiddle;
   threevector world_middle,world_maximum;
   threevector world_origin; // grid's origin location (10% larger
			     // than point cloud)

   osg::ref_ptr<osg::Geometry> geom_refptr;
   osg::ref_ptr<osgText::Text> x_axis_text_refptr;
   osg::ref_ptr<osgText::Text> y_axis_text_refptr;
   std::vector<osg::ref_ptr<osgText::Text> > x_axis_tick_texts;
   std::vector<osg::ref_ptr<osgText::Text> > y_axis_tick_texts;

   void initialize_axes_char_sizes();
   void set_xy_size(double frac_increase);
   void set_world_origin_and_middle();
   void set_world_middle();
   void reset_axes_lines();
   void reset_axes_labels(double factor);
   void reset_x_ticks();
   void reset_y_ticks();

  private:

   bool grid_on;

   // amount by which grid lines change with the key events

   double change_x_by;
   double change_y_by;
   double change_z_by;

   // number of vertices = 2*number of lines
   int n_vertices;

   HiresDataVisitor* HiresDataVisitor_ptr;
   osg::Group* root_ptr;

   osg::ref_ptr<osg::LineWidth> linewidth_refptr;
   osg::ref_ptr<osg::StateSet> stateset_refptr;
   osg::ref_ptr<osg::DrawArrays> DrawArrays_refptr;
   osg::ref_ptr<osg::Group> drawable_group_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline double Grid::get_delta_x()
{
   return delta_x;
}

inline double Grid::get_delta_y()
{
   return delta_y;
}

inline double Grid::get_xsize() const
{
   return xsize;
}

inline double Grid::get_ysize() const
{
   return ysize;
}

inline void Grid::set_zsize(double zsize)
{
   this->zsize=zsize;
}

inline double Grid::get_z_plane()
{
   return zplane;
}

inline double Grid::get_change_x_by()
{
   return change_x_by;
}

inline double Grid::get_change_y_by()
{
   return change_y_by;
}

inline double Grid::get_change_z_by()
{
   return change_z_by;
}

inline double Grid::get_axis_char_label_size()
{
   return axis_char_label_size;
}

inline double Grid::get_tick_char_label_size()
{
   return tick_char_label_size;
}

inline threevector* Grid::get_world_origin_ptr()
{
   return &world_origin;
}

inline const threevector* Grid::get_world_origin_ptr() const
{
   return &world_origin;
}

inline threevector Grid::get_world_origin()
{
   return world_origin;
}

inline threevector Grid::get_world_middle()
{
   return world_middle;
}

inline threevector Grid::get_world_maximum()
{
   return world_origin+2*(world_middle-world_origin);
}

inline void Grid::set_change_x_by(double change_x)
{
   change_x_by = change_x;
}

inline void Grid::set_change_y_by(double change_y)
{
   change_y_by = change_y;
}

inline void Grid::set_change_z_by(double change_z)
{
   change_z_by = change_z;
}

inline void Grid::set_axis_char_label_size(double size)
{
   axis_char_label_size = size;
}

inline void Grid::set_tick_char_label_size(double size)
{
   tick_char_label_size = size;
}

inline void Grid::set_rot_x_axis_label_flag(bool flag)
{
   rot_x_axis_label_flag=flag;
}

inline void Grid::set_ticks_in_xy_plane_flag(bool flag)
{
   ticks_in_xy_plane_flag=flag;
}

inline double Grid::get_min_grid_x() const
{
   return world_origin.get(0);
}

inline double Grid::get_max_grid_x() const
{
   return world_maximum.get(0);
}

inline double Grid::get_min_grid_y() const
{
   return world_origin.get(1);
}

inline double Grid::get_max_grid_y() const
{
   return world_maximum.get(1);
}

inline osg::Group* Grid::get_drawable_group_ptr()
{
   return drawable_group_refptr.get();
}

inline void Grid::set_HiresDataVisitor_ptr(HiresDataVisitor* HRDV_ptr)
{
   HiresDataVisitor_ptr=HRDV_ptr;
}

inline HiresDataVisitor* Grid::get_HiresDataVisitor_ptr()
{
   return HiresDataVisitor_ptr;
}

inline void Grid::set_root_ptr(osg::Group* r_ptr)
{
   root_ptr=r_ptr;
}

inline osg::Group* Grid::get_root_ptr()
{
   return root_ptr;
}

#endif
