// =========================================================================
// Object_Detector class member function definitions
// =========================================================================
// Last modified on 11/18/13; 11/19/13; 11/28/15
// =========================================================================

#include <iostream>

#include <dlib/svm_threaded.h>
#include <dlib/gui_widgets.h>
#include <dlib/array.h>
#include <dlib/image_keypoint.h>
#include <dlib/image_processing.h>
#include <dlib/cmd_line_parser.h>
#include <dlib/data_io.h>

#include "math/basic_math.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "image/imagefuncs.h"
#include "video/object_detector.h"
#include "video/videofuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ostream;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void object_detector::allocate_member_objects()
{
}

void object_detector::initialize_member_objects()
{
   num_threads=6;
   n_HOG_detectors=10*num_threads;
   next_HOG_detector_index=0;
   n_detected_objects=0;

   double mag_factor=3;
   max_xdim=mag_factor*640;
   max_ydim=mag_factor*480;
}		 

// ---------------------------------------------------------------------
object_detector::object_detector()
{
   initialize_member_objects();
   allocate_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

object_detector::object_detector(const object_detector& s)
{
//   cout << "inside object_detector copy constructor, this(object_detector) = " << this << endl;
   initialize_member_objects();
   allocate_member_objects();
   docopy(s);
}

object_detector::~object_detector()
{
//   cout << "inside side_detector destructor" << endl;
}

// ---------------------------------------------------------------------
void object_detector::docopy(const object_detector& s)
{
//   cout << "inside object_detector::docopy()" << endl;
//   cout << "this = " << this << endl;
}

// Overload = operator:

object_detector& object_detector::operator= (const object_detector& s)
{
//   cout << "inside object_detector::operator=" << endl;
//   cout << "this(object_detector) = " << this << endl;
   if (this==&s) return *this;
   docopy(s);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const object_detector& s)
{
   outstream << endl;
//   outstream << "Object_Detector ID = " << e.ID << endl;
   
   return outstream;
}

// =========================================================================
// HOG member functions
// =========================================================================

// Method import_image() loads the image specified by its input filename into
// a dlib::array2D object.  It then resizes the raw image so that its
// target width and height are sufficiently large to enable small HOG
// object detection.

void object_detector::import_image(
   string image_filename,dlib::array2d<dlib::rgb_pixel>& img)
   {
      dlib::load_image(img,image_filename.c_str());
   }

/*
void object_detector::import_and_resize_image(
   string image_filename,dlib::array2d<dlib::rgb_pixel>& img)
   {
      int width,height;
      imagefunc::get_image_width_height(image_filename,width,height);
      double x_scale_factor=max_xdim/width;
      double y_scale_factor=max_ydim/height;
      double scale_factor=sqrt(x_scale_factor*y_scale_factor);

      if (scale_factor > 0.9 && scale_factor < 1.1)
      {
         dlib::load_image(img,image_filename.c_str());
      }
      else
      {
         string resized_image_filename="/tmp/resized_image.jpg";
         videofunc::resize_image(
            image_filename,width,height,
            scale_factor*width,scale_factor*height,resized_image_filename);

         while (true)
         {
            if (filefunc::fileexist(resized_image_filename))
            {
               dlib::load_image(img, resized_image_filename.c_str());
               break;
            }
            else
            {
               usleep(50);
            }
         } // while loop
         

      }
   }
*/


/*
void object_detector::import_and_resize_image(
   string image_filename,dlib::array2d<dlib::rgb_pixel>& img)
   {
      dlib::array2d<dlib::rgb_pixel> orig_img;
      dlib::load_image(orig_img, image_filename.c_str());
      int orig_width=orig_img.nc();
      int orig_height=orig_img.nr();

      int mag_factor=3;
//      int mag_factor=4;
//      int mag_factor=5;
      int target_width=mag_factor*640;
      int target_height=mag_factor*480;
      
      double u_scalefactor=double(target_width)/orig_width;
      double v_scalefactor=double(target_height)/orig_height;
      int scalefactor=1+basic_math::mytruncate(
         basic_math::max(u_scalefactor,v_scalefactor));
//      cout << "Scalefactor = " << scalefactor << endl;
      double log2_scalefactor=log(double(scalefactor))/log(double(2.0));
//      cout << "log2_scalefactor = " << log2_scalefactor << endl;
//      int n_iters=basic_math::mytruncate(log2_scalefactor)+1;
      int n_iters=basic_math::mytruncate(log2_scalefactor);
//      cout << "n_iters = " << n_iters << endl;
      
      for (int n=0; n<n_iters; n++)
      {
//         cout << "n = " << n << endl;
         dlib::pyramid_down<2> pyr;
         dlib::pyramid_up(orig_img,img,pyr);
         img.swap(orig_img);
      }
   }
*/

// ---------------------------------------------------------------------
// Import trained HOG template file from disk:

void object_detector::import_detector(std::string HOG_template_filename)
{
   cout << "Imported HOG template filename = "
        << HOG_template_filename << endl;

   dlib::object_detector<image_scanner_type> HOG_detector;
   ifstream fin(HOG_template_filename.c_str(), ios::binary);
   dlib::deserialize(HOG_detector, fin);

   int n_templates=HOG_detector.num_detectors();
   cout << "Number of HOG templates in input detector = "
        << n_templates << endl;
   cout << "Number of separable filters="
        << dlib::num_separable_filters(HOG_detector) << endl;

   HOG_detectors.reserve(n_HOG_detectors);
   for (int i=0; i<n_HOG_detectors; i++)
   {
      HOG_detectors.push_back(HOG_detector);
      available_detector_map[i]=true;
   }
}

// ---------------------------------------------------------------------
// Run HOG detector on resized image:

void object_detector::HOG_filter(
   int image_index,dlib::array2d<dlib::rgb_pixel>& img,
   vector<fourvector>&  bbox_params)
{
   AVAILABLE_DETECTOR_MAP::iterator available_detector_iter;

   int HOG_detector_index=-1;
   while (true)
   {
      HOG_detector_index=get_next_HOG_detector_index();
      available_detector_iter=available_detector_map.find(HOG_detector_index);
      if (available_detector_iter->second==true)
      {
         available_detector_iter->second=false;
         break;
      }
      cout << "HOG_detector_index = " << HOG_detector_index << " is busy"
           << endl;
   }

   const vector<dlib::rectangle> obj_bboxes = 
      HOG_detectors[HOG_detector_index](img);
//   const vector<dlib::rectangle> obj_bboxes = HOG_detectors[0](img);
   available_detector_iter->second=true;

// Export object bboxes to output text file:

//   unsigned int width=img.nc();
   unsigned int height=img.nr();

   bbox_params.clear();
   for (unsigned int r=0; r<obj_bboxes.size(); r++)
   {
      dlib::rectangle curr_rect(obj_bboxes[r]);
      double Ulo=double(curr_rect.left())/height;
      double Uhi=double(curr_rect.right())/height;
      double Vlo=1-double(curr_rect.bottom())/height;
      double Vhi=1-double(curr_rect.top())/height;
      bbox_params.push_back(fourvector(Ulo,Uhi,Vlo,Vhi));
   } // loop over index r labeling bboxes

}

// ------------------------------------------------------------------------
// Member function find_all_HOG_objects()

void object_detector::find_all_HOG_objects(
   const vector<string>& image_filenames,ofstream& bbox_stream)
{
   for (unsigned int i=0; i<image_filenames.size(); i++)
   {
      if (i%20==0)
      {
         double progress_frac=double(i)/image_filenames.size();
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }

      dlib::array2d<dlib::rgb_pixel> img;
      import_image(image_filenames[i],img);

      std::vector<fourvector> bbox_params;
      HOG_filter(i,img,bbox_params);

      cout << i << "  "
           << filefunc::getbasename(image_filenames[i]) 
           << "  n_object_bboxes = " << bbox_params.size() << endl;
         
      for (unsigned int r=0; r<bbox_params.size(); r++)
      {
         bbox_stream << i << "  " << r << "  "
                     << bbox_params[r].get(0) << "  " 
                     << bbox_params[r].get(1) << "  " 
                     << bbox_params[r].get(2) << "  " 
                     << bbox_params[r].get(3) << "  " 
                     << endl;
      } // loop over index r labeling bbox for current image
   } // loop over index i labeling input images
}

// ------------------------------------------------------------------------
// On 5/21/13, Davis King reminded us that the following structure is
// completely independent of the sift_detector class!  So to call
// member functions of sift_detector, we must pass in a copy of *this
// or equivalently the "this" pointer:

struct function_object_extract_HOG_bboxes
{
   function_object_extract_HOG_bboxes( 
      vector<string>* image_filenames_ptr_,
      ofstream& bbox_stream_,
      object_detector* this_ptr_
      ) :
      image_filenames_ptr(image_filenames_ptr_),
         bbox_stream(bbox_stream_),
         this_ptr(this_ptr_) {}
      
      vector<string>* image_filenames_ptr;
      ofstream& bbox_stream;
      object_detector* this_ptr;
//      dlib::mutex m;

   void operator() (long i) const
   {
      if (i%20==0)
      {
         double progress_frac=double(i)/image_filenames_ptr->size();
         outputfunc::print_elapsed_and_remaining_time(progress_frac);
      }
      
      string curr_image_filename=image_filenames_ptr->at(i);

      dlib::array2d<dlib::rgb_pixel> img;
      this_ptr->import_image(curr_image_filename,img);
//      this_ptr->import_and_resize_image(curr_image_filename,img);

      std::vector<fourvector> bbox_params;
      this_ptr->HOG_filter(i,img,bbox_params);
      int n_bboxes=bbox_params.size();
//      this_ptr->increment_n_detected_objects(n_bboxes);
      
      cout << i << "  "
           << filefunc::getbasename(curr_image_filename)
           << "  n_object_bboxes = " << n_bboxes << endl;
         
      for (int r=0; r<n_bboxes; r++)
      {
         bbox_stream << curr_image_filename << "  " << r << "  "
                     << bbox_params[r].get(0) << "  "
                     << bbox_params[r].get(1) << "  "
                     << bbox_params[r].get(2) << "  "
                     << bbox_params[r].get(3) 
                     << endl;
      } // loop over index r labeling bbox for current image

   }
};

// ------------------------------------------------------------------------
// Member function parallel_find_HOG_objects()

void object_detector::parallel_find_HOG_objects(
   vector<string>& image_filenames,ofstream& bbox_stream)
{
   string banner="Extracting HOG object bboxes via parallel threads:";
   outputfunc::write_banner(banner);

   function_object_extract_HOG_bboxes funct(
      &image_filenames,bbox_stream,this);

   int n_images=image_filenames.size();
   dlib::parallel_for(num_threads, 0, n_images, funct);
}

// ---------------------------------------------------------------------
// Member function combine_HOG_templates() takes in a set of filenames
// which we assume correspond to equally sized HOG templates.
// Following some sample code written by Davis King, we extract
// weights from each input HOG template.  The weights are then fed
// into a combined HOG template which is exported to a new output
// binary file. 

string object_detector::combine_HOG_templates(
   const vector<string>& HOG_template_filenames)
{

   vector<dlib::matrix<double,0,1> > weights;
   for (unsigned int t=0; t<HOG_template_filenames.size(); t++)
   {
      dlib::object_detector<image_scanner_type> curr_HOG_detector;
      ifstream fin(HOG_template_filenames[t].c_str(), ios::binary);
      dlib::deserialize(curr_HOG_detector, fin);
      weights.push_back(curr_HOG_detector.get_w());
      HOG_detectors.push_back(curr_HOG_detector);
   } // loop over index t labeling input HOG template files

   dlib::object_detector<image_scanner_type> combined_detector(
      HOG_detectors.back().get_scanner(), 
      HOG_detectors.back().get_overlap_tester(), weights);
 
   string combined_detector_filename="object_detector_combined.dat";
   ofstream fout(combined_detector_filename.c_str(), ios::binary);
   dlib::serialize(combined_detector, fout);

   return combined_detector_filename;
}
