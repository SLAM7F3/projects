// ==========================================================================
// DATAGRAPH class member function definitions
// ==========================================================================
// Last modified on 4/15/07; 4/22/07; 12/21/07; 2/25/08; 11/27/11
// ==========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/observer_ptr>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include "math/adv_mathfuncs.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "osg/osgSceneGraph/DataGraph.h"
#include "general/filefuncs.h"
#include "geometry/geometry_funcs.h"
#include "geometry/linesegment.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgSceneGraph/MyNodeInfo.h"
#include "osg/osgSceneGraph/MyNodeVisitor.h"
#include "general/outputfuncs.h"
#include "osg/osgSceneGraph/scenegraphfuncs.h"
#include "general/stringfuncs.h"
#include "osg/osgSceneGraph/TreeVisitor.h"

#include "osg/osgfuncs.h"
#include "templates/mytemplates.h"

using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void DataGraph::allocate_member_objects()
{
//   cout << "inside DataGraph::allocate_member_objects()" << endl;
   z_ColorMap_ptr=new ColorMap();
   p_ColorMap_ptr=new ColorMap();
   HyperExtentsVisitor_ptr=new model::HyperExtentsVisitor();
}		       

void DataGraph::initialize_member_objects()
{
   Graphical_name="DataGraph";
   pass_ptr=NULL;
   indices_stored_flag=false;
   geode_counter=leaf_counter=0;
   curr_geometry_vertex_counter=-1;
}		       

DataGraph::DataGraph(
   int ndim,int ID,LeafNodeVisitor* LNV_ptr,TreeVisitor* TV_ptr):
   Graphical(ndim,ID)
{	
   allocate_member_objects();
   initialize_member_objects();
   LeafNodeVisitor_refptr=LNV_ptr;
   TreeVisitor_refptr=TV_ptr;
}		       

DataGraph::DataGraph(
   int ndim,int ID,Pass* p_ptr,LeafNodeVisitor* LNV_ptr,TreeVisitor* TV_ptr):
   Graphical(ndim,ID)
{	
   allocate_member_objects();
   initialize_member_objects();
   pass_ptr=p_ptr;
   LeafNodeVisitor_refptr=LNV_ptr;
   TreeVisitor_refptr=TV_ptr;
}		       

DataGraph::~DataGraph()
{	
   delete z_ColorMap_ptr;
   delete p_ColorMap_ptr;
   delete HyperExtentsVisitor_ptr;
}

// ==========================================================================
// Input member functions:   
// ==========================================================================

// This first version of member function ReadGraph extracts the IVE or
// OSGA filenames from member STL vector filenames of *pass_ptr:

osg::Node* DataGraph::ReadGraph()
{
//   cout << "inside DataGraph::ReadGraph()" << endl;
//   cout << "pass_ptr->get_input_filetype() = "
//        << pass_ptr->get_input_filetype() << endl;
   
   if (pass_ptr->get_input_filetype()==Pass::ive ||
       pass_ptr->get_input_filetype()==Pass::osga ||
       pass_ptr->get_input_filetype()==Pass::osg)
   {
      vector<string> input_filenames=pass_ptr->get_filenames();
      osg::Node* data_root_ptr = osgDB::readNodeFiles(input_filenames);
      set_DataNode_ptr(data_root_ptr);
      find_and_store_top_Matrix();
   }
   TreeVisitor_refptr->set_DataNode_ptr(get_DataNode_ptr());

// Use TreeVisitor to search for UTM information stored within
// Coordinate System Node at the top of datagraph.  Store retrieved
// UTM zone information within current Pass object:

   if (DataNode_refptr.valid())
   {
      get_DataNode_ptr()->accept(* (TreeVisitor_refptr.get()) );
      pass_ptr->set_UTM_zonenumber(TreeVisitor_refptr->get_UTM_zone());

// FAKE FAKE:  Fri Feb 22, 2013 at 1:57 pm
// Hardwire UTM zone for Florida panhandle:

//      pass_ptr->set_UTM_zonenumber(16);
      

      pass_ptr->set_northern_hemisphere_flag(
         TreeVisitor_refptr->get_northern_hemisphere_flag());
//      cout << "UTM zonenumber = " << pass_ptr->get_UTM_zonenumber()
//           << " northern_hemisphere_flag = " 
//           << pass_ptr->get_northern_hemisphere_flag() << endl;
//      cout << "pass_ptr = " << pass_ptr << endl;
   }

   return get_DataNode_ptr();
}

// ---------------------------------------------------------------------
// PlanetsGroup::generate_EarthGraph() calls this next method:

osg::Node* DataGraph::ReadGraph(string input_data_filename)
{
   if (pass_ptr->get_input_filetype()==Pass::ive ||
       pass_ptr->get_input_filetype()==Pass::osga ||
       pass_ptr->get_input_filetype()==Pass::osg)
   {
      osg::Node* data_root_ptr = osgDB::readNodeFile(input_data_filename);
      set_DataNode_ptr(data_root_ptr);
      find_and_store_top_Matrix();
   }
   if (!DataNode_refptr.valid())
   {
      cout << "Error in DataGraph::ReadGraph()" << endl;
      cout << "input_data_filename = " << input_data_filename << endl;
      cout << "DataNode_ptr = NULL !!!" << endl;
   }
   TreeVisitor_refptr->set_DataNode_ptr(get_DataNode_ptr());
   return get_DataNode_ptr();
}

// ---------------------------------------------------------------------
// Member function find_and_store_top_Matrix instantiates a
// MyNodeVisitor whose sole purpose is to crawl through an input
// datagraph and locate the topmost MatrixTransform.  We need save a
// copy of this top Matrix for point cloud coloring purposes.

osg::Matrix& DataGraph::find_and_store_top_Matrix()
{
//   cout << "inside DataGraph::find_and_store_top_Matrix()" << endl;
   MyNodeVisitor* MatrixVisitor_ptr=new MyNodeVisitor();
   get_DataNode_ptr()->accept(*MatrixVisitor_ptr);
   top_Matrix=MatrixVisitor_ptr->get_top_Matrix();

//   cout << "top matrix = " << endl;
//   osgfunc::print_matrix(top_Matrix);
   
   return top_Matrix;
}

// ==========================================================================
// Data graph generation member functions
// ==========================================================================

void DataGraph::compute_xyz_and_hyper_bboxes()
{ 
   HyperExtentsVisitor_ptr->reset();
   get_DataNode_ptr()->accept(*HyperExtentsVisitor_ptr);
   xyz_bbox=HyperExtentsVisitor_ptr->getBound();
   hyper_bbox=HyperExtentsVisitor_ptr->getExtent();

   string banner="DataGraph bounding box:";
   outputfunc::write_banner(banner);
   
   cout.precision(12);
   cout << "xmin = " << xyz_bbox.xMin() 
        << " xmax = " << xyz_bbox.xMax() << endl;
   cout << "ymin = " << xyz_bbox.yMin() 
        << " ymax = " << xyz_bbox.yMax() << endl;
   cout << "zmin = " << xyz_bbox.zMin() 
        << " zmax = " << xyz_bbox.zMax() << endl;
}

// ==========================================================================
// Geometry, vertex, color and p-value retrieval member functions:
// ==========================================================================

// Member functions get_curr_geometry and get_next_geometry use and
// update private member variable geode_counter as a geode iterator.
// The following methods could someday be replaced by a Geode Visitor
// implementation if necessary...

osg::Geometry* DataGraph::get_curr_geometry()
{ 
   if (geode_counter >= get_n_geodes())
   {
      geode_counter=0;
      return NULL;
   }
   else
   {
      pair<osg::observer_ptr<osg::Geode>,osg::Matrix> p=
         LeafNodeVisitor_refptr->get_Geodes_ptr()->at(geode_counter);
      curr_LeafMatrix=p.second;
      return scenegraphfunc::get_geometry((p.first).get());
   }
}

osg::Geometry* DataGraph::get_next_geometry()
{ 
   geode_counter++;
   return get_curr_geometry();
}

// ------------------------------------------------------------------------
// Member function increment_leaf_data_counters updates private member
// variables leaf_vertex_counter and leaf_counter so that they point
// towards the next data element within the data scene graph.

void DataGraph::increment_leaf_data_counters()
{ 

// Increment private current geometry vertex counter.  If geometry
// vertex counter value exceeds vertices count for current leaf geode,
// then increment leaf counter and reset current geometry vertex
// counter to zero:

   curr_geometry_vertex_counter++;
   if (curr_geometry_vertex_counter==scenegraphfunc::get_n_geometry_vertices(
      curr_leaf_geometry_refptr.get(),indices_stored_flag))
   {
      leaf_counter++;
      if (leaf_counter==LeafNodeVisitor_refptr->get_n_leaf_geodes()) 
         leaf_counter=0;
      curr_leaf_geometry_refptr=get_leaf_geometry(leaf_counter).first;
      curr_geometry_vertex_counter=0;
   }
}

// ------------------------------------------------------------------------
// Member function retrieve_curr_vertex_index uses private member
// variables curr_geometry_vertex_counter to retrieve the index for
// the current vertex within the data scene graph.

int DataGraph::retrieve_curr_vertex_index()
{ 
   osg::PrimitiveSet* primitiveSet = NULL;
   if ( curr_leaf_geometry_refptr->getNumPrimitiveSets() == 1 ) 
   {
      primitiveSet = curr_leaf_geometry_refptr->getPrimitiveSet(0);
      osg::DrawElementsUInt* DEUI_ptr=dynamic_cast<
         osg::DrawElementsUInt*>(primitiveSet);
      return DEUI_ptr->index(curr_geometry_vertex_counter);
   } 
   else 
   {
      return -1;     // Can't filter a set with <> 1 PrimitiveSet
   }
}

// ------------------------------------------------------------------------
// Member function get_leaf_geometry first retrieves the leaf geode
// corresponding to the input index.  It then loops over all drawables
// within the geode which should correspond to a leaf within the Data
// Scene Graph.  This method assumes that the input LeafNode contains
// only one geometry with XYZ vertex information inside it.  It
// returns an std pair consistent of a pointer to that geometry along
// with its corresponding matrix transform.

pair<osg::Geometry*,osg::Matrix> DataGraph::get_leaf_geometry(int leaf)
{ 
   pair<osg::observer_ptr<osg::Geode>,osg::Matrix> p=
      LeafNodeVisitor_refptr->get_Geodes_ptr()->at(leaf);
   curr_LeafMatrix=p.second;
   return pair<osg::Geometry*,osg::Matrix>(
      scenegraphfunc::get_geometry((p.first).get()),p.second);
}

// ------------------------------------------------------------------------
// Member function get_next_leaf_vertex uses private member variables
// vertex_counter and leaf_counter to retrieve the next 3-vertex from
// the Data Scene Graph.  It updates both counters so that they are
// ready to be used in a subsequent call to this method.

int DataGraph::get_next_leaf_vertex(double& X,double& Y,double& Z)
{ 
   increment_leaf_data_counters();
   osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>
      (curr_leaf_geometry_refptr->getVertexArray());

   int curr_vertex_index=curr_geometry_vertex_counter;
   if (indices_stored_flag)
   {
      curr_vertex_index=retrieve_curr_vertex_index();
   }

   osg::Vec3 abs_vertex(vertices->at(curr_vertex_index)*curr_LeafMatrix);
   X=abs_vertex.x();
   Y=abs_vertex.y();
   Z=abs_vertex.z();
   return curr_vertex_index;
}

threevector DataGraph::get_next_leaf_vertex()
{ 
   double X,Y,Z;
   get_next_leaf_vertex(X,Y,Z);
   return threevector(X,Y,Z);
}

// ------------------------------------------------------------------------
// Member function get_next_color uses private member variables
// curr_geometry_vertex_counter and leaf_counter to retrieve the next
// RGBA color from the Data Scene Graph.  It updates both counters so
// that they are ready to be used in a subsequent call to this method.

int DataGraph::get_next_color(osg::Vec4ub& next_color)
{ 
   increment_leaf_data_counters();
   return get_curr_color(next_color);
}

osg::Vec4ub DataGraph::get_next_color()
{ 
   osg::Vec4ub next_color;
   get_next_color(next_color);
   return next_color;
}

osg::Vec4ub DataGraph::get_curr_color()
{ 
   osg::Vec4ub curr_color;
   get_curr_color(curr_color);
   return curr_color;
}

int DataGraph::get_curr_color(osg::Vec4ub& curr_color)
{ 
   osg::Vec4ubArray* colors_ptr=dynamic_cast<osg::Vec4ubArray*>(
      curr_leaf_geometry_refptr->getColorArray());

   if (indices_stored_flag)
   {
      int j=retrieve_curr_vertex_index();
      curr_color=colors_ptr->at(j);
      return j;
   }
   else
   {
      curr_color=colors_ptr->at(curr_geometry_vertex_counter);
      return curr_geometry_vertex_counter;
   }
}

// ------------------------------------------------------------------------
// Member function set_curr_color resets RGBA for the current element
// within the data scene graph to the value passed in via curr_color.

void DataGraph::set_curr_color(const osg::Vec4ub& curr_color)
{ 
   osg::Vec4ubArray* colors_ptr=dynamic_cast<osg::Vec4ubArray*>(
      curr_leaf_geometry_refptr->getColorArray());
   if (indices_stored_flag)
   {
      int j=retrieve_curr_vertex_index();
      colors_ptr->at(j)=curr_color;
   }
   else
   {
      colors_ptr->at(curr_geometry_vertex_counter)=curr_color;
   }
}

// ==========================================================================
// OSG::Geometry member functions
// ==========================================================================

// Member function geometries_along_ray loops over all geodes within
// the DataGraph.  It retrieves the bounding sphere for each geode and
// computes the distance from the sphere's center to the semi-infinite
// ray along the input line-of-sight direction.  Any geode which lies
// less than one radius away from this ray is returned within the
// output STL vector.

vector<pair<osg::Geometry*,osg::Matrix> > 
DataGraph::geometries_along_ray(
   const threevector& ray_basepoint,const threevector& ray_ehat,
   double max_sphere_to_ray_frac_dist)
{ 
//   cout << "inside DataGraph::geoms_along_ray()" << endl;

   vector<pair<osg::Geometry*,osg::Matrix> > geoms_along_ray;
   vector<double> sphere_radii,sphere_center_ranges;

// Refresh list of geodes within *LeafNodeVisitor_refptr which may have
// been updated via Database paging since last time this method was
// called:

   LeafNodeVisitor_refptr->get_Geodes_ptr()->clear();
   get_DataNode_ptr()->accept(*(LeafNodeVisitor_refptr.get()));

   threevector sphere_center;
   for (unsigned int g=0; g<get_n_geodes(); g++)
   {
      pair<osg::observer_ptr<osg::Geode>,osg::Matrix> p=
         LeafNodeVisitor_refptr->get_Geodes_ptr()->at(g);
      osg::Geode* curr_Geode_ptr=(p.first).get();

      osg::BoundingSphere sphere=curr_Geode_ptr->getBound();
      osg::Vec4 sphere_fourcenter(sphere.center(),1);
      sphere_center=threevector(sphere_fourcenter*p.second);

      double sphere_to_ray_frac_dist=
         geometry_func::point_to_line_distance(
            sphere_center , ray_basepoint , ray_ehat) / sphere._radius;

//      cout << "g = " << g
//           << " center_to_ray dist = " 
 //          << ray.point_to_line_distance(sphere_center)
 //          << " sphere r = " << sphere._radius
 //          << " sphere_to_ray_frac_dist = "
//           << sphere_to_ray_frac_dist << endl;

      if (sphere_to_ray_frac_dist <= max_sphere_to_ray_frac_dist)
      {
         geoms_along_ray.push_back(
            pair<osg::Geometry*,osg::Matrix> 
            (scenegraphfunc::get_geometry(curr_Geode_ptr) , p.second) );
         sphere_radii.push_back(sphere._radius);

//         cout << "g=" << g 
//              << " r=" << sphere.radius() 
//              << " x=" << sphere_center.get(0)
//              << " y=" << sphere_center.get(1)
//              << " z=" << sphere_center.get(2)
//              << " frac dist = " << sphere_to_ray_frac_dist 
//              << endl;
      }
   } // loop over index g labeling all geodes in DataGraph

//   const unsigned int max_returned_geoms=5;
   const unsigned int max_returned_geoms=10;
   if (geoms_along_ray.size() > max_returned_geoms)
   {
      templatefunc::Quicksort(sphere_radii,geoms_along_ray);

//      cout << "inside DataGraph::geometries_along_ray(), sphere_radii = "
//           << endl;
//      templatefunc::printVector(sphere_radii);

      vector<pair<osg::Geometry*,osg::Matrix> > reduced_geoms_along_ray;      
      for (unsigned int i=0; i<max_returned_geoms; i++)
      {
         reduced_geoms_along_ray.push_back(geoms_along_ray[i]);
      }
      return reduced_geoms_along_ray;
   }
   else
   {
      return geoms_along_ray;
   }
}

// ==========================================================================
// File output member functions
// ==========================================================================

void DataGraph::write_IVE_file(string output_filename,string subdir)
{
   cout << "inside DataGraph::write_IVE_file()" << endl;
   if (get_store_indices_flag())
   {
      cout << "Data Scene Graph stores indices rather than vertex buckets!"
           << endl;
      cout << "Change PointCloud::GenerateCloudGraph input argument index_tree_flag to false" << endl;
      cout << "before attempting to write out .ive file!" << endl;
   }
   else
   {
      outputfunc::write_banner("Writing out IVE file:");

      ofstream binary_outstream;
      filefunc::dircreate(subdir);
      output_filename=subdir+output_filename+".ive";
      filefunc::deletefile(output_filename);

      if ( osgDB::writeNodeFile( *(get_DataNode_ptr()), output_filename) )
         osg::notify(osg::NOTICE) << "Wrote .ive file: " 
                                  << output_filename << "\n";
      else
         osg::notify(osg::WARN) << "Could not write .ive file.\n";
   }
}
