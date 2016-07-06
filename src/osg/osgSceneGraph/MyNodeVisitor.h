// ========================================================================
// MyNodeVisitor header file
// ========================================================================
// Last updated on 11/27/11; 12/2/11; 12/28/11
// ========================================================================

#ifndef MYNODEVISITOR_H
#define MYNODEVISITOR_H

#include <vector>
#include <osg/CoordinateSystemNode>
#include <osg/Geode>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/NodeVisitor>
#include <osg/PositionAttitudeTransform>
#include "osg/osgSceneGraph/ColormapPtrs.h"
#include "osg/osgSceneGraph/CommonCallbacks.h"

class MyNodeVisitor : public osg::NodeVisitor 
{
  public: 

   MyNodeVisitor();
   virtual ~MyNodeVisitor(); 

// Set & get methods:

   int get_UTM_zone() const;
   bool get_northern_hemisphere_flag() const;
   void set_LocalToWorld(const osg::Matrix& L2W);
   void set_WorldToLocal(const osg::Matrix& W2L);
   void set_ColormapPtrs_ptr(ColormapPtrs* CT_ptr);
   ColormapPtrs* get_ColormapPtrs_ptr();
   void set_CommonCallbacks_ptr(CommonCallbacks* CC_ptr);

   osg::Matrix& get_top_Matrix();
   const osg::Matrix& get_top_Matrix() const;
   osg::Matrix& get_LocalToWorld();
   const osg::Matrix& get_LocalToWorld() const;
   osg::Matrix& get_currMatrix();
   const osg::Matrix& get_currMatrix() const;
   osg::PositionAttitudeTransform* get_PAT_ptr();
   const osg::PositionAttitudeTransform* get_PAT_ptr() const;

   void set_XY(double x,double y);
   int get_n_XYZs() const;
   std::vector<osg::Vec3>& get_XYZ();
   const std::vector<osg::Vec3>& get_XYZ() const;
   osg::FloatArray* get_probs_ptr();
   const osg::FloatArray* get_probs_ptr() const;

   osg::Vec3& get_XYZ(unsigned int i);
   const osg::Vec3& get_XYZ(unsigned int i) const;

   std::vector<std::vector<int> >& get_traversal_history();
   const std::vector<std::vector<int> >& get_traversal_history() const;
   
// Traversal member functions:

   void initialize_LocalToWorld();
   virtual void apply(osg::Node& currNode);
   virtual void apply(osg::Geode& currGeode);
   virtual void apply(osg::Group& currGroup);
   virtual void apply(osg::MatrixTransform& currMT);
   virtual void apply(osg::PositionAttitudeTransform& currPAT);
   virtual void apply(osg::CoordinateSystemNode& currCSN);
   std::vector<int>& reconstruct_total_indices();
   void purge_traversal_history();
   void print_traversal_history();

// Callback insertion member functions

   void addUpdateCallback(osg::NodeCallback* cb_ptr );
   bool UpdateCallbackAlreadyExists(osg::NodeCallback* cb_ptr);
   void PrintCommonUpdateCallbacks(osg::NodeCallback* cb_ptr);

   void addCullCallback(osg::NodeCallback* cb_ptr );
   bool CullCallbackAlreadyExists(osg::NodeCallback* cb_ptr);
   void PrintCommonCullCallbacks(osg::NodeCallback* cb_ptr);

   bool find_Zs_given_XYs(
      const osg::Geode& geode,osg::Vec3Array const *curr_vertices_ptr,
      double close_enough_d2);

  protected:

   bool top_matrix_identified_flag,northern_hemisphere_flag;
   int UTM_zone;
   std::string Visitor_name;

   ColormapPtrs* ColormapPtrs_ptr;
   CommonCallbacks* CommonCallbacks_ptr;

   std::vector<osg::Vec3> XYZ;
   osg::ref_ptr<osg::FloatArray> probs_refptr;

   std::vector<double> min_sqrd_dist;
   std::vector<int> total_indices;
   std::vector<std::vector<int> > traversal_history;

   osg::NodePath node_path;
   osg::Matrix curr_Matrix,top_Matrix,LocalToWorld,WorldToLocal;
   osg::ref_ptr<osg::Vec3Array> geode_vertices_refptr;
   osg::ref_ptr<osg::PositionAttitudeTransform> curr_PAT_refptr;

  private:

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline int MyNodeVisitor::get_UTM_zone() const
{
   return UTM_zone;
}

inline bool MyNodeVisitor::get_northern_hemisphere_flag() const
{
   return northern_hemisphere_flag;
}

inline int MyNodeVisitor::get_n_XYZs() const
{
   return XYZ.size();
}

inline std::vector<osg::Vec3>& MyNodeVisitor::get_XYZ()
{
   return XYZ;
}

inline const std::vector<osg::Vec3>& MyNodeVisitor::get_XYZ() const
{
   return XYZ;
}

inline osg::Vec3& MyNodeVisitor::get_XYZ(unsigned int i) 
{
   return XYZ.at(i);
}

inline const osg::Vec3& MyNodeVisitor::get_XYZ(unsigned int i) const
{
   return XYZ.at(i);
}

inline osg::FloatArray* MyNodeVisitor::get_probs_ptr()
{
   return probs_refptr.get();
}

inline const osg::FloatArray* MyNodeVisitor::get_probs_ptr() const
{
   return probs_refptr.get();
}

inline osg::Matrix& MyNodeVisitor::get_top_Matrix()
{
   return top_Matrix;
}

inline const osg::Matrix& MyNodeVisitor::get_top_Matrix() const
{
   return top_Matrix;
}

inline void MyNodeVisitor::set_LocalToWorld(const osg::Matrix& L2W)
{
   LocalToWorld=L2W;
}

inline void MyNodeVisitor::set_WorldToLocal(const osg::Matrix& W2L)
{
   WorldToLocal=W2L;
}

inline void MyNodeVisitor::set_ColormapPtrs_ptr(ColormapPtrs* CT_ptr)
{
   ColormapPtrs_ptr=CT_ptr;
}

inline osg::Matrix& MyNodeVisitor::get_LocalToWorld()
{
   return LocalToWorld;
}

inline const osg::Matrix& MyNodeVisitor::get_LocalToWorld() const
{
   return LocalToWorld;
}

inline osg::Matrix& MyNodeVisitor::get_currMatrix()
{
   return curr_Matrix;
}

inline const osg::Matrix& MyNodeVisitor::get_currMatrix() const
{
   return curr_Matrix;
}

inline osg::PositionAttitudeTransform* MyNodeVisitor::get_PAT_ptr()
{
   return curr_PAT_refptr.get();
}

inline const osg::PositionAttitudeTransform* MyNodeVisitor::get_PAT_ptr() 
   const
{
   return curr_PAT_refptr.get();
}

inline std::vector<std::vector<int> >& MyNodeVisitor::get_traversal_history()
{
   return traversal_history;
}

inline const std::vector<std::vector<int> >& 
MyNodeVisitor::get_traversal_history() const
{
   return traversal_history;
}


#endif 
