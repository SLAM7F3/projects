// ==========================================================================
// Header file for pure virtual DATAGRAPHSGROUP class
// ==========================================================================
// Last modified 11/27/11; 12/2/11; 12/18/11
// ==========================================================================

#ifndef DATAGRAPHSGROUP_H
#define DATAGRAPHSGROUP_H

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <osg/BoundingBox>
#include <osg/Switch>
#include "osg/osgSceneGraph/CommonCallbacks.h"
#include "osg/osgSceneGraph/DataGraph.h"
#include "osg/osgGraphicals/GraphicalsGroup.h"
#include "eyeglass/model/HyperBoundingBox.h"
#include "osg/osgSceneGraph/LeafNodeVisitor.h"
#include "osg/osgSceneGraph/MyHyperFilter.h"
#include "osg/osgSceneGraph/SetupGeomVisitor.h"
#include "osg/osgSceneGraph/TreeVisitor.h"


class AnimationController;

class DataGraphsGroup : public GraphicalsGroup
{

  public:

// Initialization, constructor and destructor functions:

   DataGraphsGroup(const int p_ndims,Pass* PI_ptr,threevector* GO_ptr=NULL);
   DataGraphsGroup(const int p_ndims,Pass* PI_ptr,AnimationController* AC_ptr,
                   threevector* GO_ptr=NULL);
   virtual ~DataGraphsGroup();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const DataGraphsGroup& D);

// Set & get methods:

   DataGraph* get_DataGraph_ptr(int n) const;
   DataGraph* get_ID_labeled_DataGraph_ptr(int ID) const;
   LeafNodeVisitor* get_LeafNodeVisitor_ptr() const;
   SetupGeomVisitor* get_SetupGeomVisitor_ptr() const;
   TreeVisitor* get_TreeVisitor_ptr() const;
   const osg::BoundingBox& get_xyz_bbox() const;
   CommonCallbacks* get_CommonCallbacks_ptr();
   const CommonCallbacks* get_CommonCallbacks_ptr() const;
   osg::Switch* get_Switch_ptr();
   const osg::Switch* get_Switch_ptr() const;
   model::HyperBoundingBox& get_hyper_bbox();
   const model::HyperBoundingBox& get_hyper_bbox() const;

// osg::Geometry member functions:

   std::vector<std::pair<osg::Geometry*,osg::Matrix> > 
      geometries_along_ray(
         const threevector& ray_basepoint,const threevector& ray_ehat,
         double max_sphere_to_ray_frac_dist=1.0);
//   std::vector<std::pair<osg::Geometry*,osg::Matrix> > 
//      geometries_along_ray(const linesegment& LOS_ray,
//                           double max_sphere_to_ray_frac_dist=1.0);

// Output member functions:

   void write_IVE_file(std::string output_filename="output",
                       std::string subdir="./IVE/",int OSGsubPAT_ID=0);

// 4D data animation member functions:

   void AddSwitchChild(int frame_number,osg::Node* child_ptr);
   void add_Data_Nodes_to_Switch();
   void SetSwitchChildOn(int frame_number);
   void update_Switch();

   void compute_total_xyz_and_hyper_bboxes();

  protected:

   osg::BoundingBox xyz_bbox;
   model::HyperBoundingBox hyper_bbox;
   CommonCallbacks* CommonCallbacks_ptr;
   osg::ref_ptr<model::MyHyperFilter> MyHyperFilter_refptr;
   osg::ref_ptr<SetupGeomVisitor> SetupGeomVisitor_refptr;

// Introduce an OSG Switch for animating 3D objects over time:

   osg::ref_ptr<osg::Switch> Switch_refptr;

  private:

   osg::ref_ptr<LeafNodeVisitor> LeafNodeVisitor_refptr;
   osg::ref_ptr<TreeVisitor> TreeVisitor_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const DataGraphsGroup& f);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline DataGraph* DataGraphsGroup::get_DataGraph_ptr(int n) const
{
   return dynamic_cast<DataGraph*>(get_Graphical_ptr(n));
}

// --------------------------------------------------------------------------
inline DataGraph* DataGraphsGroup::get_ID_labeled_DataGraph_ptr(int ID) 
   const
{
   return dynamic_cast<DataGraph*>(get_ID_labeled_Graphical_ptr(ID));
}

// --------------------------------------------------------------------------
inline LeafNodeVisitor* DataGraphsGroup::get_LeafNodeVisitor_ptr() const
{
   return LeafNodeVisitor_refptr.get();
}

inline SetupGeomVisitor* DataGraphsGroup::get_SetupGeomVisitor_ptr() const
{
   return SetupGeomVisitor_refptr.get();
}

inline TreeVisitor* DataGraphsGroup::get_TreeVisitor_ptr() const
{
   return TreeVisitor_refptr.get();
}

// ------------------------------------------------------------------------
inline const osg::BoundingBox& DataGraphsGroup::get_xyz_bbox() const
{
   return xyz_bbox;
}

// ------------------------------------------------------------------------
inline CommonCallbacks* DataGraphsGroup::get_CommonCallbacks_ptr()
{
   return CommonCallbacks_ptr;
}

inline const CommonCallbacks* DataGraphsGroup::get_CommonCallbacks_ptr() const
{
   return CommonCallbacks_ptr;
}

// ------------------------------------------------------------------------
inline osg::Switch* DataGraphsGroup::get_Switch_ptr()
{
   return Switch_refptr.get();
}

inline const osg::Switch* DataGraphsGroup::get_Switch_ptr() const
{
   return Switch_refptr.get();
}

// ------------------------------------------------------------------------
inline model::HyperBoundingBox& DataGraphsGroup::get_hyper_bbox()
{
   return hyper_bbox;
}

inline const model::HyperBoundingBox& DataGraphsGroup::get_hyper_bbox() const
{
   return hyper_bbox;
}



#endif // DataGraphsGroup.h



