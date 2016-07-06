// ==========================================================================
// POINTCLOUDSGROUP class member function definitions
// ==========================================================================
// Last modified on 12/18/11; 12/29/11; 10/1/12
// ==========================================================================

#include <iomanip>
#include <string>
#include <vector>
#include <osgUtil/Optimizer>
#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgSceneGraph/ColorGeodeVisitor.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloud.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "geometry/polyline.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PointCloudsGroup::allocate_member_objects()
{
//   cout << "inside PointCloudsGroup::allocate_member_objects()" << endl;

   StateSet_refptr = new osg::StateSet();
   pt_refptr=new osg::Point;

   HiresDataVisitor_refptr=new HiresDataVisitor();

   ColorGeodeVisitor_refptr=new ColorGeodeVisitor();
   SetupGeomVisitor_refptr=new SetupGeomVisitor(StateSet_refptr.get());

   ColorGeodeVisitor_refptr->set_CommonCallbacks_ptr(
      get_CommonCallbacks_ptr());
   SetupGeomVisitor_refptr->set_CommonCallbacks_ptr(
      get_CommonCallbacks_ptr());
}		       

void PointCloudsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="PointCloudsGroup";
   auto_resize_points_flag=true;
   time_dependence_flag=false;
   colorscheme_toggle_counter=0;
   curr_colorbar_index=prev_colorbar_index=0;
   cloudptrs_ptr=NULL;
   StateSet_refptr->setAttribute(pt_refptr.get());
   pt_refptr->setSize(1);
   point_transition_altitude_factor=1;
   Terrain_Manipulator_ptr=NULL;
   ColorbarHUD_ptr=NULL;
   Operations_ptr=NULL;
   PolyLinesGroup_ptr=NULL;
   TilesGroup_ptr=NULL;
   tracks_group_ptr=NULL;

   get_OSGgroup_ptr()->setName("Clouds OSGgroup");
   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<PointCloudsGroup>(
         this, &PointCloudsGroup::update_display));

   SetupGeomVisitor_refptr->set_MyHyperFilter_ptr(MyHyperFilter_refptr.get());
   SetupGeomVisitor_refptr->add_HyperFilter_Callback();
}		       

PointCloudsGroup::PointCloudsGroup(
   Pass* PI_ptr,threevector* GO_ptr):
   DataGraphsGroup(3,PI_ptr,GO_ptr)
{	
   allocate_member_objects();
   initialize_member_objects();
}		       

PointCloudsGroup::PointCloudsGroup(
   Pass* PI_ptr,
   AnimationController* AC_ptr,threevector* GO_ptr):
   DataGraphsGroup(3,PI_ptr,AC_ptr,GO_ptr)
{	
   allocate_member_objects();
   initialize_member_objects();
}		       

PointCloudsGroup::~PointCloudsGroup()
{
   delete cloudptrs_ptr;
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const PointCloudsGroup& M)
{
   int node_counter=0;
   for (unsigned int n=0; n<M.get_n_Graphicals(); n++)
   {
      PointCloud* Cloud_ptr=M.get_Cloud_ptr(n);
      outstream << "Cloud node # " << node_counter++ << endl;
      outstream << "Cloud = " << *Cloud_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// Cloud creation and manipulation methods
// ==========================================================================

// Member function generate_Clouds loops over every pass within
// passes_group and instantiates a point cloud for each one that
// corresponds to a genuine ladar data set.  It returns an STL vector
// with pointers to the dynamically generated clouds.

vector<PointCloud*>* PointCloudsGroup::generate_Clouds(
   PassesGroup& passes_group,bool index_tree_flag,TrianglesGroup* TG_ptr)
{
//   cout << "inside PointCloudsGroup::generate_Clouds()" << endl;
//   cout << "n_passes = " << passes_group.get_n_passes() << endl;

   cloudptrs_ptr=new vector<PointCloud*>;
   for (unsigned int i=0; i<passes_group.get_n_passes(); i++)
   {
      Pass* curr_pass_ptr=passes_group.get_pass_ptr(i);

      if (curr_pass_ptr->get_passtype()==Pass::cloud)
      {
         PointCloud* curr_cloud_ptr=generate_new_Cloud(
            curr_pass_ptr,index_tree_flag,TG_ptr);
         if (curr_cloud_ptr != NULL)
            cloudptrs_ptr->push_back(curr_cloud_ptr);
      } // passtype==cloud conditional

   } // loop over index i labeling individual passes in passes_group

//   cout << "cloudptrs_ptr->size() = " << cloudptrs_ptr->size() << endl;
//   outputfunc::enter_continue_char();
   return cloudptrs_ptr;
}

// ---------------------------------------------------------------------
// Member function generate_separate_clouds_from_input_files() takes
// in *pass_ptr which is assumed to correspond to at least 2 input
// .osga files.  It imports each .osga file's data into a distinct
// PointCloud object.  The individual point cloud graphical PAT's are
// also NOT automatically inserted into OSGsubPAT.  For 4D point cloud
// animation purposes, we instead allow each point cloud's graphical
// PAT to later be inserted into this PointCloudsGroup's
// switch_refptr...

void PointCloudsGroup::generate_separate_clouds_from_input_files(Pass* pass_ptr)
{
//   cout << "inside PointCloudsGroup::generate_separate_clouds_from_input_files()" << endl;

   vector<string> input_filenames=pass_ptr->get_filenames();
   if (input_filenames.size() <= 1)
   {
      cout << "Error in PointsCloudsGroup::generate_separate_clouds_from_input_files()!" << endl;
      cout << "input_filename.size() = " << input_filenames.size() << endl;
      exit(-1);
   }

   for (unsigned int i=0; i<input_filenames.size(); i++)
   {
      string cloud_filename=input_filenames[i];
      if (!filefunc::fileexist(cloud_filename)) continue;
      cout << "i = " << i << " cloud_filename = " << cloud_filename << endl;

      int ID=get_next_unused_ID();
      cout << "ID = " << ID << endl;

      PointCloud* curr_Cloud_ptr=new PointCloud(
         pass_ptr,get_LeafNodeVisitor_ptr(),get_TreeVisitor_ptr(),
         get_ColorGeodeVisitor_ptr(),get_SetupGeomVisitor_ptr(),
         get_HiresDataVisitor_ptr(),ID);
      height_ColorMap_ptr=curr_Cloud_ptr->get_z_ColorMap_ptr();
      prob_ColorMap_ptr=curr_Cloud_ptr->get_p_ColorMap_ptr();

      ColorGeodeVisitor_refptr->set_height_ColorMap_ptr(height_ColorMap_ptr);
      ColorGeodeVisitor_refptr->set_prob_ColorMap_ptr(prob_ColorMap_ptr);

      set_dependent_coloring_var(
         pass_ptr->get_PassInfo_ptr()->get_independent_variable());

//      if (pass_ptr->get_PassInfo_ptr()->get_independent_variable()==3)
//      {
//         next_dependent_var();
//      }

      GraphicalsGroup::insert_Graphical_into_list(curr_Cloud_ptr);
      initialize_Graphical(curr_Cloud_ptr);

// Do NOT automatically insert individual point cloud's graphical PAT
// into OSGsubPAT!  For 4D point cloud animation purposes, we instead
// allow each point cloud's graphical PAT to later be inserted into
// this PointCloudsGroup's switch_refptr...

//      insert_graphical_PAT_into_OSGsubPAT(curr_Cloud_ptr,0);

// Import data from a single input file (e.g. some .osga file) into
// *DataNode_ptr:

      osg::Node* DataNode_ptr=curr_Cloud_ptr->ReadGraph(cloud_filename);
//   cout << "DataNode_ptr = " << DataNode_ptr << endl;

      curr_Cloud_ptr->InitializeCloudGraph();

// Optimize current data graph by removing redundant nodes and states:

      osgUtil::Optimizer optimizer;
      optimizer.optimize(DataNode_ptr);

      curr_Cloud_ptr->get_PAT_ptr()->addChild(DataNode_ptr);

   } // loop over index i labeling input files

   change_color_map(0);
   compute_total_xyz_and_hyper_bboxes();
}

// ---------------------------------------------------------------------
// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_Cloud from all other graphical insertion
// and manipulation methods...

PointCloud* PointCloudsGroup::generate_new_Cloud(
   bool index_tree_flag,TrianglesGroup* TG_ptr,int ID)
{
   return generate_new_Cloud(pass_ptr,index_tree_flag,TG_ptr,ID);
}

PointCloud* PointCloudsGroup::generate_new_Cloud(
   Pass* curr_pass_ptr,bool index_tree_flag,TrianglesGroup* TG_ptr,int ID)
{
//   cout << "inside PointCloudsGroup::generate_new_Cloud()" << endl;

// First check whether cloud file corresponding to pass filename
// actually exists:

   string cloud_filename=curr_pass_ptr->get_first_filename();
//   cout << "cloud first filename = " << cloud_filename << endl;
   if (!filefunc::fileexist(cloud_filename))
   {
      cout << "No point cloud file found corresponding to filename = "
           << cloud_filename << endl;
      return NULL;
   }

//   cout << "cloud_filename = " << cloud_filename << endl;
//   cout << "ID = " << ID << endl;
   
   if (ID==-1) ID=get_next_unused_ID();

   PointCloud* curr_Cloud_ptr=new PointCloud(
      curr_pass_ptr,get_LeafNodeVisitor_ptr(),get_TreeVisitor_ptr(),
      get_ColorGeodeVisitor_ptr(),get_SetupGeomVisitor_ptr(),
      get_HiresDataVisitor_ptr(),ID,TG_ptr);

   height_ColorMap_ptr=curr_Cloud_ptr->get_z_ColorMap_ptr();
   prob_ColorMap_ptr=curr_Cloud_ptr->get_p_ColorMap_ptr();

   ColorGeodeVisitor_refptr->set_height_ColorMap_ptr(height_ColorMap_ptr);
   ColorGeodeVisitor_refptr->set_prob_ColorMap_ptr(prob_ColorMap_ptr);

   set_dependent_coloring_var(
      curr_pass_ptr->get_PassInfo_ptr()->get_independent_variable());

/*
   if (curr_pass_ptr->get_PassInfo_ptr()->get_independent_variable()==3)
   {
      next_dependent_var();
   }
   else if (curr_pass_ptr->get_PassInfo_ptr()->get_independent_variable()==0)
   {
      next_dependent_var();
      next_dependent_var();
   }
   else if (curr_pass_ptr->get_PassInfo_ptr()->get_independent_variable()==1)
   {
      next_dependent_var();
      next_dependent_var();
      next_dependent_var();
   }
*/

   GraphicalsGroup::insert_Graphical_into_list(curr_Cloud_ptr);
   initialize_Graphical(curr_Cloud_ptr);
   insert_graphical_PAT_into_OSGsubPAT(curr_Cloud_ptr,0);

   osg::Node* DataNode_ptr=curr_Cloud_ptr->GenerateCloudGraph(index_tree_flag);
//   cout << "DataNode_ptr = " << DataNode_ptr << endl;

// Optimize current data graph by removing redundant nodes and states:

   osgUtil::Optimizer optimizer;
   optimizer.optimize(DataNode_ptr);

   curr_Cloud_ptr->get_PAT_ptr()->addChild(DataNode_ptr);
   change_color_map(0);
   compute_total_xyz_and_hyper_bboxes();

   return curr_Cloud_ptr;
}

// ==========================================================================
// Colormap modification member functions
// ==========================================================================

// Member function reload_all_colors() loops over all PointClouds and
// calls their reload_colormap_array() methods.  It also resets the
// LatLongGrid's color from grey to purple if background pointcloud
// appears on greyscale colormap.

void PointCloudsGroup::reload_all_colors()
{
//   cout << "inside PointCloudsGroup::reload_all_colors() " << endl;
//   cout << "get_color_mapnumber = " << get_color_mapnumber() << endl;
//   cout << "height_ColorMap_ptr->get_dependent_var() = "
//        << height_ColorMap_ptr->get_dependent_var() << endl;

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      get_Cloud_ptr(n)->reload_colormap_array();
   } 
}

// ---------------------------------------------------------------------
// Member function update_dynamic_Grid_color() resets the dynamic
// Grid's color from grey to purple if the current colormap number ==
// 6 (greyscale background).  Otherwise, this method resets the
// dynamic Grid color back to grey.  

void PointCloudsGroup::update_dynamic_Grid_color()
{
//   cout << "inside PointCloudsGroup::update_dynamic_Grid_color() " << endl;
//   cout << "get_color_mapnumber = " << get_color_mapnumber() << endl;
//   cout << "height_ColorMap_ptr->get_dependent_var() = "
//        << height_ColorMap_ptr->get_dependent_var() << endl;

   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Grid* Grid_ptr=get_Cloud_ptr(n)->get_Grid_ptr();
      if (Grid_ptr != NULL)
      {
         Grid_ptr->destroy_dynamic_grid_lines();

// If current colormap corresponds to greyscale or if dependent var
// corresponds to p-channel (which is forced to appear on a greyscale
// background in the Afghanistan LOS program), reset LatLongGrid's
// color to purple.  Otherwise, reset LatLongGrid's color to grey.

         if (get_color_mapnumber()==6 ||
             height_ColorMap_ptr->get_dependent_var()==3)
         {
//            cout << "Change grid color to purple" << endl;
            Grid_ptr->set_curr_color(
               colorfunc::get_OSG_color(colorfunc::brightpurple));
         }
         else
         {
//            cout << "Change grid color to grey" << endl;
            Grid_ptr->set_curr_color(
               colorfunc::get_OSG_color(colorfunc::grey));
         }
         Grid_ptr->update_grid_text_color();
         Grid_ptr->redraw_long_lat_lines();
//         bool refresh_flag=true;
//         Grid_ptr->redraw_long_lat_lines(refresh_flag);
      }
   } // loop over index n labeling PointClouds
}

// ---------------------------------------------------------------------
void PointCloudsGroup::next_dependent_var()
{
//   cout << "inside PointCloudsGroup::next_dependent_var()" << endl;
   change_dependent_coloring_var(1);
}

void PointCloudsGroup::prev_dependent_var()
{
//   cout << "inside PCG::prev_dependent_var()" << endl;

// For reasons we don't understand as of April 2007, database paging
// works fine when we increment the dependent coloring var by +1 but
// has problems when we decrement by -1.  So we experiment with
// instead incrementing by N_DEPENDENT_VARS-1:

//   change_dependent_coloring_var(-1);
   change_dependent_coloring_var(
      height_ColorMap_ptr->get_n_dependent_vars()-1);
}

void PointCloudsGroup::change_dependent_coloring_var(int var_increment)
{
//   cout << "inside PointCloudsGroup::change_dependent_coloring_var()" << endl;
   height_ColorMap_ptr->change_dependent_coloring_var(var_increment);
   update_ColorbarHUD();
   reload_all_colors();
}

void PointCloudsGroup::set_dependent_coloring_var(int var)
{
//   cout << "inside PointCloudsGroup::set_dependent_coloring_var()" << endl;
   height_ColorMap_ptr->set_dependent_var(var);
   update_ColorbarHUD();
   reload_all_colors();
}

int PointCloudsGroup::get_dependent_coloring_var() const
{
   return height_ColorMap_ptr->get_dependent_var();
}

// ---------------------------------------------------------------------
// Member function update_ColorbarHUD() checks if ColorbarHUD_ptr !=
// NULL.  If so, it sets the colorbar's index to 1 for p maps.
// Otherwise, this method sets the colorbar's index to 0 for x,y,z
// maps.

void PointCloudsGroup::update_ColorbarHUD()
{
//   cout << "inside PointCloudsGroup::update_ColorbarHUD()" << endl;

   if (ColorbarHUD_ptr != NULL)
   {
      int dependent_coloring_var=get_dependent_coloring_var();
      if (dependent_coloring_var==0 || dependent_coloring_var==1 ||
          dependent_coloring_var==2)
      {
         ColorbarHUD_ptr->set_colorbar_index(curr_colorbar_index);
      }
      else if (dependent_coloring_var==3)
      {
         ColorbarHUD_ptr->set_colorbar_index(1);
      }
   }
}

// ---------------------------------------------------------------------
void PointCloudsGroup::next_color_map()
{
   change_color_map(1);
}

void PointCloudsGroup::prev_color_map()
{
   change_color_map(-1);
}

void PointCloudsGroup::change_color_map(int map_increment)
{
//   cout << "inside PointCloudsGroup::change_color_map()" << endl;
   height_ColorMap_ptr->increment_mapnumber(map_increment);
   reload_all_colors();
}

void PointCloudsGroup::set_height_color_map(int colormap_ID)
{
//   cout << "inside PointCloudsGroup::set_height_color_map()" << endl;
   height_ColorMap_ptr->set_mapnumber(colormap_ID);
   reload_all_colors();
}

void PointCloudsGroup::set_prob_color_map(int colormap_ID)
{
//   cout << "inside PointCloudsGroup::set_prob_color_map()" << endl;
   prob_ColorMap_ptr->set_mapnumber(colormap_ID);
   reload_all_colors();
}

int PointCloudsGroup::get_color_mapnumber() const
{
   return height_ColorMap_ptr->get_map_number();
}

// ---------------------------------------------------------------------
void PointCloudsGroup::adjust_cyclic_colormap_offset()
{
//   cout << "inside PointCloudsGroup::adjust_cyclic_colormap_offset()"
//        << endl;
   double offset=height_ColorMap_ptr->get_cyclic_frac_offset();
//   offset += 0.001;
   offset += 0.01;
   if (offset > 1) offset -= 1;
   height_ColorMap_ptr->set_cyclic_frac_offset(offset);
   cout << "Cyclic height colormap fractional offset = " << offset << endl;
   reload_all_colors();
}

// ---------------------------------------------------------------------
void PointCloudsGroup::set_heightmap_cyclic_colormap_offset(double offset)
{
//   cout << "inside PointCloudsGroup::set_heightmap_cyclic_colormap_offset()"
//        << endl;
   height_ColorMap_ptr->set_cyclic_frac_offset(offset);
   cout << "Cyclic height colormap fractional offset = " << offset << endl;
   reload_all_colors();
}

// ---------------------------------------------------------------------
// Member function toggle_colormap is a specialized utility method
// implemented for NYC demoing purposes.  It allows the user to change
// colormaps between the fused height/satellite EO coloring, grey
// scale and large hue value sans white.  

void PointCloudsGroup::toggle_colormap()
{
   if (colorscheme_toggle_counter%3==0)
   {
      get_ColorGeodeVisitor_ptr()->set_fixed_to_mutable_colors_flag(true);
      original_colormap_number=get_color_mapnumber();
      set_height_color_map(6); // grey
   }
   else if (colorscheme_toggle_counter%3==1)
   {
      get_ColorGeodeVisitor_ptr()->set_fixed_to_mutable_colors_flag(true);
      set_height_color_map(4); // large hue value sans white
   }
   else if (colorscheme_toggle_counter%3==2)
   {
      get_ColorGeodeVisitor_ptr()->set_mutable_to_fixed_colors_flag(true);
      set_height_color_map(original_colormap_number);
   }
   colorscheme_toggle_counter++;
}

// ---------------------------------------------------------------------
// Member function reset_cloud_coloring_to_zeroth_height_colormap()
// is called from the LOSServer for the LOST project.

void PointCloudsGroup::reset_cloud_coloring_to_zeroth_height_colormap()
{
//   cout << "inside PointCloudsGroup::reset_cloud_coloring_to_zeroth_height_colormap()" << endl;

   int coloring_scheme=0;	// Z -> hue; P -> value
   set_dependent_coloring_var(coloring_scheme);
   update_dynamic_Grid_color();
}

// ==========================================================================
// Threshold modification member functions
// ==========================================================================

void PointCloudsGroup::increase_min_threshold()
{
   change_min_threshold(1);
}

void PointCloudsGroup::decrease_min_threshold()
{
   change_min_threshold(-1);
}
   
void PointCloudsGroup::change_min_threshold(int sgn)
{
//   cout << "inside PointCloudsGroup::change_min_threshold, sgn = " << sgn
//        << endl;
   float min_threshold=height_ColorMap_ptr->get_min_threshold();
   float max_threshold=height_ColorMap_ptr->get_max_threshold();
   float delta_threshold=max_threshold-min_threshold;

//   cout << "min_thresh = " << min_threshold
//        << " max_thresh = " << max_threshold
//        << " dthresh = " << delta_threshold << endl;
//   cout << "height colormap depend var = "
//        << height_ColorMap_ptr->get_dependent_var() << endl;

   float min_value=get_min_value(height_ColorMap_ptr->get_dependent_var());
//   cout << "min_value = " << min_value << endl;
   float new_min_threshold=basic_math::max(
      min_value,float(max_threshold-(1-sgn*0.1)*delta_threshold));
   set_min_threshold(new_min_threshold);
//   cout << "New minimum threshold = " << new_min_threshold << endl;
}

void PointCloudsGroup::set_min_threshold(float new_min_threshold)
{
//   cout << "inside PointCloudsGroup::set_min_threshold()" << endl;
   height_ColorMap_ptr->set_min_threshold(new_min_threshold);
   reload_all_colors();

   hyper_bbox.set(3,new_min_threshold,hyper_bbox.zMax());
   MyHyperFilter_refptr->setHyperBoundingBox(hyper_bbox);
}

void PointCloudsGroup::set_min_prob_threshold(float new_min_threshold)
{
//   cout << "inside PointCloudsGroup::set_min_prob_threshold()" << endl;
//   cout << "new min threshold = " << new_min_threshold << endl;
   prob_ColorMap_ptr->set_min_threshold(3,new_min_threshold);

   hyper_bbox.set(4,new_min_threshold,hyper_bbox.wMax(0));
   MyHyperFilter_refptr->setHyperBoundingBox(hyper_bbox);

   reload_all_colors();
}

double PointCloudsGroup::get_min_threshold() const
{
   return height_ColorMap_ptr->get_min_threshold();
}

// ---------------------------------------------------------------------
void PointCloudsGroup::increase_max_threshold()
{
   change_max_threshold(1);
}

void PointCloudsGroup::decrease_max_threshold()
{
   change_max_threshold(-1);
}
   
void PointCloudsGroup::change_max_threshold(int sgn)
{
//   cout << "inside PointCloudsGroup::change_max_threshold(), sgn = " << sgn
//        << endl;
   float min_threshold=height_ColorMap_ptr->get_min_threshold();
   float max_threshold=height_ColorMap_ptr->get_max_threshold();
   float delta_threshold=max_threshold-min_threshold;

   float max_value=get_max_value(height_ColorMap_ptr->get_dependent_var());
   float new_max_threshold=basic_math::min(
      max_value,float(min_threshold+(1+sgn*0.1)*delta_threshold));
   set_max_threshold(new_max_threshold);
//   cout << "New maximum threshold = " << new_max_threshold << endl;
}

void PointCloudsGroup::set_max_threshold(float new_max_threshold)
{
//   cout << "inside PointCloudsGroup::set_max_threshold, new_max_threshold = " 
//        << new_max_threshold << endl;
   
   height_ColorMap_ptr->set_max_threshold(new_max_threshold);
   reload_all_colors();

   hyper_bbox.set(3,hyper_bbox.zMin(),new_max_threshold);
   MyHyperFilter_refptr->setHyperBoundingBox(hyper_bbox);
}

void PointCloudsGroup::set_max_prob_threshold(float new_max_threshold)
{
//   cout << "inside PointCloudsGroup::set_max_prob_threshold()" << endl;
//   cout << "new_max_threshold = " << new_max_threshold << endl;
   
   prob_ColorMap_ptr->set_max_threshold(3,new_max_threshold);

   hyper_bbox.set(4,hyper_bbox.wMin(0),new_max_threshold);
   MyHyperFilter_refptr->setHyperBoundingBox(hyper_bbox);

   reload_all_colors();
}

double PointCloudsGroup::get_max_threshold() const
{
   return height_ColorMap_ptr->get_max_threshold();
}

// --------------------------------------------------------------------------
// Member function update_display() modifies the point size based upon
// the virtual camera's height above the Z-grid if member
// Terrain_Manipulator_ptr != NULL.

void PointCloudsGroup::update_display()
{
//   cout << "inside PointCloudsGroup::update_display()" << endl;
//   cout << "update_display_flag = " << update_display_flag
//        << " time_dependence_flag = " << time_dependence_flag << endl;

   if (!update_display_flag) return;

   if (time_dependence_flag)
   {
      update_Switch();
   }

   if (Terrain_Manipulator_ptr != NULL && auto_resize_points_flag)
   {
      double camera_height_above_zgrid=
         Terrain_Manipulator_ptr->get_camera_height_above_grid();
//      cout << "camera_height_above_zgrid = "
//           << camera_height_above_zgrid << endl;

      if (camera_height_above_zgrid < 
          200*point_transition_altitude_factor)
      {
         set_pt_size(4);
      }
      else if (camera_height_above_zgrid < 
               400*point_transition_altitude_factor)
      {
         set_pt_size(3);
      }
      else if (camera_height_above_zgrid >= 
               400*point_transition_altitude_factor)
      {
         set_pt_size(2);
      }
//      cout << "pt_size = " << get_pt_size() << endl;

//      cout << "pt size = " << get_pt_size() << endl;
   } // Terrain_Manipulator_ptr != NULL && auto_resize_points_flag conditional

   GraphicalsGroup::update_display();
}

// ==========================================================================
// ActiveMQ message member functions
// ==========================================================================

void PointCloudsGroup::broadcast_cloud_params()
{
//   cout << "inside PointCloudsGroup::broadcast_cloud_params()" << endl;

   string command,key,value;
   vector<Messenger::Property> properties;

   const int n_precision=3;

   command="SEND_CLOUD_PARAMS";

   key="Npoints";
   value=stringfunc::number_to_string(get_ntotal_points());
   properties.push_back(Messenger::Property(key,value));

   key="MinX";
   value=stringfunc::number_to_string(xyz_bbox.xMin(),n_precision);
   properties.push_back(Messenger::Property(key,value));

   key="MaxX";
   value=stringfunc::number_to_string(xyz_bbox.xMax(),n_precision);
   properties.push_back(Messenger::Property(key,value));

   key="MinY";
   value=stringfunc::number_to_string(xyz_bbox.yMin(),n_precision);
   properties.push_back(Messenger::Property(key,value));

   key="MaxY";
   value=stringfunc::number_to_string(xyz_bbox.yMax(),n_precision);
   properties.push_back(Messenger::Property(key,value));

// Broadcast min/max Z and P thresholds rather than actual extremal Z
// and P values:

   key="MinZ";
   value=stringfunc::number_to_string(
      height_ColorMap_ptr->get_min_threshold(2),n_precision);
   properties.push_back(Messenger::Property(key,value));

   key="MaxZ";
   value=stringfunc::number_to_string(
      height_ColorMap_ptr->get_max_threshold(2),n_precision);
   properties.push_back(Messenger::Property(key,value));

   key="MinP";
   value=stringfunc::number_to_string(
      prob_ColorMap_ptr->get_min_threshold(3),n_precision);
   properties.push_back(Messenger::Property(key,value));

   key="MaxP";
   value=stringfunc::number_to_string(
      prob_ColorMap_ptr->get_max_threshold(3),n_precision);
   properties.push_back(Messenger::Property(key,value));

   get_Messenger_ptr()->broadcast_subpacket(command,properties);

//   outputfunc::enter_continue_char();
}

// ========================================================================== 
// Red actor path network member functions
// ========================================================================== 

// Member function reset_PolyLine_heights_using_TilesGroup() loops
// over all PolyLines within *PolyLinesGroup_ptr.  It resamples each
// PolyLine along regularly spaced edge points separated by ds=10
// meters.  This method extracts Z values from an ALIRT map for the
// resampled path points.

void PointCloudsGroup::reset_PolyLine_heights_using_TilesGroup()
{
   cout << "inside PointCloudsGroup::reset_PolyLine_heights_using_TilesGroup()" 
        << endl;

   if (PolyLinesGroup_ptr==NULL) return;
   if (TilesGroup_ptr==NULL) return;

   unsigned int n_PolyLines=PolyLinesGroup_ptr->get_n_Graphicals();
   for (unsigned int n=0; n<n_PolyLines; n++)
   {
      PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->get_PolyLine_ptr(n);

      const double ds=10;	// meters
      vector<threevector> XYZ;
      PolyLine_ptr->get_polyline_ptr()->compute_regularly_spaced_edge_points(
         ds,XYZ);

      unsigned int n_vertices=XYZ.size();
      cout << "n_vertices = " << n_vertices << endl;
      vector<threevector> new_XYZ;
      const double dz_above_terrain=2;	// meters
      double prev_z_terrain=XYZ[0].get(2);
      for (unsigned int i=0; i<n_vertices; i++)
      {
         threevector curr_XYZ(XYZ[i]);
         double z_terrain;
         if (TilesGroup_ptr->get_ladar_z_given_xy(
            curr_XYZ.get(0),curr_XYZ.get(1),z_terrain))
         {
            XYZ[i].put(2,z_terrain+dz_above_terrain);
            prev_z_terrain=z_terrain;
         }
         else
         {
            XYZ[i].put(2,prev_z_terrain+dz_above_terrain);
         }
//         cout << "i = " << i << " Zorig = " << XYZ[i].get(2)
//              << " Znew = " << curr_XYZ.get(2) << endl;
      }

      PolyLine_ptr=PolyLinesGroup_ptr->regenerate_PolyLine(
         XYZ,PolyLine_ptr,PolyLine_ptr->get_permanent_color(),
         PolyLine_ptr->get_selected_color());
      PolyLine_ptr->get_polyline_ptr()->compute_regularly_spaced_edge_points(
         ds,XYZ);
      PolyLine_ptr=PolyLinesGroup_ptr->regenerate_PolyLine(
         XYZ,PolyLine_ptr,PolyLine_ptr->get_permanent_color(),
         PolyLine_ptr->get_selected_color());
   } // loop over index n labeling PolyLines

   generate_tracks_from_PolyLines();
}

// ---------------------------------------------------------------------
// Member function generate_tracks_from_PolyLines()

void PointCloudsGroup::generate_tracks_from_PolyLines()
{
   cout << "inside PointCloudsGroup::generate_tracks_from_PolyLines()" 
        << endl;

   if (PolyLinesGroup_ptr==NULL) return;
   if (tracks_group_ptr==NULL) return;

   unsigned int n_PolyLines=PolyLinesGroup_ptr->get_n_Graphicals();
   for (unsigned int n=0; n<n_PolyLines; n++)
   {
      PolyLine* PolyLine_ptr=PolyLinesGroup_ptr->get_PolyLine_ptr(n);
   
// Form ground track from polyline assuming constant mover speed.
// Take starting time for track to equal zero (as opposed to some
// large number of seconds since epoch):

      polyline* polyline_ptr=PolyLine_ptr->get_polyline_ptr();
      const double mover_speed=3*1000/3600.0;  // 3 km/hour = 0.83 m/sec
      double path_length=polyline_ptr->get_total_length();
      double groundpath_traversal_time=path_length/mover_speed;

// Reset master stop time = start time + ground path traversal time in
// secs.  Then recompute number of animation frames:

      Operations_ptr->set_master_world_stop_time(
         Operations_ptr->get_master_world_start_time()+
         groundpath_traversal_time);

//   cout << "world start time = "
//        << Operations_ptr->get_master_world_start_time() << endl;
//   cout << "world stop time = "
//        << Operations_ptr->get_master_world_stop_time() << endl;
   
      get_AnimationController_ptr()->set_world_time_params(
         Operations_ptr->get_master_world_start_time(),
         Operations_ptr->get_master_world_stop_time(),
         Operations_ptr->get_delta_master_world_time_step_per_master_frame());

/*
      cout << "AFTER revising master world stop time:" << endl;

      cout << "Operations.master_world_start_time = "
           << Operations_ptr->get_master_world_start_time() << endl;
      cout << "Operations.master_world_stop_time = "
           << Operations_ptr->get_master_world_stop_time() << endl;
      cout << "Operations.master_world_dt_per_frame = "
           << Operations_ptr->get_delta_master_world_time_step_per_master_frame() 
           << endl;
      cout << "n_frames = " << get_AnimationController_ptr()->get_nframes() 
           << endl;
      cout << "AC_ptr = " << get_AnimationController_ptr() << endl;
      cout << "AC first framenumber = "
           << get_AnimationController_ptr()->get_first_framenumber() << endl;
      cout << "AC last framenumber = "
           << get_AnimationController_ptr()->get_last_framenumber() << endl;
      cout << "dt per frame = " 
           << get_AnimationController_ptr()->get_delta_world_time_per_frame() 
           << endl;
*/

      int n_vertices=get_AnimationController_ptr()->get_nframes();
      double dt=get_AnimationController_ptr()->
         get_delta_world_time_per_frame();
      cout << "n_vertices = " << n_vertices << " dt = " << dt << endl;
      
      track* groundtrack_ptr=tracks_group_ptr->generate_new_track();
      for (int n=0; n<n_vertices; n++)
      {
         double curr_t=n*dt;
         double frac=double(n)/double(n_vertices);
         threevector track_posn=polyline_ptr->edge_point(frac);
         bool temporally_sort_flag=false;
         groundtrack_ptr->set_posn_velocity(
            curr_t,track_posn,Zero_vector,temporally_sort_flag);
      }
      groundtrack_ptr->compute_average_velocities();

//      cout << "ground track = " << *groundtrack_ptr << endl;

   } // loop over index n labeling PolyLines
}
