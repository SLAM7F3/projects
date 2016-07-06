// =========================================================================
// LATLONGGRID class
// =========================================================================
// Last updated on 8/8/10; 12/4/10; 1/10/11
// =========================================================================

#include <vector>
#include <osg/Geode>
#include <osg/Geometry>
#include "astro_geo/Clock.h"
#include "color/colorfuncs.h"
#include "astro_geo/geofuncs.h"
#include "osg/osgGrid/LatLongGrid.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "passes/Pass.h"
#include "general/stringfuncs.h"
#include "osg/ViewFrustum.h"

#include "templates/mytemplates.h"
#include "general/outputfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostringstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void LatLongGrid::allocate_member_objects()
{
   Ellipsoid_model_ptr=new Ellipsoid_model();
   LongLinesGroup_ptr=new PolyLinesGroup(3,pass_ptr);
   LatLinesGroup_ptr=new PolyLinesGroup(3,pass_ptr);
   longitude_lines_map_ptr=new LINES_MAP;
   latitude_lines_map_ptr=new LINES_MAP;
}		       

void LatLongGrid::initialize_member_objects()
{
   Graphical_name="LatLongGrid";
   dynamic_grid_flag=true;
   flat_grid_flag=true;
   depth_buffering_off_flag=false;
//   depth_buffering_off_flag=true; // grid lines always appear on top of data
   latitude_ptr=NULL;
   longitude_ptr=NULL;
   ndigits_after_decimal=4;
   curr_update_counter=0;
   longitude_middle=latitude_middle=NEGATIVEINFINITY;
   delta_longitude=delta_latitude=20;	// degs
   init_linewidth=1.0;
   text_size_prefactor=5.0E-4;

   UTM_grid_angle_wrt_LL_grid=NEGATIVEINFINITY;

   UTM_zonenumber=-1;
}

LatLongGrid::LatLongGrid(
   Pass* pass_ptr,osgGA::Custom3DManipulator* CM_3D_ptr): 
   Grid(3)
{
   this->pass_ptr=pass_ptr;
   allocate_member_objects();
   initialize_member_objects();
   this->CM_3D_ptr=CM_3D_ptr;
}

LatLongGrid::LatLongGrid(
   Pass* pass_ptr,int ndims,int ID,osgGA::Custom3DManipulator* CM_3D_ptr): 
   Grid(ndims,ID)
{
   this->pass_ptr=pass_ptr;
   allocate_member_objects();
   initialize_member_objects();
   this->CM_3D_ptr=CM_3D_ptr;
}

LatLongGrid::~LatLongGrid()
{
   delete Ellipsoid_model_ptr;
   delete LongLinesGroup_ptr;
   delete LatLinesGroup_ptr;
   delete longitude_lines_map_ptr;
   delete latitude_lines_map_ptr;

   delete latitude_ptr;
   delete longitude_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const LatLongGrid& g)
{
   outstream << "inside LatLongGrid::operator<<" << endl;
   outstream << static_cast<const Grid&>(g) << endl;
   return(outstream);
}

// =========================================================================
// Grid initialization member functions
// =========================================================================

// Member function initialize takes in UTM zone information as well as
// extremal UTM extents.  This method computes the corresponding
// extremal longitudes and latitudes and initializes various grid
// parameters.

void LatLongGrid::initialize(
   int ZoneNumber,bool northern_flag,
   double min_east,double max_east,
   double min_north,double max_north,double min_Z)
{
//   cout << "inside LatLongGrid::initialize() #1" << endl;
   
   UTM_zonenumber=ZoneNumber;
   northern_hemisphere_flag=northern_flag;

   double frac_increase=0.05;
//   cout << "Enter fractional increase in LatLongGrid:" << endl;
//   cin >> frac_increase;
   
   double east_avg=0.5*(min_east+max_east);
   double east_extent=(1+frac_increase)*(max_east-min_east);
   min_east=east_avg-0.5*east_extent;
   max_east=east_avg+0.5*east_extent;

   double north_avg=0.5*(min_north+max_north);
   double north_extent=(1+frac_increase)*(max_north-min_north);
   min_north=north_avg-0.5*north_extent;
   max_north=north_avg+0.5*north_extent;

   double min_lat,max_lat,min_long,max_long;
   latlongfunc::UTMtoLL(
      ZoneNumber,northern_hemisphere_flag,
      min_north,min_east,min_lat,min_long);
   latlongfunc::UTMtoLL(
      ZoneNumber,northern_hemisphere_flag,
      max_north,max_east,max_lat,max_long);

//   cout << "min_east = " << min_east << " max_east = " << max_east << endl;
//   cout << "min_long = " << min_long << " max_long = " << max_long << endl;
//   cout << "min_north = " << min_north << " max_north = " << max_north 
//        << endl;
//   cout << "min_lat = " << min_lat << " max_lat = " << max_lat << endl;
//   cout << "max_long-min_long = " << max_long-min_long << endl;
//   cout << "max_lat-min_lat = " << max_lat-min_lat << endl;

   initialize(min_long,max_long,min_lat,max_lat,min_Z);
}

// -------------------------------------------------------------------------
void LatLongGrid::initialize(
   double min_long,double max_long,
   double min_lat,double max_lat,double min_Z)
{
//   cout << "inside LatLongGrid::initialize() #2" << endl;
//   cout << "min_long = " << min_long
//        << " max_long = " << max_long << endl;
//   cout << "min_lat = " << min_lat
//        << " max_lat = " << max_lat << endl;

// If UTM zone number has not been previously specified, set its value
// as well as northern_hemisphere_flag based upon maximal longitude
// and latitude:

   if (UTM_zonenumber==-1)
   {
      geopoint max_corner(max_long,max_lat);
      UTM_zonenumber=max_corner.get_UTM_zonenumber();
      northern_hemisphere_flag=max_corner.get_northern_hemisphere_flag();
   }

   compute_long_lat_param_ranges(min_long,max_long,min_lat,max_lat);
   zplane=min_Z;
   set_world_origin_and_middle();
   set_text_character_sizes();
   update_grid();

   get_drawable_group_ptr()->addChild(
      get_LongLinesGroup_ptr()->get_OSGgroup_ptr());
   get_drawable_group_ptr()->addChild(
      get_LatLinesGroup_ptr()->get_OSGgroup_ptr());
}

// -------------------------------------------------------------------------
void LatLongGrid::compute_long_lat_param_ranges(
   double min_long,double max_long,double min_lat,double max_lat)
{
//   cout << "inside LatLongGrid::compute_long_lat_param_ranges()" << endl;
//   cout << "min_long = " << min_long << " max_long = " << max_long << endl;
//   cout << "min_lat = " << min_lat << " max_lat = " << max_lat << endl;
//   cout << "max_long-min_long = " << max_long-min_long << endl;
//   cout << "max_lat-min_lat = " << max_lat-min_lat << endl;
   
   double delta_theta=0.0005;
//   if (max_long-min_long > 0.004 || max_lat - min_lat > 0.004)
//   {
//      delta_theta=0.0005;	// degs
//      ndigits_after_decimal=4;
//   }
/*
   if (max_long-min_long > 0.02 || max_lat - min_lat > 0.02)
   {
      delta_theta=0.002;	// degs
      ndigits_after_decimal=3;
   }
*/
   if (max_long-min_long > 0.001 || max_lat - min_lat > 0.001)
   {
//      delta_theta=0.001;	// degs  (for slide generation)
      delta_theta=0.0025;	// degs
      ndigits_after_decimal=4;
   }
   if (max_long-min_long > 0.1 || max_lat - min_lat > 0.1)
   {
      delta_theta=0.01;
      ndigits_after_decimal=2;
   }
   if (max_long-min_long > 0.5 || max_lat - min_lat > 0.5)
   {
      delta_theta=0.05;
      ndigits_after_decimal=2;
   }
   if (max_long-min_long > 2 || max_lat - min_lat > 2)
   {
      delta_theta=0.2;
      ndigits_after_decimal=1;
   }
   if (max_long-min_long > 5 || max_lat - min_lat > 5)
   {
      delta_theta=1.0;
      ndigits_after_decimal=0;
   }
   
//   cout << "delta_theta = " << delta_theta << endl;
//   cout << "ndigits_after_decimal = " << ndigits_after_decimal << endl;

   min_long=delta_theta*basic_math::mytruncate(min_long/delta_theta);
   longitude_ptr=new param_range(min_long,max_long,delta_theta);
   min_lat=delta_theta*basic_math::mytruncate(min_lat/delta_theta);
   latitude_ptr=new param_range(min_lat,max_lat,delta_theta);

   longitude_middle=longitude_ptr->get_avg_value();
   latitude_middle=latitude_ptr->get_avg_value();

//   cout << "*longitude_ptr = " << *longitude_ptr << endl;
//   cout << "*latitude_ptr = " << *latitude_ptr << endl;
//   outputfunc::enter_continue_char();
}

// -------------------------------------------------------------------------
void LatLongGrid::set_world_origin_and_middle()
{
//   cout << "inside LLG:set_world_origin_and_middle()" << endl;

   bool specified_northern_hemisphere_flag=true;
   double start_north,start_east,stop_north,stop_east;
   latlongfunc::LL_to_northing_easting(
      longitude_ptr->get_start(),latitude_ptr->get_start(),
      specified_northern_hemisphere_flag,UTM_zonenumber,
      start_east,start_north);
   latlongfunc::LL_to_northing_easting(
      longitude_ptr->get_stop(),latitude_ptr->get_stop(),
      specified_northern_hemisphere_flag,UTM_zonenumber,
      stop_east,stop_north);
   set_XYZ_extents(start_east,stop_east,start_north,stop_north,get_z_plane());
   initialize_axes_char_sizes();

   x_origin=start_east;
   y_origin=start_north;
   world_origin=threevector(start_east,start_north,get_z_plane());

   double avg_longitude=
      0.5*(longitude_ptr->get_start()+longitude_ptr->get_stop());
   double avg_latitude=
      0.5*(latitude_ptr->get_start()+latitude_ptr->get_stop());
//   cout << "Avg long = " << avg_longitude
//        << " avg lat = " << avg_latitude << endl;
//   outputfunc::enter_continue_char();

   double mid_east,mid_north;
   latlongfunc::LL_to_northing_easting(
      avg_longitude,avg_latitude,
      specified_northern_hemisphere_flag,UTM_zonenumber,mid_east,mid_north);

   xmiddle=mid_east;
   ymiddle=mid_north;
   world_middle=threevector(mid_east,mid_north,get_z_plane());
   world_maximum=world_origin+2*(world_middle-world_origin);

// Recall that as of Nov 2006, we store relative vertex information
// with respect to the average of all vertices in STL vector V within
// *curr_Grid_ptr to avoid floating point problems.  So we need to
// translate the grid by its middle position in order to globally
// position it:

   double curr_t=0;
   int passnumber=0;
   set_UVW_coords(curr_t,passnumber,world_middle);

//   cout << "world_origin = " << world_origin << endl;
//   cout << "world_middle = " << world_middle << endl;
}

// -------------------------------------------------------------------------
// Member function set_text_character_sizes

void LatLongGrid::set_text_character_sizes()
{
//   cout << "inside LatLongGrid::set_text_char_sizes()" << endl;

   double x_tick_char_label_size=tick_char_label_size;
   double x_axis_char_label_size=axis_char_label_size;

//   cout << "x_tick_char_label_size = "
//        << x_tick_char_label_size << endl;
//   cout << "x_axis_char_label_size = "
//        << x_axis_char_label_size << endl;

//   cout << "long stop - long start = "
//        << longitude_ptr->get_stop()-longitude_ptr->get_start()
//        << endl;
//   cout << "ndigits_after_decimal = " 
//        << ndigits_after_decimal << endl;

// On 2/13/09, we found that tick and axis char label sizes for very
// large EarthRegions (e.g. all of western Massachusetts, all of
// Afghanistan) were too small.  So we add the following conditionals
// to amplify their sizes for these large cases:

   if (longitude_ptr->get_stop()-longitude_ptr->get_start() > 10)
   {
      x_tick_char_label_size *= 400;
      x_axis_char_label_size *= 300;
   }
   else if (longitude_ptr->get_stop()-longitude_ptr->get_start() > 1.0)
   {
      x_tick_char_label_size *= 40;
      x_axis_char_label_size *= 30;
   }
   else if (longitude_ptr->get_stop()-longitude_ptr->get_start() > 0.1)
   {
      if (ndigits_after_decimal==2)
      {
         x_tick_char_label_size *= 4;
      }
      else
      {
         x_tick_char_label_size *= 2.5;
      }
      x_axis_char_label_size *= 3;
   }

// On 7/28/07, we found that tick and axis char label sizes for very
// small EarthRegions (e.g. San Diego Ricoh parking lot) were too
// large.  So we add the next conditional to cut down their sizes for
// these small cases:

   else if (longitude_ptr->get_stop()-longitude_ptr->get_start() < 0.005)
   {
      if (ndigits_after_decimal==4)
      {
         x_tick_char_label_size *= 0.5;
         x_axis_char_label_size *= 0.5;
      }
   }

   double y_tick_char_label_size=tick_char_label_size;
   double y_axis_char_label_size=axis_char_label_size;
   if (latitude_ptr->get_stop()-latitude_ptr->get_start() > 1.0)
   {
      y_tick_char_label_size *= 40;
      y_axis_char_label_size *= 30;
   }
   else if (latitude_ptr->get_stop()-latitude_ptr->get_start() > 0.1)
   {
      if (ndigits_after_decimal==2)
      {
         y_tick_char_label_size *= 4;
      }
      else
      {
         y_tick_char_label_size *= 2.5;
      }
      y_axis_char_label_size *= 3;
   }
   else if (latitude_ptr->get_stop()-latitude_ptr->get_start() < 0.005)
   {
      if (ndigits_after_decimal==4)
      {
         y_tick_char_label_size *= 0.5;
         y_axis_char_label_size *= 0.5;
      }
   }

   tick_char_label_size=basic_math::max(x_tick_char_label_size,y_tick_char_label_size);
   axis_char_label_size=basic_math::max(x_axis_char_label_size,y_axis_char_label_size);

//   cout << "tick_char_label_size = " << tick_char_label_size << endl;
//   cout << "axis_char_label_size = " << axis_char_label_size << endl;
}

// -------------------------------------------------------------------------
void LatLongGrid::update_grid()
{
//   cout << "inside LatLongGrid::update_grid()" << endl;
   set_xy_size(0.0);

   compute_relative_latlong_lines();

   latitude_ptr->set_include_stop_point_flag(true);
   longitude_ptr->set_include_stop_point_flag(true);
   vector<double>* longs_ptr=longitude_ptr->get_value_sequence();
   vector<double>* lats_ptr=latitude_ptr->get_value_sequence();

//   for (int i=0; i<longs_ptr->size(); i++)
//   {
//      cout << "i = " << i 
//           << " longs_ptr->at(i) = " << longs_ptr->at(i) << endl;
//   }

//   for (int i=0; i<lats_ptr->size(); i++)
//   {
//      cout << "i = " << i 
//           << " lats_ptr->at(i) = " << lats_ptr->at(i) << endl;
//   }

// Axes' labels:

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

   x_axis_label="Longitude (degs)";
   if (longs_ptr->front() < 0 && longs_ptr->back() < 0)
   {
      x_axis_label="West Longitude (degs)";
   }

   x_axis_text_refptr->setText(x_axis_label);
   x_axis_text_refptr->setFont("fonts/times.ttf");
   x_axis_text_refptr->setCharacterSize(axis_char_label_size);
   //x_axis_text_refptr->setCharacterSizeMode(osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);

   double curr_north,curr_east,next_north,next_east;
   double relative_z=zplane-world_middle.get(2);

   bool specified_northern_hemisphere_flag=true;
   latlongfunc::LL_to_northing_easting(
      longitude_ptr->get_avg_value(),latitude_ptr->get_start(),
      specified_northern_hemisphere_flag,UTM_zonenumber,curr_east,curr_north);

   curr_east -= world_middle.get(0);
   curr_north -= world_middle.get(1);

   const double numer=800;
   const double factor=numer/1650.0;
//   const double factor=1.0;
   double x_extent=factor*axis_char_label_size*x_axis_label.size();
   osg::Vec3 posx(curr_east-0.5*x_extent,curr_north-1.5*axis_char_label_size,
                  relative_z);
   x_axis_text_refptr->setPosition(posx);

// Rotate x axis label so that it's locally parallel to longitude axis
// at its emplacement point:

   latlongfunc::LL_to_northing_easting(
      longitude_ptr->get_avg_value()+longitude_ptr->get_delta(),
      latitude_ptr->get_start(),
      specified_northern_hemisphere_flag,UTM_zonenumber,next_east,next_north);

   next_east -= world_middle.get(0);
   next_north -= world_middle.get(1);

   double theta_x=atan2(next_north-curr_north,next_east-curr_east);
   osg::Quat q;
   q.makeRotate(theta_x,osg::Vec3f(0,0,1));
   x_axis_text_refptr->setRotation(q);

   string y_axis_label="Latitude (degs)";
   y_axis_text_refptr->setText(y_axis_label);
   y_axis_text_refptr->setFont("fonts/times.ttf");
   y_axis_text_refptr->setCharacterSize(axis_char_label_size);
   //y_axis_text_refptr->setCharacterSizeMode(osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);

   latlongfunc::LL_to_northing_easting(
      longitude_ptr->get_start(),
      latitude_ptr->get_avg_value(),
      specified_northern_hemisphere_flag,UTM_zonenumber,curr_east,curr_north);

   curr_east -= world_middle.get(0);
   curr_north -= world_middle.get(1);

   double y_extent=factor*axis_char_label_size*y_axis_label.size();
   osg::Vec3 posy(curr_east-1.5*axis_char_label_size,
                  curr_north-0.5*y_extent,relative_z);
   y_axis_text_refptr->setPosition(posy);

// Rotate y axis label so that it's locally parallel to longitude axis
// at its emplacement point:
   
   latlongfunc::LL_to_northing_easting(
      longitude_ptr->get_start(),
      latitude_ptr->get_avg_value()+latitude_ptr->get_delta(),
      specified_northern_hemisphere_flag,UTM_zonenumber,next_east,next_north);

   next_east -= world_middle.get(0);
   next_north -= world_middle.get(1);

   double theta_y=atan2(next_north-curr_north,next_east-curr_east);
   q.makeRotate(theta_y,osg::Vec3f(0,0,1));
   y_axis_text_refptr->setRotation(q);

// Tick labels:

// Remove x axis tick text labels from geode and instantiate a new set
// of labels.  Garbage cleaning should (hopefully!) automatically
// delete unused memory slots.  Then reattach new labels to geode:

   for (unsigned int i=0; i<x_axis_tick_texts.size(); i++)
   {
      if (x_axis_tick_texts[i] != NULL)
         geode_refptr->removeDrawable(x_axis_tick_texts[i].get());
   }
   x_axis_tick_texts.clear();

   for (unsigned int i = 0; i < longs_ptr->size(); i++)
   {
      string tick_string=stringfunc::number_to_string(
         longs_ptr->at(i),ndigits_after_decimal);
      if (longs_ptr->front() < 0 && longs_ptr->back() < 0)
      {
         tick_string=stringfunc::number_to_string(
            -longs_ptr->at(i),ndigits_after_decimal);
      }
      
// For reasons which we do not understand as of 4/27/06, it appears to
// be necessary to introduce a fudge prefactor of 0.5 into the
// computation of x_extent in order to end up with tick labels that
// are centered reasonably well about grid lines:

//      x_extent=tick_char_label_size*tick_string.size();
      x_extent=0.5*tick_char_label_size*tick_string.size();
      latlongfunc::LL_to_northing_easting(
         longs_ptr->at(i),latitude_ptr->get_start(),
         specified_northern_hemisphere_flag,UTM_zonenumber,
         curr_east,curr_north);

      curr_east -= world_middle.get(0);
      curr_north -= world_middle.get(1);

      osg::Vec3 tick_pos(
         curr_east-0.5*x_extent,curr_north-0.5*axis_char_label_size,
         relative_z);

      osgText::Text* x_tick_text_ptr = new osgText::Text;
      x_tick_text_ptr->setFont("fonts/times.ttf");
      x_tick_text_ptr->setCharacterSize(tick_char_label_size);
//      x_tick_text_ptr->setCharacterSizeMode(
//         osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
      x_tick_text_ptr->setPosition(tick_pos);
      x_tick_text_ptr->setAxisAlignment(osgText::Text::SCREEN);
      x_tick_text_ptr->setText(tick_string);
       
      x_axis_tick_texts.push_back(x_tick_text_ptr);
      geode_refptr->addDrawable(x_tick_text_ptr);
   } // loop over index i labeling entries in *longs_ptr

// Remove y axis tick text labels from geode and instantiate a new set
// of labels.  Garbage cleaning should (hopefully!) automatically
// delete unused memory slots.  Then reattach new labels to geode:

   for (unsigned int i=0; i<y_axis_tick_texts.size(); i++)
   {
      if (y_axis_tick_texts[i] != NULL)
         geode_refptr->removeDrawable(y_axis_tick_texts[i].get());
   }
   y_axis_tick_texts.clear();

   for (unsigned int i = 0; i < lats_ptr->size(); i++)
   {
      string tick_string=stringfunc::number_to_string(
         lats_ptr->at(i),ndigits_after_decimal);

      x_extent=0.6*tick_char_label_size*tick_string.size();
      latlongfunc::LL_to_northing_easting(
         longitude_ptr->get_start(),lats_ptr->at(i),
         specified_northern_hemisphere_flag,UTM_zonenumber,
         curr_east,curr_north);

      curr_east -= world_middle.get(0);
      curr_north -= world_middle.get(1);

      osg::Vec3 tick_pos(
         curr_east-x_extent,curr_north,relative_z);

      osgText::Text* y_tick_text_ptr = new osgText::Text;
      y_tick_text_ptr->setFont("fonts/times.ttf");
      y_tick_text_ptr->setCharacterSize(tick_char_label_size);
//      y_tick_text_ptr->setCharacterSizeMode(
//         osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
      y_tick_text_ptr->setPosition(tick_pos);
      y_tick_text_ptr->setAxisAlignment(osgText::Text::SCREEN);
      y_tick_text_ptr->setText(tick_string);
      y_axis_tick_texts.push_back(y_tick_text_ptr);
      geode_refptr->addDrawable(y_tick_text_ptr);
   } // loop over index i labeling entries in *lats_ptr

   update_grid_text_color();

   delete longs_ptr;
   delete lats_ptr;
}

// ---------------------------------------------------------------------
// Member function update_grid_text_color() sets the color of the
// axes' and ticks' labels to the current dynamic grid color.

void LatLongGrid::update_grid_text_color()
{
//   cout << "inside LatLongGrid:update_grid_text_color()" << endl;
   x_axis_text_refptr->setColor(get_dynamic_grid_color());
   y_axis_text_refptr->setColor(get_dynamic_grid_color());
   for (unsigned int i=0; i<x_axis_tick_texts.size(); i++)
   {
      x_axis_tick_texts[i]->setColor(get_dynamic_grid_color());
   }
   for (unsigned int i=0; i<y_axis_tick_texts.size(); i++)
   {
      y_axis_tick_texts[i]->setColor(get_dynamic_grid_color());
   }
}

// -------------------------------------------------------------------------
// Member function compute_relative_latlong_lines generates a
// longitude-latitude lattice and computes the lattice site locations
// in UTM coordinates.  It adds those lattice sites as osg::Vec3
// vertices to the *geom member of the Grid class.  It also computes
// local estimates for the angle about local Z-hat about which the UTM
// subgrid needs to be rotated in order for it to align with
// underlying lines of longitude and latitude on the earth's ellipsoid.

void LatLongGrid::compute_relative_latlong_lines()
{
//   cout << "inside LatLongGrid::compute_relative_latlong_lines()" << endl;

   vertices_refptr->clear();
   UTM_to_longitude_rot_angle.clear();
   UTM_to_latitude_rot_angle.clear();

// Next set up relative lines of longitude:

   longitude_ptr->set_include_stop_point_flag(true);
   latitude_ptr->set_include_stop_point_flag(false);

   while (longitude_ptr->prepare_next_value())
   {
      double curr_longitude=longitude_ptr->get_value();
      while (latitude_ptr->prepare_next_value())
      {
         double curr_latitude=latitude_ptr->get_value();
         double next_latitude=latitude_ptr->get_next_value();
         
         bool specified_northern_hemisphere_flag=true;
         double curr_north,next_north,curr_east,next_east;
         latlongfunc::LL_to_northing_easting(
            curr_longitude,curr_latitude,
            specified_northern_hemisphere_flag,UTM_zonenumber,
            curr_east,curr_north);
         latlongfunc::LL_to_northing_easting(
            curr_longitude,next_latitude,
            specified_northern_hemisphere_flag,UTM_zonenumber,
            next_east,next_north);

         double d_east=next_east-curr_east;
         double d_north=next_north-curr_north;
         UTM_to_longitude_rot_angle.push_back(atan2(d_north,d_east));

//         cout.precision(12);
//         cout << "curr_long = " << curr_longitude 
//              << " curr_lat = " << curr_latitude
//              << " next_lat = " << next_latitude << endl;
//         cout << " curr_north = " << curr_north 
//              << " next_north = " << next_north
//              << " curr_east = " << curr_east 
//              << " next_east = " << next_east  << endl;
//         cout << "d_north = " << d_north << " d_east = " << d_east << endl;
//         cout << "world_middle = " << world_middle << endl;

         vertices_refptr->push_back(osg::Vec3(
            curr_east-world_middle.get(0),
            curr_north-world_middle.get(1),
            zplane-world_middle.get(2)));
         vertices_refptr->push_back(osg::Vec3(
            next_east-world_middle.get(0),
            next_north-world_middle.get(1),
            zplane-world_middle.get(2)));
      } // latitude_ptr while loop
   } // longitude_ptr while loop

// Set up relative lines of latitude:

   latitude_ptr->set_include_stop_point_flag(true);
   longitude_ptr->set_include_stop_point_flag(false);
  
   while (latitude_ptr->prepare_next_value())
   {
      double curr_latitude=latitude_ptr->get_value();
      while (longitude_ptr->prepare_next_value())
      {
         double curr_longitude=longitude_ptr->get_value();
         double next_longitude=longitude_ptr->get_next_value();

         bool specified_northern_hemisphere_flag=true;
         double curr_north,next_north,curr_east,next_east;
         latlongfunc::LL_to_northing_easting(
            curr_longitude,curr_latitude,
            specified_northern_hemisphere_flag,UTM_zonenumber,
            curr_east,curr_north);
         latlongfunc::LL_to_northing_easting(
            next_longitude,curr_latitude,
            specified_northern_hemisphere_flag,UTM_zonenumber,
            next_east,next_north);

         double d_east=next_east-curr_east;
         double d_north=next_north-curr_north;
         UTM_to_latitude_rot_angle.push_back(atan2(d_north,d_east));

//         cout.precision(12);
//         cout << "curr_lat = " << curr_latitude 
//              << " curr_long = " << curr_longitude
//              << " next_long = " << next_longitude << endl;
//         cout << " curr_east = " << curr_east 
//              << " next_east = " << next_east 
//              << " d_east = " << next_east-curr_east << endl;
//         cout << " curr_north = " << curr_north 
//              << " next_north = " << next_north
//              << " d_north = " << next_north-curr_north << endl << endl;

         vertices_refptr->push_back(osg::Vec3(
            curr_east-world_middle.get(0),
            curr_north-world_middle.get(1),
            zplane-world_middle.get(2)));
         vertices_refptr->push_back(osg::Vec3(
            next_east-world_middle.get(0),
            next_north-world_middle.get(1),
            zplane-world_middle.get(2)));
      } // longitude_ptr while loop
   } // latitude_ptr while loop

   longitude_ptr->set_include_stop_point_flag(false);
   latitude_ptr->set_include_stop_point_flag(false);

   geom_refptr->addPrimitiveSet(new osg::DrawArrays(
      osg::PrimitiveSet::LINES,0,vertices_refptr->size()));
}

// =========================================================================
// Long-lat to UTM conversion member functions
// =========================================================================

// Member function get_or_compute_UTM_to_latlong_gridlines_rot_angle()
// returns the rotation (measured in radians) within the
// east_hat/north_hat plane about the UTM grid's midpoint which
// optimally aligns the UTM grid with underlying lines of longitude &
// latitude.  After this rotation angle has been calculated once, it
// is stored and can be retrieved via this method with little expense.

double LatLongGrid::get_or_compute_UTM_to_latlong_gridlines_rot_angle()
{
//   cout << "inside LatLongGrid::get_or_compute_UTM_to_latlong_gridlines_rot_angle()" << endl;

   if (UTM_grid_angle_wrt_LL_grid > 0.5*NEGATIVEINFINITY)
   {
      return UTM_grid_angle_wrt_LL_grid;
   }
   else
   {
      double avg_UTM_to_longitude_rot_angle=0;
      for (unsigned int i=0; i<UTM_to_longitude_rot_angle.size(); i++)
      {
         avg_UTM_to_longitude_rot_angle += UTM_to_longitude_rot_angle[i];
      }
      
      avg_UTM_to_longitude_rot_angle /= 
         double(UTM_to_longitude_rot_angle.size());
      avg_UTM_to_longitude_rot_angle -= PI/2;
   
      double avg_UTM_to_latitude_rot_angle=0;
      for (int i=0; i<int(UTM_to_latitude_rot_angle.size()); i++)
      {
         avg_UTM_to_latitude_rot_angle += UTM_to_latitude_rot_angle[i];
      }
      avg_UTM_to_latitude_rot_angle /= 
         double(UTM_to_latitude_rot_angle.size());

      UTM_grid_angle_wrt_LL_grid=0.5*(
         avg_UTM_to_longitude_rot_angle+avg_UTM_to_latitude_rot_angle);
//      cout << "UTM_grid_angle_wrt_LL_grid = "
//           << UTM_grid_angle_wrt_LL_grid*180/PI  << " degs" << endl;

      return UTM_grid_angle_wrt_LL_grid;
   } // UTM_grid_angle_wrt_LL_grid > 0.5*NEGATIVEINFINITY conditional
}

// =========================================================================
// Dynamic grid computation member functions
// =========================================================================

// Member function initialize_extremal_longitudes_and_latitudes()
// computes a long/lat bounding box which should comfortably enclose
// the current field-of-view.  The starting and stopping longitudes
// are brought to a canonical interval so that long_start < long_stop.

void LatLongGrid::initialize_extremal_longitudes_and_latitudes()
{
//   cout << "inside LatLongGrid::init_extremal_longs_and_lats()" << endl;
   
   long_min=-180;		// degs
   long_start=long_min;    	// degs
   long_stop=long_start+360;	// degs
   lat_start=-89.9;       	// degs
   lat_stop=89.9;		// degs

   const int max_n_FOV_intersection_geopoints=4*2;

//   cout << "LOS_intersection_geopoint = "
//        << LOS_intersection_geopoint << endl;

   if (int(FOV_intersection_geopoint.size())==
       max_n_FOV_intersection_geopoints)
   {
      double LOS_longitude=basic_math::phase_to_canonical_interval(
         LOS_intersection_geopoint.get_longitude(),-180,180);
//         LOS_intersection_geopoint.get_longitude(),0,360);
//      cout << "LOS longitude = " << LOS_longitude << endl;
      if (fabs(LOS_longitude-180) < 90) long_min=0;

      long_start=POSITIVEINFINITY;
      long_stop=NEGATIVEINFINITY;
      lat_start=POSITIVEINFINITY;
      lat_stop=NEGATIVEINFINITY;
      for (unsigned int r=0; r<FOV_intersection_geopoint.size(); r++)
      {
         geopoint curr_geopoint(FOV_intersection_geopoint[r]);
         double curr_longitude=curr_geopoint.get_longitude();
         curr_longitude=basic_math::phase_to_canonical_interval(
               curr_longitude,long_min,long_min+360);
         long_start=basic_math::min(long_start,curr_longitude);
         long_stop=basic_math::max(long_stop,curr_longitude);

         lat_start=basic_math::min(lat_start,curr_geopoint.get_latitude());
         lat_stop=basic_math::max(lat_stop,curr_geopoint.get_latitude());
      }
   }

// If the window containing the earth is resized on the ISDS3D laptop
// so that its width fills the entire screen, the longitude extent
// needs to be increased beyond that set by FOV_intersection_geopoint.
// To be safe, we choose to double the longitude extent to handle this
// case:

//   cout << "initial long_min = " << long_min 
//        << " long_start = " << long_start << " long_stop = " << long_stop
//        << endl;

   double long_difference=long_stop-long_start;
   if (long_difference < 180)
   {
      double long_middle=0.5*(long_start+long_stop);
      long_start=long_middle-long_difference;
      long_stop=long_middle+long_difference;
   }

//   cout << "initial lat_start = " << lat_start << " lat_stop = " << lat_stop
//        << endl;

   double lat_difference=lat_stop-lat_start;
   if (lat_difference < 120)
   {
      double lat_middle=0.5*(lat_start+lat_stop);
      lat_start=lat_middle-0.75*lat_difference;
      lat_stop=lat_middle+0.75*lat_difference;
      lat_start=basic_math::max(lat_start,-89.9);
      lat_stop=basic_math::min(lat_stop,89.9);
   }

   crop_latlong_bounds();

//   cout << "final long_min = " << long_min 
//        << " long_start = " << long_start << " long_stop = " << long_stop
//        << endl;
//   cout << "lat_start = " << lat_start << " lat_stop = " << lat_stop
//        << endl;
}

// ---------------------------------------------------------------------
// Member function crop_latlong_bounds()

void LatLongGrid::crop_latlong_bounds()
{
//   cout << "inside LatLongGrid::crop_latlong_bounds()" << endl;
   if (longitude_ptr != NULL)
   {
      long_min=basic_math::max(long_min,longitude_ptr->get_start());
      long_start=basic_math::max(long_start,longitude_ptr->get_start());
      long_stop=basic_math::min(long_stop,longitude_ptr->get_stop());
   }

   if (latitude_ptr != NULL)
   {
      lat_start=basic_math::max(lat_start,latitude_ptr->get_start());
      lat_stop=basic_math::min(lat_stop,latitude_ptr->get_stop());
   }

//   cout << "long_start = " << long_start << " long_stop = " << long_stop 
//        << endl;
//   cout << "lat_start = " << lat_start << " lat_stop = " << lat_stop 
//        << endl;
}

// ---------------------------------------------------------------------
// Member function compute_polyline_vertex_angular_separation() adjusts
// angular starting and stopping values so that they fall on a
// regularized lattice.  It also adjust the number of latitude
// [longitude] segments which comprise each longitude [latitude] grid
// line.

void LatLongGrid::compute_polyline_vertex_angular_separation(
   double& theta_start,double& theta_stop,
   double min_theta,double max_theta,int& n_theta_bins,double& d_theta)
{
//   cout << "inside LatLongGrid::compute_polyline_vertex_angular_separation()"
//        << endl;

   const double min_angular_separation_between_gridlines=1E-5;	// degs

   theta_start=sgn(theta_start)*basic_math::mytruncate(
      fabs(theta_start)/min_angular_separation_between_gridlines)*
      min_angular_separation_between_gridlines;
   theta_stop=sgn(theta_stop)*(basic_math::mytruncate(
      fabs(theta_stop)/min_angular_separation_between_gridlines)+1)*
      min_angular_separation_between_gridlines;

   const double vertex_sampling_frac_within_gridline=0.01;

   theta_start=basic_math::max(theta_start,min_theta);
   theta_stop=basic_math::min(theta_stop,max_theta);
   d_theta=vertex_sampling_frac_within_gridline*(theta_stop-theta_start);  
   n_theta_bins=static_cast<int>((theta_stop-theta_start)/d_theta+1);

//   cout << "theta_start = " << theta_start 
//        << " theta_stop = " << theta_stop << endl;
}

// ---------------------------------------------------------------------
// Member function angular_separation_between_lines() resets angular
// spacing between lines of longitude & latitude based upon camera's
// distance from screen center intercept on LatLongGrid as well as
// maximal angular separation between currently visible lines of
// longitude & latitude.

void LatLongGrid::angular_separation_between_lines(
   double log_eye_center_dist,
   int& n_precision,double& delta_theta,double& text_size,double& linewidth)
{
//   cout << "inside LatLongGrid::angular_separation_between_lines()" << endl;
   n_precision=0;
   text_size=25.0;
   linewidth=init_linewidth;

   if (log_eye_center_dist >= 7.5)
   {
      delta_theta=30;
   }
   else if (log_eye_center_dist >= 7.1 && log_eye_center_dist < 7.5)
   {
      delta_theta=20;
   }
   else if (log_eye_center_dist >= 6.75 && log_eye_center_dist < 7.1)
   {
      delta_theta=10;
   }
   else if (log_eye_center_dist >= 6.1 && log_eye_center_dist < 6.75)
   {
      delta_theta=5;
   }
   else if (log_eye_center_dist >= 5.5 && log_eye_center_dist < 6.1)
   {
      delta_theta=1;
   }
   else if (log_eye_center_dist >= 4.92 && log_eye_center_dist < 5.5)
   {
      delta_theta=0.2;
      n_precision=1;
   }
   else if (log_eye_center_dist >= 4.4 && log_eye_center_dist < 4.92)
   {
      delta_theta=0.1;
      n_precision=1;
   }
   else if (log_eye_center_dist >= 4.1 && log_eye_center_dist < 4.4)
   {
      delta_theta=0.05;
      n_precision=2;
   }
   else if (log_eye_center_dist >= 3.8 && log_eye_center_dist < 4.1)
   {
      delta_theta=0.02;
      n_precision=2;
      text_size=30.0;
      linewidth=1.5*init_linewidth;
   }
   else if (log_eye_center_dist >= 3.3 && log_eye_center_dist < 3.8)
   {
      delta_theta=0.01;
      n_precision=2;
      text_size=35.0;
      linewidth=2.0*init_linewidth;
   }
   else if (log_eye_center_dist < 3.3)
   {
      delta_theta=0.005;
      n_precision=3;
      text_size=40.0;
      linewidth=2.5*init_linewidth;
   }

//   cout << "At end of LatLongGrid::angular_separation_between_lines()" 
//	  << endl;

//   cout << "log_eye_center_dist = " << log_eye_center_dist
//        << " delta_theta = " << delta_theta << endl;
//   cout << "n_precision = " << n_precision << endl;
}

// ---------------------------------------------------------------------
// Member function compute_LOS_intersection_geopoint() determines
// whether the camera's line-of-sight intersects the Earth's
// ellipsoid.  If so, this boolean method returns true as well as the
// distance from the camera to the intercept point within output
// parameter camera_LOS_intercept_dist.  It also stores the
// intersection point's longitude and latitude within member geopoint
// LOS_intersection_geopoint.

bool LatLongGrid::compute_LOS_intersection_geopoint(
   const threevector& camera_ECI_posn,const threevector& camera_Zhat,
   Clock* Clock_ptr,double& camera_LOS_intercept_dist)
{
//   cout << "inside LatLongGrid::compute_LOS_intersection_geopoint()" << endl;
//   cout << "camera_ECI_posn = " << camera_ECI_posn << endl;
//   cout << "camera_Zhat = " << camera_Zhat << endl;

   threevector LOS_intercept_point;
   bool LOS_intersects_ellipsoid=geofunc::groundpoints_ellipsoidal_earth(
      camera_ECI_posn,-camera_Zhat,LOS_intercept_point);
   if (LOS_intersects_ellipsoid)
   {
      camera_LOS_intercept_dist=(camera_ECI_posn-LOS_intercept_point).
         magnitude();
      double latitude,longitude,altitude;
      Ellipsoid_model_ptr->ConvertECIToLongLatAlt(
         LOS_intercept_point,*Clock_ptr,longitude,latitude,altitude);
      LOS_intersection_geopoint=geopoint(longitude,latitude);
   }

//   cout << "LOS_intersects_ellipsoid = "
//        << LOS_intersects_ellipsoid << endl;
   
   return LOS_intersects_ellipsoid;
}

// ---------------------------------------------------------------------
// Member function compute_FOV_intersection_geopoints() computes the
// intersection between each of the ViewFrustum's 4 rays with the
// ellipsoidal model.  If a ray actually intercepts the earth's
// surface, the (longitude,latitude) coordinates of the intercept
// point are saved within member STL vector FOV_intersection_geopoint.
// We use the information stored within FOV_intersection_geopoint for
// longitude/latitude ellipsoid grid drawing purposes.

vector<geopoint>& LatLongGrid::compute_FOV_intersection_geopoints(
   Clock* Clock_ptr)
{
//   cout << "inside LatLongGrid::compute_FOV_intersection_geopoints()" << endl;
   FOV_intersection_geopoint.clear();
    
   threevector camera_ECI_posn=CM_3D_ptr->get_eye_world_posn();
   ViewFrustum* ViewFrustum_ptr=CM_3D_ptr->get_ViewFrustum_ptr();

   for (unsigned int r=0; r<ViewFrustum_ptr->get_ray().size(); r++)
   {
      threevector curr_intercept_point;
      if (geofunc::groundpoints_ellipsoidal_earth(
             camera_ECI_posn,ViewFrustum_ptr->get_ray().at(r),
             curr_intercept_point))
      {
         double latitude,longitude,altitude;
         Ellipsoid_model_ptr->ConvertECIToLongLatAlt(
            curr_intercept_point,*Clock_ptr,longitude,latitude,altitude);
//         cout << "r = " << r 
//              << " longitude = " << longitude << " latitude = " << latitude
//              << endl;
         FOV_intersection_geopoint.push_back(geopoint(longitude,latitude));

// Compute (long,lat) pairs at weighted averages of intercept points
// corresponding to ViewFrustum rays r and r+1:

         int next_r=modulo(r+1,ViewFrustum_ptr->get_ray().size());

         threevector next_intercept_point;
         if (geofunc::groundpoints_ellipsoidal_earth(
                camera_ECI_posn,ViewFrustum_ptr->get_ray().at(next_r),
                next_intercept_point))
         {
            for (int j=5; j<=5; j++)
            {
               double curr_frac=0.1*j;
               threevector avg_intercept_point=
                  curr_frac*(curr_intercept_point+next_intercept_point);
               Ellipsoid_model_ptr->ConvertECIToLongLatAlt(
                  avg_intercept_point,*Clock_ptr,longitude,latitude,altitude);
               FOV_intersection_geopoint.push_back(
                  geopoint(longitude,latitude));
            } // loop over index j labeling intermediate intercept pnts
         } // next_intercept_point intercepts blue marble conditional
      } // curr_intercept_point intercepts blue marble conditional
   } // loop over index r labeling view frustum rays

   return FOV_intersection_geopoint;
}

// ---------------------------------------------------------------------
vector<geopoint>& LatLongGrid::compute_FOV_intersection_geopoints()
{
   FOV_intersection_geopoint.clear();
    
   ViewFrustum* ViewFrustum_ptr=CM_3D_ptr->get_ViewFrustum_ptr();
//   threevector camera_posn=ViewFrustum_ptr->get_camera_posn();
   threevector camera_posn=CM_3D_ptr->get_eye_world_posn();  

   for (unsigned int r=0; r<ViewFrustum_ptr->get_ray().size(); r++)
   {
      geopoint curr_intercept_geopoint=ray_geopoint_intercept_on_grid(
         ViewFrustum_ptr->get_ray().at(r));
      FOV_intersection_geopoint.push_back(curr_intercept_geopoint);
      
// Compute (long,lat) pairs at weighted averages of intercept points
// corresponding to ViewFrustum rays r and r+1:

      int next_r=modulo(r+1,ViewFrustum_ptr->get_ray().size());

      geopoint next_intercept_geopoint=ray_geopoint_intercept_on_grid(
         ViewFrustum_ptr->get_ray().at(next_r));

      for (int j=5; j<=5; j++)
      {
         double curr_frac=0.1*j;
         threevector avg_intercept_point=
            curr_frac*(curr_intercept_geopoint.get_UTM_posn()+
                       next_intercept_geopoint.get_UTM_posn());
         geopoint avg_intercept_geopoint(
            curr_intercept_geopoint.get_northern_hemisphere_flag(),
            curr_intercept_geopoint.get_UTM_zonenumber(),
            avg_intercept_point.get(0),avg_intercept_point.get(1),
            avg_intercept_point.get(2));
         FOV_intersection_geopoint.push_back(avg_intercept_geopoint);

      } // loop over index j labeling intermediate intercept pnts
   } // loop over index r labeling view frustum rays

   return FOV_intersection_geopoint;
}

// ---------------------------------------------------------------------
// Member function draw_longitude_lines first determines the angular
// separation between lines of longitude based upon the camera's
// current altitude above the earth's surface.  It next determines the
// number of latitude line segments which comprise each visible
// longitude line.  Working in geocentric XYZ coordinates, this method
// fills STL vectors with threevector long/lat vertices for each line
// of longitude.  The vertices are then passed into polyline
// constructors which are members of the LatLongGrid class'
// *get_LongLinesGroup_ptr() and *get_LatLinesGroup_ptr() members.

void LatLongGrid::draw_longitude_lines(
   double log_eye_center_dist,double eye_alt,
   int curr_iter,double annotation_altitude,bool refresh_flag)
{
//   cout << "inside LatLongGrid::draw_longitude_lines()" << endl;

   int n_long,n_lat,n_precision;
   double d_lat,text_size,linewidth;
   set_longitude_line_spacing(
      log_eye_center_dist,n_long,n_lat,d_lat,
      n_precision,text_size,linewidth);

//   cout << "long_start = " << long_start 
//        << " delta_long = " << get_delta_longitude() << endl;
//   cout << "n_long = " << n_long << " n_lat = " << n_lat
//        << " d_lat = " << d_lat << endl;

// Compute ECI vertices for polylines representing longitude grid
// lines:

   vector<threevector> V_XYZ;
   for (int i=0; i<n_long; i++)
   {
      double longitude=long_start+i*get_delta_longitude();
//      cout << "i = " << i << " longitude = " << longitude << endl;
      
      LINES_MAP::iterator PolyLine_iter=get_longitude_lines_map_ptr()->find(
         threevector(longitude,lat_start,lat_stop));

      PolyLine* curr_PolyLine_ptr=NULL;

      if (PolyLine_iter != get_longitude_lines_map_ptr()->end())
      {
         PolyLine* retrieved_PolyLine_ptr=PolyLine_iter->second.first;
         curr_PolyLine_ptr=retrieved_PolyLine_ptr;
         PolyLine_iter->second.second=curr_iter;
      } // PolyLine_iter != get_longitude_lines_map_ptr()->end() conditional

      if (curr_PolyLine_ptr==NULL || refresh_flag)
      {
         V_XYZ.clear();
         for (int j=0; j<=n_lat; j++)
         {
            V_XYZ.push_back(Zero_vector);
         }

         double prev_lat=lat_start;
         double prev_alt=0;
         double next_alt=0;
         for (int j=0; j<n_lat; j++)
         {
            double next_lat=prev_lat+d_lat;
            adjust_endpoint_altitudes(
               longitude,prev_lat,prev_alt,
               longitude,next_lat,next_alt,
               eye_alt,V_XYZ[j],V_XYZ[j+1]);

            prev_lat=next_lat;
            prev_alt=next_alt;
         } // loop over index j labeling latitude

//         cout << "V_XYZ.size() = " << V_XYZ.size() << endl;
//         for (int j=0; j<V_XYZ.size(); j++)
//         {
//            cout << "j = " << j 
//                 << " V_X = " << V_XYZ[j].get(0)
//                 << " V_X = " << V_XYZ[j].get(1)
//                 << " V_X = " << V_XYZ[j].get(2) << endl;
         //        }
         
         if (V_XYZ.size() >= 2)
         {
            bool single_polyline_per_geode_flag=true;
            int n_text_messages=1;
            threevector reference_vertex=
               get_LongLinesGroup_ptr()->compute_vertices_average(V_XYZ);

            curr_PolyLine_ptr=get_LongLinesGroup_ptr()->generate_new_PolyLine(
               reference_vertex,V_XYZ,get_dynamic_grid_color(),
               depth_buffering_off_flag,
               single_polyline_per_geode_flag,n_text_messages);

            curr_PolyLine_ptr->set_stationary_Graphical_flag(true);

            threevector curr_lll(longitude,lat_start,lat_stop);
            (*get_longitude_lines_map_ptr())[curr_lll]=pair<PolyLine*,int>(
               curr_PolyLine_ptr,curr_iter);

            curr_PolyLine_ptr->set_linewidth(linewidth);

//            cout << "*curr_PolyLine_ptr = " << *curr_PolyLine_ptr << endl;
//            outputfunc::enter_continue_char();

            string text_label;
            longitude=basic_math::phase_to_canonical_interval(
               longitude,-180,180);
//            cout << "longitude = " << longitude << endl;

            if (longitude >= -180 && longitude < 0)
            {
               if (basic_math::is_int(longitude))
               {
                  text_label=
                     "W"+stringfunc::number_to_string(fabs(longitude));
               }
               else
               {
                  text_label="W"+stringfunc::number_to_string(
                     fabs(longitude),mathfunc::ndigits_after_decimal_point(
                        fabs(longitude)));
               } // is_int conditional
            }
            else if (longitude >= 0 && longitude < 180)
            {
               if (basic_math::is_int(longitude))
               {
                  text_label=
                     "E"+stringfunc::number_to_string(longitude);
               }
               else
               {
                  text_label="E"+stringfunc::number_to_string(
                     longitude,mathfunc::ndigits_after_decimal_point(
                        longitude));
               } // is_int conditional
            }
            if (text_label=="E0" || text_label=="W0") 
            {
               text_label="Prime Meridian";
//               curr_PolyLine_ptr->set_linewidth(2*linewidth);
               curr_PolyLine_ptr->set_color(osg::Vec4(0.6, 0.6, 0.6, 1.0));
            }

//            cout << "Text label = " << text_label << endl;
            curr_PolyLine_ptr->set_text_label(0,text_label);
            curr_PolyLine_ptr->set_text_color(0,get_dynamic_grid_color());

         } // V_XYZ.size() >= 2 conditional
      } // curr_PolyLine_ptr != NULL conditional
      
      if (curr_PolyLine_ptr != NULL)
      {
         threevector text_posn;
         if (flat_grid_flag)
         {

// Restrict flat grid text labels to appear inside the LatLongGrid:

            double text_latitude=LOS_intersection_geopoint.get_latitude();
            text_latitude=basic_math::max(
               text_latitude,latitude_ptr->get_start());
            text_latitude=basic_math::min(
               text_latitude,latitude_ptr->get_stop());
            
            text_posn=geopoint(
               longitude,text_latitude,
               1.1*annotation_altitude,UTM_zonenumber).get_UTM_posn();

            double chi=90.0*PI/180.0;
            curr_PolyLine_ptr->set_text_rotation(0,chi);
         }
         else
         {
            if (curr_PolyLine_ptr->get_text_refptr_valid(0))
            {
               text_posn=Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(
                  longitude,LOS_intersection_geopoint.get_latitude(),
                  1.1*annotation_altitude);
               
// Orient text along lines-of-longitude so that it runs from south to
// north and lies flat within the local tangent plane:

               bool north_south_flag=true;
               Ellipsoid_model_ptr->align_text_with_cardinal_dirs(
                  longitude,LOS_intersection_geopoint.get_latitude(),
                  curr_PolyLine_ptr->get_text_ptr(0),north_south_flag);
            } // text_refptr_valid(0) conditional
         }

         curr_PolyLine_ptr->set_text_posn(0,text_posn);
//         cout << "text_posn = " << text_posn << endl;
         update_earthline_text_size(eye_alt,curr_PolyLine_ptr,text_size);

      } // curr_Polyline_ptr != NULL conditional
    
   } // loop over index i labeling longitude
}

// ---------------------------------------------------------------------
// Member function set_longitude_line_spacing()

void LatLongGrid::set_longitude_line_spacing(
   double log_eye_center_dist,
   int& n_long,int& n_lat,double& d_lat,
   int& n_precision,double& text_size,double& linewidth)
{
//   cout << "inside LatLongGrid::set_longitude_line_spacing()" << endl;
   
   initialize_extremal_longitudes_and_latitudes();

// Reset angular spacing between lines of longitude based upon
// camera's altitude above earth's surface:

   angular_separation_between_lines(
      log_eye_center_dist,n_precision,delta_longitude,text_size,linewidth);

   long_start=sgn(long_start)*basic_math::mytruncate(
      fabs(long_start)/delta_longitude)*delta_longitude;
   double longitude_interval=(basic_math::mytruncate(
      (long_stop-long_start)/delta_longitude)+1)*delta_longitude;
   long_stop=long_start+longitude_interval;
   n_long=static_cast<int>((long_stop-long_start)/delta_longitude+1);
   
// Cap number of longitude lines at some reasonable upper limit:

   const int max_nlines=75;
   if (n_long > max_nlines)
   {
      n_long=max_nlines;
      long_stop=long_start+n_long*delta_longitude;
   }

   crop_latlong_bounds();

// Adjust starting and stopping values as well as number of latitude
// segments which comprise each longitude grid line:

   compute_polyline_vertex_angular_separation(
      lat_start,lat_stop,-89.9,89.9,n_lat,d_lat);

   if (longitude_ptr != NULL)
   {
      while (long_start+(n_long-1)*delta_longitude > 
             longitude_ptr->get_stop())
      {
         n_long--;
      }
   }

//   cout << "long_start = " << long_start 
//        << " long_stop = " << long_stop << endl;
//   cout << "lat_start = " << lat_start 
//        << " lat_stop = " << lat_stop << endl;
//   cout << "delta_long = " << delta_longitude << " delta_lat = " << d_lat
//        << " n_long = " << n_long << " n_lat = " << n_lat 
//        << endl;
}

// ---------------------------------------------------------------------
void LatLongGrid::draw_latitude_lines(
   double log_eye_center_dist,double eye_alt,
   int curr_iter,double annotation_altitude,bool refresh_flag)
{
//   cout << "inside LatLongGrid::draw_latitude_lines()" << endl;

   int n_long,n_lat,n_precision;
   double d_long,text_size,linewidth;
   set_latitude_line_spacing(
      log_eye_center_dist,n_lat,n_long,d_long,
      n_precision,text_size,linewidth);

//   cout << "lat_start = " << lat_start 
//        << " delta_lat = " << get_delta_latitude() << endl;
//   cout << "n_long = " << n_long << " n_lat = " << n_lat
//        << " d_long = " << d_long << endl;

// Compute ECI vertices for polylines representing longitude grid
// lines:

   vector<threevector> V_XYZ;
   for (int i=0; i<n_lat; i++)
   {
      double latitude=lat_start+i*get_delta_latitude();
//      cout << "i = " << i << " latitude = " << latitude << endl;

      LINES_MAP::iterator PolyLine_iter=get_latitude_lines_map_ptr()->find(
         threevector(latitude,long_start,long_stop));

      PolyLine* curr_PolyLine_ptr=NULL;
      if (PolyLine_iter != get_latitude_lines_map_ptr()->end())
      {
         PolyLine* retrieved_PolyLine_ptr=PolyLine_iter->second.first;
         curr_PolyLine_ptr=retrieved_PolyLine_ptr;
         PolyLine_iter->second.second=curr_iter;
      }

      if (curr_PolyLine_ptr==NULL || refresh_flag)
      {
         V_XYZ.clear();
         for (int j=0; j<=n_long; j++)
         {
            V_XYZ.push_back(Zero_vector);
         }

         double prev_long=long_start;
         double prev_alt=0;
         double next_alt=0;
         for (int j=0; j<n_long; j++)
         {
            double next_long=prev_long+d_long;
            adjust_endpoint_altitudes(
               prev_long,latitude,prev_alt,
               next_long,latitude,next_alt,
               eye_alt,V_XYZ[j],V_XYZ[j+1]);

            prev_long=next_long;
            prev_alt=next_alt;
         } // loop over index j labeling longitude

         if (V_XYZ.size() >= 2)
         {
            bool single_polyline_per_geode_flag=true;
            int n_text_messages=1;
            threevector reference_vertex=
               get_LatLinesGroup_ptr()->compute_vertices_average(V_XYZ);

            curr_PolyLine_ptr=get_LatLinesGroup_ptr()->generate_new_PolyLine(
               reference_vertex,V_XYZ,get_dynamic_grid_color(),
               depth_buffering_off_flag,
               single_polyline_per_geode_flag,n_text_messages);

            curr_PolyLine_ptr->set_stationary_Graphical_flag(true);

            threevector curr_lll(latitude,long_start,long_stop);
            (*get_latitude_lines_map_ptr())[curr_lll]=pair<PolyLine*,int>(
               curr_PolyLine_ptr,curr_iter);

            curr_PolyLine_ptr->set_linewidth(linewidth);

            string text_label;

            if (latitude < 0)
            {
               if (basic_math::is_int(fabs(latitude)))
               {
                  text_label="S"+stringfunc::number_to_string(
                     fabs(latitude));
               }
               else
               {
                  text_label="S"+stringfunc::number_to_string(
                     fabs(latitude),mathfunc::ndigits_after_decimal_point(
                        fabs(latitude)));
               }
            }
            else
            {
               if (basic_math::is_int(latitude))
               {
                  text_label="N"+stringfunc::number_to_string(latitude);
               }
               else
               {
                  text_label="N"+stringfunc::number_to_string(
                     fabs(latitude),mathfunc::ndigits_after_decimal_point(
                        latitude));
               }
            }
            if (text_label=="N0" || text_label=="S0")
            {
               text_label="Equator";
               curr_PolyLine_ptr->set_linewidth(2*linewidth);
               curr_PolyLine_ptr->set_color(osg::Vec4(0.6, 0.6, 0.6, 1.0));
            }
            curr_PolyLine_ptr->set_text_label(0,text_label);
            curr_PolyLine_ptr->set_text_color(0,get_dynamic_grid_color());

         } // V_XYZ.size() >= 2) conditional
      } // curr_PolyLine_ptr != NULL conditional

      if (curr_PolyLine_ptr != NULL)
      {
         threevector text_posn;
         if (flat_grid_flag)
         {

// Restrict flat grid text labels to appear inside the LatLongGrid:

            double text_longitude=LOS_intersection_geopoint.get_longitude();
            text_longitude=basic_math::max(
               text_longitude,longitude_ptr->get_start());
            text_longitude=basic_math::min(
               text_longitude,longitude_ptr->get_stop());

            text_posn=geopoint(
               text_longitude,latitude,
               1.1*annotation_altitude,UTM_zonenumber).get_UTM_posn();
         }
         else
         {
            if (curr_PolyLine_ptr->get_text_refptr_valid(0))
            {
               text_posn=Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(
                  LOS_intersection_geopoint.get_longitude(),latitude,
                  1.1*annotation_altitude);
               Ellipsoid_model_ptr->align_text_with_cardinal_dirs(
                  LOS_intersection_geopoint.get_longitude(),latitude,
                  curr_PolyLine_ptr->get_text_ptr(0));
            } // text_refptr_valid(0) conditional
         }
         curr_PolyLine_ptr->set_text_posn(0,text_posn);
         update_earthline_text_size(eye_alt,curr_PolyLine_ptr,text_size);
      } // curr_PolyLine_ptr != NULL conditional
   } // loop over index i labeling longitude
}

// ---------------------------------------------------------------------
// Member function set_latitude_line_spacing()

void LatLongGrid::set_latitude_line_spacing(
   double log_eye_center_dist,
   int& n_lat,int& n_long,double& d_long,
   int& n_precision,double& text_size,double& linewidth)
{
//   cout << "inside LatLongGrid::set_latitude_line_spacing()" << endl;
   
   initialize_extremal_longitudes_and_latitudes();

// Reset angular spacing between lines of longitude based upon
// camera's altitude above earth's surface:

   angular_separation_between_lines(
      log_eye_center_dist,n_precision,delta_latitude,text_size,linewidth);

   lat_start=sgn(lat_start)*basic_math::mytruncate(
      fabs(lat_start)/delta_latitude)*delta_latitude;
   double latitude_interval=(basic_math::mytruncate(
      (lat_stop-lat_start)/delta_latitude)+1)*delta_latitude;
   lat_stop=lat_start+latitude_interval;
   lat_start=basic_math::max(-89.9,lat_start);
   lat_stop=basic_math::min(89.9,lat_stop);
   n_lat=static_cast<int>((lat_stop-lat_start)/delta_latitude+1);

// Cap number of latitude lines at some reasonable upper limit:

//   cout << "n_lat = " << n_lat << endl;
   const int max_nlines=40;
   if (n_lat > max_nlines)
   {
      n_lat=max_nlines;
      lat_stop=lat_start+n_lat*delta_latitude;

//      cout << "lat_start = " << lat_start << " lat_stop = "
//           << lat_stop << endl;
//      cout << "delta_lat = " << delta_latitude 
//           << " n_lat = " << n_lat << endl;
   }

   crop_latlong_bounds();

// Adjust starting and stopping values as well as number of latitude
// segments which comprise each longitude grid line:

   compute_polyline_vertex_angular_separation(
      long_start,long_stop,long_min,long_min+360,n_long,d_long);

   if (latitude_ptr != NULL)
   {
      while (lat_start+(n_lat-1)*delta_latitude > latitude_ptr->get_stop())
      {
         n_lat--;
      }
   }
}

// ---------------------------------------------------------------------
// Member function adjust_endpoint_altitudes() takes in (long,lat,alt)
// coords for two endpoints of some line of longitude/latitude
// segment.  It computes the midpoint of that segment in Cartesian XYZ
// geocentric coordinates, and then calculates the midpoint's
// altitude.  If the midpoint altitude is negative, the endpoints'
// altitudes are increased by a sufficient amount so that the new
// midpoint would lie above the ellipsoid's surface.  The adjusted
// endpoint locations are returned in output threevectors V1 and V2.

void LatLongGrid::adjust_endpoint_altitudes(
   double long1,double lat1,double& alt1,
   double long2,double lat2,double& alt2,
   double eye_alt,threevector& V1,threevector& V2)
{
//   cout << "inside LatLongGrid::adjust_endpoint_altitudes()" << endl;
//   cout << "long1 = " << long1 << " lat1 = " << lat1 << " alt1 = " << alt1
//        << endl;
//   cout << "long2 = " << long2 << " lat2 = " << lat2 << " alt2 = " << alt2
//        << endl;
//   cout << "flat_grid_flag = " << flat_grid_flag << endl;

   if (flat_grid_flag)
   {
      geopoint endpoint1(long1,lat1,get_world_origin().get(2),UTM_zonenumber);
      geopoint endpoint2(long2,lat2,get_world_origin().get(2),UTM_zonenumber);
      V1=endpoint1.get_UTM_posn();
      V2=endpoint2.get_UTM_posn();
   }
   else
   {
      V1=Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(long1,lat1,alt1);
      V2=Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(long2,lat2,alt2);

      threevector midpt=0.5*(V1+V2);
      double long_mid,lat_mid,alt_mid;
      Ellipsoid_model_ptr->ConvertXYZToLongLatAlt(
         midpt,long_mid,lat_mid,alt_mid);

      if (alt_mid < 0)
      {
         double line_alt=basic_math::min(0.001*eye_alt,1000.0);
         line_alt=basic_math::max(line_alt,10.0);

         alt1=basic_math::max(alt1,-alt_mid+line_alt);
         alt2=basic_math::max(alt2,-alt_mid+line_alt);

         V1=Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(long1,lat1,alt1);
         V2=Ellipsoid_model_ptr->ConvertLongLatAltToXYZ(long2,lat2,alt2);
      }

   } // flat_grid_flag conditional

//   cout << "V1 = " << V1 << " V2 = " << V2 << endl;
//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function get_dynamic_grid_color() returns purple [dark grey] for a
// local flat [global] grid.

osg::Vec4 LatLongGrid::get_dynamic_grid_color() const
{
//   cout << "inside LatLongGrid::get_dynmamic_grid_color()" << endl;
//   cout << "flat_grid_flag = " << flat_grid_flag << endl;
   if (flat_grid_flag)
   {
//      cout << "get_curr_color() = " << endl;
//      osgfunc::print_Vec4(get_curr_color());
      return get_curr_color();
   }
   else
   {
      osg::Vec4 dark_grey(0.4,0.4,0.4,1);
      return dark_grey;
   }
}

// ---------------------------------------------------------------------
// Member function update_earthline_text_size() adjusts the size of
// the text labels on dynamic lines of longitude & latitude according
// to the camera's altitude.

void LatLongGrid::update_earthline_text_size(
   double eye_alt,PolyLine* PolyLine_ptr,double text_size)
{
   PolyLine_ptr->set_text_size(0,text_size_prefactor*eye_alt*text_size);
}

// ---------------------------------------------------------------------
// Member function update_lines_map() iterates over input *map_ptr. It
// destroys any PolyLine whose associated update_counter time index
// does not match input index curr_iter.  It also removes the PolyLine
// entry from STL map member *map_ptr.

void LatLongGrid::update_lines_map(
   int curr_iter,PolyLinesGroup* PolyLinesGroup_ptr,LINES_MAP* map_ptr)
{
//   cout << "Inside LatLongGrid::update_lines_map()" << endl;
//   cout << "curr_iter = " << curr_iter << endl;
//   cout << "PolyLinesGroup_ptr = " << PolyLinesGroup_ptr
//        << " map_ptr = " << map_ptr << endl;

// First store all LINES_MAP iterators within a local STL vector:

   vector<LINES_MAP::iterator> map_iters;
   for (LINES_MAP::iterator map_iter=map_ptr->begin(); 
        map_iter != map_ptr->end(); map_iter++)
   {
      map_iters.push_back(map_iter);
   }

   for (unsigned int i=0; i<map_iters.size(); i++)
   {
      LINES_MAP::iterator map_iter=map_iters[i];
      int polyline_iter=map_iter->second.second;

      if (polyline_iter != curr_iter)
      {
         PolyLine* curr_PolyLine_ptr=map_iter->second.first;
         PolyLinesGroup_ptr->destroy_PolyLine(curr_PolyLine_ptr->get_ID());

// On 7/1/08, Ross Anderson taught us that we cannot increment
// map_iter within a for loop after the following erase line is
// called.  map_iter++ is no longer defined after the erase command is
// executed!  So we instead live with the ugly hack of looping over
// the iterators within the local STL vector map_iters which are
// unaffected by the following erase command:

         map_ptr->erase(map_iter);

/*
         if (map_ptr->size() != PolyLinesGroup_ptr->get_n_Graphicals())
         {
            cout << "***********************************************" << endl;
            cout << "inside LatLongGrid::update_lines_map()" << endl;
            cout << "PLG_ptr->get_n_Graphicals() = "
                 << PolyLinesGroup_ptr->get_n_Graphicals()
                 << " map_ptr->size() = " << map_ptr->size() << endl;
            cout << "***********************************************" << endl;
            outputfunc::enter_continue_char();
         }
*/
      } // polyline_pter = curr_iter conditiaonl
   } // iteration loop over *map_ptr

//   cout << "At end of LatLongGrid::update_lines_map()" << endl;
//   cout << "map_ptr->size() = " << map_ptr->size() << endl;
//   cout << "PolyLinesGroup_ptr->get_n_Graphicals() = "
//        << PolyLinesGroup_ptr->get_n_Graphicals() << endl;
}

// ---------------------------------------------------------------------
void LatLongGrid::redraw_long_lat_lines(double log_eye_alt,Clock* Clock_ptr)
{
//   cout << "inside LatLongGrid::redraw_long_lat_lines() #1" << endl;

   ViewFrustum* ViewFrustum_ptr=CM_3D_ptr->get_ViewFrustum_ptr();
   ViewFrustum_ptr->compute_params_planes_and_vertices();

   threevector camera_ECI_posn=CM_3D_ptr->get_eye_world_posn();
   threevector camera_Zhat=CM_3D_ptr->get_camera_Zhat();
   double eye_alt=Ellipsoid_model_ptr->eye_altitude(camera_ECI_posn);

   double camera_LOS_intercept_dist;
   if (!compute_LOS_intersection_geopoint(
      camera_ECI_posn,camera_Zhat,
      Clock_ptr,camera_LOS_intercept_dist)) return;

   double annotation_altitude=0.01*pow(10.0,log_eye_alt);
   const double max_annotation_altitude=10000;	// meters
   annotation_altitude=basic_math::min(
      annotation_altitude,max_annotation_altitude,0.2*eye_alt);

//   cout << "log_eye_alt = " << log_eye_alt << endl;
//   cout << "annotation_altitude = " << annotation_altitude << endl;

   compute_FOV_intersection_geopoints(Clock_ptr);

   bool refresh_flag=false;
   draw_longitude_lines(
      CM_3D_ptr->get_log_eye_center_dist(),
      eye_alt,curr_update_counter,annotation_altitude,refresh_flag);
   update_lines_map(
      curr_update_counter,get_LongLinesGroup_ptr(),longitude_lines_map_ptr);

   draw_latitude_lines(
      CM_3D_ptr->get_log_eye_center_dist(),
      eye_alt,curr_update_counter,annotation_altitude,refresh_flag);
   update_lines_map(
      curr_update_counter,get_LatLinesGroup_ptr(),latitude_lines_map_ptr);

   curr_update_counter++;
}

// ---------------------------------------------------------------------
void LatLongGrid::redraw_long_lat_lines(bool refresh_flag)
{
//   cout << "inside LatLongGrid::redraw_long_lat_lines() #2" << endl;
//   cout << "refresh_flag = " << refresh_flag << endl;
//   cout << "curr_update_counter = " << curr_update_counter << endl;

   threevector camera_posn=CM_3D_ptr->get_eye_world_posn();
   threevector camera_Zhat=CM_3D_ptr->get_camera_Zhat();

   double eye_alt=camera_posn.get(2);
   double log_eye_alt=log10(eye_alt);
//   cout << "log_eye_alt = " << log_eye_alt << endl;

   LOS_intersection_geopoint=ray_geopoint_intercept_on_grid(
      -camera_Zhat);
   threevector LOS_screen_intercept=LOS_intersection_geopoint.get_UTM_posn();
//   cout << "LOS_intersection_geopoint = " << LOS_intersection_geopoint
//        << endl;

//   compute_north_direction(eye_alt);

   double log_eye_center_dist=
      CM_3D_ptr->compute_camera_to_screen_center_distance(
         LOS_screen_intercept);

   double annotation_altitude=0.01*pow(10.0,log_eye_alt);
   const double max_annotation_altitude=10000;	// meters
   annotation_altitude=basic_math::min(
      annotation_altitude,max_annotation_altitude,0.2*eye_alt);

   annotation_altitude=basic_math::max(annotation_altitude,get_z_plane());

   compute_FOV_intersection_geopoints();

   draw_longitude_lines(
      log_eye_center_dist,eye_alt,curr_update_counter,annotation_altitude,
      refresh_flag);
   update_lines_map(
      curr_update_counter,get_LongLinesGroup_ptr(),longitude_lines_map_ptr);

   draw_latitude_lines(
      log_eye_center_dist,eye_alt,curr_update_counter,annotation_altitude,
      refresh_flag);
   update_lines_map(
      curr_update_counter,get_LatLinesGroup_ptr(),latitude_lines_map_ptr);

   curr_update_counter++;

//   cout << "LongLinesGroup_ptr->get_n_Graphicals() = "
//        << LongLinesGroup_ptr->get_n_Graphicals() << endl;
//   cout << "LatLinesGroup_ptr->get_n_Graphicals() = "
//        << LatLinesGroup_ptr->get_n_Graphicals() << endl;
}

// ---------------------------------------------------------------------
// Member function destroy_dymamic_grid_lines() is a high-level method
// which purges all longitude and latitude PolyLines and STL map
// entries.

void LatLongGrid::destroy_dynamic_grid_lines()
{
//   cout << "inside LatLongGrid::destroy_dynamic_grid_lines()" << endl;

   destroy_dynamic_grid_lines(
      get_LongLinesGroup_ptr(),longitude_lines_map_ptr);
   destroy_dynamic_grid_lines(get_LatLinesGroup_ptr(),latitude_lines_map_ptr);
}

// ---------------------------------------------------------------------
// Member function destroy_dynamic_grid_lines() iterates over input
// *map_ptr. It destroys every PolyLine within input
// *PolyLinesGroup_ptr.  This method also removes all PolyLine entries
// from STL map member *map_ptr.

void LatLongGrid::destroy_dynamic_grid_lines(
   PolyLinesGroup* PolyLinesGroup_ptr,LINES_MAP* map_ptr)
{
//   cout << "Inside LatLongGrid::destroy_dynamic_grid_Lines()" << endl;
//   cout << "PolyLinesGroup_ptr = " << PolyLinesGroup_ptr
//        << " map_ptr = " << map_ptr << endl;

// First store all LINES_MAP iterators within a local STL vector:

   vector<LINES_MAP::iterator> map_iters;
   for (LINES_MAP::iterator map_iter=map_ptr->begin(); 
        map_iter != map_ptr->end(); map_iter++)
   {
      map_iters.push_back(map_iter);
   }

   for (unsigned int i=0; i<map_iters.size(); i++)
   {
      LINES_MAP::iterator map_iter=map_iters[i];

      PolyLine* curr_PolyLine_ptr=map_iter->second.first;
      PolyLinesGroup_ptr->destroy_PolyLine(curr_PolyLine_ptr->get_ID());

// On 7/1/08, Ross Anderson taught us that we cannot increment
// map_iter within a for loop after the following erase line is
// called.  map_iter++ is no longer defined after the erase command is
// executed!  So we instead live with the ugly hack of looping over
// the iterators within the local STL vector map_iters which are
// unaffected by the following erase command:

      map_ptr->erase(map_iter);

      if (map_ptr->size() != PolyLinesGroup_ptr->get_n_Graphicals())
      {
         cout << "***********************************************" << endl;
         cout << "inside LatLongGrid::destroy_dynamic_grid_Lines()" << endl;
         cout << "PLG_ptr->get_n_Graphicals() = "
              << PolyLinesGroup_ptr->get_n_Graphicals()
              << " map_ptr->size() = " << map_ptr->size() << endl;
         cout << "***********************************************" << endl;
         outputfunc::enter_continue_char();
      }
   } // iteration loop over *map_ptr

//   cout << "At end of LatLongGrid::destroy_dynamic_grid_lines()" << endl;
//   cout << "map_ptr->size() = " << map_ptr->size() << endl;
//   cout << "PolyLinesGroup_ptr->get_n_Graphicals() = "
//        << PolyLinesGroup_ptr->get_n_Graphicals() << endl;
}

// ---------------------------------------------------------------------
// Member function toggle_dymamic_LongLatLines()

void LatLongGrid::toggle_dynamic_LongLatLines()
{
//   cout << "inside LatLongGrid::toggle_dynamic_LongLatLines()" << endl;
   get_LongLinesGroup_ptr()->toggle_OSGgroup_nodemask();
   get_LatLinesGroup_ptr()->toggle_OSGgroup_nodemask();
}

void LatLongGrid::turn_off_dynamic_LongLatLines()
{
//   cout << "inside LatLongGrid::turn_off_dynamic_LongLatLines()" << endl;
   get_LongLinesGroup_ptr()->set_OSGgroup_nodemask(0);
   get_LatLinesGroup_ptr()->set_OSGgroup_nodemask(0);
}

void LatLongGrid::turn_on_dynamic_LongLatLines()
{
//   cout << "inside LatLongGrid::turn_on_dynamic_LongLatLines()" << endl;
   get_LongLinesGroup_ptr()->set_OSGgroup_nodemask(1);
   get_LatLinesGroup_ptr()->set_OSGgroup_nodemask(1);
}

// ---------------------------------------------------------------------
// Member function ray_geopoint_intercept_on_grid() traces a ray from
// the virtual camera's current position down onto the LatLongGrid.
// It returns the geopoint corresponding to the ray's intersection
// point with the LatLongGrid.

geopoint LatLongGrid::ray_geopoint_intercept_on_grid(
   const threevector& r_hat)
{
//   cout << "inside LatLongGrid::ray_geopoint_intercept_on_grid()" << endl;

   threevector camera_posn=CM_3D_ptr->get_eye_world_posn();
   
   double theta=acos(r_hat.get(2));
   double camera_to_intercept_dist=
      (camera_posn.get(2)-get_world_origin().get(2))/cos(theta);

   threevector intercept_point=camera_posn-
      camera_to_intercept_dist*r_hat;
   geopoint intercept_geopoint(
      northern_hemisphere_flag,UTM_zonenumber,
      intercept_point.get(0),intercept_point.get(1),
      intercept_point.get(2));

//   cout << "camera_posn = " << camera_posn << endl;
//   cout << "grid origin = " << get_world_origin() << endl;
//   cout << "r_hat = " << r_hat << endl;

//   cout << "theta = " << theta*180/PI << endl;
//   cout << "camera_to_intercept_dist = " << camera_to_intercept_dist << endl;
//   cout << "intercept_point = " << intercept_point << endl;
//   cout << "intercept_geopoint = " << intercept_geopoint << endl;

   return intercept_geopoint;
}

// ---------------------------------------------------------------------
// Member function compute_north_direction() fills member threevector
// north_hat with the north direction vector computed at the LOS's
// intersection geopoint in UTM coordinates.

void LatLongGrid::compute_north_direction(double eye_alt)
{
//   cout << "inside LatLongGrid::compute_north_direction()" << endl;

   double long_start=LOS_intersection_geopoint.get_longitude();
   double long_stop=long_start;
   double lat_start=LOS_intersection_geopoint.get_latitude();
   double lat_stop=lat_start+0.001;

   double alt_start=0;
   double alt_stop=0;
   threevector Vstart,Vstop;
   adjust_endpoint_altitudes(
      long_start,lat_start,alt_start,long_stop,lat_stop,alt_stop,
      eye_alt,Vstart,Vstop);

   north_hat=(Vstop-Vstart).unitvector();
//   cout << "north_hat = " << north_hat << endl;
}
