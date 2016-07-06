// ==========================================================================
// FUSIONGROUP class member function definitions
// ==========================================================================
// Last modified on 12/25/11; 12/26/11; 2/28/13
// ==========================================================================

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <osg/Image>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "geometry/bounding_box.h"
#include "video/camera.h"
#include "video/camerafuncs.h"
#include "color/colorfuncs.h"
#include "geometry/contour.h"
#include "geometry/convexhull.h"
#include "general/filefuncs.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "filter/filterfuncs.h"
#include "osg/osgFusion/FusionGroup.h"
#include "astro_geo/geopoint.h"
#include "geometry/homography.h"
#include "image/imagefuncs.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "templates/mytemplates.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "numerical/param_range.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "geometry/polyline.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "math/prob_distribution.h"
#include "geometry/regular_polygon.h"
#include "osg/osgSceneGraph/scenegraphfuncs.h"
#include "general/stringfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "math/twovector.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::pair;
using std::setw;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void FusionGroup::allocate_member_objects()
{
}		       

void FusionGroup::initialize_member_objects()
{
   GraphicalsGroup_name="FusionGroup";

   view_draped_video_flag=false;
   orthographic_projection_flag=false;
   byte_counter=0;

   FeaturesGroup_2D_ptr=NULL;
   FeaturesGroup_3D_ptr=NULL;
   Movie_ptr=NULL;
   ObsFrustaGroup_ptr=NULL;
   Aerial_OBSFRUSTAGROUP_ptr=NULL;
   sub_Aerial_OBSFRUSTAGROUP_ptr=NULL;
   Ground_OBSFRUSTAGROUP_ptr=NULL;
   sub_Ground_OBSFRUSTAGROUP_ptr=NULL;
   PassesGroup_ptr=NULL;
   ground_photogroup_ptr=NULL;
   air_photogroup_ptr=NULL;
   PointCloud_ptr=NULL;
   PolyhedraGroup_ptr=NULL;
   PolyLinesGroup_2D_ptr=NULL;
   PolyLinesGroup_3D_ptr=NULL;
   ray_tracer_ptr=NULL;
   RectanglesGroup_ptr=NULL;

   fp_in=NULL;
   gaussian_2D_ptr=NULL;

   color_arrays_filename="draped_color_arrays.rgba";
//   cout << "Enter color arrays filename needed for 4D movie display:" << endl;
//   cin >> color_arrays_filename;

//   pngfunc::RGB_twoDarray.first=NULL;
//   pngfunc::RGB_twoDarray.second=NULL;
//   pngfunc::RGB_twoDarray.third=NULL;

   start_imagenumber=0;
   stop_imagenumber=get_Nimages();

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<FusionGroup>(
         this, &FusionGroup::update_display));
}		       

FusionGroup::FusionGroup(
   Pass* PI_ptr,PointCloud* cloud_ptr,Movie* Movie_ptr,
   AnimationController* AC_ptr,bool view_draped_video,
   bool init_cloud_members_flag):
   GraphicalsGroup(3,PI_ptr,AC_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
   PointCloud_ptr=cloud_ptr;
   this->Movie_ptr=Movie_ptr;
   if (init_cloud_members_flag) initialize_cloud_members();
   view_draped_video_flag=view_draped_video;
}		       

FusionGroup::FusionGroup(
   PassesGroup* PG_ptr,Pass* PI_ptr,
   PointCloudsGroup* PCG_ptr,Movie* Movie_ptr,
   FeaturesGroup* FG2D_ptr,FeaturesGroup* FG3D_ptr,threevector* GO_ptr,
   AnimationController* AC_ptr):
   GraphicalsGroup(3,PI_ptr,AC_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();

   PassesGroup_ptr=PG_ptr;
   PointCloudsGroup_ptr=PCG_ptr;
   PointCloud_ptr=PointCloudsGroup_ptr->get_Cloud_ptr(0);

   this->Movie_ptr=Movie_ptr;
   FeaturesGroup_2D_ptr=FG2D_ptr;
   FeaturesGroup_3D_ptr=FG3D_ptr;
}		       

FusionGroup::FusionGroup(
   PassesGroup* PG_ptr,Pass* PI_ptr,
   PointCloudsGroup* PCG_ptr,Movie* Movie_ptr,
   FeaturesGroup* FG2D_ptr,FeaturesGroup* FG3D_ptr,
   PolyLinesGroup* PLG2D_ptr,PolyLinesGroup* PLG3D_ptr,
   threevector* GO_ptr,AnimationController* AC_ptr):
   GraphicalsGroup(3,PI_ptr,AC_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();

   PassesGroup_ptr=PG_ptr;
   PointCloudsGroup_ptr=PCG_ptr;
   PointCloud_ptr=PointCloudsGroup_ptr->get_Cloud_ptr(0);

   this->Movie_ptr=Movie_ptr;
   FeaturesGroup_2D_ptr=FG2D_ptr;
   FeaturesGroup_3D_ptr=FG3D_ptr;
   PolyLinesGroup_2D_ptr=PLG2D_ptr;
   PolyLinesGroup_3D_ptr=PLG3D_ptr;
}		       

// ---------------------------------------------------------------------
// Member function initialize_cloud_members 

void FusionGroup::initialize_cloud_members()
{

// Recall ladarimage member of PointCloud class is generally NOT
// instantiated.  We need to explicitly do so here:

   PointCloud_ptr->generate_ladarimage();

   ztwoDarray_ptr=PointCloud_ptr->get_ladarimage_ptr()->
      get_z2Darray_ptr();
   filter_size=generate_gaussian_filters();
}

// ---------------------------------------------------------------------
FusionGroup::~FusionGroup()
{
   fclose(fp_in);
   delete gaussian_2D_ptr;

   for (unsigned int n=0; n<gaussian_filter.size(); n++)
   {
      delete gaussian_filter.back();
   }
   delete ObsFrustaGroup_ptr;
   delete Aerial_OBSFRUSTAGROUP_ptr;
   delete sub_Aerial_OBSFRUSTAGROUP_ptr;
   delete Ground_OBSFRUSTAGROUP_ptr;
   delete sub_Ground_OBSFRUSTAGROUP_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const FusionGroup& f)
{
   return(outstream);
}

// ==========================================================================
// Tiepoint fusion methods
// ==========================================================================

// Member function consolidate_XYZ_and_UV_feature_info transfers 3D
// XYZ world-point information from *FeaturesGroup_3D_ptr to
// *FeaturesGroup_2D_ptr.

void FusionGroup::consolidate_XYZ_and_UV_feature_info()
{
   cout << "inside FusionGroup::consolidate_XYZ_and_UV_feature_info()"
        << endl;
   
   outputfunc::write_banner("Consolidating 3D & 2D feature information");

   for (int imagenumber=0; imagenumber < Movie_ptr->get_Nimages(); 
        imagenumber++)
   {
      cout << imagenumber << " " << flush;
      double t=static_cast<double>(imagenumber);
      FeaturesGroup_2D_ptr->consolidate_feature_coords(
         t,0,FeaturesGroup_3D_ptr);
   } // loop over imagenumber index

//   double time_value=10;
//   FeaturesGroup_2D_ptr->write_feature_html_file(time_value);
}

// --------------------------------------------------------------------------
// Member function retrieve_XYZUV_feature_info dynamically generates
// and returns a genmatrix holding consolidated XYZ-UV tiepoint
// information corresponding to input time curr_t.

genmatrix* FusionGroup::retrieve_XYZUV_feature_info(int imagenumber)
{
//   cout << "inside FusionGroup::retrieve_XYZUV_feature_info, imagenumber = "
//        << imagenumber << endl;
//   cout.precision(10);
   
   vector<vector<double> > XYZUV;
   XYZUV.clear();

// Loop over 2D feature information at the current time:

   double t=static_cast<double>(imagenumber);
   for (unsigned int n=0; n<FeaturesGroup_2D_ptr->get_n_Graphicals(); n++)
   {
      Feature* feature_2D_ptr=FeaturesGroup_2D_ptr->get_Feature_ptr(n);
      int ID=feature_2D_ptr->get_ID();

      instantaneous_obs* curr_obs_ptr=feature_2D_ptr->
         get_particular_time_obs(t,FeaturesGroup_2D_ptr->get_passnumber());
      if (curr_obs_ptr != NULL)
      {
         if (curr_obs_ptr->get_npasses() > 1)
         {
            threevector XYZ=curr_obs_ptr->retrieve_UVW_coords(
               PassesGroup_ptr->get_curr_cloudpass_ID());
            threevector UVW=curr_obs_ptr->retrieve_UVW_coords(
               PassesGroup_ptr->get_videopass_ID());
            vector<double> curr_feature;
            curr_feature.push_back(ID);
            curr_feature.push_back(XYZ.get(0));
            curr_feature.push_back(XYZ.get(1));
            curr_feature.push_back(XYZ.get(2));
            curr_feature.push_back(UVW.get(0));
            curr_feature.push_back(UVW.get(1));

//            cout << "ID = " << ID 
//                 <<  " X = " << XYZ.get(0) 
//                 <<  " Y = " << XYZ.get(1) 
//                 <<  " Z = " << XYZ.get(2) 
//                 <<  " U = " << UVW.get(0) 
//                 <<  " V = " << UVW.get(1) << endl;
            XYZUV.push_back(curr_feature);
         }
      } // curr_obs_ptr_ptr != NULL conditional
   } // loop over index n labeling 2D feature number

   genmatrix* XYZUV_ptr=new genmatrix(XYZUV.size(),6);
   for (unsigned int n=0; n<XYZUV.size(); n++)
   {
      XYZUV_ptr->put(n,0,(XYZUV[n])[0]);
      XYZUV_ptr->put(n,1,(XYZUV[n])[1]);
      XYZUV_ptr->put(n,2,(XYZUV[n])[2]);
      XYZUV_ptr->put(n,3,(XYZUV[n])[3]);
      XYZUV_ptr->put(n,4,(XYZUV[n])[4]);
      XYZUV_ptr->put(n,5,(XYZUV[n])[5]);
   }

//   cout << "At end of FusionGroup::retrieve_XYZUV_feature_info, *XYZUV_ptr="
//        << *XYZUV_ptr << endl;
   
   return XYZUV_ptr;
}

// --------------------------------------------------------------------------
// Member function retrieve_XYZABC_polyline_info() dynamically generates
// and returns a genmatrix holding consolidated XYZ-ABC tieline
// information.

genmatrix* FusionGroup::retrieve_XYZABC_polyline_info()
{

   cout << "inside FusionGroup::retrieve_XYZABC_polyline_info" << endl;
   cout.precision(10);
   
   if (PolyLinesGroup_2D_ptr==NULL || PolyLinesGroup_3D_ptr==NULL)
   {
      return NULL;
   }

   vector<vector<double> > XYZABC;
   XYZABC.clear();

// Loop over 2D polyline information at the current time:

   for (unsigned int n=0; n<PolyLinesGroup_2D_ptr->get_n_Graphicals(); n++)
   {
      PolyLine* PolyLine_2D_ptr=PolyLinesGroup_2D_ptr->get_PolyLine_ptr(n);
      int PolyLine_ID=PolyLine_2D_ptr->get_ID();
      PolyLine* PolyLine_3D_ptr=PolyLinesGroup_3D_ptr->
         get_ID_labeled_PolyLine_ptr(PolyLine_ID);

      polyline* polyline_2D_ptr=PolyLine_2D_ptr->get_polyline_ptr();
      polyline* polyline_3D_ptr=PolyLine_3D_ptr->get_polyline_ptr();
      vector<double> twoD_line_coeffs=polyline_2D_ptr->
         get_first_edge_2D_line_coeffs();
      threevector XYZ_0(polyline_3D_ptr->get_vertex(0));
      threevector XYZ_1(polyline_3D_ptr->get_vertex(1));

      vector<double> curr_polyline_info;
      curr_polyline_info.push_back(PolyLine_ID);       		// 0
      curr_polyline_info.push_back(XYZ_0.get(0));		// 1
      curr_polyline_info.push_back(XYZ_0.get(1));		// 2
      curr_polyline_info.push_back(XYZ_0.get(2));		// 3
      curr_polyline_info.push_back(XYZ_1.get(0));		// 4
      curr_polyline_info.push_back(XYZ_1.get(1));		// 5
      curr_polyline_info.push_back(XYZ_1.get(2));		// 6
      curr_polyline_info.push_back(twoD_line_coeffs[0]);      	// 7
      curr_polyline_info.push_back(twoD_line_coeffs[1]);	// 8
      curr_polyline_info.push_back(twoD_line_coeffs[2]);	// 9

//      cout << "ID = " << PolyLine_ID 
//           <<  " X0 = " << XYZ_0.get(0) 
//           <<  " Y0 = " << XYZ_0.get(1) 
//           <<  " Z0 = " << XYZ_0.get(2) << endl;
//      cout <<  " X1 = " << XYZ_1.get(0) 
//           <<  " Y1 = " << XYZ_1.get(1) 
//           <<  " Z1 = " << XYZ_1.get(2) << endl;
//      cout <<  " A = " << twoD_line_coeffs[0]
//           <<  " B = " << twoD_line_coeffs[1]
//           <<  " C = " << twoD_line_coeffs[2] << endl;
      
      XYZABC.push_back(curr_polyline_info);
   } // loop over index n labeling 2D polyline number

   genmatrix* XYZABC_ptr=new genmatrix(2*XYZABC.size(),7);
   for (unsigned int n=0; n<XYZABC.size(); n++)
   {
      XYZABC_ptr->put(2*n,0,(XYZABC[n])[0]);
      XYZABC_ptr->put(2*n,1,(XYZABC[n])[1]);
      XYZABC_ptr->put(2*n,2,(XYZABC[n])[2]);
      XYZABC_ptr->put(2*n,3,(XYZABC[n])[3]);
      XYZABC_ptr->put(2*n,4,(XYZABC[n])[7]);
      XYZABC_ptr->put(2*n,5,(XYZABC[n])[8]);
      XYZABC_ptr->put(2*n,6,(XYZABC[n])[9]);

      XYZABC_ptr->put(2*n+1,0,(XYZABC[n])[0]);
      XYZABC_ptr->put(2*n+1,1,(XYZABC[n])[4]);
      XYZABC_ptr->put(2*n+1,2,(XYZABC[n])[5]);
      XYZABC_ptr->put(2*n+1,3,(XYZABC[n])[6]);
      XYZABC_ptr->put(2*n+1,4,(XYZABC[n])[7]);
      XYZABC_ptr->put(2*n+1,5,(XYZABC[n])[8]);
      XYZABC_ptr->put(2*n+1,6,(XYZABC[n])[9]);
   }

   return XYZABC_ptr;
}

// --------------------------------------------------------------------------
// Member function output_all_tiepoint_data loops over every image and
// saves its XYZ-UV tiepoint data to an individual file located within
// output subdirectory tiepoint_subdir.

void FusionGroup::output_all_tiepoint_data()
{
   outputfunc::write_banner("Outputing tiepoint data:");

   const string tiepoint_subdir="./new_tiepoint_data/";
   filefunc::dircreate(tiepoint_subdir);
   for (int imagenumber=0; imagenumber < Movie_ptr->get_Nimages(); 
        imagenumber++)
   {
      
// As of 6/5/05, we simply set the time associated with each image in
// pass #0 equal to its imagenumber.  This will eventually need to be
// generalized so that the time field corresponds to a true temporal
// measurement...

      double curr_t=static_cast<double>(imagenumber);

      string XYZUVfilename=tiepoint_subdir+
         "XYZUV_"+stringfunc::integer_to_string(imagenumber,4)+".txt";
      write_XYZUV_feature_info(curr_t,XYZUVfilename);
   }
}

// --------------------------------------------------------------------------
// Member function write_XYZUV_feature_info saves XYZUV feature
// information to the output file passed via ofstream outstream.

void FusionGroup::write_XYZUV_feature_info(double curr_t,string XYZUVfilename)
{
   ofstream outstream;
   filefunc::openfile(XYZUVfilename,outstream);

   outstream << "# t = " << curr_t << endl;
   outstream << "#   ID       X	          Y           Z            U           V" 
             << endl;

// Loop over 2D feature information at the current time:

   int n_features_written=0;
   threevector p_origin(get_grid_world_origin());

   for (unsigned int n=0; n<FeaturesGroup_2D_ptr->get_n_Graphicals(); n++)
   {
      Feature* feature_2D_ptr=FeaturesGroup_2D_ptr->get_Feature_ptr(n);
      int ID=feature_2D_ptr->get_ID();

      instantaneous_obs* curr_obs_ptr=feature_2D_ptr->
         get_particular_time_obs(
            curr_t,FeaturesGroup_2D_ptr->get_passnumber());

      if (curr_obs_ptr != NULL)
      {
         if (curr_obs_ptr->get_npasses() > 1)
         {
            threevector XYZ=curr_obs_ptr->retrieve_UVW_coords(0);
            threevector UVW=curr_obs_ptr->retrieve_UVW_coords(1);

            outstream.setf(ios::fixed);
            outstream.setf(ios::showpoint);
            outstream.precision(5);

            outstream << setw(5) << ID << "     "
//                      << setw(10) << XYZ.get(0)-p_origin.get(0)
//                      << setw(10) << XYZ.get(1)-p_origin.get(1)
//                      << setw(10) << XYZ.get(2)-p_origin.get(2)
                      << setw(11) << XYZ.get(0)
                      << setw(11) << XYZ.get(1)
                      << setw(11) << XYZ.get(2)
                      << setw(11) << UVW.get(0)
                      << setw(11) << UVW.get(1)
                      << endl;
               
//               cout << "feature ID = " << ID << endl;
//               cout << "XYZ = " << curr_coords.retrieve_UVW_coords(0)
//                    << endl;
//               cout << "UV = " << curr_coords.retrieve_UVW_coords(1)
//                    << endl;
            n_features_written++;
         }
      } // curr_obs_ptr != NULL conditional
   } // loop over index n labeling 2D feature number
   outstream << endl;

   filefunc::closefile(XYZUVfilename,outstream);

// Delete XYZUV feature file if it's empty:

//   cout << "in FusionGroup::write_XYZUV_feature_info(), curr_t = " << curr_t 
//        << " n_features_written = " << n_features_written << endl;
   
   if (n_features_written==0)
   {
      filefunc::deletefile(XYZUVfilename);
   }
}

// --------------------------------------------------------------------------
// Member function tiepoint_backprojection retrieves XYZUV feature
// information corresponding to the current time and computes the 4x3
// projection matrix that links 3D world-space to 2D image-space.
// Video color is subsequently draped onto the point cloud and
// displayed in the 3D ladar image window.

void FusionGroup::tiepoint_backprojection()
{
   outputfunc::write_banner("Performing tiepoint backprojection:");

   genmatrix* XYZUV_ptr=retrieve_XYZUV_feature_info(get_curr_framenumber());
   camera* curr_camera_ptr=Movie_ptr->get_camera_ptr(get_curr_framenumber());

   if (orthographic_projection_flag)
   {
      curr_camera_ptr->compute_orthographic_tiepoint_projection_matrix(
         XYZUV_ptr);
   }
   else
   {
      curr_camera_ptr->compute_tiepoint_projection_matrix(XYZUV_ptr);
   }
   curr_camera_ptr->check_projection_matrix(XYZUV_ptr);
   delete XYZUV_ptr;

// As of Feb 2006, we can still scene self-shadowing for video imagery
// draping onto ALIRT point clouds:

   backproject_videoframe_onto_pointcloud(get_curr_framenumber());

// Need to take shadowing into account for optical satellite imagery
// draping:

//   backproject_videoframe_onto_pointcloud_with_shadowing(get_curr_framenumber());
//   compute_and_save_shading_factors(get_curr_framenumber());
}

// --------------------------------------------------------------------------
// Member function video_backprojection uses a pre-calculated
// projection matrix to drape the current video frame onto the point
// cloud.  We wrote this method for satellite optical imagery draping
// purposes.

void FusionGroup::video_backprojection()
{
   outputfunc::write_banner("Performing video backprojection:");

   genmatrix* projection_matrix_ptr=
      PassesGroup_ptr->get_videopass_ptr()->
      get_PassInfo_ptr()->get_projection_matrix_ptr();
//   cout << "projection matrix = "
//        << *projection_matrix_ptr << endl;

   camera* curr_camera_ptr=Movie_ptr->get_camera_ptr(get_curr_framenumber());
   curr_camera_ptr->set_projection_matrix(*projection_matrix_ptr);
   backproject_videoframe_onto_pointcloud(get_curr_framenumber());
}

// --------------------------------------------------------------------------
// Member function insert_2D_image_into_3D_worldspace retrieves XYZUV
// feature information corresponding to the current time and computes
// the 4x3 projection matrix that links 3D world-space to 2D
// image-space.  It next instantiates a view frustum which graphically
// displays the camera's field-of-view.  Finally, it inserts an
// alpha-blended 2D image plane into the 3D point cloud at some radial
// displacement relative to the camera's position and with the
// camera's orientation.

void FusionGroup::insert_2D_image_into_3D_worldspace()
{
   cout << "inside FusionGroup::insert_2D_image_into_3D_worldspace()" << endl;

   if (ObsFrustaGroup_ptr==NULL)
   {
      cout << "Error in FusionGroup::insert_2D_image_into_3D_worldspace()"
           << endl;
      cout << "ObsFrustaGroup_ptr=NULL" << endl;
      cout << "Need to set this object's pointer!!!" << endl;
      return;
   }

   outputfunc::write_banner("Inserting 2D image into 3D worldspace:");

   genmatrix* XYZUV_ptr=retrieve_XYZUV_feature_info(get_curr_framenumber());
   genmatrix* XYZABC_ptr=retrieve_XYZABC_polyline_info();

   ObsFrustum* ObsFrustum_ptr=ObsFrustaGroup_ptr->
      generate_movie_ObsFrustum(Movie_ptr->get_video_filename());

//   cout << "Before generating still OBSFRUSTUM" << endl;
//   OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->
//      generate_still_image_OBSFRUSTUM(0,colorfunc::cyan);
//   cout << "After generating still OBSFRUSTUM" << endl;

   Movie* Movie_ptr=ObsFrustum_ptr->get_Movie_ptr();
   camera* curr_camera_ptr=Movie_ptr->get_camera_ptr(get_curr_framenumber());
//   cout << "curr_camera_ptr = " << curr_camera_ptr << endl;

   int nrows_to_skip=0;
//   int nrows_to_skip=1;
   int row_to_skip=-1;
//   for (int row_to_skip=0; row_to_skip < XYZUV_ptr->get_mdim();
//        row_to_skip++)
   {
      curr_camera_ptr->compute_tiepoint_projection_matrix(
         XYZUV_ptr,XYZABC_ptr,nrows_to_skip,row_to_skip);
      curr_camera_ptr->check_projection_matrix(XYZUV_ptr);
      curr_camera_ptr->check_projection_matrix_for_tielines(XYZABC_ptr);
      curr_camera_ptr->print_external_and_internal_params();
      cout << "Calibrated camera = " << *curr_camera_ptr << endl;

//      camerafunc::fit_seven_params(
//         curr_camera_ptr,Movie_ptr->get_aspect_ratio(),
//         XYZUV_ptr,XYZABC_ptr);

//      cout << "row_to_skip = " << row_to_skip << endl;
//      outputfunc::enter_continue_char();

      curr_camera_ptr->compute_fields_of_view(
         Movie_ptr->get_maxU(),Movie_ptr->get_minU(),
         Movie_ptr->get_maxV(),Movie_ptr->get_minV());
      cout << "FOV_u = " << curr_camera_ptr->get_FOV_u()*180/PI << " degs"
           << endl;
      cout << "FOV_v = " << curr_camera_ptr->get_FOV_v()*180/PI << " degs"
           << endl;

   }
   delete XYZUV_ptr;

   vector<threevector> UV_corner_dir=curr_camera_ptr->
      get_UV_corner_world_ray();

//   cout << "camera_world_posn = " << curr_camera_ptr->get_world_posn() 
//        << endl;
//   cout << "curr_framenumber = " << get_curr_framenumber() << endl;
//   for (unsigned int c=0; c<UV_corner_dir.size(); c++)
//   {
//      cout << "c = " << c << " UV_corner_dir[c] = " 
//           << UV_corner_dir[c] << endl;
//   }

   double z_offset=50;			// meters
   double downrange_distance=100;	// meters
   ObsFrustum_ptr->build_frustum_with_movie(
      get_curr_t(),get_passnumber(),z_offset,downrange_distance);
}

// --------------------------------------------------------------------------
// Member function constrained_image_insertion assumes the internal
// parameters for the camera are fixed.  It then loops over the
// camera's extrinsic parameter values and recomputes the 3x4
// projection matrix at each point on a 6D lattice. After performing a
// brute-force chisq minimization, this method prints the best-fit
// projection matrix and instantiates an ObsFrustum within the 3D
// point cloud corresponding to that matrix.

void FusionGroup::constrained_image_insertion()
{
//   cout << "inside FusionGroup::constrained_image_insertion()" << endl;

   genmatrix* XYZUV_ptr=retrieve_XYZUV_feature_info(get_curr_framenumber());

   ObsFrustum* ObsFrustum_ptr=ObsFrustaGroup_ptr->
      generate_movie_ObsFrustum(Movie_ptr->get_video_filename());
   camera* camera_ptr=ObsFrustum_ptr->get_Movie_ptr()->
      get_camera_ptr(get_curr_framenumber());

// NYC piers3.jpg compromise fit internal parameter values determined
// from mosaicing with piers2.jpg:

   double fu=-1.79636363215;
   double fv=-1.72363636785;
   double u0 = 0.610321226453;
   double v0 = 0.534545448221;
   double theta = 90.9045454018;
   double rho = 1;
   
   camera_ptr->set_internal_params(fu,fv,u0,v0,theta);
   camera_ptr->set_rho(rho);

// NYC piers3 best-fit external parameter values determined from
// ladar/EO tiepoint picking:
   
   double x_0 = 584603.4399;
   double y_0 = 4506745.47;
   double z_0 = 42.41162754;

   double az_0 = -169.2152593;
   double el_0 = 1.762265107;
   double roll_0 = -0.6914543779;

   double delta_dist=20;	// meters
   double delta_ang=10;		// degs
   int n_dist_steps=8;
   int n_ang_steps=9;

   param_range x(x_0-delta_dist,x_0+delta_dist,n_dist_steps);
   param_range y(y_0-delta_dist,y_0+delta_dist,n_dist_steps);
   param_range z(z_0-delta_dist,z_0+delta_dist,n_dist_steps);
   param_range az(az_0-delta_ang,az_0+delta_ang,n_ang_steps);
   param_range el(el_0-delta_ang,el_0+delta_ang,n_ang_steps);
   param_range roll(roll_0-delta_ang,roll_0+delta_ang,n_ang_steps);
   
   double min_chisq=POSITIVEINFINITY;
//   const int n_iters=2;
//   const int n_iters=5;
   const int n_iters=10;
   for (int iter=0; iter<n_iters; iter++)
   {
      outputfunc::newline();
      cout << "Iteration = " << iter << endl;

      while (x.prepare_next_value())
      {
         while (y.prepare_next_value())
         {
            while (z.prepare_next_value())
            {
               camera_ptr->set_world_posn(threevector(
                  x.get_value(),y.get_value(),z.get_value()));
               while (az.prepare_next_value())
               {
                  while (el.prepare_next_value())
                  {
                     while (roll.prepare_next_value())
                     {
                        camera_ptr->set_Rcamera(
                           az.get_value()*PI/180,
                           el.get_value()*PI/180,
                           roll.get_value()*PI/180);
                        camera_ptr->construct_projection_matrix_for_fixed_K();
                        double curr_chisq=
                           camera_ptr->fast_check_projection_matrix(
                              XYZUV_ptr);
                        if (curr_chisq < min_chisq)
                        {
                           min_chisq=curr_chisq;
                           x.set_best_value();
                           y.set_best_value();
                           z.set_best_value();
                           az.set_best_value();
                           el.set_best_value();
                           roll.set_best_value();
                        } 
                     }	 // roll while loop
                  } // el while loop
               } // az while loop
            } // z while looop
         } // y while loop
      } // x while loop
      
      outputfunc::newline();
      cout << "best x = " << x.get_best_value() << endl;
      cout << "best y = " << y.get_best_value() << endl;
      cout << "best z = " << z.get_best_value() << endl;
      cout << "best az = " << az.get_best_value() << endl;
      cout << "best el = " << el.get_best_value() << endl;
      cout << "best roll = " << roll.get_best_value() << endl;
      cout << "min_chisq = " << min_chisq << endl;

      camera_ptr->set_world_posn(threevector(
         x.get_best_value(),y.get_best_value(),z.get_best_value()));
      camera_ptr->set_Rcamera(
         az.get_best_value()*PI/180,
         el.get_best_value()*PI/180,
         roll.get_best_value()*PI/180);
      camera_ptr->construct_projection_matrix(false);
      cout << "Projection matrix = " 
           << *(camera_ptr->get_P_ptr()) << endl;

      double frac=0.45;
      x.shrink_search_interval(x.get_best_value(),frac);
      y.shrink_search_interval(y.get_best_value(),frac);
      z.shrink_search_interval(z.get_best_value(),frac);
      az.shrink_search_interval(az.get_best_value(),frac);
      el.shrink_search_interval(el.get_best_value(),frac);
      roll.shrink_search_interval(roll.get_best_value(),frac);

   } // loop over iteration loop
   
   delete XYZUV_ptr;

   vector<threevector> UV_corner_dir=camera_ptr->
      get_UV_corner_world_ray();

   double z_offset=50;			// meters
   double downrange_distance=100;	// meters
   ObsFrustum_ptr->build_frustum_with_movie(
      get_curr_t(),get_passnumber(),z_offset,downrange_distance);
}

// ==========================================================================
// Pointcloud coloring methods
// ==========================================================================

// Member function compute_backprojected_color_arrays() loops over all
// video images from start_imagenumber to stop_imagenumber and uses
// 3x4 projection matrices derived from tiepoints to backproject video
// color onto the point cloud.  It then saves each image's color array
// into a binary output file for later, faster playback purposes.

void FusionGroup::compute_backprojected_color_arrays()
{
   outputfunc::write_banner("Computing backprojected color arrays:");

   FILE* fp_out = fopen(color_arrays_filename.c_str(), "wb");
   for (int imagenumber=start_imagenumber; imagenumber < stop_imagenumber;
        imagenumber++)
   {
      cout << imagenumber << " " << flush;

      genmatrix* XYZUV_ptr=retrieve_XYZUV_feature_info(imagenumber);
      Movie_ptr->get_camera_ptr(get_curr_framenumber())->
         compute_tiepoint_projection_matrix(XYZUV_ptr);
      delete XYZUV_ptr;

      backproject_videoframe_onto_pointcloud(imagenumber);

// Note: As of 5/11/06, we are trying to abandon our earlier approach
// of storing all color information within a monolithic color array
// and instead incorporate it into the Data Scene Graph tree.  So the
// following line will someday have to be replaced by something much
// more complicated that scans through the tree's visible leaf nodes
// and retrieves their color information prior to writeout:

      fwrite( &((*PointCloud_ptr->get_colors_ptr())[0]),
              get_color_array_size_in_bytes(),1,fp_out);

   } // loop over image numbers
   cout << endl;
   fclose(fp_out);
}

// ---------------------------------------------------------------------
// Member function backproject_videoframe_onto_pointcloud assumes that
// the 3x4 projection matrix corresponding to the input
// video_imagenumber has already been computed prior to this method
// being called.  It uses that matrix stored within
// *(Movie_ptr->get_camera_ptr(imagenumber)) to backproject every XYZ
// point in the cloud onto its counterpart video UV pixel.  It
// subsequently stores the RGB color information into the
// osg::Vec4ubArray* colors member of the point cloud.  This method
// works well for draping video onto ALIRT imagery.

void FusionGroup::backproject_videoframe_onto_pointcloud(
   int video_imagenumber,double s_weight)
{
   cout << "inside FusionGroup::backproject_videoframe_onto_pointcloud()"
        << endl;

   string banner="Backprojecting video image "+stringfunc::number_to_string(
      video_imagenumber)+" onto point cloud:";
   outputfunc::write_banner(banner);

   Movie_ptr->displayFrame(video_imagenumber);
   camera* curr_camera_ptr=Movie_ptr->get_camera_ptr(video_imagenumber);
//   cout << "curr_camera = " << *curr_camera_ptr << endl;

// Base PointCloud's default coloring upon height:

   PointCloud_ptr->get_z_ColorMap_ptr()->set_dependent_var(2);	
//   cout << "Colormap number = " << PointCloud_ptr->get_z_ColorMap_ptr()->
//      get_map_number() << endl;
//   cout << "min_z_threshold = " 
//        << PointCloud_ptr->get_z_ColorMap_ptr()->get_min_threshold(2)
//        << endl;
//   cout << "max_z_threshold = " 
//        << PointCloud_ptr->get_z_ColorMap_ptr()->get_max_threshold(2)
//        << endl;

   const unsigned char alpha_byte=
      static_cast<unsigned char>(stringfunc::ascii_integer_to_char(255));
   double U,V;
//   double s_weight=0.6;
//   cout << "Enter saturation weight:" << endl;
//   cin >> s_weight;

   int g=0;
   for (osg::Geometry* curr_Geometry_ptr=PointCloud_ptr->
           get_curr_geometry(); curr_Geometry_ptr != NULL; 
        curr_Geometry_ptr=PointCloud_ptr->get_next_geometry())
   {
      if (g%100==0) cout << g/100 << " " << flush;
      g++;
      
      osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
         curr_Geometry_ptr->getVertexArray());

      osg::Vec4ubArray* curr_colors_ptr=dynamic_cast<osg::Vec4ubArray*>(
         curr_Geometry_ptr->getColorArray());

      if (curr_colors_ptr==NULL)
      {
         int n_vertices=curr_vertices_ptr->size();
         bool fill_color_array=true;
         curr_colors_ptr=scenegraphfunc::instantiate_color_array(
            n_vertices,fill_color_array,curr_Geometry_ptr,
            scenegraphfunc::get_mutable_colors_label());
         curr_Geometry_ptr->setColorArray(curr_colors_ptr);
      }

// Added these next few lines on December 18, 2007 for satellite
// shadowing compensation purposes:

      model::Metadata* curr_metadata_ptr=model::getMetadataForGeometry(
         *curr_Geometry_ptr);
      for (int i=0; i<int(curr_vertices_ptr->size()); i++)
      {
         bool vertex_in_shadow=false;
         if (curr_metadata_ptr != NULL)
         {
            double curr_p=curr_metadata_ptr->get(i,0);
            if (nearly_equal(curr_p,0)) vertex_in_shadow=true;
         }
         
         curr_camera_ptr->project_XYZ_to_UV_coordinates(
            curr_vertices_ptr->at(i).x(),curr_vertices_ptr->at(i).y(),
            curr_vertices_ptr->at(i).z(),U,V);

// If current 3D XYZ point projects into 2D UV plane outside current
// video image's borders, take RGB color to be that set by height
// color map:

         cout << "U = " << U << " V = " << V << endl;
         if (U < Movie_ptr->get_minU() || U > Movie_ptr->get_maxU() ||
             V < Movie_ptr->get_minV() || V > Movie_ptr->get_maxV())
         {

// Recall PointCloud's default coloring is according to height.  We
// need the next line only if the PointCloud's colors vary over time:

            double curr_z=curr_vertices_ptr->at(i).z();
            curr_colors_ptr->at(i)=
               PointCloud_ptr->get_z_ColorMap_ptr()->retrieve_curr_color(
                  curr_z);
         }
         else
         {
            int R,G,B;
            Movie_ptr->get_RGB_values(U,V,R,G,B);

            if (vertex_in_shadow)
            {
               R=G=0;
               B=255;
            }

// Commented out next section for satellite fusion purposes on
// December 27, 2007

// ........................................................................
// Next several lines are for fusing Boston ALIRT and EO imagery.
// Comment out these lines when draping HAFB G99 video imagery onto
// HAFB death pass...

            osg::Vec4ub curr_vec4ub=
               PointCloud_ptr->get_z_ColorMap_ptr()->retrieve_curr_color(
                  curr_vertices_ptr->at(i).z());

            double Rcloud=static_cast<double>(curr_vec4ub.r());
            double Gcloud=static_cast<double>(curr_vec4ub.g());
            double Bcloud=static_cast<double>(curr_vec4ub.b());

            cout << "Rcloud = " << Rcloud << " Gcloud = " << Gcloud
                 << " Bcloud = " << Bcloud << endl;
            
//            HSV_fuse(Rcloud,Gcloud,Bcloud,R,G,B);
            HSV_fuse(Rcloud,Gcloud,Bcloud,s_weight,R,G,B);
//            cout << "R = " << R << " G = " << G << " B = " << B << endl;

// ........................................................................

            curr_colors_ptr->at(i)=osg::Vec4ub(
               static_cast<unsigned char>(static_cast<unsigned int>(R)),
               static_cast<unsigned char>(static_cast<unsigned int>(G)),
               static_cast<unsigned char>(static_cast<unsigned int>(B)),
               alpha_byte);
         } // UV point outside video image borders conditional
      } // loop over index i labeling vertices in curr Geometry
   } // loop over index g labeling all vertex geometries  in *DataGraph_ptr
   cout << endl;
}

// ---------------------------------------------------------------------
// Member function compute_and_save_shading_factors calculates the dot
// product between local surface normals and the range direction
// vector which points from the camera towards the 3D target.  It
// saves the negative of this dotproduct within an output binary file.
// If the output value is negative, the local surface is not in view
// of the illuminator.  [Note: The local surface may also not be in
// view even when the output value equals unity, for it may be
// shadowed by some other surface further upstream towards the
// illuminator...]

void FusionGroup::compute_and_save_shading_factors(int video_imagenumber)
{
   if (PointCloud_ptr->get_normals_ptr() != NULL)
   {
      string banner="Computing shading factors:";
      outputfunc::write_banner(banner);

      Movie_ptr->displayFrame(video_imagenumber);
      camera* curr_camera_ptr=Movie_ptr->get_camera_ptr(video_imagenumber);
      threevector r_hat=-curr_camera_ptr->get_What();

      vector<float>* shading_factor_ptr=new vector<float>;
      for (int i=0; i<get_cloud_npoints(); i++)
      {
         float dotproduct=static_cast<float>(r_hat.dot((
            *(PointCloud_ptr->get_normals_ptr()))[i]));
         shading_factor_ptr->push_back(-dotproduct);
      }

      string shading_filename="shading_factors.p";
      filefunc::deletefile(shading_filename);
      xyzpfunc::write_p_data(shading_filename,shading_factor_ptr,false);
      delete shading_factor_ptr;
   } // normals_ptr != NULL conditional
   else
   {
      cout << "No normals stored within point cloud" << endl;
      cout << "So cannot compute and store shading factors" << endl << endl;
   }
}

// ---------------------------------------------------------------------
// Member function
// backproject_videoframe_onto_pointcloud_with_shadowing generalizes
// the previous backproject_videoframe_onto_pointcloud inasmuch as it
// takes target self-shadowing into account.  We developed this method
// in order to drape optical imagery onto satellite point clouds.

void FusionGroup::backproject_videoframe_onto_pointcloud_with_shadowing(
   int video_imagenumber)
{
   string banner="Backprojecting optical image "+stringfunc::number_to_string(
      video_imagenumber)+" onto point cloud:";
   outputfunc::write_banner(banner);

   Movie_ptr->displayFrame(video_imagenumber);
   camera* curr_camera_ptr=Movie_ptr->get_camera_ptr(video_imagenumber);

// Use range direction vector pointing from camera to 3D target to
// determine which XYZ points lie in shadow:

   threevector r_hat=-curr_camera_ptr->get_What();
   PointCloud_ptr->generate_shadow_map(r_hat);
   PointCloud_ptr->determine_shaded_points(r_hat);

   const bool pure_video_draping_flag=true;
//   const bool pure_video_draping_flag=false;

   double s_weight=0.1;	// works well for SPASE ISAR and optical imagery
//   cout << "Enter saturation weight:" << endl;
//   cin >> s_weight;

   const unsigned char alpha_byte=
      static_cast<unsigned char>(stringfunc::ascii_integer_to_char(255));
//   vector<bool>* shadows_ptr=PointCloud_ptr->get_shadows_ptr();

   double U,V;

   int g=0;
   for (osg::Geometry* curr_Geometry_ptr=PointCloud_ptr->
           get_curr_geometry(); curr_Geometry_ptr != NULL; 
        curr_Geometry_ptr=PointCloud_ptr->get_next_geometry())
   {
      if (g%100==0) cout << g/100 << " " << flush;
      g++;

      osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
         curr_Geometry_ptr->getVertexArray());
      osg::Vec4ubArray* curr_colors_ptr=dynamic_cast<osg::Vec4ubArray*>(
         curr_Geometry_ptr->getColorArray());
      model::Metadata* curr_metadata_ptr=model::getMetadataForGeometry(
         *curr_Geometry_ptr);

      for (int i=0; i<int(curr_vertices_ptr->size()); i++)
      {
         curr_camera_ptr->project_XYZ_to_UV_coordinates(
            curr_vertices_ptr->at(i).x(),curr_vertices_ptr->at(i).y(),
            curr_vertices_ptr->at(i).z(),U,V);


// If current 3D XYZ point projects into 2D UV plane outside current
// video image's borders, take RGB color to be that set by height
// color map:

         if (U > Movie_ptr->get_minU() || U < Movie_ptr->get_maxU() ||
             V > Movie_ptr->get_minV() || V < Movie_ptr->get_maxV())
         {
//            if ( !(*shadows_ptr)[j] )
            {
               int R,G,B;
               Movie_ptr->get_RGB_values(U,V,R,G,B);

// Reset intensity for current voxel equal to the grey scale value
// derived from RGB.  This can act as a sentinel indicating the voxel
// has been colored:

               if (pure_video_draping_flag) 
               {
                  double h,s,v;
                  colorfunc::RGB_to_hsv(R/255.0,G/255.0,B/255.0,h,s,v);
                  curr_metadata_ptr->set(i,0,v);
               }
               else
               {
                  osg::Vec4ub curr_vec4ub=curr_colors_ptr->at(i);
                  double Rcloud=static_cast<double>(curr_vec4ub.r());
                  double Gcloud=static_cast<double>(curr_vec4ub.g());
                  double Bcloud=static_cast<double>(curr_vec4ub.b());
                  HSV_fuse(Rcloud,Gcloud,Bcloud,s_weight,R,G,B);
               }

               curr_colors_ptr->at(i)=osg::Vec4ub(
                  static_cast<unsigned char>(static_cast<unsigned int>(R)),
                  static_cast<unsigned char>(static_cast<unsigned int>(G)),
                  static_cast<unsigned char>(static_cast<unsigned int>(B)),
                  alpha_byte);

            } // XYZ point not in shadow conditional
         } // UV point inside video image borders conditional
      } // loop over index i labeling vertices in curr Geometry
   } // loop over index g labeling all vertex geometries  in *DataGraph_ptr
   cout << endl;
}

// --------------------------------------------------------------------------
// Member function HSV_fuse correlates a point's hue and intensity
// value with two images generated by two distinctly different sensors
// (e.g. ISAR and optical).  It also sets the fused saturation equal
// to a linear combination of the two inputs.  This approach yields
// reasonable results for joint ISAR/optical satellite images.

void FusionGroup::HSV_fuse(double Rcloud,double Gcloud,double Bcloud,
                           double saturation_weight,int& R,int& G,int& B)
{
   double cloud_r=Rcloud/255.0;
   double cloud_g=Gcloud/255.0;
   double cloud_b=Bcloud/255.0;
   double cloud_h,cloud_s,cloud_v;
   colorfunc::RGB_to_hsv(cloud_r,cloud_g,cloud_b,cloud_h,cloud_s,cloud_v);

   double video_r=R/255.0;
   double video_g=G/255.0;
   double video_b=B/255.0;
   double video_h,video_s,video_v;
   colorfunc::RGB_to_hsv(video_r,video_g,video_b,video_h,video_s,video_v);

//   const double saturation_weight=0.1; 
   // OK for SPASE PNG photos 3 and 8 + Hyrum's 9 image ISAR composite

//   const double min_s=0.5;
   double s_avg=saturation_weight*video_s+(1-saturation_weight)*cloud_s;
//   s_avg=basic_math::max(s_avg,min_s);

//   cout << "cloud_s = " << cloud_s << " video_s = " << video_s
//        << " s_weight = " << saturation_weight 
//        << " s_avg = " << s_avg << endl;

   double fused_r,fused_g,fused_b;
//   colorfunc::hsv_to_RGB(cloud_h,1,video_v,fused_r,fused_g,fused_b);
//   colorfunc::hsv_to_RGB(cloud_h,0,video_v,fused_r,fused_g,fused_b);
   colorfunc::hsv_to_RGB(cloud_h,s_avg,video_v,fused_r,fused_g,fused_b);

   R=basic_math::round(fused_r*255);
   G=basic_math::round(fused_g*255);
   B=basic_math::round(fused_b*255);

//   cout << "cloud_v = " << cloud_v << " R = " << R << " G = " << G
//        <<  " B = " << B << endl;

//   cout << "cloud_r = " << red << " cloud_g = " << green
//        << " cloud_b = " << blue << " cloud_h = " << cloud_h 
//        << " new_cloud_h = " << new_cloud_h << endl;
} 

void FusionGroup::HSV_fuse(double Rcloud,double Gcloud,double Bcloud,
                           int& R,int& G,int& B)
{
   double cloud_r=Rcloud/255.0;
   double cloud_g=Gcloud/255.0;
   double cloud_b=Bcloud/255.0;
   double cloud_h,cloud_s,cloud_v;
   colorfunc::RGB_to_hsv(cloud_r,cloud_g,cloud_b,cloud_h,cloud_s,cloud_v);

   double video_r=R/255.0;
   double video_g=G/255.0;
   double video_b=B/255.0;
   double video_h,video_s,video_v;
   colorfunc::RGB_to_hsv(video_r,video_g,video_b,video_h,video_s,video_v);

   double fused_r,fused_g,fused_b;
//   colorfunc::hsv_to_RGB(cloud_h,video_s,video_v,fused_r,fused_g,fused_b);
   colorfunc::hsv_to_RGB(cloud_h,video_s,cloud_v,fused_r,fused_g,fused_b);

   R=basic_math::round(fused_r*255);
   G=basic_math::round(fused_g*255);
   B=basic_math::round(fused_b*255);
} 

// ---------------------------------------------------------------------
// Member function color_pointcloud_with_next_videoframe

void FusionGroup::color_pointcloud_with_next_videoframe()
{
   color_pointcloud(get_curr_framenumber());
}

void FusionGroup::color_pointcloud_with_prev_videoframe()
{
   color_pointcloud(get_curr_framenumber()-2);
}

// ---------------------------------------------------------------------
// Member function color_pointcloud reads a block of color array
// information from a binary file into the start of the point cloud
// color's Vec4ubArray via a fast fread() command.

bool FusionGroup::color_pointcloud(int video_imagenumber)
{
//   cout << "inside FG::color_pointcloud, video_imagenumber = "
//        << video_imagenumber << endl;

   byte_counter = get_color_array_size_in_bytes() * 
      (video_imagenumber-start_imagenumber);
//   cout << "byte_counter = " << byte_counter << endl;
//   cout << "get_color_array_size_in_bytes() = "
//        << get_color_array_size_in_bytes() << endl;

   if (fp_in==NULL) fp_in = fopen(color_arrays_filename.c_str(), "rb");

//   cout << "fp_in = " << fp_in << endl;
//   cout << "color_arrays_filename = " << color_arrays_filename << endl;
   if (fp_in != NULL)
   {
      fseeko(fp_in,byte_counter,SEEK_SET);

// On 9/27/05, Hyrum taught us that the colors UByte4Array class
// (which sits inside of the PointCloud class) is generally LARGER
// than its STL vector which holds all the UByte4s.  So we CANNOT
// simply pass PointCloud_ptr->get_colors_ptr() as the address where
// data should be stored after being read in from the outside file.
// Instead, we need to explicitly pass the address of the 0th element
// of the STL vector as the starting location of the RGBA array:

// Note: As of 5/11/06, we are trying to abandon our earlier approach
// of storing all color information within a monolithic color array
// and instead incorporate it into the Data Scene Graph tree.  So the
// following line will someday have to be replaced by something much
// more complicated which places input color information into the
// tree's visible leaf nodes:

//      cout << "PointCloud_ptr->get_colors_ptr() = "
//           << PointCloud_ptr->get_colors_ptr() << endl;

      if(fread( &((*PointCloud_ptr->get_colors_ptr())[0]),
                get_color_array_size_in_bytes(),1,fp_in) == 0)
      {
         cout << "No input read within FusionGroup::color_pointcloud()"
              << endl;
      }
      

//   PointCloud_ptr->check_colors_array();
   }
   
//   cout << "At end of FG::color_pointcloud()" << endl;
   return true;
}

// ==========================================================================
// Video imagery scoring methods
// ==========================================================================

// Member function generate_gaussian_filters instantiates a zeroth,
// first and second derivative order gaussian filter whose widths are
// set by the ladarimage ztwoDarray's delta_x spacing.  We try to keep
// these widths as small as possible for speed purposes.  Yet they
// must be large enough to yield reasonable estimates for first and
// second image intensity derivatives.

int FusionGroup::generate_gaussian_filters()
{
   outputfunc::write_banner("Generating gaussian filters:");

   double delta=ztwoDarray_ptr->get_deltax();
   double sigma=1.5;	// meter
//   double sigma=1.0*delta;
   const double e_folding_distance=4;

// First generate 2D gaussian filter to supply averaging weights in
// get_backprojected_video_intensity() to help counteract temporal
// noise fluctuations:

   gaussian_2D_ptr=filterfunc::gaussian_2D_filter(
      delta,sigma,e_folding_distance);

// Next generate 1D gaussian filter for intensity derivative
// evaluation purposes:

   int nsize=filterfunc::gaussian_filter_size(
      sigma,delta,e_folding_distance);

   double* filter0_ptr=new double[nsize];
   double* filter1_ptr=new double[nsize];
   double* filter2_ptr=new double[nsize];

   filterfunc::gaussian_filter(nsize,0,sigma,delta,filter0_ptr);
   filterfunc::gaussian_filter(nsize,1,sigma,delta,filter1_ptr);
   filterfunc::gaussian_filter(nsize,2,sigma,delta,filter2_ptr);
   gaussian_filter.push_back(filter0_ptr);
   gaussian_filter.push_back(filter1_ptr);
   gaussian_filter.push_back(filter2_ptr);
   
//   for (int n=0; n<nsize; n++)
//   {
//      cout << "n = " << n 
//           << " filter0 = " << (gaussian_filter[0])[n]
//           << " filter1 = " << (gaussian_filter[1])[n]
//           << " filter2 = " << (gaussian_filter[2])[n] << endl;
//   }
   return nsize;
}

// ---------------------------------------------------------------------
// Member function 

void FusionGroup::test_score()
{
   int init_curr_imagenumber=get_curr_framenumber();
   int curr_imagenumber=init_curr_imagenumber;

   int n_image_pairs=1;
   cout << "Enter number of images to refine KLT features:" << endl;
   cin >> n_image_pairs;
   int n_iters=2;
   cout << "Enter number of refinement iterations to perform per image pair:"
        << endl;
   cin >> n_iters;
   for (int next_imagenumber=init_curr_imagenumber+1; next_imagenumber <
           init_curr_imagenumber+1+n_image_pairs; next_imagenumber++)
   {
      for (int iter=0; iter<n_iters; iter++)
      {
         double RMS_UV_diff=adjust_backprojected_feature_locations(
            curr_imagenumber,next_imagenumber);
         cout << "curr_imagenumber = " << curr_imagenumber 
              << " iter = " << iter << " RMS_UV_diff = " << RMS_UV_diff 
              << endl;
      } // loop over iter index
      if (curr_imagenumber  != init_curr_imagenumber)
      {
         for (int iter=0; iter<n_iters; iter++)
         {
            double RMS_UV_diff=adjust_backprojected_feature_locations(
               init_curr_imagenumber,next_imagenumber);
            cout << "init_curr_imagenumber = " << init_curr_imagenumber 
                 << " iter = " << iter << " RMS_UV_diff = " << RMS_UV_diff 
                 << endl;
         } // loop over iter index
      }

      curr_imagenumber=next_imagenumber;
   } // loop over next_imagenumber index
}

/*
void FusionGroup::test_score()
{
   int curr_imagenumber=get_curr_framenumber();
   int next_imagenumber;
   cout << "Enter next imagenumber:" << endl;
   cin >> next_imagenumber;

   const int n_iters=2;
   for (int iter=0; iter<n_iters; iter++)
   {
//      match_plaquettes(
//         curr_imagenumber,next_imagenumber);
      double RMS_UV_diff=adjust_backprojected_feature_locations(
         curr_imagenumber,next_imagenumber);
      cout << "iter = " << iter << " RMS_UV_diff = " << RMS_UV_diff << endl;
   }
}
*/

// ---------------------------------------------------------------------
// Member function get_backprojected_video_intensity takes in a video
// imagenumber as well as ** 3D world-space ** coordinates X and Y.
// We assume that a 3x4 projection matrix which maps from XYZ to UV
// coordinates has already been previously calculated and stored for
// the input imagenumber within
// *(Movie_ptr->get_camera_ptr(imagenumber)). If the input X,Y
// coordinates map onto a valid (U,V) location in the video image, a
// weighted average greyscale intensity is computed in the pixel
// neighborhood [px-delta_p <= qx <= px+delta_p, py-delta_p <= qy <=
// py+delta_p].  The averaged result ranging from 0 - 255 is returned.
// Otherwise, this method returns -1.

int FusionGroup::get_backprojected_video_intensity(
   const camera* curr_camera_ptr,double X,double Y)
{
   unsigned int px,py;
   if (PointCloud_ptr->xypoint_to_pixel(X,Y,px,py))
   {
      double Z=ztwoDarray_ptr->get(px,py);
      const double SMALL=0.001;
      if (Z > get_grid_world_origin().get(2)-SMALL)
      {
         double U,V;
         curr_camera_ptr->project_XYZ_to_UV_coordinates(X,Y,Z,U,V);
         double curr_I=Movie_ptr->get_intensity(U,V);

//      if (curr_I < 0)
//      {
//         threevector origin(get_grid_world_origin());
//         cout << "inside FusionGroup::get_projected_video_intensity()" 
//              << endl;
//         cout << "X = " << X-origin.get(0) 
//              << " Y = " << Y-origin.get(1) 
//              << " Z = " << Z-origin.get(2) << endl;
//         cout << "U = " << U << " V = " << V << endl;
//         cout << "curr_I = " << curr_I << endl;
//      }
         return static_cast<int>(curr_I);
      } // Z > world_origin.get(2) conditional
   } // xypoint_to_pixel() conditional

//   cout << "at end of FusionGroup::get_projected_video_intensity()" << endl;
//   cout << "X = " << X << " Y = " << Y << endl;
//   cout << "px = " << px << " mdim = " << ztwoDarray_ptr->get_mdim()
//   << endl;
//   cout << "py = " << py << " ndim = " << ztwoDarray_ptr->get_ndim()
//   << endl;
   
   return -1;
}

double FusionGroup::get_avg_backprojected_video_intensity(
   const camera* curr_camera_ptr,double X,double Y)
{
   const int half_mdim=gaussian_2D_ptr->get_mdim()/2;
   const int half_ndim=half_mdim;
   const int delta_p=half_mdim;
   const threevector grid_origin(get_grid_world_origin());
   
   unsigned int px,py;
   if (PointCloud_ptr->xypoint_to_pixel(X,Y,px,py))
   {
      double numer=0;
      double denom=0;
      for (unsigned int qx=basic_math::max(Unsigned_Zero,px-delta_p); 
           qx<=basic_math::min(ztwoDarray_ptr->get_mdim(),px+delta_p); qx++)
      {
         for (unsigned int qy=basic_math::max(Unsigned_Zero,py-delta_p); 
              qy<=basic_math::min(ztwoDarray_ptr->get_ndim(),py+delta_p); qy++)
         {
            const double SMALL=0.001;
            double curr_Z=ztwoDarray_ptr->get(qx,qy);
            if (curr_Z > grid_origin.get(2)-SMALL)
            {
               double curr_X,curr_Y,curr_U,curr_V;
               ztwoDarray_ptr->pixel_to_point(qx,qy,curr_X,curr_Y);
               curr_camera_ptr->project_XYZ_to_UV_coordinates(
                  curr_X,curr_Y,curr_Z,curr_U,curr_V);
               double curr_I=Movie_ptr->get_intensity(curr_U,curr_V);

               double weight=gaussian_2D_ptr->get(
                  half_mdim+qx-px,half_ndim+qy-py);

               numer += weight*curr_I;
               denom += weight;

//               cout << "qx = " << qx << " qy = " << qy
//                    << " half_mdim+qx-px = " << half_mdim+qx-px
//                   << " half_ndim+qy-py = " << half_ndim+qy-py 
//                   << " w = " << weight << endl;
//               cout << "curr_I = " << curr_I << " numer = " << numer
//                    << " denom = " << denom << endl;
               
            } // sanity check on Z value conditional
         } // loop over qy index
      } // loop over qx index
      double avg_intensity=numer/denom;

      if (avg_intensity < 0)
      {
         cout << "inside FusionGroup::get_projected_video_intensity()" 
              << endl;
         cout << "X = " << X << " Y = " << Y << endl;
         cout << "Avg_intensity = " << avg_intensity << endl;
      }
      
      return avg_intensity;
   }  // xypoint_to_pixel(X,Y,px,py))
   cout << "at end of FusionGroup::get_projected_video_intensity()" << endl;
   cout << "X = " << X << " Y = " << Y << endl;
   cout << "px = " << px << " mdim = " << ztwoDarray_ptr->get_mdim() << endl;
   cout << "py = " << py << " ndim = " << ztwoDarray_ptr->get_ndim() << endl;
   
   return -1;
}

// ---------------------------------------------------------------------
// Member functions backprojected_video_intensity_X[Y]deriv takes in
// camera object *curr_camera_ptr which is assumed to contain a 3x4
// projection matrix that maps 3D XYZ worldspace onto 2D UV image
// space.  It also takes in some particular XY point within the
// orthorectified video image after it is draped onto the point cloud.
// It returns the deriv_order (=1 or 2) derivative in the X[Y]
// direction at the XY location.

double FusionGroup::backprojected_video_intensity_Xderiv(
   const camera* curr_camera_ptr,double X,double Y,int deriv_order)
{
   double dx=ztwoDarray_ptr->get_deltax();
   if (get_backprojected_video_intensity(
      curr_camera_ptr,X+filter_size/2*dx,Y) > 0
       && get_backprojected_video_intensity(
          curr_camera_ptr,X-filter_size/2*dx,Y) > 0)
   {
      double xderiv=0;
      for (int n=-filter_size/2; n<=filter_size/2; n++)
      {
         double curr_x=X+n*dx;
         xderiv += gaussian_filter[deriv_order][filter_size/2+n]*
            get_backprojected_video_intensity(curr_camera_ptr,curr_x,Y);
      }
      return xderiv;
   }
   return NEGATIVEINFINITY;
}

double FusionGroup::backprojected_video_intensity_Yderiv(
   const camera* curr_camera_ptr,double X,double Y,int deriv_order)
{
   double dy=ztwoDarray_ptr->get_deltay();
   if (get_backprojected_video_intensity(
      curr_camera_ptr,X,Y+filter_size/2*dy) > 0
       && get_backprojected_video_intensity(
          curr_camera_ptr,X,Y-filter_size/2*dy) > 0)
   {
      double yderiv=0;
      for (int n=-filter_size/2; n<=filter_size/2; n++)
      {
         double curr_y=Y+n*dy;
         yderiv += gaussian_filter[deriv_order][filter_size/2+n]*
            get_backprojected_video_intensity(curr_camera_ptr,X,curr_y);
      }
      return yderiv;
   }
   return NEGATIVEINFINITY;
}
   
// ---------------------------------------------------------------------
// Member function match plaquettes implements an experimental score
// function which attempts to match sub-image "plaquettes" centered
// around features in the orthorectified map plane between
// curr_imagenumber and next_imagenumber.  The score is taken to equal
// the average integrated absolute difference between the plaquette
// intensities.  In theory, minimizing this score with respect to
// translations of the next_imagenumber's features should yield
// improved KLT results.  But on 12/27/05, we found that this approach
// is badly hampered by noisy temporal fluctations.  The plaquettes
// must also be quite large in their pixel dimensions.  So evaluating
// this integrated difference score function becomes too slow to be
// practical...

void FusionGroup::match_plaquettes(
   int curr_imagenumber,int next_imagenumber)
{
   outputfunc::write_banner("Matching plaquettes:");
//   cout << "curr_imagenumber = " << curr_imagenumber << endl;
//   cout << "next_imagenumber = " << next_imagenumber << endl;

   genmatrix* curr_XYZUV_ptr=retrieve_XYZUV_feature_info(curr_imagenumber);
   genmatrix* next_XYZUV_ptr=retrieve_XYZUV_feature_info(next_imagenumber);

//   cout << "Before perturbation, *next_XYZUV_ptr = " << *next_XYZUV_ptr 
//        << endl;

   Movie_ptr->get_camera_ptr(curr_imagenumber)->
      compute_tiepoint_projection_matrix(curr_XYZUV_ptr);
   Movie_ptr->get_camera_ptr(next_imagenumber)->
      compute_tiepoint_projection_matrix(next_XYZUV_ptr);
   camera* curr_camera_ptr=Movie_ptr->get_camera_ptr(curr_imagenumber);
   camera* next_camera_ptr=Movie_ptr->get_camera_ptr(next_imagenumber);

   threevector p_origin(get_grid_world_origin());

   for (unsigned int m=0; m<curr_XYZUV_ptr->get_mdim(); m++)
   {
      int curr_ID=static_cast<int>(curr_XYZUV_ptr->get(m,0));
      for (unsigned int mnext=0; mnext<next_XYZUV_ptr->get_mdim(); mnext++)
      {
         int next_ID=static_cast<int>(next_XYZUV_ptr->get(mnext,0));
         if (curr_ID==next_ID)
         {
            double X=curr_XYZUV_ptr->get(m,1);
            double Y=curr_XYZUV_ptr->get(m,2);

//            double U=curr_XYZUV_ptr->get(m,4);
//            double V=curr_XYZUV_ptr->get(m,5);

// First fill plaquette with intensities in neighborhood of (X,Y) for
// curr_imagenumber:

            const int nsize=21;
//            const int nsize=9;
//            const int nsize=3;
            genmatrix curr_plaquette(nsize,nsize);

            unsigned int px,py;
            if (PointCloud_ptr->xypoint_to_pixel(X,Y,px,py))
            {
               Movie_ptr->displayFrame(curr_imagenumber);
               unsigned int qx_lo=px-nsize/2;
               unsigned int qx_hi=px+nsize/2;
               unsigned int qy_lo=py-nsize/2;
               unsigned int qy_hi=py+nsize/2;
               double currX,currY;
               for (unsigned int qy=qy_hi; qy >= qy_lo; qy--)
               {
                  for (unsigned int qx=qx_lo; qx <= qx_hi; qx++)
                  {
                     ztwoDarray_ptr->pixel_to_point(qx,qy,currX,currY);
                     double currI=get_backprojected_video_intensity(
                        curr_camera_ptr,currX,currY);
                     curr_plaquette.put(qy-qy_lo,qx-qx_lo,currI);
//                     curr_plaquette.put(qy_hi-qy,qx-qx_lo,currI);
//                     curr_plaquette.put(qx-qx_lo,qy-qy_lo,currI);

//                     cout << "X = " << currX-p_origin.get(0) 
//                          << " Y = " << currY-p_origin.get(1)
//                          << " qx = " << qx << " qy = " << qy 
//                          << " I = " << currI << endl;
                  } // loop over local qy index
               } // loop over local qx index
               
//               cout << "curr_plaquette = " << curr_plaquette << endl;

// Next fill plaquette with intensities in neighborhood of (X+dX,Y+dY)
// for next_imagenumber:

               genmatrix next_plaquette(nsize,nsize);
//                genmatrix best_abs_diff(nsize,nsize);

//               const int search_radius=1;
//               const int search_radius=7;
//               const int search_radius=15;
               const int search_radius=27;
               int best_d_px=0;
               int best_d_py=0;
               double min_avg_integ_abs_diff=POSITIVEINFINITY;
               for (int d_px=-search_radius; d_px <= search_radius; d_px += 2)
               {
                  for (int d_py=-search_radius; d_py <= search_radius; 
                       d_py += 2)
                  {
                     qx_lo=px-nsize/2+d_px;
                     qx_hi=px+nsize/2+d_px;
                     qy_lo=py-nsize/2+d_py;
                     qy_hi=py+nsize/2+d_py;
                     for (unsigned int qx=qx_lo; qx <= qx_hi; qx++)
                     {
                        for (unsigned int qy=qy_lo; qy <= qy_hi; qy++)
                        {
                           ztwoDarray_ptr->pixel_to_point(qx,qy,currX,currY);

// Recall that ALIRT ladar data generally has holes where Z values are
// not defined.  At such locations,
// get_backprojected_video_intensity() returns -1.  We simply choose
// not to include these bad data locations within the average
// integrated absolute difference calculated below:

                           double currI=
                              get_backprojected_video_intensity(
                                 next_camera_ptr,currX,currY);

//                           if (currI <= 0)
//                           {
//                              cout << "ID = " << curr_ID 
//                                   << " qx = " << qx << " qy = " << qy 
//                                   << " d_px = " << d_px 
//                                   << " d_py = " << d_py
//                                   << " currI = " << currI << endl;
//                              cout << "currX = " << currX-p_origin.get(0)
//                                   << " currY = " << currY-p_origin.get(1)
//                                   << endl;
//                           }
                           
                           next_plaquette.put(qy-qy_lo,qx-qx_lo,currI);
//                           next_plaquette.put(qy_hi-qy,qx-qx_lo,currI);
//                           next_plaquette.put(qx-qx_lo,qy-qy_lo,currI);
                        } // loop over local qy index
                     } // loop over local qx index

// Take score to equal integrated absolute difference between
// next_plaquette and curr_plaquette intensities:

                     int n_valid_comparisons=0;
                     double integ_abs_diff=0;
                     for (unsigned int m=0; m<curr_plaquette.get_mdim(); m++)
                     {
                        for (unsigned int n=0; n<curr_plaquette.get_ndim(); 
                             n++)
                        {
                           double curr_I=curr_plaquette.get(m,n);
                           double next_I=next_plaquette.get(m,n);
                           if (curr_I >= 0 && next_I >= 0)
                           {
                              integ_abs_diff += fabs(next_I-curr_I);
                              n_valid_comparisons++;
                           }
                        }
                     }
                     double avg_integ_abs_diff=integ_abs_diff/
                        double(n_valid_comparisons);
                     
//                     cout << "d_px = " << d_px << " d_py = " << d_py 
//                          << " integrated absolute difference = "
//                          << integ_abs_diff << endl;
//                     cout << "next_plaquette = " << next_plaquette << endl;

                     if (avg_integ_abs_diff < min_avg_integ_abs_diff)
                     {
                        min_avg_integ_abs_diff=avg_integ_abs_diff;
                        best_d_px=d_px;
                        best_d_py=d_py;
                        
/*
                        for (int m=0; m<curr_plaquette.get_mdim(); m++)
                        {
                           for (int n=0; n<curr_plaquette.get_ndim(); n++)
                           {
                              best_abs_diff.put(
                                 m,n,fabs(next_plaquette.get(m,n)-
                                          curr_plaquette.get(m,n)));
                           }
                        }
*/
                     } // avg_integ_abs_diff < min_avg_integ_abs_diff
		       //  conditional
                  } // loop over d_py
               } // loop over d_px

               cout << "ID = " << curr_ID 
                    << " min avg integ abs difference = "
                    << min_avg_integ_abs_diff 
                    << " best d_px = " << best_d_px << " best d_py = " 
                    << best_d_py << endl;
//               cout << "best_abs_diff = " << best_abs_diff << endl;

            } // xypoint_to_pixel(X,Y,px,py) conditional
         } // curr_ID==next_ID conditional
      } // loop over index mnext labeling rows in *next_XYZUV_ptr genmatrix
   } // loop over index m labeling rows in *curr_XYZUV_ptr genmatrix

//   cout << "After perturbation, *next_XYZUV_ptr = " << *next_XYZUV_ptr 
//        << endl;
   
   delete curr_XYZUV_ptr;
   delete next_XYZUV_ptr;
}
   
// ---------------------------------------------------------------------
// Member function adjust_backprojected_feature_locations retrieves
// the XYZ-UV tiepoint information for the 2 images specified by input
// parameters curr_imagenumber and next_imagenumber.  It constructs
// the 3x4 projection matrices corresponding to these two sets of
// tiepoints.  

// For every feature in both curr_imagenumber and next_imagenumber,
// this method implements a "KLT-like" algorithm to estimate
// orthorectified map perturbations dX and dY which satisfy the
// relation

// I(X,Y,t+dt) approx equal I(X+dX,Y+dY,t) 

//   approx equal I(X,Y,t) + dI/dX(X,Y,t) dX + dI/dY(X,Y,t) dY + ...

// We solve this relation for dX and dY subject to the constraint that
// (dX)**2+(dY)**2 is minimal.  This method then calls
// construct_local_homography to convert (dX,dY) in the orthorectified
// map plane to (dU,dV) in the video image plane for next_imagenumber.
// Finally, it updates the UV tiepoint values for next_imagenumber in
// *FeaturesGroup_2D_ptr.

double FusionGroup::adjust_backprojected_feature_locations(
   int curr_imagenumber,int next_imagenumber)
{
   outputfunc::write_banner("Adjusting backprojected feature locations:");
//   cout << "curr_imagenumber = " << curr_imagenumber << endl;
//   cout << "next_imagenumber = " << next_imagenumber << endl;

   genmatrix* curr_XYZUV_ptr=retrieve_XYZUV_feature_info(curr_imagenumber);
   genmatrix* next_XYZUV_ptr=retrieve_XYZUV_feature_info(next_imagenumber);
   genmatrix orig_next_XYZUV(*next_XYZUV_ptr);

//   cout << "*curr_XYZUV_ptr = " << *curr_XYZUV_ptr << endl;
//   cout << "Before perturbation, *next_XYZUV_ptr = " << orig_next_XYZUV
//        << endl;

   Movie_ptr->get_camera_ptr(curr_imagenumber)->
      compute_tiepoint_projection_matrix(curr_XYZUV_ptr);
   Movie_ptr->get_camera_ptr(next_imagenumber)->
      compute_tiepoint_projection_matrix(next_XYZUV_ptr);
   camera* curr_camera_ptr=Movie_ptr->get_camera_ptr(curr_imagenumber);
   camera* next_camera_ptr=Movie_ptr->get_camera_ptr(next_imagenumber);

   for (unsigned int m=0; m<curr_XYZUV_ptr->get_mdim(); m++)
   {
      int curr_ID=static_cast<int>(curr_XYZUV_ptr->get(m,0));
      for (unsigned int mnext=0; mnext<next_XYZUV_ptr->get_mdim(); mnext++)
      {
         int next_ID=static_cast<int>(next_XYZUV_ptr->get(mnext,0));
         if (curr_ID==next_ID)
         {
            double X=curr_XYZUV_ptr->get(m,1);
            double Y=curr_XYZUV_ptr->get(m,2);
            double Z=curr_XYZUV_ptr->get(m,3);
            double U=curr_XYZUV_ptr->get(m,4);
            double V=curr_XYZUV_ptr->get(m,5);

            unsigned int px,py;
            if (PointCloud_ptr->xypoint_to_pixel(X,Y,px,py))
            {
               Movie_ptr->displayFrame(curr_imagenumber);
               double I=get_avg_backprojected_video_intensity(
                  curr_camera_ptr,X,Y);
               double dIdX=backprojected_video_intensity_Xderiv(
                  curr_camera_ptr,X,Y,1);
               double dIdY=backprojected_video_intensity_Yderiv(
                  curr_camera_ptr,X,Y,1);
               Movie_ptr->displayFrame(next_imagenumber);
               double Inext=get_avg_backprojected_video_intensity(
                  next_camera_ptr,X,Y);

               double delta_I=Inext-I;
               double denom=sqr(dIdX)+sqr(dIdY);
               double delta_X=dIdX*delta_I/denom;
               double delta_Y=dIdY*delta_I/denom;
               
// Impose sanity limit on maximum allowed magnitude for
// sqr(delta_X)+sqr(delta_Y):

               const double max_sqrd_shift=sqr(1.5);	// meters**2
               double sqrd_shift=sqr(delta_X)+sqr(delta_Y);
               if (sqrd_shift > max_sqrd_shift)
               {
                  const threevector grid_origin(get_grid_world_origin());
                  cout << "ID = " << curr_ID << " Icurr = " << I
                       << " Inext = " << Inext 
                       << " delta_I = " << delta_I << endl;
                  cout << "dIdX = " << dIdX << " dIdY = " << dIdY << endl;
                  cout << "Xorig = " << X-grid_origin.get(0) 
                       << " Yorig = " << Y-grid_origin.get(1) 
                       << " Zorig = " << Z-grid_origin.get(2) << endl;
                  cout << "Xnew = " << X+delta_X-grid_origin.get(0) 
                       << " Ynew = " << Y+delta_Y-grid_origin.get(1) 
                       << endl;
                  cout << "   delta_X = " << delta_X 
                       << " delta_Y = " << delta_Y << endl;
                  cout << "sqrt(sqr(dX)+sqr(dY)) = " << sqrt(sqrd_shift)
                       << endl;
                  cout << endl;
               }
               else
               {
                  double delta_U,delta_V;
                  bool print_flag=false;
//               if (curr_ID==10) print_flag=true;
                  construct_local_homography(
                     curr_camera_ptr,X,Y,delta_X,delta_Y,delta_U,delta_V,
                     print_flag);

                  next_XYZUV_ptr->put(m,4,next_XYZUV_ptr->get(m,4)-delta_U);
                  next_XYZUV_ptr->put(m,5,next_XYZUV_ptr->get(m,5)-delta_V);
      
                  if (fabs(delta_U) > 0.002 || fabs(delta_V) > 0.002)
                  {
                     const threevector grid_origin(get_grid_world_origin());
                     cout << "ID = " << curr_ID << " Icurr = " << I
                          << " Inext = " << Inext 
                          << " delta_I = " << delta_I << endl;
                     cout << "dIdX = " << dIdX << " dIdY = " << dIdY << endl;
                     cout << "Xorig = " << X-grid_origin.get(0) 
                          << " Yorig = " << Y-grid_origin.get(1) 
                          << " Zorig = " << Z-grid_origin.get(2) << endl;
                     cout << "Xnew = " << X+delta_X-grid_origin.get(0) 
                          << " Ynew = " << Y+delta_Y-grid_origin.get(1) 
                          << endl;
                     cout << "   delta_X = " << delta_X 
                          << " delta_Y = " << delta_Y << endl;
                     cout << "Uorig = " << U << " Vorig = " << V << endl;
                     cout << "dU = " << delta_U << " dV = " << delta_V 
                          << endl;
                     cout << endl;
                  }
               }

            } // xypoint_to_pixel(X,Y,px,py) conditional
         } // curr_ID==next_ID conditional
      } // loop over index mnext labeling rows in *next_XYZUV_ptr genmatrix
   } // loop over index m labeling rows in *curr_XYZUV_ptr genmatrix

//   cout << "After perturbation, *next_XYZUV_ptr = " << *next_XYZUV_ptr 
//        << endl;
   
   update_XYZUV_feature_info(next_imagenumber,next_XYZUV_ptr);

// Compute sum of sqrd differences between new and original (U,V)
// pairs:

   double sqrd_diff_sum=0;
   for (unsigned int m=0; m<curr_XYZUV_ptr->get_mdim(); m++)
   {
      sqrd_diff_sum += sqr(next_XYZUV_ptr->get(m,4)-orig_next_XYZUV.get(m,4));
      sqrd_diff_sum += sqr(next_XYZUV_ptr->get(m,5)-orig_next_XYZUV.get(m,5));
   }
   double RMS_UV_diff=sqrt(sqrd_diff_sum/curr_XYZUV_ptr->get_mdim());

   delete curr_XYZUV_ptr;
   delete next_XYZUV_ptr;

   return RMS_UV_diff;
}

// ---------------------------------------------------------------------
// Member function construct_local_homography takes in an XY location
// in the orthorectified map plane as well as some estimated temporal
// perturbation (dX,dY) for where the backprojected video intensity
// moves over a small time step.  It first constructs a local
// homography based upon a bundle of (X,Y) pixels in the map plane and
// their (U.V) counterparts in the video image plane.  It then relates
// (X+dX,Y+dY) to (U+dU,V+dV).  This method returns the image plane
// perbations dU and dV.

void FusionGroup::construct_local_homography(
   const camera* curr_camera_ptr,double X,double Y,double dX,double dY,
   double& dU,double& dV,bool print_flag)
{
   unsigned int px,py;
   const threevector grid_origin(get_grid_world_origin());
   vector<twovector> XY,UV;

// Locate pixel (px,py) within *ztwoDarray_ptr corresponding to
// backprojected (X,Y) pair.  Then generate STL vector of XY pairs
// corresponding to pixels neighbors surrounding center location.
// Retrieve Z values for each of these XY locations from
// *ztwoDarray_ptr.  Then back project XYZ triples into UV image
// plane.  Store UV locations in STL vector.  

   const unsigned int q_extent=3;
   const unsigned int q_skip=1;
   if (PointCloud_ptr->xypoint_to_pixel(X,Y,px,py))
   {
      vector<double> sqr_XY_dist;
      for (unsigned int qx=basic_math::max(Unsigned_Zero,px-q_extent); 
           qx<=basic_math::min(ztwoDarray_ptr->get_mdim(),px+q_extent); 
           qx += q_skip)
      {
         for (unsigned int qy=basic_math::max(Unsigned_Zero,py-q_extent); 
              qy<=basic_math::min(ztwoDarray_ptr->get_ndim(),py+q_extent); 
              qy += q_skip)
         {
            const double SMALL=0.001;
            double curr_Z=ztwoDarray_ptr->get(qx,qy);
            if (curr_Z > grid_origin.get(2)-SMALL)
            {
               double curr_X,curr_Y;
               ztwoDarray_ptr->pixel_to_point(qx,qy,curr_X,curr_Y);
               XY.push_back(twovector(curr_X,curr_Y));
               sqr_XY_dist.push_back( sqr(curr_X-X) + sqr(curr_Y-Y) );
               
               double curr_U,curr_V;
               curr_camera_ptr->project_XYZ_to_UV_coordinates(
                  curr_X,curr_Y,curr_Z,curr_U,curr_V);
               UV.push_back(twovector(curr_U,curr_V));
               
            } // sanity check on curr_Z value
         } // loop over qy index
      } // loop over qx index

// Sort entries in XY and UV STL vectors according to entries in
// sqr_XY_dist STL vector:

      templatefunc::Quicksort(sqr_XY_dist,XY,UV);

      homography H;
      const int n_tiepoint_pairs=4;
//      const int n_tiepoint_pairs=6;
      H.parse_homography_inputs(XY,UV,n_tiepoint_pairs);
      H.compute_homography_matrix();
//      double RMS_residual=H.check_homography_matrix(XY,UV);

      twovector XY_orig(X,Y);
      twovector UV_orig=H.project_world_plane_to_image_plane(XY_orig);
      twovector XY_new=XY_orig+twovector(dX,dY);
      twovector UV_new=H.project_world_plane_to_image_plane(XY_new);
      dU=(UV_new-UV_orig).get(0);
      dV=(UV_new-UV_orig).get(1);

      if (print_flag)
      {
         cout << "inside FusionGroup::construct_local_homography()" << endl;
         for (int i=0; i<n_tiepoint_pairs; i++)
         {
            cout << i << " X = " << XY[i].get(0)-grid_origin.get(0)
                 << " Y = " << XY[i].get(1)-grid_origin.get(1)
                 << " U = " << UV[i].get(0) << " V = " << UV[i].get(1)
                 << endl;
         }

         cout << "Xorig = " << XY_orig.get(0)-grid_origin.get(0) 
              << " Yorig = " << XY_orig.get(1)-grid_origin.get(1) << endl;
         cout << "Xnew = " << XY_new.get(0)-grid_origin.get(0) 
              << " Ynew = " << XY_new.get(1)-grid_origin.get(1) << endl;

         cout << "Uorig = " << UV_orig.get(0) << " Vorig = " << UV_orig.get(1)
              << endl;
         cout << "Unew = " << UV_new.get(0) << " Vnew = " << UV_new.get(1)
              << endl;
         cout << "dU = " << dU <<  " dV = " << dV << endl;
      }
      
/*
      double Z=ztwoDarray_ptr->get(px,py);
      double Uprime,Vprime;
      curr_camera_ptr->project_XYZ_to_UV_coordinates(
         X+dX,Y+dY,Z,Uprime,Vprime);
      double delta_U=Uprime-Uorig;
      double delta_V=Vprime-Vorig;
      cout << "Uorig = " << Uorig << " Unew = " << Uprime 
           << " delta_U = " << delta_U << endl;
      cout << "Vorig = " << Vorig << " Vnew = " << Vprime 
           << " delta_V = " << delta_V << endl;
*/
    
   } // xypoint_to_pixel(X,Y,px,py) conditional
}

// --------------------------------------------------------------------------
// Member function update_XYZUV_feature_info takes in an imagenumber
// as well as genmatrix *XYZUV_ptr.  It copies the UV information [
// which we assume has been previously modified by some iteration of
// adjust_backprojected_feature_locations() ] stored in each row of
// the genmatrix into the appropriate feature stored within member
// object *FeaturesGroup_2D_ptr.

void FusionGroup::update_XYZUV_feature_info(
   int imagenumber,const genmatrix* XYZUV_ptr)
{
   double t=static_cast<double>(imagenumber);
   for (unsigned int m=0; m<XYZUV_ptr->get_mdim(); m++)
   {
      int ID=static_cast<int>(XYZUV_ptr->get(m,0));
      Feature* feature_2D_ptr=FeaturesGroup_2D_ptr->
         get_ID_labeled_Feature_ptr(ID);

      instantaneous_obs* curr_obs_ptr=feature_2D_ptr->
         get_particular_time_obs(t,FeaturesGroup_2D_ptr->get_passnumber());
      if (curr_obs_ptr != NULL)
      {
         if (curr_obs_ptr->get_npasses() > 1)
         {
            threevector UVW(XYZUV_ptr->get(m,4),XYZUV_ptr->get(m,5));
            curr_obs_ptr->change_UVW_coords(
               FeaturesGroup_2D_ptr->get_passnumber(),UVW);

//            cout << "ID = " << ID << " orig UVW = " << UVW_orig << endl;
//            cout << "new UVW = " << UVW_new << endl;
            
         }
      } // curr_obs_ptr != NULL conditional
   } // loop over index m labeling rows in *XYZUV_ptr
}

// ==========================================================================
// Animation methods
// ==========================================================================

void FusionGroup::update_display()
{   
//   cout << "inside FusionGroup::update_display()" << endl;
//   double curr_t=static_cast<double>(get_curr_framenumber());
//   cout << "curr_t = " << curr_t << endl;
   int MovieState=AnimationController_ptr->getState();

   if (view_draped_video_flag)
   {
      if (MovieState==AnimationController::PLAY ||
          MovieState==AnimationController::INCREMENT_FRAME)
      {
         color_pointcloud_with_next_videoframe();
      }
      else if (MovieState==AnimationController::REVERSE ||
               MovieState==AnimationController::DECREMENT_FRAME)
      {
         color_pointcloud_with_prev_videoframe();
      }
   }

   GraphicalsGroup::update_display();
   
//   cout << "At end of FG::update_display()" << endl;
}

// ==========================================================================
// 2D rectangle backprojection into 3D member functions
// ==========================================================================

// Member function backproject_2D_rectangle_into_3D() takes in the
// latest, selected 2D rectangle from *RectanglesGroup_ptr.  It forms
// a 10x10 grid of UV points over the input 2D rectangle.  Each UV
// grid point is raytracted back into the 3D ladar map.  A middle
// value from the distribution of ranges each backprojected UV grid
// point is calculated as the representative depth corresponding to
// the input 2D rectangle.  A cylindrical polyhedron is instantiated
// within *Polyhedron_ptr which represents our best estimate for the
// bulk of 3D worldspace corresponding to the input 2D rectangle.  A
// sub-frustum is also generated within *sub_Ground_OBSFRUSTAGROUP_ptr
// which extends from the camera's world space position out to the
// representative depth. 

polyhedron* FusionGroup::backproject_2D_rectangle_into_3D()
{
//   cout << "inside FusionGroup::backproject_2D_rectangle_into_3D()"
//        << endl;

   if (ray_tracer_ptr==NULL) return NULL;
   if (Ground_OBSFRUSTAGROUP_ptr==NULL) return NULL;
   if (sub_Ground_OBSFRUSTAGROUP_ptr==NULL) return NULL;
   if (PolyhedraGroup_ptr==NULL) return NULL;

//    unsigned int n_Rectangles=RectanglesGroup_ptr->get_n_Graphicals();
//   cout << "n_Rectangles = " << n_Rectangles << endl;
   Rectangle* Rectangle_ptr=RectanglesGroup_ptr->get_selected_Rectangle_ptr();
   sub_Ground_OBSFRUSTAGROUP_ptr->set_new_subfrustum_ID(
      sub_Ground_OBSFRUSTAGROUP_ptr->get_n_Graphicals());
   sub_Ground_OBSFRUSTAGROUP_ptr->set_subfrustum_bbox_ptr(
      Rectangle_ptr->get_bbox_ptr());

//   if (sub_Aerial_OBSFRUSTAGROUP_ptr != NULL) 
//   {
//      sub_Aerial_OBSFRUSTAGROUP_ptr->destroy_all_OBSFRUSTA();
//      sub_Aerial_OBSFRUSTAGROUP_ptr->set_new_subfrustum_ID(-1);
//   }

// Establish UV grid covering bbox:

   bounding_box bbox=Rectangle_ptr->get_bbox();
   vector<twovector> bbox_UVs;

   int nU=10;
   int nV=10;
   double du=1.0/nU;
   double dv=1.0/nV;
   double u,v;
   for (int i=0; i<nU; i++)
   {
      double ufrac=(i+0.5)*du;
      for (int j=0; j<nV; j++)
      {
         double vfrac=(j+0.5)*dv;
         bbox.XY_frac_coords(ufrac,vfrac,u,v);
         bbox_UVs.push_back(twovector(u,v));
//         cout << "i = " << i << " j = " << j << " u = " << u << " v = " << v
//         << endl;
      }
   }
   
// Backproject UV grid points into 3D worldspace:

   OBSFRUSTUM* selected_ground_OBSFRUSTUM_ptr=Ground_OBSFRUSTAGROUP_ptr->
      get_selected_OBSFRUSTUM_ptr();
   if (selected_ground_OBSFRUSTUM_ptr == NULL) return NULL;
   
   Movie* Movie_ptr=selected_ground_OBSFRUSTUM_ptr->get_Movie_ptr();
   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   threevector ground_camera_posn=camera_ptr->get_world_posn();

   const double min_raytrace_range=10;		// meters
   const double max_raytrace_range=10*1000;	// meters
   const double ds=0.5;				// meter
   threevector curr_occluded_ray_posn;
   vector<double> occluded_ranges;
   vector<threevector> occluded_posns;
   vector<double> occluded_phis;
   for (unsigned int i=0; i<bbox_UVs.size(); i++)
   {
      threevector rhat=camera_ptr->pixel_ray_direction(bbox_UVs[i]);
      int occlusion_flag=ray_tracer_ptr->trace_individual_ray(
         ground_camera_posn,rhat,max_raytrace_range,min_raytrace_range,
         ds,curr_occluded_ray_posn);
      if (occlusion_flag==0)
      {
         double curr_range=(curr_occluded_ray_posn-ground_camera_posn).
            magnitude();
         twovector curr_rho(curr_occluded_ray_posn-ground_camera_posn);
         occluded_ranges.push_back(curr_range);
         occluded_posns.push_back(curr_occluded_ray_posn);
         occluded_phis.push_back(atan2(curr_rho.get(1),curr_rho.get(0)));
//         cout << "i = " << i
//              << " range = " << curr_range
//              << endl;
      }
   } // loop over index i labeling UV grid points

// Compute 10th and 90th percentile ranges for 3D raytraced
// counterparts to gridded UV points:
   
   prob_distribution prob(
      occluded_ranges,int(max_raytrace_range),min_raytrace_range,1);
   double r_near=prob.find_x_corresponding_to_pcum(0.50);
   double r_middle=prob.find_x_corresponding_to_pcum(0.65);
   double r_far=prob.find_x_corresponding_to_pcum(0.80);
   cout << "r_near = " << r_near 
        << " r_middle = " << r_middle
        << " r_far = " << r_far << endl;
   sub_Ground_OBSFRUSTAGROUP_ptr->set_subfrustum_downrange_distance(r_middle);

   double zmin=POSITIVEINFINITY;
   double zmax=NEGATIVEINFINITY;
   double min_occluded_phi=POSITIVEINFINITY;
   double max_occluded_phi=NEGATIVEINFINITY;
   vector<threevector> worldspace_points;

   for (unsigned int i=0; i<occluded_ranges.size(); i++)
   {
      min_occluded_phi=basic_math::min(min_occluded_phi,occluded_phis[i]);
      max_occluded_phi=basic_math::max(max_occluded_phi,occluded_phis[i]);
      double curr_range=occluded_ranges[i];
      if (curr_range > r_near && curr_range < r_far)
      {
         worldspace_points.push_back(occluded_posns[i]);
         zmin=basic_math::min(zmin,occluded_posns[i].get(2));
         zmax=basic_math::max(zmax,occluded_posns[i].get(2));
      }
   } // loop over index i labeling occluded ranges
//   cout << "zmin = " << zmin << " zmax = " << zmax << endl;

// Generate cylindrical polyhedron located a depth r_middle from the
// ground camera.  Cylinder represents best estimate for majority of
// 3D volume corresponding to input 2D rectangle:

   min_occluded_phi=basic_math::phase_to_canonical_interval(
      min_occluded_phi,max_occluded_phi-PI,max_occluded_phi+PI);
   double avg_phi=0.5*(max_occluded_phi+min_occluded_phi);
//   cout << "Avg_phi = " << avg_phi*180/PI << endl;
   double delta_phi=max_occluded_phi-min_occluded_phi;
//   cout << "delta_phi = " << delta_phi*180/PI << endl;
   double diameter=r_middle*delta_phi;
   double min_diameter=5;	// meters
   diameter=basic_math::max(min_diameter,diameter);
//   cout << "Diameter = " << diameter << endl;

   threevector cylinder_center=ground_camera_posn+
      r_middle*threevector(cos(avg_phi),sin(avg_phi),0);
   cylinder_center.put(2,zmin);
   regular_polygon cyl(16,0.5*diameter);
   cyl.absolute_position(cylinder_center);
   contour bottom_face_contour(&cyl);

   double height=zmax-zmin;
   polyhedron* hull_3D_ptr=new polyhedron();
   hull_3D_ptr->generate_prism_with_rectangular_faces(
      bottom_face_contour,height);
//   cout << "*hull_3D_ptr = " << *hull_3D_ptr << endl;

   Polyhedron* Polyhedron_ptr=PolyhedraGroup_ptr->generate_new_Polyhedron(
      hull_3D_ptr);

   double alpha=0.1;
   osg::Vec4 edge_color=colorfunc::get_OSG_color(colorfunc::white);
   osg::Vec4 volume_color=colorfunc::get_OSG_color(colorfunc::white,alpha);
   Polyhedron_ptr->set_color(edge_color,volume_color);
   Polyhedron_ptr->set_permanent_color(colorfunc::white);
   Polyhedron_ptr->set_blinking_color(colorfunc::black);

   double blink_period=600;	// secs
   PolyhedraGroup_ptr->blink_Geometrical(
      Polyhedron_ptr->get_ID(),blink_period);

//   cout << "polyhedron = " << *(Polyhedron_ptr->get_polyhedron_ptr())
//        << endl;

   return Polyhedron_ptr->get_polyhedron_ptr();
}
      
// --------------------------------------------------------------------------
// Member function project_ground_polyhedron_into_aerial_OBSFRUSTA()
// retrieves the cylindrical polyhedron corresponding to an input 2D
// rectangle within a ground camera's image plane.  It loops over all
// OBSFRUSTA within *Aerial_OBSFRUSTAGROUP_ptr.  For each aerial
// OBSFRUSTUM, this method projects the ground polyhedron's vertices
// into the aerial image plane.  It then forms a bounding box in the
// UV aerial image plane.  The aerial OBSFRUSTUM with the large UV
// bounding box is taken to be the one with the best aerial view of
// the input ground cylindrical prism.  This method instantiates a
// subfrustum within *sub_Aerial_OBSFRUSTAGROUP_ptr which extends from
// the aerial camera's position nearly down to the ground polyhedron's
// position.

void FusionGroup::project_ground_polyhedron_into_aerial_OBSFRUSTA()
{
   cout << "inside FusionGroup::project_ground_polyhedron_into_aerial_OBSFRUSTA()"
        << endl;

   unsigned int n_Polyhedra=PolyhedraGroup_ptr->get_n_Graphicals();
   if (n_Polyhedra==0) return;
   Polyhedron* Polyhedron_ptr=PolyhedraGroup_ptr->get_Polyhedron_ptr(
      n_Polyhedra-1);
   polyhedron* polyhedron_ptr=Polyhedron_ptr->get_polyhedron_ptr();
   if (polyhedron_ptr==NULL)
   {
      cout << "polyhedron_ptr = NULL " << endl;
      return;
   }
   
   int n_vertices=polyhedron_ptr->get_n_vertices();
//   cout << "n_vertices = " << n_vertices << endl;
   vector<threevector> polyhedron_vertices;
   for (int n=0; n<n_vertices; n++)
   {
      polyhedron_vertices.push_back(polyhedron_ptr->get_vertex(n).get_posn());
   }

// Loop over aerial OBSFRUSTA.  For each aerial OBSFRUSTUM, compute
// projection of ground polyhedron vertices into its image plane.
// Compute bounding box in of projected ground polyhedron vertices
// into aerial image's UV plane.  Define best aerial OBSFRUSTUM as
// that which has the largest UV bounding box.

   int best_aerial_OBSFRUSTUM_ID=-1;
   double max_UV_bbox_area=NEGATIVEINFINITY;
   threevector curr_UVW;
   bounding_box best_UV_bbox;
   for (unsigned int i=0; i<Aerial_OBSFRUSTAGROUP_ptr->get_n_Graphicals(); i++)
   {
      OBSFRUSTUM* aerial_OBSFRUSTUM_ptr=Aerial_OBSFRUSTAGROUP_ptr->
         get_OBSFRUSTUM_ptr(i);
      Movie* Movie_ptr=aerial_OBSFRUSTUM_ptr->get_Movie_ptr();
      camera* camera_ptr=Movie_ptr->get_camera_ptr();
      bounding_box* UV_bbox_ptr=camera_ptr->get_UV_bbox_ptr();

      vector<threevector> interior_vertices;
      for (int n=0; n<n_vertices; n++)
      {
         threevector curr_vertex=polyhedron_vertices[n];
         camera_ptr->project_XYZ_to_UV_coordinates(curr_vertex,curr_UVW);
         if (!UV_bbox_ptr->point_inside(curr_UVW.get(0),curr_UVW.get(1)))
            continue;
         interior_vertices.push_back(curr_UVW);
      }

      if (interior_vertices.size() < 0.5*n_vertices) continue;

      bounding_box UV_bbox;
      UV_bbox.reset_bounds(interior_vertices);
      double curr_UV_bbox_area=UV_bbox.get_area();

      if (curr_UV_bbox_area > max_UV_bbox_area)
      {
         max_UV_bbox_area=curr_UV_bbox_area;
         best_UV_bbox=UV_bbox;
         best_aerial_OBSFRUSTUM_ID=aerial_OBSFRUSTUM_ptr->get_ID();
//         cout << "best_UV_bbox = " << best_UV_bbox << endl;
//         cout << "max_UV_bbox_area = " << max_UV_bbox_area 
//              << " best aerial OBSFRUSTUM_ID = "
//              << best_aerial_OBSFRUSTUM_ID << endl;
      }

   } // loop over index i labeling aerial OBSFRUSTA

   if (best_aerial_OBSFRUSTUM_ID < 0) return;

   int selected_ground_OBSFRUSTUM_ID=Ground_OBSFRUSTAGROUP_ptr->
      get_selected_Graphical_ID();
   
   cout << "selected_ground_OBSFRUSTUM_ID = " 
        << selected_ground_OBSFRUSTUM_ID << endl;
   cout << " best aerial OBSFRUSTUM_ID = "
        << best_aerial_OBSFRUSTUM_ID << endl;

   Aerial_OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(
      best_aerial_OBSFRUSTUM_ID);
   OBSFRUSTUM* Aerial_OBSFRUSTUM_ptr=Aerial_OBSFRUSTAGROUP_ptr->
      get_selected_OBSFRUSTUM_ptr();   
   Movie* Movie_ptr=Aerial_OBSFRUSTUM_ptr->get_Movie_ptr();
   camera* camera_ptr=Movie_ptr->get_camera_ptr();
   bounding_box* UV_bbox_ptr=camera_ptr->get_UV_bbox_ptr();

   Ground_OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(
      selected_ground_OBSFRUSTUM_ID);

// Generate aerial sub frustum which extends selected aerial
// OBSFRUSTUM from camera's world position down nearly to the input
// ground cylindrical polyhedron:

   if (sub_Aerial_OBSFRUSTAGROUP_ptr==NULL) return;

   sub_Aerial_OBSFRUSTAGROUP_ptr->set_new_subfrustum_ID(
      sub_Aerial_OBSFRUSTAGROUP_ptr->get_n_Graphicals());

   sub_Aerial_OBSFRUSTAGROUP_ptr->set_subfrustum_bbox_ptr(UV_bbox_ptr);
   sub_Aerial_OBSFRUSTAGROUP_ptr->set_subfrustum_volume_alpha(0);
   sub_Aerial_OBSFRUSTAGROUP_ptr->set_subfrustum_color(colorfunc::red);
//      colorfunc::cyan);

   threevector polyhedron_COM=polyhedron_ptr->compute_COM();
   double aerial_range=0.95*(camera_ptr->get_world_posn()-polyhedron_COM).
      magnitude();
   sub_Aerial_OBSFRUSTAGROUP_ptr->set_subfrustum_downrange_distance(
      aerial_range);

//   cout << "Aerial range = " << aerial_range << endl;
}
      
// --------------------------------------------------------------------------
// Member function fly_to_best_aerial_OBSFRUSTUM()

void FusionGroup::fly_to_best_aerial_OBSFRUSTUM()
{
//   cout << "inside FusionGroup::fly_to_best_aerial_OBSFRUSTUM()" << endl;

   Aerial_OBSFRUSTAGROUP_ptr->fly_to_entered_OBSFRUSTUM(
      Aerial_OBSFRUSTAGROUP_ptr->get_selected_Graphical_ID());
}

// --------------------------------------------------------------------------
// Member function fly_to_best_aerial_OBSFRUSTUM()

void FusionGroup::destroy_all_sub_aerial_OBSFRUSTA()
{
//   cout << "inside FusionGroup::destroy_all_sub_aerial_OBSFRUSTA()" << endl;

   if (sub_Aerial_OBSFRUSTAGROUP_ptr != NULL) 
   {
      sub_Aerial_OBSFRUSTAGROUP_ptr->set_new_subfrustum_ID(-1);
      sub_Aerial_OBSFRUSTAGROUP_ptr->null_SUBFRUSTUM_ptr();
      Aerial_OBSFRUSTAGROUP_ptr->null_SUBFRUSTUM_ptr();

      sub_Aerial_OBSFRUSTAGROUP_ptr->destroy_all_OBSFRUSTA();
//      cout << "n_sub_Aerial_frusta = "
//           << sub_Aerial_OBSFRUSTAGROUP_ptr->get_n_Graphicals() << endl;

   }
}

// ==========================================================================
// WISP mask generation member functions
// ==========================================================================

// Member function generate_terrain_masks() works with one of the 10
// 36-degree cameras which model WISP's panoramic view.  It also works
// with an ALIRT ladar map which is assumed to have been read into
// *DTED_ztwoDarray_ptr.  The method iterates over all pixels in
// *DTED_ztwoDarray_ptr and finds those which lie within the camera's
// FOV.  It also searches for vertical discontinuities in the ladar
// map and fills in voxel chimneys in their vicinity.
// generate_terrain_masks() next computes a sorted range list for all
// potentially visible ALIRT voxels.  Working from longest to shortest
// ranges, it projects each voxel into 2D imageplane masks which store
// voxel X,Y,Z and R information.  Pixels corresponding to the solid
// angle subtended by a 3D voxel are colored according to the
// 3D metadata.  After the iteration over all ALIRT voxels is
// complete, the 2D masks are written to output jpeg files.

void FusionGroup::generate_terrain_masks()
{
//   cout << "inside FusionGroup::generate_terrain_masks()" << endl;

   int selected_photo_ID=ground_photogroup_ptr->get_selected_photo_ID();
//   cout << "selected photo ID = " << selected_photo_ID << endl;
   generate_terrain_masks(selected_photo_ID);
}

void FusionGroup::generate_terrain_masks(int photo_ID)
{
//   cout << "inside FusionGroup::generate_terrain_masks()" << endl;

   string photo_ID_label=stringfunc::number_to_string(photo_ID);
   string banner="Generating X,Y,Z & range terrain maps for photo "+
      photo_ID_label;
   outputfunc::write_big_banner(banner);

   photograph* photo_ptr=ground_photogroup_ptr->get_photograph_ptr(photo_ID);
   camera* camera_ptr=photo_ptr->get_camera_ptr();
//   cout << "*camera_ptr = " << *camera_ptr << endl;
   double Umax=photo_ptr->get_maxU();
   threevector camera_posn=camera_ptr->get_world_posn();
   double camera_X=camera_posn.get(0);
   double camera_Y=camera_posn.get(1);
   double camera_Z=camera_posn.get(2);
   threevector pointing_dir=-camera_ptr->get_What();
   double pointing_dir_X=pointing_dir.get(0);
   double pointing_dir_Y=pointing_dir.get(1);
   double pointing_dir_Z=pointing_dir.get(2);
   double focal_param=camera_ptr->get_fu();

   threevector Uhat=camera_ptr->get_Uhat();
   threevector Vhat=camera_ptr->get_Vhat();

   double aspect_ratio=double(photo_ptr->get_xdim())/
      double(photo_ptr->get_ydim());

   double FOV_u,FOV_v;
   camerafunc::horiz_vert_FOVs_from_f_and_aspect_ratio(
      focal_param,aspect_ratio,FOV_u,FOV_v);

   cout << "FOV_u = " << FOV_u*180/PI << endl;
   cout << "FOV_v = " << FOV_v*180/PI << endl;

   TilesGroup* TilesGroup_ptr=ray_tracer_ptr->get_TilesGroup_ptr();
   twoDarray* DTED_ztwoDarray_ptr=TilesGroup_ptr->get_DTED_ztwoDarray_ptr();
   cout << "*DTED_ztwoDarray_ptr = " << *DTED_ztwoDarray_ptr << endl;

// Instantiate mask to hold range information:

   unsigned int mask_mdim=0.5*photo_ptr->get_xdim();
   unsigned int mask_ndim=0.5*photo_ptr->get_ydim();
//   int mask_mdim=photo_ptr->get_xdim();
//   int mask_ndim=photo_ptr->get_ydim();
   cout << "mask_mdim = " << mask_mdim << " mask_ndim = " << mask_ndim 
        << endl;

   twoDarray* range_mask_ptr=new twoDarray(mask_mdim,mask_ndim);
   range_mask_ptr->init_coord_system(0,Umax,0,1);
   range_mask_ptr->initialize_values(-1.0);

   unsigned int pu,pv;
   double x,y,z,u,v;
   double dz=DTED_ztwoDarray_ptr->get_deltax();    // meters
   cout << "dz = " << dz << endl;

   vector<double>* X_ptr=new vector<double>;
   vector<double>* Y_ptr=new vector<double>;
   vector<double>* Z_ptr=new vector<double>;
   vector<double>* R_ptr=new vector<double>;

   X_ptr->reserve(mask_mdim*mask_ndim);
   Y_ptr->reserve(mask_mdim*mask_ndim);
   Z_ptr->reserve(mask_mdim*mask_ndim);
   R_ptr->reserve(mask_mdim*mask_ndim);

   int nsize=5;
   int w=(nsize-1)/2;
   vector<double> neighborhood_Z;

   for (unsigned int px=w; px<DTED_ztwoDarray_ptr->get_mdim()-w; px++)
   {
      outputfunc::update_progress_fraction(
         px,1000,DTED_ztwoDarray_ptr->get_mdim());
      
      for (unsigned int py=w; py<DTED_ztwoDarray_ptr->get_ndim()-w; py++)
      {
         DTED_ztwoDarray_ptr->fast_pixel_to_XYZ(px,py,x,y,z);

         if (z < 0) continue; // z=-1 indicates missing ladar data

// Make sure threevector(x,y,z) lies in front of camera!

         double dotproduct=
            pointing_dir_X*(x-camera_X)+pointing_dir_Y*(y-camera_Y)
            +pointing_dir_Z*(z-camera_Z);
         if (dotproduct < 0) continue;

//         camera_ptr->project_XYZ_to_UV_coordinates(x,y,z,u,v);
         camera_ptr->cyl_project_XYZ_to_UV_coordinates(x,y,z,u,v);

         if (u < 0 || u > Umax || v < 0 || v > 1) continue;

// Search within nsize x nsize neighborhood around current (px,py)
// pixel for lowest Z value zlo.  Only in the vicinity of a sharp
// vertical discontinuity does zlo differ appreciably from z:

         neighborhood_Z.clear();
         for (int i=0; i<nsize; i++)
         {
            for (int j=0; j<nsize; j++)
            {
               double curr_z=DTED_ztwoDarray_ptr->get(px-w+i,py-w+j);
               if (curr_z > 0) neighborhood_Z.push_back(curr_z);
            }
         } 

         double zlo=z;
         if (neighborhood_Z.size() > 0)
         {
            std::sort(neighborhood_Z.begin(),neighborhood_Z.end());
            zlo=neighborhood_Z.front();
         }
//         cout << "neighborhood_Z.size() = " << neighborhood_Z.size()
//              << " zlo = " << zlo << endl;

// We assume all world-space voxels lying below z but above zlo are
// opaque.  So we project all of these "chimney" voxels into the UV
// image plane:

         unsigned int pdim=basic_math::mytruncate((z-zlo)/dz)+1;
         double delta_z=0;
         if (pdim > 1)
         {
            delta_z=(z-zlo)/(pdim-1);
         }
//         cout << "pdim = " << pdim << " delta_z = " << delta_z << endl;
         for (unsigned int pz=0; pz<=pdim; pz++)
         {
            double curr_z=zlo+pz*delta_z;
//            cout << "pz = " << pz << " curr_z = " << curr_z << endl;

//            camera_ptr->project_XYZ_to_UV_coordinates(x,y,curr_z,u,v);
            camera_ptr->cyl_project_XYZ_to_UV_coordinates(x,y,curr_z,u,v);
            if (u < 0 || u > Umax || v < 0 || v > 1) continue;

// Calculate range to current 3D worldspace point:

            double prev_range=range_mask_ptr->fast_XY_to_Z(u,v,pu,pv);
            if (pu < 0 || pu >= mask_mdim) continue;
            if (pv < 0 || pv >= mask_ndim) continue;

/*
            cout << "px = " << px << " py = " << py << endl;
            cout << "x = " << x << " y = " << y << " curr_z = " 
                 << curr_z << endl;
            cout << "u = " << u << " v = " << v << endl;
            cout << "pu = " << pu << " pv = " << pv << endl;
*/
            double curr_range=
               sqrt(sqr(x-camera_X)+sqr(y-camera_Y)+sqr(curr_z-camera_Z));

            if (prev_range < 0 || curr_range < prev_range)
            {
               range_mask_ptr->put(pu,pv,curr_range);

               X_ptr->push_back(x);
               Y_ptr->push_back(y);
               Z_ptr->push_back(curr_z);
               R_ptr->push_back(curr_range);
            } // curr_range < prev_range conditional

         } // loop over pz index
      } // loop over py index
   } // loop over px index

   cout << endl;

// Sort *R_ptr while making corresponding modifications to
// *X_ptr, *Y_ptr and *Z_ptr:

   cout << "Sorting XYZ points according to range" << endl;
   templatefunc::Quicksort_descending(*R_ptr,*X_ptr,*Y_ptr,*Z_ptr);

// Write out mini point cloud corresponding to raytraced XYZ points:

   string tdp_filename="cloud_subset_"+photo_ID_label+".tdp";   
   tdpfunc::write_xyz_data(tdp_filename,X_ptr,Y_ptr,Z_ptr);

// Instantiate image plane masks to hold projected XYZ information:

   twoDarray* X_mask_ptr=new twoDarray(mask_mdim,mask_ndim);
   twoDarray* Y_mask_ptr=new twoDarray(mask_mdim,mask_ndim);
   twoDarray* Z_mask_ptr=new twoDarray(mask_mdim,mask_ndim);
   twoDarray* R_mask_ptr=new twoDarray(mask_mdim,mask_ndim);

   X_mask_ptr->init_coord_system(0,Umax,0,1);
   Y_mask_ptr->init_coord_system(0,Umax,0,1);
   Z_mask_ptr->init_coord_system(0,Umax,0,1);
   R_mask_ptr->init_coord_system(0,Umax,0,1);

   X_mask_ptr->initialize_values(-1.0);
   Y_mask_ptr->initialize_values(-1.0);
   Z_mask_ptr->initialize_values(-1.0);
   R_mask_ptr->initialize_values(-1.0);

// Calculate angle-angle subtended by individual mask pixel assuming
// panel's horizontal FOV = 36 degrees:

   double d_angle=(36*PI/180) / mask_mdim;
//   cout << "d_angle = " << d_angle*180/PI << endl;
//   cout << "Vert FOV = " << mask_ndim*d_angle*180/PI << endl;

   cout << "Projecting sorted XYZ points into UV image plane" << endl;

// Iterate over sorted X_ptr, Y_ptr and Z_ptr arrays.  Project each
// XYZ point into UV image plane.  Then transfer their world-space
// inforamtion into image plane masks.

//   cout << "R.size() = " << R_ptr->size() << endl;
   for (unsigned int i=0; i<R_ptr->size(); i++)
   {
      double x=X_ptr->at(i);
      double y=Y_ptr->at(i);
      double z=Z_ptr->at(i);
      double r=R_ptr->at(i);
      
//      double u_proj,v_proj;
//      camera_ptr->project_XYZ_to_UV_coordinates(x,y,z,u_proj,v_proj);
      camera_ptr->cyl_project_XYZ_to_UV_coordinates(x,y,z,u,v);

      range_mask_ptr->fast_XY_to_Z(u,v,pu,pv);

      if (pu < 0 || pu >= mask_mdim) continue;
      if (pv < 0 || pv >= mask_ndim) continue;

//      cout << "i=" << i 
//           << " x=" << x
//           << " y=" << y 
//           << " z=" << z
//           << " pu=" << pu
//           << " pv=" << pv << endl;

// Calculate solid angle subtended by voxel within WISP image plane:

      double ds=dz;
      const double fudge_factor=1;
      double d_theta=fudge_factor*ds/r;
      int n_covered_pixels=basic_math::mytruncate(d_theta/d_angle)+1;
      if (is_even(n_covered_pixels)) n_covered_pixels++;

//      cout << "d_theta = " << d_theta*180/PI 
//           << " r = " << r
//           << " n_covered_pixels = " << n_covered_pixels << endl;
               
      for (unsigned int qu=pu-n_covered_pixels/2; qu<=pu+n_covered_pixels/2; 
           qu++)
      {
         if (qu < 0 || qu >= mask_mdim) continue;
         for (unsigned int qv=pv-n_covered_pixels/2; 
              qv<=pv+n_covered_pixels/2; qv++)
         {
            if (qv < 0 || qv >= mask_ndim) continue;

            X_mask_ptr->put(qu,qv,x);
            Y_mask_ptr->put(qu,qv,y);
            Z_mask_ptr->put(qu,qv,z);
            R_mask_ptr->put(qu,qv,r);
         } // loop over qv
      } // loop over qu
   } // loop over index i labeing sorted XYZ points

   delete range_mask_ptr;

   delete X_ptr;
   delete Y_ptr;
   delete Z_ptr;
   delete R_ptr;

// Write out 2D image plane masks:

   cout << "Writing out 2D image plane masks" << endl;

   ColorMap color_map;
   color_map.set_mapnumber(4);	// large hue value sans white

   color_map.set_min_value(2,0);
   color_map.set_max_value(2,1);
   color_map.set_min_threshold(0);
   color_map.set_max_threshold(1);

   string label;
   for (int iter=0; iter<4; iter++)
   {
      twoDarray* mask_ptr=NULL;
      if (iter==0)
      {
         mask_ptr=X_mask_ptr;
         label="X_"+photo_ID_label;
      }
      else if (iter==1)
      {
         mask_ptr=Y_mask_ptr;
         label="Y_"+photo_ID_label;
      }
      else if (iter==2)
      {
         mask_ptr=Z_mask_ptr;
         label="Z_"+photo_ID_label;
      }
      else if (iter==3)
      {
         mask_ptr=R_mask_ptr;
         label="R_"+photo_ID_label;
      }

      int n_channels=3;
      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         mask_mdim,mask_ndim,1,n_channels,NULL);
      texture_rectangle_ptr->initialize_twoDarray_image(mask_ptr,n_channels);

      for (unsigned int pu=0; pu<mask_mdim; pu++)
      {
         for (unsigned int pv=0; pv<mask_ndim; pv++)
         {
            double curr_value=mask_ptr->get(pu,pv);

//            cout << "pu = " << pu << " pv = " << pv 
//                 << " value = " << curr_value << endl;
            
//               const double renorm_denom=20;	// meters
//               const double renorm_denom=50;	// meters
            const double renorm_denom=100;	// meters
            double renorm_value=curr_value/renorm_denom;
//            cout << "renorm_value = " << renorm_value << endl;

            colorfunc::RGBA curr_RGBA(0,0,0,0);
            if (renorm_value > 0)
            {
               double frac=renorm_value-basic_math::mytruncate(renorm_value);
//               cout << "frac = " << frac << endl;
               
               curr_RGBA=color_map.retrieve_curr_RGBA(frac);
            }
            
            int R=255*curr_RGBA.first;
            int G=255*curr_RGBA.second;
            int B=255*curr_RGBA.third;
//            cout << "R = " << R << " G = " << G << " B = " << B << endl;
             
            texture_rectangle_ptr->set_pixel_RGB_values(pu,pv,R,G,B);

         } // loop over pv index
      } // loop over pu index

      string mask_filename="mask_"+label+".jpg";
      texture_rectangle_ptr->write_curr_frame(mask_filename);

      int height=640;
      int yoffset=0;
      int px_start=10;
      int px_stop=976;
      int width=px_stop-px_start;
      imagefunc::crop_image(
         mask_filename,width,height,px_start,yoffset);

      string banner="Wrote projected mask to "+ mask_filename;
      outputfunc::write_big_banner(banner);

      delete texture_rectangle_ptr;
   } // loop over iter index labeling mask type

   delete X_mask_ptr;
   delete Y_mask_ptr;
   delete Z_mask_ptr;
   delete R_mask_ptr;
}
