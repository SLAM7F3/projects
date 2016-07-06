// ==========================================================================
// PassesGroup class member function definitions
// ==========================================================================
// Last updated on 7/3/13; 8/5/13; 8/12/13; 3/28/14
// ==========================================================================

#include <iostream>
#include <set>
#include "general/filefuncs.h"
#include "astro_geo/Clock.h"
#include "math/constant_vectors.h"
#include "math/genmatrix.h"
#include "passes/PassesGroup.h"
#include "math/rpy.h"
#include "general/stringfuncs.h"
#include "datastructures/Triple.h"

#include "general/outputfuncs.h"
#include "templates/mytemplates.h"

using std::cout;
using std::endl;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:

void PassesGroup::allocate_member_objects()
{
}

void PassesGroup::initialize_member_objects()
{
   pick_points_on_Zplane_flag=continuously_update_UAV_paths_flag=false;
   classification=unclassified;
   graph_component_ID=0;
   campaign_ID=mission_ID=graph_hierarchy_ID=max_child_node_ID=-1;
   n_total_nodes_level0=n_total_nodes_level1=n_total_nodes_level2=0;
   n_total_links_level0=n_total_links_level1=n_total_links_level2=0;
   line_width=4;	// default value for ISDS3D laptop
   n_ROI_states=2;
   virtual_horiz_FOV=-1;
   fitted_world_to_bundler_distance_ratio=1;
   bundler_translation=Zero_vector;
   global_az=global_el=global_roll=0;
   world_time_step=0.5;	// secs
   world_start_UTC_string=world_stop_UTC_string="";
   initial_mode_string=OSGButtonServer_URL=SKSDataServer_URL=
      LogicServer_URL=VideoServer_URL=HTMLServer_URL=Dynamic_WikiPage_URL="";
   broker_URL=message_queue_channel_name="";
   SKSDataServer_query_type="vehicle";

   argument_parser_ptr=NULL;
   curr_imagenumber=cumulative_imagecounter=0;
   currpass_ID=0;
   pass_ptr_list.clear();
}

PassesGroup::PassesGroup()
{
   allocate_member_objects();
   initialize_member_objects();
}

PassesGroup::PassesGroup(string pass_filename)
{
//   cout << "inside PassesGroup constructor #2" << endl;
   allocate_member_objects();
   initialize_member_objects();
   arguments.push_back(pass_filename);

   int pass_start_index=0;
   int pass_stop_index=0;
   compute_pass_bounding_argument_indices(pass_start_index,pass_stop_index,0);
}

PassesGroup::PassesGroup(const vector<string>& pass_filenames)
{
//   cout << "inside PassesGroup constructor #3" << endl;
   allocate_member_objects();
   initialize_member_objects();

   import_arguments(pass_filenames);
   generate_passes_from_arguments();
}

PassesGroup::PassesGroup(osg::ArgumentParser* ap_ptr)
{
//   cout << "inside PassesGroup constructor #4" << endl;
   allocate_member_objects();
   initialize_member_objects();
   argument_parser_ptr=ap_ptr;

   interpret_arguments();
   generate_passes_from_arguments();
}

PassesGroup::~PassesGroup()
{
   for (unsigned int i=0; i<pass_ptr_list.size(); i++)
   {
      delete pass_ptr_list[i];
   }
   for (unsigned int i=0; i<pass_metadata_ptrs.size(); i++)
   {
      delete pass_metadata_ptrs[i];
   }
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

unsigned int PassesGroup::get_n_passes() const
{
   return pass_ptr_list.size(); 
}

// ---------------------------------------------------------------------
Pass* PassesGroup::get_pass_ptr(int ID) 
{
//   cout << "inside PassesGroup::get_pass_ptr()" << endl;
   for (unsigned int i = 0; i < pass_ptr_list.size(); i++)
   {
      if (pass_ptr_list[i]->get_ID() == ID) 
      {
         return pass_ptr_list[i];
      }
      
   }
   cout << "Potential trouble in PassesGroup::get_pass_ptr(int ID)" << endl;
   cout << "ID = " << ID << " does not match any pass in pass_list!"
        << endl;
   return NULL;
}

const Pass* PassesGroup::get_pass_ptr(int ID) const
{
//   cout << "inside const PassesGroup::get_pass_ptr() const" << endl;
   for (unsigned int i = 0; i < pass_ptr_list.size(); i++)
   {
      if (pass_ptr_list[i]->get_ID() == ID)
      {
         return pass_ptr_list[i];         
      }
      
   }
   cout << "Potential trouble in PassesGroup::get_pass_ptr(int ID)" << endl;
   cout << "ID = " << ID << " does not match any pass in pass_list!"
        << endl;
   return NULL;
}

// ---------------------------------------------------------------------
Pass* PassesGroup::get_videopass_ptr() 
{
   for (unsigned int p=0; p<pass_ptr_list.size(); p++)
   {
      Pass* curr_pass_ptr=pass_ptr_list[p];
      if (curr_pass_ptr->get_passtype()==Pass::video)
      {
         return curr_pass_ptr;
      }
   }
   return NULL;
}

const Pass* PassesGroup::get_videopass_ptr() const
{
   for (unsigned int p=0; p<pass_ptr_list.size(); p++)
   {
      Pass* curr_pass_ptr=pass_ptr_list[p];
      if (curr_pass_ptr->get_passtype()==Pass::video)
      {
         return curr_pass_ptr;
      }
   }
   return NULL;
}

int PassesGroup::get_videopass_ID() const
{
   for (unsigned int p=0; p<pass_ptr_list.size(); p++)
   {
      Pass* curr_pass_ptr=pass_ptr_list[p];
      if (curr_pass_ptr->get_passtype()==Pass::video)
      {
         return curr_pass_ptr->get_ID();
      }
   }
   return -1;
}

int PassesGroup::get_curr_cloudpass_ID() const
{
   for (unsigned int p=0; p<pass_ptr_list.size(); p++)
   {
      Pass* curr_pass_ptr=pass_ptr_list[p];
      if (curr_pass_ptr->get_passtype()==Pass::cloud)
      {
         return curr_pass_ptr->get_ID();
      }
   }
   return -1;
}

int PassesGroup::get_curr_texturepass_ID() const
{
   for (unsigned int p=0; p<pass_ptr_list.size(); p++)
   {
      Pass* curr_pass_ptr=pass_ptr_list[p];
      if (curr_pass_ptr->get_passtype()==Pass::surface_texture)
      {
         return curr_pass_ptr->get_ID();
      }
   }
   return -1;
}

vector<int> PassesGroup::get_GISlayer_IDs() const
{
//   cout << "inside PassesGroup::get_GISlayer_IDs()" << endl;
   vector<int> GISlayer_IDs;
   for (unsigned int p=0; p<pass_ptr_list.size(); p++)
   {
      Pass* curr_pass_ptr=pass_ptr_list[p];
      if (curr_pass_ptr->get_passtype()==Pass::GIS_layer)
      {
         GISlayer_IDs.push_back(curr_pass_ptr->get_ID());
//         cout << "p = " << p 
//              << " GISlayer_ID = " << GISlayer_IDs.back() << endl;
      }
   }
   return GISlayer_IDs;
}

vector<int> PassesGroup::get_dataserver_IDs() const
{
   vector<int> dataserver_IDs;
   for (unsigned int p=0; p<pass_ptr_list.size(); p++)
   {
      Pass* curr_pass_ptr=pass_ptr_list[p];
      if (curr_pass_ptr->get_passtype()==Pass::dataserver)
      {
         dataserver_IDs.push_back(curr_pass_ptr->get_ID());
      }
   }
   return dataserver_IDs;
}

int PassesGroup::get_curr_sensormetadatapass_ID() const
{
   for (unsigned int p=0; p<pass_ptr_list.size(); p++)
   {
      Pass* curr_pass_ptr=pass_ptr_list[p];
      if (curr_pass_ptr->get_passtype()==Pass::sensor_metadata)
      {
         return curr_pass_ptr->get_ID();
      }
   }
   return -1;
}

// ==========================================================================
// Argument parsing member functions
// ==========================================================================

void PassesGroup::import_arguments(const vector<string>& input_args)
{
//   cout << "inside PassesGroup::import_arguments()" << endl;
   arguments.clear();

   for (unsigned int e=0; e<input_args.size(); e++)
   {
      arguments.push_back(input_args[e]);
//      cout << "arguments.back() = " << arguments.back() << endl;
   } // loop over index e labeling external filenames

   arguments.push_back("--newpass");

//   cout << "arguments = " << endl;
//   templatefunc::printVector(arguments);

   interpret_current_arguments();

//   cout << "pass_metadata_ptrs.size() = " 
//        << pass_metadata_ptrs.size() << endl;
//   for (unsigned int i=0; i<pass_metadata_ptrs.size(); i++)
//   {
//      PassInfo* passinfo_ptr=get_passinfo_ptr(i);
//      cout << "i = " << i 
//           << " Passinfo_ptr = " << passinfo_ptr << endl;
//      cout << "*Passinfo_ptr = " << *passinfo_ptr << endl;
//   }

}

// Member function interpret_arguments

void PassesGroup::interpret_arguments()
{
   arguments.clear();
   arguments=segment_argument_list(argument_parser_ptr);
   interpret_current_arguments();
}

void PassesGroup::interpret_arguments(const vector<string>& ext_filenames)
{
//   cout << "inside PassesGroup::interpret_arguments() #2" << endl;
   arguments.clear();

//   cout << "ext_filenames.size() = " << ext_filenames.size() << endl;
   for (unsigned int e=0; e<ext_filenames.size(); e++)
   {
      vector<string> ext_arguments=parse_ext_file_args(ext_filenames[e]);
      for (unsigned int s=0; s<ext_arguments.size(); s++)
      {
         arguments.push_back(ext_arguments[s]);
      }
      arguments.push_back("--newpass");
//      cout << "arguments.back() = " << arguments.back() << endl;
   } // loop over index e labeling external filenames

//   cout << "arguments = " << endl;
//   templatefunc::printVector(arguments);
   interpret_current_arguments();

//   cout << "pass_metadata_ptrs.size() = " 
//        << pass_metadata_ptrs.size() << endl;
//   for (unsigned int i=0; i<pass_metadata_ptrs.size(); i++)
//   {
//      PassInfo* passinfo_ptr=get_passinfo_ptr(i);
//      cout << "i = " << i 
//           << " Passinfo_ptr = " << passinfo_ptr << endl;
//      cout << "*Passinfo_ptr = " << *passinfo_ptr << endl;
//   }
}

// Member function interpret_current_arguments()

void PassesGroup::interpret_current_arguments()
{
//   cout << "inside PassesGroup::interpret_current_arguments" << endl;
//   cout << "arguments.size() = " << arguments.size() << endl;
//   cout.precision(14);

   bool freeze_pass_stop_index=false;
   int pass_start_index=0;
   int pass_stop_index=pass_start_index;
   string curr_argument,next_argument;
   unsigned int a=0;
   while (a<arguments.size())
   {
      get_arguments(a,curr_argument,next_argument);
//      cout << "a = " << a << " curr_arg = " << curr_argument << endl;

      if (curr_argument=="--newpass")
      {
         pass_stop_index--;
         freeze_pass_stop_index=false;

         compute_pass_bounding_argument_indices(
            pass_start_index,pass_stop_index,a+1);
      }
      else if (curr_argument=="--continue")
      {
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--pick_points_on_Zplane_flag")
      {
         pick_points_on_Zplane_flag=stringfunc::string_to_boolean(
            next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--continuously_update_UAV_paths_flag")
      {
         continuously_update_UAV_paths_flag=stringfunc::string_to_boolean(
            next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--classification")
      {
         classification=static_cast<ClassificationType>(
            stringfunc::string_to_integer(next_argument));
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--campaign_ID")
      {
         campaign_ID=stringfunc::string_to_number(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--mission_ID")
      {
         mission_ID=stringfunc::string_to_number(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--graph_hierarchy_ID")
      {
         graph_hierarchy_ID=stringfunc::string_to_number(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--graph_component_ID")
      {
         graph_component_ID=stringfunc::string_to_number(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--max_child_node_ID")
      {
         max_child_node_ID=stringfunc::string_to_number(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--n_total_nodes_level0")
      {
         n_total_nodes_level0=stringfunc::string_to_number(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--n_total_nodes_level1")
      {
         n_total_nodes_level1=stringfunc::string_to_number(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--n_total_nodes_level2")
      {
         n_total_nodes_level2=stringfunc::string_to_number(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--n_total_links_level0")
      {
         n_total_links_level0=stringfunc::string_to_number(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--n_total_links_level1")
      {
         n_total_links_level1=stringfunc::string_to_number(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--n_total_links_level2")
      {
         n_total_links_level2=stringfunc::string_to_number(next_argument);
         freeze_pass_stop_index=true;
      }

      else if (curr_argument=="--line_width")
      {
         line_width=stringfunc::string_to_number(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--virtual_horiz_FOV")
      {
         virtual_horiz_FOV=stringfunc::string_to_number(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--n_ROI_states")
      {
         n_ROI_states=stringfunc::string_to_number(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--world_time_step")
      {
         world_time_step=stringfunc::string_to_number(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--world_start_UTC")
      {
         world_start_UTC_string=next_argument;
         cout << "world_start_UTC = " << world_start_UTC_string << endl;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--world_stop_UTC")
      {
         world_stop_UTC_string=next_argument;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--initial_mode")
      {
         initial_mode_string=next_argument;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--OSGButtonServer_URL")
      {
         OSGButtonServer_URL=next_argument;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--SKSDataServer_URL")
      {
         SKSDataServer_URL=next_argument;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--SKSDataServer_query_type")
      {
         SKSDataServer_query_type=next_argument;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--LogicServer_URL")
      {
         LogicServer_URL=next_argument;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--VideoServer_URL")
      {
         VideoServer_URL=next_argument;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--HTMLServer_URL")
      {
         HTMLServer_URL=next_argument;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--Dynamic_WikiPage_URL")
      {
         Dynamic_WikiPage_URL=next_argument;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--broker_URL")
      {
         broker_URL=next_argument;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--message_queue_channel_name")
      {
         message_queue_channel_name=next_argument;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--image_list_filename")
      {
         image_list_filename=next_argument;
//         cout << "image_list_filename = " << image_list_filename << endl;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--image_sizes_filename")
      {
         image_sizes_filename=next_argument;
//         cout << "image_sizes_filename = " << image_sizes_filename << endl;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--bundle_filename")
      {
         bundle_filename=next_argument;
//         cout << "bundle_filename = " << bundle_filename << endl;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--camera_views_filename")
      {
         camera_views_filename=next_argument;
//         cout << "camera_views_filename = " << camera_views_filename << endl;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--xyz_pnts_filename")
      {
         xyz_pnts_filename=next_argument;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--photoids_xyzids_filename")
      {
         photoids_xyzids_filename=next_argument;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--edgelist_filename")
      {
         edgelist_filename=next_argument;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--common_planes_filename")
      {
         common_planes_filename=next_argument;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--fitted_world_to_bundler_distance_ratio")
      {
         fitted_world_to_bundler_distance_ratio=
            stringfunc::string_to_number(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--bundler_translation_X")
      {
         double trans_X=stringfunc::string_to_number(next_argument);
         bundler_translation.put(0,trans_X);
//         cout << "trans_X = " << trans_X << endl;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--bundler_translation_Y")
      {
         double trans_Y=stringfunc::string_to_number(next_argument);
         bundler_translation.put(1,trans_Y);
//         cout << "trans_Y = " << trans_Y << endl;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--bundler_translation_Z")
      {
         double trans_Z=stringfunc::string_to_number(next_argument);
         bundler_translation.put(2,trans_Z);
//         cout << "trans_Z = " << trans_Z << endl;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--global_az")
      {
         global_az=stringfunc::string_to_number(next_argument);
         global_az *= PI/180;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--global_el")
      {
         global_el=stringfunc::string_to_number(next_argument);
         global_el *= PI/180;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--global_roll")
      {
         global_roll=stringfunc::string_to_number(next_argument);
         global_roll *= PI/180;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--bundler_rotation_origin_X")
      {
         double origin_X=stringfunc::string_to_number(next_argument);
         bundler_rotation_origin.put(0,origin_X);
//         cout << "origin_X = " << origin_X << endl;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--bundler_rotation_origin_Y")
      {
         double origin_Y=stringfunc::string_to_number(next_argument);
         bundler_rotation_origin.put(1,origin_Y);
//         cout << "origin_Y = " << origin_Y << endl;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--bundler_rotation_origin_Z")
      {
         double origin_Z=stringfunc::string_to_number(next_argument);
         bundler_rotation_origin.put(2,origin_Z);
//         cout << "origin_Z = " << origin_Z << endl;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--package_subdir")
      {
         currPassInfo.set_package_subdir(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--package_filename_prefix")
      {
         currPassInfo.set_package_filename_prefix(next_argument);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--portrait_mode")
      {
         bool portrait_mode_flag=stringfunc::string_to_boolean(
            next_argument);
         currPassInfo.set_portrait_mode_flag(portrait_mode_flag);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--OSGsubPAT_ID")
      {
         int OSGsubPAT_ID=stringfunc::string_to_number(next_argument);
         currPassInfo.set_OSGsubPAT_ID(OSGsubPAT_ID);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--height_colormap")
      {
         int height_colormap_number=stringfunc::string_to_number(
            next_argument);
         currPassInfo.set_height_colormap_number(height_colormap_number);
//         cout << "height_colormap_number = " 
//              << currPassInfo.get_height_colormap_number() << endl;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--prob_colormap")
      {
         int prob_colormap_number=stringfunc::string_to_number(
            next_argument);
         currPassInfo.set_prob_colormap_number(prob_colormap_number);
//         cout << "prob_colormap_number = " 
//              << currPassInfo.get_prob_colormap_number() << endl;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--independent_var")
      {
         int indep_var=stringfunc::string_to_number(next_argument);
         currPassInfo.set_independent_variable(indep_var);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--height_cyclic_frac_offset")
      {
         double cyclic_frac_offset=stringfunc::string_to_number(
            next_argument);
         currPassInfo.set_height_colormap_cyclic_fraction_offset(
            cyclic_frac_offset);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--prob_cyclic_frac_offset")
      {
         double cyclic_frac_offset=stringfunc::string_to_number(
            next_argument);
         currPassInfo.set_prob_colormap_cyclic_fraction_offset(
            cyclic_frac_offset);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--max_x_threshold")
      {
         double max_threshold=stringfunc::string_to_number(next_argument);
         currPassInfo.set_max_threshold(0,max_threshold);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--min_x_threshold")
      {
         double min_threshold=stringfunc::string_to_number(next_argument);
         currPassInfo.set_min_threshold(0,min_threshold);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--max_y_threshold")
      {
         double max_threshold=stringfunc::string_to_number(next_argument);
         currPassInfo.set_max_threshold(1,max_threshold);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--min_y_threshold")
      {
         double min_threshold=stringfunc::string_to_number(next_argument);
         currPassInfo.set_min_threshold(1,min_threshold);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--max_z_threshold")
      {
         double max_threshold=stringfunc::string_to_number(next_argument);
         currPassInfo.set_max_threshold(2,max_threshold);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--min_z_threshold")
      {
         double min_threshold=stringfunc::string_to_number(next_argument);
         currPassInfo.set_min_threshold(2,min_threshold);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--max_p_threshold")
      {
         double max_threshold=stringfunc::string_to_number(next_argument);
         currPassInfo.set_max_threshold(3,max_threshold);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--min_p_threshold")
      {
         double min_threshold=stringfunc::string_to_number(next_argument);
         currPassInfo.set_min_threshold(3,min_threshold);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--max_threshold_frac")
      {
         double max_threshold_frac=stringfunc::string_to_number(
            next_argument);
         currPassInfo.set_max_threshold_fraction(max_threshold_frac);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--min_threshold_frac")
      {
         double min_threshold_frac=stringfunc::string_to_number(
            next_argument);
         currPassInfo.set_min_threshold_fraction(min_threshold_frac);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--longitude_lo")
      {
         double longitude_lo=stringfunc::string_to_number(next_argument);
         currPassInfo.set_longitude_lo(longitude_lo);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--longitude_hi")
      {
         double longitude_hi=stringfunc::string_to_number(next_argument);
         currPassInfo.set_longitude_hi(longitude_hi);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--latitude_lo")
      {
         double latitude_lo=stringfunc::string_to_number(next_argument);
         currPassInfo.set_latitude_lo(latitude_lo);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--latitude_hi")
      {
         double latitude_hi=stringfunc::string_to_number(next_argument);
         currPassInfo.set_latitude_hi(latitude_hi);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--easting_lo")
      {
         double easting_lo=stringfunc::string_to_number(next_argument);
         currPassInfo.set_easting_lo(easting_lo);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--easting_hi")
      {
         double easting_hi=stringfunc::string_to_number(next_argument);
         currPassInfo.set_easting_hi(easting_hi);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--northing_lo")
      {
         double northing_lo=stringfunc::string_to_number(next_argument);
         currPassInfo.set_northing_lo(northing_lo);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--northing_hi")
      {
         double northing_hi=stringfunc::string_to_number(next_argument);
         currPassInfo.set_northing_hi(northing_hi);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--altitude")
      {
         double altitude=stringfunc::string_to_number(next_argument);
         currPassInfo.set_altitude(altitude);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--UTM_zonenumber")
      {
         int UTM_zonenumber=stringfunc::string_to_number(next_argument);
         cout << "UTM zone = " << UTM_zonenumber << endl;
         currPassInfo.set_UTM_zonenumber(UTM_zonenumber);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--northern_hemisphere_flag")
      {
         bool flag=stringfunc::string_to_boolean(next_argument);
         cout << "hemisphere flag = " << flag << endl;
         currPassInfo.set_northern_hemisphere_flag(flag);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--elapsed_secs_since_epoch_lo")
      {
         double elapsed_secs_since_epoch_lo=
            stringfunc::string_to_number(next_argument);
         currPassInfo.set_elapsed_secs_since_epoch_lo(
            elapsed_secs_since_epoch_lo);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--elapsed_secs_since_epoch_hi")
      {
         double elapsed_secs_since_epoch_hi=
            stringfunc::string_to_number(next_argument);
         currPassInfo.set_elapsed_secs_since_epoch_hi(
            elapsed_secs_since_epoch_hi);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--height_offset")
      {
         double height_offset=stringfunc::string_to_number(next_argument);
         currPassInfo.set_height_offset(height_offset);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--start_frame_ID")
      {
         int start_frame_ID=stringfunc::string_to_number(next_argument);
         currPassInfo.set_start_frame_ID(start_frame_ID);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--stop_frame_ID")
      {
         int stop_frame_ID=stringfunc::string_to_number(next_argument);
         currPassInfo.set_stop_frame_ID(stop_frame_ID);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--photo_ID")
      {
         int photo_ID=stringfunc::string_to_number(next_argument);
         currPassInfo.set_photo_ID(photo_ID);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--focal_length")
      {
         double focal_length=stringfunc::string_to_number(next_argument);
         currPassInfo.set_focal_length(focal_length);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--Uaxis_focal_length")
      {
         double Uaxis_focal_length=stringfunc::string_to_number(
            next_argument);
         currPassInfo.set_Uaxis_focal_length(Uaxis_focal_length);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--Vaxis_focal_length")
      {
         double Vaxis_focal_length=stringfunc::string_to_number(
            next_argument);
         currPassInfo.set_Vaxis_focal_length(Vaxis_focal_length);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--U0")
      {
         double U0=stringfunc::string_to_number(next_argument);
         currPassInfo.set_U0(U0);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--V0")
      {
         double V0=stringfunc::string_to_number(next_argument);
         currPassInfo.set_V0(V0);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--pixel_skew_angle")
      {
         double theta=stringfunc::string_to_number(next_argument); // degs
         currPassInfo.set_pixel_skew_angle(theta*PI/180);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--relative_az")
      {
         double relative_az=stringfunc::string_to_number(next_argument);
//         cout << "relative_az = " << relative_az << endl;

         currPassInfo.set_relative_az(relative_az);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--relative_el")
      {
         double relative_el=stringfunc::string_to_number(next_argument);
         currPassInfo.set_relative_el(relative_el);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--relative_roll")
      {
         double relative_roll=stringfunc::string_to_number(next_argument);
         currPassInfo.set_relative_roll(relative_roll);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--camera_longitude")
      {
         double camera_lon=stringfunc::string_to_number(next_argument);
//         cout << "camera_lon = " << camera_lon << endl;
         currPassInfo.set_camera_longitude(camera_lon);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--camera_latitude")
      {
         double camera_lat=stringfunc::string_to_number(next_argument);
//         cout << "camera_lat = " << camera_lat << endl;
         currPassInfo.set_camera_latitude(camera_lat);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--camera_x_posn")
      {
         double camera_x_posn=stringfunc::string_to_number(next_argument);
//         cout << "camera_x_posn = " << camera_x_posn << endl;
         currPassInfo.set_camera_x_posn(camera_x_posn);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--camera_y_posn")
      {
         double camera_y_posn=stringfunc::string_to_number(next_argument);
//         cout << "camera_y_posn = " << camera_y_posn << endl;
         currPassInfo.set_camera_y_posn(camera_y_posn);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--camera_z_posn")
      {
         double camera_z_posn=stringfunc::string_to_number(next_argument);
//         cout << "camera_z_posn = " << camera_z_posn << endl;
         currPassInfo.set_camera_z_posn(camera_z_posn);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--frustum_color")
      {
         string frustum_color=next_argument;
         currPassInfo.set_frustum_color(frustum_color);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--frustum_sidelength")
      {
         double frustum_sidelength=stringfunc::string_to_number(
            next_argument);
//         cout << "frustum_sidelength = " << frustum_sidelength << endl;
         currPassInfo.set_frustum_sidelength(frustum_sidelength);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--downrange_distance")
      {
         double downrange_dist=stringfunc::string_to_number(next_argument);
         currPassInfo.set_downrange_distance(downrange_dist);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--imageplane_x")
      {
         double imageplane_x=stringfunc::string_to_number(next_argument);
         currPassInfo.set_imageplane_x(imageplane_x);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--imageplane_y")
      {
         double imageplane_y=stringfunc::string_to_number(next_argument);
         currPassInfo.set_imageplane_y(imageplane_y);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--imageplane_z")
      {
         double imageplane_z=stringfunc::string_to_number(next_argument);
         currPassInfo.set_imageplane_z(imageplane_z);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--imageplane_w")
      {
         double imageplane_w=stringfunc::string_to_number(next_argument);
         currPassInfo.set_imageplane_w(imageplane_w);
         freeze_pass_stop_index=true;
      }

      else if (curr_argument=="--magnetic_yaw")
      {
         double magnetic_yaw=stringfunc::string_to_number(next_argument);
         currPassInfo.set_magnetic_yaw(magnetic_yaw);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--filter_alpha_value")
      {
         double filter_alpha_value=
            stringfunc::string_to_number(next_argument);
         currPassInfo.set_filter_alpha_value(filter_alpha_value);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--PostGIS_hostname")
      {
         string hostname=next_argument;
//         cout << "hostname = " << hostname << endl;
         currPassInfo.set_PostGIS_hostname(hostname);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--PostGIS_database_name")
      {
         string database_name=next_argument;
//         cout << "database_name = " << database_name << endl;
         currPassInfo.set_PostGIS_database_name(database_name);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--PostGIS_username")
      {
         string username=next_argument;
         currPassInfo.set_PostGIS_username(username);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--gispoints_tablename")
      {
         string tablename=next_argument;
         cout << "GIS points tablename = " << tablename << endl;
         currPassInfo.pushback_gispoints_tablename(tablename);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--gislines_tablename")
      {
         string tablename=next_argument;
         cout << "GIS lines tablename = " << tablename << endl;
         currPassInfo.pushback_gislines_tablename(tablename);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--gispolys_tablename")
      {
         string tablename=next_argument;
//         cout << "GIS tablename = " << tablename << endl;
         currPassInfo.pushback_gispolys_tablename(tablename);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--ActiveMQ_hostname")
      {
         string hostname=next_argument;
         currPassInfo.set_ActiveMQ_hostname(hostname);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--video_corner_vertex")
      {
         threevector V;
         for (int i=0; i<3; i++)
         {
            double v_entry=stringfunc::string_to_number(next_argument);
            V.put(i,v_entry);
            get_arguments(++a,curr_argument,next_argument);
         }
         currPassInfo.pushback_video_corner_vertex(V);
//         cout.precision(15);
//         cout << "video corner vertex = " << V << endl;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--bbox_top_left_corner")
      {
         threevector V;
         for (int i=0; i<3; i++)
         {
            double v_entry=stringfunc::string_to_number(next_argument);
            V.put(i,v_entry);
            get_arguments(++a,curr_argument,next_argument);
         }
         currPassInfo.pushback_bbox_top_left_corner(V);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--bbox_bottom_right_corner")
      {
         threevector V;
         for (int i=0; i<3; i++)
         {
            double v_entry=stringfunc::string_to_number(next_argument);
            V.put(i,v_entry);
            get_arguments(++a,curr_argument,next_argument);
         }
         currPassInfo.pushback_bbox_bottom_right_corner(V);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--bbox_color")
      {
         string bbox_color_string=next_argument;
         currPassInfo.pushback_bbox_color(bbox_color_string);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--bbox_label")
      {
         string bbox_label=next_argument;
         bbox_label=stringfunc::find_and_replace_char(bbox_label,"_"," ");
         currPassInfo.pushback_bbox_label(bbox_label);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--bbox_label_color")
      {
         string bbox_label_color_string=next_argument;
         currPassInfo.pushback_bbox_label_color(bbox_label_color_string);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--ROI_skeleton_height")
      {
         double ROI_skeleton_height=stringfunc::string_to_number(
            next_argument);
         currPassInfo.set_ROI_skeleton_height(ROI_skeleton_height);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--ROI_skeleton_color")
      {
         string ROI_skeleton_color=next_argument;
         currPassInfo.set_ROI_skeleton_color(ROI_skeleton_color);
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--projection_matrix")
      {
         genmatrix P(3,4);
         for (int r=0; r<3; r++)
         {
            for (int c=0; c<4; c++)
            {
               double p_entry=stringfunc::string_to_number(next_argument);
               P.put(r,c,p_entry);
               get_arguments(++a,curr_argument,next_argument);
            }
         }
         currPassInfo.set_projection_matrix(&P);
//         cout << "proj matrix = " 
//              << *(currPassInfo.get_projection_matrix_ptr()) << endl;
         freeze_pass_stop_index=true;
      }
      else if (curr_argument=="--animation_times_vs_frames")
      {
         int n_frame_times=stringfunc::string_to_number(next_argument);
         cout << "n_frame_times = " << n_frame_times << endl;
         get_arguments(++a,curr_argument,next_argument);
         for (int n=0; n<n_frame_times; n++)
         {
            int year=stringfunc::string_to_number(next_argument);
            get_arguments(++a,curr_argument,next_argument);
            int month=stringfunc::string_to_number(next_argument);
            get_arguments(++a,curr_argument,next_argument);
            int day=stringfunc::string_to_number(next_argument);
            get_arguments(++a,curr_argument,next_argument);
            int hour=stringfunc::string_to_number(next_argument);
            get_arguments(++a,curr_argument,next_argument);
            int mins=stringfunc::string_to_number(next_argument);
            get_arguments(++a,curr_argument,next_argument);
            double secs=stringfunc::string_to_number(next_argument);
            get_arguments(++a,curr_argument,next_argument);            

            Clock clock;
            clock.set_UTC_time(year,month,day,hour,mins,secs);
            double elapsed_secs=clock.secs_elapsed_since_reference_date();
            
            int frame_number=stringfunc::string_to_number(next_argument);
            get_arguments(++a,curr_argument,next_argument);            

//            cout << "date = " << clock.YYYY_MM_DD_H_M_S() << endl;
//            cout << "frame = " << frame_number << endl;

            twovector curr_frame_time(elapsed_secs,frame_number);
            currPassInfo.pushback_frame_time(curr_frame_time);

//            cout.precision(12);
//            cout << "curr_frame_time = " << curr_frame_time << endl;
            
         } // loop over index n labeling frame-time pairs
      }
      else if (curr_argument=="--sensor_posns_and_orientations")
      {
         int n_frames=stringfunc::string_to_number(next_argument);
//         cout << "n_frames = " << n_frames << endl;
         get_arguments(++a,curr_argument,next_argument);
         for (int n=0; n<n_frames; n++)
         {
            double sensor_X=stringfunc::string_to_number(next_argument);
            get_arguments(++a,curr_argument,next_argument);
            double sensor_Y=stringfunc::string_to_number(next_argument);
            get_arguments(++a,curr_argument,next_argument);
            double sensor_Z=stringfunc::string_to_number(next_argument);
            get_arguments(++a,curr_argument,next_argument);
            double sensor_roll=stringfunc::string_to_number(next_argument);
            get_arguments(++a,curr_argument,next_argument);
            double sensor_pitch=stringfunc::string_to_number(next_argument);
            get_arguments(++a,curr_argument,next_argument);
            double sensor_heading=stringfunc::string_to_number(next_argument);
            get_arguments(++a,curr_argument,next_argument);            
            int frame_number=stringfunc::string_to_number(next_argument);
            get_arguments(++a,curr_argument,next_argument);            

            threevector sensor_posn(sensor_X,sensor_Y,sensor_Z);
            rpy sensor_rpy(sensor_roll,sensor_pitch,sensor_heading);
            Triple<threevector,rpy,int> posn_orientation_frame(
               sensor_posn,sensor_rpy,frame_number);
            currPassInfo.pushback_posn_orientation_frame(
               posn_orientation_frame);
            
//            pair<threevector,rpy> retrieved_po=
//               currPassInfo.get_posn_orientation(
//                  currPassInfo.get_n_posn_orientations()-1);
//            cout << "posn = " << retrieved_po.first << endl;
//            cout << "orientation = " << retrieved_po.second << endl;

         } // loop over index n labeling frame-time pairs
      }
      else
      {
         if (!freeze_pass_stop_index) pass_stop_index++;
      }

      a++;
      
   } // while loop over index a labeling segmented arguments

//   cout << "Before exiting at end of PassesGroup::interpret_current_arguments()" 
//        << endl;
//   exit(-1);

}

// ---------------------------------------------------------------------
void PassesGroup::get_arguments(
   int a,string& curr_argument,string& next_argument)
{
//   cout << "inside PassesGroup::get_arguments, a = " << a << endl;

   curr_argument=arguments[a];
   next_argument="";
   if (a < int(arguments.size()-1))
   {
      next_argument=string(arguments[a+1]);
   }

//   cout << "curr_arg = " << curr_argument << endl;
//   cout << " next_arg = " << next_argument << endl << endl;
}

// ---------------------------------------------------------------------
// Member function segment_argument_list effectively inserts
// "--newpass" string entries into member STL vector arguments between
// any two arguments with different suffix types.  The resulting
// argument list is then segmented into contiguous chunks with the
// same suffix ending separated by "--newpass" markers (which may have
// also been manually inserted at the command line).

vector<string> PassesGroup::segment_argument_list(
   osg::ArgumentParser* arg_parser_ptr,bool command_line_flag)
{
//   cout << "inside PassesGroup::segment_argument_list()" << endl;

   vector<string> new_arguments;
   int a_start=0;

// If argument list was entered at command line, skip over the very
// first argument within the argument parser list which we assume
// corresponds to the name of the main program itself:

   if (command_line_flag) a_start=1; 
   int a_max=arg_parser_ptr->argc();
//   cout << "a_max = " << a_max << endl;
   string prev_argument="";
   for (int a=a_start; a<a_max; a++)
   {
      if (new_arguments.size() > 0) prev_argument=new_arguments.back();
      bool prev_arg_is_number_flag=stringfunc::is_number(prev_argument);
      string prev_suffix=stringfunc::suffix(prev_argument);
      
      string curr_argument((*arg_parser_ptr)[a]);
      bool curr_arg_is_number_flag=stringfunc::is_number(curr_argument);
      string curr_suffix=stringfunc::suffix(curr_argument);

      string next_argument="";
      if (a < a_max-1)
      {
         next_argument=string((*arg_parser_ptr)[a+1]);
      }
//      cout << "a = " << a 
//           << " prev_arg = " << prev_argument
//           << " prev_arg_is_num = " << prev_arg_is_number_flag << endl;
//      cout << "curr_arg = " << curr_argument 
//           << " curr_arg_is_num = " << curr_arg_is_number_flag 
//           << endl;
//      cout << "next_arg = " << next_argument << endl;

      if (prev_argument=="--surface_texture")
      {
//         cout << "doing nothing" << endl;
      }
      else if (curr_argument=="--surface_texture")
      {
         parse_package_arguments(curr_argument,next_argument,new_arguments);
      }
      else if (curr_argument=="--pick_points_on_Zplane_flag")
      {
         new_arguments.push_back("--pick_points_on_Zplane_flag");
      }
      else if (curr_argument=="--continuously_update_UAV_paths_flag")
      {
         new_arguments.push_back("--continuously_update_UAV_paths_flag");
      }
      else if (curr_argument=="--classification")
      {
         new_arguments.push_back("--classification");
      }
      else if (curr_argument=="--campaign_ID")
      {
         new_arguments.push_back("--campaign_ID");
      }
      else if (curr_argument=="--mission_ID")
      {
         new_arguments.push_back("--mission_ID");
      }
      else if (curr_argument=="--graph_hierarchy_ID")
      {
         new_arguments.push_back("--graph_hierarchy_ID");
      }
      else if (curr_argument=="--graph_component_ID")
      {
         new_arguments.push_back("--graph_component_ID");
      }
      else if (curr_argument=="--max_child_node_ID")
      {
         new_arguments.push_back("--max_child_node_ID");
      }
      else if (curr_argument=="--n_total_nodes_level0")
      {
         new_arguments.push_back("--n_total_nodes_level0");
      }
      else if (curr_argument=="--n_total_nodes_level1")
      {
         new_arguments.push_back("--n_total_nodes_level1");
      }
      else if (curr_argument=="--n_total_nodes_level2")
      {
         new_arguments.push_back("--n_total_nodes_level2");
      }
      else if (curr_argument=="--n_total_links_level0")
      {
         new_arguments.push_back("--n_total_links_level0");
      }
      else if (curr_argument=="--n_total_links_level1")
      {
         new_arguments.push_back("--n_total_links_level1");
      }
      else if (curr_argument=="--n_total_links_level2")
      {
         new_arguments.push_back("--n_total_links_level2");
      }
      else if (curr_argument=="--line_width")
      {
         new_arguments.push_back("--line_width");
      }
      else if (curr_argument=="--virtual_horiz_FOV")
      {
         new_arguments.push_back("--virtual_horiz_FOV");
      }
      else if (curr_argument=="--n_ROI_states")
      {
         new_arguments.push_back("--n_ROI_states");
      }
      else if (curr_argument=="--world_time_step")
      {
         new_arguments.push_back("--world_time_step");
      }
      else if (curr_argument=="--world_start_UTC")
      {
         new_arguments.push_back("--world_start_UTC");
      }
      else if (curr_argument=="--world_stop_UTC")
      {
         new_arguments.push_back("--world_stop_UTC");
      }
      else if (curr_argument=="--initial_mode")
      {
         new_arguments.push_back("--initial_mode");
      }
      else if (curr_argument=="--OSGButtonServer_URL")
      {
         new_arguments.push_back("--OSGButtonServer_URL");
      }
      else if (curr_argument=="--SKSDataServer_URL")
      {
         new_arguments.push_back("--SKSDataServer_URL");
      }
      else if (curr_argument=="--SKSDataServer_query_type")
      {
         new_arguments.push_back("--SKSDataServer_query_type");
      }
      else if (curr_argument=="--LogicServer_URL")
      {
         new_arguments.push_back("--LogicServer_URL");
      }
      else if (curr_argument=="--VideoServer_URL")
      {
         new_arguments.push_back("--VideoServer_URL");
      }
      else if (curr_argument=="--HTMLServer_URL")
      {
         new_arguments.push_back("--HTMLServer_URL");
      }
      else if (curr_argument=="--Dynamic_WikiPage_URL")
      {
         new_arguments.push_back("--Dynamic_WikiPage_URL");
      }
      else if (curr_argument=="--broker_URL")
      {
         new_arguments.push_back("--broker_URL");
      }
      else if (curr_argument=="--message_queue_channel_name")
      {
         new_arguments.push_back("--message_queue_channel_name");
      }
      else if (prev_argument=="--GIS_layer")
      {
//         cout << "doing nothing" << endl;
      }
      else if (curr_argument=="--GIS_layer")
      {
         cout << "--GIS_layer detected" << endl;
         cout << "curr_argument = " << curr_argument
              << " next_argument = " << next_argument << endl;
         parse_package_arguments(curr_argument,next_argument,new_arguments);
      }
      else if (prev_argument=="--dataserver")
      {
//         cout << "doing nothing" << endl;
      }
      else if (curr_argument=="--dataserver")
      {
         parse_package_arguments(curr_argument,next_argument,new_arguments);
      }
      else if (prev_argument=="--sensor_metadata")
      {
//         cout << "doing nothing" << endl;
      }
      else if (curr_argument=="--sensor_metadata")
      {
         parse_package_arguments(curr_argument,next_argument,new_arguments);
      }
      else if (prev_argument=="--dted")
      {
//         cout << "doing nothing" << endl;
      }
      else if (curr_argument=="--dted")
      {
         parse_package_arguments(curr_argument,next_argument,new_arguments);
      }
      else if (prev_argument=="--region_filename")
      {
//         cout << "doing nothing" << endl;
      }
      else if (curr_argument=="--region_filename")
      {
         parse_package_arguments("",next_argument,new_arguments);
      }
      else
      {
//         cout << "Pushing back curr_argument = " << curr_argument << endl;
         new_arguments.push_back(curr_argument);

         string next_suffix=stringfunc::suffix(next_argument);
//         cout << "curr_suffix = " << curr_suffix 
//              << " next_suffix = " << next_suffix << endl;

         if (curr_suffix != next_suffix &&
             curr_suffix != "--newpass" && 
             next_suffix != "--newpass" &&

             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--OSGsubPAT_ID") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--pick_points_on_Zplane_flag") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--continuously_update_UAV_paths_flag") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--classification") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--campaign_ID") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--mission_ID") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--graph_hierarchy_ID") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--graph_component_ID") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--max_child_node_ID") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--n_total_nodes_level0") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--n_total_nodes_level1") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--n_total_nodes_level2") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--n_total_links_level0") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--n_total_links_level1") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--n_total_links_level2") &&

             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--line_width") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--virtual_horiz_FOV") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--n_ROI_states") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--world_time_step") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--world_start_UTC") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--world_stop_UTC") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--initial_mode") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--OSGButtonServer_URL") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--SKSDataServer_URL") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--SKSDataServer_query_type") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--LogicServer_URL") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--VideoServer_URL") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--HTMLServer_URL") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--Dynamic_WikiPage_URL") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--broker_URL") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--message_queue_channel_name") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--package_subdir") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--package_filename_prefix") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--portrait_mode") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--height_colormap") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--prob_colormap") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--independent_var") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--height_cyclic_frac_offset") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--prob_cyclic_frac_offset") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--max_x_threshold") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--min_x_threshold") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--max_y_threshold") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--min_y_threshold") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--max_z_threshold") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--min_z_threshold") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--max_p_threshold") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--min_p_threshold") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--max_threshold_frac") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--min_threshold_frac") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--longitude_lo") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--longitude_hi") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--latitude_lo") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--latitude_hi") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--easting_lo") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--easting_hi") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--northing_lo") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--northing_hi") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--altitude") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--UTM_zonenumber") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--northern_hemisphere_flag") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--elapsed_secs_since_epoch_lo") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--elapsed_secs_since_epoch_hi") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--height_offset") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--start_frame_ID") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--stop_frame_ID") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--photo_ID") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--focal_length") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--Uaxis_focal_length") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--Vaxis_focal_length") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--U0") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--V0") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--pixel_skew_angle") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--relative_az") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--relative_el") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--relative_roll") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--camera_longitude") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--camera_latitude") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--camera_x_posn") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--camera_y_posn") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--camera_z_posn") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--frustum_color") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--frustum_sidelength") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--downrange_distance") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--imageplane_x") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--imageplane_y") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--imageplane_z") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--imageplane_w") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--image_list_filename") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--image_sizes_filename") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--bundle_filename") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--camera_views_filename") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--xyz_pnts_filename") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--photoids_xyzids_filename") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--edgelist_filename") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--common_planes_filename") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--fitted_world_to_bundler_distance_ratio") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--bundler_translation_X") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--bundler_translation_Y") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--bundler_translation_Z") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--global_az") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--global_el") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--global_roll") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--bundler_rotation_origin_X") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--bundler_rotation_origin_Y") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--bundler_rotation_origin_Z") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--magnetic_yaw") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--filter_alpha_value") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--PostGIS_hostname") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--PostGIS_database_name") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--PostGIS_username") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--gispoints_tablename") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--gislines_tablename") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--gispolys_tablename") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--ActiveMQ_hostname") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--WebServer_URL") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--video_corner_vertex") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--bbox_top_left_corner") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--bbox_bottom_right_corner") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--bbox_color") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--bbox_label") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--bbox_label_color") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--ROI_skeleton_height") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--ROI_skeleton_color") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--projection_matrix") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--animation_times_vs_frames") &&
             check_suffix_match(prev_suffix,curr_suffix,next_suffix,
                                "--sensor_posns_and_orientations") &&

// Assume that any set of numbers (e.g. for 3x4 projection matrix) are
// contiguous within input .pkg file.  So do NOT insert a --newpass
// token between any two adjacent input numbers:

             !(prev_arg_is_number_flag && curr_arg_is_number_flag)

            )
         {
            new_arguments.push_back("--newpass");
         }
      }
      
   } // loop over index a labeling arguments

// Add final "--newpass" entry if it does not already exist at end of
// new_arguments:

   if (new_arguments.size()==0)
   {
      new_arguments.push_back("--newpass");
   }
   else if (command_line_flag && new_arguments.back() != "--newpass") 
   {
      new_arguments.push_back("--newpass");
   }
   return new_arguments;
}

// ---------------------------------------------------------------------
void PassesGroup::parse_package_arguments(
   string curr_argument,string next_argument,vector<string>& new_arguments)
{
//   cout << "inside PassesGroup::parse_package_arguments()" << endl;
//   cout << "curr_arg = " << curr_argument
//        << " next_arg = " << next_argument << endl;

   string pkg_filename=next_argument;
   vector<string> external_arguments=parse_ext_file_args(pkg_filename);

   if (curr_argument.size() > 0)
   {
      new_arguments.push_back(curr_argument);
   }
   for (unsigned int i=0; i<external_arguments.size(); i++)
   {
      new_arguments.push_back(external_arguments[i]);
//      cout << "i = " << i << " new_arg = " << new_arguments.back() << endl;
   }
}

// ---------------------------------------------------------------------
// Member function 

bool PassesGroup::check_suffix_match(
   string prev_suffix,string curr_suffix,string next_suffix,
   string param_description)
{
   return (prev_suffix != param_description && 
           curr_suffix != param_description && 
           next_suffix != param_description);
}

// ---------------------------------------------------------------------
// Member function parse_ext_file_args takes in the name for some file
// passed in as a command line argument to a main program.  After
// opening the file, it performs wild * character pattern matching on
// any arguments which could correspond to genuine pass constituents
// (i.e. it expands *.osga into foo1.osga, foo2.osga, foo3.osga if
// these files all lie within the current working directory).  This
// methods stores and returns the (expanded) arguments within an STL
// vector<string>.

vector<string> PassesGroup::parse_ext_file_args(string ext_filename)
{
//   cout << "inside PassesGroup::parse_ext_file_args()" << endl;
//   cout << "ext_filename = " << ext_filename << endl;

   vector<string> ext_arguments;
   if (!filefunc::ReadInfile(ext_filename))
   {
      cout << "Trouble in PassesGroup::parse_ext_file_args()" << endl;
      cout << "Could not read external file " << ext_filename << endl;
      return ext_arguments;
   }
   
   string command_line="";
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      command_line += filefunc::text_line[i]+" ";
   }

   vector<string> string_patterns=
      stringfunc::decompose_string_into_substrings(command_line);
   vector<string> substrings;
   
   for (unsigned int i=0; i<string_patterns.size(); i++)
   {
      string pattern_to_match=string_patterns[i];

      Pass::PassType curr_passtype=Pass::other;
      determine_file_and_pass_type_from_suffix(
         stringfunc::suffix(pattern_to_match),curr_passtype);

      if (curr_passtype != Pass::other)
      {
         vector<string> matching_files=filefunc::files_matching_pattern(
            pattern_to_match);

         for (unsigned int j=0; j<matching_files.size(); j++)
         {
            substrings.push_back(matching_files[j]);
         }
      }
      else
      {
         substrings.push_back(string_patterns[i]);
      }
   } // loop over index i labeling string_patterns

//   for (unsigned int k=0; k<substrings.size(); k++)
//   {
//      cout << "k = " << k << " substring = " << substrings[k] << endl;
//   }
   
   int _argc=substrings.size();
   char** _argv=new char*[_argc];

   for (unsigned int i=0; i<substrings.size(); i++)
   {
      _argv[i]=const_cast<char*>(substrings[i].c_str());
   }

   osg::ArgumentParser external_arguments(&_argc,_argv);   

   bool command_line_flag=false;
   ext_arguments=segment_argument_list(&external_arguments,command_line_flag);
   delete [] _argv;

//   for (unsigned int n=0; n<ext_arguments.size(); n++)
//   {
//      cout << "n = " << n
//           << " ext_arguments[n] = " << ext_arguments[n] << endl;
//   }
//   outputfunc::enter_continue_char();

   return ext_arguments;
}

// ---------------------------------------------------------------------
// Member function compute_pass_bounding_argument_indices stores pairs
// of starting and stopping indices within member STL vector
// pass_bounding_arguments.  This information can be later used to
// identify individual constituent files corresponding to particular
// passes.

void PassesGroup::compute_pass_bounding_argument_indices(
   int& pass_start_index,int& pass_stop_index,int next_index)
{
//   cout << "inside PassesGroup::compute_pass_bounding_argument_indices()"
//        << endl;
   
//   cout << "pair(start,stop) = " << "(" << pass_start_index << ","
//        << pass_stop_index << ")" << endl;
//   cout << "next_index = " << next_index << endl;
//   cout << "currpass_ID = " << currpass_ID << endl;

   currPassInfo.set_ID(currpass_ID++);
   currPassInfo.set_start_argument_index(pass_start_index);
   currPassInfo.set_stop_argument_index(pass_stop_index);
   PassInfo* PassInfo_ptr=new PassInfo(currPassInfo);
   pass_metadata_ptrs.push_back(PassInfo_ptr);
   currPassInfo.clear_params();

   pass_start_index=pass_stop_index=next_index;
}

// ---------------------------------------------------------------------
// Member function generate_passes_from_arguments

void PassesGroup::generate_passes_from_arguments()
{
//   cout << "inside PassesGroup::generate_passes_from_args()" << endl;
//   cout << "pass_metadata_ptrs.size() = "
//        << pass_metadata_ptrs.size() << endl;

   int ID=get_n_passes();
   int r_start=get_n_passes();
   for (unsigned int r=r_start; r < pass_metadata_ptrs.size(); r++)
   {
      if (generate_new_pass(r,ID)) ID++;
   }
}

// ---------------------------------------------------------------------
bool PassesGroup::generate_new_pass(int r,int ID)
{
//   cout << "inside PassesGroup::generate_new_pass() #1, r = " << r
//        << " ID = " << ID << endl;

   Pass::PassType pass_type=Pass::other;

// Set pass type based upon explicit initial input arguments.
// Otherwise, rely upon suffix of first file within current pass list
// to determine pass type:

   int pass_start_index=pass_metadata_ptrs[r]->get_start_argument_index();
   int pass_stop_index=pass_metadata_ptrs[r]->get_stop_argument_index();
//   cout << "start_index = " << pass_start_index 
//        << " stop_index = " << pass_stop_index << endl;

   Pass::InputFileType input_file_type = Pass::unknown;

   string first_argument=arguments[pass_start_index];
//   cout << "first_argument = " << first_argument << endl;

   if (first_argument=="--surface_texture")
   {
      pass_type=Pass::surface_texture;
      pass_start_index++;
   }
   else if (first_argument=="--GIS_layer")
   {
      pass_type=Pass::GIS_layer;
      pass_start_index++;
   }
   else if (first_argument=="--dataserver")
   {
      pass_type=Pass::dataserver;
      pass_start_index++;
   }
   else if (first_argument=="--sensor_metadata")
   {
      pass_type=Pass::sensor_metadata;
      pass_start_index++;
   }
   else if (first_argument=="--dted")
   {
      pass_type=Pass::dted;
      pass_start_index++;
   }
   else
   {
      string suffix=stringfunc::suffix(arguments[pass_start_index]);
      input_file_type=determine_file_and_pass_type_from_suffix(
         suffix,pass_type);
   }

// Do NOT instantiate a pass if its type is not known:

//   cout << "pass_type = " << pass_type << endl;
   if (pass_type == Pass::other) return false;

   Pass* curr_pass_ptr=new Pass(ID);
   curr_pass_ptr->set_PassInfo_ptr(get_passinfo_ptr(r));
   curr_pass_ptr->set_passtype(pass_type);
   curr_pass_ptr->set_input_filetype(input_file_type);
   
   for (int a=pass_start_index; a<=pass_stop_index; a++)
   {
//      cout << "a = " << a << " filename = " << arguments[a] << endl;
      curr_pass_ptr->pushback_filename(arguments[a]);
   } // loop over index a labeling arguments corresponding to new pass

   pass_ptr_list.push_back(curr_pass_ptr);

//   cout << "At end of PassesGroup::generate_new_pass() #1" << endl;
//   cout << "r = " << r << endl;
//   cout << "curr_pass_ptr->ID = " << curr_pass_ptr->get_ID() << endl;
//   cout << "curr_pass_ptr->get_passtype() = "
//        << curr_pass_ptr->get_passtype() << endl;
//   cout << "curr_pass_ptr->filenames = " << endl;
//   templatefunc::printVector(curr_pass_ptr->get_filenames());
//   cout << "curr_pass->pass_info = " 
//        << *(curr_pass_ptr->get_PassInfo_ptr()) << endl;

   return true;
}

// ---------------------------------------------------------------------
int PassesGroup::generate_new_pass(string pass_filename,int specified_ID)
{
//   cout << "inside PassesGroup::generate_new_pass() #2" << endl;
   
   string suffix=stringfunc::suffix(pass_filename);

   Pass::PassType pass_type;
   Pass::InputFileType input_filetype=
      determine_file_and_pass_type_from_suffix(suffix,pass_type);
   return generate_new_pass(pass_filename,pass_type,input_filetype,
                            specified_ID);
}

// ---------------------------------------------------------------------
int PassesGroup::generate_new_pass(
   string pass_filename,Pass::PassType pass_type,
   Pass::InputFileType input_filetype,int specified_ID)
{
//   cout << "inside PassesGroup::generate_new_pass() #3" << endl;
//   cout << "pass_filename = " << pass_filename << endl;
//   cout << "specified_ID = " << specified_ID << endl;
//   cout << "pass_metadata_ptrs.size() = " << pass_metadata_ptrs.size() 
//        << endl;

   Pass* curr_pass_ptr;
   if (specified_ID >= 0)
   {
      curr_pass_ptr=new Pass(specified_ID);
      curr_pass_ptr->set_PassInfo_ptr(get_passinfo_ptr(specified_ID));
//      cout << "*get_passinfo_ptr(specified_ID) = "
//           << *(get_passinfo_ptr(specified_ID)) << endl;
   }

// The following else part of the conditional is called by (at least)
// programs EARTH, JUSTEARTH & SPASE_IMAGEPLANES.

   else
   {
      curr_pass_ptr=new Pass(currpass_ID);
      currPassInfo.set_ID(currpass_ID);
//      cout << "currPassInfo = " << currPassInfo << endl;
      PassInfo* PassInfo_ptr=new PassInfo(currPassInfo);
      pass_metadata_ptrs.push_back(PassInfo_ptr);
      currPassInfo.clear_params();
      curr_pass_ptr->set_PassInfo_ptr(get_passinfo_ptr(currpass_ID));
//      cout << "currpass_ID = " << currpass_ID << endl;
//      cout << "*get_passinfo_ptr(currpass_ID) = "
//           << *(get_passinfo_ptr(currpass_ID)) << endl;
   }

   curr_pass_ptr->set_passtype(pass_type);
   curr_pass_ptr->set_input_filetype(input_filetype);
   curr_pass_ptr->pushback_filename(pass_filename);
   pass_ptr_list.push_back(curr_pass_ptr);

   currpass_ID++;
   return currpass_ID-1;
}

// ---------------------------------------------------------------------
Pass::InputFileType PassesGroup::determine_file_and_pass_type_from_suffix(
   string suffix,Pass::PassType& pass_type)
{
//   cout << "inside PassesGroup::determine_file_and_pass_type_from_suffix()"
//        << endl;
//   cout << "suffix = " << suffix << endl;

   Pass::InputFileType input_filetype=Pass::unknown;
   pass_type=Pass::other;

   if (suffix=="xyzp")
   {
      input_filetype=Pass::xyzp;
      pass_type=Pass::cloud;
   }
   if (suffix=="xyz")
   {
      input_filetype=Pass::xyz;
      pass_type=Pass::cloud;
   }
   else if (suffix=="fxyz")
   {
      input_filetype=Pass::fxyz;
      pass_type=Pass::cloud;
   }
   else if (suffix=="xyzrgba")
   {
      input_filetype=Pass::xyzrgba;
      pass_type=Pass::cloud;
   }
   else if (suffix=="tdp")
   {
      input_filetype=Pass::tdp;
      pass_type=Pass::cloud;
   }
   else if (suffix=="ive")
   {
      input_filetype=Pass::ive;
      pass_type=Pass::cloud;
   }
   else if (suffix=="osga")
   {
      input_filetype=Pass::osga;
      pass_type=Pass::cloud;
   }
   else if (suffix=="osg")
   {
      input_filetype=Pass::osg;
      pass_type=Pass::cloud;
   }
   else if (suffix=="vid" || suffix=="mp4" || suffix=="MP4" ||
            suffix=="mov" || suffix=="MOV" || suffix=="avi" )
   {
      input_filetype=Pass::vid;
      pass_type=Pass::video;
   }
   else if (suffix=="png")
   {
      input_filetype=Pass::png;
      pass_type=Pass::video;
   }
   else if (suffix=="jpg" || suffix=="JPG" || suffix=="jpeg" 
            || suffix=="JPEG")
   {
      input_filetype=Pass::jpg;
      pass_type=Pass::video;
   }
   else if (suffix=="tif" || suffix=="tiff")
   {
      input_filetype=Pass::tif;
      pass_type=Pass::video;
   }
   else if (suffix=="rgb")
   {
      input_filetype=Pass::rgb;
      pass_type=Pass::video;
   }
   else if (suffix=="ntf" || suffix=="I21" || suffix=="I22")
   {
      input_filetype=Pass::ntf;
      pass_type=Pass::video;
   }
   else if (suffix=="dt0")
   {
      input_filetype=Pass::dt0;
      pass_type=Pass::dted;
   }
   else if (suffix=="dt1")
   {
      input_filetype=Pass::dt1;
      pass_type=Pass::dted;
   }
   else if (suffix=="dt2")
   {
      input_filetype=Pass::dt2;
      pass_type=Pass::dted;
   }
   else if (suffix=="pkg")
   {
      input_filetype=Pass::pkg;
      pass_type=Pass::other;
   }
   return input_filetype;
}
