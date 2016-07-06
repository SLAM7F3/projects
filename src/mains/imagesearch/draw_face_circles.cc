// ========================================================================
// Program DRAW_FACE_CIRCLES imports face circle parameters generated
// via program FINDFACES.  It superposes purple circles on images
// containing detected faces.  The annotated pictures are exported to
// faces_subdir.
// ========================================================================
// Last updated on 10/6/13; 11/1/13
// ========================================================================

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "general/filefuncs.h"
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
   string root_subdir=JAV_subdir;

   string images_subdir=root_subdir+"jpg_frames/";

   string faces_subdir=root_subdir+"faces/";
   string face_detections_filename=faces_subdir+"face_detections.txt";

   typedef map<string,vector<threevector> > FACES_MAP;
   FACES_MAP faces_map;
   FACES_MAP::iterator iter;

// Import face detection circles:

   filefunc::ReadInfile(face_detections_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      string image_filename=substrings[0];
      double Ucenter=stringfunc::string_to_number(substrings[1]);
      double Vcenter=stringfunc::string_to_number(substrings[2]);
      double radius=stringfunc::string_to_number(substrings[3]);
      threevector circle_params(Ucenter,Vcenter,radius);

      iter=faces_map.find(image_filename);
      if (iter==faces_map.end())
      {
         vector<threevector> V;
         V.push_back(circle_params);
         faces_map[image_filename]=V;
      }
      else
      {
         iter->second.push_back(circle_params);
      }
   } // loop over index i labeling lines in face_detections_filename
   
// Superpose purple circles on images containing detected faces.
// Export annotated images to faces_subdir.

   int i=0;
   for (iter=faces_map.begin(); iter != faces_map.end(); iter++)
   {
      outputfunc::update_progress_fraction(i++,100,faces_map.size());

      string image_filename=iter->first;
      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         image_filename,NULL);
      vector<threevector> V=iter->second;

      vector<double> circle_radii;
      vector<twovector> circle_centers;
      for (unsigned int c=0; c<V.size(); c++)
      {
         circle_centers.push_back(twovector(V[c]));
         circle_radii.push_back(V[c].get(2));
      }

      string human_faces_image_filename=faces_subdir+
         filefunc::getbasename(image_filename);
      videofunc::display_circles(
         texture_rectangle_ptr,human_faces_image_filename,
         circle_centers,circle_radii);
      delete texture_rectangle_ptr;

      string banner="Exported "+human_faces_image_filename;
      outputfunc::write_banner(banner);
   } // loop over faces_map iterator

   cout << "At end of program DRAW_FACE_CIRCLES" << endl;
   outputfunc::print_elapsed_time();
}
