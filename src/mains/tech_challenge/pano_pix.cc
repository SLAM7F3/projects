// ========================================================================
// Program PANO_PIX queries the user to enter the starting and
// stopping image numbers for photos to be mosaiced as well as the
// image number skip.  Scanning through the subdirectory
// containing all input JPEG images, this program generates and
// executes a unix command which copies the requested mosaic photo
// constituents to a subdirectory of
// /data/tech_challenge_local/pano_pix/.

//				pano_pix

// ========================================================================
// Last updated on 8/31/10; 9/1/10; 9/12/10
// ========================================================================

#include <iostream>
#include <string>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;

// ==========================================================================
int main( int argc, char** argv )
{
   string basefilename="image";
//   cout << "Enter panorama base filename:" << endl;
//   cin >> basefilename;
  
   string suffix=".jpg";
   string imagedir=
      "/data/tech_challenge_local/field_tests/DavisField_Aug19/";
//   cout << "Enter subdir containing panorama JPG images:"<< endl;
//   cin >> imagedir;

   int n_start,n_stop;
   videofunc::find_min_max_photo_numbers(imagedir,n_start,n_stop);
   int ndigits=mathfunc::ndigits_before_decimal_point(n_stop);
//   cout << "ndigits = " << ndigits << endl;

   cout << "Enter starting image number:" << endl;
   cin >> n_start;
   
   cout << "Enter stopping image number:" << endl;
   cin >> n_stop;

   int n_skip=1;
   cout << "Enter image number skip:" << endl;
   cin >> n_skip;

   string pano_basedir="/data/tech_challenge_local/pano_pix/";
   string subdir=stringfunc::integer_to_string(n_start,ndigits)+"-"+
      stringfunc::integer_to_string(n_stop,ndigits)+"/";
   string pano_subdir=pano_basedir+subdir;
   filefunc::dircreate(pano_subdir);

   for (int n=n_start; n <= n_stop; n += n_skip)
   {
      string curr_filename=
         imagedir+"IMG_"+stringfunc::integer_to_string(n,ndigits)+".JPG";
      string unix_cmd="cp "+curr_filename+" "+pano_subdir;
      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
   }
}
