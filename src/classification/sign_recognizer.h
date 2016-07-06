// ==========================================================================
// Header file for sign_recognizer class
// ==========================================================================
// Last modified on 11/5/12
// ==========================================================================

#ifndef SIGN_RECOGNIZER_H
#define SIGN_RECOGNIZER_H

#include <iostream>
#include <string>
#include <vector>
#include "video/camera.h"
#include "image/extremal_regions_group.h"

class Clock;
class texture_rectangle;

class sign_recognizer
{

  public:

   sign_recognizer();
   sign_recognizer(const sign_recognizer& sr);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~sign_recognizer();
   sign_recognizer& operator= (const sign_recognizer& sr);
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const sign_recognizer& sr);

// Set and get member functions:

   void increment_image_counter();
   int get_image_counter() const;
   std::string get_tank_signs_subdir() const;
   std::string get_JSON_string() const;
   camera* get_camera_ptr();
   const camera* get_camera_ptr() const;

// Initialization member functions

   void initialize_tank_sign_recognition();
   void initialize_PointGrey_camera(int camera_ID);

// Tank sign recognition member functions:

   void initialize_tank_subdirectories();
   void initialize_relative_tank_position_file();
   void close_relative_tank_position_file();
   bool search_for_tank_sign(std::string image_filename);
   void export_RYCP_results(
      std::string image_filename,const std::vector<polygon>& RYCP_polygons);

// TOC12 sign recognition member functions:


// Input/output member functions

   void report_processing_time();

  private: 

   int PointGrey_camera_ID,image_counter;
   std::string input_images_subdir,archived_images_subdir,output_subdir;
   std::string tank_signs_subdir,JSON_string;
   std::ofstream output_stream;
   threevector tank_posn_rel_to_camera;
   camera* camera_ptr;
   Clock* clock_ptr;
   extremal_regions_group* regions_group_ptr;
   texture_rectangle *texture_rectangle_ptr,*binary_texture_rectangle_ptr;
   extremal_regions_group::ID_REGION_MAP* bright_regions_map_ptr;


   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const sign_recognizer& sr);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void sign_recognizer::increment_image_counter()
{
   image_counter++;
}

inline int sign_recognizer::get_image_counter() const
{
   return image_counter;
}

inline std::string sign_recognizer::get_tank_signs_subdir() const
{
   return tank_signs_subdir;
}

inline std::string sign_recognizer::get_JSON_string() const
{
   return JSON_string;
}

inline camera* sign_recognizer::get_camera_ptr()
{
   return camera_ptr;
}

inline const camera* sign_recognizer::get_camera_ptr() const
{
   return camera_ptr;
}

#endif  // sign_recognizer.h



