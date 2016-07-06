// ========================================================================
// Program GENERATE_PANELS reads in "bundler-style" image_list and
// image_sizes files for set of 360 deg panorama mosaics.  It chops up
// each input panorama into 10 individual panels which should be
// oriented 36 degrees apart in azimuth.  The individual panel images
// are written to a panels subdirectory of the images directory
// holding all of the input mosaics.  Each panel is labeled by the
// name of its parent panorama and a "pn" descriptor where n ranges
// from 0 to 9.

//				generate_panels

// ========================================================================
// Last updated on 1/19/11; 2/8/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "math/constant_vectors.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "image/imagefuncs.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osgOperations/Operations.h"
#include "optimum/optimizer.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgWindow/ViewerManager.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{

   string bundler_IO_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/rasr/";
   string image_list_filename=bundler_IO_subdir+"image_list.dat";
   string image_sizes_filename=bundler_IO_subdir+"image_sizes.dat";
   
   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->generate_bundler_photographs(
      bundler_IO_subdir,image_list_filename,image_sizes_filename);

   int n_photos(photogroup_ptr->get_n_photos());
   cout << "n_photos = " << n_photos << endl;

   int n_panels=10;
//   n_photos=1;
   for (int n=0; n<n_photos; n++)
   {
      photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(n);
      string filename=photo_ptr->get_filename();

      int xdim=photo_ptr->get_xdim();
      int ydim=photo_ptr->get_ydim();
      
//      cout << "n = " << n << " filename = " << filename 
//           << " xdim = " << xdim
//           << " ydim = " << ydim 
//           << endl;

      string subdir=filefunc::getdirname(filename);
      string basename=filefunc::getbasename(filename);
      string filename_copy=subdir+"copy_"+basename;
      string panels_subdir=subdir+"panels/";
      filefunc::dircreate(panels_subdir);

      cout << "filename_copy = " << filename_copy << endl;
      cout << "panels_subdir = " << panels_subdir << endl;

      for (int p=0; p<n_panels; p++)
      {
         string unix_cmd="cp "+filename+" "+filename_copy;
         sysfunc::unix_command(unix_cmd);
         
         int width=double(xdim)/double(n_panels);
         int height=ydim;
         int xoffset=double(p*xdim)/double(n_panels);
         int yoffset=0;
         imagefunc::crop_image(filename_copy,width,height,xoffset,yoffset);
         
         string basename_prefix=stringfunc::prefix(basename);
         string basename_suffix=stringfunc::suffix(basename);

         unix_cmd="mv "+filename_copy+" "+panels_subdir+basename_prefix+
            "_p"+stringfunc::number_to_string(p)+"."+basename_suffix;
         sysfunc::unix_command(unix_cmd);
      } // loop over index p labeling panels
   } // loop over index n labeling photos


}

