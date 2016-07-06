// ==========================================================================
// Program GENERATE_TEST_POINTS exports an OSGA file containing 4
// colored points located at (0,0,0), (10,0,0), (0,10,0) and (0,0,10).
// ==========================================================================
// Last updated on 10/16/13
// ==========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "math/prob_distribution.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   unsigned char max_char=stringfunc::ascii_integer_to_unsigned_char(255);
   unsigned char min_char=stringfunc::ascii_integer_to_unsigned_char(0);
   osg::Vec4ub red_RGBA(max_char,min_char,min_char,max_char);
   osg::Vec4ub green_RGBA(min_char,max_char,min_char,max_char);
   osg::Vec4ub blue_RGBA(min_char,min_char,max_char,max_char);
   osg::Vec4ub white_RGBA(max_char,max_char,max_char,max_char);

   vector<threevector>* testpoints_vertices_ptr=new vector<threevector>;
   osg::Vec4ubArray* testpoints_colors_ptr=new osg::Vec4ubArray;

   testpoints_vertices_ptr->push_back(threevector(0,0,0));
   testpoints_vertices_ptr->push_back(threevector(10,0,0));
   testpoints_vertices_ptr->push_back(threevector(0,10,0));
   testpoints_vertices_ptr->push_back(threevector(0,0,10));

   testpoints_colors_ptr->push_back(white_RGBA);
   testpoints_colors_ptr->push_back(red_RGBA);
   testpoints_colors_ptr->push_back(green_RGBA);
   testpoints_colors_ptr->push_back(blue_RGBA);

// Export 3D point cloud corresponding to matching image pairs:

   string testpoints_tdp_filename="test_points.tdp";
   tdpfunc::write_relative_xyzrgba_data(
      testpoints_tdp_filename,"",
      testpoints_vertices_ptr->at(0),
      testpoints_vertices_ptr,testpoints_colors_ptr);
   string unix_cmd="lodtree "+testpoints_tdp_filename;
   sysfunc::unix_command(unix_cmd);
   string testpoints_osga_filename="testpoints_points.osga";

   string banner="Exported "+testpoints_osga_filename;
   outputfunc::write_banner(banner);

}

