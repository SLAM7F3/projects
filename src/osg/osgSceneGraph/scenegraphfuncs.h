// ==========================================================================
// Header file for scenegraph methods
// ==========================================================================
// Last modified on 11/10/06; 9/9/07; 11/12/10
// ==========================================================================

#ifndef SCENEGRAPHFUNCS_H
#define SCENEGRAPHFUNCS_H

#include <iostream>
#include <string>
#include <vector>
#include <osg/Array>
#include <osg/Geode>
#include <osg/Geometry>

class ColorMap;
class ColormapPtrs;
class Tdp_file;
class threevector;

namespace scenegraphfunc
{
   osg::Geometry* get_geometry(osg::Geode* Geode_ptr);
   std::vector<osg::Geometry*> get_geometries(osg::Geode* Geode_ptr);
   int get_n_geometry_vertices(osg::Geometry* Geometry_ptr,
                               bool indices_stored_flag);

// Coloring methods:

   osg::Vec4ubArray* instantiate_color_array(
      unsigned int nbins,bool fill_color_array,osg::Geometry* geometry_ptr,
      std::string geometry_name);
   std::string get_mutable_colors_label();
   std::string get_fixed_colors_label();
   void save_fixed_colors(osg::Geometry* curr_Geometry_ptr);

// Geometry output methods:

   void write_geometry_xyzp(
      std::ofstream& binary_outstream,osg::Geometry* curr_Geometry_ptr,
      const osg::Matrix& LocalToWorld);
   void write_geometry_xyzrgba(
      std::ofstream& binary_outstream,osg::Geometry* curr_Geometry_ptr,
      const osg::Matrix& LocalToWorld);
   void write_geometry_relative_xyzrgba(
      const threevector& zeroth_xyz,Tdp_file& tdp_file,
      int& xyz_byte_counter,int& color_byte_counter,
      ColormapPtrs* ColormapPtrs_ptr,
      osg::Geometry* curr_Geometry_ptr,const osg::Matrix& LocalToWorld);
}

#endif // scenegraphfuncs.h



