// ==========================================================================
// Header file for POLYLINESGROUP class
// ==========================================================================
// Last modified on 5/1/14; 1/22/16; 7/6/16; 7/7/16
// ==========================================================================

#ifndef POLYLINESGROUP_H
#define POLYLINESGROUP_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <osg/Depth>
#include <osg/Group>
#include <osg/LineWidth>
#include <osg/Vec4>
#include "osg/Custom3DManipulator.h"
#include "osg/osgSceneGraph/DataGraph.h"
#include "osg/osgGeometry/GeometricalsGroup.h"
#include "track/mover.h"
#include "track/movers_group.h"
#include "video/photogroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLine.h"

class PointFinder;
class postgis_database;

// Forward declaration of PolygonsGroup class which sits inside
// osgGeometry namespace:

namespace osgGeometry
{
   class PolygonsGroup;
}

class PolyLinesGroup : public GeometricalsGroup
{

  public:

   typedef std::map<string, std::vector<bounding_box> > ANNOTATED_BBOXES_MAP;
// independent string: image_ID_str
// dependent STL vector: annotated bboxes

   typedef map<int, pair<string, int> > OSG_BBOXES_MAP;
// independent int: OSG PolyLine ID
// dependent pair:  string = image_ID_str, int = curr image bbox index

   typedef map<string, pair<int, int> > IMAGE_SIZES_MAP;
// independent int: image_ID_str
// dependent pair:  xdim, ydim


// Initialization, constructor and destructor functions:

// Note added at noontime, Mon Feb 18, 2008: Should eventually
// eliminate first constructor in favor of more general 2nd
// constructor...

   PolyLinesGroup(const int p_ndims,Pass* PI_ptr,threevector* GO_ptr=NULL);
   PolyLinesGroup(const int p_ndims,Pass* PI_ptr,AnimationController* AC_ptr,
                  threevector* GO_ptr=NULL);
   PolyLinesGroup(
      const int p_ndims,Pass* PI_ptr,osgGeometry::PolygonsGroup* PG_ptr,
      AnimationController* AC_ptr, threevector* GO_ptr=NULL);
   PolyLinesGroup(
      const int p_ndims,Pass* PI_ptr,osgGeometry::PolygonsGroup* PG_ptr,
      PolyhedraGroup* PHG_ptr,AnimationController* AC_ptr, 
      threevector* GO_ptr=NULL);

   PolyLinesGroup(const int p_ndims,Pass* PI_ptr,postgis_database* bgdb_ptr,
                  threevector* GO_ptr=NULL);

   virtual ~PolyLinesGroup();

   friend std::ostream& operator<< 
      (std::ostream& outstream,const PolyLinesGroup& P);

// Set & get methods:

   void set_altitude_dependent_labels_flag(bool flag);
   void set_ID_labels_flag(bool flag);
   bool get_ID_labels_flag() const;
   void set_variable_Point_size_flag(bool flag);
   bool get_variable_Point_size_flag() const;

   PolyLine* get_PolyLine_ptr(int n) const;
   PolyLine* get_ID_labeled_PolyLine_ptr(int ID) const;
   PolyLine* get_selected_PolyLine_ptr() const;
   std::vector<PolyLine*> get_all_PolyLine_ptrs() const;

   void set_skeleton_height(double height);
   void set_constant_vertices_altitude(double alt);
   double get_constant_vertices_altitude() const;
   void set_movers_group_ptr(movers_group* mg_ptr);
   void set_Pointsize_scalefactor(double factor);
   double get_Pointsize_scalefactor() const;
   void set_textsize_scalefactor(double factor);
   double get_textsize_scalefactor() const;
   
   movers_group* get_movers_group_ptr() const;
   osgGeometry::PointsGroup* get_Intersection_PointsGroup_ptr();
   const osgGeometry::PointsGroup* get_Intersection_PointsGroup_ptr() const;
   void set_PolygonsGroup_ptr(osgGeometry::PolygonsGroup* PolygonsGroup_ptr);
   osgGeometry::PolygonsGroup* get_PolygonsGroup_ptr();
   const osgGeometry::PolygonsGroup* get_PolygonsGroup_ptr() const;
   PolyhedraGroup* get_PolyhedraGroup_ptr();
   const PolyhedraGroup* get_PolyhedraGroup_ptr() const;
   void set_DataGraph_ptr(DataGraph* DG_ptr);
   void set_PolyLinesGroup_3D_ptr(PolyLinesGroup* PLG_ptr);

   void set_imageplane_PolyLinesGroup_ptr(PolyLinesGroup* PLG_ptr);
   PolyLinesGroup* get_imageplane_PolyLinesGroup_ptr();
   void set_photogroup_ptr(photogroup* pg_ptr);
   void set_annotated_bboxes_map_ptr(ANNOTATED_BBOXES_MAP* abm_ptr);
   void set_osg_bboxes_map_ptr(OSG_BBOXES_MAP* osgm_ptr);
   void set_image_sizes_map_ptr(IMAGE_SIZES_MAP* ism_ptr);

// PolyLine generation member functions:

   PolyLine* generate_new_PolyLine(
      bool force_display_flag=false,bool single_polyline_per_geode_flag=true,
      int n_text_messages=0,int ID=-1,int OSGsubPAT_number=0);
   PolyLine* generate_new_PolyLine(
      const threevector& reference_origin,
      bool force_display_flag=false,bool single_polyline_per_geode_flag=true,
      int n_text_messages=0,int ID=-1,int OSGsubPAT_number=0);
   PolyLine* generate_new_PolyLine(
      const std::vector<threevector>& V,bool force_display_flag=false,
      bool single_polyline_per_geode_flag=true,int n_text_messages=0,
      int ID=-1,int OSGsubPAT_number=0);
   PolyLine* generate_new_PolyLine(
      const threevector& reference_origin,const std::vector<threevector>& V,
      bool force_display_flag=false,bool single_polyline_per_geode_flag=true,
      int n_text_messages=0,int ID=-1,int OSGsubPAT_number=0);
   PolyLine* generate_new_PolyLine(
      const std::vector<threevector>& V,const osg::Vec4& uniform_color,
      bool force_display_flag=false,
      bool single_polyline_per_geode_flag=true,int n_text_messages=0,
      int ID=-1,int OSGsubPAT_number=0);
   PolyLine* generate_new_PolyLine(
      const threevector& reference_origin,const std::vector<threevector>& V,
      const osg::Vec4& uniform_color,bool force_display_flag=false,
      bool single_polyline_per_geode_flag=true,int n_text_messages=0,
      int ID=-1,int OSGsubPAT_number=0);
   PolyLine* generate_new_PolyLine(
      const threevector& reference_origin,const std::vector<threevector>& V,
      const std::vector<osg::Vec4>& colors,bool force_display_flag=false,
      bool single_polyline_per_geode_flag=true,int n_text_messages=0,
      int ID=-1,int OSGsubPAT_number=0);

   PolyLine* generate_UV_PolyLine(
      const threevector& l,int n_text_messages=0,int ID=-1,
      int OSGsubPAT_number=0);

// PolyLine destruction member functions:

   void destroy_all_PolyLines();
   bool destroy_PolyLine();
   bool destroy_PolyLine(int ID);
   bool destroy_PolyLine(PolyLine* curr_PolyLine_ptr);

// PolyLine manipulation member functions:

   PolyLine* regenerate_PolyLine(
      const std::vector<threevector>& vertices,
      PolyLine* orig_PolyLine_ptr,osg::Vec4 permanent_color,
      osg::Vec4 selected_color,bool display_vertices_flag=true,int ID=-1);
   void form_polyhedron_skeleton(
      PolyLine* bottom_PolyLine_ptr,
      osg::Vec4 permanent_color,osg::Vec4 selected_color,
      bool display_vertices_flag=true);
   void lowpass_filter_polyline(PolyLine* curr_PolyLine_ptr);
   void reset_PolyLine_altitudes(PolyLine* curr_PolyLine_ptr);
   PolyLine* merge_PolyLines(PolyLine* original_PolyLine_ptr);
   
   void move_z(double dz);
   void rescale(PolyLine* PolyLine_ptr,double ds);
   void rotate(PolyLine* PolyLine_ptr,double ds);

// PolyLine display properties member functions:

   void set_width(double width);
   double get_width() const;
   
   void adjust_depth_buffering(bool force_display_flag);
   void set_uniform_color(colorfunc::Color uniform_color);
   void set_uniform_color(const osg::Vec4& uniform_color);
   void set_uniform_color(
      const osg::Vec4& uniform_color,int OSGsubPAT_number);
   void recolor_based_on_zcolormap_dependent_var();

//   bool erase_PolyLine();
//   bool unerase_PolyLine();

// Update member functions:

   void update_display();
   void compute_altitude_dependent_text_label_sizes();

// Polyline properties member functions:

   threevector compute_vertices_average(const std::vector<threevector>& V);

// PolyLine intersection member functions:

   std::vector<threevector> find_intersection_points(
      bool northern_hemisphere_flag,int UTM_zonenumber,
      double endpoint_distance_threshold=1E-8,
      double max_dotproduct=0.99);
   void clip_PolyLines();

// Message handling member functions:

   int find_PolyLine_ID_given_track_label(int track_label);
   void issue_add_vertex_message(
      PolyLine* bottom_PolyLine_ptr,colorfunc::Color mover_color,
      std::string annotation_label="");

// Text label member functions:

   void set_next_PolyLine_label(std::string label);
   std::string get_next_PolyLine_label() const;
   void reset_labels();

   void set_next_PolyLine_mover_ptr(mover* mover_ptr);
   mover* get_next_PolyLine_mover_ptr() const;

// Ascii feature file I/O methods:

   bool export_info_to_file();
   std::string save_info_to_file();
   std::string save_info_to_file(std::string output_filename);
   bool read_info_from_file(
      std::string segments_filename,std::vector<double>& curr_time,
      std::vector<int>& polyline_ID,std::vector<int>& pass_number,
      std::vector<threevector>& V,std::vector<osg::Vec4>& color);
   std::string reconstruct_polylines_from_file_info();
   std::string reconstruct_polylines_from_file_info(
      std::string polylines_filename);

   void assign_heights_using_pointcloud(PointFinder* PF_ptr);

// PolyLine video plane projection member functions:

   bool project_PolyLines_into_selected_aerial_video_frame(
      double minU,double maxU,double minV,double maxV);
//   bool project_PolyLines_into_selected_aerial_video_frame();

// polygon member functions:

   void convert_polylines_to_polygons();

// Bounding box labeling methods:

   void increment_frame();
   void decrement_frame();
   void increment_currimage_PolyLine();
   void decrement_currimage_PolyLine();
   void set_PolyLine_attribute(int attribute_ID);
   void set_all_PolyLine_attributes(int attribute_ID);
   void display_PolyLine_attribute(
      int PolyLine_ID, std::string attribute_value);
   void write_bboxes_to_file();
   void generate_image_bboxes(std::string image_ID_str);

  protected:

   movers_group* movers_group_ptr;

  private:

   bool shrunken_label_flag,altitude_dependent_labels_flag;
   bool ID_labels_flag;
   bool variable_Point_size_flag;
   std::string next_PolyLine_label;
   int secs_since_Y2K;
   int prev_framenumber;
   int currimage_PolyLine_index;
   double skeleton_height,constant_vertices_altitude;
   double Pointsize_scalefactor,textsize_scalefactor;
   mover* next_PolyLine_mover_ptr;
   photogroup* photogroup_ptr;
   postgis_database* babygis_database_ptr;
   osgGeometry::PointsGroup* Intersection_PointsGroup_ptr;
   osgGeometry::PolygonsGroup* PolygonsGroup_ptr;
   PolyhedraGroup* PolyhedraGroup_ptr;
   PolyLinesGroup* PolyLinesGroup_3D_ptr;
   PolyLinesGroup* imageplane_PolyLinesGroup_ptr;
   osg::ref_ptr<osg::Depth> depth_off_refptr;
   osg::ref_ptr<osg::Depth> depth_on_refptr;
   osg::ref_ptr<osg::LineWidth> linewidth_refptr;
   DataGraph* DataGraph_ptr;
   
   ANNOTATED_BBOXES_MAP* annotated_bboxes_map_ptr;
   ANNOTATED_BBOXES_MAP::iterator annotated_bboxes_iter;

   OSG_BBOXES_MAP* osg_bboxes_map_ptr;
   OSG_BBOXES_MAP::iterator osg_bboxes_iter;

   IMAGE_SIZES_MAP* image_sizes_map_ptr;
   IMAGE_SIZES_MAP::iterator image_sizes_iter;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const PolyLinesGroup& f);
   
   void initialize_new_PolyLine(
      PolyLine* curr_PolyLine_ptr,bool single_polyline_per_geode_flag=true,
      bool force_display_flag=false,int OSGsubPAT_number=0);

   virtual bool parse_next_message_in_queue(message& curr_message);
   std::string get_image_ID_str();
   void set_selected_bbox();
   bounding_box* get_selected_bbox();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void PolyLinesGroup::set_altitude_dependent_labels_flag(bool flag)
{
   altitude_dependent_labels_flag=flag;
}

inline void PolyLinesGroup::set_ID_labels_flag(bool flag)
{
   ID_labels_flag=flag;
}

inline bool PolyLinesGroup::get_ID_labels_flag() const
{
   return ID_labels_flag;
}

inline void PolyLinesGroup::set_variable_Point_size_flag(bool flag)
{
   variable_Point_size_flag=flag;
}

inline bool PolyLinesGroup::get_variable_Point_size_flag() const
{
   return variable_Point_size_flag;
}

inline PolyLine* PolyLinesGroup::get_PolyLine_ptr(int n) const
{
   return dynamic_cast<PolyLine*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline PolyLine* PolyLinesGroup::get_ID_labeled_PolyLine_ptr(int ID) const
{
   return dynamic_cast<PolyLine*>(get_ID_labeled_Graphical_ptr(ID));
}

inline PolyLine* PolyLinesGroup::get_selected_PolyLine_ptr() const
{
   return dynamic_cast<PolyLine*>(get_selected_Graphical_ptr());
}

inline void PolyLinesGroup::set_skeleton_height(double height)
{
   skeleton_height=height;
}

inline void PolyLinesGroup::set_constant_vertices_altitude(double alt)
{
   constant_vertices_altitude=alt;
}

inline double PolyLinesGroup::get_constant_vertices_altitude() const
{
   return constant_vertices_altitude;
}

inline void PolyLinesGroup::set_Pointsize_scalefactor(double factor)
{
   Pointsize_scalefactor = factor;
}

inline double PolyLinesGroup::get_Pointsize_scalefactor() const
{
   return Pointsize_scalefactor;
}

inline void PolyLinesGroup::set_textsize_scalefactor(double factor)
{
   textsize_scalefactor = factor;
}

inline double PolyLinesGroup::get_textsize_scalefactor() const
{
   return textsize_scalefactor;
}

inline void PolyLinesGroup::set_movers_group_ptr(movers_group* mg_ptr)
{
   movers_group_ptr=mg_ptr;
}

inline movers_group* PolyLinesGroup::get_movers_group_ptr() const
{
   return movers_group_ptr;
}

inline osgGeometry::PointsGroup* 
PolyLinesGroup::get_Intersection_PointsGroup_ptr()
{
   return Intersection_PointsGroup_ptr;
}

inline const osgGeometry::PointsGroup* 
PolyLinesGroup::get_Intersection_PointsGroup_ptr() const
{
   return Intersection_PointsGroup_ptr;
}

inline void PolyLinesGroup::set_PolygonsGroup_ptr(
   osgGeometry::PolygonsGroup* PolygonsGroup_ptr)
{
   this->PolygonsGroup_ptr=PolygonsGroup_ptr;
}

inline osgGeometry::PolygonsGroup* PolyLinesGroup::get_PolygonsGroup_ptr()
{
   return PolygonsGroup_ptr;
}

inline const osgGeometry::PolygonsGroup* 
PolyLinesGroup::get_PolygonsGroup_ptr() const
{
   return PolygonsGroup_ptr;
}

inline PolyhedraGroup* PolyLinesGroup::get_PolyhedraGroup_ptr()
{
   return PolyhedraGroup_ptr;
}

inline const PolyhedraGroup* PolyLinesGroup::get_PolyhedraGroup_ptr() const
{
   return PolyhedraGroup_ptr;
}

inline void PolyLinesGroup::set_next_PolyLine_label(std::string label)
{
   next_PolyLine_label=label;
}

inline std::string PolyLinesGroup::get_next_PolyLine_label() const
{
   return next_PolyLine_label;
}

inline void PolyLinesGroup::set_next_PolyLine_mover_ptr(mover* mover_ptr)
{
   next_PolyLine_mover_ptr=mover_ptr;
}

inline mover* PolyLinesGroup::get_next_PolyLine_mover_ptr() const
{
   return next_PolyLine_mover_ptr;
}

inline void PolyLinesGroup::set_DataGraph_ptr(DataGraph* DG_ptr)
{
   DataGraph_ptr=DG_ptr;
}

inline void PolyLinesGroup::set_PolyLinesGroup_3D_ptr(PolyLinesGroup* PLG_ptr)
{
   PolyLinesGroup_3D_ptr=PLG_ptr;
}

inline void PolyLinesGroup::set_imageplane_PolyLinesGroup_ptr(
   PolyLinesGroup* PLG_ptr)
{
   imageplane_PolyLinesGroup_ptr=PLG_ptr;
}

inline PolyLinesGroup* PolyLinesGroup::get_imageplane_PolyLinesGroup_ptr()
{
   return imageplane_PolyLinesGroup_ptr;
}

inline void PolyLinesGroup::set_photogroup_ptr(photogroup* pg_ptr)
{
   photogroup_ptr=pg_ptr;
}

inline void PolyLinesGroup::set_annotated_bboxes_map_ptr(
   PolyLinesGroup::ANNOTATED_BBOXES_MAP* abm_ptr)
{
   annotated_bboxes_map_ptr = abm_ptr;
}

inline void PolyLinesGroup::set_osg_bboxes_map_ptr(
   PolyLinesGroup::OSG_BBOXES_MAP* osgm_ptr)
{
   osg_bboxes_map_ptr = osgm_ptr;
}

inline void PolyLinesGroup::set_image_sizes_map_ptr(
   PolyLinesGroup::IMAGE_SIZES_MAP* ism_ptr)
{
   image_sizes_map_ptr = ism_ptr;
}

#endif // PolyLinesGroup.h



