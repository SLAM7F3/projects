// ====================================================================
// Program HOG_DETECT is a nontrivial rewrite of Davis King's HOG
// template-based object detector.  It first imports a set of image
// filenames from some specified directory.  It also imports a
// previously trained HOG template.  Using DLIB's parallel-for
// mechanism, HOG_DETECT then searches for objects matching the HOG
// template within each input image.  Detected object bounding boxes
// are exported to an output text file.
// ====================================================================
// Last updated on 11/29/13; 11/30/13; 12/1/13
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

//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string JAV_subdir="/data/video/JAV/UIUC_Broadcast_News/";

//   string root_subdir=JAV_subdir;
//   string images_subdir=root_subdir+"jpg_frames/";
//   string images_subdir=root_subdir+"faces_4/";
//   string images_subdir=root_subdir+"faces_3/";
//   string images_subdir=root_subdir+"faces_2/";

//   string root_subdir="./bundler/aleppo_1K/";
//   string images_subdir=root_subdir+"faces_3/";
//   string root_subdir="./bundler/GrandCanyon/";
//   string images_subdir=root_subdir+"faces_2/";

//   string root_subdir="/home/cho/Desktop/profile_faces/";
//   string images_subdir=root_subdir+"individuals/";
//   string images_subdir=root_subdir+"difficult/resized_images/homogenized/";

   string root_subdir="/home/cho/Downloads/people/standing/";
//   string images_subdir=root_subdir+"individuals/";
   string images_subdir=root_subdir+"difficult/resized_images/";

//   string objects_subdir=images_subdir+"FaceBboxes/";
   string objects_subdir=images_subdir+"StandingPeopleBboxes/";
   filefunc::dircreate(objects_subdir);

// Import image filenames:

   vector<string> image_filenames=filefunc::image_files_in_subdir(    
      images_subdir);

   object_detector* object_detector_ptr=new object_detector();
   object_detector_ptr->set_n_images(image_filenames.size());
      
// Import trained HOG template from file on disk:

   dlib::command_line_parser parser;
   parser.parse(argc, argv);
   string HOG_template_filename(parser[0]);
   object_detector_ptr->import_detector(HOG_template_filename);

// Initialize output text file for object bboxes:

   string bbox_filename=objects_subdir+"object_bboxes.dat";
   ofstream bbox_stream;         
   filefunc::openfile(bbox_filename,bbox_stream);
   bbox_stream << "# image_index bbox_index Ulo Uhi Vlo Vhi" 
               << endl << endl;

//   object_detector_ptr->find_all_HOG_objects(image_filenames,bbox_stream);
   object_detector_ptr->parallel_find_HOG_objects(image_filenames,bbox_stream);

   delete object_detector_ptr;

   filefunc::closefile(bbox_filename,bbox_stream);
   string banner="Exported "+bbox_filename;
   outputfunc::write_banner(banner);
   
   filefunc::ReadInfile(bbox_filename);
   int n_detected_objects=filefunc::text_line.size();
   banner="Detected "+stringfunc::number_to_string(n_detected_objects)+
      " objects";
   outputfunc::write_big_banner(banner);

   banner="At end of program HOG_DETECT";
   outputfunc::write_big_banner(banner);
   outputfunc::print_elapsed_time();   
}

