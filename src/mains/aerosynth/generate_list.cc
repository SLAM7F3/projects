// ==========================================================================
// Program GENERATELIST is a little utility which we wrote in order to
// hard code an initial guess for a video camera's focal length (measured in
// pixels) into a list.txt file read by Noah's Bundler codes.  

// Bundler needs some estimate for the focal parameter.  And for video
// frames, focal information is contained within input frames' exif
// metadata tags.  So Noah told us in Dec 2010 that we need to
// recreate the list.txt file with focal parameter information
// included.  We then need to set the IMAGE_LIST environment variable
// to point towards list.txt.  In bash, chant	 

// 			export IMAGE_LIST=list.txt

// Then run Noah's BundlerVocab script on LLGrid.

//				generate_list

// ==========================================================================
// Last updated on 9/2/11; 9/8/11; 3/14/12
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::string;
using std::vector;


// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string list_filename="list.txt";
   ofstream outstream;
   filefunc::openfile(list_filename,outstream);

   string subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/imagesearch/aerial_1.3K_MITframes/";
   
   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("jpg");
   vector<string> jpg_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,subdir);

// "5 deg" FLIR imagery of HAFB:

//   int n_start=159;	// Geo15
//   int n_start=160;	// Geo15
//   int n_stop=573;	// Geo15
//   int n_start=224;	// Geo4
//   int n_stop=535;	// Geo4
//   int n_start=761;
//   int n_stop=1634;
//   int n_start=1;
//   int n_stop=507;
//   int n_start=125;
//   int n_stop=310;
//   int n_start=0;
//   int n_stop=260;
//   int dn=1;
//   int dn=2;
//   const string subdir="images/";
   int dn=9;
   double FOV_u=4.57*PI/180;
   double FOV_v=2.57*PI/180;
   int n_horiz_pixels=1280;
   int n_vert_pixels=720;

/*
// Cropped GEO video frame size:

   int n_horiz_pixels=1152;
   int n_vert_pixels=628;

//   string imagefilename_prefix="LL_clip17_May25_flight1_";
//   string imagefilename_prefix="CreditUnion_May25_flight1_";
//   string imagefilename_prefix="LL_May25_flight1_";
//   string imagefilename_prefix="GC_FLIR_clip5_-";
//   string imagefilename_prefix="Passfour_";
   string imagefilename_prefix="Passfifteen_";
*/

/*
// "30 deg" FLIR imagery of HAFB:

   int n_start=1;
   int n_stop=223;
//   int n_start=585;
//   int n_stop=760;
//   int n_start=1635;
//   int n_stop=1999;
//   int n_start=100;
//   int n_stop=411;
   int dn=1;
   string subdir="images/";
   double FOV_u=28.4*PI/180;
   double FOV_v=16.2*PI/180;

//   int n_horiz_pixels=1280;
//   int n_vert_pixels=720;
*/

/*
// Cropped GEO video frame size:

   int n_horiz_pixels=1152;
   int n_vert_pixels=628;

//   string imagefilename_prefix="GC_FLIR_clip5_-";
//   string imagefilename_prefix="GCFLIR";
//   string imagefilename_prefix="HAFB_";
   string imagefilename_prefix="Passfour_";
//   string imagefilename_prefix="Passfifteen_";
*/

/*
// Light Hawk imagery over Lubbock Stadium:

   int n_start=0;
   int n_stop=1550;
//   int n_stop=1559;
   int dn=1;
//   int dn=4;
//   int dn=5;
   string subdir="images/";

   double FOV_u=33.43*PI/180;
   double FOV_v=22.63*PI/180;

//   int n_horiz_pixels=4008;	// raw LH image size
//   int n_vert_pixels=2671;	// raw LH image size

   int n_horiz_pixels=2400;	// bundler sampled LH image size
   int n_vert_pixels=1599;	// bundler sampled LH image size
*/

   double aspect_ratio=double(n_horiz_pixels)/double(n_vert_pixels);
   cout << "n_horiz/n_vert = " << aspect_ratio << endl;

   double f;
   camerafunc::f_and_aspect_ratio_from_horiz_vert_FOVs(
      FOV_u,FOV_v,f,aspect_ratio);
   cout << "f = " << f << " calculated aspect ratio = " << aspect_ratio
        << endl;

   double f_n_horiz_pixels=fabs(f)*n_horiz_pixels;
   double f_n_vert_pixels=fabs(f)*n_vert_pixels;
   cout << "f*n_horiz_pixels = " << f_n_horiz_pixels << endl;
   cout << "f*n_vert_pixels = " << f_n_vert_pixels << endl;

   outputfunc::enter_continue_char();

// Critical note:  f_noah in bundler = f_n_vert_pixels !

/*
   for (int n=n_start; n<=n_stop; n += dn)
   {
      if (n==0) continue;
//      string curr_filename=subdir+"HAFB_"+stringfunc::integer_to_string(n,4)+
//         ".jpg";
      const int ndigits=4;
      string curr_filename=subdir+imagefilename_prefix+
         stringfunc::integer_to_string(n,ndigits)+".jpg";
//      string curr_filename=subdir+"frame"+stringfunc::integer_to_string(n,6)+
//         ".jpg";
      outstream << curr_filename 
                << " 0 "
                << f_n_vert_pixels
                << endl;
   }
*/

   for (int n=0; n<jpg_filenames.size(); n += dn)
   {
      string curr_filename=filefunc::getbasename(jpg_filenames[n]);
      outstream << curr_filename 
                << " 0 "
                << f_n_vert_pixels
                << endl;
   }


   filefunc::closefile(list_filename,outstream);


   string banner="Wrote images and estimated value for f*Npy to "+
      list_filename;
   outputfunc::write_banner(banner);
}
