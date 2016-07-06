// ========================================================================
// MyNodeVisitor class member function definitions
// ========================================================================
// Last updated on 12/2/11; 12/28/11; 12/29/11; 4/6/14
// ========================================================================

#include <iostream>
#include <set>
#include <vector>
#include "math/basic_math.h"
#include "math/constants.h"
#include "astro_geo/geofuncs.h"
#include "osg/osgSceneGraph/MyNodeVisitor.h"
#include "osg/osgfuncs.h"
#include "osg/osgSceneGraph/scenegraphfuncs.h"

#include "general/outputfuncs.h"
#include "templates/mytemplates.h"

using std::cout;
using std::endl;
using std::pair;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void MyNodeVisitor::allocate_member_objects()
{
   curr_PAT_refptr=new osg::PositionAttitudeTransform();
}

void MyNodeVisitor::initialize_member_objects()
{
   top_matrix_identified_flag=false;
   ColormapPtrs_ptr=NULL;
   CommonCallbacks_ptr=NULL;
}

MyNodeVisitor::MyNodeVisitor():
   osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
{ 
   allocate_member_objects();
   initialize_member_objects();
} 

MyNodeVisitor::~MyNodeVisitor()
{
}

// ------------------------------------------------------------------------
// Member function set_XY clears out member STL vectors XYZ and
// min_sqrd_dist.  It then pushes back a single (x,y,0) entry into XYZ
// and NEGATIVEINFINITY into min_sqrd_dist.  This method is useful for
// finding Z values for individual XY points.

void MyNodeVisitor::set_XY(double x,double y)
{
//   cout << "inside MyNodeVisitor:set_XY(), x = " << x << " y = " << y << endl;
   XYZ.clear();
   XYZ.push_back(osg::Vec3(x,y,0));
   min_sqrd_dist.clear();
   min_sqrd_dist.push_back(POSITIVEINFINITY);
}

void MyNodeVisitor::set_CommonCallbacks_ptr(CommonCallbacks* CC_ptr)
{
   CommonCallbacks_ptr=CC_ptr;
}

// ========================================================================
// Traversal member functions
// ========================================================================

void MyNodeVisitor::initialize_LocalToWorld()
{ 
//   cout << "inside MyNodeVisitor::initialize_LocalToWorld()" << endl;
   LocalToWorld=LocalToWorld.identity();
//   osgfunc::print_matrix(LocalToWorld);
}

// ------------------------------------------------------------------------
void MyNodeVisitor::apply(osg::Node& currNode) 
{ 
//   cout << "Inside MyNodeVisitor::apply(Node)" << endl;
//   cout << "classname = " << currNode.className() << endl;

   osg::NodeVisitor::apply(currNode);
}

// ------------------------------------------------------------------------
void MyNodeVisitor::apply(osg::Geode& currGeode) 
{ 
//   cout << "Inside MyNodeVisitor::apply(Geode)" << endl;
//   cout << "&Geode = " << &currGeode << endl;
//   cout << "classname = " << currGeode.className() << endl;

   vector<osg::Geometry*> geometries=scenegraphfunc::
      get_geometries(&currGeode);
//   cout << "geometries.size() = " << geometries.size() << endl;

   osg::Geometry* curr_geom_ptr=scenegraphfunc::get_geometry(&currGeode);
//   cout << "curr_geom_ptr = " << curr_geom_ptr << endl;
   if (curr_geom_ptr != NULL)
   {
      geode_vertices_refptr=dynamic_cast<osg::Vec3Array*>(
         curr_geom_ptr->getVertexArray());
//      cout << "geode_vertices_ptr = " << geode_vertices_refptr.get() << endl;
//      cout << "# geode vertices = " << geode_vertices_refptr->size() << endl;

//      for (unsigned int j=0; j<10; j++)
//      {
//         osg::Vec3 curr_vertex=geode_vertices_refptr->at(j);
//         cout << "j = " << j 
//              <<  " vertex = " << curr_vertex.x() << ","
//              << curr_vertex.y() << "," << curr_vertex.z() << endl;
//      }
   }

// Set Geodes' update [cull] callbacks to linked list stored in
// commonUpdate[Cull]_refptr:

   if (CommonCallbacks_ptr != NULL)
   {
      currGeode.setUpdateCallback( 
         CommonCallbacks_ptr->get_CommonUpdateCallback_ptr());
      currGeode.setCullCallback( 
         CommonCallbacks_ptr->get_CommonCullCallback_ptr());
   }
   
   osg::NodeVisitor::apply(currGeode);
}

// ------------------------------------------------------------------------
void MyNodeVisitor::apply(osg::Group& currGroup) 
{ 
//   cout << "Inside MyNodeVisitor::apply(Group)" << endl;
//   cout << "classname = " << currGroup.className() << endl;

   osg::NodeVisitor::apply(currGroup);
}

// ------------------------------------------------------------------------
void MyNodeVisitor::apply(osg::MatrixTransform& currMT)
{
//   cout << "inside MyNodeVisitor::apply(MatrixTransform)" << endl;
//   cout << "classname = " << currMT.className() << endl;

// Store copy of LocalToWorld before subgraph below the current node is
// traversed:

   osg::Matrix currLocalToWorld(LocalToWorld);

// For scenegraph display purposes, record copy of current Matrix:

   curr_Matrix=currMT.getMatrix();

// For point cloud coloring purposes, we need to store a copy of the
// topmost MatrixTransform within an input datagraph tree if it
// exists:

   if (!top_matrix_identified_flag)
   {
      top_Matrix=curr_Matrix;
      top_matrix_identified_flag=true;
   }

// Update cumulative LocalToWorld matrix which affects all vertices
// below it within the scenegraph:

   LocalToWorld *= curr_Matrix;

//   cout << "Before traversing lower scenegraph nodes, MyNodeVisitor LocalToWorld = " << endl;
//   osgfunc::print_matrix(LocalToWorld);
   
   osg::NodeVisitor::apply(currMT);

// Reset LocalToWorld back to its original state after subgraph below
// the current node has been traversed:

   LocalToWorld=currLocalToWorld;
}

// ------------------------------------------------------------------------
void MyNodeVisitor::apply(osg::PositionAttitudeTransform& currPAT) 
{ 
//   cout << "Inside MyNodeVisitor::apply(PAT)" << endl;
//   cout << "classname = " << currPAT.className() << endl;

   curr_PAT_refptr->setPosition(currPAT.getPosition());
   curr_PAT_refptr->setAttitude(currPAT.getAttitude());
   curr_PAT_refptr->setScale(currPAT.getScale());
   curr_PAT_refptr->setPivotPoint(currPAT.getPivotPoint());

   osg::NodeVisitor::apply(currPAT);
}

// ------------------------------------------------------------------------
void MyNodeVisitor::apply(osg::CoordinateSystemNode& currCSN) 
{ 
//   cout << "Inside MyNodeVisitor::apply(CSN)" << endl;
//   cout << "classname = " << currCSN.className() << endl;

   pair<int,bool> p=geofunc::get_UTM_zone(&currCSN);
   UTM_zone=p.first;
   northern_hemisphere_flag=p.second;
//   cout << "UTM_zone = " << UTM_zone << " northern_hemisphere_flag = "
//        << northern_hemisphere_flag << endl;
   osg::NodeVisitor::apply(currCSN);
}

// ------------------------------------------------------------------------
// Member function reconstruct_total_indices computes the cumulative
// children indices for the current Node.  The level = 0 root node has
// total indices = {0}.  The level=1 nth child has total indices
// {0,n}.  The left-most level=2 child has total indices {0,0,0}.  The
// integer strands returned within member STL vector total_indices
// provide unique labels for each node in the scenegraph.

vector<int>& MyNodeVisitor::reconstruct_total_indices()
{ 
//   cout << "inside MyNodeVisitor::reconstruct_total_indices()" << endl;
   
   total_indices.clear();

   node_path.clear();
   node_path=getNodePath();
   unsigned int level=node_path.size()-1;

   for (unsigned int l=0; l<=level; l++)
   {
      int child_index=0;
      if (l >= 1)
      {
         osg::Node* parent_node_ptr=node_path.at(l-1);
         osg::Group* parent_group_ptr=parent_node_ptr->asGroup();
         child_index=parent_group_ptr->getChildIndex(node_path.at(l));
      }
      
//      cout << "l = " << l 
//           << " parent_node_ptr = " << parent_node_ptr 
//           << " child index = " << child_index << endl;
      total_indices.push_back(child_index);
   }

//   cout << "total indices = " << endl;
//   templatefunc::printVector(total_indices);
   
   return total_indices;
} 

// ------------------------------------------------------------------------
// Member function purge_traversal_history

void MyNodeVisitor::purge_traversal_history()
{ 
//   cout << "inside MyNodeVisitor::purge_traversal_history()" << endl;
   for (unsigned int i=0; i<traversal_history.size(); i++)
   {
      traversal_history[i].clear();
   }
   traversal_history.clear();
}

void MyNodeVisitor::print_traversal_history()
{ 
   cout << "inside MyNodeVisitor::print_traversal_history()" << endl;
   cout << "traversal_history.size() = " << traversal_history.size()
        << endl;
   for (unsigned int i=0; i<traversal_history.size(); i++)
   {
      vector<int> visited_node=traversal_history.at(i);
      cout << "Visited node i = " << i << endl;
      templatefunc::printVector(visited_node);
   }
}

// ========================================================================
// Callback insertion member functions
// ========================================================================

void MyNodeVisitor::addUpdateCallback(osg::NodeCallback* cb_ptr)
{
//   cout << "inside MyNodeVisitor::addUpdateCallback(), cb_ptr = " 
//        << cb_ptr << endl;
   
   if ( !cb_ptr ) return;
   if (UpdateCallbackAlreadyExists(cb_ptr)) return;

   CommonCallbacks_ptr->get_CommonUpdateCallback_ptr()->
      addNestedCallback(cb_ptr);

//   PrintCommonUpdateCallbacks(
//      CommonCallbacks_ptr->get_CommonUpdateCallback_ptr());
}

// ------------------------------------------------------------------------
bool MyNodeVisitor::UpdateCallbackAlreadyExists(osg::NodeCallback* cb_ptr)
{
//   cout << "inside MyNodeVisitor::UpdateCallbackAlreadyExists()" << endl;
   
   if (cb_ptr==CommonCallbacks_ptr->get_CommonUpdateCallback_ptr()) 
      return true;
   
   osg::NodeCallback* nested_cb_ptr=cb_ptr->getNestedCallback();
   if (nested_cb_ptr==NULL) 
   {
      return false;
   }
   else
   {
      return UpdateCallbackAlreadyExists(nested_cb_ptr);
   }
}

// ------------------------------------------------------------------------
void MyNodeVisitor::PrintCommonUpdateCallbacks(osg::NodeCallback* cb_ptr)
{
//   cout << "inside MyNodeVisitor::PrintCommonUpdateCallbacks()" << endl;

   cout << "Common update callback ptr = " << cb_ptr << endl;
   
   osg::NodeCallback* nested_cb_ptr=cb_ptr->getNestedCallback();
   if (nested_cb_ptr==NULL) 
   {
   }
   else
   {
      PrintCommonUpdateCallbacks(nested_cb_ptr);
   }
}

// ------------------------------------------------------------------------
void MyNodeVisitor::addCullCallback(osg::NodeCallback* cb_ptr)
{
//   cout << "inside MyNodeVisitor::addCullCallback()" << endl;
//   cout << "Visitor name = " << Visitor_name << endl;
//   cout << "cb_ptr = " << cb_ptr << endl;

   if ( !cb_ptr ) return;
   if (CullCallbackAlreadyExists(cb_ptr)) return;	

   CommonCallbacks_ptr->get_CommonCullCallback_ptr()->
      addNestedCallback(cb_ptr);

//   PrintCommonCullCallbacks(
//      CommonCallbacks_ptr->get_CommonCullCallback_ptr());
}

// ------------------------------------------------------------------------
bool MyNodeVisitor::CullCallbackAlreadyExists(osg::NodeCallback* cb_ptr)
{
//   cout << "inside MyNodeVisitor::CullCallbackAlreadyExists()" << endl;
//   if (!commonCullCallback_refptr.valid()) return false;
   
   if (cb_ptr==CommonCallbacks_ptr->get_CommonCullCallback_ptr()) 
      return true;
   
   osg::NodeCallback* nested_cb_ptr=cb_ptr->getNestedCallback();
   if (nested_cb_ptr==NULL) 
   {
      return false;
   }
   else
   {
      return CullCallbackAlreadyExists(nested_cb_ptr);
   }
}

// ------------------------------------------------------------------------
void MyNodeVisitor::PrintCommonCullCallbacks(osg::NodeCallback* cb_ptr)
{
//   cout << "inside MyNodeVisitor::PrintCommonCullCallbacks()" << endl;

   cout << "Common cull callback ptr = " << cb_ptr << endl;
   
   osg::NodeCallback* nested_cb_ptr=cb_ptr->getNestedCallback();
   if (nested_cb_ptr==NULL) 
   {
   }
   else
   {
      PrintCommonCullCallbacks(nested_cb_ptr);
   }
}

// ------------------------------------------------------------------------
// Member function find_Zs_given_XYs loops over all entries in member
// STL vector XYZ.  For each STL vector entry, it scans over the
// current geode's XYZ points and finds the one closest in XY.  It
// then saves the closest point's Z value into the XYZ STL vector.  We
// wrote this method in July 2006 for satcom hummer height
// determination purposes.

bool MyNodeVisitor::find_Zs_given_XYs(
   const osg::Geode& geode,osg::Vec3Array const *curr_vertices_ptr,
   double close_enough_d2)
{
//   cout << "inside MyNodeVisitor::find_Zs_given_XYs()" << endl;
   
   bool local_height_found_flag=false;
   
//   const double close_enough_d2=sqr(1.0);     // meters**2

   osg::BoundingSphere sphere=geode.getBound();
   osg::Vec3 sphere_center(sphere.center());
   double sqrd_sphere_radius=sqr(sphere.radius());

//   int n_XYpoints_in_currgeode=0;
   for (unsigned int j=0; j<XYZ.size(); j++)
   {

// First inverse transform current XYZ from World to Local space.
// Then check whether it lies within current geode's XY bounding
// circle:

      osg::Vec3 curr_XYZ(XYZ.at(j) * WorldToLocal);
      double sqrd_dist=sqr(sphere_center.x()-curr_XYZ.x())+
         sqr(sphere_center.y()-curr_XYZ.y());
      if (sqrd_dist < sqrd_sphere_radius)
      {

// Next loop over all vertices within geode, and find vertex closest
// to inverse transformed curr_XYZ.  Save this closest vertex's Z
// value within XYZ[j].  Store minimal squared distance between this
// closest vertex within auxilliary STL vector min_sqrd_dist:

         for (unsigned int k=0; k<curr_vertices_ptr->size(); k++)
         {
            double d2=sqr(curr_vertices_ptr->at(k).x()-curr_XYZ.x())+
               sqr(curr_vertices_ptr->at(k).y()-curr_XYZ.y());
            if (d2 < min_sqrd_dist[j])
            {
               min_sqrd_dist[j]=d2;
               osg::Vec3 closest_XYZ(curr_vertices_ptr->at(k)*
                                     LocalToWorld);
               XYZ[j].set(XYZ[j].x(),XYZ[j].y(),closest_XYZ.z());
//               cout << "min dist**2 = " << d2
//                    << " x = " << XYZ[j].x() 
//                    << " closest x = " << closest_XYZ.x()
//                    << " y = " << XYZ[j].y()
//                    << " closest y = " << closest_XYZ.y() << endl;
            }
            if (d2 < close_enough_d2)
            {
               local_height_found_flag=true;
               break;
            }

         } // loop over index k labeling input XY points

//         n_XYpoints_in_currgeode++;
      }
   } // loop over index j labeling input XYZ points

//      if (n_XYpoints_in_currgeode > 0)
//      {
//         cout.precision(8);
//         cout << "paged filename = " << paged_filename << endl;
//         for (unsigned int j=0; j<XYZ.size(); j++)
//         {
//            cout << " index=" << j
//                 << " X=" << XYZ[j].x()
//                 << " Y=" << XYZ[j].y()
//                 << " Z=" << XYZ[j].z()
//                 << " min dist=" << sqrt(min_sqrd_dist[j]) << endl;
         //       }
         
//         cout << "r=" << sphere.radius() 
//              << " X=" << sphere_center.x()
//              << " Y=" << sphere_center.y() 
//              << " Z=" << sphere_center.z()
//              << " npnts=" << npoints_in_currgeode 
//              << " XYZ pnts=" << XYZ.size() << endl;
//      }

   return local_height_found_flag;
}


ColormapPtrs* MyNodeVisitor::get_ColormapPtrs_ptr()
{
//   cout << "inside MyNodeVisitor::get_ColormapPtrs_ptr()" << endl;
//   cout << "ColormapPtrs_ptr = " << ColormapPtrs_ptr << endl;
   return ColormapPtrs_ptr;
}
