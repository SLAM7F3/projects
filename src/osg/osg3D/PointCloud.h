// ==========================================================================
// Header file for POINTCLOUD class
// ==========================================================================
// Last modified on 12/13/10; 11/27/11; 12/18/11; 4/6/14
// ==========================================================================

#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include <osg/Array>
#include <osg/Geode>
#include <osg/Point>
#include <set>
#include <string>
#include <vector>
#include "osg/osgSceneGraph/ColorGeodeVisitor.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "datastructures/Databin.h"
#include "osg/osgSceneGraph/DataGraph.h"
#include "osg/osgGraphicals/Graphical.h"
#include "osg/osgGrid/Grid.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "model/HyperBoundingBox.h"
#include "ladar/ladarimage.h"
#include "model/Metadata.h"
#include "osg/osgSceneGraph/SetupGeomVisitor.h"
#include "image/TwoDarray.h"

class ColormapPtrs;
class genmatrix;
class ladarimage;
class TrianglesGroup;

class PointCloud : public DataGraph
{

  public:

   PointCloud(
      Pass* currpass_ptr,
      LeafNodeVisitor* LNV_ptr,TreeVisitor* TV_ptr,ColorGeodeVisitor* CGV_ptr,
      SetupGeomVisitor* SGV_ptr,HiresDataVisitor* HRV_ptr,
      int ID=-1,TrianglesGroup* TG_ptr=NULL);

   virtual ~PointCloud();

// Set & get member functions:

   unsigned int get_npoints() const;
   void set_max_threshold_frac(double f);
   void set_min_threshold_frac(double f);
   
   double get_max_value(int i) const;
   double get_min_value(int i) const;
   double get_avg_value(int i) const;
   double get_theta_xy_uv() const;
   void get_extremal_uv_values(
      double& umin,double& umax,double& vmin,double& vmax) const;

   osg::Vec3Array* get_vertices_ptr();
   model::Metadata* get_metadata_ptr();
   osg::Vec4ubArray* get_colors_ptr();
   std::vector<bool>* get_shadows_ptr();
   std::vector<threevector>* get_normals_ptr();

   ladarimage* get_ladarimage_ptr();
   const ladarimage* get_ladarimage_ptr() const;
   void set_ColorGeodeVisitor(ColorGeodeVisitor* CGV_ptr);
   void set_SetupGeomVisitor(SetupGeomVisitor* SGV_ptr);
   void set_HiresDataVisitor(HiresDataVisitor* HRV_ptr);
   HiresDataVisitor* get_HiresDataVisitor_ptr();
   const HiresDataVisitor* get_HiresDataVisitor_ptr() const;
   void set_Grid_ptr(Grid* G_ptr);
   Grid* get_Grid_ptr() const;

// Input member functions:   

   void parse_input_data();

   void read_normals_from_xyz_file();
   void read_normals_from_xyzp_file();

// Scene graph generation member functions:

   osg::Node* GenerateCloudGraph(bool index_tree_flag=false);
   void InitializeCloudGraph();
   void Generate_Ross_Tree(
      const osg::Vec3Array* vertices_ptr, 
      const model::Metadata* metadata_ptr,
      const osg::Vec4ubArray* colors_ptr,
      const osg::UIntArray* indices_ptr, 
      const osg::BoundingBox& box,bool index_tree_flag=false);
   threevector get_zeroth_vertex();

// Color manipulation member functions:

   void reload_colormap_array();
   void pure_hues();
   void pure_intensities();
   void modify_hues();
   void modify_intensities();
   void equalize_intensity_histogram();
   void remap_intensities();
   void check_colors_array();
   
// P-value retrieval member functions:

   int get_curr_p(float& curr_p);
   float get_curr_p();
   void set_curr_p(float curr_p);

// Point manipulation member functions:

   void transform_vertices();
   void mark_aboveTIN_snowflake_points(std::string triangles_subdir);
   void remove_snowflakes(std::string triangles_subdir,
                          double ceiling_min_z=POSITIVEINFINITY);
   
// Symmetry direction determination member functions:

   threevector center_of_mass();
   double XYplane_sym_rot_angle();
   void find_extremal_uv_values();

// Point searching member functions:

   std::pair<double,double> extremal_screenspace_Z_values();
   bool find_Z_given_XY(double x,double y,double& z);

// Orthorectification member functions:

   void orthorectify();

// Ladar image member functions:

   void find_extremal_z_values(
      double xlo,double xhi,double ylo,double yhi,
      double& zmin,double &zmax);
   void retrieve_hires_XYZPs();
   void retrieve_hires_XYZs_in_bbox();
   void retrieve_hires_XYZs_in_bbox(
      double xmin,double xmax,double ymin,double ymax);
   void generate_ladarimage(double delta_x=0.3,double delta_y=0.3);
   void generate_ladarimage(
      double xmin,double xmax,double ymin,double ymax,
      double delta_x,double delta_y);
   bool xypoint_to_pixel(double X,double Y,unsigned int& px,unsigned int& py);

// File output member functions:

   void write_output_file(std::string output_filename="output",
                          bool set_origin_to_zeroth_xyz=true);
   void write_XYZP_file(std::string output_filename="output",
                        std::string subdir="./XYZP/");
   void write_XYZRGBA_file(std::string output_filename="output",
                           std::string subdir="./XYZRGBA/");
   void write_TDP_file(
      std::string output_filename="output",std::string subdir="./TDP/",
      bool set_origin_to_zeroth_xyz=true);
   void write_XYZ_file(std::string output_filename="output",
                       std::string subdir="./XYZ/",
                       bool set_origin_to_zeroth_xyz=true);

// Shadowing member functions:

   double interpoint_RMS_distance(double minimal_separation);
   void generate_shadow_map(const threevector& r_hat);
   void determine_shaded_points(const threevector& r_hat);

  private:

   bool use_maps_to_color_flag;
   unsigned int n_points;
   std::vector<int> level_ID;
   std::string UTMzone;
   threevector UTMoffset;

   SetupGeomVisitor* SetupGeomVisitor_ptr;
   ColorGeodeVisitor* ColorGeodeVisitor_ptr;
   HiresDataVisitor* HiresDataVisitor_ptr;
   ColormapPtrs* ColormapPtrs_ptr;
   Grid* Grid_ptr;

   double min_threshold_frac,max_threshold_frac;
   std::vector<double> min_threshold,max_threshold;
   std::vector<bool>* shadows_ptr;
   std::vector<threevector>* normals_ptr;
   osg::ref_ptr<osg::Vec3Array> vertices; // holds XYZs for every cloud point
   osg::ref_ptr<model::Metadata> metadata;
   osg::ref_ptr<osg::Vec4ubArray> colors;
   osg::ref_ptr<osg::UIntArray> indices_refptr;

   double min_p,max_p;
   ladarimage* ladarimage_ptr;
   TrianglesGroup* TrianglesGroup_ptr;

// Symmetry UV dirs are rotated by angle theta relative to initial XY dirs:

   double theta_xy_uv; 
   double u_min,u_max,v_min,v_max;	

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const PointCloud& P);

   bool read_input_file();
   osg::Node* build_datagraph_tree( 
      int level,int parent_ID,
      const osg::Vec3Array* vertices_ptr,
      const model::Metadata* metadata_ptr,
      const osg::Vec4ubArray* colors_ptr,
      const osg::UIntArray* indices_ptr, 
      const osg::BoundingBox& box);

   void read_extrainfo_from_TDP_file();
   void read_points_from_XYZ_file();
   void read_points_from_XYZRGBA_file();
   void insert_fake_xyzp_points_for_dataviewer_coloring();

   void compute_points_thresholds();
   void compute_prob_dist();
   void reset_ColorMap_thresholds();

   void refresh_leafnodevisitor_geodes();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline unsigned int PointCloud::get_npoints() const
{
   return n_points;
}

inline void PointCloud::set_min_threshold_frac(double f)
{
   min_threshold_frac=f;
}

inline void PointCloud::set_max_threshold_frac(double f)
{
   max_threshold_frac=f;
}

inline double PointCloud::get_max_value(int i) const
{
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
            return max_p;
            break;
      }
   std::cout << "Error in PointCloud::get_max_value() !" << std::endl;
   std::cout << "Input i = " << i << " lies out of bounds" << std::endl;
   return NEGATIVEINFINITY;
}

inline double PointCloud::get_min_value(int i) const
{
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
            return min_p;
            break;
      }
   std::cout << "Error in PointCloud::get_min_value() !" << std::endl;
   std::cout << "Input i = " << i << " lies out of bounds" << std::endl;
   return POSITIVEINFINITY;
}

inline double PointCloud::get_avg_value(int i) const
{
   return 0.5*(get_max_value(i)+get_min_value(i));
}

inline double PointCloud::get_theta_xy_uv() const
{
   return theta_xy_uv;
}

inline void PointCloud::get_extremal_uv_values(
   double& umin,double& umax,double& vmin,double& vmax) const
{
   umin=u_min;
   umax=u_max;
   vmin=v_min;
   vmax=v_max;
}

inline osg::Vec3Array* PointCloud::get_vertices_ptr()
{
   return vertices.get();
}

inline model::Metadata* PointCloud::get_metadata_ptr()
{
   return metadata.get();
}

inline osg::Vec4ubArray* PointCloud::get_colors_ptr()
{
   return colors.get();
}

inline std::vector<bool>* PointCloud::get_shadows_ptr()
{
   return shadows_ptr;
}

inline std::vector<threevector>* PointCloud::get_normals_ptr()
{
   return normals_ptr;
}

// ---------------------------------------------------------------------
inline ladarimage* PointCloud::get_ladarimage_ptr()
{
   return ladarimage_ptr;
}

inline const ladarimage* PointCloud::get_ladarimage_ptr() const
{
   return ladarimage_ptr;
}

// ---------------------------------------------------------------------
inline void PointCloud::set_ColorGeodeVisitor(ColorGeodeVisitor* CGV_ptr)
{
   ColorGeodeVisitor_ptr=CGV_ptr;
}

// ------------------------------------------------------------------------
inline void PointCloud::set_SetupGeomVisitor(SetupGeomVisitor* SGV_ptr)
{
   SetupGeomVisitor_ptr=SGV_ptr;
}

// ------------------------------------------------------------------------
inline void PointCloud::set_HiresDataVisitor(HiresDataVisitor* HRV_ptr)
{
   HiresDataVisitor_ptr=HRV_ptr;
}

inline HiresDataVisitor* PointCloud::get_HiresDataVisitor_ptr()
{
   return HiresDataVisitor_ptr;
}

inline const HiresDataVisitor* PointCloud::get_HiresDataVisitor_ptr() const
{
   return HiresDataVisitor_ptr;
}

// ---------------------------------------------------------------------
// Member function xypoint_to_pixel returns the discrete ladarimage
// ztwoDarray pixel coordinates corresponding to input continuous
// (X,Y) coordinates:

inline bool PointCloud::xypoint_to_pixel(
   double X,double Y,unsigned int& px,unsigned int& py)
{
   return (ladarimage_ptr->get_z2Darray_ptr()->point_to_pixel(X,Y,px,py));
}

// ---------------------------------------------------------------------
inline void PointCloud::set_Grid_ptr(Grid* G_ptr)
{
   Grid_ptr=G_ptr;
}

inline Grid* PointCloud::get_Grid_ptr() const
{
   return Grid_ptr;
}


#endif // PointCloud.h




