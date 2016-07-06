// ==========================================================================
// Header file for POINTCLOUDSGROUP class
// ==========================================================================
// Last modified on 12/2/11; 12/15/11; 12/18/11
// ==========================================================================

#ifndef POINTCLOUDSGROUP_H
#define POINTCLOUDSGROUP_H

#include <iostream>
#include <vector>
#include <osg/Group>
#include <osg/Node>
#include <osg/StateSet>
#include "osg/osg2D/ColorbarHUD.h"
#include "osg/osgSceneGraph/DataGraphsGroup.h"
#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "osg/osgOperations/Operations.h"
#include "osg/osg3D/PointCloud.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgTiles/TilesGroup.h"
#include "track/tracks_group.h"

class AnimationController;
class ColorGeodeVisitor;
class ColorMap;
class HiresDataVisitor;
class PassesGroup;
class TrianglesGroup;

class PointCloudsGroup : public DataGraphsGroup
{

  public:

// Initialization, constructor and destructor functions:

   PointCloudsGroup(Pass* PI_ptr,threevector* GO_ptr=NULL);
   PointCloudsGroup(
      Pass* PI_ptr,AnimationController* AC_ptr,threevector* GO_ptr=NULL);
   virtual ~PointCloudsGroup();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const PointCloudsGroup& P);

// Set & get methods:
 
   void set_point_transition_altitude_factor(double factor);
   PointCloud* get_Cloud_ptr(int n) const;
   PointCloud* get_ID_labeled_Cloud_ptr(int ID) const;

   ColorGeodeVisitor* get_ColorGeodeVisitor_ptr() const;
   HiresDataVisitor* get_HiresDataVisitor_ptr();
   void set_auto_resize_points_flag(bool flag);
   void set_time_dependence_flag(bool flag);
   void set_pt_size(double sz);
   double get_pt_size() const;
   double get_max_value(int i) const;
   double get_min_value(int i) const;
   int get_ntotal_points() const;

   void set_Terrain_Manipulator_ptr(osgGA::Terrain_Manipulator* TM_ptr);
   void set_ColorbarHUD_ptr(ColorbarHUD* HUD_ptr);
   void set_curr_colorbar_index(int index);
   int get_curr_colorbar_index() const;
   int get_prev_colorbar_index() const;

   void set_Operations_ptr(Operations* O_ptr);
   void set_PolyLinesGroup_ptr(PolyLinesGroup* PLG_ptr);
   void set_TilesGroup_ptr(TilesGroup* TG_ptr);
   void set_tracks_group_ptr(tracks_group* tg_ptr);

// Cloud creation and manipulation methods:

   std::vector<PointCloud*>* generate_Clouds(
      PassesGroup& passes_group,bool index_tree_flag=false,
      TrianglesGroup* TG_ptr=NULL);
   void generate_separate_clouds_from_input_files(Pass* pass_ptr);
   PointCloud* generate_new_Cloud(
      bool index_tree_flag=false,TrianglesGroup* TG_ptr=NULL,int ID=-1);
   PointCloud* generate_new_Cloud(
      Pass* curr_pass_ptr,bool index_tree_flag=false,
      TrianglesGroup* TG_ptr=NULL,int ID=-1);

// Colormap modification member functions:

   void reload_all_colors();
   void update_dynamic_Grid_color();

   void next_dependent_var();
   void prev_dependent_var();
   void change_dependent_coloring_var(int var_increment);
   void set_dependent_coloring_var(int var);
   int get_dependent_coloring_var() const;
   void next_color_map();
   void prev_color_map();
   void change_color_map(int map_increment);
   void set_height_color_map(int colormap_ID);
   void set_prob_color_map(int colormap_ID);
   int get_color_mapnumber() const;
   void adjust_cyclic_colormap_offset();
   void set_heightmap_cyclic_colormap_offset(double offset);
   void toggle_colormap();
   void update_ColorbarHUD();

   void reset_cloud_coloring_to_zeroth_height_colormap();

// Threshold modification member functions:

   void increase_min_threshold();
   void decrease_min_threshold();
   void change_min_threshold(int sgn);
   void set_min_threshold(float new_min_threshold);
   void set_min_prob_threshold(float new_min_threshold);
   double get_min_threshold() const;

   void increase_max_threshold();
   void decrease_max_threshold();
   void change_max_threshold(int sgn);
   void set_max_threshold(float new_max_threshold);
   void set_max_prob_threshold(float new_max_threshold);
   double get_max_threshold() const;

   void update_display();

// Point picking member functions

   void find_closest_worldspace_point();

// ActiveMQ message member functions

   void broadcast_cloud_params();

// Red actor path network member functions

   void reset_PolyLine_heights_using_TilesGroup();
   void generate_tracks_from_PolyLines();

  protected:

  private:

   bool auto_resize_points_flag,time_dependence_flag;
   int colorscheme_toggle_counter,original_colormap_number;
   int curr_colorbar_index,prev_colorbar_index;
   double point_transition_altitude_factor;
   ColorMap *height_ColorMap_ptr,*prob_ColorMap_ptr;

   osg::ref_ptr<ColorGeodeVisitor> ColorGeodeVisitor_refptr;
   osg::ref_ptr<HiresDataVisitor> HiresDataVisitor_refptr;
   
   std::vector<PointCloud*>* cloudptrs_ptr;
   ColorbarHUD* ColorbarHUD_ptr;

   osg::ref_ptr<osg::StateSet> StateSet_refptr;
   osg::ref_ptr<osg::Point> pt_refptr;
   osgGA::Terrain_Manipulator* Terrain_Manipulator_ptr;

   Operations* Operations_ptr;
   PolyLinesGroup* PolyLinesGroup_ptr;
   TilesGroup* TilesGroup_ptr;
   tracks_group* tracks_group_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const PointCloudsGroup& PCG);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void PointCloudsGroup::set_auto_resize_points_flag(bool flag)
{
   auto_resize_points_flag=flag;
}

inline void PointCloudsGroup::set_time_dependence_flag(bool flag)
{
   time_dependence_flag=flag;
}

// --------------------------------------------------------------------------
inline void PointCloudsGroup::set_point_transition_altitude_factor(
   double factor)
{
   point_transition_altitude_factor=factor;
}

// --------------------------------------------------------------------------
inline PointCloud* PointCloudsGroup::get_Cloud_ptr(int n) const
{
   return dynamic_cast<PointCloud*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline PointCloud* PointCloudsGroup::get_ID_labeled_Cloud_ptr(int ID) const
{
   return dynamic_cast<PointCloud*>(get_ID_labeled_Graphical_ptr(ID));
}

// --------------------------------------------------------------------------
inline ColorGeodeVisitor* PointCloudsGroup::get_ColorGeodeVisitor_ptr() const
{
   return ColorGeodeVisitor_refptr.get();
}

inline HiresDataVisitor* PointCloudsGroup::get_HiresDataVisitor_ptr()
{
   return HiresDataVisitor_refptr.get();
}

// --------------------------------------------------------------------------
inline void PointCloudsGroup::set_pt_size(double sz)
{
   if (sz < 1) sz=1;
   pt_refptr->setSize(sz);
}

inline double PointCloudsGroup::get_pt_size() const
{
   return pt_refptr->getSize();
}

// --------------------------------------------------------------------------
inline double PointCloudsGroup::get_max_value(int i) const
{
//    double max_p=NEGATIVEINFINITY;
   switch (i)
   {
      case 0:
         return xyz_bbox.xMax();
         break;
      case 1:
         return xyz_bbox.yMax();
         break;
      case 2:
         return xyz_bbox.zMax();
         break;
      case 3:

//         for (unsigned int n=0; n<get_n_Graphicals(); n++)
//         {
//            max_p=basic_math::max(max_p,get_Cloud_ptr(n)->get_max_value(3));
//         }
//         return max_p;
         return 1;
   }

   std::cout << "Error in PointCloudsGroup::get_max_value() !" << std::endl;
   std::cout << "Input i = " << i << " lies out of bounds" << std::endl;
   return NEGATIVEINFINITY;
}

// --------------------------------------------------------------------------
inline double PointCloudsGroup::get_min_value(int i) const
{
//    double min_p=POSITIVEINFINITY;
   switch (i)
   {
      case 0:
         return xyz_bbox.xMin();
         break;
      case 1:
         return xyz_bbox.yMin();
         break;
      case 2:
         return xyz_bbox.zMin();
         break;
      case 3:
//         for (unsigned int n=0; n<get_n_Graphicals(); n++)
//         {
//            min_p=basic_math::min(min_p,get_Cloud_ptr(n)->get_min_value(3));
//         }
//         return min_p;
	 return 0;
   }

   std::cout << "Error in PointCloudsGroup::get_min_value() !" << std::endl;
   std::cout << "Input i = " << i << " lies out of bounds" << std::endl;
   return POSITIVEINFINITY;
}

// --------------------------------------------------------------------------
inline int PointCloudsGroup::get_ntotal_points() const
{
   int ntotal_points=0;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      ntotal_points += get_Cloud_ptr(n)->get_npoints();
   }
   return ntotal_points;
}

// --------------------------------------------------------------------------
inline void PointCloudsGroup::set_Terrain_Manipulator_ptr(
   osgGA::Terrain_Manipulator* TM_ptr)
{
   Terrain_Manipulator_ptr=TM_ptr;
}

// --------------------------------------------------------------------------
inline void PointCloudsGroup::set_ColorbarHUD_ptr(ColorbarHUD* HUD_ptr)
{
   ColorbarHUD_ptr=HUD_ptr;
}

inline void PointCloudsGroup::set_curr_colorbar_index(int index)
{
//   std::cout << "inside PointCloudsGroup::set_curr_colorbar_index()" 
//             << std::endl;
   prev_colorbar_index=curr_colorbar_index;
   curr_colorbar_index=index;
}

inline int PointCloudsGroup::get_curr_colorbar_index() const
{
   return curr_colorbar_index;
}

inline int PointCloudsGroup::get_prev_colorbar_index() const
{
   return prev_colorbar_index;
}

inline void PointCloudsGroup::set_Operations_ptr(Operations* O_ptr)
{
   Operations_ptr=O_ptr;
}

inline void PointCloudsGroup::set_PolyLinesGroup_ptr(PolyLinesGroup* PLG_ptr)
{
   PolyLinesGroup_ptr=PLG_ptr;
}

inline void PointCloudsGroup::set_TilesGroup_ptr(TilesGroup* TG_ptr)
{
   TilesGroup_ptr=TG_ptr;
}

inline void PointCloudsGroup::set_tracks_group_ptr(tracks_group* tg_ptr)
{
   tracks_group_ptr=tg_ptr;
}



#endif // PointCloudsGroup.h



