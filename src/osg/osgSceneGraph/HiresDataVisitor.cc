// ==========================================================================
// HIRESDATAVISITOR class member function definitions
// ==========================================================================
// Last modified on 11/15/09; 11/12/10; 5/16/11
// ==========================================================================

#include <iostream>
#include <string>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LOD>
#include <osg/PagedLOD>
#include <osg/Transform>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>
#include <osgDB/DatabasePager>
#include <osgDB/ReadFile>

#include <model/Metadata.h>
#include "math/basic_math.h"
#include "math/constants.h"
#include "osg/osgSceneGraph/HiresDataVisitor.h"
#include "general/outputfuncs.h"
#include "osg/osgSceneGraph/scenegraphfuncs.h"
#include "math/threevector.h"

using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void HiresDataVisitor::allocate_member_objects()
{
   bbox_ptr=new bounding_box();
   probs_refptr=new osg::FloatArray;
}

void HiresDataVisitor::initialize_member_objects()
{
   local_height_found_flag=false;
   application_index=tdp_export;
   n_hires_vertices=0;
   _pager=NULL;
   XYZ.clear();
//   P.clear();
   min_sqrd_dist.clear();
}

HiresDataVisitor::HiresDataVisitor()
{
   allocate_member_objects();
   initialize_member_objects();
}

HiresDataVisitor::HiresDataVisitor(Application_Type app)
{
   allocate_member_objects();
   initialize_member_objects();
   application_index=app;
}

HiresDataVisitor::HiresDataVisitor(
   Application_Type app,osgDB::DatabasePager* DBP_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   application_index=app;
   _pager=DBP_ptr;
}

HiresDataVisitor::HiresDataVisitor(
   osgDB::DatabasePager* DBP_ptr,vector<osg::Vec3>& P)
{
   allocate_member_objects();
   initialize_member_objects();

   application_index=local_point_height;
   _pager=DBP_ptr;

   set_input_XYZs(P);
}

HiresDataVisitor::~HiresDataVisitor()
{
   delete bbox_ptr;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

void HiresDataVisitor::set_local_height_found_flag(bool flag)
{
   local_height_found_flag=flag;
}

void HiresDataVisitor::set_application_type(Application_Type ap_type)
{
//   cout << "inisde HiresDataVisitor::set_application_type()" << endl;
//   cout << "ap_type = " << ap_type << endl;
   application_index=ap_type;
}

void HiresDataVisitor::set_input_XYZs(vector<osg::Vec3>& P)
{
   for (unsigned int i=0; i<P.size(); i++)
   {
      XYZ.push_back(P.at(i));
      min_sqrd_dist.push_back(POSITIVEINFINITY);
   }
}

void HiresDataVisitor::set_input_XYZs(vector<threevector>& P)
{
   for (unsigned int i=0; i<P.size(); i++)
   {
      XYZ.push_back( osg::Vec3( P[i].get(0) , P[i].get(1) , P[i].get(2) ) );
      min_sqrd_dist.push_back(POSITIVEINFINITY);
   }
}

// ==========================================================================
// Visitor apply member functions
// ==========================================================================

void HiresDataVisitor::apply(osg::Geode& geode)
{
//   cout << "inside HDV::apply(geode), paged_filename = " 
//        << paged_filename << endl;

   if (application_index==local_point_height &&
       local_height_found_flag) return;

/*
// Call geode's update and cull callbacks to ensure it is up to date:

   if ( osg::NodeCallback* nc = geode.getUpdateCallback() )
      (*nc)(&geode, this);	// call the node's update callback

   if ( osg::NodeCallback* nc = geode.getCullCallback() )
      (*nc)(&geode, this);	// call the node's cull callback
*/

   osg::Geometry* geometry_ptr=scenegraphfunc::get_geometry(&geode);

   if ( geometry_ptr != NULL && geometry_ptr->getVertexArray()->getType() == 
        osg::Array::Vec3ArrayType )
   {
      osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
         geometry_ptr->getVertexArray());
      model::Metadata* metadata=model::getMetadataForGeometry(*geometry_ptr);

//      osg::NodePath curr_node_path=getNodePath();
//      cout << "top node in path = " << curr_node_path.front() << endl;
      
      LocalToWorld = osg::computeLocalToWorld( getNodePath() ); 
      WorldToLocal = LocalToWorld.inverse(LocalToWorld);

      if (application_index==vertex_count)
      {
         n_hires_vertices += curr_vertices_ptr->size();
      }
      else if (application_index==tdp_export)
      {
         scenegraphfunc::write_geometry_relative_xyzrgba(
            *zeroth_xyz_ptr,*tdp_file_ptr,
            *xyz_byte_counter_ptr,*color_byte_counter_ptr,
            get_ColormapPtrs_ptr(),geometry_ptr,LocalToWorld);
      }
      else if (application_index==local_point_height)
      {
//         const double close_enough_d2=sqr(0.3);	// meter**2
         const double close_enough_d2=sqr(1.0);	// meter**2
         find_Zs_given_XYs(geode,curr_vertices_ptr,close_enough_d2);
      }
      else if (application_index==retrieve_XYZs)
      {
         for (unsigned int i=0; i<curr_vertices_ptr->size(); i++)
         {
            XYZ.push_back(curr_vertices_ptr->at(i)*LocalToWorld);
         }
      }
      else if (application_index==retrieve_XYZs_inside_bbox)
      {
         osg::BoundingSphere sphere=geode.getBound();
         osg::Vec4 sphere_fourcenter(sphere.center(),1);
         threevector sphere_center(sphere_fourcenter*LocalToWorld);
         double radius=sphere._radius;
         double geode_min_x=sphere_center.get(0)-radius;
         double geode_max_x=sphere_center.get(0)+radius;
         double geode_min_y=sphere_center.get(1)-radius;
         double geode_max_y=sphere_center.get(1)+radius;

// First check whether any corner of XY bounding box surrounding
// current geode lies inside *bbox_ptr.  If not, none of geode's
// vertices can possibly lie inside *bbox_ptr.  Don't waste lots of
// time testing individual vertices if this first coarse bounding box
// test fails:
         
         if (bbox_ptr->point_inside(geode_min_x,geode_min_y) ||
             bbox_ptr->point_inside(geode_min_x,geode_max_y) ||
             bbox_ptr->point_inside(geode_max_x,geode_min_y) ||
             bbox_ptr->point_inside(geode_max_x,geode_max_y))
         {
            for (unsigned int i=0; i<curr_vertices_ptr->size(); i++)
            {
               osg::Vec3 transformed_vertex(curr_vertices_ptr->at(i)*
                                            LocalToWorld);
               if (bbox_ptr->point_inside(
                      transformed_vertex.x(),transformed_vertex.y()))
               {
                  XYZ.push_back(transformed_vertex);
               }
            } // loop over index i labeling vertices
         } // coarse bbox check conditional
      }
      else if (application_index==retrieve_XYZPs)
      {
         for (unsigned int i=0; i<curr_vertices_ptr->size(); i++)
         {
            XYZ.push_back(curr_vertices_ptr->at(i)*LocalToWorld);
            if (metadata != NULL)
            {
               probs_refptr->push_back(metadata->get(i,0));            
            }
         }
      }

   } // geometry type == Vec3 array conditional

//   MyNodeVisitor::apply(geode);
}

// ------------------------------------------------------------------------
void HiresDataVisitor::apply(osg::LOD& node)
{
//   cout << "inside HDV::apply, LOD_name = " << node.getName() << endl;

   const osg::LOD::RangeList& rangeList = node.getRangeList();
   float range = 0;

   // visit the highest resolution child
   for(unsigned int i=0; i<rangeList.size(); i++)
      range = osg::maximum( range, rangeList[i].first );

   unsigned int numChildren = node.getNumChildren();
   if ( rangeList.size() < numChildren) numChildren = rangeList.size();

   for(unsigned int i=0; i<numChildren; i++)
   {    
      if ( rangeList[i].first<=range && range<rangeList[i].second)
         node.getChild(i)->accept(*this);
   }
}

// ------------------------------------------------------------------------
void HiresDataVisitor::apply(osg::PagedLOD& node)
{
//   cout << "inside HiresDataVisitor::apply(PagedLOD)" << endl;

   const osg::PagedLOD::RangeList& rangeList = node.getRangeList();
   float range = 0;

// if(node.getmode()?==pixel_size_on_screen) call next block of code


   // visit the highest resolution child
   for (unsigned int i=0; i<rangeList.size(); i++)
      range = osg::maximum( range, rangeList[i].first );

//else   
//   {
      
//   for(unsigned int i=0; i<rangeList.size(); i++)
//      range = osg::minimum( range, rangeList[i].first );
//   }
   
   unsigned int numRanges = rangeList.size(); 
   // = total number of pages in and out of memory

   unsigned int numChildren = node.getNumChildren();
   // = number of pages currently residing in memory

   for (unsigned int i=0; i<numRanges; i++)
   {    
      if ( rangeList[i].first<=range && range<rangeList[i].second) 
      {
         osg::ref_ptr<osg::Node> lodChild = NULL;
			
         if ( i < numChildren ) 
         {

            if (application_index==reset_pageLOD_child_filenames)
            {
               blank_PagedLOD_child_filename(node,i);
            }

// Page already in memory.  So just visit it:

            lodChild = node.getChild(i);
            lodChild->accept(*this);
         } 
         else 
         {

// Page not already in memory.  Need to load it into scene graph,
// visit it and then unload it from scene graph:

            paged_filename = node.getDatabasePath() + node.getFileName(i);
            lodChild = osgDB::readNodeFile(paged_filename);
				
// Apply standard pager treatment, as if we loaded it in the viewer:

            if ( lodChild.valid() && _pager.valid() ) 
            {
               _pager->registerPagedLODs( lodChild.get() );
//               cout << "Paged in filename = " << paged_filename << endl;

// On 11/15/09, we attempted to implement Ross Anderson's idea for
// exporting OSG files containing all point cloud, photo, etc data
// explicitly within the ascii output file and not references to
// filenames on a data disk.  Ross suggests reseting the paged
// filename to blank.

               if (application_index==reset_pageLOD_child_filenames)
               {
                  blank_PagedLOD_child_filename(node,i);
               }
            }

            node.addChild(lodChild.get());
            lodChild->accept(*this);
//            node.removeChild(lodChild.get());

         }
      }
   } // loop over index i labeling ranges
}

// ------------------------------------------------------------------------
// Member function blank_PagedLOD_child_filename() implements Ross
// Anderson's idea for exporting OSG files containing all point cloud,
// photo, etc data explicitly within the ascii output file and not
// references to filenames on a data disk.  Ross suggests resetting the
// paged filename to blank.

void HiresDataVisitor::blank_PagedLOD_child_filename(
   osg::PagedLOD& node,int i)
{
//   cout << "inside HiresDataVisitor::apply(PagedLOD)" << endl;

   paged_filename = node.getDatabasePath() + node.getFileName(i);
//   cout << "Original filename = " << paged_filename << endl;
   node.setFileName(i,"");
   paged_filename = node.getDatabasePath() + node.getFileName(i);
//   cout << "i = " << i << " node.getFileName(i).empty() = "
//        << node.getFileName(i).empty() << endl;
}

