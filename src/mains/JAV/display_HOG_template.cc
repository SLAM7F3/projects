// ====================================================================
// Program DISPLAY_HOG_TEMPLATE queries the user to enter the filename
// for some individual or combined HOG template file residing in
// HOG_template_subdir.  It then calls dlib's draw_fhog command which
// pops open a window displaying the HOG templates as a lattice of
// edgelets.
// ====================================================================
// Last updated on 11/30/13; 12/1/13
// ====================================================================

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <dlib/svm_threaded.h>
#include <dlib/gui_widgets.h>
#include <dlib/array.h>
#include <dlib/array2d.h>
#include <dlib/image_keypoint.h>
#include <dlib/image_processing.h>
#include <dlib/cmd_line_parser.h>
#include <dlib/data_io.h>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::exception;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;

int main(int argc, char** argv)
{  
   const int pyramid_levels=8;

   string HOG_template_subdir="./HOG_templates/";
   string HOG_template_filename;
   cout << "Enter HOG template filename:" << endl;
   cin >> HOG_template_filename;
   HOG_template_filename=HOG_template_subdir+HOG_template_filename;
//   cout << "Imported HOG template filename = "
//        << HOG_template_filename << endl;
   ifstream fin(HOG_template_filename.c_str(), ios::binary);

   typedef dlib::scan_fhog_pyramid<dlib::pyramid_down<pyramid_levels> > 
      image_scanner_type;
   dlib::object_detector<image_scanner_type> detector;
   dlib::deserialize(detector, fin);

   int n_templates=detector.num_detectors();
   cout << "Number of HOG templates in input detector = "
        << n_templates << endl;
   cout << "Number of separable filters="
        << dlib::num_separable_filters(detector) << endl;

   for (int d=0; d<n_templates; d++)
   {
      dlib::image_window hogwin(dlib::draw_fhog(detector,d));
      outputfunc::enter_continue_char();
   }

}

