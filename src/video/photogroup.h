// ==========================================================================
// Header file for photogroup class
// ==========================================================================
// Last modified on 12/29/13; 4/5/14; 4/6/14; 6/7/14
// ==========================================================================

#ifndef PHOTOGROUP_H
#define PHOTOGROUP_H

#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "geometry/bounding_box.h"
#include "color/colorfuncs.h"
#include "math/fourvector.h"
#include "graphs/graph.h"
#include "graphs/graphfuncs.h"
#include "messenger/Messenger.h"
#include "video/photograph.h"

class plane;
class PassesGroup;

class photogroup : public graph
{

  public:

   typedef std::map<int,plane*> IMAGE_PLANES_MAP;
   typedef std::map<std::pair<int,int>,plane*> COMMON_PLANES_MAP;

// Initialization, constructor and destructor functions:

   photogroup(int ID=0,int level=0);
   photogroup(const photogroup& pg);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~photogroup();
   photogroup& operator= (const photogroup& pg);
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const photogroup& pg);

// Set and get member functions:

   void set_base_URL(std::string URL);
   std::string get_base_URL() const;
   unsigned int get_n_photos() const;
   void set_selected_photo_ID(int ID);
   int get_selected_photo_ID() const;
   void set_northern_hemisphere_flag(bool flag);
   bool get_northern_hemisphere_flag() const;
   void set_UTM_zonenumber(int zone);
   int get_UTM_zonenumber() const;

   std::vector<int>& get_photo_order();
   int get_photo_index_given_order(int p);
   int get_photo_order_given_index(int i);
   void set_photo_order(std::string filename);
   void set_photo_order_equal_to_ID();

   void set_bundler_to_world_scalefactor(double s);
   double get_bundler_to_world_scalefactor() const;
   void set_bundler_to_world_az(double a);
   double get_bundler_to_world_az() const;
   void set_bundler_to_world_el(double e);
   double get_bundler_to_world_el() const;
   void set_bundler_to_world_roll(double r);
   double get_bundler_to_world_roll() const;
   void set_bundler_to_world_translation(const threevector& trans);
   const threevector& get_bundler_to_world_translation() const;
   void set_bundler_IO_subdir(std::string subdir);
   std::string get_bundler_IO_subdir() const;

   void set_bbox();
   bounding_box& get_bbox();
   const bounding_box& get_bbox() const;

   void set_Messenger_ptr(Messenger* M_ptr);
   Messenger* get_Messenger_ptr();
   const Messenger* get_Messenger_ptr() const;
   photogroup* get_cluster_photogroup_ptr();
   const photogroup* get_cluster_photogroup_ptr() const;

// Photogroup construction & manipulation member functions:
   
   photograph* generate_single_photograph(std::string photo_filename);
   void destroy_single_photograph(photograph* photo_ptr);
   photograph* generate_single_photograph(
      std::string photo_filename,int xdim,int ydim,
      double fu,double fv,double U0,double V0,double az,double el,double roll,
      const threevector& camera_posn,double frustum_sidelength);
   void read_photographs(PassesGroup& passes_group);
   void read_photographs(
      PassesGroup& passes_group,std::string image_sizes_filename);
   void read_photographs(std::string images_subdir);

   double average_camera_elevation();

   std::vector<std::string> import_image_filenames(
      std::string images_subdir,std::string image_list_filename,
      int& n_photos_to_reconstruct);
   void import_image_sizes(std::string image_sizes_filename,
                           std::vector<int>& xdim,std::vector<int>& ydim);
   void export_image_sizes(std::string output_filename);
   void estimate_internal_camera_params(double FOV_u=NEGATIVEINFINITY);
   
   photograph* get_photograph_ptr(int ID);
   const photograph* get_photograph_ptr(int ID) const;
   photograph* get_ordered_photograph_ptr(int p);
   const photograph* get_ordered_photograph_ptr(int p) const;
   photograph* get_selected_photograph_ptr();
   const photograph* get_selected_photograph_ptr() const;
   void destroy_all_photos();

// Subgroup member functions:

   photogroup* generate_subgroup(unsigned int ID_start,unsigned int ID_stop);

// Photograph parameter manipulation member functions:

   void save_initial_camera_f_az_el_roll_params();
   void restore_initial_camera_f_az_el_roll_params();
   void globally_reset_camera_world_posn(const threevector& posn);
   void rescale_focal_lengths(double scale_factor);
   void globally_rotate(const rotation& R_global);
   void export_photo_parameters(
      std::string packages_subdir,bool photos_ordered_flag=true,
      double frustum_sidelength=-1);

// Photo resizing member functions:

   void generate_thumbnails();
   void downsize_photos(unsigned int max_xdim,unsigned int max_ydim);
   void standard_size_photos(
      unsigned int output_xdim,unsigned int output_ydim,
      std::string standard_sized_images_subdir);

// Bundler member functions:

   void generate_bundler_photographs(
      std::string bundler_IO_subdir,std::string image_list_filename,
      int n_photos_to_reconstruct=-1,
      bool parse_exift_metadata_flag=false,
      bool check_for_corrupted_images_flag=false);
   void generate_bundler_photographs(
      std::string bundler_IO_subdir,std::string image_list_filename,
      std::string image_sizes_filename,int n_photos_to_reconstruct=-1,
      bool parse_exift_metadata_flag=false,
      bool check_for_corrupted_images_flag=false);
   void generate_bundler_photographs(
      const std::vector<std::string>& image_filenames,
      const std::vector<int>& xdim,const std::vector<int>& ydim,
      bool parse_exif_metadata_flag=false,
      bool check_for_corrupted_images_flag=false);
   

   void reconstruct_bundler_cameras(
      std::string bundler_IO_subdir,std::string image_list_filename,
      std::string image_sizes_filename,std::string bundle_filename,
      int n_photos_to_reconstruct=-1);
   void reconstruct_bundler_cameras(
      std::string bundle_filename, int n_photos_to_reconstruct=-1);
   void compute_cameras_COM_and_plane();
   void print_bundler_to_world_params();
   void read_photograph_covariance_traces(std::string covariances_filename);

//   void export_bundle_dot_out();

// Image plane member functions:

   plane* get_image_plane(int i);
   void import_common_photo_planes(std::string common_planes_filename);
   plane* get_common_image_plane(int i,int j);
   std::vector<plane*> get_overlapping_image_planes(int photo_ID);
   std::vector<int> get_overlapping_imageplane_orders(int photo_ID);

// Message handling member functions:

   void issue_add_vertex_message(photograph* photograph_ptr);
   void issue_add_edge_message(
      photograph* photograph1_ptr,photograph* photograph2_ptr,
      int n_SIFT_matches);
   void issue_delete_vertex_message(photograph* photograph_ptr);
   void issue_delete_edge_message(photograph* photograph1_ptr,
                                  photograph* photograph2_ptr);
   void issue_delete_all_message();

   void issue_show_banner_message();
   void issue_hide_banner_message();
   void issue_set_banner_message(std::string banner);

// Graph building via GraphML JSON commands:

   void issue_build_graph_message(std::string JSON_URL);

// Photo ordering member functions:

   void order_photos_by_their_scores();
   photograph* get_score_ordered_photo(int i);
   void print_compositing_order();

// Photo clustering member functions:

   photogroup* generate_cluster_photogroup();
   photogroup* generate_grandparent_photogroup();

// SQL generation member functions:

   void write_SQL_insert_photo_commands(std::string SQL_photo_filename);
   std::string output_photo_to_SQL(photograph* photograph_ptr);

// Virtual camera member functions:

   photograph* generate_blank_photograph(
      double horiz_FOV,double vert_FOV,
      double az, double el,double roll,const threevector& camera_posn,
      double frustum_sidelength,double blank_grey_level=0.5);
   std::string generate_blank_imagefile(
      int n_horiz_pixels,int n_vertical_pixels);

  private:

   bool northern_hemisphere_flag;
   int n_params_per_photo,selected_photo_ID;
   int UTM_zonenumber;
   double bundler_to_world_scalefactor;
   double bundler_to_world_az,bundler_to_world_el,bundler_to_world_roll;
   std::string base_URL,bundler_IO_subdir;
   threevector bundler_to_world_translation;
   bounding_box photo_bbox;

   std::vector<int> photo_order;

   std::vector<photograph*> score_ordered_photo_ptrs;
   IMAGE_PLANES_MAP* image_planes_map_ptr;
   COMMON_PLANES_MAP* common_planes_map_ptr;
   Messenger* Messenger_ptr;
   photogroup* cluster_photogroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const photogroup& m);

   void push_back_photo_ID_onto_photo_order(int ID);
   colorfunc::RGB compute_node_color(photograph* photograph_ptr);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline unsigned int photogroup::get_n_photos() const
{
   return get_n_nodes();
}

// ---------------------------------------------------------------------
inline void photogroup::set_selected_photo_ID(int ID)
{
   selected_photo_ID=ID;
}

inline int photogroup::get_selected_photo_ID() const
{
   return selected_photo_ID;
}

// ---------------------------------------------------------------------
inline void photogroup::set_northern_hemisphere_flag(bool flag)
{
   northern_hemisphere_flag=flag;
}

inline bool photogroup::get_northern_hemisphere_flag() const
{
   return northern_hemisphere_flag;
}

inline void photogroup::set_UTM_zonenumber(int zone)
{
   UTM_zonenumber=zone;
}

inline int photogroup::get_UTM_zonenumber() const
{
   return UTM_zonenumber;
}

// ---------------------------------------------------------------------
inline std::vector<int>& photogroup::get_photo_order()
{
   return photo_order;
}

// ---------------------------------------------------------------------
inline int photogroup::get_photo_index_given_order(int p)
{
   return photo_order[p];
}

// ---------------------------------------------------------------------
inline int photogroup::get_photo_order_given_index(int i)
{
   for (int p=0; p<int(photo_order.size()); p++)
   {
      if (photo_order[p]==i) return p;
   }
   return -1;
}

// ---------------------------------------------------------------------
inline void photogroup::set_bundler_to_world_scalefactor(double s)
{
   bundler_to_world_scalefactor=s;
}

inline double photogroup::get_bundler_to_world_scalefactor() const
{
   return bundler_to_world_scalefactor;
}

// ---------------------------------------------------------------------
inline void photogroup::set_bundler_to_world_az(double a)
{
   bundler_to_world_az=a;
}

inline double photogroup::get_bundler_to_world_az() const
{
   return bundler_to_world_az;
}

inline void photogroup::set_bundler_to_world_el(double e)
{
   bundler_to_world_el=e;
}

inline double photogroup::get_bundler_to_world_el() const
{
   return bundler_to_world_el;
}

inline void photogroup::set_bundler_to_world_roll(double r)
{
   bundler_to_world_roll=r;
}

inline double photogroup::get_bundler_to_world_roll() const
{
   return bundler_to_world_roll;
}

// ---------------------------------------------------------------------
inline void photogroup::set_bundler_to_world_translation(
   const threevector& trans)
{
   bundler_to_world_translation=trans;
}

inline const threevector& photogroup::get_bundler_to_world_translation() const
{
   return bundler_to_world_translation;
}

inline void photogroup::set_bundler_IO_subdir(std::string subdir)
{
   bundler_IO_subdir=subdir;
}

inline std::string photogroup::get_bundler_IO_subdir() const
{
   return bundler_IO_subdir;
}

// ---------------------------------------------------------------------
inline bounding_box& photogroup::get_bbox()
{
   return photo_bbox;
}

inline const bounding_box& photogroup::get_bbox() const
{
   return photo_bbox;
}

// ---------------------------------------------------------------------
inline void photogroup::set_Messenger_ptr(Messenger* M_ptr)
{
   Messenger_ptr=M_ptr;
}

inline Messenger* photogroup::get_Messenger_ptr()
{
   return Messenger_ptr;
}

inline const Messenger* photogroup::get_Messenger_ptr() const
{
   return Messenger_ptr;
}

// ---------------------------------------------------------------------
inline photogroup* photogroup::get_cluster_photogroup_ptr()
{
   return cluster_photogroup_ptr;
}

inline const photogroup* photogroup::get_cluster_photogroup_ptr() const
{
   return cluster_photogroup_ptr;
}


#endif  // photogroup.h



