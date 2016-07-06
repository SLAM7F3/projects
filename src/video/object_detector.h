// ==========================================================================
// Header file for object_detector class
// ==========================================================================
// Last modified on 11/28/13; 11/29/13; 12/1/13
// ==========================================================================

#ifndef OBJECT_DETECTOR_H
#define OBJECT_DETECTOR_H

#include <fstream>
#include <string>
#include <vector>

// Q:  How do we forward-declare dlib::array2d

#include <dlib/array2d.h>

class fourvector;

class object_detector
{

  public:

// Q: What exactly is the impact of increasing/decreasing
// pyramid_levels?

//   const int pyramid_levels=12;
   typedef dlib::scan_fhog_pyramid<dlib::pyramid_down<10> > 
      image_scanner_type;

   object_detector();
   object_detector(const object_detector& s);
   ~object_detector();
   object_detector& operator= (const object_detector& s);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const object_detector& s);

// Set & get member functions:

   void set_num_threads(int n);
   void set_n_images(int n);
   int get_n_detected_objects() const;
   void increment_n_detected_objects(int delta_n);

// HOG member functions:

   void import_image(
      std::string image_filename,dlib::array2d<dlib::rgb_pixel>& img);
   void import_and_resize_image(
      std::string image_filename,dlib::array2d<dlib::rgb_pixel>& img);
   void import_detector(std::string HOG_template_filename);

   int get_next_HOG_detector_index();
   void HOG_filter(
      int image_index,dlib::array2d<dlib::rgb_pixel>& img,
      std::vector<fourvector>& bbox_params);
   void find_all_HOG_objects(
      const std::vector<std::string>& image_filenames,
      std::ofstream& bbox_stream);
   void parallel_find_HOG_objects(
      std::vector<std::string>& image_filenames,
      std::ofstream& bbox_stream);

   std::string combine_HOG_templates(
      const std::vector<std::string>& HOG_template_filenames);


  private:

   int num_threads,n_images;
   int n_HOG_detectors,next_HOG_detector_index;
   int n_detected_objects;
   double max_xdim,max_ydim;

//   dlib::object_detector<image_scanner_type> HOG_detector;
   std::vector<dlib::object_detector<image_scanner_type> > HOG_detectors;


   typedef std::map<int,bool> AVAILABLE_DETECTOR_MAP;
// independent int = HOG_detector index
// dependent bool = true if associated HOG_detector is available for use

   AVAILABLE_DETECTOR_MAP available_detector_map;


   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const object_detector& s);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void object_detector::set_num_threads(int n)
{
   num_threads=n;
}

inline void object_detector::set_n_images(int n)
{
   n_images=n;
}

inline int object_detector::get_next_HOG_detector_index()
{
   next_HOG_detector_index=(next_HOG_detector_index+1)%n_HOG_detectors;
   return next_HOG_detector_index;
}

inline int object_detector::get_n_detected_objects() const
{
   return n_detected_objects;
}

inline void object_detector::increment_n_detected_objects(int delta_n)
{
   n_detected_objects += delta_n;
}



#endif // object_detector.h

