// ==========================================================================
// Header file for HIRESDATAVISITOR class which is a variant of Ross
// Anderson's ExportDataVisitor class.
// ==========================================================================
// Last modified on 11/19/09; 11/20/09; 5/16/11
// ==========================================================================

#ifndef __HIRESDATAVISITOR_H__
#define __HIRESDATAVISITOR_H__ 1

#include <set>
#include <string>
#include <vector>
#include <osgDB/DatabasePager>
#include <osg/NodeVisitor>
#include <osg/Vec3>
#include <libtdp/tdp.h>
#include "geometry/bounding_box.h"
#include "osg/osgSceneGraph/MyNodeVisitor.h"

// The HiresDataVisitor class will visit a complete LOD tree of data,
// and export the highest resolution visible data found in the leaf
// geodes.

class threevector;

class HiresDataVisitor : public MyNodeVisitor

{
  public:
		
   enum Application_Type
   {
      vertex_count,tdp_export,local_point_height,retrieve_XYZs,
      retrieve_XYZs_inside_bbox,retrieve_XYZPs,reset_pageLOD_child_filenames
   };

   HiresDataVisitor();
   HiresDataVisitor(Application_Type app);
   HiresDataVisitor(Application_Type app,osgDB::DatabasePager* DBP_ptr);
   HiresDataVisitor(osgDB::DatabasePager* DBP_ptr,std::vector<osg::Vec3>& P);
   virtual ~HiresDataVisitor();
		
// Set & get member functions:

   void set_local_height_found_flag(bool flag);
   void set_application_type(Application_Type ap_type);
   void setDatabasePager( osgDB::DatabasePager* dbp );
   osgDB::DatabasePager* getDatabasePager();
   void set_input_XYZs(std::vector<osg::Vec3>& P);
   void set_input_XYZs(std::vector<threevector>& P);

   void set_tdp_file_ptr(Tdp_file* tdp_ptr);
   void set_zeroth_xyz_ptr(threevector* xyz0_ptr);
   void set_xyz_byte_counter_ptr(int* counter_ptr);
   void set_color_byte_counter_ptr(int* counter_ptr);

   void reset_n_hires_vertices();
   int get_n_hires_vertices() const;
   bounding_box* get_bbox_ptr();
   
// Visitor apply member functions:

//   virtual void apply(osg::Node& node);
   virtual void apply(osg::Geode& node);
   virtual void apply(osg::LOD& node);
   virtual void apply(osg::PagedLOD& node);
	
  private:
		
   bool local_height_found_flag;
   Application_Type application_index;
   int n_hires_vertices;
   int *xyz_byte_counter_ptr,*color_byte_counter_ptr;
   std::string paged_filename;
   threevector* zeroth_xyz_ptr;
   bounding_box* bbox_ptr;
   Tdp_file* tdp_file_ptr;

   osg::ref_ptr<osgDB::DatabasePager> _pager;

   void allocate_member_objects();
   void initialize_member_objects();
   void blank_PagedLOD_child_filename(osg::PagedLOD& node,int i);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void HiresDataVisitor::setDatabasePager( osgDB::DatabasePager* dbp ) 
{ 
   _pager = dbp; 
}

inline osgDB::DatabasePager* HiresDataVisitor::getDatabasePager() 
{ 
   return _pager.get(); 
}

inline void HiresDataVisitor::reset_n_hires_vertices()
{
   n_hires_vertices=0;
}

inline int HiresDataVisitor::get_n_hires_vertices() const
{
   return n_hires_vertices;
}

inline void HiresDataVisitor::set_tdp_file_ptr(Tdp_file* tdp_ptr)
{
   tdp_file_ptr=tdp_ptr;
}

inline void HiresDataVisitor::set_zeroth_xyz_ptr(threevector* xyz0_ptr)
{
   zeroth_xyz_ptr=xyz0_ptr;
}

inline void HiresDataVisitor::set_xyz_byte_counter_ptr(int* counter_ptr)
{
   xyz_byte_counter_ptr=counter_ptr;
}

inline void HiresDataVisitor::set_color_byte_counter_ptr(int* counter_ptr)
{
   color_byte_counter_ptr=counter_ptr;
}

inline bounding_box* HiresDataVisitor::get_bbox_ptr()
{
   return bbox_ptr;
}

#endif // __HIRESDATAVISITOR_H__
