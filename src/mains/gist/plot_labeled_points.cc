// ==========================================================================
// Program PLOT_LABELED_POINTS imports 3-feature vectors for pairs of
// manually labeled matching and nonmatching image pairs.  It
// generates and exports .osga files which visualize these labeled
// feature vectors as green and red points in 3D space.
// PLOT_LABELED_POINTS also exports probability density distributions
// for color, GIST and texture feature values for both matching and
// nonmatching image pair samples.
// ==========================================================================
// Last updated on 10/15/13; 10/16/13
// ==========================================================================

// Observations for 231 matching pairs and 680 nonmatching pairs:

// Global matching color score < 0.28  --> image pair does not match
// Global matching color score > 0.72  --> image pair does match

// Local matching color score < 0.32 --> image pair does not match
// Local matching color score > 0.72 --> image pair does match

// Global matching texture score > 0.76 --> image pair does match

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
 
   string ImageEngine_subdir="/data/ImageEngine/";
   string tidmarsh_subdir=ImageEngine_subdir+"tidmarsh/";
   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";

   string root_subdir=JAV_subdir;
//   string root_subdir=tidmarsh_subdir;

   string matching_images_filename=root_subdir+"matching_images.dat";
   string nonmatching_images_filename=root_subdir+"nonmatching_images.dat";

   vector< vector<double> > matching_number_rows=filefunc::ReadInRowNumbers(
      matching_images_filename);
   vector< vector<double> > nonmatching_number_rows=filefunc::ReadInRowNumbers(
      nonmatching_images_filename);

   vector<threevector>* matching_vertices_ptr=new vector<threevector>;
   osg::Vec4ubArray* matching_colors_ptr=new osg::Vec4ubArray;
   vector<threevector>* nonmatching_vertices_ptr=new vector<threevector>;
   osg::Vec4ubArray* nonmatching_colors_ptr=new osg::Vec4ubArray;

   unsigned char max_char=stringfunc::ascii_integer_to_unsigned_char(255);
   unsigned char min_char=stringfunc::ascii_integer_to_unsigned_char(0);
   osg::Vec4ub red_RGBA(max_char,min_char,min_char,max_char);
   osg::Vec4ub green_RGBA(min_char,max_char,min_char,max_char);
   osg::Vec4ub blue_RGBA(min_char,min_char,max_char,max_char);
   
// Assemble STL vectors containing threevectors corresponding to
// matching and non-matching image pairs:

   threevector COM(Zero_vector);
   vector<double> matching_color,matching_gist,matching_texture;
   for (unsigned int i=0; i<matching_number_rows.size(); i++)
   {
      matching_vertices_ptr->push_back(threevector(
         matching_number_rows[i].at(0),
         matching_number_rows[i].at(1),
         matching_number_rows[i].at(2)));
      matching_colors_ptr->push_back(green_RGBA);
      COM += matching_vertices_ptr->back();

      matching_color.push_back(matching_number_rows[i].at(0));
      matching_gist.push_back(matching_number_rows[i].at(1));
      matching_texture.push_back(matching_number_rows[i].at(2));
   }

   prob_distribution matching_color_prob(matching_color,50,0);
   matching_color_prob.writeprobdists(false);
   string unix_cmd="mv prob_density.jpg matching_color_density.jpg";
   sysfunc::unix_command(unix_cmd);   

   prob_distribution matching_gist_prob(matching_gist,50,0);
   matching_gist_prob.writeprobdists(false);
   unix_cmd="mv prob_density.jpg matching_gist_density.jpg";
   sysfunc::unix_command(unix_cmd);   

   prob_distribution matching_texture_prob(matching_texture,50,0);
   matching_texture_prob.writeprobdists(false);
   unix_cmd="mv prob_density.jpg matching_texture_density.jpg";
   sysfunc::unix_command(unix_cmd);   

   vector<double> nonmatching_color,nonmatching_gist,nonmatching_texture;
   for (unsigned int i=0; i<nonmatching_number_rows.size(); i++)
   {
      nonmatching_vertices_ptr->push_back(threevector(
         nonmatching_number_rows[i].at(0),
         nonmatching_number_rows[i].at(1),
         nonmatching_number_rows[i].at(2)));
      nonmatching_colors_ptr->push_back(red_RGBA);
      COM += nonmatching_vertices_ptr->back();

      nonmatching_color.push_back(nonmatching_number_rows[i].at(0));
      nonmatching_gist.push_back(nonmatching_number_rows[i].at(1));
      nonmatching_texture.push_back(nonmatching_number_rows[i].at(2));
   }

   prob_distribution nonmatching_color_prob(nonmatching_color,50,0);
   nonmatching_color_prob.writeprobdists(false);
   unix_cmd="mv prob_density.jpg nonmatching_color_density.jpg";
   sysfunc::unix_command(unix_cmd);   

   prob_distribution nonmatching_gist_prob(nonmatching_gist,50,0);
   nonmatching_gist_prob.writeprobdists(false);
   unix_cmd="mv prob_density.jpg nonmatching_gist_density.jpg";
   sysfunc::unix_command(unix_cmd);   

   prob_distribution nonmatching_texture_prob(nonmatching_texture,50,0);
   nonmatching_texture_prob.writeprobdists(false);
   unix_cmd="mv prob_density.jpg nonmatching_texture_density.jpg";
   sysfunc::unix_command(unix_cmd);   

   COM /= (matching_vertices_ptr->size()+nonmatching_vertices_ptr->size());
   cout << "COM = " << COM << endl;
   outputfunc::enter_continue_char();

// Artificially magnify offsets wrt COM for visualization purposes
// only:

   threevector magnification_factors(1,1,1);
//   threevector magnification_factors(1,10,1);
   for (unsigned int i=0; i<matching_vertices_ptr->size(); i++)
   {
      threevector curr_delta=matching_vertices_ptr->at(i)-COM;
      for (unsigned int d=0; d<curr_delta.get_mdim(); d++)
      {
         curr_delta.put(d,magnification_factors.get(d)*curr_delta.get(d));
      }
      matching_vertices_ptr->at(i)=COM+curr_delta;
   } // loop over index i labeling matching vertices

   for (unsigned int i=0; i<nonmatching_vertices_ptr->size(); i++)
   {
      threevector curr_delta=nonmatching_vertices_ptr->at(i)-COM;
      for (unsigned int d=0; d<curr_delta.get_mdim(); d++)
      {
         curr_delta.put(d,magnification_factors.get(d)*curr_delta.get(d));
      }
      nonmatching_vertices_ptr->at(i)=COM+curr_delta;
   } // loop over index i labeling nonmatching vertices

// Export 3D point cloud corresponding to matching image pairs:

   string matching_tdp_filename="matching_points.tdp";
   tdpfunc::write_relative_xyzrgba_data(
      matching_tdp_filename,"",matching_vertices_ptr->at(0),
      matching_vertices_ptr,matching_colors_ptr);
   unix_cmd="lodtree "+matching_tdp_filename;
   sysfunc::unix_command(unix_cmd);
   string matching_osga_filename="matching_points.osga";
   unix_cmd="/bin/rm "+matching_tdp_filename;
   sysfunc::unix_command(unix_cmd);

   string banner="Exported "+matching_osga_filename;
   outputfunc::write_banner(banner);

// Export 3D point cloud corresponding to nonmatching image pairs:


   string nonmatching_tdp_filename="nonmatching_points.tdp";
   tdpfunc::write_relative_xyzrgba_data(
      nonmatching_tdp_filename,"",nonmatching_vertices_ptr->at(0),
      nonmatching_vertices_ptr,nonmatching_colors_ptr);
   unix_cmd="lodtree "+nonmatching_tdp_filename;
   sysfunc::unix_command(unix_cmd);
   string nonmatching_osga_filename="nonmatching_points.osga";
   unix_cmd="/bin/rm "+nonmatching_tdp_filename;
   sysfunc::unix_command(unix_cmd);

   banner="Exported "+nonmatching_osga_filename;
   outputfunc::write_banner(banner);

}

