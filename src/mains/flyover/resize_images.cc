// ==========================================================================
// Program RESIZE_IMAGES needs to be executed from within some
// subdirectory containg a set of PNG or JPG files.  It generates
// subsampled versions of each input image whose maximum width and
// height do not exceed 1280 x 960.  The output resized images are
// exported to ./resized_images.

//			       resize_images

// ==========================================================================
// Last updated on 2/18/14; 2/21/14; 2/25/14
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ofstream;
   using std::string;
   using std::vector;

   std::set_new_handler(sysfunc::out_of_memory);

//   string imagery_subdir="./images/Boston/";
//   string imagery_subdir="./images/000175/";
//   string imagery_subdir="./images/HalfMoonBay/";
//   string imagery_subdir="./images/Toronto/";
   string imagery_subdir="./images/Sam/";
   string resized_imagery_subdir=imagery_subdir+"resized_images/";
   filefunc::dircreate(resized_imagery_subdir);

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("png");
   allowed_suffixes.push_back("jpg");
   allowed_suffixes.push_back("JPG");
   vector<string> image_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,imagery_subdir);

   unsigned int max_xdim=640;
   unsigned int max_ydim=480;
//   unsigned int max_xdim=1280;
//   unsigned int max_ydim=960;
   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
      cout << "Downsizing image "+stringfunc::number_to_string(i)+" of "+
         stringfunc::number_to_string(image_filenames.size()) << endl;

      unsigned int width,height;
      imagefunc::get_image_width_height(image_filenames[i],width,height);

      string downsized_image_filename=
         resized_imagery_subdir+filefunc::getbasename(image_filenames[i]);
      videofunc::downsize_image(
         image_filenames[i],max_xdim,max_ydim,downsized_image_filename);

      unsigned int downsized_width,downsized_height;
      imagefunc::get_image_width_height(
         downsized_image_filename, downsized_width, downsized_height);

      double width_frac=double(downsized_width)/double(width);
      double height_frac=double(downsized_height)/double(height);
      cout << "width_frac = " << width_frac << " height_frac = " << height_frac
	   << endl;

      string image_subdir=filefunc::getdirname(image_filenames[i]);
      string image_prefix=filefunc::getprefix(image_filenames[i]);
      int prefix_size=image_prefix.size();
      if (prefix_size < 9) continue;
      cout << "image_prefix = " << image_prefix << endl;
      string image_prefix_wo_training=image_prefix.substr(8,prefix_size-8);
      cout << "image_prefix_wo_training = " << image_prefix_wo_training 
           << endl;
      string outline_filename=image_subdir+"outline"+
         image_prefix_wo_training+".txt";
      cout << "outline_filename = " << outline_filename << endl;

      string downsized_outline_filename=resized_imagery_subdir+
         "outline"+image_prefix_wo_training+".txt";
      ofstream outstream;
      filefunc::openfile(downsized_outline_filename,outstream);

      if (!filefunc::fileexist(outline_filename)) continue;
      vector< vector<double> > row_numbers=
         filefunc::ReadInRowNumbers(outline_filename);

// Ignore final row in Sam's outline text files which is sometimes
// corrupted:

      unsigned int prev_downsized_px=-1000;
      unsigned int prev_downsized_py=-1000;
      unsigned int n_rows=row_numbers.size();
      for (unsigned int r=0; r<n_rows-1; r++)
      {
         double px=row_numbers[r].at(0);
         double py=row_numbers[r].at(1);
         unsigned int downsized_px=width_frac * px;
         unsigned int downsized_py=height_frac * py;

	 downsized_px = basic_math::max(unsigned(0),downsized_px);
	 downsized_py = basic_math::max(unsigned(0),downsized_py);
	 downsized_px = basic_math::min(width-1,downsized_px);
	 downsized_py = basic_math::min(height-1,downsized_py);
	 
	 if ( (downsized_px != prev_downsized_px) ||
              (downsized_py != prev_downsized_py) )
	 {
	   outstream << downsized_px << "  " << downsized_py << endl;
	 }
         prev_downsized_px=downsized_px;
         prev_downsized_py=downsized_py;
      }

      filefunc::closefile(downsized_outline_filename,outstream);
      
   } // loop over index i labeling image filenames

   string banner="Exported "+stringfunc::number_to_string(
      image_filenames.size())+" downsized images "+resized_imagery_subdir;
   outputfunc::write_big_banner(banner);
   
} 

