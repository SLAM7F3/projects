// =========================================================================
// ALIRTGRID class
// =========================================================================
// Last updated on 6/9/10; 12/4/10; 3/26/13
// =========================================================================

#include <iostream>
#include <osg/BoundingBox>
#include "osg/osgGrid/AlirtGrid.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::ostringstream;
using std::string;

// -------------------------------------------------------------------------
void AlirtGrid::allocate_member_objects()
{
}		       

void AlirtGrid::initialize_member_objects()
{
   Graphical_name="AlirtGrid";
}		       

AlirtGrid::AlirtGrid(bool wopillc_flag,int ndims,int ID): 
   Grid(ndims,ID)
{
//   cout << "inside AlirtGrid constructor #1" << endl;
   allocate_member_objects();
   initialize_member_objects();
   world_origin_precisely_in_lower_left_corner=wopillc_flag;
}

AlirtGrid::AlirtGrid(colorfunc::Color c,bool wopillc_flag,int ndims,int ID): 
   Grid(ndims,ID)
{
   allocate_member_objects();
   initialize_member_objects();
   set_curr_color(colorfunc::get_OSG_color(c));
   world_origin_precisely_in_lower_left_corner=wopillc_flag;
}

AlirtGrid::~AlirtGrid()
{
}

// -------------------------------------------------------------------------
void AlirtGrid::update_grid()
{
//   cout << "inside AlirtGrid::update_grid()" << endl;
//   cout << "delta_x = " << delta_x << endl;

   if (delta_x < 1) delta_x = 1;
   if (delta_y < 1) delta_y = 1;

   const double frac_increase=0.1;
   set_xy_size(frac_increase);
   
   if (world_origin_precisely_in_lower_left_corner)
   {
      set_world_origin_and_middle(0,0);
   }
   else
   {
      set_world_origin_and_middle();
   }

   reset_axes_lines();

// Axes' and tick labels:

   const double factor=700.0/1650.0;

   reset_axes_labels(factor);
   reset_x_ticks();
   reset_y_ticks();
}

// -------------------------------------------------------------------------
// Member function initialize_ALIRT_grid sets parameters which are
// appropriate for generic ALIRT imagery:

void AlirtGrid::initialize_ALIRT_grid(
   const osg::BoundingBox& bbox,Grid::Distance_Scale distance_scale,
   double delta_s,double magnification_factor)
{
   initialize_ALIRT_grid(
      bbox.xMin(),bbox.xMax(),bbox.yMin(),bbox.yMax(),bbox.zMin(),
      distance_scale,delta_s,magnification_factor);
}

void AlirtGrid::initialize_ALIRT_grid(
   double min_X,double max_X,double min_Y,double max_Y,double min_Z,
   Grid::Distance_Scale distance_scale,double delta_s,
   double magnification_factor)
{
   initialize_ALIRT_grid(min_X,max_X,min_Y,max_Y,min_Z,min_Z,
   distance_scale,delta_s,magnification_factor);
}

void AlirtGrid::initialize_ALIRT_grid(
   double min_X,double max_X,double min_Y,double max_Y,
   double min_Z,double max_Z,Grid::Distance_Scale distance_scale,
   double delta_s,double magnification_factor)
{
//   cout << "inside AlirtGrid::init_ALIRT_grid()" << endl;
//   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
//   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
//   cout << "min_Z = " << min_Z << " max_Z = " << max_Z << endl;
   if (distance_scale==0)
   {
//      cout << "distance_scale = centimeter" << endl;
   }
   else if (distance_scale==1)
   {
//      cout << "distance_scale = meter" << endl;
   }
   else if (distance_scale==2)
   {
//      cout << "distance_scale = kilometer" << endl;
   }

   double avg_X=0.5*(min_X+max_X);
   double avg_Y=0.5*(min_Y+max_Y);
   double X_size=0.5*(max_X-min_X);
   double Y_size=0.5*(max_Y-min_Y);
   min_X=avg_X-magnification_factor*X_size;
   max_X=avg_X+magnification_factor*X_size;
   min_Y=avg_Y-magnification_factor*Y_size;
   max_Y=avg_Y+magnification_factor*Y_size;

   double grid_Z=0.5*(min_Z+max_Z);
   set_XYZ_extents(min_X,max_X,min_Y,max_Y,grid_Z);
   initialize_axes_char_sizes();
   set_axes_labels("Meters East","Meters North");
   delta_x=delta_y=50;	 // meters

   double xtick_char_label_size=tick_char_label_size;
   double x_axis_char_label_size=axis_char_label_size;

   if (distance_scale==Grid::centimeter)
   {
      x_axis_label="Relative X (cms)";
      y_axis_label="Relative Y (cms)";
   }
   else if (distance_scale==Grid::meter)
   {
      x_axis_label="Relative X (m)";
      y_axis_label="Relative Y (m)";
   }
   else if (distance_scale==Grid::kilometer)
   {
      x_axis_label="Kilometers East";
      y_axis_label="Kilometers North";
   }

//   cout << "xmax-xmin = " << xmax-xmin << endl;
//   cout << "ymax-ymin = " << ymax-ymin << endl;
   
   if (xmax-xmin < 15)
   {
//      x_distance_scale=centimeter;
//      x_axis_label="Relative X (cms)";
      delta_x=10;
      x_axis_char_label_size /= 30;
      xtick_char_label_size /= 30;
   }
   else if (xmax-xmin >= 15 && xmax-xmin < 100)
   {
//      x_axis_label="Relative X (m)";
      delta_x=5;
      x_axis_char_label_size /= 10;
      xtick_char_label_size /= 10;
   }
   else if (xmax-xmin > 1000)
   {
//      x_distance_scale=kilometer;
//      x_axis_label="Kilometers East";
      delta_x=100;	// meters
      xtick_char_label_size *= 1.5;
      if (xmax-xmin > 4000) 
      {
         delta_x = 500;	// meters
         xtick_char_label_size *= 1.5;
      }
      if (xmax-xmin > 10000) 
      {
         delta_x = 1000;	// meters
         xtick_char_label_size *= 2.5;
         x_axis_char_label_size *= 2.5;
      }
      if (xmax-xmin > 25000) 
      {
         delta_x = 2500;	// meters
         xtick_char_label_size *= 3;
         x_axis_char_label_size *= 2.5;
         x_distance_scale=kilometer;
         x_axis_label="Kilometers East";
      }
      if (xmax-xmin > 100000) 
      {
         delta_x = 10000;	// meters = 10 kms
         xtick_char_label_size *= 5;
         x_axis_char_label_size *= 5;
         x_distance_scale=kilometer;
         x_axis_label="Kilometers East";
      }
      if (xmax-xmin > 1000000) 
      {
         delta_x = 1000000;	// meters
         xtick_char_label_size *= 250.0;
         x_axis_char_label_size *= 250.0;
//         x_axis_label="Relative X (Kilometers)";
      }
   }

   double ytick_char_label_size=tick_char_label_size;
   double y_axis_char_label_size=axis_char_label_size;
   if (ymax-ymin < 15)
   {
//      y_distance_scale=centimeter;
//      y_axis_label="Relative Y (cms)";
      delta_y=10;
      y_axis_char_label_size /= 30;
      ytick_char_label_size /= 30;
   }
   else if (ymax-ymin >= 15 && ymax-ymin < 100)
   {
//      y_axis_label="Relative Y (m)";
      delta_y=5;
      y_axis_char_label_size /= 10;
      ytick_char_label_size /= 10;
   }
   else if (ymax-ymin > 1000)
   {
//      y_distance_scale=kilometer;
//      y_axis_label="Kilometers North";
      delta_y=100;	// meters
      ytick_char_label_size *= 1.5;
      if (ymax-ymin > 4000) 
      {
         delta_y = 500;    	// meters
         ytick_char_label_size *= 1.5;
      }
      if (ymax-ymin > 10000) 
      {
         delta_y = 1000;	// meters
         ytick_char_label_size *= 2.5;
         y_axis_char_label_size *= 2.5;
      }
      if (ymax-ymin > 25000) 
      {
         delta_y = 2500;	// meters
         ytick_char_label_size *= 3;
         y_axis_char_label_size *= 2.5;
         y_distance_scale=kilometer;
         y_axis_label="Kilometers North";
      }
      if (ymax-ymin > 1000000) 
      {
         delta_y = 1000000;	// meters
         ytick_char_label_size *= 250.0;
         y_axis_char_label_size *= 250.0;
//         y_axis_label="Relative Y (Kilometers)";
      }
   }

// Force grid cells to be square by setting dx=dy=max(dx,dy):

   delta_x=basic_math::max(delta_x,delta_y);
   delta_y=basic_math::max(delta_x,delta_y);

   if (delta_s > 0)
   {
      delta_x=delta_y=delta_s;
   }
//   cout << "delta_x = " << delta_x
//        << " delta_y = " << delta_y << endl;

   if (x_distance_scale==kilometer)
   {
      y_distance_scale=kilometer;
      y_axis_label="Kilometers North";
   }



/*
   if (x_distance_scale==centimeter)
   {
      y_distance_scale=centimeter;
      y_axis_label="Centimeters North";
   }
   if (y_distance_scale==kilometer) 
   {
      x_distance_scale=kilometer;
      x_axis_label="Kilometers East";
   }
   if (y_distance_scale==centimeter) 
   {
      x_distance_scale=centimeter;
      x_axis_label="Centimeters East";
   }
*/
 
//   cout << "xtick_char_label_size = " << xtick_char_label_size
//        << endl;
//   cout << "ytick_char_label_size = " << ytick_char_label_size
//        << endl;
   
//   cout << "x_axis_char_label_size = " << x_axis_char_label_size
//        << endl;
//   cout << "y_axis_char_label_size = " << y_axis_char_label_size
//        << endl;East

   tick_char_label_size=basic_math::max(
      xtick_char_label_size,ytick_char_label_size);
   axis_char_label_size=basic_math::max(
      x_axis_char_label_size,y_axis_char_label_size);

//   cout << "tick_char_label_size = " << tick_char_label_size << endl;
//   cout << "axis_char_label_size = " << axis_char_label_size << endl;
//   outputfunc::enter_continue_char();

   update_grid();
}

// -------------------------------------------------------------------------
// Member function initialize_satellite_grid sets parameters which are
// appropriate for a SPASE holodeck grid:

void AlirtGrid::initialize_satellite_grid(bool SPASE_flag)
{
   double min_X=0;
   double max_X=1;
   double min_Y=0;
   double max_Y=1;
   double min_Z=0;

   if (!SPASE_flag)
   {

// Note added in December 2007: Following grid dimensions are
// appropriate for SJ7:

      max_X=3;
      min_X=-max_X;
      max_Y=3;
      min_Y=-max_Y;
      min_Z=0;
   }

   set_XYZ_extents(min_X,max_X,min_Y,max_Y,min_Z);
   initialize_axes_char_sizes();
   set_axes_labels("Relative X (meters)","Relative Y (meters)");
   set_delta_xy(1,1);
   set_axis_char_label_size(0.2);
   set_tick_char_label_size(0.2);
   set_change_x_by(2);
   set_change_y_by(2);
   set_change_z_by(2);

   rot_x_axis_label_flag=false;
   ticks_in_xy_plane_flag=true;
   update_grid();
}

