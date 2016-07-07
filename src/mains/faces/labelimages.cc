// ========================================================================
// Program LABELIMAGES is a variant of VIEWIMAGES.  It can be used
// to scan through all image files contained within the present
// working directory.  If all the input image files do not have
// uniform pixel size, borders will be dynamically added as necessary
// to each input image so that effectively all images end up with the
// same pixel widths and heights.

// LABELIMAGES enables users to annotate individual images and
// export all such annotations to CSV text files.

//			./labelimages

// ========================================================================
// Last updated on 7/5/16; 7/6/16; 7/7/16
// ========================================================================

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGraphicals/CentersGroup.h"
#include "osg/osgGraphicals/CenterPickHandler.h"
#include "astro_geo/Clock.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "image/imagefuncs.h"
#include "osg/osg2D/ImageNumberHUD.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "time/timefuncs.h"
#include "osg/osgWindow/ViewerManager.h"

// ========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::map;
   using std::pair;
   using std::string;
   using std::vector;

   timefunc::initialize_timeofday_clock(); 
   std::set_new_handler(sysfunc::out_of_memory);

   bool ignore_hands_flag = true;

   string faces_rootdir = "/data/TrainingImagery/faces/";
   string labeled_faces_subdir = faces_rootdir + "images/";
   string bbox_labels_filename = labeled_faces_subdir+
      "labeled_face_hand_bboxes.txt";
   if(ignore_hands_flag)
   {
      bbox_labels_filename = labeled_faces_subdir+"labeled_face_bboxes.txt";
   }
   filefunc::ReadInfile(bbox_labels_filename);

   typedef map<string, vector<bounding_box> > ANNOTATED_BBOXES_MAP;
// independent string: image_ID_str
// dependent STL vector: annotated bboxes

   ANNOTATED_BBOXES_MAP annotated_bboxes_map;
   ANNOTATED_BBOXES_MAP::iterator annotated_bboxes_iter;

   typedef map<int, pair<string, int> > OSG_BBOXES_MAP;
// independent int: OSG PolyLine ID
// dependent pair:  string = image_ID_str, int = curr image bbox index

   OSG_BBOXES_MAP osg_bboxes_map;

   bool first_image_flag = true;
   string image_ID_str = "";
   vector<bounding_box> annotated_bboxes;
   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
      string curr_line=filefunc::text_line[i];

      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         curr_line);
      if(substrings[0] == "Image:")
      {
         if(first_image_flag)
         {
            first_image_flag = false;
         }
         else
         {
            // Save previous image info into data structures
            annotated_bboxes_map[image_ID_str] = annotated_bboxes;
            annotated_bboxes.clear();
         }
         annotated_bboxes.clear();
         image_ID_str = substrings[6];
      }
      else
      {
         vector<string> substrings = 
            stringfunc::decompose_string_into_substrings(curr_line);
         string bbox_label = substrings[1];
         int px_min = stringfunc::string_to_number(substrings[2]);
         int px_max = stringfunc::string_to_number(substrings[3]);
         int py_min = stringfunc::string_to_number(substrings[4]);
         int py_max = stringfunc::string_to_number(substrings[5]);
         bounding_box curr_bbox(px_min, px_max, py_min, py_max);

         colorfunc::Color bbox_color = colorfunc::black;
         if(bbox_label == "face")
         {
//            bbox_color = colorfunc::orange;
            bbox_color = colorfunc::red;
         }
         else if(bbox_label == "hand")
         {
//            bbox_color = colorfunc::yegr;
            bbox_color = colorfunc::cyan;
         }

         curr_bbox.set_label(bbox_label);
         curr_bbox.set_color(bbox_color);

// Search for any bbox attribute key-value pairs:

         int n_attribute_pairs = (substrings.size() - 6)/2;
         for(int ap = 0; ap < n_attribute_pairs; ap++)
         {
            string attr_key = substrings[6 + ap * 2];
            string attr_value = substrings[6 + ap * 2 + 1];
            curr_bbox.set_attribute_value(attr_key, attr_value);
         }

         annotated_bboxes.push_back(curr_bbox);
//         cout << annotated_bboxes.back() << endl;

      } // substrings[0] == "Image:" conditional
   } // loop over index i labeling lines in detections text file

   // Save final image info into data structures
   annotated_bboxes_map[image_ID_str] = annotated_bboxes;

// Parse text file generated by program IMAGE_SIZES and store xdim,
// ydim values within STL map.  Find maximum pixel width and height
// among all input images:

   typedef map<string, pair<int, int> > IMAGE_SIZES_MAP;
// independent int: image_ID_str
// dependent pair:  xdim, ydim

   IMAGE_SIZES_MAP image_sizes_map;
   IMAGE_SIZES_MAP::iterator image_sizes_iter;

   string image_sizes_filename=labeled_faces_subdir+"image_sizes.dat";
   filefunc::ReadInfile(image_sizes_filename);
   int max_image_width = 0, max_image_height = 0;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i]);
      string image_basename=substrings[0];
      string image_ID_str = image_basename.substr(6,5);

      int xdim = stringfunc::string_to_number(substrings[1]);
      int ydim = stringfunc::string_to_number(substrings[2]);
      pair<int, int> P;
      P.first = xdim;
      P.second = ydim;
      image_sizes_map[image_ID_str] = P;

      if(ydim > max_image_height)
      {
         cout << "ydim = " << ydim
              << " image_basename = " << image_basename << endl;
      }


      max_image_width = basic_math::max(max_image_width, xdim);
      max_image_height = basic_math::max(max_image_height, ydim);

      
   }
   cout << "max_image_width = " << max_image_width
        << " max_image_height = " << max_image_height << endl;

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Initialize constants and parameters read in from command line as
// well as ascii text file:

   const int ndims=2;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
   cout << "videopass_ID = " << videopass_ID << endl;

// Construct the viewer and instantiate a ViewerManager:

   WindowManager* window_mgr_ptr=new ViewerManager();
   window_mgr_ptr->initialize_window("2D imagery");

// Create OSG root node:

   osg::Group* root = new osg::Group;

// Instantiate Operations object to handle mode, animation and image
// number control:

   bool display_movie_state=true;
   bool display_movie_number=true;
   bool display_movie_world_time=false;
   bool display_movie_elapsed_time=false;

   Operations operations(
      ndims,window_mgr_ptr,passes_group,display_movie_state,
      display_movie_number,display_movie_world_time,
      display_movie_elapsed_time);

   ModeController* ModeController_ptr=operations.get_ModeController_ptr();
//   ModeController_ptr->setState(ModeController::RUN_MOVIE);
   ModeController_ptr->setState(ModeController::MANIPULATE_POLYLINE);
   AnimationController* AnimationController_ptr=
      operations.get_AnimationController_ptr();
   root->addChild(operations.get_OSGgroup_ptr());

// Add a custom manipulator to the event handler list:

   osgGA::Custom2DManipulator* CM_2D_ptr = 
      new osgGA::Custom2DManipulator(ModeController_ptr,window_mgr_ptr);
   window_mgr_ptr->set_CameraManipulator(CM_2D_ptr);

// Instantiate group to hold all decorations:
   
   Decorations decorations(window_mgr_ptr,ModeController_ptr,CM_2D_ptr);

// Instantiate PolyLines decorations group:

   PolyLinesGroup* PolyLinesGroup_ptr= decorations.add_PolyLines(
      ndims,passes_group.get_pass_ptr(videopass_ID), AnimationController_ptr);
   PolyLinesGroup_ptr->set_erase_Graphicals_except_at_curr_time_flag(true);
   PolyLinesGroup_ptr->set_annotated_bboxes_map_ptr(&annotated_bboxes_map);
   PolyLinesGroup_ptr->set_osg_bboxes_map_ptr(&osg_bboxes_map);
   PolyLinesGroup_ptr->set_image_sizes_map_ptr(&image_sizes_map);

// Instantiate group to hold movie:

   MoviesGroup movies_group(
      ndims,passes_group.get_pass_ptr(videopass_ID),
      decorations.get_PointsGroup_ptr(),
      decorations.get_PolygonsGroup_ptr(),
      AnimationController_ptr);

   string images_subdir="./";
   AnimationController_ptr->store_unordered_image_filenames(images_subdir);
   int Nimages=AnimationController_ptr->get_n_ordered_image_filenames();
   cout << "Nimages = " << Nimages << endl;



//   vector<string> image_filenames =filefunc::image_files_in_subdir(
//      images_subdir);
//   string image_filename=AnimationController_ptr->get_curr_image_filename();
//   cout << "Initial image filename = " << image_filename << endl;
//   texture_rectangle* texture_rectangle_ptr=
//      movies_group.generate_new_texture_rectangle(image_filename);
   texture_rectangle *texture_rectangle_ptr = new texture_rectangle(
      max_image_width, max_image_height, 1, 4, AnimationController_ptr);
   Movie* movie_ptr=movies_group.generate_new_Movie(texture_rectangle_ptr);

   AnimationController_ptr->set_nframes(Nimages);
   movie_ptr->get_texture_rectangle_ptr()->set_first_frame_to_display(0);
   movie_ptr->get_texture_rectangle_ptr()->set_last_frame_to_display(
      Nimages-1);
   root->addChild( movies_group.get_OSGgroup_ptr() );

   MovieKeyHandler* MoviesKeyHandler_ptr=
      new MovieKeyHandler(ModeController_ptr,&movies_group);
   window_mgr_ptr->get_EventHandlers_ptr()->push_back(MoviesKeyHandler_ptr);

// Loop over all bboxes for each image.  Convert dimensionful bbox
// pixel coordinates into dimensionless (u,v) coordinates.  Then
// convert (u,v) coordinates into corresponding (U,V) coordinates
// corresponding to maximal bbox width and height:

   PolyLinesGroup_ptr->set_erase_Graphicals_except_at_curr_time_flag(true);
//   for(int n = AnimationController_ptr->get_first_framenumber(); 
//       n <= AnimationController_ptr->get_last_framenumber(); n++)
   for(int n = AnimationController_ptr->get_first_framenumber(); 
       n <= 10; n++)
   {
      if(n%10 == 0)
      {
         double progress_frac = double(n)/double(
            AnimationController_ptr->get_last_framenumber());
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      AnimationController_ptr->set_curr_framenumber(n);
      string curr_image_filename=
         AnimationController_ptr->get_curr_image_filename();
      vector<string> substrings = 
         stringfunc::decompose_string_into_substrings(
            curr_image_filename,"_.");
      string image_ID_str = substrings[1];
      
      int curr_width, curr_height;
      image_sizes_iter = image_sizes_map.find(image_ID_str);
      curr_width = image_sizes_iter->second.first;
      curr_height = image_sizes_iter->second.second;
      
      annotated_bboxes_iter = annotated_bboxes_map.find(image_ID_str);
      vector<bounding_box>* curr_bboxes_ptr = &annotated_bboxes_iter->second;

      for(unsigned int b = 0; b < curr_bboxes_ptr->size(); b++)
      {
         double ulo=curr_bboxes_ptr->at(b).get_xmin()/curr_height;
         double uhi=curr_bboxes_ptr->at(b).get_xmax()/curr_height;
         double vlo=1 - curr_bboxes_ptr->at(b).get_ymin()/curr_height;
         double vhi=1 - curr_bboxes_ptr->at(b).get_ymax()/curr_height;
      
         double alpha = 0.5 * double(max_image_width-curr_width)/
            max_image_height;
         double beta = 0.5 * double(max_image_height-curr_height)/
            max_image_height;
         double gamma = double(curr_height)/max_image_height;
         
         double Ulo = alpha + gamma * ulo;
         double Uhi = alpha + gamma * uhi;
         double Vlo = beta + gamma * vlo;
         double Vhi = beta + gamma * vhi;

         vector<threevector> bbox_vertices;
         bbox_vertices.push_back(threevector(Ulo,Vlo));
         bbox_vertices.push_back(threevector(Uhi,Vlo));
         bbox_vertices.push_back(threevector(Uhi,Vhi));
         bbox_vertices.push_back(threevector(Ulo,Vhi));
         bbox_vertices.push_back(threevector(Ulo,Vlo));
         osg::Vec4 uniform_color=colorfunc::get_OSG_color(
            curr_bboxes_ptr->at(b).get_color());

         bool force_display_flag = false;
         bool single_polyline_per_geode_flag = true;
         int n_text_messages = 1;


         PolyLine* bbox_PolyLine_ptr = 
            PolyLinesGroup_ptr->generate_new_PolyLine(
               bbox_vertices, uniform_color, force_display_flag, 
               single_polyline_per_geode_flag, n_text_messages);
         int PolyLine_ID = bbox_PolyLine_ptr->get_ID();

// Tie together IDs for bounding boxes and their corresponding
// OSG PolyLines:

         curr_bboxes_ptr->at(b).set_ID(PolyLine_ID);

// Store association between OSG PolyLine ID and (image_ID_str, b)
// pair:

         pair<string, int> P;
         P.first = image_ID_str;
         P.second = b;
         osg_bboxes_map[PolyLine_ID] = P;

         string attribute_key = "gender";
         string attribute_value = curr_bboxes_ptr->at(b).
            get_attribute_value(attribute_key);
         
         if(attribute_value.size() > 0 && attribute_value != "unknown")
         {
            PolyLinesGroup_ptr->display_PolyLine_attribute(
               PolyLine_ID, attribute_value);
         }

      } // loop over index b labeling bboxes for curr_image
   } // loop over index n labeling frame numbers
   AnimationController_ptr->set_curr_framenumber(0);

   cout << endl;
   cout << "******************************************************" << endl;
   cout << "Keyboard control notes:" << endl << endl;
   cout << "Note: Image window must be active before following keys will be recognized" 
        << endl << endl;
   cout << "'p' key toggles image player between 'play' and 'pause' modes " 
        << endl;
   cout << "'r' key reverses image's time direction" << endl;
   cout << "Right and left arrow keys step one image frame forward/backward"
        << endl;
   cout << "'a' key allows user to enter an annotation for current frame"
        << endl;
   cout << "'e' key exports all image annotations to output CSV file" 
        << endl;
   cout << "spacebar 'homes' image window to default settings" << endl;
   cout << endl;
   cout << "******************************************************" << endl;
   cout << "Mouse control notes:" << endl << endl;
   cout << "Left mouse button translates image frame in 2D" << endl;
   cout << "Center mouse button rotates image window in 2D" << endl;
   cout << "Right mouse button zooms image frame in/out" << endl;
   cout << "******************************************************" << endl;
   cout << endl;

   decorations.set_DataNode_ptr(movie_ptr->getGeode());

   root->addChild(decorations.get_OSGgroup_ptr());

// Attach the scene graph to the viewer:

   window_mgr_ptr->setSceneData(root);

// Create the windows and run the threads.  Viewer's realize method
// calls the CustomManipulator's home() method:

   window_mgr_ptr->realize();

   while( !window_mgr_ptr->done() )
   {
      window_mgr_ptr->process();
   }
}

