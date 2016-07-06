// ==========================================================================
// PassInfo class member function definitions
// ==========================================================================
// Last updated on 12/16/09; 7/3/13; 8/5/13; 4/5/14
// ==========================================================================

#include "math/constant_vectors.h"
#include "templates/mytemplates.h"
#include "passes/PassInfo.h"


#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:

void PassInfo::allocate_member_objects()
{
}

void PassInfo::initialize_member_objects()
{
   independent_var=2;

   min_threshold.clear();
   max_threshold.clear();
   for (unsigned int i=0; i<4; i++)
   {
      min_threshold.push_back(POSITIVEINFINITY);
      max_threshold.push_back(NEGATIVEINFINITY);
   }
   height_offset=0;
   UTM_zonenumber=-1;
   northern_hemisphere_flag=true;
   P_ptr=NULL;
   imageplane_x=imageplane_y=imageplane_z=imageplane_w=NEGATIVEINFINITY;
}

PassInfo::PassInfo()
{
//    cout << "inside PassInfo void constructor" << endl;
   allocate_member_objects();
   initialize_member_objects();
   clear_params();
}

// Copy constructor:

PassInfo::PassInfo(const PassInfo& pi)
{
//   cout << "inside PassInfo copy constructor" << endl;
   allocate_member_objects();
   initialize_member_objects();
   docopy(pi);
}

PassInfo::~PassInfo()
{
//   cout << "inside Passinfo destructor, this = " << this << endl;
   delete P_ptr;
   P_ptr=NULL;
}

// ---------------------------------------------------------------------
double PassInfo::get_min_threshold(int i) const
{
   return min_threshold[i];
}

double PassInfo::get_max_threshold(int i) const
{
//   cout << "inside PassInfo::get_max_threshold()" << endl;
//   cout << "i = " << i << " max_threshold[i] = " << max_threshold[i] << endl;
   return max_threshold[i];
}

// ---------------------------------------------------------------------
void PassInfo::docopy(const PassInfo& pi)
{
//   cout << "inside PassInfo::docopy()" << endl;

   portrait_mode_flag=pi.portrait_mode_flag;
   ID=pi.ID;
   OSGsubPAT_ID=pi.OSGsubPAT_ID;
   start_argument_index=pi.start_argument_index;
   stop_argument_index=pi.stop_argument_index;
   height_colormap_number=pi.height_colormap_number;
   prob_colormap_number=pi.prob_colormap_number;
   independent_var=pi.independent_var;
   height_colormap_cyclic_fraction_offset=
      pi.height_colormap_cyclic_fraction_offset;
   prob_colormap_cyclic_fraction_offset=
      pi.prob_colormap_cyclic_fraction_offset;

   for (unsigned int i=0; i<min_threshold.size(); i++)
   {
      min_threshold[i]=pi.min_threshold[i];
      max_threshold[i]=pi.max_threshold[i];
   }
   min_threshold_fraction=pi.min_threshold_fraction;
   max_threshold_fraction=pi.max_threshold_fraction;

   longitude_lo=pi.longitude_lo;
   longitude_hi=pi.longitude_hi;
   latitude_lo=pi.latitude_lo;
   latitude_hi=pi.latitude_hi;
   easting_lo=pi.easting_lo;
   easting_hi=pi.easting_hi;
   northing_lo=pi.northing_lo;
   northing_hi=pi.northing_hi;
   altitude=pi.altitude;
   UTM_zonenumber=pi.UTM_zonenumber;
   northern_hemisphere_flag=pi.northern_hemisphere_flag;

   elapsed_secs_since_epoch_lo=pi.elapsed_secs_since_epoch_lo;
   elapsed_secs_since_epoch_hi=pi.elapsed_secs_since_epoch_hi;

   start_frame_ID=pi.start_frame_ID;
   stop_frame_ID=pi.stop_frame_ID;
   photo_ID=pi.photo_ID;
   height_offset=pi.height_offset;
   focal_length=pi.focal_length;
   Uaxis_focal_length=pi.Uaxis_focal_length;
   Vaxis_focal_length=pi.Vaxis_focal_length;
   U0=pi.U0;
   V0=pi.V0;
   pixel_skew_angle=pi.pixel_skew_angle;
   relative_az=pi.relative_az;
   relative_el=pi.relative_el;
   relative_roll=pi.relative_roll;
   camera_posn=pi.camera_posn;

   frustum_color=pi.frustum_color;
   frustum_sidelength=pi.frustum_sidelength;
   downrange_distance=pi.downrange_distance;
   magnetic_yaw=pi.magnetic_yaw;
   imageplane_x=pi.imageplane_x;
   imageplane_y=pi.imageplane_y;
   imageplane_z=pi.imageplane_z;
   imageplane_w=pi.imageplane_w;
   filter_alpha_value=pi.filter_alpha_value;

   PostGIS_hostname=pi.PostGIS_hostname;
   PostGIS_database_name=pi.PostGIS_database_name;
   PostGIS_username=pi.PostGIS_username;

   package_subdir=pi.package_subdir;
   package_filename_prefix=pi.package_filename_prefix;

   ActiveMQ_hostname=pi.ActiveMQ_hostname;

   gispoints_tablenames.clear();
   for (unsigned int t=0; t<pi.gispoints_tablenames.size(); t++)
   {
      gispoints_tablenames.push_back(pi.gispoints_tablenames[t]);
   }
   
   gislines_tablenames.clear();
   for (unsigned int t=0; t<pi.gislines_tablenames.size(); t++)
   {
      gislines_tablenames.push_back(pi.gislines_tablenames[t]);
   }

   gispolys_tablenames.clear();
   for (unsigned int t=0; t<pi.gispolys_tablenames.size(); t++)
   {
      gispolys_tablenames.push_back(pi.gispolys_tablenames[t]);
   }

   video_corner_vertices.clear();
   for (unsigned int c=0; c<pi.video_corner_vertices.size(); c++)
   {
      video_corner_vertices.push_back(pi.video_corner_vertices[c]);
   }

   bbox_top_left_corners.clear();
   bbox_bottom_right_corners.clear();
   bbox_colors.clear();
   bbox_labels.clear();
   bbox_label_colors.clear();
   for (unsigned int b=0; b<pi.bbox_top_left_corners.size(); b++)
   {
      bbox_top_left_corners.push_back(pi.bbox_top_left_corners[b]);
      bbox_bottom_right_corners.push_back(pi.bbox_bottom_right_corners[b]);
   }
   for (unsigned int b=0; b<pi.bbox_colors.size(); b++)
   {
      bbox_colors.push_back(pi.bbox_colors[b]);
   }

   for (unsigned int b=0; b<pi.bbox_labels.size(); b++)
   {
      bbox_labels.push_back(pi.bbox_labels[b]);
      bbox_label_colors.push_back(pi.bbox_label_colors[b]);
   }

   ROI_skeleton_height=pi.ROI_skeleton_height;
   ROI_skeleton_color=pi.ROI_skeleton_color;

//   cout << "P_ptr = " << P_ptr << endl;
//   cout << "pi.P_ptr = " << pi.P_ptr << endl;

   if (pi.P_ptr==NULL)
   {
      delete P_ptr;
      P_ptr=NULL;
   }
   else
   {
      if (P_ptr==NULL)
      {
         P_ptr=new genmatrix(*pi.P_ptr);
      }
      else
      {
         *P_ptr=*pi.P_ptr;
      }
   }

   frame_times.clear();
   for (unsigned int ft=0; ft<pi.frame_times.size(); ft++)
   {
      frame_times.push_back(pi.frame_times[ft]);
   }

   posn_orientation_frames.clear();
   for (unsigned int po=0; po<pi.posn_orientation_frames.size(); po++)
   {
      posn_orientation_frames.push_back(pi.posn_orientation_frames[po]);
   }
}	

// ---------------------------------------------------------------------
// Overload = operator:

PassInfo& PassInfo::operator= (const PassInfo& pi)
{
//   cout << "inside PassInfo::operator=" << endl;
   if (this==&pi) return *this;
   docopy(pi);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const PassInfo& p)
{
   outstream << endl;
   outstream << "portrait mode flag = " << p.portrait_mode_flag << endl;
   outstream << "ID = " << p.ID << endl;
   outstream << "OSGsubPAT_ID = " << p.OSGsubPAT_ID << endl;
   outstream << "start arg index = " << p.start_argument_index 
             << " stop arg index = " << p.stop_argument_index << endl;
   outstream << "height colormap # = " << p.height_colormap_number 
             << " prob colormap # = " << p.prob_colormap_number << endl;
   outstream << "indep var = " << p.independent_var 
             << " height_colormap cyclic frac offset = " 
             << p.height_colormap_cyclic_fraction_offset 
             << " prob_colormap cyclic frac offset = " 
             << p.prob_colormap_cyclic_fraction_offset 
             << endl;
   for (unsigned int i=0; i<p.min_threshold.size(); i++)
   {
      outstream << "i = " << i << " min_thresh = "
                << p.min_threshold[i] << endl;
      outstream << "i = " << i << " max_thresh = "
                << p.max_threshold[i] << endl;
   }
   outstream << "min thresh frac = " << p.min_threshold_fraction 
             << " max thresh frac = " << p.max_threshold_fraction << endl;
   outstream << "long_lo = " << p.longitude_lo 
             << " long_hi = " << p.longitude_hi 
             << " lat_lo = " << p.latitude_lo
             << " lat_hi = " << p.latitude_hi << endl;
   outstream << "east_lo = " << p.easting_lo
             << " east_hi = " << p.easting_hi
             << " north_lo = " << p.northing_lo
             << " north_hi = " << p.northing_hi
             << endl;
   outstream << "altitude = " << p.altitude << endl;
   outstream << "elapsed secs since epoch lo = "
             << p.elapsed_secs_since_epoch_lo << endl;
   outstream << "elapsed secs since epoch hi = "
             << p.elapsed_secs_since_epoch_hi << endl;
   outstream << "height_offset = " << p.height_offset << endl;
   outstream << "start_frame_ID = " << p.start_frame_ID << endl;
   outstream << "stop_frame_ID = " << p.stop_frame_ID << endl;
   outstream << "photo_ID = " << p.photo_ID << endl;
   outstream << "focal length = " << p.focal_length
             << " Uaxis_focal_length = " << p.Uaxis_focal_length
             << " Vaxis_focal_length = " << p.Vaxis_focal_length << endl;
   outstream << "U0 = " << p.U0 << " V0 = " << p.V0 << endl;
   outstream << "Pixel skew angle = " << p.pixel_skew_angle*180/PI 
             << " degs" << endl;
   outstream << " rel az = " << p.relative_az
             << " rel el = " << p.relative_el
             << " rel roll = " << p.relative_roll << endl;
   outstream << " camera posn = " << p.camera_posn << endl;

   outstream << "frustum color = " << p.frustum_color << endl;
   outstream << "frustum sidelength = " << p.frustum_sidelength << endl;
   outstream << "downrange dist = " << p.downrange_distance << endl;
   outstream << "magnetic yaw = " << p.magnetic_yaw << endl;
   outstream << "filter alpha value = " << p.filter_alpha_value << endl;
   outstream << "PostGIS_hostname = " << p.PostGIS_hostname << endl;
   outstream << "PostGIS_database_name = " << p.PostGIS_database_name << endl;
   outstream << "PostGIS_username = " << p.PostGIS_username << endl;
   outstream << "gispoints_tablenames = " << endl;
   templatefunc::printVector(p.gispoints_tablenames);
   outstream << "gislines_tablenames = " << endl;
   templatefunc::printVector(p.gislines_tablenames);
   outstream << "gispolys_tablenames = " << endl;
   templatefunc::printVector(p.gispolys_tablenames);
   outstream << "Video corner vertices = " << endl;
   templatefunc::printVector(p.video_corner_vertices);
   outstream << "Bounding box top left corners = " << endl;
   templatefunc::printVector(p.bbox_top_left_corners);
   outstream << "Bounding box bottom right corners = " << endl;
   templatefunc::printVector(p.bbox_bottom_right_corners);
   outstream << "Bounding box colors = " << endl;
   templatefunc::printVector(p.bbox_colors);
   outstream << "Bounding box labels = " << endl;
   templatefunc::printVector(p.bbox_labels);
   outstream << "Bounding box label colors = " << endl;
   templatefunc::printVector(p.bbox_label_colors);

   outstream << "ROI skeleton height = " << p.ROI_skeleton_height << endl;
   outstream << "ROI skeleton color = " << p.ROI_skeleton_color << endl;
   
   outstream << "ActiveMQ_hostname = " << p.ActiveMQ_hostname << endl;

   if (p.P_ptr==NULL)
   {
      outstream << "Projection matrix ptr = NULL" << endl;
   }
   else
   {
      outstream << "Projection matrix = " << *(p.P_ptr) << endl;
   }

   outstream << "Frame times = " << endl;
   templatefunc::printVector(p.frame_times);

   return(outstream);
}

// ---------------------------------------------------------------------
void PassInfo::clear_params()
{
//   cout << "inside PassInfo::clear_params()" << endl;
//    cout << "this = " << this << endl;

   portrait_mode_flag=false;
   ID=OSGsubPAT_ID=-1;
   start_argument_index=stop_argument_index=-1;
   height_colormap_number=prob_colormap_number=-1;

   height_colormap_cyclic_fraction_offset=
      prob_colormap_cyclic_fraction_offset=0;
   min_threshold_fraction=max_threshold_fraction=-1;   

   longitude_lo=longitude_hi=latitude_lo=latitude_hi=altitude=0;
   easting_lo=easting_hi=northing_lo=northing_hi=0;
   elapsed_secs_since_epoch_lo=elapsed_secs_since_epoch_hi=0;

   start_frame_ID=stop_frame_ID=photo_ID=-1;
   focal_length=U0=V0=-1;
   Uaxis_focal_length=Vaxis_focal_length=POSITIVEINFINITY;
   pixel_skew_angle=90*PI/180;	// rads
   relative_az=relative_el=relative_roll=magnetic_yaw=0;
   camera_posn=Zero_vector;
   frustum_color="null";
   frustum_sidelength=-1;	// meters
   downrange_distance=-1;
   filter_alpha_value=-1;

   if (P_ptr != NULL) P_ptr->clear_values();
}

// ---------------------------------------------------------------------
void PassInfo::set_projection_matrix(const genmatrix* proj_ptr)
{
//   cout << "inside PassInfo::set_projection_matrix()" << endl;
   if (proj_ptr->get_mdim() != 3 || proj_ptr->get_ndim() != 4)
   {
      cout << "Error in PassInfo::set_projection_matrix()" << endl;
      cout << "Input matrix: mdim = " << proj_ptr->get_mdim() 
           << " ndim = " << proj_ptr->get_ndim() << endl;
      return;
   }
   
   P_ptr=new genmatrix(3,4);
   for (unsigned int r=0; r<3; r++)
   {
      for (unsigned int c=0; c<4; c++)
      {
         P_ptr->put(r,c,proj_ptr->get(r,c));
      }
   }
//   cout << "*P_ptr = " << *P_ptr << endl;
}
