// ========================================================================
// Program DRAW_OBJ_BBOXES imports a bounding box parameters for a set
// of images.  It draws purple rectangles on copies of the input
// images and exports the annotated results to a new subdirectory.

//				./draw_obj_bboxes

// ========================================================================
// Last updated on 11/27/13; 11/28/13; 11/29/13
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::ofstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   timefunc::initialize_timeofday_clock();  

//   string images_subdir="/data/ImageEngine/BostonBombing/clips_1_thru_133/";
//   string JAV_subdir="/data/video/JAV/NewsWraps/early_Sep_2013/";
   string JAV_subdir="/data/video/JAV/NewsWraps/w_transcripts/";
//   string root_subdir=JAV_subdir;
//   string images_subdir=root_subdir+"jpg_frames/";

//   string root_subdir="./bundler/aleppo_1K/";
//   string images_subdir=root_subdir+"images/";
//   string root_subdir="./bundler/GrandCanyon/";

//   string root_subdir="/home/cho/Desktop/profile_faces/";
//   string images_subdir=root_subdir+"individuals/";
//   string images_subdir=root_subdir+"difficult/resized_images/homogenized/";

   string root_subdir="/home/cho/Downloads/people/standing/";
   string images_subdir=root_subdir+"difficult/resized_images/";

//   string objects_subdir=images_subdir+"FaceBboxes/";
   string objects_subdir=images_subdir+"StandingPeopleBboxes/";

   string obj_bboxes_filename=objects_subdir+"object_bboxes.dat";
   string object_detections_subdir=objects_subdir+"object_detection_bboxes/";
   filefunc::dircreate(object_detections_subdir);

   typedef map<string,vector<fourvector> > OBJ_BBOXES_MAP;

// independent string = image filename
// dependent STL vector of fourvector contains (Ulo,Uhi,Vlo,Vhi) for
//   detected object bounding boxes

   OBJ_BBOXES_MAP obj_bboxes_map;
   OBJ_BBOXES_MAP::iterator iter;

// Import object detection bboxes:

   vector<vector<string> > substrings=filefunc::ReadInSubstrings(
      obj_bboxes_filename);

   for (unsigned int i=0; i<substrings.size(); i++)
   {
      vector<string> column_strings=substrings[i];

      string image_filename=column_strings[0];
//      int bbox_index=stringfunc::string_to_number(column_strings[1]);

      double Ulo=stringfunc::string_to_number(column_strings[2]);
      double Uhi=stringfunc::string_to_number(column_strings[3]);
      double Vlo=stringfunc::string_to_number(column_strings[4]);
      double Vhi=stringfunc::string_to_number(column_strings[5]);
      fourvector bbox_params(Ulo,Uhi,Vlo,Vhi);

//      cout << "image_index = " << image_index 
//           << " bbox_index = " << bbox_index << endl;
//      cout << "Ulo = " << Ulo << " Uhi = " << Uhi
//           << " Vlo = " << Vlo << " Vhi = " << Vhi << endl;
//      outputfunc::enter_continue_char();

      iter=obj_bboxes_map.find(image_filename);
      if (iter==obj_bboxes_map.end())
      {
         vector<fourvector> V;
         V.push_back(bbox_params);
         obj_bboxes_map[image_filename]=V;
      }
      else
      {
         iter->second.push_back(bbox_params);
      }
   } // loop over index i labeling lines in obj_bboxes_filename
   
// Superpose purple bboxes on images containing detected objects.
// Export annotated images to object_detections_subdir.

   int i=0;
   for (iter=obj_bboxes_map.begin(); iter != obj_bboxes_map.end(); iter++)
   {
      outputfunc::update_progress_fraction(i++,100,obj_bboxes_map.size());

      string image_filename=iter->first;
//      cout << "image_filename = " << image_filename << endl;
      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         image_filename,NULL);

      vector<fourvector> bbox_params=iter->second;

      int bbox_color_index=colorfunc::brightpurple;
      int line_thickness=1;
      videofunc::display_bboxes(
         bbox_params,texture_rectangle_ptr,bbox_color_index,line_thickness);

      string basename=filefunc::getbasename(image_filename);
      string annotated_filename=object_detections_subdir+basename;
      
      texture_rectangle_ptr->write_curr_frame(annotated_filename);
      delete texture_rectangle_ptr;

      string banner="Exported "+annotated_filename;
      outputfunc::write_banner(banner);
   } // loop over obj_bboxes_map iterator

   cout << "At end of program DRAW_OBJ_BBOXES" << endl;
   outputfunc::print_elapsed_time();
}
