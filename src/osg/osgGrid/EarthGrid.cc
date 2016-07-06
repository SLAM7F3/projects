// =========================================================================
// EARTHGRID class
// =========================================================================
// Last updated on 11/25/06; 12/17/06; 9/17/07; 3/10/09
// =========================================================================

#include <iostream>
#include <osg/PositionAttitudeTransform>
#include "osg/osgGrid/EarthGrid.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::ostringstream;
using std::string;

// -------------------------------------------------------------------------
void EarthGrid::allocate_member_objects()
{
}

void EarthGrid::initialize_member_objects()
{
   Graphical_name="EarthGrid";
   const double maximum=6.37906e+06;	// meters
   double minimum=-maximum;
   const double grid_Z=0;
   set_XYZ_extents(minimum,maximum,minimum,maximum,grid_Z);
   initialize_axes_char_sizes();
   set_axes_labels("ECI X (kilometers)","ECI Y (kilometers)");
   rot_x_axis_label_flag=false;

   delta_x=delta_y=1000000;	// meters
   tick_char_label_size=232031;
   axis_char_label_size=406250;

   x_distance_scale=y_distance_scale=kilometer;
}

EarthGrid::EarthGrid(colorfunc::Color c): 
   Grid(3)
{
   allocate_member_objects();
   initialize_member_objects();

//    osg::Geode* geode_ptr=
      generate_drawable_geode();
//   get_PAT_ptr()->addChild(geode_ptr);
   
   set_curr_color(colorfunc::get_OSG_color(c));
   update_grid();
}

EarthGrid::~EarthGrid()
{
}

// -------------------------------------------------------------------------
void EarthGrid::update_grid()
{
   const double frac_increase=0.23;
   set_xy_size(frac_increase);
   set_world_origin_and_middle();

   reset_axes_lines();

// Axes' labels:

   const double factor=700.0/1650.0;
   reset_axes_labels(factor);

// Tick labels:

   int number_y_crosslines=basic_math::round(xsize/delta_x)+1;
   xtick_value0=-number_y_crosslines/2*delta_x;
   reset_x_ticks();

   int number_x_crosslines=basic_math::round(ysize/delta_y)+1;
   ytick_value0=-number_x_crosslines/2*delta_y;
   reset_y_ticks();
}

