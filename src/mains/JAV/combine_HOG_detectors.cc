// ====================================================================
// Program COMBINE_HOG_DETECTORS imports a set of equally sized HOG
// templates.  It combines and exports them into a single binary file.  
// ====================================================================
// Last updated on 12/1/13
// ====================================================================

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <dlib/array.h>
#include <dlib/array2d.h>
#include <dlib/image_processing.h>
#include <dlib/cmd_line_parser.h>
#include <dlib/data_io.h>

#include "math/basic_math.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "video/object_detector.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char** argv)
{  
   timefunc::initialize_timeofday_clock(); 

// Import trained HOG template from file on disk:

   vector<string> HOG_template_filenames;
   string HOG_template_subdir="./HOG_templates/";
   HOG_template_filenames.push_back(
      HOG_template_subdir+"right_face_profile_detector.dat");
   HOG_template_filenames.push_back(
      HOG_template_subdir+"left_face_profile_detector.dat");

   object_detector* object_detector_ptr=new object_detector();
   string combined_detector_filename=
      object_detector_ptr->combine_HOG_templates(HOG_template_filenames);
   delete object_detector_ptr;

   string banner="Exported combined HOG template to "
      +combined_detector_filename;
   outputfunc::write_big_banner(banner);
}

