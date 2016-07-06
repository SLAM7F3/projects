// ==========================================================================
// Program WEBTEST
// ==========================================================================
// Last updated on 2/16/07
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "web/curl.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   cout.precision(15);

//   string url_name="http://llwww/";
//   string url_name="http://llwww/Image-Lib/nav/bg2a.jpg";

   string url_name="http://mdax/cgi-bin/mapserv?map=sci.map";
   url_name +="&SERVICE=WMS&VERSION=1.1.1&REQUEST=GetMap";

   string layers_substring="&LAYERS=imagery_us_ca_san_clemente_hires";
   url_name += layers_substring;

   double min_longitude=-118.657;
   double min_latitude=32.7757;
   double max_longitude=-118.3147;
   double max_latitude=33.0644;
   
   const int nprecision=12;
   string longlat_bbox_substring="&BBOX="
      +stringfunc::number_to_string(min_longitude,nprecision)+","
      +stringfunc::number_to_string(min_latitude,nprecision)+","
      +stringfunc::number_to_string(max_longitude,nprecision)+","
      +stringfunc::number_to_string(max_latitude,nprecision);
   url_name += longlat_bbox_substring;

//   int pixel_width=800;
//   int pixel_height=800;
   int pixel_size;
   cout << "Enter pixel size:" << endl;
   cin >> pixel_size;
   int pixel_width=pixel_size;
   int pixel_height=pixel_size;
   string image_dims_substring=
      "&WIDTH="+stringfunc::number_to_string(pixel_width)+
      "&HEIGHT="+stringfunc::number_to_string(pixel_height);
   url_name += image_dims_substring;

//   cout << "url_name = " << url_name << endl << endl;

   curl curr_curl(url_name);
   curr_curl.configure_handle();
   curr_curl.retrieve_URL_contents();
}

