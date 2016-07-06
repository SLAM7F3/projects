// ==========================================================================
// Header file for PassesGroup class.  
// ==========================================================================
// Last updated on 1/28/13; 5/26/13; 5/29/13
// ==========================================================================

#ifndef PASSESGROUP_H
#define PASSESGROUP_H

#include <osg/ArgumentParser>
#include <string>
#include <vector>
#include "passes/Pass.h"
#include "passes/PassInfo.h"

class PassesGroup
{

  public:

   enum ClassificationType
   {
      unclassified, FOUO, secret
   };

   PassesGroup();
   PassesGroup(std::string pass_filename);
   PassesGroup(const std::vector<std::string>& pass_filenames);
   PassesGroup(osg::ArgumentParser* ap_ptr);
   ~PassesGroup();

// Set & get methods:

   bool get_pick_points_on_Zplane_flag() const;
   bool get_continuously_update_UAV_paths_flag() const;
   ClassificationType get_classification() const;
   int get_campaign_ID() const;
   int get_mission_ID() const;
   int get_graph_hierarchy_ID() const;
   int get_graph_component_ID() const;
   int get_max_child_node_ID() const;

   int get_n_total_nodes_level0() const;
   int get_n_total_nodes_level1() const;
   int get_n_total_nodes_level2() const;
   int get_n_total_links_level0() const;
   int get_n_total_links_level1() const;
   int get_n_total_links_level2() const;

   double get_line_width() const;
   double get_world_time_step() const;
   double get_virtual_horiz_FOV() const;
   std::string get_initial_mode_string() const;
   std::string get_world_start_UTC_string() const;
   std::string get_world_stop_UTC_string() const;
   void set_image_list_filename(std::string filename);
   std::string get_image_list_filename() const;
   std::string get_image_sizes_filename() const;
   std::string get_bundle_filename() const;
   std::string get_camera_views_filename() const;
   std::string get_xyz_points_filename() const;
   std::string get_photoids_xyzids_filename() const;
   std::string get_edgelist_filename() const;
   std::string get_common_planes_filename() const;
   double get_fitted_world_to_bundler_distance_ratio() const;
   threevector get_bundler_translation() const;
   double get_global_az() const;
   double get_global_el() const;
   double get_global_roll() const;
   const threevector& get_bundler_rotation_origin() const;

   int get_n_ROI_states() const;
   std::string get_OSGButtonServer_URL() const;
   std::string get_SKSDataServer_URL() const;
   std::string get_SKSDataServer_query_type() const;
   std::string get_LogicServer_URL() const;
   std::string get_VideoServer_URL() const;
   std::string get_HTMLServer_URL() const;
   std::string get_Dynamic_WikiPage_URL() const;
   std::string get_broker_URL() const;
   std::string get_message_queue_channel_name() const;

   void set_currpass_ID(int id); // For future "Google Earth" switching 
				 //    between passes
   int get_currpass_ID() const;
   unsigned int get_n_passes() const;
   Pass* get_pass_ptr(int ID);
   const Pass* get_pass_ptr(int ID) const;
   PassInfo& get_currPassInfo();
   const PassInfo& get_currPassInfo() const;
   
   PassInfo* get_passinfo_ptr(int ID);
   const PassInfo* get_passinfo_ptr(int ID) const;

   Pass* get_videopass_ptr();
   const Pass* get_videopass_ptr() const;
   int get_videopass_ID() const;
   int get_curr_cloudpass_ID() const;
   int get_curr_texturepass_ID() const;
   std::vector<int> get_GISlayer_IDs() const;
   std::vector<int> get_dataserver_IDs() const;
   int get_curr_sensormetadatapass_ID() const;

// Argument parsing member functions:

   void import_arguments(const std::vector<std::string>& input_args);
   void interpret_arguments();
   void interpret_arguments(const std::vector<std::string>& ext_filenames);
   void interpret_current_arguments();
   void get_arguments(
      int a,std::string& curr_argument,std::string& next_argument);
   std::vector<std::string> segment_argument_list(
      osg::ArgumentParser* arg_parser_ptr,bool command_line_flag=true);
   bool check_suffix_match(
      std::string prev_suffix,std::string curr_suffix,std::string next_suffix,
      std::string param_description);
   std::vector<std::string> parse_ext_file_args(std::string ext_filename);
   void compute_pass_bounding_argument_indices(
      int& pass_start_index,int& pass_stop_index,int next_index);

// Note added on 5/10/10: generate_new_pass() member functions should
// really return Pass* rather than either bool or int !!!

   bool generate_new_pass(int r,int ID);
   int generate_new_pass(std::string pass_filename,int specified_ID=-1);
   int generate_new_pass(
      std::string pass_filename,Pass::PassType pass_type,
      Pass::InputFileType input_filetype,int specified_ID=-1);

   void generate_passes_from_arguments();

  private:

   bool pick_points_on_Zplane_flag;
   bool continuously_update_UAV_paths_flag;
   ClassificationType classification;
   int currpass_ID,n_ROI_states;
   int campaign_ID,mission_ID;
   int graph_hierarchy_ID,graph_component_ID,max_child_node_ID;
   int n_total_nodes_level0,n_total_nodes_level1,n_total_nodes_level2;
   int n_total_links_level0,n_total_links_level1,n_total_links_level2;

   double line_width,world_time_step;
   double virtual_horiz_FOV,fitted_world_to_bundler_distance_ratio;
   double global_az,global_el,global_roll;
   threevector bundler_translation,bundler_rotation_origin;
   std::string world_start_UTC_string,world_stop_UTC_string;
   std::string initial_mode_string;
   std::string image_list_filename,image_sizes_filename,bundle_filename;
   std::string camera_views_filename,xyz_pnts_filename;
   std::string photoids_xyzids_filename,edgelist_filename;
   std::string common_planes_filename;
   std::string OSGButtonServer_URL,SKSDataServer_URL,HTMLServer_URL;
   std::string Dynamic_WikiPage_URL,LogicServer_URL,VideoServer_URL;
   std::string broker_URL,message_queue_channel_name;
   std::string SKSDataServer_query_type;
   osg::ArgumentParser* argument_parser_ptr;
   std::vector<std::string> arguments;
   PassInfo currPassInfo;
   std::vector<PassInfo*> pass_metadata_ptrs;
   std::vector<Pass::PassType> pass_types;

   int curr_imagenumber,cumulative_imagecounter;

// STL vector pass_ptr_list tallies pass filename, pass file type,
// pass number and number of images which each independent data pass
// contains.  The size of the STL vector is the total number of passes
// registered within the current PassesGroup object.

   std::vector<Pass*> pass_ptr_list;

   void allocate_member_objects();
   void initialize_member_objects();


   void parse_package_arguments(
      std::string curr_argument,std::string next_argument,
      std::vector<std::string>& new_arguments);

   Pass::InputFileType determine_file_and_pass_type_from_suffix(
      std::string suffix,Pass::PassType& pass_type);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline bool PassesGroup::get_pick_points_on_Zplane_flag() const
{
   return pick_points_on_Zplane_flag;
}

inline bool PassesGroup::get_continuously_update_UAV_paths_flag() const
{
   return continuously_update_UAV_paths_flag;
}

inline PassesGroup::ClassificationType PassesGroup::get_classification() const
{
   return classification;
}

inline int PassesGroup::get_campaign_ID() const
{
   return campaign_ID;
}

inline int PassesGroup::get_mission_ID() const
{
   return mission_ID;
}

inline int PassesGroup::get_graph_hierarchy_ID() const
{
   return graph_hierarchy_ID;
}

inline int PassesGroup::get_graph_component_ID() const
{
   return graph_component_ID;
}

inline int PassesGroup::get_max_child_node_ID() const
{
   return max_child_node_ID;
}

inline int PassesGroup::get_n_total_nodes_level0() const
{
   return n_total_nodes_level0;
}

inline int PassesGroup::get_n_total_nodes_level1() const
{
   return n_total_nodes_level1;
}

inline int PassesGroup::get_n_total_nodes_level2() const
{
   return n_total_nodes_level2;
}

inline int PassesGroup::get_n_total_links_level0() const
{
   return n_total_links_level0;
}

inline int PassesGroup::get_n_total_links_level1() const
{
   return n_total_links_level1;
}

inline int PassesGroup::get_n_total_links_level2() const
{
   return n_total_links_level2;
}

inline int PassesGroup::get_n_ROI_states() const
{
   return n_ROI_states;
}

inline double PassesGroup::get_line_width() const
{
   return line_width;
}

inline double PassesGroup::get_world_time_step() const
{
   return world_time_step;
}

inline double PassesGroup::get_virtual_horiz_FOV() const
{
   return virtual_horiz_FOV;
}

inline double PassesGroup::get_fitted_world_to_bundler_distance_ratio() const
{
   return fitted_world_to_bundler_distance_ratio;
}

inline threevector PassesGroup::get_bundler_translation() const
{
   return bundler_translation;
}

inline double PassesGroup::get_global_az() const
{
   return global_az;
}

inline double PassesGroup::get_global_el() const
{
   return global_el;
}

inline double PassesGroup::get_global_roll() const
{
   return global_roll;
}

inline const threevector& PassesGroup::get_bundler_rotation_origin() const
{
   return bundler_rotation_origin;
}

inline void PassesGroup::set_image_list_filename(std::string filename)
{
   image_list_filename=filename;
}

inline std::string PassesGroup::get_image_list_filename() const
{
   return image_list_filename;
}

inline std::string PassesGroup::get_image_sizes_filename() const
{
   return image_sizes_filename;
}

inline std::string PassesGroup::get_bundle_filename() const
{
   return bundle_filename;
}

inline std::string PassesGroup::get_camera_views_filename() const
{
   return camera_views_filename;
}

inline std::string PassesGroup::get_xyz_points_filename() const
{
   return xyz_pnts_filename;
}

inline std::string PassesGroup::get_photoids_xyzids_filename() const
{
   return photoids_xyzids_filename;
}

inline std::string PassesGroup::get_edgelist_filename() const
{
   return edgelist_filename;
}

inline std::string PassesGroup::get_common_planes_filename() const
{
   return common_planes_filename;
}

inline std::string PassesGroup::get_world_start_UTC_string() const
{
   return world_start_UTC_string;
}

inline std::string PassesGroup::get_world_stop_UTC_string() const
{
   return world_stop_UTC_string;
}

inline std::string PassesGroup::get_initial_mode_string() const
{
   return initial_mode_string;
}

inline std::string PassesGroup::get_OSGButtonServer_URL() const
{
   return OSGButtonServer_URL;
}

inline std::string PassesGroup::get_SKSDataServer_URL() const
{
   return SKSDataServer_URL;
}

inline std::string PassesGroup::get_SKSDataServer_query_type() const
{
   return SKSDataServer_query_type;
}

inline std::string PassesGroup::get_LogicServer_URL() const
{
   return LogicServer_URL;
}

inline std::string PassesGroup::get_VideoServer_URL() const
{
   return VideoServer_URL;
}

inline std::string PassesGroup::get_HTMLServer_URL() const
{
   return HTMLServer_URL;
}

inline std::string PassesGroup::get_Dynamic_WikiPage_URL() const
{
   return Dynamic_WikiPage_URL;
}

inline std::string PassesGroup::get_broker_URL() const
{
   return broker_URL;
}

inline std::string PassesGroup::get_message_queue_channel_name() const
{
   return message_queue_channel_name;
}

inline void PassesGroup::set_currpass_ID(int id)
{
   currpass_ID=id;
}

inline int PassesGroup::get_currpass_ID() const
{
   return currpass_ID;
}

inline PassInfo& PassesGroup::get_currPassInfo()
{
   return currPassInfo;
}

inline const PassInfo& PassesGroup::get_currPassInfo() const
{
   return currPassInfo;
}

inline PassInfo* PassesGroup::get_passinfo_ptr(int id)
{
   if (id < 0 || id >= int(pass_metadata_ptrs.size()))
   {
      return NULL;
   }
   else 
   {
      return pass_metadata_ptrs.at(id);
   }
}

inline const PassInfo* PassesGroup::get_passinfo_ptr(int id) const
{
   if (id < 0 || id >= int(pass_metadata_ptrs.size()))
   {
      return NULL;
   }
   else 
   {
      return pass_metadata_ptrs.at(id);
   }
}

#endif // PassesGroup.h

