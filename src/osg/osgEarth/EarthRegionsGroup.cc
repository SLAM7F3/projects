// ==========================================================================
// EARTHREGIONSGROUP class member function definitions
// ==========================================================================
// Last modified on 5/10/10; 12/4/10; 9/14/11; 4/5/14; 1/22/16
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osg/BoundingBox>
#include <osg/MatrixTransform>
#include "astro_geo/Clock.h"
#include "color/colorfuncs.h"
#include "osg/osgEarth/Earth.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "track/graphquery.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "track/movers_group.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgfuncs.h"
#include "general/outputfuncs.h"
#include "passes/PassInfo.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgRegions/RegionPolyLinesGroup.h"
#include "general/stringfuncs.h"
#include "video/texture_rectangle.h"
#include "osg/osgEarth/TextureSectorsGroup.h"
#include "track/tracks_group.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void EarthRegionsGroup::allocate_member_objects()
{
   TextureSectorsGroup_ptr=new TextureSectorsGroup(
      get_pass_ptr(),MoviesGroup_ptr);
   if (MoviesGroup_ptr != NULL)
   {
      TextureSectorsGroup_ptr->set_AnimationController_ptr(
         MoviesGroup_ptr->get_AnimationController_ptr());
   }
}		       

void EarthRegionsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="EarthRegionsGroup";

   northern_hemisphere_flag=true;
   propagate_all_tracks_flag=true;
   check_Cylinder_ROI_intersections_flag=false;
   specified_UTM_zonenumber=-1;
   ROI_color=colorfunc::white;
   KOZ_color=colorfunc::blue;
   curr_region_ID=0;
   movie_alpha=0;

   clouds_group_ptr=NULL;
   latlonggrids_group_ptr=NULL;
   Earth_ptr=NULL;
   MoviesGroup_ptr=NULL;
   CylindersGroup_ptr=NULL;
   PolyhedraGroup_ptr=NULL;
   urban_network_Messenger_ptr=NULL;
   robots_Messenger_ptr=NULL;
   GoogleEarth_Messenger_ptr=NULL;
   aircraft_Messenger_ptr=NULL;
   blueforce_car_Messenger_ptr=NULL;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<EarthRegionsGroup>(
         this, &EarthRegionsGroup::update_display));
}		       

void EarthRegionsGroup::assign_EarthRegionsGroup_Messenger_ptrs()
{
   for (unsigned int i=0; i<get_n_Messenger_ptrs(); i++)
   {
      Messenger* curr_Messenger_ptr=get_Messenger_ptr(i);
      if (curr_Messenger_ptr->get_topicName()=="urban_network")
      {
         urban_network_Messenger_ptr=curr_Messenger_ptr;
      }
      else if (curr_Messenger_ptr->get_topicName()=="robots")
      {
         robots_Messenger_ptr=curr_Messenger_ptr;
      }

// As of Oct 2009, Michael Yee's messenger for the LOST thin client
// listens on the "viewer_update" channel.  We recycle summer 2008
// touch table code where we sent UAV track info to Tim Schreiner's
// GoogleEarth thick client for the LOST project.  So we now set the
// GoogleEarth Messenger to equal the current Messenger if the queue
// name==viewer_update or GoogleEarth:

      else if (curr_Messenger_ptr->get_topicName()=="GoogleEarth" ||
               curr_Messenger_ptr->get_topicName()=="viewer_update")
      {
         GoogleEarth_Messenger_ptr=curr_Messenger_ptr;
      }
      else if (curr_Messenger_ptr->get_topicName()=="aircraft")
      {
         aircraft_Messenger_ptr=curr_Messenger_ptr;
      }
      else if (curr_Messenger_ptr->get_topicName()=="blueforce_car")
      {
         blueforce_car_Messenger_ptr=curr_Messenger_ptr;
      }
   }
//   cout << "At end of EarthRegionsGroup::assign_messenger_ptrs()" << endl;
//   cout << "urban_network_Messenger_ptr = "<< urban_network_Messenger_ptr 
//        << endl;
//   cout << "robots_Messenger_ptr = " << robots_Messenger_ptr << endl;
//   cout << "GoogleEarth_Messenger_ptr = " << GoogleEarth_Messenger_ptr
//        << endl;
}

EarthRegionsGroup::EarthRegionsGroup(Pass* PI_ptr,Earth* E_ptr):
   GeometricalsGroup(3,PI_ptr)
{	
//   cout << "inside EarthRegionsGroup constructor #1" << endl;
   initialize_member_objects();
   allocate_member_objects();
   Earth_ptr=E_ptr;
}		       

EarthRegionsGroup::EarthRegionsGroup(
   Pass* PI_ptr,LatLongGridsGroup* LLGG_ptr,Earth* E_ptr):
   GeometricalsGroup(3,PI_ptr)
{	
//   cout << "inside EarthRegionsGroup constructor #2" << endl;

   initialize_member_objects();
   allocate_member_objects();
   latlonggrids_group_ptr=LLGG_ptr;
   Earth_ptr=E_ptr;
}		       

EarthRegionsGroup::EarthRegionsGroup(
   Pass* PI_ptr,PointCloudsGroup* PCG_ptr,LatLongGridsGroup* LLGG_ptr,
   Earth* E_ptr):
   GeometricalsGroup(3,PI_ptr)
{	
//   cout << "inside EarthRegionsGroup constructor #3" << endl;
   initialize_member_objects();
   allocate_member_objects();
   clouds_group_ptr=PCG_ptr;
   latlonggrids_group_ptr=LLGG_ptr;
   Earth_ptr=E_ptr;
}		       

EarthRegionsGroup::EarthRegionsGroup(
   Pass* PI_ptr,LatLongGridsGroup* LLGG_ptr,MoviesGroup* MG_ptr,Earth* E_ptr):
   GeometricalsGroup(3,PI_ptr)
{	
//   cout << "inside EarthRegionsGroup constructor #4" << endl;

   initialize_member_objects();
   MoviesGroup_ptr=MG_ptr;
   allocate_member_objects();
   latlonggrids_group_ptr=LLGG_ptr;
   Earth_ptr=E_ptr;
}		       

EarthRegionsGroup::EarthRegionsGroup(
   Pass* PI_ptr,PointCloudsGroup* PCG_ptr,LatLongGridsGroup* LLGG_ptr,
   MoviesGroup* MG_ptr,Earth* E_ptr):
   GeometricalsGroup(3,PI_ptr)
{	
//   cout << "inside EarthRegionsGroup constructor #5" << endl;
   initialize_member_objects();
   MoviesGroup_ptr=MG_ptr;
   allocate_member_objects();
   clouds_group_ptr=PCG_ptr;
   latlonggrids_group_ptr=LLGG_ptr;
   Earth_ptr=E_ptr;
}		       

EarthRegionsGroup::~EarthRegionsGroup()
{
   delete TextureSectorsGroup_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const EarthRegionsGroup& RG)
{
   return(outstream);
}
// --------------------------------------------------------------------------
// Member function get_TextureSector_posn returns the UVW position
// coordinates for the TextureSector labeled by input integer ID.

threevector EarthRegionsGroup::get_TextureSector_posn(int ID)
{
//   cout << "inside EarthRegionsGroup::get_TextureSector_posn(int ID)
//        << endl;
   
   TextureSector* TextureSector_ptr=
      TextureSectorsGroup_ptr->get_ID_labeled_TextureSector_ptr(ID);
   threevector posn;
   TextureSector_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),posn);
   return posn;
}

// ==========================================================================
// EarthRegion creation member functions
// ==========================================================================

// Member function generate_regions loops over all passes within the
// input PassesGroup and generates either PointCloud or SurfaceTexture
// EarthRegions.  It also stores the northern hemisphere flag and UTM
// zonenumber specified within *get_pass_ptr() into this
// EarthRegionsGroup's corresponding member variables.

void EarthRegionsGroup::generate_regions(
   PassesGroup& passes_group,bool place_onto_bluemarble_flag,
   bool generate_pointcloud_LatLongGrid_flag,
   bool display_SurfaceTexture_LatLonGrid_flag)
{
//   cout << "inside EarthRegionsGroup::generate_regions()" << endl;
//   cout << "get_n_passes = " << passes_group.get_n_passes() << endl;
   
   for (unsigned int r=0; r<passes_group.get_n_passes(); r++)
   {
      Pass* pass_ptr=passes_group.get_pass_ptr(r);
//      cout << "pass_ptr = " << pass_ptr << endl;
      
      if (pass_ptr==NULL) continue;

//      cout << "r = " << r 
//           << " passtype = " << pass_ptr->get_passtype() << endl;

      if (pass_ptr->get_passtype()==Pass::cloud)
      {
         generate_PointCloud_EarthRegion(
            pass_ptr,place_onto_bluemarble_flag,
            generate_pointcloud_LatLongGrid_flag);
      } 
      else if (pass_ptr->get_passtype()==Pass::surface_texture)
      {
         generate_SurfaceTexture_EarthRegion(
            pass_ptr,display_SurfaceTexture_LatLonGrid_flag);
      } 
      else if (pass_ptr->get_passtype()==Pass::other)
      {
         generate_empty_EarthRegion(pass_ptr);
      } // passtype conditional
   } // loop over index r labeling passes

   if (get_pass_ptr()->get_passtype() != Pass::other &&
       get_specified_UTM_zonenumber()==-1)
   {
      set_northern_hemisphere_flag(
         get_pass_ptr()->get_northern_hemisphere_flag());
      set_specified_UTM_zonenumber(
         get_pass_ptr()->get_UTM_zonenumber());
   }

//   cout << "get_specified_UTM_zonenumber() = "
//        << get_specified_UTM_zonenumber() << endl;
//   cout << "northern_hemisphere_flag = "
//        << get_northern_hemisphere_flag() << endl;
}

// --------------------------------------------------------------------------
// Member function generate_empty_EarthRegion() generates an
// EarthRegion which has no associated 3D or 2D input imagery data.
// It also forms a LatLongGrid based upon bbox coords passed as input
// package file arguments.

void EarthRegionsGroup::generate_empty_EarthRegion(Pass* pass_ptr)
{
//   cout << "inside EarthRegionsGroup::generate_empty_EarthRegion()" << endl;
//   cout << "pass_ptr = " << pass_ptr << endl;
//   cout << "passinfo_ptr = " << pass_ptr->get_PassInfo_ptr() << endl;

   double z_grid=0;
   double altitude_offset=pass_ptr->get_PassInfo_ptr()->get_height_offset();
//   cout << "altitude_offset = " << altitude_offset << endl;

// If LatLongGrid is NOT being placed onto blue marble but rather
// underneath some point cloud and/or satellite texture image, allow
// user to adjust its height by passing in height_offset value within
// a package file:

   if (Earth_ptr==NULL)
   {
      z_grid += altitude_offset;
   }
//   cout << "z_grid = " << z_grid << endl;

// Search for longitude/latitude bounding box passed as input pass
// parameters.  If no such bbox was specified, use ladar point cloud's
// bounding box to establish LatLongGrid extents:

   double longitude_lo=pass_ptr->get_PassInfo_ptr()->get_longitude_lo();
   double longitude_hi=pass_ptr->get_PassInfo_ptr()->get_longitude_hi();
   double latitude_lo=pass_ptr->get_PassInfo_ptr()->get_latitude_lo();
   double latitude_hi=pass_ptr->get_PassInfo_ptr()->get_latitude_hi();

//   cout << "longitude_lo = " << longitude_lo 
//        << " longitude_hi = " << longitude_hi << endl;
//   cout << "latitude_lo = " << latitude_lo 
//        << " latitude_hi = " << latitude_hi << endl;
//   cout << "get_pass_ptr()->get_UTM_zonenumber() = "
//        << get_pass_ptr()->get_UTM_zonenumber() << endl;

// Compute UTM coordinates for lower left and upper right
// longitude/latitude corners in common zone fixed by lower left corner:

   geopoint lower_left_corner(longitude_lo,latitude_lo);
   lower_left_corner.recompute_UTM_coords();
//   cout << "lower_left_corner = " << lower_left_corner << endl;

   set_northern_hemisphere_flag(
      lower_left_corner.get_northern_hemisphere_flag());
   set_specified_UTM_zonenumber(
      lower_left_corner.get_UTM_zonenumber());

   geopoint upper_right_corner(longitude_hi,latitude_hi);
   upper_right_corner.recompute_UTM_coords(get_specified_UTM_zonenumber());
//   cout << "upper_right_corner = " << upper_right_corner << endl;

   double xmin=lower_left_corner.get_UTM_easting();
   double ymin=lower_left_corner.get_UTM_northing();
   double xmax=upper_right_corner.get_UTM_easting();
   double ymax=upper_right_corner.get_UTM_northing();

//   cout << "xmin = " << xmin << " xmax = " << xmax << endl;
//   cout << "ymin = " << ymin << " ymax = " << ymax << endl;
//   cout << "get_specified_UTM_zonenumber() = " 
//        << get_specified_UTM_zonenumber() << endl;
//   cout << "get_northern_hemisphere_flag() = " 
//        << get_northern_hemisphere_flag() << endl;
   
   LatLongGrid* LatLongGrid_ptr=latlonggrids_group_ptr->
      generate_new_Grid(
         get_specified_UTM_zonenumber(),
         get_northern_hemisphere_flag(),
         xmin,xmax,ymin,ymax,z_grid);

   EarthRegion* EarthRegion_ptr=generate_new_EarthRegion(
      LatLongGrid_ptr,Earth_ptr,NULL);

// Assign robots_Messenger_ptr to point cloud's EarthRegion:

   EarthRegion_ptr->set_robots_Messenger_ptr(robots_Messenger_ptr);
} 

// --------------------------------------------------------------------------
// Member function generate_PointCloud_EarthRegion() generates a
// PointCloud and associated LatLongGrid for each cloud entry within
// input passes_group.  It then assigns both the cloud and LatLongGrid
// to a dynamically generated EarthRegion which stores UTM zone
// information.

void EarthRegionsGroup::generate_PointCloud_EarthRegion(
   Pass* pass_ptr,bool place_onto_bluemarble_flag,
   bool generate_pointcloud_LatLongGrid_flag)
{
//   cout << "inside EarthRegionsGroup::generate_PointCloud_EarthRegion()" 
//        << endl;

   PointCloud* cloud_ptr=clouds_group_ptr->generate_new_Cloud(pass_ptr);

   if (cloud_ptr==NULL) return;
   double z_grid=cloud_ptr->get_z_ColorMap_ptr()->get_min_threshold(2);

   double altitude_offset=pass_ptr->get_PassInfo_ptr()->get_height_offset();
//   cout << "altitude_offset = " << altitude_offset << endl;

// If LatLongGrid is NOT being placed onto blue marble but rather
// underneath some point cloud and/or satellite texture image, allow
// user to adjust its height by passing in height_offset value within
// a package file:

   if (Earth_ptr==NULL)
   {
      z_grid += altitude_offset;
   }
//   cout << "z_grid = " << z_grid << endl;

// Search for longitude/latitude bounding box passed as input pass
// parameters.  If no such bbox was specified, use ladar point cloud's
// bounding box to establish LatLongGrid extents:

   double longitude_lo=pass_ptr->get_PassInfo_ptr()->get_longitude_lo();
   double longitude_hi=pass_ptr->get_PassInfo_ptr()->get_longitude_hi();
   double latitude_lo=pass_ptr->get_PassInfo_ptr()->get_latitude_lo();
   double latitude_hi=pass_ptr->get_PassInfo_ptr()->get_latitude_hi();

//   cout << "longitude_lo = " << longitude_lo 
//        << " longitude_hi = " << longitude_hi << endl;
//   cout << "latitude_lo = " << latitude_lo 
//        << " latitude_hi = " << latitude_hi << endl;
   
   double xmin,xmax,ymin,ymax;
   if (!nearly_equal(longitude_lo,0) || !nearly_equal(longitude_hi,0) ||
       !nearly_equal(latitude_lo,0) || !nearly_equal(latitude_hi,0))
   {

// Compute UTM coordinates for lower left and upper right
// longitude/latitude corners in common zone fixed by input ladar
// point cloud pass:

      geopoint lower_left_corner(longitude_lo,latitude_lo);
      lower_left_corner.recompute_UTM_coords(
         get_pass_ptr()->get_UTM_zonenumber());
//      cout << "lower_left_corner = " << lower_left_corner << endl;

      geopoint upper_right_corner(longitude_hi,latitude_hi);
      upper_right_corner.recompute_UTM_coords(
         get_pass_ptr()->get_UTM_zonenumber());
//      cout << "upper_right_corner = " << upper_right_corner << endl;

      xmin=lower_left_corner.get_UTM_easting();
      ymin=lower_left_corner.get_UTM_northing();
      xmax=upper_right_corner.get_UTM_easting();
      ymax=upper_right_corner.get_UTM_northing();
   }
   else
   {
      osg::BoundingBox bbox=cloud_ptr->get_xyz_bbox();
//   cout << "bbox.xmin() = " << bbox.xMin() 
//        << " bbox.xmax() = " << bbox.xMax() << endl;
//   cout << "bbox.ymin() = " << bbox.yMin() 
//        << " bbox.ymax() = " << bbox.yMax() << endl;

      xmin=bbox.xMin();
      xmax=bbox.xMax();
      ymin=bbox.yMin();
      ymax=bbox.yMax();
   }

//   cout << "xmin = " << xmin << " xmax = " << xmax << endl;
//   cout << "ymin = " << ymin << " ymax = " << ymax << endl;

   LatLongGrid* LatLongGrid_ptr=NULL;
   if (generate_pointcloud_LatLongGrid_flag)
   {
      LatLongGrid_ptr=latlonggrids_group_ptr->generate_new_Grid(
         pass_ptr->get_UTM_zonenumber(),
         pass_ptr->get_northern_hemisphere_flag(),
         xmin,xmax,ymin,ymax,z_grid);
   }
   
   EarthRegion* EarthRegion_ptr=generate_new_EarthRegion(
      LatLongGrid_ptr,Earth_ptr,cloud_ptr);

   if (Earth_ptr != NULL && place_onto_bluemarble_flag)
   {
      EarthRegion_ptr->place_cloud_onto_ellipsoid_surface(
         altitude_offset);
      EarthRegion_ptr->place_latlonggrid_onto_ellipsoid_surface(
         0.1*altitude_offset);
   }

// Assign robots_Messenger_ptr to point cloud's EarthRegion:

   EarthRegion_ptr->set_robots_Messenger_ptr(robots_Messenger_ptr);
} 

// --------------------------------------------------------------------------
// Member function generate_SurfaceTexture_EarthRegion() is used by
// program EARTH (to show both the HAFB video playing over Baghdad as
// well as the San Clemente texture) and by the CH video within the
// Bluegrass demo.  So until we extricate these programs' dependence
// upon this awfully complicated method, we have to keep it around...

void EarthRegionsGroup::generate_SurfaceTexture_EarthRegion(
   Pass* pass_ptr,bool display_LatLongGrid_flag)
{
   cout << "inside original EarthRegionsGroup::generate_SurfaceTexture_EarthRegion()"
        << endl;

   string texture_filename=pass_ptr->get_first_filename();
   cout << "texture_filename = pass_ptr->get_first_filename() = "
        << pass_ptr->get_first_filename() << endl;

   TextureSector* TextureSector_ptr=
      TextureSectorsGroup_ptr->generate_new_TextureSector(Earth_ptr);
//   cout << "TextureSector_ptr = " << TextureSector_ptr << endl;

   double altitude=pass_ptr->get_PassInfo_ptr()->get_altitude();
   TextureSector_ptr->set_center_altitude(altitude);
//   cout << "altitude = " << altitude << endl;

   double lon_lo=pass_ptr->get_PassInfo_ptr()->get_longitude_lo();
   double lon_hi=pass_ptr->get_PassInfo_ptr()->get_longitude_hi();
   double lat_lo=pass_ptr->get_PassInfo_ptr()->get_latitude_lo();
   double lat_hi=pass_ptr->get_PassInfo_ptr()->get_latitude_hi();

   if (nearly_equal(lon_lo,0) && nearly_equal(lon_hi,0) &&
   nearly_equal(lat_lo,0) && nearly_equal(lat_hi,0))
   {
      TextureSector_ptr->construct_easting_northing_bbox(
         pass_ptr->get_PassInfo_ptr()->get_northern_hemisphere_flag(),
         pass_ptr->get_PassInfo_ptr()->get_UTM_zonenumber(),
         pass_ptr->get_PassInfo_ptr()->get_easting_lo(),
         pass_ptr->get_PassInfo_ptr()->get_easting_hi(),
         pass_ptr->get_PassInfo_ptr()->get_northing_lo(),
         pass_ptr->get_PassInfo_ptr()->get_northing_hi(),
         altitude);
   }
   else
   {
      TextureSector_ptr->construct_long_lat_bbox(
         lon_lo,lon_hi,lat_lo,lat_hi,altitude);
   }
//   cout << "*TextureSector_ptr = " << *TextureSector_ptr << endl;

   pass_ptr->set_UTM_zonenumber(
      TextureSector_ptr->get_lower_left_corner_ptr()->
      get_UTM_zonenumber());
   pass_ptr->set_northern_hemisphere_flag(
      TextureSector_ptr->get_lower_left_corner_ptr()->
      get_northern_hemisphere_flag());
   TextureSectorsGroup_ptr->generate_TextureSector_geode(
      TextureSector_ptr,texture_filename);

   double delta_long=TextureSector_ptr->get_longitude_hi()-
      TextureSector_ptr->get_longitude_lo();
   double delta_lat=TextureSector_ptr->get_latitude_hi()-
      TextureSector_ptr->get_latitude_lo();
   double mean_delta=sqrt(delta_long*delta_lat);
//   cout << "delta_long = " << delta_long 
//        << " delta_lat = " << delta_lat 
//        << " mean_delta = " << mean_delta << endl;

   double grid_altitude=TextureSector_ptr->get_center_altitude();
//   cout << "grid_altitude = " << grid_altitude << endl;

// Offset LatLongGrid in altitude from TextureSector center's altitude
// by an amount which depends upon the RMS angular size of the
// LatLongGrid:

   const double max_delta_grid_altitude=10;	// meters
   const double min_delta_grid_altitude=1;	// meters
   double delta_grid_altitude=
      min_delta_grid_altitude+
      (max_delta_grid_altitude-min_delta_grid_altitude)/(0.3-0.0015)*(
         mean_delta-0.0015);
   delta_grid_altitude=basic_math::min(
      max_delta_grid_altitude,delta_grid_altitude);
   delta_grid_altitude=basic_math::max(
      min_delta_grid_altitude,delta_grid_altitude);
//   cout << "delta_grid_altitude = " << delta_grid_altitude << endl;
   grid_altitude += delta_grid_altitude;

//   cout << "long_lo = " << TextureSector_ptr->get_longitude_lo()
//        << " long_hi = " << TextureSector_ptr->get_longitude_hi() << endl;
//   cout << "lat_lo = " << TextureSector_ptr->get_latitude_lo()
//        << " lat_hi = " << TextureSector_ptr->get_latitude_hi() << endl;

   LatLongGrid* LatLongGrid_ptr=latlonggrids_group_ptr->
      generate_new_Grid(
         TextureSector_ptr->get_longitude_lo(),
         TextureSector_ptr->get_longitude_hi(),
         TextureSector_ptr->get_latitude_lo(),
         TextureSector_ptr->get_latitude_hi(),grid_altitude);

//   cout << "display_LatLongGrid_flag = "
//        << display_LatLongGrid_flag << endl;
   if (!display_LatLongGrid_flag)
   {
      latlonggrids_group_ptr->mask_Graphical_for_all_times(
         LatLongGrid_ptr);
   }

//   cout << "*latlon_grid_origin_ptr = " 
//        << *(LatLongGrid_ptr->get_world_origin_ptr()) << endl;
//   cout << "*LatLongGrid_ptr = " << *LatLongGrid_ptr << endl;
//   cout << "LatLongGrid_ptr = " << LatLongGrid_ptr << endl;
//   cout << "latlonggrids_group_ptr->get_n_Graphicals() = "
//        << latlonggrids_group_ptr->get_n_Graphicals() << endl;

   EarthRegion* EarthRegion_ptr=generate_new_EarthRegion(
      LatLongGrid_ptr,Earth_ptr);
   EarthRegion_ptr->set_TextureSector_ptr(TextureSector_ptr);
   EarthRegion_ptr->get_PAT_ptr()->addChild(
      TextureSector_ptr->get_PAT_ptr());

// If SurfaceTexture corresponds to a movie, instantiate a
// TextureSector video chip to manage the movie:

   if (TextureSector_ptr->get_texture_rectangle_ptr() != NULL)
   {
      generate_EarthRegion_video_chip(EarthRegion_ptr->get_ID(),
                                      movie_alpha);
   }

// Display latlonggrid underneath a surface texture only for algorithm
// development purposes:

//   EarthRegion_ptr->place_latlonggrid_onto_ellipsoid_surface();
} 

/*
// --------------------------------------------------------------------------
// As of Feb 2010, this next, simpler version of
// generate_SurfaceTexture_EarthRegion() is used by program GSTREETS
// in order to display G76 indoor ladar plus "wagon-wheel" panoramas
// against aerial photo of LL.  We will eventually want all main
// programs to use this version rather than its awful preceding
// counterpart.

// Member function generate_SurfaceTexture_EarthRegion()

void EarthRegionsGroup::generate_SurfaceTexture_EarthRegion(
   Pass* pass_ptr,bool display_LatLongGrid_flag)
{
   cout << "inside EarthRegionsGroup::generate_SurfaceTexture_EarthRegion()"
        << endl;
//   cout << "texture_filename = pass_ptr->get_first_filename() = "
//        << pass_ptr->get_first_filename() << endl;

   MoviesGroup* MoviesGroup_ptr=TextureSectorsGroup_ptr->
      get_MoviesGroup_ptr();
   if (MoviesGroup_ptr==NULL) return;

   TextureSector* TextureSector_ptr=
      TextureSectorsGroup_ptr->generate_new_TextureSector(Earth_ptr);
//   cout << "TextureSector_ptr = " << TextureSector_ptr << endl;

   double altitude=pass_ptr->get_PassInfo_ptr()->get_altitude();
   TextureSector_ptr->set_center_altitude(altitude);

   TextureSector_ptr->construct_long_lat_bbox(
      pass_ptr->get_PassInfo_ptr()->get_longitude_lo(),
      pass_ptr->get_PassInfo_ptr()->get_longitude_hi(),
      pass_ptr->get_PassInfo_ptr()->get_latitude_lo(),
      pass_ptr->get_PassInfo_ptr()->get_latitude_hi());
//   cout << "*TextureSector_ptr = " << *TextureSector_ptr << endl;

   string texture_filename=pass_ptr->get_first_filename();

//   MoviesGroup_ptr->destroy_all_Movies();
   Movie* Movie_ptr=MoviesGroup_ptr->generate_new_Movie(texture_filename);
   TextureSector_ptr->reposition_Movie(Movie_ptr);

   pass_ptr->set_UTM_zonenumber(
      TextureSector_ptr->get_lower_left_corner_ptr()->
      get_UTM_zonenumber());
   pass_ptr->set_northern_hemisphere_flag(
      TextureSector_ptr->get_lower_left_corner_ptr()->
      get_northern_hemisphere_flag());

// Offset LatLongGrid in altitude from TextureSector center's altitude
// by an amount which depends upon the RMS angular size of the
// LatLongGrid:

   double grid_altitude=TextureSector_ptr->get_center_altitude();

   double delta_long=TextureSector_ptr->get_longitude_hi()-
      TextureSector_ptr->get_longitude_lo();
   double delta_lat=TextureSector_ptr->get_latitude_hi()-
      TextureSector_ptr->get_latitude_lo();
   double mean_delta=sqrt(delta_long*delta_lat);
//   cout << "delta_long = " << delta_long 
//        << " delta_lat = " << delta_lat 
//        << " mean_delta = " << mean_delta << endl;

   const double max_delta_grid_altitude=10;	// meters
   const double min_delta_grid_altitude=1;	// meters
   double delta_grid_altitude=
      min_delta_grid_altitude+
      (max_delta_grid_altitude-min_delta_grid_altitude)/(0.3-0.0015)*(
         mean_delta-0.0015);
         delta_grid_altitude=basic_math::min(
         max_delta_grid_altitude,delta_grid_altitude);
         delta_grid_altitude=basic_math::max(
         min_delta_grid_altitude,delta_grid_altitude);
//   cout << "delta_grid_altitude = " << delta_grid_altitude << endl;
   grid_altitude += delta_grid_altitude;

   LatLongGrid* LatLongGrid_ptr=latlonggrids_group_ptr->
      generate_new_Grid(
         TextureSector_ptr->get_longitude_lo(),
         TextureSector_ptr->get_longitude_hi(),
         TextureSector_ptr->get_latitude_lo(),
         TextureSector_ptr->get_latitude_hi(),grid_altitude);

//   cout << "display_LatLongGrid_flag = "
//        << display_LatLongGrid_flag << endl;
   if (!display_LatLongGrid_flag)
   {
      latlonggrids_group_ptr->mask_Graphical_for_all_times(
         LatLongGrid_ptr);
   }
   
//   cout << "long_lo = " << TextureSector_ptr->get_longitude_lo()
//        << " long_hi = " << TextureSector_ptr->get_longitude_hi() << endl;
//   cout << "lat_lo = " << TextureSector_ptr->get_latitude_lo()
//        << " lat_hi = " << TextureSector_ptr->get_latitude_hi() << endl;
//   cout << "*latlon_grid_origin_ptr = " 
//        << *(LatLongGrid_ptr->get_world_origin_ptr()) << endl;
//   cout << "*LatLongGrid_ptr = " << *LatLongGrid_ptr << endl;
//   cout << "LatLongGrid_ptr = " << LatLongGrid_ptr << endl;
//   cout << "latlonggrids_group_ptr->get_n_Graphicals() = "
//        << latlonggrids_group_ptr->get_n_Graphicals() << endl;

   EarthRegion* EarthRegion_ptr=generate_new_EarthRegion(
      LatLongGrid_ptr,Earth_ptr);
   EarthRegion_ptr->set_TextureSector_ptr(TextureSector_ptr);
   EarthRegion_ptr->get_PAT_ptr()->addChild(
      TextureSector_ptr->get_PAT_ptr());
} 
*/

// --------------------------------------------------------------------------
// This overloaded version of member function
// generate_SurfaceTexture_EarthRegion()

void EarthRegionsGroup::generate_SurfaceTexture_EarthRegion(
   string Movie_filename,vector<threevector>& video_corner_vertices)
{
//   cout << "inside EarthRegionsGroup::generate_SurfaceTexture_EarthRegion() #2"
//        << endl;

   TextureSector* TextureSector_ptr=
      TextureSectorsGroup_ptr->generate_new_TextureSector(Earth_ptr);
//   cout << "TextureSector_ptr = " << TextureSector_ptr << endl;

   double alpha=1.0;
   TextureSector_ptr->set_alpha_blending_value(alpha);

   TextureSectorsGroup_ptr->generate_TextureSector_geode(
      video_corner_vertices,TextureSector_ptr,Movie_filename);

   LatLongGrid* LatLongGrid_ptr=latlonggrids_group_ptr->
      generate_new_Grid(0,0,0,0,0);
//         TextureSector_ptr->get_longitude_lo(),
//         TextureSector_ptr->get_longitude_hi(),
//         TextureSector_ptr->get_latitude_lo(),
//         TextureSector_ptr->get_latitude_hi(),grid_altitude);
         
   EarthRegion* EarthRegion_ptr=generate_new_EarthRegion(
      LatLongGrid_ptr,Earth_ptr);
   EarthRegion_ptr->set_TextureSector_ptr(TextureSector_ptr);
   EarthRegion_ptr->get_PAT_ptr()->addChild(
      TextureSector_ptr->get_PAT_ptr());

//   cout << "EarthRegion_ptr->get_ID() = " 
//        << EarthRegion_ptr->get_ID() << endl;
} 

// ---------------------------------------------------------------------
EarthRegion* EarthRegionsGroup::generate_new_EarthRegion(
   LatLongGrid* LatLongGrid_ptr,Earth* Earth_ptr,DataGraph* DataGraph_ptr,
   int ID,int OSGsubPAT_number)
{
//   cout << "inside EarthRegionsGroup::generate_new_EarthRegion()" << endl;
//   cout << "LatLongGrid_ptr = " << LatLongGrid_ptr << endl;
   if (ID==-1) ID=get_next_unused_ID();
   EarthRegion* curr_EarthRegion_ptr=new EarthRegion(
      LatLongGrid_ptr,Earth_ptr,DataGraph_ptr,ID);

   initialize_new_EarthRegion(curr_EarthRegion_ptr,OSGsubPAT_number);

   return curr_EarthRegion_ptr;
}

// ---------------------------------------------------------------------
void EarthRegionsGroup::initialize_new_EarthRegion(
   EarthRegion* EarthRegion_ptr,int OSGsubPAT_number)
{
//   cout << "inside EarthRegionsGroup::initialize_new_EarthRegion()" << endl;
   
   GraphicalsGroup::insert_Graphical_into_list(EarthRegion_ptr);
   initialize_Graphical(EarthRegion_ptr);
   insert_graphical_PAT_into_OSGsubPAT(EarthRegion_ptr,OSGsubPAT_number);

   EarthRegion_ptr->set_AnimationController_ptr(
      get_AnimationController_ptr());
   EarthRegion_ptr->set_UTM_zonenumber(pass_ptr->get_UTM_zonenumber());
   EarthRegion_ptr->set_northern_hemisphere_flag(
      pass_ptr->get_northern_hemisphere_flag());

//   cout << "UTM zonenumber = " << EarthRegion_ptr->get_UTM_zonenumber()
//        << endl;
//   cout << "northern hemisphere flag = " 
//        << EarthRegion_ptr->get_northern_hemisphere_flag() << endl;

   EarthRegion_ptr->get_movers_group_ptr()->set_Messenger_ptr(
      urban_network_Messenger_ptr);
}

/*
// ---------------------------------------------------------------------
void EarthRegionsGroup::insert_into_cloud_transform(int r,osg::Node* node_ptr)
{
   EarthRegion* curr_region_ptr=get_ID_labeled_EarthRegion_ptr(r);
   osg::MatrixTransform* SurfaceTransform_ptr=
      curr_region_ptr->cloud_surface_transform(node_ptr);
   curr_region_ptr->get_PAT_ptr()->addChild(SurfaceTransform_ptr);
}
*/

/*
void EarthRegionsGroup::insert_into_surface_transform(
   int r,osg::Node* node_ptr)
{
   EarthRegion* curr_region_ptr=get_ID_labeled_EarthRegion_ptr(r);
   osg::MatrixTransform* SurfaceTransform_ptr=
      curr_region_ptr->get_SurfaceTransform_ptr();
   curr_region_ptr->get_PAT_ptr()->addChild(SurfaceTransform_ptr);
}
*/

// ---------------------------------------------------------------------
// Member function UTM_to_surface_transform returns the
// MatrixTransform needed to place decorations onto the blue marble
// surface for the EarthRegion labeled by input integer r.  If r does
// not correspond to a valid region, this method returns a NULL
// pointer.

osg::MatrixTransform* EarthRegionsGroup::UTM_to_surface_transform(
   unsigned int r,const threevector& UTM_translation,
    double origin_longitude,double origin_latitude,double origin_altitude,
   osg::Node* node_ptr)
{
//   cout << "inside EarthRegionsGroup::UTM_to_surface_transform()" << endl;
   if (r >= 0 && r < get_n_Graphicals())
   {
      EarthRegion* curr_region_ptr=get_ID_labeled_EarthRegion_ptr(r);
//      EarthRegion* curr_region_ptr=get_EarthRegion_ptr(r);

      threevector origin_long_lat_alt(
         origin_longitude,origin_latitude,origin_altitude);
      osg::MatrixTransform* DecorationSurfaceTransform_ptr=
         curr_region_ptr->UTM_to_surface_transform(
            UTM_translation,origin_long_lat_alt,node_ptr);
//      cout << "*DecorationSurfaceTransform_ptr = " << endl;
//      osgfunc::print_matrix(DecorationSurfaceTransform_ptr->getMatrix());
      curr_region_ptr->get_PAT_ptr()->addChild(
         DecorationSurfaceTransform_ptr);
      return DecorationSurfaceTransform_ptr;
   } // 0 <= r < get_n_Graphicals() conditional

   return NULL;
}

// ==========================================================================
// Animation member functions
// ==========================================================================

// Member function generate_EarthRegion_video_chip() is a high-level
// helper method for instantiating a video chip.

Movie* EarthRegionsGroup::generate_EarthRegion_video_chip(int r,double alpha)
{
//   cout << "inside EarthRegionsGroup::generate_EarthRegion_video_chip()"
//	  << endl;
//   cout << "r = " << r << endl;

   EarthRegion* EarthRegion_ptr=get_ID_labeled_EarthRegion_ptr(r);
   if (EarthRegion_ptr==NULL) return NULL;
   
   TextureSector* TextureSector_ptr=EarthRegion_ptr->get_TextureSector_ptr();
//   cout << "*TextureSector_ptr = " << *TextureSector_ptr << endl;
   Movie* Movie_ptr=TextureSectorsGroup_ptr->
      generate_TextureSector_video_chip(TextureSector_ptr,alpha);
//   cout << "*Movie_ptr = " << *Movie_ptr << endl;

   return Movie_ptr;
}

// --------------------------------------------------------------------------
Movie* EarthRegionsGroup::get_EarthRegion_video_chip(int r)
{
//   cout << "inside EarthRegionsGroup::get_EarthRegion_video_chip()"
//	  << endl;

   EarthRegion* EarthRegion_ptr=get_ID_labeled_EarthRegion_ptr(r);
   if (EarthRegion_ptr==NULL) return NULL;
   
   MoviesGroup* MoviesGroup_ptr=TextureSectorsGroup_ptr->
      get_MoviesGroup_ptr();
   if (MoviesGroup_ptr != NULL && MoviesGroup_ptr->get_n_Graphicals() > 0)
   {
      return MoviesGroup_ptr->get_Movie_ptr(0);
   }
   else
   {
      return NULL;
   }
}

// --------------------------------------------------------------------------
void EarthRegionsGroup::destroy_EarthRegion_video_chip(
   int r,Movie* Movie_ptr)
{
//   cout << "inside EarthRegionsGroup::destroy_EarthRegion_video_chip()" 
//        << endl;
   EarthRegion* EarthRegion_ptr=get_ID_labeled_EarthRegion_ptr(r);
   if (EarthRegion_ptr==NULL) return;
   TextureSector* TextureSector_ptr=EarthRegion_ptr->get_TextureSector_ptr();

   if (TextureSector_ptr->get_texture_rectangle_ptr() != NULL)
   {
//      cout << "TextureSector_ptr = " << TextureSector_ptr << endl;
//      cout << "Movie_ptr = " << Movie_ptr << endl;
//      cout << "TextureSectorsGroup_ptr = " << TextureSectorsGroup_ptr << endl;

      TextureSectorsGroup_ptr->destroy_TextureSector_Movie(
         TextureSector_ptr,Movie_ptr);
   }
}

// --------------------------------------------------------------------------
// Member function generate_EarthRegion_texture_rectangle() is a high-level
// helper method for instantiating a texture rectangle.

texture_rectangle* EarthRegionsGroup::generate_EarthRegion_texture_rectangle(
   int r,string texture_filename)
{
   cout << "inside EarthRegionsGroup::generate_EarthRegion_texture_rectangle()"
	  << endl;
   cout << "texture_filename = " << texture_filename << endl;

   EarthRegion* EarthRegion_ptr=get_ID_labeled_EarthRegion_ptr(r);
   if (EarthRegion_ptr==NULL) return NULL;
   
   TextureSector* TextureSector_ptr=EarthRegion_ptr->get_TextureSector_ptr();
   texture_rectangle* TextureRectangle_ptr=TextureSectorsGroup_ptr->
      generate_TextureSector_texture_rectangle(
         TextureSector_ptr,texture_filename);

   return TextureRectangle_ptr;
}

// --------------------------------------------------------------------------
// Member function update_display()

void EarthRegionsGroup::update_display()
{
//   cout << "inside EarthRegionsGroup::update_display()" << endl;

   if (!update_display_flag) return;

   parse_latest_messages();

   TextureSectorsGroup_ptr->update_display();

   if (CylindersGroup_ptr != NULL)
   {
      for (unsigned int r=0; r<get_n_Graphicals(); r++)
      {
//         cout << "r = " << r << endl;
         EarthRegion* curr_EarthRegion_ptr=get_ID_labeled_EarthRegion_ptr(r);

         if (propagate_all_tracks_flag)
         {
//         curr_EarthRegion_ptr->get_robots_Messenger_ptr()->
//            broadcast_subpacket("START_PACKET");
            curr_EarthRegion_ptr->propagate_all_tracks(CylindersGroup_ptr);
//         curr_EarthRegion_ptr->broadcast_dynamic_tracks();

//         curr_EarthRegion_ptr->get_robots_Messenger_ptr()->
//	      broadcast_subpacket("STOP_PACKET");
         }

// Real-time persistent surveillance functionality:
         
         if (check_Cylinder_ROI_intersections_flag)
         {
            curr_EarthRegion_ptr->check_Cylinder_ROI_intersections(
               CylindersGroup_ptr);

// FAKE FAKE: Mon May 4, 2009 at 4:43 pm Experiment with instantiating
// ROI following Cylinder mover

            bool bind_ROI_to_selected_Cylinder_flag=true;
            curr_EarthRegion_ptr->track_Cylinder_with_ROI_Polyhedron(
               CylindersGroup_ptr,PolyhedraGroup_ptr,
               bind_ROI_to_selected_Cylinder_flag);
         }

      } // loop over index r labeling Earth Regions

   }
   GraphicalsGroup::update_display();
}

// ==========================================================================
// Annotation member functions
// ==========================================================================

// Member function generate_annotation_bboxes loops over every 3D bbox
// within the EarthRegion specified by input integer r.  It recovers
// top_left corner, bottom_right corner, bbox color and bbox label
// information from *PassInfo_ptr.  It subsequently generates a
// translucent OSG Polyhedron to represent the bounding boxes.

void EarthRegionsGroup::generate_annotation_bboxes(int r)
{
//   cout << "inside EarthRegionsGroup::generate_annotation_bboxes()" << endl;

   if (PolyhedraGroup_ptr==NULL) return;
   int Polyhedra_subgroup=PolyhedraGroup_ptr->get_n_OSGsubPATs();

   EarthRegion* EarthRegion_ptr=get_ID_labeled_EarthRegion_ptr(r);
   if (EarthRegion_ptr==NULL) return;

   double z_grid=EarthRegion_ptr->get_LatLongGrid_ptr()->
      get_world_origin_ptr()->get(2);

   PassInfo* PassInfo_ptr=get_pass_ptr()->get_PassInfo_ptr();
   unsigned int n_bboxes=PassInfo_ptr->get_bbox_top_left_corners().size();
   for (unsigned int b=0; b<n_bboxes; b++)
   {
      threevector top_left_corner(PassInfo_ptr->
                                  get_bbox_top_left_corner(b));
      threevector bottom_right_corner(PassInfo_ptr->
                                      get_bbox_bottom_right_corner(b));
      geopoint top_left(top_left_corner.get(0),
                        top_left_corner.get(1),
                        top_left_corner.get(2),specified_UTM_zonenumber);
      geopoint bottom_right(bottom_right_corner.get(0),
                            bottom_right_corner.get(1),z_grid,
                            specified_UTM_zonenumber);

//      cout << "Bbox b = " << b << " n_bboxes = " << n_bboxes << endl;
//         cout << "top left geopoint = " << top_left << endl;
//         cout << "bottom right geopoint = " << bottom_right << endl;
//         outputfunc::enter_continue_char();

// Note added on 2/24/08: Need to perform some check on whether bbox
// lies within EarthRegion labeled by index n.  If not, this method
// should simply continue...

      string bbox_color=PassInfo_ptr->get_bbox_color(b);
      string bbox_label=PassInfo_ptr->get_bbox_label(b);
      string bbox_label_color=PassInfo_ptr->get_bbox_label_color(b);
         
      EarthRegion_ptr->generate_bbox(
         PolyhedraGroup_ptr,Polyhedra_subgroup,
         top_left,bottom_right,bbox_color,bbox_label,bbox_label_color);
   } // loop over index b labeling 3D bboxes
}

// ---------------------------------------------------------------------
// Member function generate_roadlines_group takes in integer r
// labeling a particular EarthRegion.  It instantiates and returns the
// EarthRegion's PolyLinesGroup *roadlines_group_ptr member.

PolyLinesGroup* EarthRegionsGroup::generate_roadlines_group(int r)
{
//   cout << "inside EarthRegionsGroup::generate_roadlines_group, r = " << r
//        << endl;

   return get_ID_labeled_EarthRegion_ptr(r)->
      generate_roadlines_group(get_pass_ptr());
}

// ---------------------------------------------------------------------
// Member function generate_ROIlines_group takes in integer r labeling
// a particular EarthRegion.  It instantiates and returns the
// EarthRegion's PolyLinesGroup *ROIlines_group_ptr member.

RegionPolyLinesGroup* EarthRegionsGroup::generate_ROIlines_group(int r)
{
//   cout << "inside EarthRegionsGroup::generate_ROIlines_group, r = " << r
//        << endl;

   return get_ID_labeled_EarthRegion_ptr(r)->
      generate_ROILinesGroup(get_pass_ptr(),get_AnimationController_ptr());
}

// ---------------------------------------------------------------------
// Member function generate_KOZlines_group takes in integer r labeling
// a particular EarthRegion.  It instantiates and returns the
// EarthRegion's PolyLinesGroup *KOZlines_group_ptr member.

RegionPolyLinesGroup* EarthRegionsGroup::generate_KOZlines_group(int r)
{
//   cout << "inside EarthRegionsGroup::generate_KOZlines_group, r = " << r
//        << endl;

   return get_ID_labeled_EarthRegion_ptr(r)->
      generate_KOZLinesGroup(get_pass_ptr(), get_AnimationController_ptr());
}

// ==========================================================================
// Dataserver track retrieval & display member functions
// ==========================================================================

// Member function initialize_track_mover_Cylinders() instantiates a
// new Cylinder member of *CylindersGroup_ptr for each track within
// *(EarthRegion_ptr->get_dynamic_tracks_group_ptr()).

void EarthRegionsGroup::initialize_track_mover_Cylinders(
   int r,double cylinder_radius,double cylinder_height,
   double cylinder_alpha,bool associate_tracks_with_movers_flag)
{
//   cout << "inside EarthRegionsGroup::initialize_track_mover_Cylinders()"
//        << endl;

   string banner="Initializing track mover Cylinders:";
   outputfunc::write_banner(banner);

   if (CylindersGroup_ptr==NULL) return;
   
   CylindersGroup_ptr->set_rh(cylinder_radius,cylinder_height);

//   cout << "Number of earth regions = " << get_n_Graphicals() << endl;
   EarthRegion* EarthRegion_ptr=get_ID_labeled_EarthRegion_ptr(r);
   if (EarthRegion_ptr==NULL) return;

   osg::Quat trivial_q(0,0,0,1);
   colorfunc::Color cylinder_color=colorfunc::red;
   int n_text_messages=1;
   threevector text_displacement(
      CylindersGroup_ptr->get_radius()+20,
      CylindersGroup_ptr->get_radius()+5,
      CylindersGroup_ptr->get_height()+10);

   double text_size=2.5*cylinder_radius;
   bool text_screen_axis_alignment_flag=true;
//   bool text_screen_axis_alignment_flag=false;

   tracks_group* dynamic_tracks_group_ptr=EarthRegion_ptr->
      get_dynamic_tracks_group_ptr();
   vector<track*> dynamic_track_ptrs=
      dynamic_tracks_group_ptr->get_all_track_ptrs();
   movers_group* movers_group_ptr=EarthRegion_ptr->get_movers_group_ptr();

   cout.precision(12);
   for (unsigned int t=0; t< dynamic_track_ptrs.size(); t++)
   {
      track* curr_track_ptr=dynamic_track_ptrs[t];
      cout << t << " " 
//           << curr_track_ptr->get_label() 
           << flush;

      int label_ID=curr_track_ptr->get_label_ID();
      if (associate_tracks_with_movers_flag)
      {
         movers_group_ptr->generate_new_vehicle(curr_track_ptr,label_ID);
      }
      
// Displace text towards upper right relative to Cylinder's center
// position in order for it to not obscure movers within underlying
// video:

      Cylinder* curr_Cylinder_ptr=CylindersGroup_ptr->generate_new_Cylinder(
         Zero_vector,trivial_q,cylinder_color,
         text_displacement,n_text_messages,text_size,
         text_screen_axis_alignment_flag,label_ID);

//   double alpha=0.15;		// for DiamondTouch operation
//   double alpha=0.25;
//   double alpha=0.30;		// for ppt presentation
//   double alpha=0.5;
      EarthRegion_ptr->initialize_track_mover_Cylinder(
         cylinder_alpha,curr_Cylinder_ptr,curr_track_ptr);

   } // loop over index t labeling tracks
   cout << endl;
}

// --------------------------------------------------------------------------
// Member function form_SKS_query_for_tracks_intersecting_polyline
// takes in *PolyLinesGroup_ptr whose final PolyLine is assumed to
// correspond to a user-entered, closed contour surrounding some
// Region of Interest (ROI).  This method converts the polyline's UTM
// coordinates to longitude,latitude.  It then starts to form the
// query to the SKS DataServer which should return all vehicle tracks
// intersecting the ROI.

string EarthRegionsGroup::form_SKS_query_for_tracks_intersecting_polyline(
   PolyLinesGroup* PolyLinesGroup_ptr,polyline*& ROI_polyline_ptr)
{
//   cout << "inside EarthRegionsGroup::form_SKS_query_for_tracks_intersecting_polyline()"
//        << endl;
   
   unsigned int n_PolyLines=PolyLinesGroup_ptr->get_n_Graphicals();
   string SKS_query="";
   
   if (n_PolyLines==0) return SKS_query;

   PolyLine* curr_PolyLine_ptr=PolyLinesGroup_ptr->get_PolyLine_ptr(
      n_PolyLines-1);
   curr_PolyLine_ptr->construct_polyline();
   ROI_polyline_ptr=curr_PolyLine_ptr->get_polyline_ptr();  
//   cout << "*ROI_polyline_ptr = " << *ROI_polyline_ptr << endl;

// Convert polyline vertices from UTM to Longitude/latitude coordinates:

//   SKS_query += "http://"+PassesGroup_ptr->get_SKSDataServer_URL();
//   SKS_query += "/track?

   SKS_query="mode=all&type=vehicle&labelfield=id_bg";
   SKS_query += "&ewkt=SRID=4326;POLYGON((";
   for (unsigned int v=0; v<ROI_polyline_ptr->get_n_vertices(); v++)
   {
      threevector curr_vertex(ROI_polyline_ptr->get_vertex(v));

      geopoint curr_geopoint(
         northern_hemisphere_flag,specified_UTM_zonenumber,
         curr_vertex.get(0),curr_vertex.get(1));
      double longitude=curr_geopoint.get_longitude();
      double latitude=curr_geopoint.get_latitude();
//      cout << "v = " << v << " long = " << longitude
//           << " lat = " << latitude << endl;
      SKS_query += stringfunc::number_to_string(longitude)+" "
         +stringfunc::number_to_string(latitude);
      if (v < ROI_polyline_ptr->get_n_vertices()-1)
      {
         SKS_query += ", ";
      }
      else
      {
         SKS_query += "))";
      }
   } // loop over index v labeling closed polyline vertices

   SKS_query += "&t0="+stringfunc::number_to_string(t_start);
   SKS_query += "&t1="+stringfunc::number_to_string(t_stop);

//   cout << "SKS_query = " << SKS_query << endl;
   return SKS_query;
}

// --------------------------------------------------------------------------
// Member function form_SKS_query_for_speed_ROIs loops over all
// EarthRegions and extracts their vehicle tracks' entity IDs.  It
// then forms an SKS string query for all tracklets where vehicle
// speeds lie between some specified range for at least some minimum
// time duration.  SKS should then return an XML list of tracklets
// along with an automatically derived list of Regions of Interest
// (ROIs) where these speed and time conditions hold.  The input
// radius parameter is needed to thicken tracklet lines for ROI
// generation...

string EarthRegionsGroup::form_SKS_query_for_speed_ROIs(
   double minSpeed,double maxSpeed,double minDuration,double radius,
   const vector<tracks_group*>& vehicle_tracks_group_ptrs)
{
//   cout << "inside EarthRegionsGroup::form_SKS_query_for_speed_ROIs()" 
//        << endl;

   string SKS_query="mode=speedregions&ids=";
   
// Add comma separated list of SKS entity IDs for all tracks within
// every EarthRegion's *dynamic_tracks_group_ptr to SKS_query:

   for (unsigned int v=0; v<vehicle_tracks_group_ptrs.size(); v++)
   {
      tracks_group* dynamic_tracks_group_ptr=vehicle_tracks_group_ptrs[v];

      for (unsigned int r=0; r<get_n_Graphicals(); r++)
      {
//         cout << "r = " << r << endl;
         EarthRegion* EarthRegion_ptr=get_ID_labeled_EarthRegion_ptr(r);
         movers_group* movers_group_ptr=EarthRegion_ptr->
            get_movers_group_ptr();

// Recall IDs for vehicles within *movers_group_ptr are given by
// label_IDs within *dynamic_tracks_group_ptr:

         vector<int> vehicle_label_IDs=movers_group_ptr->
            get_particular_mover_IDs(mover::VEHICLE);

         unsigned int n_tracks=vehicle_label_IDs.size();
//         cout << "n_tracks = " << n_tracks << endl;
         for (unsigned int t=0; t<n_tracks; t++)
         {
//            cout << "t = " << t << " vehicle_label_IDs[t] = " 
//                 << vehicle_label_IDs[t] << endl;
            track* curr_track_ptr=dynamic_tracks_group_ptr->
               get_track_given_label(vehicle_label_IDs[t]);
            if (curr_track_ptr==NULL) continue;

            SKS_query += curr_track_ptr->get_entityID();
//            cout << "entity_ID() = "
//                 << curr_track_ptr->get_entityID() << endl;
            if (t < n_tracks-1) SKS_query += ",";
         } // loop over index t labeling tracks
      } // loop over index r labeling EarthRegions
   } // loop over index v labeling vehicle tracks_groups
   
   SKS_query += "&t0="+stringfunc::number_to_string(t_start);
   SKS_query += "&t1="+stringfunc::number_to_string(t_stop);
   SKS_query += "&minSpeed="+stringfunc::number_to_string(minSpeed);
   SKS_query += "&maxSpeed="+stringfunc::number_to_string(maxSpeed);
   SKS_query += "&minDuration="+stringfunc::number_to_string(minDuration);
   SKS_query += "&radius="+stringfunc::number_to_string(radius);
   SKS_query += "&labelfield=id_bg";

//   cout << "SKS_query = " << SKS_query << endl;
   return SKS_query;
}

// ==========================================================================
// Message handling member functions
// ==========================================================================

bool EarthRegionsGroup::parse_next_message_in_queue(message& curr_message)
{
//   cout << "inside EarthRegionsGroup::parse_next_message_in_queue()" << endl;
//   cout << "curr_message.get_text_message() = "
//        << curr_message.get_text_message() << endl;

   bool message_handled_flag=false;

   if (curr_message.get_text_message()=="DELETE_VERTEX")
   {
//       cout << "Received DELETE_VERTEX message from ActiveMQ" << endl;
      curr_message.extract_and_store_property_keys_and_values();
      string type=curr_message.get_property_value("TYPE");
      if (type=="VEHICLE")
      {
         string ID=curr_message.get_property_value("ID");  
         int track_label_ID=stringfunc::string_to_integer(ID);

         for (unsigned int r=0; r<get_n_Graphicals(); r++)
         {
//            cout << "EarthRegion r = " << r << endl;
            movers_group* movers_group_ptr=
               get_ID_labeled_EarthRegion_ptr(r)->get_movers_group_ptr();
//            cout << "track_label_ID = " << track_label_ID << endl;
            movers_group_ptr->delete_mover(mover::VEHICLE,track_label_ID);
         } // loop over index r labeling EarthRegions
      }
      else if (type=="ROI")
      {
         string ID=curr_message.get_property_value("ID");  
         int ROI_ID=stringfunc::string_to_integer(ID);

         for (unsigned int r=0; r<get_n_Graphicals(); r++)
         {
            movers_group* movers_group_ptr=
               get_ID_labeled_EarthRegion_ptr(r)->get_movers_group_ptr();
            movers_group_ptr->delete_mover(mover::ROI,ROI_ID);
         } // loop over index r labeling EarthRegions
      } // type conditional
      message_handled_flag=true;
   }
   else if (curr_message.get_text_message()=="ADD_ROI_CENTER" ||
            curr_message.get_text_message()=="UPDATE_WAYPOINTS")
   {
      geopoint ROI_center;
      cout << "n_properties = " << curr_message.get_n_properties()
           << endl;
      for (unsigned int n=0; n<curr_message.get_n_properties(); n++)
      {
         message::Property curr_property=curr_message.get_property(n);
         string key=curr_property.first;

         string value=curr_property.second;
         vector<string> key_substrings=
            stringfunc::decompose_string_into_substrings(key);
         vector<string> value_substrings=
            stringfunc::decompose_string_into_substrings(value);
         
         if (key_substrings[0] != "LONGITUDE" || 
             key_substrings[1] != "LATITUDE")
         {
            continue;
         }
         ROI_center=geopoint(value_substrings,get_specified_UTM_zonenumber());

//      cout << "ROI_center: long = " << ROI_center.get_longitude()
//           << " lat = " << ROI_center.get_latitude() << endl;
//      cout << "easting = " << ROI_center.get_UTM_easting()
//           << " northing = " << ROI_center.get_UTM_northing() << endl;

         generate_ROI(ROI_center);

      } // loop over index n labeling message properties
      
      message_handled_flag=true;
   } // curr_message.get_text_message() conditional
   else if (curr_message.get_text_message()=="PURGE_WAYPOINTS")
   {

// FAKE FAKE: As of 8/18/08, we force added ROIs to be inserted into
// EarthRegion #0:
   
      EarthRegion* EarthRegion_ptr=get_ID_labeled_EarthRegion_ptr(0);
      EarthRegion_ptr->clear_all_ROIs();

      message_handled_flag=true;
   } // curr_message.get_text_message() conditional

   else if (curr_message.get_text_message()=="UPDATE_IPHONE_METADATA")
   {
      string value=curr_message.get_property_value("iPhone_ID");
      int ID=stringfunc::string_to_number(value);

      value=curr_message.get_property_value("imagenumber");
//      int imagenumber=stringfunc::string_to_number(value);

      value=curr_message.get_property_value("elapsed_secs");
      double elapsed_secs=stringfunc::string_to_number(value);

      value=curr_message.get_property_value("longitude");
      double longitude=stringfunc::string_to_number(value);

      value=curr_message.get_property_value("latitude");
      double latitude=stringfunc::string_to_number(value);

      value=curr_message.get_property_value("altitude");
      double altitude=stringfunc::string_to_number(value);

      value=curr_message.get_property_value("horiz_uncertaintyelapsed_secs");
//      double horiz_uncertainty=stringfunc::string_to_number(value);

      value=curr_message.get_property_value("vert_uncertainty");
      value=stringfunc::string_to_number("vert_uncertainty");
//       double vert_uncertainty=stringfunc::string_to_number(value);
//      cout << "elapsed_secs = " << elapsed_secs
//           << " longitude = " << longitude
//           << " latitude = " << latitude
//           << " altitude = " << altitude << endl;

      update_blueforce_car_posn(ID,elapsed_secs,longitude,latitude,altitude);
      message_handled_flag=true;
   } // get_text_message() conditional

   return message_handled_flag;
}

// --------------------------------------------------------------------------
// Member function generate_ROI() takes in a candidate ROI_center
// geopoint.  It first checks whether any ROI already essentially
// exists at the input ROI's location.  If so, this boolean method
// returns false.  Otherwise, this method generates a new nominated
// ROI and returns true.

bool EarthRegionsGroup::generate_ROI(const geopoint& ROI_center)
{
//   cout << "inside EarthRegionsGroup::generate_ROI()" << endl;

   const double ROI_radius=50;	// meters
//      const double ROI_radius=150;	// meters (viewgraph generation)
   const double min_separation_distance=2*ROI_radius;

// FAKE FAKE: As of 8/18/08, we force added ROIs to be inserted into
// EarthRegion #0:
   
   EarthRegion* EarthRegion_ptr=get_ID_labeled_EarthRegion_ptr(0);

// Check whether new input ROI already essentially exists within ROI
// spatially fixed tracks group.  If so, do not instantiate it again:

   bool ROI_already_exists_flag=false;
            
   tracks_group* tracks_group_ptr=EarthRegion_ptr->
      get_spatially_fixed_tracks_group_ptr();
   vector<track*> track_ptrs=tracks_group_ptr->get_all_track_ptrs();
   for (unsigned int t=0; t<track_ptrs.size(); t++)
   {
      track* track_ptr=track_ptrs[t];
      threevector ROI_posn(track_ptr->get_earliest_posn());
//         cout << "t = " << t 
//              << " ROI_posn.x = " << ROI_posn.get(0)
//              << " ROI_posn.y = " << ROI_posn.get(1) << endl;
      double curr_separation=
         sqrt(sqr(ROI_posn.get(0)-ROI_center.get_UTM_easting())+
              sqr(ROI_posn.get(1)-ROI_center.get_UTM_northing()));
      if (curr_separation < min_separation_distance)
      {
         ROI_already_exists_flag=true;
//            cout << "ROI_already_exists!" << endl;
      }
   } // loop over index t labeling tracks

//   cout << "world origin = " 
//        << EarthRegion_ptr->get_LatLongGrid_ptr()->get_world_origin()
//        << endl;

// cout << "ROI_already_exists_flag = "
//      << ROI_already_exists_flag << endl;

   if (!ROI_already_exists_flag)
   {
//      double ROI_center_altitude=EarthRegion_ptr->get_LatLongGrid_ptr()->
//         get_world_origin().get(2);
      vector<threevector> vertices=
         EarthRegion_ptr->generate_ROI_circle_vertices(
            ROI_center.get_longitude(),ROI_center.get_latitude(),
            ROI_center.get_altitude(),
            ROI_radius,specified_UTM_zonenumber);
      
      EarthRegion_ptr->generate_nominated_ROI(vertices,ROI_color);
   } // !ROI_already_exists_flag conditional

   return !ROI_already_exists_flag;
}

// ==========================================================================
// Real time persistent surveillance specific member functions
// ==========================================================================

// Member function initialize_RTPS_mover_Cylinders() pre-instantiates
// n_cylinders Cylinders for the real-time persistent surveillance
// project.  Since Cylinder generation is time consuming, we prefer to
// pre-instantiate some reasonable number at the time the main program
// starts up.  Then these cylinders will already be in place to
// respond to ActiveMQ messages indicating where they should be
// positioned within the Milwaukee map.  This method also instantiates
// a new track within
// *(EarthRegion_ptr->get_dynamic_tracks_group_ptr()) corresponding to
// each Cylinder as well as a vehicle within movers_group_ptr if
// associate_tracks_with_movers_flag==true.

void EarthRegionsGroup::initialize_RTPS_mover_Cylinders(
   int n_cylinders,int starting_ID,bool associate_tracks_with_movers_flag)
{
//   cout << "inside EarthRegionsGroup::initialize_RTPS_mover_Cylinders()"
//        << endl;

   if (CylindersGroup_ptr==NULL) return;

   string banner="Initializing RTPS Cylinders:";
   outputfunc::write_banner(banner);

// As of 5/25/09, we assume all Cylinders exist within EarthRegion #0:

   int r=0;
   EarthRegion* EarthRegion_ptr=get_ID_labeled_EarthRegion_ptr(r);
   if (EarthRegion_ptr==NULL) return;

   osg::Quat trivial_q(0,0,0,1);
   int n_text_messages=1;
   threevector text_displacement(
      CylindersGroup_ptr->get_radius()+20,
      CylindersGroup_ptr->get_radius()+5,
      CylindersGroup_ptr->get_height()+10);
   double text_size=5*CylindersGroup_ptr->get_radius();
   bool text_screen_axis_alignment_flag=true;

   double alpha_permanent=0.20;
   double alpha_selected=0.20;
   colorfunc::Color permanent_cylinder_color=colorfunc::red;
   colorfunc::Color selected_cylinder_color=colorfunc::pink;

// As of 6/4/09, we assume IDs for Cylinders greater than 10000
// represent moving iPhones:

   if (starting_ID >= 10000)
   {
      permanent_cylinder_color=colorfunc::blue;
      selected_cylinder_color=colorfunc::cyan;
   }

   tracks_group* dynamic_tracks_group_ptr=EarthRegion_ptr->
      get_dynamic_tracks_group_ptr();
   vector<track*> dynamic_track_ptrs=
      dynamic_tracks_group_ptr->get_all_track_ptrs();
   movers_group* movers_group_ptr=EarthRegion_ptr->get_movers_group_ptr();

   for (int ID=starting_ID; ID<starting_ID+n_cylinders; ID++)
   {
      cout << ID << " " << flush;
      Cylinder* Cylinder_ptr=CylindersGroup_ptr->generate_new_Cylinder(
         Zero_vector,trivial_q,permanent_cylinder_color,
         text_displacement,n_text_messages,text_size,
         text_screen_axis_alignment_flag,ID); 

      Cylinder_ptr->set_permanent_color(colorfunc::get_OSG_color(
         permanent_cylinder_color,alpha_permanent));
      Cylinder_ptr->set_selected_color(colorfunc::get_OSG_color(
         selected_cylinder_color,alpha_selected));

      string text_label=stringfunc::number_to_string(ID);
      if (ID >= 10000)
      {
         text_label=stringfunc::number_to_string(ID-10000);
      }
      Cylinder_ptr->set_text_label(0,text_label);
      Cylinder_ptr->set_text_color(0,permanent_cylinder_color);

      Cylinder_ptr->set_stationary_Graphical_flag(false);

      track* curr_track_ptr=dynamic_tracks_group_ptr->generate_new_track();

      curr_track_ptr->set_label_ID(ID);
      int label_ID=curr_track_ptr->get_label_ID();

// Add (track_label_ID,track_ID) pair to *dynamic_tracks_group_ptr's
// track_label_map member:

      (*dynamic_tracks_group_ptr->get_track_labels_map_ptr())
         [curr_track_ptr->get_label_ID()]=curr_track_ptr->get_ID();

      if (associate_tracks_with_movers_flag)
      {
         movers_group_ptr->generate_new_vehicle(curr_track_ptr,label_ID);
      }
      
// Associate track with Cylinder and vice-versa:

      Cylinder_ptr->set_track_ptr(curr_track_ptr);
      curr_track_ptr->set_Cylinder_ID(Cylinder_ptr->get_ID());

   } // loop over ID index 
   cout << endl;
}

// --------------------------------------------------------------------------
// Member function update_blueforce_car_posn() takes in time and
// position metadata presumably transmited by an iPhone.  

void EarthRegionsGroup::update_blueforce_car_posn(
   int blueforce_track_ID,double elapsed_secs,
   double longitude,double latitude,double altitude)
{
//   cout << "inside EarthRegionsGroup::update_blueforce_car_posn()" << endl;
//   cout << "blueforce_track_ID = " << blueforce_track_ID << endl;
//   cout << "elapsed_secs = " << elapsed_secs
//        << " longitude = " << longitude << " latitude = " << latitude
//        << endl;

/*
// FAKE FAKE: For iPhone testing purposes only, we offset input
// longitude,latitude values by a (Milwaukee-LL) translation:

   const double LL_longitude=-71.278;
   const double LL_latitude=42.4595;

   const double Milwaukee_longitude=-88;
   const double Milwaukee_latitude=43;

   longitude=Milwaukee_longitude+(longitude-LL_longitude);
   latitude=Milwaukee_latitude+(latitude-LL_latitude);
*/

   CylindersGroup_ptr->update_blueforce_car_posn(
      blueforce_track_ID,elapsed_secs,longitude,latitude,altitude,
      specified_UTM_zonenumber);
}
