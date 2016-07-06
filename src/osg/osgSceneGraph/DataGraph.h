// ==========================================================================
// Header file for DATAGRAPH class
// ==========================================================================
// Last modified on 2/25/08; 11/27/11; 1/29/13; 4/6/14
// ==========================================================================

#ifndef DATAGRAPH_H
#define DATAGRAPH_H

#include <osg/Array>
#include <osg/BoundingBox>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/LOD>
#include <osg/Node>
#include <vector>
#include "model/HyperBoundingBox.h"
#include "model/HyperExtentsVisitor.h"
#include "osg/osgGraphicals/Graphical.h"
#include "osg/osgSceneGraph/LeafNodeVisitor.h"
#include "passes/Pass.h"
#include "math/threevector.h"

class ColorMap;
class linesegment;
class TreeVisitor;

class DataGraph : public Graphical
{

  public:

// Initialization, constructor and destructor functions:

   DataGraph(int ndim,int ID,LeafNodeVisitor* LNV_ptr,TreeVisitor* TV_ptr);
   DataGraph(int ndim,int ID,Pass* p_ptr,LeafNodeVisitor* LNV_ptr,
             TreeVisitor* TV_ptr);
   virtual ~DataGraph();

// Set & get member functions:

   void set_pass_ptr(Pass* p_ptr);
   Pass* get_pass_ptr();
   const Pass* get_pass_ptr() const;
   ColorMap* get_z_ColorMap_ptr();
   const ColorMap* get_z_ColorMap_ptr() const;
   ColorMap* get_p_ColorMap_ptr();
   const ColorMap* get_p_ColorMap_ptr() const;
   const osg::BoundingBox& get_xyz_bbox() const;
   osg::BoundingBox* get_xyz_bbox_ptr();
   const osg::BoundingBox* get_xyz_bbox_ptr() const;
   const model::HyperBoundingBox& get_hyper_bbox() const;
   model::HyperBoundingBox* get_hyper_bbox_ptr();
   const model::HyperBoundingBox* get_hyper_bbox_ptr() const;

   void set_store_indices_flag(bool indices_flag);
   bool get_store_indices_flag() const;
   void set_DataNode_ptr(osg::Node* DN_ptr);
   osg::Node* get_DataNode_ptr();
   const osg::Node* get_DataNode_ptr() const;
   unsigned int get_ntotal_leaf_vertices() const;	// only leaf geodes
   unsigned int get_n_geodes() const;
   unsigned int get_n_leaf_geodes() const;
   osg::Matrix& get_top_Matrix();
   const osg::Matrix& get_top_Matrix() const;

// Input member functions:   

   osg::Node* ReadGraph();
   osg::Node* ReadGraph(std::string input_data_filename);
   osg::Matrix& find_and_store_top_Matrix();

// Data graph generation member functions:

   void compute_xyz_and_hyper_bboxes();

// Geometry, vertex and color value retrieval member functions:

   osg::Geometry* get_curr_geometry();
   osg::Geometry* get_next_geometry();
   osg::Geometry* get_geometry(osg::Geode* Geode_ptr);

   int get_next_leaf_vertex(double& X,double& Y,double& Z);
   threevector get_next_leaf_vertex();

   int get_next_color(osg::Vec4ub& next_color);
   osg::Vec4ub get_next_color();
   osg::Vec4ub get_curr_color();
   int get_curr_color(osg::Vec4ub& curr_color);
   void set_curr_color(const osg::Vec4ub& curr_color);

// OSG::Geometry member functions:

   std::vector<std::pair<osg::Geometry*,osg::Matrix> > 
      geometries_along_ray(
         const threevector& ray_basepoint,const threevector& ray_ehat,
         double max_sphere_to_ray_frac_dist=1.0);

// File output member functions:

   void write_IVE_file(std::string output_filename="output",
                       std::string subdir="./IVE/");

  protected:

   std::string data_filename;
   bool indices_stored_flag;
   int curr_geometry_vertex_counter;
   Pass* pass_ptr;
   ColorMap *z_ColorMap_ptr,*p_ColorMap_ptr;
   osg::BoundingBox xyz_bbox;
   model::HyperBoundingBox hyper_bbox;
   osg::Matrix curr_LeafMatrix,top_Matrix;

   osg::ref_ptr<osg::Node> DataNode_refptr;
   osg::ref_ptr<LeafNodeVisitor> LeafNodeVisitor_refptr;
   osg::ref_ptr<TreeVisitor> TreeVisitor_refptr;
   osg::ref_ptr<osg::Geometry> curr_leaf_geometry_refptr;

   std::pair<osg::Geometry*,osg::Matrix> get_leaf_geometry(int leaf);
   int retrieve_curr_vertex_index();

  private:

   unsigned int geode_counter,leaf_counter;
   model::HyperExtentsVisitor* HyperExtentsVisitor_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const DataGraph& D);

   void increment_leaf_data_counters();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void DataGraph::set_pass_ptr(Pass* p_ptr)
{
   pass_ptr=p_ptr;
}

inline Pass* DataGraph::get_pass_ptr()
{
   return pass_ptr;
}

inline const Pass* DataGraph::get_pass_ptr() const
{
   return pass_ptr;
}

inline void DataGraph::set_store_indices_flag(bool indices_flag)
{
   indices_stored_flag=indices_flag;
}

inline bool DataGraph::get_store_indices_flag() const
{
   return indices_stored_flag;
}

inline void DataGraph::set_DataNode_ptr(osg::Node* DN_ptr)
{
   DataNode_refptr=DN_ptr;
}

inline osg::Node* DataGraph::get_DataNode_ptr()
{
   return DataNode_refptr.get();
}

inline const osg::Node* DataGraph::get_DataNode_ptr() const
{
   return DataNode_refptr.get();
}

inline unsigned int DataGraph::get_ntotal_leaf_vertices() const
{
   return LeafNodeVisitor_refptr->get_ntotal_leaf_vertices(
      indices_stored_flag);
}

inline unsigned int DataGraph::get_n_geodes() const
{
   return LeafNodeVisitor_refptr->get_Geodes_ptr()->size();
}

inline unsigned int DataGraph::get_n_leaf_geodes() const
{
   return LeafNodeVisitor_refptr->get_n_leaf_geodes();
}

// ------------------------------------------------------------------------
inline const osg::BoundingBox& DataGraph::get_xyz_bbox() const
{
   return xyz_bbox;
}

inline osg::BoundingBox* DataGraph::get_xyz_bbox_ptr() 
{
   return &xyz_bbox;
}

inline const osg::BoundingBox* DataGraph::get_xyz_bbox_ptr() const
{
   return &xyz_bbox;
}

inline const model::HyperBoundingBox& DataGraph::get_hyper_bbox() const
{
   return hyper_bbox;
}

inline model::HyperBoundingBox* DataGraph::get_hyper_bbox_ptr()
{
   return &hyper_bbox;
}

inline const model::HyperBoundingBox* DataGraph::get_hyper_bbox_ptr() const
{
   return &hyper_bbox;
}

// ------------------------------------------------------------------------
inline ColorMap* DataGraph::get_z_ColorMap_ptr()
{
   return z_ColorMap_ptr;
}

inline const ColorMap* DataGraph::get_z_ColorMap_ptr() const
{
   return z_ColorMap_ptr;
}

inline ColorMap* DataGraph::get_p_ColorMap_ptr()
{
   return p_ColorMap_ptr;
}

inline const ColorMap* DataGraph::get_p_ColorMap_ptr() const
{
   return p_ColorMap_ptr;
}

// ------------------------------------------------------------------------
inline osg::Matrix& DataGraph::get_top_Matrix()
{
   return top_Matrix;
}

inline const osg::Matrix& DataGraph::get_top_Matrix() const
{
   return top_Matrix;
}

#endif // DataGraph.h



