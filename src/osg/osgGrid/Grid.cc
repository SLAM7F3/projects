// =========================================================================
// Grid base class 
// =========================================================================
// Last updated on 11/4/11; 11/24/11; 3/7/12
// =========================================================================

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/LineWidth>
#include "math/basic_math.h"
#include "color/colorfuncs.h"
#include "math/constant_vectors.h"
#include "osg/osgGrid/Grid.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Grid::allocate_member_objects()
{
   vertices_refptr = new osg::Vec3Array;
   color_array_refptr = new osg::Vec4Array(1);
   linewidth_refptr = new osg::LineWidth();
   stateset_refptr = new osg::StateSet;
   drawable_group_refptr=new osg::Group;
}		       

void Grid::initialize_member_objects()
{
//   cout << "inside Grid::initialize_member_objects()" << endl;
   Graphical_name="Grid";

   x_distance_scale=y_distance_scale=meter;
   x_axis_label="X Axis";
   y_axis_label="Y Axis";
   grid_on = true;
   zsize=-1;

   set_curr_color(colorfunc::get_OSG_color(colorfunc::grey));
//   set_curr_color(colorfunc::get_OSG_color(colorfunc::brightpurple));
   
   xtick_value0=ytick_value0=0;
   delta_x=delta_y=50;					// meters
   change_x_by = change_y_by = change_z_by = 50;	// meters

   rot_x_axis_label_flag=true;
   ticks_in_xy_plane_flag=false; // default screen alignment for tick #'s

   HiresDataVisitor_ptr=NULL;
   root_ptr=NULL;
}		       

Grid::Grid(const int ndims,int ID):
   Geometrical(ndims,ID)
{
//   cout << "Inside Grid constructor" << endl;
   allocate_member_objects();
   initialize_member_objects();
}

Grid::~Grid()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Grid& g)
{
   outstream << "inside Grid::operator<<" << endl;
//   outstream << static_cast<const Geometrical&>(c) << endl;

   outstream << "xmin = " << g.xmin 
             << " xmax = " << g.xmax 
             << " dx = " << g.delta_x << endl;
   outstream << "ymin = " << g.ymin 
             << " ymax = " << g.ymax 
             << " dy = " << g.delta_y << endl;
   return(outstream);
}

// ==========================================================================
// Drawing member functions
// ==========================================================================

// Member function generate_drawable_geode() instantiates an OSG geode

osg::Geode* Grid::generate_drawable_geode()
{
//   cout << "inside Grid::generate_drawable_geode()" << endl;

   geode_refptr = new osg::Geode;
   geom_refptr = new osg::Geometry;

   fill_drawable_geode();
   return geode_refptr.get();
}

void Grid::fill_drawable_geode()
{
   geode_refptr->addDrawable(geom_refptr.get());
   geode_refptr->setNodeMask(1);

// Ross Anderson pointed out on 6/13/06 that the Color Binding should
// be set equal to BIND_OVERALL since the grid's color is
// monochromatic.  We therefore instantiate a color Vec4array with
// just a single entry set equal to line_color:

   geom_refptr->setColorArray(color_array_refptr.get());
   geom_refptr->setColorBinding(osg::Geometry::BIND_OVERALL);

   geom_refptr->setStateSet(stateset_refptr.get());
   geom_refptr->setVertexArray(vertices_refptr.get());

   drawable_group_refptr->setNodeMask(1);

   linewidth_refptr->setWidth(3.0);
   stateset_refptr->setAttributeAndModes(
      linewidth_refptr.get(),osg::StateAttribute::ON);
}

void Grid::update_grid_text_color()
{
   cout << "inside Grid::update_grid_text_color() dummy method()"
        << endl;
}

void Grid::redraw_long_lat_lines(bool refresh_flag)
{
   cout << "inside Grid::redraw_long_lat_lines() dummy method()" << endl;
}

void Grid::destroy_dynamic_grid_lines()
{
   cout << "inside Grid::destroy_dynamic_grid_lines() dummy method()" << endl;
}

threevector Grid::get_north_hat()
{
   cout << "inside Grid::get_north_hat() dummy method()" << endl;
   return Zero_vector;
}

const threevector Grid::get_north_hat() const
{
   cout << "inside Grid::get_north_hat() dummy method()" << endl;
   return Zero_vector;
}

// =========================================================================
// Set & get member functions
// =========================================================================

void Grid::set_XYZ_extents(
   double min_x,double max_x,double min_y,double max_y,double min_z)
{
//   cout << "inside Grid::Set_XYZ_extents()" << endl;
   xmin=min_x;
   xmax=max_x;
   ymin=min_y;
   ymax=max_y;
   zplane=min_z;

//   cout << "zplane = " << zplane << endl;
}

void Grid::initialize_axes_char_sizes()
{
//   cout << "inside Grid::initialize_axes_char_sizes()" << endl;

   double xlog=trunclog(xmax-xmin);  
   double ylog=trunclog(ymax-ymin);
   double max_log=basic_math::max(2.0,log10(xlog),log10(ylog));

// Use the next definition for max_log when working with Lowell_NS
// data that ranges over multi-kilometer span:

//   double max_log=basic_math::max(2.0,log10(xmax-xmin),log10(ymax-ymin));
   
   double magnification=5*(max_log-2.0);
   axis_char_label_size = 25*(1+magnification);

// On 4/27/06, we empirically found that the tick_char_label_size
// needed to be significantly reduced for multi-kilometer strips.  So
// we clamp down the magnification factor in this case:

   if (max_log >= 3) magnification=2*(max_log-2.0);

   tick_char_label_size = 15*(1+magnification);

//   cout << "xmax = " << xmax << " xmin = " << xmin << endl;
//   cout << "ymax = " << ymax << " ymin = " << ymin << endl;
//   cout << "xlog = " << xlog << " ylog = " << ylog  << endl;
//   cout << "log10(xmax-xmin)=" << log10(xmax-xmin) 
//        << " log10(ymax-ymin) = " << log10(ymax-ymin) << endl;
//   cout << " max_log = " << max_log 
//        << " magnification = " << magnification << endl;
//   cout << "tick_char_label_size = " << tick_char_label_size << endl;
}

void Grid::set_axes_labels(string xlabel,string ylabel)
{
   x_axis_label = xlabel;
   y_axis_label = ylabel;
}

void Grid::set_xtick_value0(double x0)
{
   xtick_value0=x0;
}

void Grid::set_ytick_value0(double y0)
{
   ytick_value0=y0;
}

void Grid::set_delta_xy(double deltax,double deltay)
{
   delta_x=deltax;
   delta_y=deltay;
}

void Grid::set_delta_x(double deltax)
{
   delta_x = deltax;
}

void Grid::set_delta_y(double deltay)
{
   delta_y = deltay;
}
        
void Grid::set_z_plane(double zplane)
{
   this->zplane = zplane;
}

void Grid::set_thickness(float thickness)
{
   linewidth_refptr->setWidth(thickness);
}

// Member function get_corner() returns the X & Y coordinates of the
// world locations of the grid's corners for input integer 0 <= c <=
// 3.  If c==4, this method returns the X & Y coordinates of the
// grid's world middle location.  For 5 <= c <= 8, the X & Y coords of
// the midpoints along the grid's boundary are returned.

threevector Grid::get_corner(int c) const
{
   double min_grid_x=world_origin.get(0);
   double max_grid_x=world_maximum.get(0);
   double min_grid_y=world_origin.get(1);
   double max_grid_y=world_maximum.get(1);
   double grid_z=world_origin.get(2);

   if (c==0)
   {
      return threevector(min_grid_x,min_grid_y,grid_z);
   }
   else if (c==1)
   {
      return threevector(max_grid_x,min_grid_y,grid_z);
   }
   else if (c==2)
   {
      return threevector(max_grid_x,max_grid_y,grid_z);
   }
   else if (c==3)
   {
      return threevector(min_grid_x,max_grid_y,grid_z);
   }
   else if (c==4)
   {
      return world_middle;
   }
   else if (c==5)
   {
      return threevector(0.5*(min_grid_x+max_grid_x),min_grid_y,grid_z);
   }
   else if (c==6)
   {
      return threevector(max_grid_x,0.5*(min_grid_y+max_grid_y),grid_z);
   }
   else if (c==7)
   {
      return threevector(0.5*(min_grid_x+max_grid_x),max_grid_y,grid_z);
   }
   else if (c==8)
   {
      return threevector(min_grid_x,0.5*(min_grid_y+max_grid_y),grid_z);
   }
   else
   {
      cout << "Error in Grid::get_corner()!" << endl;
      cout << "c = " << c << " is invalid" << endl;
      return threevector(NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
   }
}

// =========================================================================
// User initiated grid manipulation methods
// =========================================================================

// If mask_value==0, Grid is hidden.  If mask_value==1, Grid becomes
// visible.

void Grid::set_mask(int mask_value)
{
//   cout << "inside Grid::set_mask, mask_value = " << mask_value << endl;
   geode_refptr->setNodeMask(mask_value);
   drawable_group_refptr->setNodeMask(mask_value);
}

void Grid::toggle_mask()
{
//   cout << "inside Grid::toggle_mask()" << endl;
   grid_on = ! grid_on;
   set_mask(grid_on);

//   cout << "grid_on = " << grid_on << endl;
}

// Methods that increase/decrease spacing between grid lines or grid's
// z value:

void Grid::increase_delta_x()
{
   if (!grid_on) return;

   if (delta_x == 1)
      set_delta_x(change_x_by);
   else
      set_delta_x(delta_x+change_x_by);

   set_axis_char_label_size(get_axis_char_label_size()*1.25);
   set_tick_char_label_size(get_tick_char_label_size()*1.25);
}

void Grid::decrease_delta_x()
{
   if (!grid_on) return;

   if (change_x_by >= delta_x)
   {
      set_change_x_by(0.5*delta_x);
   }
   set_delta_x(delta_x-change_x_by);

   set_axis_char_label_size(get_axis_char_label_size()/1.25);
   set_tick_char_label_size(get_tick_char_label_size()/1.25);
}

void Grid::increase_delta_y()
{
   if (!grid_on) return;

   if (delta_y == 1)
      set_delta_y(change_y_by);
   else
      set_delta_y(delta_y+change_y_by);
}

void Grid::decrease_delta_y()
{
   if (!grid_on) return;

   if (change_y_by >= delta_y)
   {
      set_change_y_by(0.5*delta_y);
   }
   set_delta_y(delta_y-change_y_by);
}

void Grid::increase_z_plane()
{
   if (!grid_on) return;
   set_z_plane(get_z_plane()+get_change_z_by());
}

void Grid::decrease_z_plane()
{
   if (!grid_on) return;
   set_z_plane(get_z_plane()-get_change_z_by());
}

// ---------------------------------------------------------------------
// Member function set_world_origin_and_middle laterally expands the
// XY grid beyond its nominal size set by xmax-xmin and ymax-ymin by
// input fraction frac_increase.  It stores within member threevectors
// world_origin and world_middle the XYZ points corresponding to the
// (0,0,0) and (xsize/2,ysize/2,0) points on the world grid.

void Grid::set_xy_size(double frac_increase)
{
   xsize = (1+frac_increase)*(xmax-xmin);
   xsize = ((int)(xsize/delta_x)*delta_x)+delta_x;

   ysize = (1+frac_increase)*(ymax-ymin);
   ysize = ((int)(ysize/delta_y)*delta_y)+delta_y;
}

// ---------------------------------------------------------------------
void Grid::set_world_origin_and_middle()
{
   x_origin = xmin - ((xsize - (xmax-xmin) )/2);
   y_origin = ymin - ((ysize - (ymax-ymin) )/2);
   world_origin = threevector(x_origin, y_origin, zplane);
//   cout << "At end of Grid::set_world_origin_and_middle()" << endl;
//   cout << "world_origin = " << world_origin << endl;

   set_world_middle();
}

// This overloaded version of set_world_origin_and_middle allows one
// to manually specify the world space location of the grid origin.

void Grid::set_world_origin_and_middle(double x,double y)
{
//   cout << "Inside Grid::set_world_origin_and_middle(double x,double y)" 
//        << endl;
//   cout << "world_origin = " << world_origin << endl;

   x_origin=x;
   y_origin=y;
   world_origin=threevector(x_origin,y_origin,zplane);

   set_world_middle();
}

// -------------------------------------------------------------------------
void Grid::set_world_middle()
{
//   cout << "inside Grid::set_world_middle()" << endl;
   
   xmiddle=x_origin+0.5*xsize;
   ymiddle=y_origin+0.5*ysize;
   world_middle = threevector(xmiddle, ymiddle, zplane);   
   world_maximum=world_origin+2*(world_middle-world_origin);

// Recall that as of Nov 2006, we store relative vertex information
// with respect to the average of all vertices in STL vector V within
// *curr_Grid_ptr to avoid floating point problems.  So we need to
// translate the grid by its middle position in order to globally
// position it:

   double curr_t=0;
   int passnumber=0;
   set_UVW_coords(curr_t,passnumber,world_middle);
}

// -------------------------------------------------------------------------
void Grid::reset_axes_lines()
{
//   cout << "inside Grid::reset_axes_lines()" << endl;
//   cout << "Delta_x = " << delta_x << endl;

// First delete existing set of axes lines from geometry and clear all
// grid line endpoint positions from *vertices_refptr:

   if (geom_refptr->getNumPrimitiveSets() > 0) 
      geom_refptr->removePrimitiveSet(0);
   vertices_refptr->clear();

   osg::Vec3 xlinesize(xsize,0,0);
   osg::Vec3 ylinesize(0,ysize,0);

   int number_y_crosslines=basic_math::round(xsize/delta_x)+1;
   int number_x_crosslines=basic_math::round(ysize/delta_y)+1;

// Set up X-Axis Cross lines:

   for (int i=0; i < number_x_crosslines; i++)
   {
      osg::Vec3 xdir(
         x_origin-world_middle.get(0),
         y_origin-world_middle.get(1)+i*delta_y,
         zplane-world_middle.get(2));
      vertices_refptr->push_back(xdir);
      vertices_refptr->push_back(xdir+xlinesize);
   }

// Set up Y-Axis Cross lines:

   for (int i=0; i < number_y_crosslines; i ++)
   {
      osg::Vec3 ydir(
         x_origin-world_middle.get(0)+i*delta_x,
         y_origin-world_middle.get(1),
         zplane-world_middle.get(2));
      vertices_refptr->push_back(ydir);
      vertices_refptr->push_back(ydir+ylinesize);
   }

// If zsize > 0, extend XY grid in Z direction.  Draw second
// grid outline above first one, and connect the corners of the two
// grids with vertical lines parallel to Z_hat:

   if (zsize > 0)
   {
//      double zsize=0.5*(xsize+ysize);
      osg::Vec3 zlinesize(0,0,zsize);

      osg::Vec3 origin1(
         x_origin-world_middle.get(0),
         y_origin-world_middle.get(1),
         zplane-world_middle.get(2));

      vertices_refptr->push_back(origin1);
      vertices_refptr->push_back(origin1+zlinesize);

      vertices_refptr->push_back(origin1+xlinesize);
      vertices_refptr->push_back(origin1+xlinesize+zlinesize);

      vertices_refptr->push_back(origin1+ylinesize);
      vertices_refptr->push_back(origin1+ylinesize+zlinesize);

      vertices_refptr->push_back(origin1+xlinesize+ylinesize);
      vertices_refptr->push_back(origin1+xlinesize+ylinesize+zlinesize);

      osg::Vec3 origin2(
         x_origin-world_middle.get(0),
         y_origin-world_middle.get(1),
         zplane-world_middle.get(2)+zsize);

      vertices_refptr->push_back(origin2);
      vertices_refptr->push_back(origin2+xlinesize);

      vertices_refptr->push_back(origin2);
      vertices_refptr->push_back(origin2+ylinesize);

      vertices_refptr->push_back(origin2+xlinesize);
      vertices_refptr->push_back(origin2+xlinesize+ylinesize);

      vertices_refptr->push_back(origin2+ylinesize);
      vertices_refptr->push_back(origin2+xlinesize+ylinesize);
   } // zsize > 0 conditional
   
   DrawArrays_refptr=new osg::DrawArrays(
      osg::PrimitiveSet::LINES,0,vertices_refptr->size());

// Insert new set of grid lines into geometry:
   
   geom_refptr->insertPrimitiveSet(0,DrawArrays_refptr.get());
}

// -------------------------------------------------------------------------
void Grid::reset_axes_labels(double factor)
{
//   cout << "inside Grid::reset_axes_labels()" << endl;

// Remove axes' text labels from geode and instantiate a new set of
// labels.  Garbage cleaning should (hopefully!) automatically delete
// unused x_axis_text and y_axis_text memory slots.  Then reattach new
// labels to geode:

   if (x_axis_text_refptr.valid()) geode_refptr->removeDrawable(
      x_axis_text_refptr.get());
   if (y_axis_text_refptr.valid()) geode_refptr->removeDrawable(
      y_axis_text_refptr.get());
   
   x_axis_text_refptr = new osgText::Text;
   y_axis_text_refptr = new osgText::Text;

   geode_refptr->addDrawable(x_axis_text_refptr.get());
   geode_refptr->addDrawable(y_axis_text_refptr.get());

   double x_extent=factor*axis_char_label_size*x_axis_label.size();
   double y_extent=factor*axis_char_label_size*y_axis_label.size();
   threevector half_extent(world_middle-world_origin);

   double x_start=x_origin+half_extent.get(0)-0.5*x_extent;
   osg::Vec3 posx(x_start-world_middle.get(0),
                  y_origin-world_middle.get(1)-1.2*delta_y, 
                  zplane-world_middle.get(2));
   if (rot_x_axis_label_flag)
   {
      x_start=x_origin+half_extent.get(0)+0.5*x_extent;
      posx=osg::Vec3(x_start-world_middle.get(0),
                     y_origin-world_middle.get(1)-0.8*delta_y, 
                     zplane-world_middle.get(2));
   }

   x_axis_text_refptr->setFont("fonts/times.ttf");
   x_axis_text_refptr->setCharacterSize(axis_char_label_size);
   //x_axis_text_refptr->setCharacterSizeMode(osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
   x_axis_text_refptr->setPosition(posx);
   x_axis_text_refptr->setAxisAlignment(osgText::Text::XY_PLANE);
//   x_axis_text_refptr->setAxisAlignment(osgText::Text::REVERSED_XY_PLANE);
   x_axis_text_refptr->setColor(color_array_refptr->at(0));
   
   x_axis_text_refptr->setText(x_axis_label);

   if (rot_x_axis_label_flag)
   {
      
// Added next 3 lines on 10/5/05 in order to rotate x axis label for
// HAFB "death pass" display purposes:

      osg::Quat q;
      q.makeRotate(PI,osg::Vec3f(0,0,1));
      x_axis_text_refptr->setRotation(q);
   }

   double y_start=y_origin+half_extent.get(1)-0.5*y_extent;
   osg::Vec3 posy(x_origin-world_middle.get(0)-1.0*delta_x, 
                  y_start-world_middle.get(1), 
                  zplane-world_middle.get(2));

   y_axis_text_refptr->setFont("fonts/times.ttf");
   y_axis_text_refptr->setCharacterSize(axis_char_label_size);
   //y_axis_text_refptr->setCharacterSizeMode(osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
   y_axis_text_refptr->setPosition(posy);
   y_axis_text_refptr->setAxisAlignment(osgText::Text::XY_PLANE);
   y_axis_text_refptr->setColor(color_array_refptr->at(0));
   y_axis_text_refptr->setText(y_axis_label);
   osg::Quat r1;
   r1.makeRotate(-osg::inDegrees(-90.0f),0.0f,0.0f,1.0f);
   y_axis_text_refptr->setRotation(r1);
}

// -------------------------------------------------------------------------
void Grid::reset_x_ticks()
{
//   cout << "inside Grid::reset_x_ticks()" << endl;
//   cout << "delta_x = " << delta_x << endl;

// Remove x axis tick labels from geode and instantiate a new set of
// labels.  Garbage cleaning should (hopefully!) automatically delete
// unused memory slots.  Then reattach new labels to geode:

   for (int i=0; i<int(x_axis_tick_texts.size()); i++)
   {
      if (x_axis_tick_texts[i].valid())
         geode_refptr->removeDrawable(x_axis_tick_texts[i].get());
   }
   x_axis_tick_texts.clear();
 
   double prefactor=1;
   if (x_distance_scale==kilometer) prefactor=0.001;

   int number_y_crosslines=basic_math::round(xsize/delta_x)+1;
   for (int i = 0; i < number_y_crosslines; i++)
   {
      double tick_value = xtick_value0+i*delta_x;
      string tick_string;
      if (basic_math::is_int(prefactor*tick_value))
      {
         tick_string=stringfunc::integer_to_string(prefactor*tick_value,0);
      }
      else
      {
         int ndigits_after_decimal=1;         
         tick_string=stringfunc::number_to_string(
            prefactor*(tick_value),ndigits_after_decimal);
      }

// For reasons which we do not understand as of 4/27/06, it appears to
// be necessary to introduce a fudge prefactor of 0.5 into the
// computation of x_extent in order to end up with tick labels that
// are centered reasonably well about grid lines:

//      x_extent=tick_char_label_size*tick_string.size();
      double x_extent=0.5*tick_char_label_size*tick_string.size();
      osg::Vec3 tick_pos(
         x_origin-world_middle.get(0)+tick_value-xtick_value0-0.5*x_extent,
         y_origin-world_middle.get(1)-0.5*delta_y, 
         zplane-world_middle.get(2));

      osg::ref_ptr<osgText::Text> x_tick_text_refptr = new osgText::Text;
      x_tick_text_refptr->setFont("fonts/times.ttf");
      x_tick_text_refptr->setCharacterSize(tick_char_label_size);
//      x_tick_text_refptr->setCharacterSizeMode(
//         osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
      x_tick_text_refptr->setPosition(tick_pos);
      x_tick_text_refptr->setColor(color_array_refptr->at(0));
      x_tick_text_refptr->setText(tick_string);

      if (ticks_in_xy_plane_flag)
      {
         x_tick_text_refptr->setAxisAlignment(osgText::Text::XY_PLANE);
//         x_tick_text_refptr->setAxisAlignment(osgText::Text::REVERSED_XY_PLANE);
      }
      else
      {
         x_tick_text_refptr->setAxisAlignment(osgText::Text::SCREEN);
      }

      x_axis_tick_texts.push_back(x_tick_text_refptr);
      geode_refptr->addDrawable(x_tick_text_refptr.get());
   }
}

// -------------------------------------------------------------------------
void Grid::reset_y_ticks()
{

// Remove y axis tick labels from geode and instantiate a new set of
// labels.  Garbage cleaning should (hopefully!) automatically delete
// unused memory slots.  Then reattach new labels to geode:

   for (int i=0; i<int(y_axis_tick_texts.size()); i++)
   {
      if (y_axis_tick_texts[i].valid())
         geode_refptr->removeDrawable(y_axis_tick_texts[i].get());
   }
   y_axis_tick_texts.clear();

   double prefactor=1;
   if (x_distance_scale==kilometer) prefactor=0.001;

   int number_x_crosslines=basic_math::round(ysize/delta_y)+1;
   for (int i = 0; i < number_x_crosslines; i++)
   {
      double tick_value = ytick_value0+i*delta_y;
      string tick_string;
      if (basic_math::is_int(prefactor*tick_value))
      {
         tick_string=stringfunc::integer_to_string(prefactor*tick_value,0);
      }
      else
      {
         int ndigits_after_decimal=1;         
         tick_string=stringfunc::number_to_string(
            prefactor*(tick_value),ndigits_after_decimal);
      }

      double y_extent=0.5*tick_char_label_size*tick_string.size();
         osg::Vec3 tick_pos(x_origin-world_middle.get(0)-0.5*delta_x,
         y_origin-world_middle.get(1)+tick_value-ytick_value0-0.5*y_extent,
         zplane-world_middle.get(2));

      osg::ref_ptr<osgText::Text> y_tick_text_refptr = new osgText::Text;
      y_tick_text_refptr->setFont("fonts/times.ttf");
      y_tick_text_refptr->setCharacterSize(tick_char_label_size);
//      y_tick_text_refptr->setCharacterSizeMode(
//         osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
      y_tick_text_refptr->setPosition(tick_pos);
      y_tick_text_refptr->setColor(color_array_refptr->at(0));
      y_tick_text_refptr->setText(tick_string);
        
      if (ticks_in_xy_plane_flag)
      {
         y_tick_text_refptr->setAxisAlignment(osgText::Text::XY_PLANE);
      }
      else
      {
         y_tick_text_refptr->setAxisAlignment(osgText::Text::SCREEN);
      }

      y_axis_tick_texts.push_back(y_tick_text_refptr);
      geode_refptr->addDrawable(y_tick_text_refptr.get());
   }
}

// -------------------------------------------------------------------------
// Boolean member function point_lies_inside_grid_borders() returns
// true if the input point's X and Y values lie within the intervals
// [xmin,xmax] and [ymin,ymax]

bool Grid::point_lies_inside_grid_borders(const threevector& XYZ) const
{
   bool point_lies_inside_flag=true;
   if (XYZ.get(0) < xmin || XYZ.get(0) > xmax ||
       XYZ.get(1) < ymin || XYZ.get(1) > ymax)
   {
      point_lies_inside_flag=false;
   }
   return point_lies_inside_flag;
}

