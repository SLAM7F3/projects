// ==========================================================================
// Sign_Recognizer class member function definitions
// ==========================================================================
// Last modified on 11/5/12; 11/6/12; 12/1/13; 6/7/14
// ==========================================================================

#include "astro_geo/Clock.h"
#include "image/extremal_regions_group.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "classification/signrecogfuncs.h"
#include "classification/sign_recognizer.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void sign_recognizer::initialize_member_objects()
{
   image_counter=0;
}

void sign_recognizer::allocate_member_objects()
{
//   cout << "inside sign_recognizer::allocate_member_objects()" << endl;

   camera_ptr=new camera();
   clock_ptr=new Clock();
   texture_rectangle_ptr=new texture_rectangle();
   binary_texture_rectangle_ptr=new texture_rectangle();
   regions_group_ptr=new extremal_regions_group();
   bright_regions_map_ptr=new extremal_regions_group::ID_REGION_MAP;
}		       

// ---------------------------------------------------------------------
sign_recognizer::sign_recognizer()
{
   initialize_member_objects();
   allocate_member_objects();
}

// Copy constructor:

sign_recognizer::sign_recognizer(const sign_recognizer& sr)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(sr);
}

sign_recognizer::~sign_recognizer()
{
   delete camera_ptr;
   delete clock_ptr;
   delete texture_rectangle_ptr;
   delete binary_texture_rectangle_ptr;
   delete regions_group_ptr;
   delete bright_regions_map_ptr;
}

// ---------------------------------------------------------------------
void sign_recognizer::docopy(const sign_recognizer& sr)
{
//   cout << "inside sign_recognizer::docopy()" << endl;
}

// Overload = operator:

sign_recognizer& sign_recognizer::operator= (const sign_recognizer& sr)
{
   if (this==&sr) return *this;
   docopy(sr);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const sign_recognizer& sr)
{
   outstream << endl;

//   cout << "inside sign_recognizer::operator<<" << endl;

   return outstream;
}

// ==========================================================================
// Initialization member functions
// ==========================================================================

void sign_recognizer::initialize_tank_sign_recognition()
{
   timefunc::initialize_timeofday_clock();
   initialize_tank_subdirectories();
   initialize_PointGrey_camera(501890);	
   initialize_relative_tank_position_file();
}

void sign_recognizer::initialize_PointGrey_camera(int camera_ID)
{
   PointGrey_camera_ID=camera_ID;
//   int PointGrey_camera_ID=-1;	// non-PointGrey camera
//   int PointGrey_camera_ID=501207;	// "Mark's" camera
//   int PointGrey_camera_ID=501208;	// "Pat's" camera
//   int PointGrey_camera_ID=501890;	// "Bryce's" camera
   signrecogfunc::initialize_PointGrey_camera_params(
      PointGrey_camera_ID,camera_ptr);
}

// ==========================================================================
// Tank sign recognition member functions
// ==========================================================================

void sign_recognizer::initialize_tank_subdirectories()
{
   input_images_subdir="./images/incoming_PointGrey_images/";

   tank_signs_subdir="./images/tank_signs/";
   archived_images_subdir=
      signrecogfunc::generate_timestamped_archive_subdir(
         tank_signs_subdir);
   output_subdir=tank_signs_subdir+"tank_sign_results/";
   filefunc::dircreate(output_subdir);
}

// ---------------------------------------------------------------------
// Initialize output relative tank position file:

void sign_recognizer::initialize_relative_tank_position_file()
{
   string output_filename=output_subdir+"rel_tank_posns.txt";
   filefunc::openfile(output_filename,output_stream);
   output_stream << "# Frame   Image name	    Time stamp		   Rel X    Rel Y    Rel Z" << endl;
   output_stream << endl;
}

void sign_recognizer::close_relative_tank_position_file()
{
   string output_filename=output_subdir+"rel_tank_posns.txt";
   filefunc::closefile(output_filename,output_stream);
}

// ---------------------------------------------------------------------     
// Boolean member function search_for_tank_sign() returns true if it
// finds a colored-checkerboard symbol within the input image.

bool sign_recognizer::search_for_tank_sign(string image_filename)
{
   report_processing_time();
   increment_image_counter();
//    int frame_number=get_image_counter();

   unsigned int xdim,ydim;
   imagefunc::get_image_width_height(image_filename,xdim,ydim);
   cout << "image_filename = " << image_filename << endl;

   texture_rectangle_ptr->reset_texture_content(image_filename);
   binary_texture_rectangle_ptr->reset_texture_content(image_filename);

// Find bright connected components:

   binary_texture_rectangle_ptr->convert_color_image_to_greyscale();
//   string greyscale_filename=output_subdir
//      +"greyscale_"+stringfunc::integer_to_string(frame_number,5)+".jpg";
//      binary_texture_rectangle_ptr->write_curr_frame(greyscale_filename);

   regions_group_ptr->destroy_all_regions(bright_regions_map_ptr);

   signrecogfunc::compute_connected_components_for_tank_sign_image(
      xdim,ydim,image_filename,bright_regions_map_ptr);
      
// Analyze color content of reduced bright connected components:

   vector<polygon> RYCP_polygons;
   tank_posn_rel_to_camera=threevector(
      NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);

   extremal_regions_group::ID_REGION_MAP::iterator bright_region_iter;
   for (bright_region_iter=bright_regions_map_ptr->begin(); 
        bright_region_iter != bright_regions_map_ptr->end();
        bright_region_iter++)
   {
      extremal_region* region_ptr=bright_region_iter->second;
      signrecogfunc::search_for_colored_checkerboard_in_bright_region(
         xdim,region_ptr,RYCP_polygons,camera_ptr,
         texture_rectangle_ptr,tank_posn_rel_to_camera);
   } // loop over reduced bright regions

   if (tank_posn_rel_to_camera.magnitude() < 0.5*POSITIVEINFINITY) 
   {
      export_RYCP_results(image_filename,RYCP_polygons);
      return true;
   }
   return false;
}

// ---------------------------------------------------------------------     
// Method export_RYCP_results()

void sign_recognizer::export_RYCP_results(
   string image_filename,const vector<polygon>& RYCP_polygons)
{
//   cout << "inside sign_recognizer::export_RYCP_results()" << endl;

   string RYCP_COMs_filename=output_subdir
      +"RYCP_COMS_"+stringfunc::integer_to_string(image_counter,5)+".png";

   texture_rectangle_ptr->reset_texture_content(image_filename);
   double thickness=1;
   videofunc::display_polygons(
      RYCP_polygons,texture_rectangle_ptr,16,thickness);
   texture_rectangle_ptr->write_curr_frame(RYCP_COMs_filename);

//   cout << "tank_posn_rel_to_camera = "
//        << tank_posn_rel_to_camera << endl;

   clock_ptr->set_time_based_on_local_computer_clock();
   string time_stamp=clock_ptr->YYYY_MM_DD_H_M_S();
   output_stream << image_counter << "  "
                 << filefunc::getbasename(image_filename) << "  "
                 << time_stamp << "  "
                 << tank_posn_rel_to_camera.get(0) << "  "
                 << tank_posn_rel_to_camera.get(1) << "  "
                 << tank_posn_rel_to_camera.get(2) << endl;

   JSON_string="{ \n";
   JSON_string += "      image_counter: "+stringfunc::number_to_string(
      image_counter)+", \n";
   JSON_string += "      image_filename: '"+image_filename+"', \n";
   JSON_string += "      time_stamp: '"+time_stamp+"', \n";
   JSON_string += "      tank_posn_rel_camera_X: "+
      stringfunc::number_to_string(tank_posn_rel_to_camera.get(0))+", \n";
   JSON_string += "      tank_posn_rel_camera_Y: "+
      stringfunc::number_to_string(tank_posn_rel_to_camera.get(1))+", \n";
   JSON_string += "      tank_posn_rel_camera_Z: "+
      stringfunc::number_to_string(tank_posn_rel_to_camera.get(2))+" \n";
   JSON_string += " } \n";

//   cout << "JSON_string = " << JSON_string << endl;
}

// ==========================================================================
// Input/output member functions
// ==========================================================================

void sign_recognizer::report_processing_time()
{
   if (image_counter >= 1)
   {
      double total_time=timefunc::elapsed_timeofday_time();
      cout << "TOTAL PROCESSING TIME = " << total_time << " secs = " 
           << total_time / 60.0 << " minutes" << endl;
      double avg_time_per_image=
         timefunc::elapsed_timeofday_time()/image_counter;
      cout << "***********************************************" << endl;
      cout << "AVERAGE TIME PER IMAGE = " << avg_time_per_image 
           << " secs" << " n_images = " << image_counter << endl;
      cout << "***********************************************" << endl;
   }
}

