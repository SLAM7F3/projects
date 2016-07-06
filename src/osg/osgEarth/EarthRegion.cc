// ==========================================================================
// EarthRegion class member function definitions
// ==========================================================================
// Last updated on 5/10/10; 5/11/10; 1/2/11; 1/22/16
// ==========================================================================

#include <iostream>
#include <set>
#include "color/colorfuncs.h"
#include "osg/osgGeometry/CylindersGroup.h"
#include "osg/osgEarth/Earth.h"
#include "osg/osgEarth/EarthRegion.h"
#include "astro_geo/geopoint.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "osg/osg3D/PointCloud.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "geometry/polyhedron.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "math/statevector.h"

#include "osg/osgfuncs.h"
#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void EarthRegion::allocate_member_objects()
{
   dynamic_tracks_group_ptr=new tracks_group();
   spatially_fixed_tracks_group_ptr=new tracks_group();
   KOZ_tracks_group_ptr=new tracks_group();
   movers_group_ptr=new movers_group();
}		       

void EarthRegion::initialize_member_objects()
{
   Graphical_name="EarthRegion";
   UTM_zonenumber=-1;
   prev_framenumber=prev_framenumber2=-1;
   northern_hemisphere_flag=false;
   Earth_ptr=NULL;
   DataGraph_ptr=NULL;
   LatLongGrid_ptr=NULL;
   robots_Messenger_ptr=NULL;
   roadlines_group_ptr=NULL;
   ROILinesGroup_ptr=NULL;
   KOZLinesGroup_ptr=NULL;
   TextureSector_ptr=NULL;
   SurfaceTransform_ptr=NULL;
   GridSurfaceTransform_ptr=NULL;
}

EarthRegion::EarthRegion(int id):
   Geometrical(3,id)
{	
//   cout << "Inside simple EarthRegion constructor" << endl;
   allocate_member_objects();
   initialize_member_objects();
}		       

EarthRegion::EarthRegion(
   LatLongGrid* LLG_ptr,Earth* E_ptr,DataGraph* DG_ptr,int id):
   Geometrical(3,id)
{	
//   cout << "Inside EarthRegion constructor, id = " << id << endl;

   allocate_member_objects();
   initialize_member_objects();

   LatLongGrid_ptr=LLG_ptr;
   Earth_ptr=E_ptr;
   DataGraph_ptr=DG_ptr;
}		       

EarthRegion::~EarthRegion()
{
   delete dynamic_tracks_group_ptr;
   delete spatially_fixed_tracks_group_ptr;
   delete KOZ_tracks_group_ptr;
   delete movers_group_ptr;

   for (unsigned int b=0; b<bbox_ptrs.size(); b++)
   {
      delete bbox_ptrs[b];
   }
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const EarthRegion& r)
{
   outstream << "inside EarthRegion::operator<<" << endl;
   return(outstream);
}

// ==========================================================================
// MatrixTransform member functions
// ==========================================================================

// Member function place_cloud_[latlonggrid]_onto_surface computes the
// MatrixTransforms needed to place point clouds with their attendant
// LatLongGrids onto the blue marble surface for the current
// EarthRegion.

void EarthRegion::place_cloud_onto_ellipsoid_surface(double altitude_offset)
{
//   cout << "inside EarthRegion::place_cloud_onto_ellipsoid_surface()" << endl;
   
   if (Earth_ptr != NULL)
   {
      threevector UTM_translation=Earth_ptr->undo_datagraph_translation(
         dynamic_cast<PointCloud*>(DataGraph_ptr));
      threevector origin_long_lat_alt=Earth_ptr->
         datagraph_origin_long_lat_alt(dynamic_cast<PointCloud*>(
            DataGraph_ptr),UTM_translation,altitude_offset);
      SurfaceTransform_ptr=UTM_to_surface_transform(
         UTM_translation,origin_long_lat_alt,
         DataGraph_ptr->get_DataNode_ptr());
//      cout << "*Surface transform = " << endl;
//      osgfunc::print_matrix(SurfaceTransform_ptr->getMatrix());
      get_PAT_ptr()->addChild(SurfaceTransform_ptr);
   }
}

void EarthRegion::place_latlonggrid_onto_ellipsoid_surface(
   double altitude_offset)
{
//   cout << "inside EarthRegion::place_latlonggrid_onto_ellipsoid_surface()"
//        << endl;
//   cout << "altitude_offset = " << altitude_offset << endl;
   
   if (Earth_ptr != NULL)
   {
      threevector UTM_translation(0,0,0);
      threevector origin_long_lat_alt(
         LatLongGrid_ptr->get_longitude_middle(),
         LatLongGrid_ptr->get_latitude_middle(),altitude_offset);
      GridSurfaceTransform_ptr=UTM_to_surface_transform(
         UTM_translation,origin_long_lat_alt,
         LatLongGrid_ptr->get_geode_ptr());
//      cout << "*Grid Surface transform = " << endl;
//      osgfunc::print_matrix(GridSurfaceTransform_ptr->getMatrix());
      get_PAT_ptr()->addChild(GridSurfaceTransform_ptr);
   }
}

// ---------------------------------------------------------------------
// Member function UTM_to_surface_transform first performs a
// transformation on input *node_ptr to counter the input
// UTM_translation (i.e. it translates *node_ptr from whatever
// coordinate position it has in some UTM zone to (0,0,0).)  It next
// rotates *node_ptr so that its 3D orientation is consistent with the
// LatLongGrid's tangent plane.  Finally, this method retranslates
// *node_ptr so that its origin's coordinates in XYZ geocentric
// coordinates equal those specified within input threevector
// origin_long_lat_alt.  *node_ptr is attached to the composite
// MatrixTransform generated and returned by this method.

osg::MatrixTransform* EarthRegion::UTM_to_surface_transform(
   const threevector& UTM_translation,
   const threevector& origin_long_lat_alt,osg::Node* node_ptr)
{
//   cout << "inside EarthRegion::UTM_to_surface_trans()" << endl;

// First counter UTM translation and bring object to (0,0,0) in UTM
// space: 

   osg::MatrixTransform* TransTransform_ptr=osgfunc::generate_trans(
      -UTM_translation);
//   cout << "TransTransform_ptr->get_matrix() = " << endl;
//   osgfunc::print_matrix(TransTransform_ptr->getMatrix());

// As of Jun 25, we're not sure why removing the following line leads
// to a Baghdad appearing blank red when we fly into it with the EARTH
// program.  So for now, we do not comment out this line...

   if (node_ptr != NULL) TransTransform_ptr->addChild(node_ptr);

// First Z-rotate decoration about its midpoint in order to optimally
// align the UTM subgrid with underlying lines of longitude and
// latitude on the earth ellipsoid:

   osg::MatrixTransform* UTM_to_LL_rot_transform_ptr=Earth_ptr->
      generate_UTM_to_latlong_grid_rot_MatrixTransform(
         LatLongGrid_ptr->
         get_or_compute_UTM_to_latlong_gridlines_rot_angle());
//   cout << "UTM_to_LL_rot_transform_ptr->get_matrix() = " << endl;
//   osgfunc::print_matrix(UTM_to_LL_rot_transform_ptr->getMatrix());

// Next place decoration onto blue marble's surface so that its origin
// lands at the longitude, latitude and altitude passed into this
// method via the const threevector argument.  Moreover, rotate the
// decoration's XYZ axes so they become aligned with the local east,
// north and radial directions:

   osg::MatrixTransform* EarthSurfaceTransform_ptr=Earth_ptr->
      generate_earthsurface_MatrixTransform(origin_long_lat_alt);
//   cout << "EarthSurfaceTransform_ptr->get_matrix() = " << endl;
//   osgfunc::print_matrix(EarthSurfaceTransform_ptr->getMatrix());

// Consolidate TransTransform, UTM_to_LL_rot_transform and
// EarthSurfaceTransform into a single MatrixTransform.  Recall that
// OSG multiplies on the right rather than left when concatenating
// matrices together:

   osg::Matrixd total_transform_mat=TransTransform_ptr->getMatrix() * 
      UTM_to_LL_rot_transform_ptr->getMatrix() * 
      EarthSurfaceTransform_ptr->getMatrix();
   osg::MatrixTransform* TotalTransform_ptr=
      new osg::MatrixTransform(total_transform_mat);

   if (node_ptr != NULL) TotalTransform_ptr->addChild(node_ptr);

   return TotalTransform_ptr;
}

// ==========================================================================
// Coordinate transformation member functions
// ==========================================================================

// Member function retrieve_lat_long_alt first checks whether a UTM
// zone has been defined for the current pass.  If so, this method
// converts the input UTM coordinates to a (latitude, longitude,
// altitude) triple.  The information is returned within
// curr_geopoint, and this boolean method returns true.  Otherwise,
// this method returns false.

bool EarthRegion::retrieve_lat_long_alt(
   const threevector& UTM_coords,geopoint* geopoint_ptr)
{   
//   cout << "inside EarthRegion::retrieve_lat_long_alt()" << endl;
   
   bool geopoint_successfully_computed=false;
   if (UTM_zonenumber > 0)
   {
      *geopoint_ptr=latlongfunc::compute_geopoint(
         northern_hemisphere_flag,UTM_zonenumber,UTM_coords);

/*
      cout << "Latitude: " << geopoint_ptr->get_lat_degs() << " degs " 
           << geopoint_ptr->get_lat_mins() << " mins " 
           << geopoint_ptr->get_lat_secs() << " secs = "
           << geopoint_ptr->get_latitude() << endl;
      cout << "Longitude: " << geopoint_ptr->get_long_degs() << " degs " 
           << geopoint_ptr->get_long_mins() << " mins " 
           << geopoint_ptr->get_long_secs() << " secs = "
           << geopoint_ptr->get_longitude() << endl;
      cout << "Altitude = " << geopoint_ptr->get_altitude() 
           << " meters" << endl;
      cout << "UTM_coords = " << UTM_coords << endl;
*/

      geopoint_successfully_computed=true;
   }
   return geopoint_successfully_computed;
}


// ==========================================================================
// GMTI target member functions
// ==========================================================================

void EarthRegion::add_GMTI_target(const geopoint& curr_GMTI_target)
{
   GMTI_targets.push_back(curr_GMTI_target);
}

// ==========================================================================
// Annotation member functions
// ==========================================================================

// Member function generate_bbox instantiates a 3D bounding box based
// upon the top_left and bottom_right geopoint eastings and northings
// passed in as arguments.  The max [min] height for the bbox is
// assumed to be specified by the altitude of the top_left
// [bottom_right] geopoint.  This method instantiates an OSG
// Polyhedron and attaches it to *PolyhedraGroup_ptr.  Bounding_box
// member *bbox_ptr is also instantiated and filled by this method.

Polyhedron* EarthRegion::generate_bbox(
   PolyhedraGroup* PolyhedraGroup_ptr,int Polyhedra_subgroup,
   const geopoint& top_left_corner,const geopoint& bottom_right_corner,
   string bbox_color_str,string bbox_label,string bbox_label_color_str)
{
//   cout << "inside EarthRegion::generate_bbox()" << endl;
//   cout << "EarthRegion get_ID() = " << get_ID() << endl;
//   cout << "bbox_color_str = " << bbox_color_str << endl;
//   cout << "bbox_label = " << bbox_label << endl;
//   cout << "bbox_label_color = " << bbox_label_color << endl;

   if (PolyhedraGroup_ptr==NULL) return NULL;

   double min_X=top_left_corner.get_UTM_easting();
   double max_X=bottom_right_corner.get_UTM_easting();
   double min_Y=bottom_right_corner.get_UTM_northing();
   double max_Y=top_left_corner.get_UTM_northing();
   double min_Z=bottom_right_corner.get_altitude();
   double max_Z=top_left_corner.get_altitude();

//   cout << "min_X = " << min_X << " max_X = " << max_X << endl;
//   cout << "min_Y = " << min_Y << " max_Y = " << max_Y << endl;
//   cout << "min_Z = " << min_Z << " max_Z = " << max_Z << endl;
   bounding_box* curr_bbox_ptr=new bounding_box(min_X,max_X,min_Y,max_Y);
//   cout << "EarthRegion ID = " << get_ID() << endl;
//   cout << "Curr bbox ID = " << bbox_ptrs.size() << endl;
//   cout << "*curr_bbox_ptr = " << *curr_bbox_ptr << endl;
   bbox_ptrs.push_back(curr_bbox_ptr);

   polyhedron P;
   P.generate_box(min_X,max_X,min_Y,max_Y,min_Z,max_Z);

   Polyhedron* Polyhedron_ptr=
      PolyhedraGroup_ptr->generate_new_Polyhedron(&P,-1,Polyhedra_subgroup);
//   Polyhedron_ptr->build_current_polyhedron(
//      PolyhedraGroup_ptr->get_curr_t(),
//      PolyhedraGroup_ptr->get_passnumber());

   colorfunc::Color bbox_color=colorfunc::string_to_color(bbox_color_str);

   double alpha=0.25;
   Polyhedron_ptr->set_color(
      colorfunc::get_OSG_color(bbox_color),
      colorfunc::get_OSG_color(bbox_color,alpha));

// Specify text's position relative to local 3D bounding box origin
// (lower right corner) rather than in global world space:

   threevector text_posn(
      0.5*(max_X-min_X),0.5*(max_Y-min_Y),1.05*(max_Z-min_Z));
   bool text_screen_axis_alignment_flag=false;

   colorfunc::Color bbox_label_color=
      colorfunc::string_to_color(bbox_label_color_str);
   Polyhedron_ptr->generate_text(0,text_posn,bbox_label_color,
                                 text_screen_axis_alignment_flag);
   Polyhedron_ptr->set_text_label(0,bbox_label);

   double text_size=0.15*sqrt(text_posn.get(0)*text_posn.get(1));
//   cout << "text_size = " << text_size << endl;
   Polyhedron_ptr->set_text_size(0,text_size);

   return Polyhedron_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_roadlines_group instantiates
// PolyLinesGroup member *roadlines_group_ptr and masks its contents.

PolyLinesGroup* EarthRegion::generate_roadlines_group(Pass* pass_ptr)
{
//   cout << "inside EarthRegion::generate_roadlines_group()" << endl;

   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
   roadlines_group_ptr=new PolyLinesGroup(3,pass_ptr,grid_origin_ptr);
   roadlines_group_ptr->set_width(2); // for touch table projector
//   roadlines_group_ptr->set_width(3);

   get_PAT_ptr()->addChild(roadlines_group_ptr->get_OSGgroup_ptr());

// Initially mask roadlines_group's contents.  Recall that
// roadlines_group's nodemask can be toggled on & off via
// EarthRegionsKeyHandler call:

   roadlines_group_ptr->set_OSGgroup_nodemask(0);

   return roadlines_group_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_ROILinesGroup instantiates PolyLinesGroup
// member *ROILinesGroup_ptr.

RegionPolyLinesGroup* EarthRegion::generate_ROILinesGroup(
   Pass* pass_ptr, AnimationController* AC_ptr)
{
//   cout << "inside EarthRegion::generate_ROILinesGroup()" << endl;

   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
   ROILinesGroup_ptr=new RegionPolyLinesGroup(
      3,pass_ptr,AC_ptr,grid_origin_ptr);
   ROILinesGroup_ptr->set_width(2);  // for touch table projector

   get_PAT_ptr()->addChild(ROILinesGroup_ptr->get_OSGgroup_ptr());

   return ROILinesGroup_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_KOZLinesGroup instantiates
// PolyLinesGroup member *KOZLinesGroup_ptr.

RegionPolyLinesGroup* EarthRegion::generate_KOZLinesGroup(
   Pass* pass_ptr, AnimationController* AC_ptr)
{
//   cout << "inside EarthRegion::generate_KOZLinesGroup()" << endl;

   threevector* grid_origin_ptr=LatLongGrid_ptr->get_world_origin_ptr();
   KOZLinesGroup_ptr=new RegionPolyLinesGroup(
      3,pass_ptr,AC_ptr,grid_origin_ptr);
   KOZLinesGroup_ptr->set_width(2);  // for touch table projector

   get_PAT_ptr()->addChild(KOZLinesGroup_ptr->get_OSGgroup_ptr());

   return KOZLinesGroup_ptr;
}

// ==========================================================================
// Track member functions
// ==========================================================================

// Member function generate_track_colors assigns distinct colors to
// each track within input *curr_tracks_group_ptr.  Permanent, uniform
// track color information is saved within each track's RGB_color
// member.

void EarthRegion::generate_track_colors(
   int n_cumulative_tracks,tracks_group* curr_tracks_group_ptr)
{
//   cout << "inside EarthRegion::generate_track_colors()" << endl;

   int n_tracks=curr_tracks_group_ptr->get_n_tracks();
//   cout << "n_tracks = " << n_tracks << endl;
   vector<colorfunc::RGB> output_RGBs=
      colorfunc::generate_distinct_colors(n_tracks,n_cumulative_tracks);

   vector<track*> curr_track_ptrs=curr_tracks_group_ptr->get_all_track_ptrs();
   for (unsigned int t=0; t<curr_track_ptrs.size(); t++)
   {
      track* curr_track_ptr=curr_track_ptrs[t];
//      cout << "t = " << t << " curr_track_ptr = " << curr_track_ptr << endl;

// Store track color within its RGB_color member:

//   cout << "curr_RGB.first = " << output_RGBs[t].first
//        << " curr_RGB.second = " << output_RGBs[t].second
//        << " curr_RGB.third = " << output_RGBs[t].third << endl;
      curr_track_ptr->set_RGB_color(output_RGBs[t]);
   } // loop over index t labeling tracks
}

// --------------------------------------------------------------------------
// Member function display_tracks_as_PolyLines draws every track
// within input tracks_group *curr_tracks_group_ptr as colored
// PolyLines.  Arrows indicating individual track flows are also
// periodically displayed.

void EarthRegion::display_tracks_as_PolyLines(
   int n_cumulative_tracks,tracks_group* curr_tracks_group_ptr,
   PointFinder* pointfinder_ptr,PolyLinesGroup* PolyLinesGroup_ptr)
{
//   cout << "inside EarthRegion::display_tracks_as_PolyLines()" << endl;

   if (PolyLinesGroup_ptr==NULL) return;

   int n_tracks=curr_tracks_group_ptr->get_n_tracks();
//   cout << "n_tracks = " << n_tracks << endl;
   const double distance_between_arrows=500;	// meters

   vector<track*> curr_track_ptrs=curr_tracks_group_ptr->get_all_track_ptrs();
   for (unsigned int t=0; t<curr_track_ptrs.size(); t++)
   {
      track* curr_track_ptr=curr_track_ptrs[t];
//      cout << "label = " << curr_track_ptr->get_label_ID()
//           <<  " start time = " << curr_track_ptr->get_earliest_time()
//           << " stop time = " << curr_track_ptr->get_latest_time()
//           << " npnts = " << curr_track_ptr->size()
//           << endl;
      
      if (curr_track_ptr==NULL)
      {
         cout << "Error in EarthRegion::display_tracks_as_PolyLines()"
              << endl;
         cout << "t = " << t << " n_tracks = " << n_tracks
              << " curr_track_ptr=NULL" << endl;
         outputfunc::enter_continue_char();
         continue;
      }
      
      display_track_as_PolyLine(
         pointfinder_ptr,PolyLinesGroup_ptr,curr_track_ptr,
         distance_between_arrows);

   } // loop over index t labeling tracks
}

// ---------------------------------------------------------------------
// Member function display_track_as_PolyLine takes in track
// *curr_track_ptr and draws it as a colored PolyLine with
// periodically spaced arrows indicating track flow.  This method also
// forms a kdtree from quasi-regular samples along the track's
// polyline for fast PolyLine picking purposes.  It also associates
// the track with the PolyLine and vice-versa. This method stores
// mover network index for *curr_track_ptr track in
// *curr_PolyLine_ptr.

void EarthRegion::display_track_as_PolyLine(
   PointFinder* pointfinder_ptr,PolyLinesGroup* PolyLinesGroup_ptr,
   track* curr_track_ptr,double distance_between_arrows)
{
//   cout << "inside EarthRegion::display_track_as_PolyLine()" << endl;

   if (PolyLinesGroup_ptr==NULL) return;

// Take LatLongGrid's middle as reference origin point:

   if (LatLongGrid_ptr==NULL)
   {
      cout << "Error in EarthRegion::display_track_as_PolyLine()" << endl;
      cout << "LatLongGrid_ptr=NULL!" << endl;
      exit(-1);
   }
   threevector reference_origin=LatLongGrid_ptr->get_world_origin();

   colorfunc::RGB track_RGB=curr_track_ptr->get_RGB_color();
   osg::Vec4 track_color(track_RGB.first,track_RGB.second,track_RGB.third,1);

// Recall that track positions lie on top of each other whenever the
// mover stands still.  So we use only the subset of distinct track
// positions to generate the new PolyLine:

   curr_track_ptr->compute_distinct_posns();
   PolyLine* curr_PolyLine_ptr=PolyLinesGroup_ptr->generate_new_PolyLine(
      reference_origin,curr_track_ptr->get_distinct_posns(),track_color);
//   cout << "curr_PolyLine_ptr = " << curr_PolyLine_ptr << endl;
   curr_PolyLine_ptr->set_selected_color(colorfunc::white);
   curr_PolyLine_ptr->add_flow_direction_arrows(
      distance_between_arrows,PolyLinesGroup_ptr->get_width());

// Quasi-regulary sample track's polyline and form a kdtree from the
// polyline position samples.  We need to precompute this kdtree for
// fast track picking purposes:

//   double ds=1;	// meter
//   double ds=5;	// meter
   double ds=20;	// meter
//   double ds=100;	// meter
   curr_PolyLine_ptr->get_or_set_polyline_ptr()->
      generate_sampled_edge_points_kdtree(ds);

// Associate track with PolyLine and vice-versa:

   curr_PolyLine_ptr->set_track_ptr(curr_track_ptr);
   curr_track_ptr->set_PolyLine_ID(curr_PolyLine_ptr->get_ID());

// Store mover network index for current track in *curr_PolyLine_ptr:

   int mover_label_ID=curr_track_ptr->get_label_ID();
   int r=movers_group_ptr->get_mover_network_index(
      mover::VEHICLE,mover_label_ID);

   mover* curr_mover_ptr=movers_group_ptr->get_mover_ptr(r);
   curr_PolyLine_ptr->set_mover_ptr(curr_mover_ptr);

//   curr_PolyLine_ptr->assign_heights_using_pointcloud(pointfinder_ptr);
}

// ---------------------------------------------------------------------
// Member function initialize_track_mover_Cylinder takes in track
// *curr_track_ptr.  It instantiates a new semi-translucent Cylinder
// whose instantaneous 3D position over time is set by the input
// track.  For Bluegrass ground truth vehicle identification purposes,
// the Cylinder's text label is slaved to the track's label.

void EarthRegion::initialize_track_mover_Cylinder(
   double alpha,Cylinder* curr_Cylinder_ptr,track* curr_track_ptr)
{
//   cout << "inside EarthRegion::initialize_track_mover_Cylinder()" << endl;

//   double alpha_selected=0.10;
   double alpha_selected=0.15;
   colorfunc::Color permanent_cylinder_color=colorfunc::red;
   colorfunc::Color selected_cylinder_color=colorfunc::green;

   curr_Cylinder_ptr->set_permanent_color(
      colorfunc::get_OSG_color(permanent_cylinder_color,alpha));
   curr_Cylinder_ptr->set_selected_color(
      colorfunc::get_OSG_color(selected_cylinder_color,alpha_selected));
   curr_Cylinder_ptr->set_stationary_Graphical_flag(false);

// Associate track with Cylinder and vice-versa:

   curr_Cylinder_ptr->set_track_ptr(curr_track_ptr);
   curr_track_ptr->set_Cylinder_ID(curr_Cylinder_ptr->get_ID());
   
// Recall Bluegrass tracks have nontrivial labels associated with them
// within the SKSDataServer.  Display the numbers associated with
// those labels rather than conventional, sequential integer IDs for
// Bluegrass movers:

   string text_label=stringfunc::number_to_string(
      curr_Cylinder_ptr->get_ID());
   string track_label=curr_track_ptr->get_label();
   if (track_label.size() > 0)
   {
      text_label=stringfunc::suffix(track_label,"V");
      cout << "Generating Cylinder for track labeled as " << text_label
           << endl;
   }
   
   curr_Cylinder_ptr->set_text_label(0,text_label);
   colorfunc::Color text_color=colorfunc::red;
   curr_Cylinder_ptr->set_text_color(0,text_color);
}

// ---------------------------------------------------------------------
// Member function propagate_all_tracks retrieves the current
// framenumber and converts it to the current world time.  For each
// track within *tracks_group_ptr, it retrieves the current
// statevector and updates the position for the corresponding Cylinder
// within *CylindersGroup_ptr.  If no valid position for a track's
// Cylinder is found, the Cylinder is masked.

void EarthRegion::propagate_all_tracks(CylindersGroup* CylindersGroup_ptr)
{
//   cout << "inside EarthRegion::propagate_all_tracks()" << endl;
//   cout << "AnimationController_ptr = " << AnimationController_ptr << endl;
   if (AnimationController_ptr==NULL) return;

   int curr_framenumber=AnimationController_ptr->get_curr_framenumber();
   if (curr_framenumber==prev_framenumber)
   {
      return;
   }
   else
   {
      prev_framenumber=curr_framenumber;
   }
//   cout << "curr_framenumber= " << curr_framenumber << endl;
   
   double secs_elapsed_since_epoch=
      AnimationController_ptr->get_time_corresponding_to_curr_frame();
//   cout << "secs_elapsed_since_epoch = "
//        << secs_elapsed_since_epoch << endl;
   int pass_number=CylindersGroup_ptr->get_passnumber();

//   cout << "n_tracks = " << dynamic_tracks_group_ptr->get_n_tracks() << endl;
   for (unsigned int t=0; t<CylindersGroup_ptr->get_n_Graphicals(); t++)
   {
      Cylinder* curr_Cylinder_ptr=CylindersGroup_ptr->get_Cylinder_ptr(t);
      const track* curr_track_ptr=curr_Cylinder_ptr->get_track_ptr();

      statevector curr_statevector;
      if (curr_track_ptr->get_interpolated_statevector(
         secs_elapsed_since_epoch,curr_statevector))
      {

// In order to avoid annoying "race conditions" between updating robot
// statevectors and drawing robot icons within the main OSG event
// loop, we set the robot's statevector at both the current and next
// framenumber equal to curr_statevector.get_position():

//         cout << "Track # = " << t 
//              << " start time = " << curr_track_ptr->get_earliest_time()
//              << " curr time = " << secs_elapsed_since_epoch << " "
//              << " stop time = " << curr_track_ptr->get_latest_time()
//              << curr_statevector.get_position().get(0) << " , " 
//              << curr_statevector.get_position().get(1) << "   " 
//              << curr_statevector.get_velocity().get(0) << " , " 
//              << curr_statevector.get_velocity().get(1) << " , " 
//              << endl;
         
         threevector curr_posn(curr_statevector.get_position());
         int n_repeats=1;
         
         for (int r=0; r<n_repeats; r++)
         {
            curr_Cylinder_ptr->set_mask(curr_framenumber+r,pass_number,false);
            curr_Cylinder_ptr->
               set_UVW_coords(curr_framenumber+r,pass_number,curr_posn);
         }
/*
         curr_Cylinder_ptr->set_mask(curr_framenumber+1,pass_number,false);
         curr_Cylinder_ptr->
            set_UVW_coords(curr_framenumber,pass_number,curr_posn);
         curr_Cylinder_ptr->
            set_UVW_coords(curr_framenumber+1,pass_number,curr_posn);
*/

      } 
      else
      {
         curr_Cylinder_ptr->set_mask(curr_framenumber,pass_number,true);
         curr_Cylinder_ptr->set_mask(curr_framenumber+1,pass_number,true);
         cout << "Cylinder t = " << t << " is masked for frame "
              << curr_framenumber << endl;
      } // get_interpolated_statevector conditional
   } // loop over index t labeling tracks
}

// ==========================================================================
// Messenger member functions
// ==========================================================================

// Member function broadcast_dynamic_tracks sends out ActiveMQ
// messages for all vehicles within *dynamic_tracks_group_ptr whose
// broadcast_content_flags == true.

void EarthRegion::broadcast_dynamic_tracks()
{
//   cout << "inside EarthRegion::broadcast_dynamic_tracks()" << endl;
   
   double secs_elapsed_since_epoch=
      AnimationController_ptr->get_time_corresponding_to_curr_frame();
//   cout << "secs_elapsed_since_epoch = "
//        << secs_elapsed_since_epoch << endl;

   vector<int> encountered_vehicle_IDs=
      movers_group_ptr->get_encountered_vehicle_IDs();
//   cout << "encountered_vehicle_IDs.size() = "
//        << encountered_vehicle_IDs.size() << endl;

   cout << "n_dynamic_tracks = " 
        << dynamic_tracks_group_ptr->get_n_tracks() << endl;
   vector<track*> dynamic_track_ptrs=dynamic_tracks_group_ptr->
      get_all_track_ptrs();
   
   for (unsigned int t=0; t<dynamic_track_ptrs.size(); t++)
   {
      const track* curr_track_ptr=dynamic_track_ptrs[t];

// Check whether current dynamic mover has been previously visited
// before by some UAV.  If so, do NOT broadcast fixed site to
// ActiveMQ.  As of Sep 13, 2008, we implement this condition so that
// UAVs are not directed by Michael Yee's travelling salesman
// algorithm to revisit sites which were previously visited by some
// UAV in the past:

      bool vehicle_previously_encountered_flag=false;
      for (unsigned int e=0; e<encountered_vehicle_IDs.size(); e++)
      {
//         cout << "e = " << e 
//              << " encountered_vehicle_IDs[e] = "
//              << encountered_vehicle_IDs[e] << endl;
         if (curr_track_ptr->get_label_ID()==encountered_vehicle_IDs[e])
         {
            vehicle_previously_encountered_flag=true;
            cout << "Vehicle previously encountered!!!" << endl;
         }
      } // loop over e index labeling encountered_vehicle_IDs
      
      if (curr_track_ptr->get_broadcast_contents_flag() &&
          !vehicle_previously_encountered_flag)
      {
         statevector curr_statevector;
         if (curr_track_ptr->get_interpolated_statevector(
            secs_elapsed_since_epoch,curr_statevector))
         {

// As of Jan 22, 2009, we choose to not broadcast any dynamic ground
// target's statevector (e.g. Modified Bluegrass truth car tracks for
// Baghdad multi-UAV demo) which lies outside the LatLongGrid:

            if (LatLongGrid_ptr->point_lies_inside_grid_borders(
               curr_statevector.get_position()))
            {
               curr_track_ptr->broadcast_statevector(
                  curr_statevector,LatLongGrid_ptr->get_world_origin_ptr(),
                  robots_Messenger_ptr);
            }

         } // get_interpolated_statevector conditional
      } // broadcast_contents_flag conditional
   } // loop over index t labeling dynamic tracks
}

// ---------------------------------------------------------------------
// Member function broadcast_spatially_fixed_tracks writes out
// ActiveMQ messages including track ID and statevector information
// for each fixed Region of Interest.

void EarthRegion::broadcast_spatially_fixed_tracks()
{
//   cout << "inside EarthRegion::broadcast_spatially_fixed_tracks()" << endl;
   
   double secs_elapsed_since_epoch=
      AnimationController_ptr->get_time_corresponding_to_curr_frame();
//   cout << "secs_elapsed_since_epoch = "
//        << secs_elapsed_since_epoch << endl;

   cout << "n_spatially_fixed_tracks = " 
        << spatially_fixed_tracks_group_ptr->get_n_tracks() << endl;

   vector<int> encountered_ROI_IDs=
      movers_group_ptr->get_encountered_ROI_IDs();
   
   vector<track*> spatially_fixed_track_ptrs=
      spatially_fixed_tracks_group_ptr->get_all_track_ptrs();
   for (unsigned int t=0; t<spatially_fixed_track_ptrs.size(); t++)
   {
      const track* curr_track_ptr=spatially_fixed_track_ptrs[t];
//      cout << "track ID = " << curr_track_ptr->get_ID() << endl;      

// Check whether current spatially fixed site has been previously
// visited before by some UAV.  If so, do NOT broadcast fixed site to
// ActiveMQ.  As of Sep 13, 2008, we implement this condition so that
// UAVs are not directed by Michael Yee's travelling salesman
// algorithm to revisit sites which were previously visited by some
// UAV in the past:

      bool ROI_previously_encountered_flag=false;
      for (unsigned int e=0; e<encountered_ROI_IDs.size(); e++)
      {
//         cout << "e = " << e 
//              << " encountered_ROI_IDs[e] = "
//              << encountered_ROI_IDs[e] << endl;
         if (curr_track_ptr->get_ID()==encountered_ROI_IDs[e])
         {
            ROI_previously_encountered_flag=true;
//            cout << "ROI previously encountered!!!" << endl;
         }
      } // loop over e index labeing encountered_ROI_IDs
      
      if (curr_track_ptr->get_broadcast_contents_flag() &&
          !ROI_previously_encountered_flag)
      {
         statevector curr_statevector;
         if (curr_track_ptr->get_interpolated_statevector(
            secs_elapsed_since_epoch,curr_statevector))
         {
//            cout << "curr_statevector = " << curr_statevector << endl;
//            threevector origin=*(LatLongGrid_ptr->get_world_origin_ptr());
//            threevector posn=curr_statevector.get_position()-origin;
//            cout << curr_track_ptr->get_ID() << " , "
//                 << curr_statevector.get_time() << " , " 
//                 << posn.get(0) << " , "
//                 << posn.get(1) << " , "
//                 << posn.get(2) << endl;
            
            curr_track_ptr->broadcast_statevector(
               curr_statevector,
               LatLongGrid_ptr->get_world_origin_ptr(),robots_Messenger_ptr);
         } // get_interpolated_statevector conditional
      } // broadcast_contents_flag conditional
   } // loop over index t labeling dynamic tracks
}

// ---------------------------------------------------------------------
// Member function broadcast_KOZ_tracks writes out ActiveMQ messages
// including track ID and bbox information for each Keep Out Zone.

void EarthRegion::broadcast_KOZ_tracks()
{
//   cout << "inside EarthRegion::broadcast_KOZ_tracks()" << endl;
   
   double secs_elapsed_since_epoch=
      AnimationController_ptr->get_time_corresponding_to_curr_frame();
//   cout << "secs_elapsed_since_epoch = "
//        << secs_elapsed_since_epoch << endl;

   cout << "n_KOZ_tracks = " << KOZ_tracks_group_ptr->get_n_tracks() << endl;

   vector<track*> KOZ_track_ptrs=
      KOZ_tracks_group_ptr->get_all_track_ptrs();
   for (unsigned int t=0; t<KOZ_track_ptrs.size(); t++)
   {
      const track* curr_track_ptr=KOZ_track_ptrs[t];
//      cout << "track ID = " << curr_track_ptr->get_ID() << endl;      
      
      if (curr_track_ptr->get_broadcast_contents_flag())
      {
         statevector curr_statevector;
         if (curr_track_ptr->get_interpolated_statevector(
            secs_elapsed_since_epoch,curr_statevector))
         {
//            cout << "curr_statevector = " << curr_statevector << endl;
//            threevector origin=*(LatLongGrid_ptr->get_world_origin_ptr());
//            threevector posn=curr_statevector.get_position()-origin;
//            cout << curr_track_ptr->get_ID() << " , "
//                 << curr_statevector.get_time() << " , " 
//                 << posn.get(0) << " , "
//                 << posn.get(1) << " , "
//                 << posn.get(2) << endl;
            
            curr_track_ptr->broadcast_statevector(
               curr_statevector,
               LatLongGrid_ptr->get_world_origin_ptr(),robots_Messenger_ptr);
         } // get_interpolated_statevector conditional
      } // broadcast_contents_flag conditional
   } // loop over index t labeling dynamic tracks
}

// ==========================================================================
// Region of Interest( ROI) member functions
// ==========================================================================

// Member function generate_nominated_ROI takes in threevector
// vertices for a Region-of-Interest nominated via some manual or
// automatic mechanism (e.g. vehicle speed or Google Earth hot-point
// considerations).  It instantiates a new polyhedral skeleton for the
// ROI.  This method also generates a new ROI member of
// *movers_group_ptr.

mover* EarthRegion::generate_nominated_ROI(
   const vector<threevector>& vertices,colorfunc::Color ROI_PolyLine_color,
   string annotation_label)
{
//   cout << "inside EarthRegion::generate_nominated_ROI()" << endl;
   
   bool force_display_flag=false;
   bool single_polyline_per_geode_flag=true;

   PolyLine* bottom_PolyLine_ptr=ROILinesGroup_ptr->
      generate_new_PolyLine(
         vertices,force_display_flag,single_polyline_per_geode_flag,
         ROILinesGroup_ptr->get_n_text_messages());
//   cout << "bottom_PolyLine_ptr = " << bottom_PolyLine_ptr << endl;

   mover* new_mover_ptr=movers_group_ptr->generate_new_ROI(
      spatially_fixed_tracks_group_ptr);

   int curr_ROI_ID=new_mover_ptr->get_ID();
   string label="ROI "+stringfunc::number_to_string(curr_ROI_ID);
//         cout << "ROI label = " << label << endl;
//         cout << "new_mover_ptr->get_ID() = " << curr_ROI_ID << endl;

   ROILinesGroup_ptr->set_next_PolyLine_mover_ptr(new_mover_ptr);
   ROILinesGroup_ptr->set_next_PolyLine_label(label);
   movers_group_ptr->add_mover_to_outgoing_queue(new_mover_ptr);

//   osg::Vec4 permanent_color=colorfunc::get_OSG_color(colorfunc::white);
   osg::Vec4 permanent_color=colorfunc::get_OSG_color(ROI_PolyLine_color);

   osg::Vec4 selected_color=colorfunc::get_OSG_color(colorfunc::green);
   bool display_vertices_flag=false;
   ROILinesGroup_ptr->form_polyhedron_skeleton(
      bottom_PolyLine_ptr,permanent_color,selected_color,
      display_vertices_flag);

   ROILinesGroup_ptr->issue_add_vertex_message(
      bottom_PolyLine_ptr,ROI_PolyLine_color,annotation_label);

   return new_mover_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_ROI_circle_vertices returns an STL vector
// of threevectors that lie along a cirle of radius ROI_radius from
// the center point specified by input params center_longitude,
// center_latitude and center_altitude.

vector<threevector> EarthRegion::generate_ROI_circle_vertices(
   double center_longitude,double center_latitude,
   double center_altitude,double ROI_radius,int specified_UTM_zonenumber)
{
//   cout << "inside EarthRegion::generate_ROI_circle_vertices()" << endl;
//   cout << "center_altitude = " << center_altitude << endl;
   geopoint ROI_center(center_longitude,center_latitude,center_altitude,
                       specified_UTM_zonenumber);
   threevector origin(
      ROI_center.get_UTM_easting(),ROI_center.get_UTM_northing(),
      ROI_center.get_altitude());

   int n_vertices=10;
   double dtheta=2*PI/double(n_vertices);
   vector<threevector> vertices;
   for (int n=0; n<n_vertices; n++)
   {
      double theta=0+n*dtheta;
      vertices.push_back(
         origin+ROI_radius*threevector(cos(theta),sin(theta),0));
//      cout << "n = " << n
//           << " vertex = " << vertices.back() << endl;
   }

   return vertices;
}

// ---------------------------------------------------------------------
tracks_group* EarthRegion::generate_ROI_tracks_group()
{
   tracks_group* ROI_tracks_group_ptr=new tracks_group();
   ROI_tracks_group_ptrs.push_back(ROI_tracks_group_ptr);
   return ROI_tracks_group_ptrs.back();
}

// ---------------------------------------------------------------------
void EarthRegion::purge_ROI_tracks_group_ptrs()
{
//   cout << "inside EarthRegion::purge_ROI_tracks_group_ptrs()" << endl;
   
   for (unsigned int i=0; i<ROI_tracks_group_ptrs.size(); i++)
   {
      delete ROI_tracks_group_ptrs[i];
   }
   ROI_tracks_group_ptrs.clear();
}

// ---------------------------------------------------------------------
// Member function clear_all_ROIs() purges the tracks within
// *spatially_fixed_tracks_group_ptr, the ROI movers within
// *movers_group_ptr and the entries within movers_group_ptr's
// encountered_ROI_IDs.

void EarthRegion::clear_all_ROIs()
{
   spatially_fixed_tracks_group_ptr->destroy_all_tracks();

   if (ROILinesGroup_ptr != NULL) ROILinesGroup_ptr->destroy_all_PolyLines();

   if (movers_group_ptr != NULL)
   {
      movers_group_ptr->purge_all_particular_movers(mover::ROI);
      movers_group_ptr->get_encountered_ROI_IDs().clear();
   }
}

// ---------------------------------------------------------------------
// Member function check_Cylinder_ROI_intersections() loops over all
// Cylinders within input *CylindersGroup_ptr and extracts their
// current positions.  It checks whether each Cylinder lies inside any
// ROI polyline at the current timestep and did not do so at the
// previous timestep.  If so, this method blinks the ROI polyline and
// adds its ID to the ROI PolyLine's *objects_inside_map_ptr.  When
// the object departs the ROI, its entry inside the STL map is erased.

void EarthRegion::check_Cylinder_ROI_intersections(
   CylindersGroup* CylindersGroup_ptr)
{
//   cout << "inside EarthRegion::check_Cylinder_ROI_intersections()" << endl;
//   cout << "ROILinesGroup_ptr->get_n_Graphicals() = "
//        << ROILinesGroup_ptr->get_n_Graphicals() << endl;

//   cout << "AnimationController_ptr = " << AnimationController_ptr << endl;
   if (AnimationController_ptr==NULL) return;

   int curr_framenumber=AnimationController_ptr->get_curr_framenumber();
   if (curr_framenumber==prev_framenumber)
   {
      return;
   }
   else
   {
      prev_framenumber=curr_framenumber;
   }
//   cout << "curr_framenumber= " << curr_framenumber << endl;
   
   double curr_t=CylindersGroup_ptr->get_curr_t();
   int pass_number=CylindersGroup_ptr->get_passnumber();

   for (unsigned int c=0; c<CylindersGroup_ptr->get_n_Graphicals(); c++)
   {
      Cylinder* curr_Cylinder_ptr=CylindersGroup_ptr->get_Cylinder_ptr(c);
      threevector cyl_posn;
      curr_Cylinder_ptr->get_UVW_coords(curr_t,pass_number,cyl_posn);
      cyl_posn.put(2,0);

      for (unsigned int r=1; r<ROILinesGroup_ptr->get_n_Graphicals(); r += 2)
      {
         PolyLine* PolyLine_ptr=ROILinesGroup_ptr->get_PolyLine_ptr(r);
         bounding_box bbox(PolyLine_ptr->get_bbox());
         if (bbox.point_inside(cyl_posn.get(0),cyl_posn.get(1)))
         {
//            cout << "Cyl c = " << c << " currently lies inside bbox r = "
//                 << r << endl;

// Check if Cylinder previously was inside *PolyLine_ptr.  If so,
// don't bother to blink PolyLine again as an alert:

            PolyLine::OBJECTS_INSIDE_MAP* objects_inside_map_ptr=
               PolyLine_ptr->get_objects_inside_PolyLine_map_ptr();

            bool Cylinder_previously_inside_PolyLine_flag=false;
            PolyLine::OBJECTS_INSIDE_MAP::iterator obj_iter=
               objects_inside_map_ptr->find(r);
            if (obj_iter != objects_inside_map_ptr->end())
            {
//               cout << "Cylinder previously located inside PolyLine"
//                    << endl;
               Cylinder_previously_inside_PolyLine_flag=true;
            }
               
            polygon roi_contour(*(PolyLine_ptr->get_polyline_ptr()));
//            cout << "roi_contour = " << roi_contour << endl;
            polygon roi_contour_XY(roi_contour.xy_projection());
//            cout << "roi_contour_XY = " << roi_contour_XY << endl;
            if (roi_contour_XY.point_inside_polygon(cyl_posn))
            {
//               cout << "Cyl c = " << c 
//                    << " currently lies inside polygon r = " << r << endl;

               if (!Cylinder_previously_inside_PolyLine_flag)
               {
                  cout << "Cyl c = " << c 
                       << " enters SIA = " << (r-1)/2 << endl;
                  (*objects_inside_map_ptr)[r]=1;

                  vector<int> PolyLine_IDs_to_blink;
                  PolyLine_IDs_to_blink.push_back(r);
                  PolyLine_IDs_to_blink.push_back(r-1);
                  const double max_blink_period=20;	// secs
                  ROILinesGroup_ptr->blink_Geometricals(
                     PolyLine_IDs_to_blink,max_blink_period);
               }
            }
            else
            {
               if (Cylinder_previously_inside_PolyLine_flag)
               {
                  cout << "Cyl c = " << c 
                       << " departs SIA = " << (r-1)/2 << endl;
                  objects_inside_map_ptr->erase(obj_iter);
               }
            } // cyl_posn inside roi_contour_XY conditional

         } // point inside bbox conditional
      } // loop over index r labeling ROILines

   } // loop over index c labeling Cylinders
}

// ---------------------------------------------------------------------
// Member function track_Cylinder_with_ROI_Polyhedron() checks whether
// any mover Cylinder has been selected.  If so, it instantiates a ROI
// bbox Polyhedron if bind_ROI_to_selected_Cylinder_flag==true and if
// such a Polyhedron does not already exist.  This method also loops
// through all Cylinders.  It resets the position of any Polyhedron
// which is bound to a Cylinder to the Cylinder's position.

void EarthRegion::track_Cylinder_with_ROI_Polyhedron(
   CylindersGroup* CylindersGroup_ptr,PolyhedraGroup* PolyhedraGroup_ptr,
   bool bind_ROI_to_selected_Cylinder_flag)
{
//   cout << "inside EarthRegion::track_Cylinder_with_ROI_Polyhedron()" 
//        << endl;

//   cout << "AnimationController_ptr = " << AnimationController_ptr << endl;
   if (AnimationController_ptr==NULL) return;

   int curr_framenumber=AnimationController_ptr->get_curr_framenumber();
//   cout << "curr_framenumber = " << curr_framenumber << endl;
//   cout << "prev_framenumber2 = " << prev_framenumber2 << endl;
   if (curr_framenumber==prev_framenumber2)
   {
      return;
   }
   else
   {
      prev_framenumber2=curr_framenumber;
   }
//   cout << "curr_framenumber= " << curr_framenumber << endl;


   Cylinder* selected_Cylinder_ptr=dynamic_cast<Cylinder*>(
      CylindersGroup_ptr->get_selected_Graphical_ptr());
//   cout << "selected_Cylinder_ptr = " << selected_Cylinder_ptr << endl;
   
   if (selected_Cylinder_ptr==NULL) bind_ROI_to_selected_Cylinder_flag=false;

   if (bind_ROI_to_selected_Cylinder_flag)
   {
      double curr_t=CylindersGroup_ptr->get_curr_t();
      int pass_number=CylindersGroup_ptr->get_passnumber();
      threevector cyl_posn;
      selected_Cylinder_ptr->get_UVW_coords(curr_t,pass_number,cyl_posn);
      cyl_posn.put(2,0);
//   cout << "cyl_posn = " << cyl_posn << endl;

      int Polyhedra_subgroup=0;
      string bbox_label="ROI "+stringfunc::number_to_string(
         PolyhedraGroup_ptr->get_n_Graphicals());

      Polyhedron* curr_Polyhedron_ptr=selected_Cylinder_ptr->
         get_Polyhedron_ptr();
      if (curr_Polyhedron_ptr==NULL)
      {
         double alpha=0.1;
         colorfunc::Color bbox_color=colorfunc::orange;
         if (selected_Cylinder_ptr->get_ID() >= 10000)
         {
            bbox_color=colorfunc::blgr;
         }
         curr_Polyhedron_ptr=PolyhedraGroup_ptr->generate_bbox(
            Polyhedra_subgroup,bbox_color,alpha);
         selected_Cylinder_ptr->set_Polyhedron_ptr(curr_Polyhedron_ptr);
         curr_Polyhedron_ptr->set_Cylinder_ptr(selected_Cylinder_ptr);
      }
   }

// Loop over all Cylinders and retrieve their bound Polyhedra bboxes.
// Update bound Polyhedra positions to those of their parent
// Cylinders.

   double curr_t=CylindersGroup_ptr->get_curr_t();
   int pass_number=CylindersGroup_ptr->get_passnumber();
   threevector cyl_posn;   
   for (unsigned int c=0; c<CylindersGroup_ptr->get_n_Graphicals(); c++)
   {
      Cylinder* Cylinder_ptr=CylindersGroup_ptr->get_Cylinder_ptr(c);
      Polyhedron* curr_Polyhedron_ptr=Cylinder_ptr->get_Polyhedron_ptr();
      if (curr_Polyhedron_ptr==NULL) continue;

      Cylinder_ptr->get_UVW_coords(curr_t,pass_number,cyl_posn);
      cyl_posn.put(2,0);
//      cout << "cyl_posn = " << cyl_posn << endl;

      curr_Polyhedron_ptr->set_UVW_coords(
         PolyhedraGroup_ptr->get_curr_t(),
         PolyhedraGroup_ptr->get_passnumber(),cyl_posn);
   } // loop over index c labeling Cylinders
   

}
