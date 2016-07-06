// =========================================================================
// Sift_Detector class member function definitions
// =========================================================================
// Last modified on 4/3/14; 4/5/14; 4/11/15; 11/28/15
// =========================================================================

#include <map>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "dlib/threads.h"
#include <dlib/image_keypoint/draw_surf_points.h>
#include <dlib/image_io.h>
#include <dlib/image_keypoint.h>

#include "geometry/affine_transform.h"
#include "cluster/akm.h"
#include "kdtree/ann_analyzer.h"
#include "math/basic_math.h"
#include "math/binaryfuncs.h"
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "math/fourvector.h"
#include "geometry/geometry_funcs.h"
#include "geometry/homography.h"
#include "image/imagefuncs.h"
#include "math/ltduple.h"
#include "math/lttwovector.h"
#include "datastructures/map_unionfind.h"
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"
#include "video/photograph.h"
#include "video/photogroup.h"
#include "image/pngfuncs.h"
#include "video/RGB_analyzer.h"
#include "video/sift_detector.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"

#include "time/timefuncs.h"
#include "math/twovector.h"
#include "graphs/vptree.h"



using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::istream;
using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void sift_detector::allocate_member_objects()
{
   akm_map_ptr=new AKM_MAP;
   H_ptr=new homography();
   fundamental_ptr=new fundamental();
}

void sift_detector::initialize_member_objects()
{
   forward_feature_matching_flag=true;
   num_threads=6;
   max_n_features_to_consider_per_image=100000;
   sampson_error_flag=false;
   root_sift_matching_flag=true;
   perform_SOH_corner_angle_test_flag=perform_Hamming_distance_test_flag=
      perform_min_descriptor_entropy_test_flag=true;

   min_allowed_U=min_allowed_V=0;
   max_allowed_U=POSITIVEINFINITY;
   max_allowed_V=1;
   
   f_dims=12;
   n_images=0;

   photogroup_ptr=NULL;
   akm_ptr=NULL;

   ANN_ptr=NULL;
   inverse_ANN_ptr=NULL;

   feature_counter=n_duplicate_features=0;

// Sub-orientation histogram parameters:

   for (signed int d=0; d<8; d++)
   {
      double orientation=d*PI/4.0;
      sin_orientation.push_back(sin(orientation));
      cos_orientation.push_back(cos(orientation));
   } // loop over index d labeling 8 subregion orientations

   SOH_binsize=15*PI/180;
//   SOH_binsize=30*PI/180;
//   SOH_binsize=90*PI/180;
   max_quadruple_index=basic_math::round(2*PI/SOH_binsize);

   if (FLANN_flag)
   {
      akm_ptr=new akm(FLANN_flag);
   }
   else
   {
      initialize_ANN_analyzers();
   }
}		 

// ---------------------------------------------------------------------
void sift_detector::initialize_ANN_analyzers()
{
//   cout << "inside sift_detector::initialize_ANN_analyzers()" << endl;
   
   int dim=128;
   int n_nearest_neighbors=2;
//   int maxPts=10000;
//   int maxPts=15000;
   int maxPts=20000;

//   double eps=0;
   double eps=3;
//   cout << "Enter epsilon:" << endl;
//   cin >> eps;

   ANN_ptr=new ann_analyzer(dim,n_nearest_neighbors,maxPts,eps);
   inverse_ANN_ptr=new ann_analyzer(dim,n_nearest_neighbors,maxPts,eps);
}

// ---------------------------------------------------------------------
sift_detector::sift_detector(photogroup* photogroup_ptr,bool FLANN_flag)
{
   this->FLANN_flag=FLANN_flag;
   initialize_member_objects();
   allocate_member_objects();

   this->photogroup_ptr=photogroup_ptr;
   if (photogroup_ptr != NULL) 
   {
      n_images=photogroup_ptr->get_n_photos();
      for (unsigned int n=0; n<n_images; n++)
      {
         image_feature_indices.push_back(0);
      }
   }
}

// ---------------------------------------------------------------------
// Copy constructor:

sift_detector::sift_detector(const sift_detector& s)
{
//   cout << "inside sift_detector copy constructor, this(sift_detector) = " << this << endl;
   initialize_member_objects();
   allocate_member_objects();
   docopy(s);
}

sift_detector::~sift_detector()
{
//   cout << "inside side_detector destructor" << endl;
   delete akm_ptr;
   delete akm_map_ptr;
   delete ANN_ptr;
   delete inverse_ANN_ptr;
   delete H_ptr;
   delete fundamental_ptr;

   for (unsigned int i=0; i<image_feature_info.size(); i++)
   {
//      cout << "i = " << i << endl;
      destroy_allocated_features_for_specified_image(
         get_image_feature_info_ptr(i));
   }
}

// ---------------------------------------------------------------------
// Member function destroy_allocated_features_for_specified_image
// loops over all feature pairs for image i.  It destroys both the
// first *F_ptr and second *D_ptr components of each feature.  We need
// to call this method whenever we're done processing the features for
// an image prior to starting another one in order to avoid running
// out of memory!

void sift_detector::destroy_allocated_features_for_specified_image(
   vector<feature_pair>* currimage_feature_info_ptr)
{
//   cout << "inside sift_detector::destroy_allocated_features_for_specified_image()" << endl;

   if (currimage_feature_info_ptr==NULL) return;
   for (unsigned int j=0; j<currimage_feature_info_ptr->size(); j++)
   {
//      cout << "j = " << j << " size = " 
//           << currimage_feature_info_ptr->size() << endl;
      descriptor* F_ptr=currimage_feature_info_ptr->at(j).first;
      descriptor* D_ptr=currimage_feature_info_ptr->at(j).second;

      delete F_ptr;
      delete D_ptr;
   }
}

// ---------------------------------------------------------------------
void sift_detector::docopy(const sift_detector& s)
{
//   cout << "inside sift_detector::docopy()" << endl;
//   cout << "this = " << this << endl;
}

// Overload = operator:

sift_detector& sift_detector::operator= (const sift_detector& s)
{
//   cout << "inside sift_detector::operator=" << endl;
//   cout << "this(sift_detector) = " << this << endl;
   if (this==&s) return *this;
   docopy(s);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const sift_detector& s)
{
   outstream << endl;
//   outstream << "Sift_Detector ID = " << e.ID << endl;
   
   return outstream;
}

// =========================================================================
// Set & get member functions
// =========================================================================

/*
std::vector<unsigned long >*
sift_detector::get_binary_descriptor_left_ptr(int i)
{
   return &(binary_descriptor_left.at(i));
}

std::vector<unsigned long >*
sift_detector::get_binary_descriptor_right_ptr(int i)
{
   return &(binary_descriptor_right.at(i));
}
*/

std::vector<descriptor* >*
sift_detector::get_Dbinary_ptrs_ptr(int i)
{
   return &(Dbinary_ptrs.at(i));
}

vptree* sift_detector::get_vptree_ptr(int i)
{
   return vptree_ptrs.at(i);
}

// =========================================================================
// SIFT feature extraction member functions
// =========================================================================

// Member function add_image_feature_info() takes in image index i
// along with STL vector currimage_feature_info which is assumed to
// contain feature info for image i.  It pushes the contents of
// currimage_feature_info back onto the ith STL vector in
// image_feature_info.

void sift_detector::add_image_feature_info(
   int i,const vector<feature_pair>& currimage_feature_info)
{
//   cout << "inside sift_detector::add_image_feature_info()" << endl;

// If vector of vectors image_feature_info is empty, fill it with
// empty vectors:

   vector<feature_pair> empty_image_feature_info;
   if (get_image_feature_info().size()==0)
   {
      for (unsigned int i=0; i<n_images; i++)
      {
         image_feature_info.push_back(empty_image_feature_info);
      }
   }

// Loop over currimage_feature_info and push its entries onto ith
// vector in image_feature_info:

   for (unsigned int j=0; j<currimage_feature_info.size(); j++)
   {
      get_image_feature_info(i).push_back(currimage_feature_info[j]);
   }
}

// ---------------------------------------------------------------------
void sift_detector::destroy_image_feature_info(int i)
{
//   cout << "inside sift_detector::destroy_image_feature_info()" << endl;

   for (unsigned int j=0; j<get_image_feature_info(i).size(); j++)
   {
      feature_pair currimage_feature_pair=get_image_feature_info(i).
         at(j);
      delete currimage_feature_pair.first;
      delete currimage_feature_pair.second;
   }
   get_image_feature_info(i).clear();
}

// ---------------------------------------------------------------------
// Member function extract_SIFT_features() loops over each image 
// and calls Lowe's SIFT binary to extract feature ID, U, V, scale
// and orientation information along with 128-dimensional descriptors
// for each feature.  SIFT feature information for each image is
// stored into member STL vector of vectors image_feature_info.

void sift_detector::extract_SIFT_features(
   string sift_keys_subdir,bool delete_pgm_file_flag)
{
   string banner="Extracting SIFT features:";
   outputfunc::write_banner(banner);

   bool Lowe_SIFT_flag=true;
   vector<feature_pair> currimage_feature_info;
//   cout << "n_images = " << n_images << endl;
   
   for (unsigned int i=0; i<n_images; i++)
   {
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);
      string image_filename=photograph_ptr->get_filename();
      int image_width=photograph_ptr->get_xdim();
      int image_height=photograph_ptr->get_ydim();

      string sift_key_filename=generate_Lowe_keyfile(
         sift_keys_subdir,image_filename,delete_pgm_file_flag);
      parse_Lowe_features(
         Lowe_SIFT_flag,image_width,image_height,sift_key_filename,
         currimage_feature_info,i);
      add_image_feature_info(i,currimage_feature_info);

//      cout << "i = " << i 
//           << " currimage_feature_info.size() = " 
//           << currimage_feature_info.size() << endl;

//      export_feature_tracks(i);
   } // loop over index i labeling input images
}

// ---------------------------------------------------------------------
// This overloaded version of extract_SIFT_features() works with a set
// of input image filenames rather than a photogroup object.

void sift_detector::extract_SIFT_features(
   vector<string>& image_filenames,string sift_keys_subdir,
   bool delete_pgm_file_flag)
{
   string banner="Extracting SIFT features:";
   outputfunc::write_banner(banner);

   n_images=image_filenames.size();
   bool Lowe_SIFT_flag=true;
   vector<feature_pair> currimage_feature_info;
   for (unsigned int i=0; i<n_images; i++)
   {
      string image_filename=image_filenames[i];
      unsigned int image_width,image_height;
      imagefunc::get_image_width_height(
         image_filename,image_width,image_height);

      string sift_key_filename=generate_Lowe_keyfile(
         sift_keys_subdir,image_filename,delete_pgm_file_flag);
      parse_Lowe_features(
         Lowe_SIFT_flag,image_width,image_height,sift_key_filename,
         currimage_feature_info,i);
      add_image_feature_info(i,currimage_feature_info);

//      cout << "i = " << i 
//           << " currimage_feature_info.size() = " 
//           << currimage_feature_info.size() << endl;

//      export_feature_tracks(i);
   } // loop over index i labeling input images
}

// ---------------------------------------------------------------------

// On 5/21/13, Davis King reminded us that the following structure is
// completely independent of the sift_detector class!  So to call
// member functions of sift_detector, we must pass in a copy of *this
// or equivalently the "this" pointer:

struct function_object_extract_SIFT
{
   function_object_extract_SIFT( 
      sift_detector* this_ptr_,
      photogroup* photogroup_ptr_ , string subdir_, bool flag_ ) :
      this_ptr(this_ptr_),
      photogroup_ptr(photogroup_ptr_),
         sift_keys_subdir(subdir_), delete_pgm_file_flag(flag_)  {}
      
      bool delete_pgm_file_flag;
      string sift_keys_subdir;
      photogroup* photogroup_ptr;
      sift_detector* this_ptr;
//      dlib::mutex m;

      void operator() (long i) const
      {
         photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);
         string sift_key_filename=this_ptr->generate_Lowe_keyfile(
            sift_keys_subdir,photograph_ptr,delete_pgm_file_flag);
         cout << "sift_key_filename = " << sift_key_filename << endl;

         bool Lowe_SIFT_flag=true;
         vector<sift_detector::feature_pair> currimage_feature_info;
         this_ptr->parse_Lowe_features(
            Lowe_SIFT_flag,photograph_ptr->get_xdim(),
            photograph_ptr->get_ydim(),sift_key_filename,
            currimage_feature_info,i);

         cout << "i = " << i 
              << " currimage_feature_info.size() = " 
              << currimage_feature_info.size() << endl;

         this_ptr->add_image_feature_info(i,currimage_feature_info);

//         dlib::auto_mutex lock(m);
//         sum += vect[i];
      }
};

void sift_detector::parallel_extract_SIFT_features(
   string sift_keys_subdir,bool delete_pgm_file_flag)
{
   string banner="Extracting SIFT features via parallel threads:";
   outputfunc::write_banner(banner);

//   cout << "Enter number of threads:" << endl;
//   cin >> num_threads;

   function_object_extract_SIFT funct(
      this,photogroup_ptr,sift_keys_subdir,delete_pgm_file_flag);
   dlib::parallel_for(num_threads, 0, n_images, funct);
}

// ---------------------------------------------------------------------
// Member function compute_OpenCV_SIFT_features() calls OpenCV methods
// which compute UV image plane interest points as well as
// 128-dimensional SIFT descriptors.  On 9/26/12, we empirically found
// that this method runs slightly faster than Lowe's binary (which
// involves writing/reading text files).  But it may not yield as many
// good SIFT keypoints as Lowe's binary.

bool sift_detector::compute_OpenCV_SIFT_features(
   string image_filename,vector<feature_pair>& currimage_feature_info)
{
//   cout << "inside sift_detector::compute_OpenCV_SIFT_features()" 
//        << endl;

// This method needs reworking for Open CV 3.0.0

/*
   const cv::Mat img = cv::imread(image_filename.c_str(), 0); 

//   int xdim=img.cols;
   int ydim=img.rows;

   double threshold=0.04;
   double edge_threshold=12;

//   cout << "Enter edge_threshold" << endl;
//   cin >> edge_threshold;

// Detect 2D interest "keypoints":

   cv::SiftFeatureDetector detector(threshold,edge_threshold);

   std::vector<cv::KeyPoint> keypoints;
   detector.detect(img, keypoints);

// Compute 128-dim descriptors:

   cv::SiftDescriptorExtractor extractor;

   cv::Mat descriptors;
   extractor.compute( img, keypoints, descriptors );

//   int n_rows=descriptors.rows;
//   int n_columns=descriptors.cols;
//   cout << "n_rows = " << n_rows << " n_cols = " << n_columns << endl;

   currimage_feature_info.clear();

   for (unsigned int r=0; r<keypoints.size(); r++)
   {
      descriptor* F_ptr=new descriptor(f_dims);
      F_ptr->put(0,feature_counter);
      F_ptr->put(10,r);		// feature index for current pass
      F_ptr->put(11,-1);	// image ID

      double px=keypoints[r].pt.x;
      double py=keypoints[r].pt.y;
      double U=px/ydim;
      double V=1-py/ydim;
      F_ptr->put(1,U);
      F_ptr->put(2,V);

//      cout << "r = " << r << " U = " << U << " V = " << V << endl;

      double scale=1;
      double orientation=0;

      F_ptr->put(3,scale);
      F_ptr->put(4,orientation);

// 6th component of *F_ptr acts as counter indicating number of images
// in which feature exists:

      F_ptr->put(5,1);

      F_ptr->put(6,NEGATIVEINFINITY);
      F_ptr->put(7,NEGATIVEINFINITY);
      F_ptr->put(8,NEGATIVEINFINITY);

      d_dims=128;
      descriptor* D_ptr=new descriptor(d_dims);

      int D_sum=0;
      for (unsigned int d=0; d<d_dims; d++)
      {
         int curr_descrip=descriptors.at<float>(r,d);
         D_sum += curr_descrip;
         D_ptr->put(d,curr_descrip);
      }
      if (D_sum==0) continue;

      pair<descriptor*,descriptor*> P(F_ptr,D_ptr);
      currimage_feature_info.push_back(P);
      feature_counter++;

   } // loop over index r labeling keypoints
*/

   return true;
}

// ---------------------------------------------------------------------
// Member function parse_Lowe_features() parses SIFT feature keys
// and descriptors for input *photograph_ptr from a previously
// calculated SIFT keys file.  The values are stored within
// 9-dimensional *F_ptr and 128-dimensional *D_ptr descriptors.  This
// method returns STL vector currimage_feature_info containing pairs
// of these feature descriptor pointers (F_ptr,D_ptr). If the SIFT
// keyfile is successfully parsed, this boolean method returns true.

bool sift_detector::parse_Lowe_features(
   bool Lowe_SIFT_flag,int photo_xdim,int photo_ydim,string sift_keys_filename,
   vector<feature_pair>& currimage_feature_info,int image_ID)
{
//   cout << "inside sift_detector::parse_Lowe_features(photograph_ptr)" 
//        << endl;
//   cout << "sift_keys_filename = " << sift_keys_filename << endl;
//   cout << "photo_xdim= " << photo_xdim
//        << " photo_ydim = " << photo_ydim << endl;

   set_max_allowed_U(double(photo_xdim)/double(photo_ydim));

// If SIFT keys filename comes from Noah's bundler codes, they are
// likely gzipped.  So first check for .gz suffix and gunzip SIFT key
// filename if found:

   string suffix=stringfunc::suffix(sift_keys_filename);
   if (suffix=="gz")
   {
      string unix_cmd="gunzip "+sift_keys_filename;
      sysfunc::unix_command(unix_cmd);
      sift_keys_filename=sift_keys_filename.substr(
         0,sift_keys_filename.size()-3);
   }

// On 4/12/12 and again on 4/30/12, we learned the hard and painful
// way that sometimes SIFT key files generated by Lowe's binary are
// empty.  So explicitly check input file byte size to see if input
// file contains any information:

   long long file_size=filefunc::size_of_file_in_bytes(sift_keys_filename);
   if (file_size < 10) return false;

   ifstream input_stream;
   filefunc::openfile(sift_keys_filename,input_stream);

   unsigned int n_features;
   input_stream >> n_features >> d_dims;
//   cout << "n_features = " << n_features << " d_dims = " << d_dims
//        << endl;

// Limit number of features for current image to some reasonable upper
// bound:

   n_features=basic_math::min(n_features,max_n_features_to_consider_per_image);

// On 2/4/13, we explicitly saw that Lowe's SIFT binary sometimes
// returns multiple instances of the same interest point.  So we must
// explicitly check if an interest point already exists before adding
// to the list of extracted features:

   typedef map<twovector,int,lttwovector> INTEREST_POINTS_MAP;

// First independent var: UV feature coords for particular image
// Second dependent int = feature frequency

   INTEREST_POINTS_MAP interest_points_map;

/*
// Instantiate twoDarray *feature_counter_twoDarray_ptr that count
// number of parsed features within relatively coarse, quantized image
// cell bins:

   double Umax=double(photo_xdim)/double(photo_ydim);
//   int ydim=20;
//   int ydim=25;
   int ydim=30;
   int xdim=Umax*ydim;
   twoDarray* feature_counter_twoDarray_ptr=new twoDarray(xdim,ydim);
   feature_counter_twoDarray_ptr->clear_values();
   feature_counter_twoDarray_ptr->init_coord_system(0,Umax,0,1);

//   const int n_bin_features_threshold=5;
//   const int n_bin_features_threshold=10;
//   const int n_bin_features_threshold=15;
   const int n_bin_features_threshold=20;

//   int n_features_randomly_ignored=0;
*/

   float f0,f1,f2,f3;
   currimage_feature_info.clear();

   for (unsigned int f=0; f<n_features; f++)
   {
      input_stream >> f0 >> f1 >> f2 >> f3;

      double U,V;
      if (Lowe_SIFT_flag)	// Lowe's SIFT binary
      {
         U=f1/photo_ydim;
         V=f0/photo_ydim;
      }
      else			// ASIFT 
      {
         U=f0/photo_ydim;
         V=f1/photo_ydim;
      }
      V=1-V;

      float curr_descriptor;
      vector<double> descriptors;
      for (unsigned int j=0; j<d_dims; j++)
      {
         input_stream >> curr_descriptor;
         descriptors.push_back(curr_descriptor);
      }

      if (U < min_allowed_U || U > max_allowed_U ||
          V < min_allowed_V || V > max_allowed_V)
      {
         cout << "U = " << U 
              << " min_allowed_U = " << min_allowed_U
              << " max_allowed_U = " << max_allowed_U << endl;
         cout << "V = " << V 
              << " min_allowed_V = " << min_allowed_V
              << " max_allowed_V = " << max_allowed_V << endl;
//         outputfunc::enter_continue_char();
         continue;
      }

//      cout << "feature = " << f
//           << " x = " << f1
//           << " y = " << f0
//           << " scale = " << f2
//           << " orientation = " << f3
//           << endl;

// Make sure U,V doesn't already exist in interest_points_map!
      
      INTEREST_POINTS_MAP::iterator interest_points_map_iter=
         interest_points_map.find(twovector(U,V));
      if (interest_points_map_iter==interest_points_map.end())
      {
         interest_points_map[twovector(U,V)]=1;
      }
      else
      {
         interest_points_map_iter->second=interest_points_map_iter->second+1;
         continue;
      }

/*
// Check number of existing entries within image cell bin of 
// *feature_counter_twoDarray_ptr corresponding to current UV.  If
// many already exist that bin, randomly ignore new entry:

      int pu,pv;
      feature_counter_twoDarray_ptr->fast_XY_to_Z(U,V,pu,pv);
      pu=basic_math::max(pu,0);
      pv=basic_math::max(pv,0);
      pu=basic_math::min(pu,xdim-1);
      pv=basic_math::min(pv,ydim-1);
  
      int n_binned_features=feature_counter_twoDarray_ptr->get(pu,pv);
//      cout << " n_binned_features = " << n_binned_features << endl;

      if (n_binned_features > n_bin_features_threshold)
      {
         if (nrfunc::ran1() > 1.0/(n_binned_features-n_bin_features_threshold))
         {
            n_features_randomly_ignored++;
            continue;
         }
      }
      feature_counter_twoDarray_ptr->put(pu,pv,n_binned_features+1);
*/


      descriptor* F_ptr=new descriptor(f_dims);
      F_ptr->put(0,feature_counter++);

      if (image_ID >= 0 && image_feature_indices.size() > 0)
      {
         int curr_feature_index=image_feature_indices[image_ID];
         F_ptr->put(10,curr_feature_index);
         F_ptr->put(11,image_ID);
         curr_feature_index++;
         image_feature_indices[image_ID]=curr_feature_index;
      }
    
      F_ptr->put(1,U);
      F_ptr->put(2,V);
      F_ptr->put(3,f2);
      F_ptr->put(4,f3);

// 6th component of *F_ptr acts as counter indicating number of images
// in which feature exists:

      F_ptr->put(5,1);

// If current photograph's camera is calibrated, compute backprojected
// 3D ray corresponding to (U,V):

//      camera* camera_ptr=photograph_ptr->get_camera_ptr();
//      if (camera_ptr->get_calibration_flag())
//      {
//         threevector n_hat=camera_ptr->pixel_ray_direction(U,V);
//         F_ptr->put(6,n_hat.get(0));
//         F_ptr->put(7,n_hat.get(1));
//         F_ptr->put(8,n_hat.get(2));
//      }
//      else
      {
         F_ptr->put(6,NEGATIVEINFINITY);
         F_ptr->put(7,NEGATIVEINFINITY);
         F_ptr->put(8,NEGATIVEINFINITY);
      }

      double descriptor_sum=0;
      if (root_sift_matching_flag)
      {
         for (unsigned int d=0; d<d_dims; d++)
         {
            descriptor_sum += fabs(descriptors[d]);
         }
//      cout << "descriptor_sum = " << descriptor_sum << endl;
      }
      
      descriptor* D_ptr=new descriptor(d_dims);

// Note added on 2/10/13: R. Arandjelovic and A. Zisserman in their
// CVPR 2012 paper "Three things everyone should know to improve
// object retrieval" insist that L1-normalizing a 128-SIFT descriptor
// (i.e. divide by the sum of the 128 absolute values) followed by
// taking the element-wise square root of each 128 descriptor value
// yields better SIFT matching results than the conventional approach
// of Lowe.  After some testing, we believe this assertion is true for
// SIFT feature matching.  

// However, "root-SIFT" matching appears to yield inferior results for
// Affine-SIFT features than conventional "SIFT" matching !

      for (unsigned int d=0; d<d_dims; d++)
      {
         if (root_sift_matching_flag)
         {
            double root_sift=255*sqrt(descriptors[d]/descriptor_sum);	// L1
            D_ptr->put(d,root_sift);
         }
         else
         {
            D_ptr->put(d,descriptors[d]);
//            cout << "d = " << d << " descriptor = " << descriptors[d]
//                 << endl;
         }
      }
//      outputfunc::enter_continue_char();

      pair<descriptor*,descriptor*> P(F_ptr,D_ptr);
      currimage_feature_info.push_back(P);

   } // loop over index f labeling curr image's features

   filefunc::closefile(sift_keys_filename,input_stream);
//   delete feature_counter_twoDarray_ptr;

//   cout << "n_features = " << n_features << endl;
//   cout << "n_features_randomly_ignored = "
//        << n_features_randomly_ignored << endl;
//   outputfunc::enter_continue_char();

// Re-gzip input keys file if it was initially gzipped:

   if (suffix=="gz")
   {
      string unix_cmd="gzip "+sift_keys_filename;
      sysfunc::unix_command(unix_cmd);
   }

   return true;
}

// ---------------------------------------------------------------------
// Member function parse_Lowe_descriptors() takes in a SIFT keyfile 
// (which may be gzipped) generated by Lowe's binary.  It extracts
// just the 128-dimensional descriptors and NOT the U,V,scale,rotation
// metadata for each SIFT feature.  The descriptors from the SIFT
// keyfile are returned within *D_ptrs_ptr.  If the SIFT keyfile is
// successfully parsed, this boolean method returns true.

// We wrote this specialized method in April 2012 to speed up
// conversion of SIFT key text files to HDF5 binary format for image
// vocabulary generation purposes.

bool sift_detector::parse_Lowe_descriptors(
   string sift_keys_filename,vector<descriptor*>* D_ptrs_ptr)
{
//   cout << "inside sift_detector::parse_Lowe_descriptors(photograph_ptr)" 
//        << endl;
//   cout << "sift_keys_filename = " << sift_keys_filename << endl;

// If SIFT keys filename comes from Noah's bundler codes, they are
// likely gzipped.  So first check for .gz suffix and gunzip SIFT key
// filename if found:

   string suffix=stringfunc::suffix(sift_keys_filename);
   if (suffix=="gz")
   {
      string unix_cmd="gunzip "+sift_keys_filename;
      sysfunc::unix_command(unix_cmd);
      sift_keys_filename=sift_keys_filename.substr(
         0,sift_keys_filename.size()-3);
   }

   filefunc::ReadInfile(sift_keys_filename);

   if (suffix=="gz")
   {
      string unix_cmd="gzip "+sift_keys_filename;
      sysfunc::unix_command(unix_cmd);
   }

// On 4/12/12 and again on 4/30/12, we learned the hard and painful
// way that sometimes SIFT key files generated by Lowe's binary are
// empty.  So check filefunc::text_line.size() to see if input file
// contains any information:

   if (filefunc::text_line.size()==0) return false;

   int linenumber=0;
   vector<double> features_descriptors=stringfunc::string_to_numbers(
      filefunc::text_line[linenumber++]);
   unsigned int n_features=features_descriptors[0];
//   cout << "n_features = " << n_features << endl;

   for (unsigned int f=0; f<n_features; f++)
   {
      vector<double> feature_keys=stringfunc::string_to_numbers(
         filefunc::text_line[linenumber++]);
                                                 
//      cout << "feature = " << f
//           << " x = " << feature_keys[1]
//           << " y = " << feature_keys[0]
//           << " scale = " << feature_keys[2]
//           << " orientation = " << feature_keys[3]
//           << endl;

      string descriptor_line;
      for (unsigned int l=0; l<7; l++)
      {
         descriptor_line += filefunc::text_line[linenumber++]+" ";
      }
//      cout << "descriptor_line = " << descriptor_line << endl;
      vector<double> descriptors=stringfunc::string_to_numbers(
         descriptor_line);
//      outputfunc::enter_continue_char();

      d_dims=128;
      descriptor* D_ptr=new descriptor(d_dims);
      for (unsigned int d=0; d<d_dims; d++)
      {
         D_ptr->put(d,descriptors[d]);
      }
//      cout << "*D_ptr = " << *D_ptr << endl;

      D_ptrs_ptr->push_back(D_ptr);

   } // loop over index f labeling curr image's features

   return true;
}

// ---------------------------------------------------------------------
// This overloaded version of member function
// parse_Lowe_descriptors() parses a SIFT keys file
// generated by Lowe's binary or libsiftfast.  It extracts all
// descriptors from the input keys file and exports them as unsigned
// chars (bytes) to a binary output file.

bool sift_detector::parse_Lowe_descriptors(
   bool Lowe_SIFT_flag,string sift_keys_filename,string descriptors_filename)
{
//   cout << "inside sift_detector::parse_SIFT_descriptors()" << endl;
//   cout << "sift_keys_filename = " << sift_keys_filename << endl;
   
// If SIFT keys filename comes from Noah's bundler codes, they are
// likely gzipped.  So first check for .gz suffix and gunzip SIFT key
// filename if found:

   string suffix=stringfunc::suffix(sift_keys_filename);
   if (suffix=="gz")
   {
      string unix_cmd="gunzip "+sift_keys_filename;
      sysfunc::unix_command(unix_cmd);
      sift_keys_filename=sift_keys_filename.substr(
         0,sift_keys_filename.size()-3);
   }

// On 4/12/12 and again on 4/30/12, we learned the hard and painful
// way that sometimes SIFT key files generated by Lowe's binary are
// empty.  So explicitly check input file byte size to see if input
// file contains any information:

   long long file_size=filefunc::size_of_file_in_bytes(sift_keys_filename);
   if (file_size < 10) return false;

   ifstream input_stream;
   filefunc::openfile(sift_keys_filename,input_stream);

   unsigned int n_features;
   input_stream >> n_features >> d_dims;
//   cout << "n_features = " << n_features << " d_dims = " << d_dims
//        << endl;

   int curr_descriptor;
   float f0,f1,f2,f3;

   int n_bytes=n_features*d_dims;
   unsigned char* data_ptr=new unsigned char[n_bytes];
   int byte_counter=0;
   for (unsigned int f=0; f<n_features; f++)
   {
      input_stream >> f0 >> f1 >> f2 >> f3;
      for (unsigned int d=0; d<d_dims; d++)
      {
         input_stream >> curr_descriptor;
         data_ptr[byte_counter++]=
            stringfunc::ascii_integer_to_unsigned_char(curr_descriptor);
      }
   } // loop over index f labeling features imported from keyfile

   filefunc::ExportUnsignedChars(data_ptr,descriptors_filename,n_bytes);
   delete [] data_ptr;
   
   filefunc::closefile(sift_keys_filename,input_stream);

// Re-gzip input keys file if it was initially gzipped:

   if (suffix=="gz")
   {
      string unix_cmd="gzip "+sift_keys_filename;
      sysfunc::unix_command(unix_cmd);
   }

   return true;
}

// ---------------------------------------------------------------------
// Member function generate_SIFT_keyfiles() loops over each image within
// *photogroup_ptr.  It calls Lowe's SIFT binary on each image and
// writes out the results to keyfiles within sift_keys_subdir.

vector<string>& sift_detector::generate_SIFT_keyfiles(
   string sift_keys_subdir,bool delete_pgm_file_flag)
{
   string banner="Generating SIFT keyfiles:";
   outputfunc::write_banner(banner);

   filefunc::dircreate(sift_keys_subdir);

   for (unsigned int i=0; i<n_images; i++)
   {
      cout << "i = " << i << " n_images = " << n_images << endl;
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);
      string curr_sift_keys_filename=
         generate_Lowe_keyfile(sift_keys_subdir,photograph_ptr,
         delete_pgm_file_flag);
      sift_keys_filenames.push_back(curr_sift_keys_filename);
   } // loop over index i labeling input images
   return sift_keys_filenames;
}

// ---------------------------------------------------------------------
// Member function generate_Lowe_keyfile() takes in photograph
// *photograph_ptr and generates a PGM version of the input image.  It
// then calls Lowe's SIFT binary which creates a ".key" file
// containing extracted SIFT feature information. This method
// deletes the PGM image, gzips the keys file and returns the name of
// the keys file.

string sift_detector::generate_Lowe_keyfile(
   string sift_keys_subdir,photograph* photograph_ptr,
   bool delete_pgm_file_flag)
{
//   cout << "inside sift_detector::generate_Lowe_keyfile(photograph_ptr)" 
//        << endl;
   return generate_Lowe_keyfile(
      sift_keys_subdir,photograph_ptr->get_filename(),delete_pgm_file_flag);
}

// ---------------------------------------------------------------------
string sift_detector::generate_Lowe_keyfile(
   string sift_keys_subdir,string image_filename,bool delete_pgm_file_flag)
{
//   cout << "inside sift_detector::generate_Lowe_keyfile()" 
//        << endl;

   filefunc::dircreate(sift_keys_subdir);
//   cout << "sift_keys_subdir = " << sift_keys_subdir << endl;

// Create sift_keys_filename based upon image_filename:

   string basename=filefunc::getbasename(image_filename);
   string prefix=stringfunc::prefix(basename);
   string sift_keys_filename=sift_keys_subdir+prefix+".key";
//   cout << "sift_keys_filename = " << sift_keys_filename << endl;

   generate_Lowe_keyfile(
      delete_pgm_file_flag,sift_keys_filename,image_filename);
   return sift_keys_filename;
}

// ---------------------------------------------------------------------
// Note: On 4/12/12 and again on 4/30/12, we discovered the painful
// way that Lowe's binary can fail to produce non-empty output key files!

void sift_detector::generate_Lowe_keyfile(
   bool delete_pgm_file_flag,string sift_keys_filename,string image_filename)
{
//   cout << "inside sift_detector::generate_Lowe_keyfile()" << endl;

   string banner="Extracting SIFT features for "+image_filename;
   outputfunc::write_banner(banner);

// As of Feb 2013, we assume that if sift_keys_filename already
// exists, it does NOT need to be regenerated:

   if (filefunc::fileexist(sift_keys_filename))
   {
//      cout << "sift_keys_filename = " << sift_keys_filename
//           << " already exists" << endl;
      cout << "SIFT keys file already exists" << endl;
      return;
   }

// Check whether PGM version of image_filename exists.  If not,
// generate it via ImageMagick:

   string pgm_filename=stringfunc::prefix(image_filename)+".pgm";
   if (filefunc::fileexist(pgm_filename))
   {
   }
   else
   {
      pgm_filename=imagefunc::convert_image_to_pgm(image_filename);
   }

// As of 5/14/13, we no longer call Lowe's original binary.  Instead,
// we now call the much faster siftfast executable:

//   string unixcommandstr="Lowe_sift < "+pgm_filename+" > "+sift_keys_filename;
   string unixcommandstr="siftfast < "+pgm_filename+" > "+sift_keys_filename;
//   cout << "unix_cmd = " << unixcommandstr << endl;
   sysfunc::unix_command(unixcommandstr);

   if (delete_pgm_file_flag) filefunc::deletefile(pgm_filename);
}

// ---------------------------------------------------------------------
// For each image, print information for the first few features:

void sift_detector::print_features(unsigned int n_features_to_print)
{
   for (unsigned int i=0; i<n_images; i++)
   {
      cout << "============================================================"
           << endl;
      cout << "image i = " << i << endl;

      vector<feature_pair> currimage_feature_info=image_feature_info[i];

      cout << "currimage_feature_info.size() = "
           << currimage_feature_info.size() << endl;

      for (unsigned int f=0; f<n_features_to_print; f++)
      {
         descriptor* F_ptr=currimage_feature_info[f].first;
         descriptor* D_ptr=currimage_feature_info[f].second;

         cout << "feature = " << F_ptr->get(0)
              << " U = " << F_ptr->get(1)
              << " V = " << F_ptr->get(2)
              << " scale = " << F_ptr->get(3)
              << " orientation = " << F_ptr->get(4)
              << endl;
         cout << " nx = " << F_ptr->get(6)
              << " ny = " << F_ptr->get(7)
              << " nz = " << F_ptr->get(8) << endl;

         int counter=0;
         for (unsigned int r=0; r<7; r++)
         {
            unsigned int c_stop=20;
            if (r==6) c_stop=8;
            for (unsigned int c=0; c<c_stop; c++)
            {
               int renorm_descriptor=floor(0.5+512*D_ptr->get(counter++));
               cout << renorm_descriptor << " ";
            }
            cout << endl;
            if (r==6) cout << endl;
         } // loop over r index labeling rows of 20 columns
      } // loop over index f labeling features
   } // loop over index i labeling input images
}

// =========================================================================
// FREAK feature extraction and matching member functions
// =========================================================================

// Member function detect_OpenCV_keypoints() calls several OpenCV
// methods which compute UV image plane interest points.  Keypoint
// coordinates measured in non-integer pixels are returned within STL
// vector keypoints.

void sift_detector::detect_OpenCV_keypoints(
   string image_filename,vector<cv::KeyPoint>& keypoints)
{
   cout << "inside sift_detector::detect_OpenCV_keypoints()"  << endl;

// Comment out this method for Open CV 3.0.0

/*
   const cv::Mat img = cv::imread(image_filename.c_str(), 0); 
//   int xdim_=img.cols;
//   int ydim_=img.rows;

// Detect 2D interest "keypoints":

   double threshold=0.04;
   double edge_threshold=2;
   cv::SiftFeatureDetector sift_detector(threshold,edge_threshold);
   cv::SurfFeatureDetector surf_detector(400,4);
   cv::StarFeatureDetector star_detector;
   cv::MserFeatureDetector mser_detector;
   cv::GoodFeaturesToTrackDetector GFTT_detector;

   vector<cv::KeyPoint> sift_keypoints,surf_keypoints,star_keypoints,
      mser_keypoints,GFTT_keypoints;

   sift_detector.detect(img, sift_keypoints);

// SURF seems OK

   surf_detector.detect(img, surf_keypoints);

// Star detector definitely yields some mismatches

//   star_detector.detect(img, star_keypoints);

// MSER looks OK

   mser_detector.detect(img, mser_keypoints);	

// GFTT detector yields a few mismatches but several correct matches
// as well

   GFTT_detector.detect(img, GFTT_keypoints);

   for (unsigned int k=0; k<sift_keypoints.size(); k++)
   {
      keypoints.push_back(sift_keypoints[k]);
   }
   for (unsigned int k=0; k<surf_keypoints.size(); k++)
   {
      keypoints.push_back(surf_keypoints[k]);
   }
   for (unsigned int k=0; k<star_keypoints.size(); k++)
   {
      keypoints.push_back(star_keypoints[k]);
   }
   for (unsigned int k=0; k<mser_keypoints.size(); k++)
   {
      keypoints.push_back(mser_keypoints[k]);
   }
   for (unsigned int k=0; k<GFTT_keypoints.size(); k++)
   {
      keypoints.push_back(GFTT_keypoints[k]);
   }
*/

}

// ---------------------------------------------------------------------
// Member function detect_OpenCV_keypoints() calls OpenCV methods
// which compute UV image plane interest points.  Keypoint coordinates
// measured in non-integer pixels are returned within STL vectors
// keypoints1 and keypoints2. 

void sift_detector::detect_OpenCV_keypoints(
   string image1_filename,string image2_filename,
   vector<cv::KeyPoint>& keypoints1,vector<cv::KeyPoint>& keypoints2)
{
   cout << "inside sift_detector::detect_OpenCV_keypoints()" 
        << endl;


// Comment out this method for OpenCV 3.0.0

/*

   const cv::Mat img1 = cv::imread(image1_filename.c_str(), 0); 
//   int xdim_1=img1.cols;
//   int ydim_1=img1.rows;

   const cv::Mat img2 = cv::imread(image2_filename.c_str(), 0); 
//   int xdim_2=img2.cols;
//   int ydim_2=img2.rows;

// Detect 2D interest "keypoints":

   double threshold=0.04;
   double edge_threshold=12;
   cv::SiftFeatureDetector sift_detector(threshold,edge_threshold);
   cv::SurfFeatureDetector surf_detector(400,4);
   cv::StarFeatureDetector star_detector;
   cv::MserFeatureDetector mser_detector;
   cv::GoodFeaturesToTrackDetector GFTT_detector;

   vector<cv::KeyPoint> sift_keypoints1,surf_keypoints1,star_keypoints1,
      mser_keypoints1,GFTT_keypoints1;
   vector<cv::KeyPoint> sift_keypoints2,surf_keypoints2,star_keypoints2,
      mser_keypoints2,GFTT_keypoints2;

   sift_detector.detect(img1, sift_keypoints1);
   sift_detector.detect(img2, sift_keypoints2);

// SURF seems OK

   surf_detector.detect(img1, surf_keypoints1);
   surf_detector.detect(img2, surf_keypoints2);

// Star detector definitely yields some mismatches

//   star_detector.detect(img1, star_keypoints1);
//   star_detector.detect(img2, star_keypoints2);

// MSER looks OK

   mser_detector.detect(img1, mser_keypoints1);	
   mser_detector.detect(img2, mser_keypoints2);

// GFTT detector yields a few mismatches but several correct matches
// as well

   GFTT_detector.detect(img1, GFTT_keypoints1);
   GFTT_detector.detect(img2, GFTT_keypoints2);

   for (unsigned int k=0; k<sift_keypoints1.size(); k++)
   {
      keypoints1.push_back(sift_keypoints1[k]);
   }
   for (unsigned int k=0; k<surf_keypoints1.size(); k++)
   {
      keypoints1.push_back(surf_keypoints1[k]);
   }
   for (unsigned int k=0; k<star_keypoints1.size(); k++)
   {
      keypoints1.push_back(star_keypoints1[k]);
   }
   for (unsigned int k=0; k<mser_keypoints1.size(); k++)
   {
      keypoints1.push_back(mser_keypoints1[k]);
   }
   for (unsigned int k=0; k<GFTT_keypoints1.size(); k++)
   {
      keypoints1.push_back(GFTT_keypoints1[k]);
   }

   for (unsigned int k=0; k<sift_keypoints2.size(); k++)
   {
      keypoints2.push_back(sift_keypoints2[k]);
   }
   for (unsigned int k=0; k<surf_keypoints2.size(); k++)
   {
      keypoints2.push_back(surf_keypoints2[k]);
   }
   for (unsigned int k=0; k<star_keypoints2.size(); k++)
   {
      keypoints2.push_back(star_keypoints2[k]);
   }
   for (unsigned int k=0; k<mser_keypoints2.size(); k++)
   {
      keypoints2.push_back(mser_keypoints2[k]);
   }
   for (unsigned int k=0; k<GFTT_keypoints2.size(); k++)
   {
      keypoints2.push_back(GFTT_keypoints2[k]);
   }

*/

}

// ---------------------------------------------------------------------
// Member function extract_OpenCV_FREAK_features() imports a set of
// 2D OpenCV interest points within STL vector keypoints.
// It returns corresponding FREAK descriptors within output OpenCV
// matrix descriptors.

void sift_detector::extract_OpenCV_FREAK_features(
   string image_filename,vector<cv::KeyPoint>& keypoints,
   cv::Mat& descriptors)
{
// Comment out this method for OpenCV 3.0.0

/*

//   cout << "inside sift_detector::extract_OpenCV_FREAK_features()" 
//        << endl;

// Here is an example on how to use the descriptor presented in the
// following paper:
// A. Alahi, R. Ortiz, and P. Vandergheynst. FREAK: Fast Retina
// Keypoint. In IEEE Conference on Computer Vision and Pattern
// Recognition, 2012.
//
//	Copyright (C) 2011-2012  Signal processing laboratory 2, EPFL,
//	Kirell Benzi (kirell.benzi@epfl.ch),
//	Raphael Ortiz (raphael.ortiz@a3.epfl.ch),
//	Alexandre Alahi (alexandre.alahi@epfl.ch)
//	and Pierre Vandergheynst (pierre.vandergheynst@epfl.ch)

   const cv::Mat img = cv::imread(image_filename.c_str(), 0); 
//   int xdim=img.cols;
//   int ydim=img.rows;

// Compute FREAK descriptors:


// FREAK doesn't appear to be supported in OpenCV 3.0.0
   cv::FREAK extractor;
   extractor.compute(img,keypoints,descriptors);

//   cout << "descriptors.rows = " << descriptors.rows
//        << " descriptors.cols = " << descriptors.cols << endl;

*/

}

// ---------------------------------------------------------------------
// Member function raw_match_OpenCV_FREAK_features() calls OpenCV
// methods which compute UV image plane interest points as well as
// FREAK descriptors.  UV coordinates are returned within STL vectors
// keypoints1 and keypoints2.  FREAK descriptors are returned within
// STL vector matches.

void sift_detector::raw_match_OpenCV_FREAK_features(
   cv::Mat& descriptors1,cv::Mat& descriptors2,
   vector<cv::DMatch>& matches)
{

// Comment out this method for OpenCV 3.0.0

/*
//   cout << "inside sift_detector::raw_match_OpenCV_FREAK_features()" 
//        << endl;

// Here is an example on how to use the descriptor presented in the
// following paper:
// A. Alahi, R. Ortiz, and P. Vandergheynst. FREAK: Fast Retina
// Keypoint. In IEEE Conference on Computer Vision and Pattern
// Recognition, 2012.
//
//	Copyright (C) 2011-2012  Signal processing laboratory 2, EPFL,
//	Kirell Benzi (kirell.benzi@epfl.ch),
//	Raphael Ortiz (raphael.ortiz@a3.epfl.ch),
//	Alexandre Alahi (alexandre.alahi@epfl.ch)
//	and Pierre Vandergheynst (pierre.vandergheynst@epfl.ch)

// Compute FREAK matching using standard Hamming distance as
// BruteForceMatcher<Hamming> matcher;
// or the proposed cascade of hamming distance using SSSE3

// FREAK descriptors do not appear to be supported in OpenCV 3.0.0

//   cv::BruteForceMatcher<cv::Hamming> matcher;
//   matcher.match(descriptors1, descriptors2, matches);

//   cout << "Raw FREAK matches = " << matches.size() << endl;
//   outputfunc::enter_continue_char();

*/

}

// ---------------------------------------------------------------------
// Member function raw_match_OpenCV_FREAK_features() calls OpenCV
// methods which compute UV image plane interest points as well as
// FREAK descriptors.  UV coordinates are returned within STL vectors
// keypoints1 and keypoints2.  FREAK descriptors are returned within
// STL vector matches.

void sift_detector::raw_match_OpenCV_FREAK_features(
   string image1_filename,string image2_filename,
   vector<cv::KeyPoint>& keypoints1,vector<cv::KeyPoint>& keypoints2,
   vector<cv::DMatch>& matches)
{

// Comment out this method for OpenCV 3.0.0

/*
//   cout << "inside sift_detector::raw_match_OpenCV_FREAK_features()" 
//        << endl;

// Here is an example on how to use the descriptor presented in the
// following paper:
// A. Alahi, R. Ortiz, and P. Vandergheynst. FREAK: Fast Retina
// Keypoint. In IEEE Conference on Computer Vision and Pattern
// Recognition, 2012.
//
//	Copyright (C) 2011-2012  Signal processing laboratory 2, EPFL,
//	Kirell Benzi (kirell.benzi@epfl.ch),
//	Raphael Ortiz (raphael.ortiz@a3.epfl.ch),
//	Alexandre Alahi (alexandre.alahi@epfl.ch)
//	and Pierre Vandergheynst (pierre.vandergheynst@epfl.ch)

   const cv::Mat img1 = cv::imread(image1_filename.c_str(), 0); 
//   int xdim_1=img1.cols;
//   int ydim_1=img1.rows;

   const cv::Mat img2 = cv::imread(image2_filename.c_str(), 0); 
//   int xdim_2=img2.cols;
//   int ydim_2=img2.rows;

// FREAK descriptors do not appear to be supported in OpenCV 3.0.0

// Compute FREAK descriptors:

   cv::FREAK extractor;
   cv::Mat descriptors1,descriptors2;
   extractor.compute( img1, keypoints1, descriptors1 );
   extractor.compute( img2, keypoints2, descriptors2 );

//   cout << "descriptors1.rows = " << descriptors1.rows
//        << " descriptors1.cols = " << descriptors1.cols << endl;
//   cout << "descriptors2.rows = " << descriptors2.rows
//        << " descriptors2.cols = " << descriptors2.cols << endl;

// Compute FREAK matching using standard Hamming distance as
// BruteForceMatcher<Hamming> matcher;
// or the proposed cascade of hamming distance using SSSE3

   cv::BruteForceMatcher<cv::Hamming> matcher;
   matcher.match(descriptors1, descriptors2, matches);


//   cout << "Raw FREAK matches = " << matches.size() << endl;
//   outputfunc::enter_continue_char();

*/
}

// ---------------------------------------------------------------------
// Member function color_prune_FREAK_matches() takes in UV imageplane
// keypoints and FREAK descriptors.  For each candidate UV tiepoint
// pair, this method compares RGB values.  For video frames spaced
// close in time, we expect RGB values at genuinely matching keypoints
// to be fairly close.  So we eliminate any putative FREAK matches
// whose separation in RGB color space exceeds some threshold.  The
// surviving FREAK matches are returned within updated STL vectors
// keypoints_1 and keypoints_2.

void sift_detector::color_prune_FREAK_matches(
   vector<cv::KeyPoint>& keypoints_1,vector<cv::KeyPoint>& keypoints_2,
   vector<cv::DMatch>& matches,RGB_analyzer* RGB_analyzer_ptr,
   string image1_filename,string image2_filename,
   texture_rectangle* texture_rectangle_1_ptr,
   texture_rectangle* texture_rectangle_2_ptr)
{
   texture_rectangle_1_ptr->reset_texture_content(image1_filename);
   texture_rectangle_2_ptr->reset_texture_content(image2_filename);

   unsigned int n_tiepoints=matches.size();


   typedef map<int,pair<twovector,colorfunc::RGB> > FEATURE_MAP;
   FEATURE_MAP feature_1_map;
   FEATURE_MAP feature_2_map;
   FEATURE_MAP::iterator feature_1_iter,feature_2_iter;
    
   cout << "Number initial tiepoints = " << n_tiepoints << endl;
   for (unsigned int m=0; m<n_tiepoints; m++)
   {
      double px_1=keypoints_1[m].pt.x;
      double py_1=keypoints_1[m].pt.y;
      double u_1=px_1/texture_rectangle_1_ptr->getHeight();
      double v_1=1-py_1/texture_rectangle_1_ptr->getHeight();
      twovector uv_1(u_1,v_1);
       
      double px_2=keypoints_2[m].pt.x;
      double py_2=keypoints_2[m].pt.y;
      double u_2=px_2/texture_rectangle_2_ptr->getHeight();
      double v_2=1-py_2/texture_rectangle_2_ptr->getHeight();
      twovector uv_2(u_2,v_2);

      int feature_1_ID=matches[m].trainIdx;
      int feature_2_ID=matches[m].queryIdx;

      int Red_1,Green_1,Blue_1;
      texture_rectangle_1_ptr->get_pixel_RGB_values(
         px_1,py_1,Red_1,Green_1,Blue_1);
      colorfunc::RGB RGB_1(Red_1,Green_1,Blue_1);

      int Red_2,Green_2,Blue_2;
      texture_rectangle_2_ptr->get_pixel_RGB_values(
         px_2,py_2,Red_2,Green_2,Blue_2);
      colorfunc::RGB RGB_2(Red_2,Green_2,Blue_2);

      pair<twovector,colorfunc::RGB> P_1(uv_1,RGB_1);
      pair<twovector,colorfunc::RGB> P_2(uv_2,RGB_2);

      feature_1_iter=feature_1_map.find(feature_1_ID);
      if (feature_1_iter != feature_1_map.end()) continue;

      feature_2_iter=feature_2_map.find(feature_2_ID);
      if (feature_2_iter != feature_2_map.end()) continue;

      feature_1_map[feature_1_ID]=P_1;
      feature_2_map[feature_2_ID]=P_2;
       
   } // loop over index m labeling raw tieppoints

//   cout << "feature_1_map.size() = " << feature_1_map.size() << endl;
//   cout << "feature_2_map.size() = " << feature_2_map.size() << endl;

   keypoints_1.clear();
   keypoints_2.clear();

   int n_color_acceptances=0;
   int n_color_rejections=0;
//   const double max_color_separation=25;
//   const double max_color_separation=30;
//   const double max_color_separation=40;
   const double max_color_separation=100;
   for (feature_1_iter=feature_1_map.begin(); feature_1_iter !=
           feature_1_map.end(); feature_1_iter++)
   {
      int feature_1_ID=feature_1_iter->first;
      twovector uv_1=feature_1_iter->second.first;
      colorfunc::RGB RGB_1=feature_1_iter->second.second;

      feature_2_iter=feature_2_map.find(feature_1_ID);
      if (feature_2_iter==feature_2_map.end())
      {
//          cout << "Error: no tiepoint found!" << endl;
         continue;
      }
      else
      {
         int feature_2_ID=feature_2_iter->first;
         twovector uv_2=feature_2_iter->second.first;
         colorfunc::RGB RGB_2=feature_2_iter->second.second;

         double color_dist=colorfunc::color_distance(RGB_1,RGB_2);
         if (color_dist > max_color_separation) 
         {
            n_color_rejections++;
            continue;
         }
         else
         {
            n_color_acceptances++;
         }

// Store feature IDs within class_ID attribute of cv::KeyPoint
// objects:

         double size=1;
         double angle=-1;
         double response=0;
         int octave=0;
         cv::KeyPoint K1(
            uv_1.get(0),uv_1.get(1),size,angle,response,octave,feature_1_ID);
         cv::KeyPoint K2(
            uv_2.get(0),uv_2.get(1),size,angle,response,octave,feature_2_ID);

         keypoints_1.push_back(K1);
         keypoints_2.push_back(K2);

      } // feature_2_iter conditional
   } // loop over feature_1_iter

   int n_feature_pairs=n_color_rejections+n_color_acceptances;
   double color_rejection_frac=double(n_color_rejections)/
      double(n_feature_pairs);
   cout << "max_color_separation = " << max_color_separation << endl;
   cout << "n_color_acceptances = " << n_color_acceptances << endl;
   cout << "n_color_rejections = " << n_color_rejections << endl;
   cout << "color_rejection_frac = " << color_rejection_frac << endl;
//   outputfunc::enter_continue_char();

}

// =========================================================================
// Harris corner detector functions
// =========================================================================

// Member function extract_harris_corners() takes in an image within
// *texture_rectangle_ptr which is assumed to also exist in
// *corners_texture_rectangle_ptr.  On the other hand,
// *edges_texture_rectangle_ptr is assumed to have initialized with
// some greyscale version of the input image.  This method computes
// squared products of horizontal and vertical image derivatives at
// each pixel location in the image.  It next computes response R of
// the Harris corner detector at each pixel.  If R exceeds input
// threshold R_min, the pixel is declared to be a corner.  Corner
// pixel locations are returned within an STL vector of integer pairs.

vector<cv::KeyPoint> sift_detector::extract_harris_corners(
   double R_min,string image_filename,
   texture_rectangle* texture_rectangle_ptr,
   texture_rectangle* edges_texture_rectangle_ptr,
   texture_rectangle* corners_texture_rectangle_ptr,
   twoDarray* xderiv_twoDarray_ptr,twoDarray* yderiv_twoDarray_ptr,
   twoDarray* xxderiv_twoDarray_ptr,twoDarray* xyderiv_twoDarray_ptr,
   twoDarray* yyderiv_twoDarray_ptr)
{
   texture_rectangle_ptr->reset_texture_content(image_filename);
   edges_texture_rectangle_ptr->reset_texture_content(image_filename);
   corners_texture_rectangle_ptr->reset_texture_content(image_filename);

//   edges_texture_rectangle_ptr->convert_color_image_to_greyscale(); 
   edges_texture_rectangle_ptr->convert_color_image_to_luminosity();
   string greyscale_filename="greyscale.jpg";
   edges_texture_rectangle_ptr->write_curr_frame(greyscale_filename);

   texture_rectangle_ptr->refresh_ptwoDarray_ptr();
   twoDarray* ptwoDarray_ptr=texture_rectangle_ptr->get_ptwoDarray_ptr();
         
   if (xderiv_twoDarray_ptr==NULL)
   {
      xderiv_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
      yderiv_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
      xxderiv_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
      xyderiv_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
      yyderiv_twoDarray_ptr=new twoDarray(ptwoDarray_ptr);
   }

   vector<cv::KeyPoint> corner_pixel_keypoints=extract_harris_corners(
      R_min,texture_rectangle_ptr,edges_texture_rectangle_ptr,
      corners_texture_rectangle_ptr,
      xderiv_twoDarray_ptr,yderiv_twoDarray_ptr,
      xxderiv_twoDarray_ptr,xyderiv_twoDarray_ptr,yyderiv_twoDarray_ptr);

   return corner_pixel_keypoints;
}

// ---------------------------------------------------------------------
// Member function extract_harris_corners() takes in an image within
// *texture_rectangle_ptr which is assumed to also exist in
// *corners_texture_rectangle_ptr.  On the other hand,
// *edges_texture_rectangle_ptr is assumed to have initialized with
// some greyscale version of the input image.  This method computes
// squared products of horizontal and vertical image derivatives at
// each pixel location in the image.  It next computes response R of
// the Harris corner detector at each pixel.  If R exceeds input
// threshold R_min, the pixel is declared to be a corner.  Corner
// pixel locations are returned within an STL vector of integer pairs.

vector<cv::KeyPoint> sift_detector::extract_harris_corners(
   double R_min,texture_rectangle* texture_rectangle_ptr,
   texture_rectangle* edges_texture_rectangle_ptr,
   texture_rectangle* corners_texture_rectangle_ptr,
   twoDarray* xderiv_twoDarray_ptr,twoDarray* yderiv_twoDarray_ptr,
   twoDarray* xxderiv_twoDarray_ptr,twoDarray* xyderiv_twoDarray_ptr,
   twoDarray* yyderiv_twoDarray_ptr)
{

   twoDarray* ptwoDarray_ptr=texture_rectangle_ptr->get_ptwoDarray_ptr();
   ptwoDarray_ptr->set_deltax(1);
   ptwoDarray_ptr->set_deltay(1);
   int xdim=ptwoDarray_ptr->get_xdim();
   int ydim=ptwoDarray_ptr->get_ydim();

   double spatial_resolution=0.25;
   imagefunc::compute_x_y_deriv_fields(
      spatial_resolution,ptwoDarray_ptr,
      xderiv_twoDarray_ptr,yderiv_twoDarray_ptr);

// Compute products of derivs at every pixel:

   for (int py=0; py<ydim; py++)
   {
      for (int px=0; px<xdim; px++)
      {
         double Ix=xderiv_twoDarray_ptr->get(px,py);
         double Iy=yderiv_twoDarray_ptr->get(px,py);
         xxderiv_twoDarray_ptr->put(px,py,Ix*Ix);
         xyderiv_twoDarray_ptr->put(px,py,Ix*Iy);
         yyderiv_twoDarray_ptr->put(px,py,Iy*Iy);
      } // loop over px 
   } // loop over py

   double dx=1;
   double sigma=1;
   double e_folding_distance=3;
   genmatrix* gaussian_filter_ptr=filterfunc::gaussian_2D_filter(
      dx,sigma,e_folding_distance);
//   cout << "gaussian_filter = " << *gaussian_filter_ptr << endl;
//   int n_rows=gaussian_filter_ptr->get_mdim();
   int n_columns=gaussian_filter_ptr->get_ndim();
//   cout << "n_rows = " << n_rows << " n_columns = " << n_columns << endl;

   int n_offset=n_columns/2;
   int px_start=n_offset;
   int px_stop=xdim-n_offset;
   int py_start=n_offset;
   int py_stop=ydim-n_offset;
   
   int n_corner_pixels=0;
   double kappa=0.05;
   genmatrix M(2,2);

   vector<cv::KeyPoint> corner_pixel_keypoints;
   for (int py=py_start; py<py_stop; py++)
   {
      for (int px=px_start; px<px_stop; px++)
      {
         M.clear_matrix_values();

         for (int i=-n_offset; i<=n_offset; i++)
         {
            int qx=px+i;
            for (int j=-n_offset; j<=n_offset; j++)
            {
               double w=gaussian_filter_ptr->get(i,j);

               int qy=py+j;
               double xxderiv=M.get(0,0)+w*xxderiv_twoDarray_ptr->get(qx,qy);
               double xyderiv=M.get(0,1)+w*xyderiv_twoDarray_ptr->get(qx,qy);
               double yyderiv=M.get(1,1)+w*yyderiv_twoDarray_ptr->get(qx,qy);
               M.put(0,0,xxderiv);
               M.put(0,1,xyderiv);
               M.put(1,0,xyderiv);
               M.put(1,1,yyderiv);
            } // loop over j index
         } // loop over i index

         double detM=M.get(0,0)*M.get(1,1)-M.get(0,1)*M.get(1,0);
         double traceM=M.get(0,0)+M.get(1,1);
         double R=detM-kappa*sqr(traceM);
         
         if (R > R_min)
         {
            corners_texture_rectangle_ptr->set_pixel_RGB_values(
               px,py,255,0,0);
            n_corner_pixels++;
//            cout << "px = " << px << " py = " << py << " R = " << R << endl;

            double keypoint_neighborhood_size=1;
            cv::KeyPoint curr_keypoint(px,py,keypoint_neighborhood_size);
            corner_pixel_keypoints.push_back(curr_keypoint);
         }

      } // loop over px
   } // loop over py

   cout << "n_corner_pixels = " << n_corner_pixels << endl;
   delete gaussian_filter_ptr;

   return corner_pixel_keypoints;
}

// =========================================================================
// Affine-SIFT (ASIFT) member functions
// =========================================================================

// Member function extract_ASIFT_features()

void sift_detector::extract_ASIFT_features(string asift_keys_subdir)
{
   string banner="Extracting AFFINE-SIFT features:";
   outputfunc::write_banner(banner);

   filefunc::dircreate(asift_keys_subdir);
//   cout << "image_feature_info.size() = " << image_feature_info.size() << endl;

   vector<feature_pair> currimage_feature_info;
   for (unsigned int i=0; i<n_images; i++)
   {
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);
      string image_filename=photograph_ptr->get_filename();
//      cout << "image_filename = " << image_filename << endl;
      string asift_keys_filename=filefunc::replace_suffix(
         image_filename,"key");
      cout << "asift_keys_filename = " << asift_keys_filename << endl;

      string asift_features_filename=extract_asift_descriptors(
         image_filename,asift_keys_filename,asift_keys_subdir);

      bool Lowe_SIFT_flag=false;
      parse_Lowe_features(
         Lowe_SIFT_flag,photograph_ptr->get_xdim(),
         photograph_ptr->get_ydim(),asift_features_filename,
         currimage_feature_info,i);
      add_image_feature_info(i,currimage_feature_info);

      cout << "i = " << i 
           << " currimage_feature_info.size() = " 
           << currimage_feature_info.size() << endl;
      cout << "image_feature_info(i).size() = "
           << get_image_feature_info(i).size() << endl;

   } // loop over index i labeling input images
}

// ---------------------------------------------------------------------
// On 5/21/13, Davis King reminded us that the following structure is
// completely independent of the sift_detector class!  So to call
// member functions of sift_detector, we must pass in a copy of *this
// or equivalently the "this" pointer:

struct function_object_extract_ASIFT
{
   function_object_extract_ASIFT( 
      sift_detector* this_ptr_, 
      photogroup* photogroup_ptr_ , string subdir_ ) :
      this_ptr(this_ptr_), photogroup_ptr(photogroup_ptr_),
         asift_keys_subdir(subdir_) {}

      string asift_keys_subdir;
      photogroup* photogroup_ptr;
      sift_detector* this_ptr;
//      dlib::mutex m;

      void operator() (long i) const
      {
         photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);
         string image_filename=photograph_ptr->get_filename();
         string asift_keys_filename=filefunc::replace_suffix(
            image_filename,"key");

         string asift_features_filename=this_ptr->extract_asift_descriptors(
            image_filename,asift_keys_filename,asift_keys_subdir);
         cout << "asift_features_filename = " << asift_features_filename 
              << endl;

         bool Lowe_SIFT_flag=false;
         vector<sift_detector::feature_pair> currimage_feature_info;
         this_ptr->parse_Lowe_features(
            Lowe_SIFT_flag,photograph_ptr->get_xdim(),
            photograph_ptr->get_ydim(),asift_features_filename,
            currimage_feature_info,i);

         cout << "i = " << i 
              << " currimage_feature_info.size() = " 
              << currimage_feature_info.size() << endl;

         this_ptr->add_image_feature_info(i,currimage_feature_info);
      }
};

void sift_detector::parallel_extract_ASIFT_features(string asift_keys_subdir)
{
   string banner="Extracting ASIFT features via parallel threads:";
   outputfunc::write_banner(banner);

   filefunc::dircreate(asift_keys_subdir);

//   cout << "Enter number of threads:" << endl;
//   cin >> num_threads;

   function_object_extract_ASIFT funct(
      this,photogroup_ptr,asift_keys_subdir);
   dlib::parallel_for(num_threads, 0, n_images, funct);
}

// ---------------------------------------------------------------------
// Member function extract_asift_descriptors() takes in a PNG image
// file.  It calls the Affine-SIFT executable
// '/usr/local/bin/extract_ASIFT' which exports ASIFT descriptors to
// the specified output text file.

string sift_detector::extract_asift_descriptors(
   string image_filename,string asift_keys_filename,
   string asift_keys_subdir)
{
//   cout << "inside sift_detector::extract_asift_descriptors()" << endl;

// As of Feb 2013, we assume that if asift keys file already
// exists in asift_keys_subdir, it does NOT need to be regenerated:

//   cout << "asift_keys_filename = " << asift_keys_filename << endl;
   string asift_features_filename=asift_keys_subdir+
      filefunc::getbasename(asift_keys_filename);
   if (filefunc::fileexist(asift_features_filename))
   {
      cout << "asift_keys_filename = " << asift_features_filename
           << " already exists in " << asift_keys_subdir << endl;
      return asift_features_filename;
   }

// Check whether PNG version of image_filename exists.  If not,
// generate it via ImageMagick:

   string prefix=stringfunc::prefix(image_filename);
   string png_filename=prefix+".png";
   if (filefunc::fileexist(png_filename))
   {
   }
   else
   {
      png_filename=pngfunc::convert_image_to_PNG(image_filename);
   }

   string unix_cmd=
      "extract_ASIFT "+png_filename+" "+asift_keys_filename;
   sysfunc::unix_command(unix_cmd);

   unix_cmd="mv "+asift_keys_filename+" "+asift_keys_subdir;
   sysfunc::unix_command(unix_cmd);

   return asift_features_filename;
}

// =========================================================================
// Oxford detector-descriptor feature extraction member functions
// =========================================================================

// Member function extract_Oxford_features()

void sift_detector::extract_Oxford_features(
   string detector_name,string Oxford_keys_subdir,
   bool delete_pgm_file_flag)
{
   string banner="Extracting Oxford "+detector_name+" features:";
   outputfunc::write_banner(banner);

   filefunc::dircreate(Oxford_keys_subdir);
//   cout << "image_feature_info.size() = " << image_feature_info.size() << endl;

   vector<feature_pair> currimage_feature_info;
   for (unsigned int i=0; i<n_images; i++)
   {
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);
      string image_filename=photograph_ptr->get_filename();
//      string image_basename=filefunc::getbasename(image_filename);
//      string image_subdir=filefunc::getdirname(image_filename);

//      string Oxford_keys_filename=filefunc::replace_suffix(
//         image_basename,"key");
//      cout << "Oxford_keys_filename = " << Oxford_keys_filename << endl;

      string Oxford_keys_filename=generate_detector_descriptor_keyfile(
         detector_name,Oxford_keys_subdir,image_filename,
         delete_pgm_file_flag);

      parse_detector_descriptor_features(
         photograph_ptr,Oxford_keys_filename,currimage_feature_info);
      add_image_feature_info(i,currimage_feature_info);

//      cout << "i = " << i 
//           << " currimage_feature_info.size() = " 
//           << currimage_feature_info.size() << endl;
//      cout << "image_feature_info(i).size() = "
//           << get_image_feature_info(i).size() << endl;

   } // loop over index i labeling input images
}

// ---------------------------------------------------------------------
vector<string>& sift_detector::generate_detector_descriptor_keyfiles(
   string detector_name,string sift_keys_subdir,bool delete_pgm_file_flag)
{
   string banner="Generating "+detector_name+" keyfiles:";
   outputfunc::write_banner(banner);

   filefunc::dircreate(sift_keys_subdir);

   for (unsigned int i=0; i<n_images; i++)
   {
      cout << "i = " << i << " n_images = " << n_images << endl;
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);
      string image_filename=photograph_ptr->get_filename();

      string curr_sift_keys_filename=
         generate_detector_descriptor_keyfile(
            detector_name,sift_keys_subdir,image_filename,
            delete_pgm_file_flag);
      sift_keys_filenames.push_back(curr_sift_keys_filename);
   } // loop over index i labeling input images
   return sift_keys_filenames;
}

// ---------------------------------------------------------------------
string sift_detector::generate_detector_descriptor_keyfile(
   string detector_name,string sift_keys_subdir,string image_filename,
   bool delete_pgm_file_flag)
{
//   cout << "inside sift_detector::generate_detector_descriptor_keyfile(photograph_ptr)" 
//        << endl;

   string banner="Extracting "+detector_name+" features for "+image_filename;
   outputfunc::write_banner(banner);

   string prefix=stringfunc::prefix(image_filename);
   string pgm_filename=prefix+".pgm";
   string base_pgm_filename=filefunc::getbasename(pgm_filename);
   string base_pgm_prefix=stringfunc::prefix(base_pgm_filename);
   string sift_keys_filename=sift_keys_subdir+base_pgm_prefix+".key";

//   cout << "sift_keys_filename = " << sift_keys_filename << endl;

// As of April 2013, we assume that if sift_keys_filename already
// exists, we do NOT need to compute it again:

   if (filefunc::fileexist(sift_keys_filename))
   {
      cout << "sift_keys_filename = " << sift_keys_filename
           << " already exists in " << sift_keys_subdir << endl;
      return sift_keys_filename;
   }

// Check whether PGM version of image_filename exists.  If not,
// generate it via ImageMagick:

   if (filefunc::fileexist(pgm_filename))
   {
//      cout << "pgm_filename = " << pgm_filename << " already exists"
//           << endl;
   }
   else
   {
      pgm_filename=imagefunc::convert_image_to_pgm(image_filename);
   }

//   string detector="-harlap";       
//   string detector="-heslap"; 
//   string detector="-haraff"; 
//   string detector="-hesaff"; // lowe ratio < 0.85; F product < 0.01; OK
//   string detector="-harhes";
//   string detector="-sedgelap";
//   string detector="-harThres";
//   string detector="-hesThres";
//   string detector="-edgeLThres";
//   string detector="-edgeHThres";

   string unixcommandstr=
      "extract_features_64bit.ln -"+detector_name
      +" -sift -i "+pgm_filename+" -o1 "+sift_keys_filename;
//   cout << "unixcmd = " << unixcommandstr << endl;
   sysfunc::unix_command(unixcommandstr);

   if (delete_pgm_file_flag) filefunc::deletefile(pgm_filename);
   return sift_keys_filename;
}

// ---------------------------------------------------------------------
void sift_detector::parse_detector_descriptor_features(
   photograph* photograph_ptr,string sift_keys_filename,
   vector<feature_pair>& currimage_feature_info)
{
//   cout << "inside sift_detector::parse_detector_descriptor_features(photograph_ptr)" 
//        << endl;
//   cout << "photo filename = " << photograph_ptr->get_filename() << endl;
//   cout << "sift_keys_filename = " << sift_keys_filename << endl;

   filefunc::ReadInfile(sift_keys_filename);

   int linenumber=0;
   unsigned int d_dims=stringfunc::string_to_number(
      filefunc::text_line[linenumber++]);
   unsigned int n_features=stringfunc::string_to_number(
      filefunc::text_line[linenumber++]);

//   cout << "n_features = " << n_features << endl;

   int ydim=photograph_ptr->get_ydim();   
   currimage_feature_info.clear();
   for (unsigned int f=0; f<n_features; f++)
   {
      descriptor* F_ptr=new descriptor(f_dims);
      F_ptr->put(0,feature_counter++);
      F_ptr->put(10,f);
      F_ptr->put(11,photograph_ptr->get_ID());

      vector<double> descriptors=stringfunc::string_to_numbers(
         filefunc::text_line[linenumber++]);
                                                 
//      cout << "feature = " << f
//           << " pu = " << descriptors[0]
//           << " pv = " << descriptors[1]
//           << endl;
      double U=descriptors[0]/ydim;
      double V=descriptors[1]/ydim;
      V=1-V;
      
      if (U < min_allowed_U || U > max_allowed_U ||
          V < min_allowed_V || V > max_allowed_V) continue;

/*
// Explicitly check if current (U,V) coordinates already exist within
// some previous extracted feature.  If so, skip processing current
// feature:

      bool duplicate_feature_flag=false;
      const double min_delta=0.0001;
      for (unsigned int g=0; g<currimage_feature_info.size(); g++)
      {
         descriptor* Fprev_ptr=currimage_feature_info[g].first;
         double Uprev=Fprev_ptr->get(1);
         double Vprev=Fprev_ptr->get(2);
         if ( sqr(Uprev-U) + sqr(Vprev-V) < sqr(min_delta))
         {
            n_duplicate_features++;
            duplicate_feature_flag=true;
         }
      } // loop over index g labeling curr image feature pairs

      if (duplicate_feature_flag) continue;
*/

      F_ptr->put(1,U);
      F_ptr->put(2,V);

      F_ptr->put(3,descriptors[2]);
      F_ptr->put(4,descriptors[3]);

// 6th component of *F_ptr acts as counter indicating number of images
// in which feature exists:

      F_ptr->put(5,1);

// If current photograph's camera is calibrated, compute backprojected
// 3D ray corresponding to (U,V):

      camera* camera_ptr=photograph_ptr->get_camera_ptr();
      if (camera_ptr->get_calibration_flag())
      {
         threevector n_hat=camera_ptr->pixel_ray_direction(U,V);
         F_ptr->put(6,n_hat.get(0));
         F_ptr->put(7,n_hat.get(1));
         F_ptr->put(8,n_hat.get(2));
      }
      else
      {
         F_ptr->put(6,NEGATIVEINFINITY);
         F_ptr->put(7,NEGATIVEINFINITY);
         F_ptr->put(8,NEGATIVEINFINITY);
      }

      d_dims=128;
      descriptor* D_ptr=new descriptor(d_dims);
      for (unsigned int d=0; d<d_dims; d++)
      {
         D_ptr->put(d,descriptors[5+d]);
      }
//      cout << "*D_ptr = " << *D_ptr << endl;

      pair<descriptor*,descriptor*> P(F_ptr,D_ptr);
      currimage_feature_info.push_back(P);

   } // loop over index f labeling curr image's features
}

// ---------------------------------------------------------------------
// Member function append_tiepoint_inliers() computes the inner product
// between candidate tiepoint tiepoints using the fundamental matrix
// previously calculated from SIFT matching.  Tiepoints with
// scalar products exceeding max_scalar_product are rejected as
// outliers.

void sift_detector::append_tiepoint_inliers(
   int i,int j,const vector<cv::KeyPoint>& keypoints1,
   const vector<cv::KeyPoint>& keypoints2,
   double max_scalar_product)
{
   cout << "inside sift_detector::append_tiepoint_inliers()" << endl;
   cout << "inlier_tiepoint_pairs.size() = "
        << inlier_tiepoint_pairs.size() << endl;
//   cout << "*fundamental_ptr = " << *fundamental_ptr << endl;
//   outputfunc::enter_continue_char();

   vector<feature_pair>* currimage_feature_info_ptr=&(image_feature_info[i]);
   vector<feature_pair>* nextimage_feature_info_ptr=&(image_feature_info[j]);

   int feature_ID_offset=10000;

   const double scale=1;
   const double orientation=0;

   int n_inlier_tiepoints=0;
   for (unsigned int k=0; k<keypoints1.size(); k++)
   {
      twovector UV(keypoints1[k].pt.x,keypoints1[k].pt.y);
      twovector UVmatch(keypoints2[k].pt.x,keypoints2[k].pt.y);
      
      if (fabs(fundamental_ptr->scalar_product(UV,UVmatch))
      > max_scalar_product) continue;

      int feature_1_ID=keypoints1[k].class_id+feature_ID_offset;
      int feature_2_ID=feature_1_ID;

//      cout << "UV = " << UV << " feature1 ID = " << feature_1_ID << endl;
//      cout << "UVmatch = " << UVmatch << " feature2 ID = " << feature_2_ID
//           << endl;

      descriptor* F_ptr=new descriptor(f_dims);
      descriptor* Fmatch_ptr=new descriptor(f_dims);
      
      F_ptr->put(0,feature_1_ID);
      Fmatch_ptr->put(0,feature_2_ID);
      F_ptr->put(10,k);		// feature index
      Fmatch_ptr->put(10,k);	// feature index
      F_ptr->put(11,i);		// image ID
      Fmatch_ptr->put(11,j);	// image ID

      F_ptr->put(1,UV.get(0));
      F_ptr->put(2,UV.get(1));


      Fmatch_ptr->put(1,UVmatch.get(0));
      Fmatch_ptr->put(2,UVmatch.get(1));

      F_ptr->put(3,scale);
      F_ptr->put(4,orientation);
      Fmatch_ptr->put(3,scale);
      Fmatch_ptr->put(4,orientation);

// 6th component of *F_ptr acts as counter indicating number of images
// in which feature exists:

      F_ptr->put(5,2);
      Fmatch_ptr->put(5,2);

      F_ptr->put(6,NEGATIVEINFINITY);
      F_ptr->put(7,NEGATIVEINFINITY);
      F_ptr->put(8,NEGATIVEINFINITY);

      Fmatch_ptr->put(6,NEGATIVEINFINITY);
      Fmatch_ptr->put(7,NEGATIVEINFINITY);
      Fmatch_ptr->put(8,NEGATIVEINFINITY);

      inlier_tiepoint_pairs.push_back(feature_pair(F_ptr,Fmatch_ptr));

      d_dims=64;	// tiepoint descriptors' dim  ????????
      descriptor* D_ptr=new descriptor(d_dims);
      D_ptr->clear_values();
      descriptor* Dmatch_ptr=new descriptor(d_dims);
      Dmatch_ptr->clear_values();

      pair<descriptor*,descriptor*> P(F_ptr,D_ptr);
      currimage_feature_info_ptr->push_back(P);
      pair<descriptor*,descriptor*> Pmatch(Fmatch_ptr,Dmatch_ptr);
      nextimage_feature_info_ptr->push_back(Pmatch);

      n_inlier_tiepoints++;
   } // loop over index k labeling candidate tiepoint tiepoints

   cout << "Initially, n_inlier_tiepoints = " << keypoints1.size() << endl;
   cout << "After fundamental test, n_inlier_tiepoints = "
        << n_inlier_tiepoints << endl;
   cout << "After appending, inlier_tiepoint_pairs.size() = "
        << inlier_tiepoint_pairs.size() << endl;
}

// =========================================================================
// Consolidated SIFT + Hessian affine feature tracking member functions
// =========================================================================

// Member function import_consolidated_features() loops over each
// image and recovers consolidated SIFT and Hessian-affine F & D
// feature descriptor vectors from previously calculated binary hdf5
// files. Feature information for each image is stored into member STL
// vector of vectors image_feature_info.

void sift_detector::import_consolidated_features(string bundler_IO_subdir)
{
   string banner="Importing consolidated SIFT & Hessian-affine features:";
   outputfunc::write_banner(banner);

   vector<feature_pair> currimage_feature_info;
   for (unsigned int i=0; i<n_images; i++)
   {
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);

      parse_consolidated_features(
         bundler_IO_subdir,photograph_ptr,currimage_feature_info);
      add_image_feature_info(i,currimage_feature_info);

      cout << "i = " << i 
           << " currimage_feature_info.size() = " 
           << currimage_feature_info.size() << endl;
      export_feature_tracks(i);
   } // loop over index i labeling input images
}

// ---------------------------------------------------------------------
// Member function parse_consolidated_features() imports F and D
// descriptors stored within binary hdf5 files for the input
// *photograph_ptr.  It reconstructs 9-dimensional *F_ptr and
// 128-dimensional *D_ptr descriptors from the input binary files.
// This method returns STL vector currimage_feature_info containing
// pairs of feature descriptor pointers (F_ptr,D_ptr).

void sift_detector::parse_consolidated_features(
   string bundler_IO_subdir,photograph* photograph_ptr,
   vector<feature_pair>& currimage_feature_info)
{
   cout << "inside sift_detector::parse_consolidated_features(photograph_ptr)" 
        << endl;

// FAKE FAKE: Sat Apr 11, 2015
// comment out this method when trying to compile with OpenCV 3.0.0

/*

   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string images_subdir=bundler_IO_subdir+"images/";
   string sift_keys_subdir=images_subdir+"keys/";
   cout << "sift_keys_subdir = " << sift_keys_subdir << endl;

   string image_filename=photograph_ptr->get_filename();
   string basename=filefunc::getbasename(image_filename);
   basename=stringfunc::prefix(basename);
   string dirname=filefunc::getdirname(image_filename);
   cout << "image_filename = " << image_filename << endl;
   cout << "basename = " << basename << endl;

   string F_filename=sift_keys_subdir+"F_"+basename+".hdf5";
   string D_filename=sift_keys_subdir+"D_"+basename+".hdf5";
   
   cout << "F_filename = " << F_filename << endl;
   cout << "D_filename = " << D_filename << endl;


   flann::Matrix<float> F_descriptors;
   flann::Matrix<float> D_descriptors;

   flann::load_from_file(
      F_descriptors,F_filename.c_str(),"sift_features");
   flann::load_from_file(
      D_descriptors,D_filename.c_str(),"sift_features");
   
   cout << "F_descriptors.rows = " << F_descriptors.rows << endl;
   cout << "F_descriptors.cols = " << F_descriptors.cols << endl;
   cout << "D_descriptors.rows = " << D_descriptors.rows << endl;
   cout << "D_descriptors.cols = " << D_descriptors.cols << endl;
   
   unsigned int n_Fcols=F_descriptors.cols;
   unsigned int n_Dcols=D_descriptors.cols;

   currimage_feature_info.clear();   

   for (unsigned int f=0; f<F_descriptors.rows; f++)
   {
      descriptor* F_ptr=new descriptor(n_Fcols);
      for (unsigned int c=0; c<n_Fcols; c++)
      {
         F_ptr->put(c,F_descriptors[f][c]);
      } // loop over index c
      F_ptr->put(0,feature_counter++);

      descriptor* D_ptr=new descriptor(n_Dcols);
      for (unsigned int c=0; c<n_Dcols; c++)
      {
         D_ptr->put(c,D_descriptors[f][c]);
      } // loop over index c
      pair<descriptor*,descriptor*> P(F_ptr,D_ptr);
      currimage_feature_info.push_back(P);
   } // loop over index f labeling consolidated features

   delete [] F_descriptors.ptr();
   delete [] D_descriptors.ptr();

*/

}

// =========================================================================
// SIFT feature import member functions
// =========================================================================

// Member function import_compressed_sift_hdf5_filenames() searches for 
// files within the specified input subdirectory which contain the
// substring "hdf5." in their names.  So both gzipped and LZOP
// compressed files with names like foo.hdf5.gz or foo.hdf5.lzo are
// found by this method.  The names for all such compressed HDF5 files are
// returned in an STL vector.

vector<string>& sift_detector::import_compressed_sift_hdf5_filenames(
   string sift_keys_subdir)
{
//   cout << "inside sift_detector::import_compressed_sift_hdf5_filenames()" << endl;
//   cout << "sift_keys_subdir = " << sift_keys_subdir << endl;

   sift_keys_filenames.clear();
   string substring="hdf5.";
   sift_keys_filenames=filefunc::file_basenames_in_subdir_matching_substring(
      sift_keys_subdir,substring);
//   cout << "sift_keys_filenames.size() = " << sift_keys_filenames.size() 
//        << endl;
   
//   for (unsigned int s=0; s<sift_keys_filenames.size(); s++)
//   {
//      cout << "s = " << s
//           << " sift_keys_filenames[s] = "
//           << sift_keys_filenames[s] << endl;
//   }

   return sift_keys_filenames;
}

// ---------------------------------------------------------------------
// Member function import_sift_keys_filenames()

vector<string>& sift_detector::import_sift_keys_filenames(
   string sift_keys_subdir)
{
//   cout << "inside sift_detector::import_sift_keys_filenames()" << endl;
//   cout << "sift_keys_subdir = " << sift_keys_subdir << endl;

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("gz");
   allowed_suffixes.push_back("key");
   allowed_suffixes.push_back("keys");

   sift_keys_filenames.clear();
   string substring=".key";
   sift_keys_filenames=filefunc::file_basenames_in_subdir_matching_substring(
      sift_keys_subdir,substring);

//   cout << "sift_keys_filenames.size() = " << sift_keys_filenames.size() 
//        << endl;
   
//   for (unsigned int s=0; s<sift_keys_filenames.size(); s++)
//   {
//      cout << "s = " << s
//           << " sift_keys_filenames[s] = "
//           << sift_keys_filenames[s] << endl;
//   }

   return sift_keys_filenames;
}

// ---------------------------------------------------------------------
// Member function import_D_ptrs() 

void sift_detector::import_D_ptrs(
   string sift_keys_subdir,vector<int>* image_IDs_ptr,
   vector<descriptor*>* D_ptrs_ptr)
{
   cout << "inside sift_detector::import_D_ptrs()" << endl;

   import_sift_keys_filenames(sift_keys_subdir);

   vector<feature_pair> currimage_feature_info;
   for (unsigned int i=0; i<sift_keys_filenames.size(); i++)
   {
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);
      cout << "i = " << i 
           << " sift_key_filename = " << sift_keys_filenames[i]
           << endl;

      bool Lowe_SIFT_flag=true;
      parse_Lowe_features(
         Lowe_SIFT_flag,photograph_ptr->get_xdim(),photograph_ptr->get_ydim(),
         sift_keys_filenames[i],currimage_feature_info);
      for (unsigned int f=0; f<currimage_feature_info.size(); f++)
      {
         image_IDs_ptr->push_back(i);
         D_ptrs_ptr->push_back(currimage_feature_info[f].second);
      }
   } // loop over index i labeling input images

   cout << "D_ptrs_ptr->size() = " << D_ptrs_ptr->size() << endl;
}

// ---------------------------------------------------------------------
// Member function import_D_ptrs() 

void sift_detector::import_D_ptrs(
   string sift_keys_filename,int image_ID,vector<descriptor*>* D_ptrs_ptr)
{
   cout << "inside sift_detector::import_D_ptrs() #2" << endl;
   cout << "image_ID = " << image_ID 
        << " sift_keys_filename = " << sift_keys_filename
        << endl;

   photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(image_ID);

   vector<feature_pair> currimage_feature_info;      

   bool Lowe_SIFT_flag=true;
   parse_Lowe_features(
      Lowe_SIFT_flag,photograph_ptr->get_xdim(),photograph_ptr->get_ydim(),
      sift_keys_filename,currimage_feature_info);

   for (unsigned int f=0; f<currimage_feature_info.size(); f++)
   {
      D_ptrs_ptr->push_back(currimage_feature_info[f].second);
   }

   cout << "D_ptrs_ptr->size() = " << D_ptrs_ptr->size() << endl;
}

// ---------------------------------------------------------------------
// Member function compute_SIFT_features_covar_matrix_sqrt() 

genmatrix* sift_detector::compute_SIFT_features_covar_matrix_sqrt(
   const vector<descriptor*>* D_ptrs_ptr,string covar_sqrt_filename)
{
   cout << "inside sift_detector::compute_SIFT_features_covar_matrix_sqrt()" 
        << endl;

// FAKE FAKE:  Fri May 31, 2013:

// Cluge: Convert vector of descriptors to vector of genvectors:

   vector<genvector*>* V_ptrs_ptr=new vector<genvector*>;
   for (unsigned int i=0; i<D_ptrs_ptr->size(); i++)
   {
      genvector* curr_V_ptr=new genvector(*(D_ptrs_ptr->at(i)));
      V_ptrs_ptr->push_back(curr_V_ptr);
   } // loop over index i 

   genmatrix* covar_matrix_ptr=
      mathfunc::covariance_matrix(*V_ptrs_ptr);
//   cout << "covar matrix = " << *covar_matrix_ptr << endl;

   for (unsigned int i=0; i<V_ptrs_ptr->size(); i++)
   {
      delete V_ptrs_ptr->at(i);
   }

   d_dims=128;
   genmatrix* covar_sqrt_ptr=new genmatrix(d_dims,d_dims);
   covar_matrix_ptr->square_root(*covar_sqrt_ptr);
   delete covar_matrix_ptr;

// Export square root of SIFT feature covariance matrix to text file
// so that it does not need to be computed more than once:

   ofstream outstream;
   filefunc::openfile(covar_sqrt_filename,outstream);

   for (unsigned int i=0; i<d_dims; i++)
   {
      for (unsigned int j=0; j<d_dims; j++)
      {
         outstream << covar_sqrt_ptr->get(i,j) << endl;
      }
   }
   filefunc::closefile(covar_sqrt_filename,outstream);   

   string banner="Wrote square root of SIFT feature covariance matrix to "+
      covar_sqrt_filename;
   outputfunc::write_big_banner(banner);

   return covar_sqrt_ptr;
}

// ---------------------------------------------------------------------
// Member function import_SIFT_features_covar_matrix_sqrt() 

genmatrix* sift_detector::import_SIFT_features_covar_matrix_sqrt(
   string covar_sqrt_filename)
{
   cout << "inside sift_detector::import_SIFT_features_covar_matrix_sqrt()" 
        << endl;

   d_dims=128;
   genmatrix* covar_sqrt_ptr=new genmatrix(d_dims,d_dims);

// Export square root of SIFT feature covariance matrix to text file
// so that it does not need to be computed more than once:

   filefunc::ReadInfile(covar_sqrt_filename);

   int counter=0;
   for (unsigned int i=0; i<d_dims; i++)
   {
      for (unsigned int j=0; j<d_dims; j++)
      {
         double curr_value=stringfunc::string_to_number(
            filefunc::text_line[counter]);
         covar_sqrt_ptr->put(i,j,curr_value);
         counter++;
      }
   }

   string banner=
      "Imported square root of SIFT feature covariance matrix from "+
      covar_sqrt_filename;
   outputfunc::write_big_banner(banner);

   return covar_sqrt_ptr;   
}

// =========================================================================
// HOG feature extraction member functions
// =========================================================================

void sift_detector::extract_HOG_features(unsigned int n_rows,unsigned int n_columns)
{
   string banner="Extracting HOG features:";
   outputfunc::write_banner(banner);

   vector<feature_pair> currimage_feature_info;
   for (unsigned int i=0; i<n_images; i++)
   {
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);

      n_duplicate_features=0;
      extract_HOG_features(
         photograph_ptr->get_filename(),n_rows,n_columns,
         currimage_feature_info);
      add_image_feature_info(i,currimage_feature_info);

      cout << "image i = " << i << " had " << n_duplicate_features
           << " duplicate features" << endl;
      cout << "currimage_feature_info.size() = " 
           << currimage_feature_info.size() << endl;
      
      export_feature_tracks(i);
   } // loop over index i labeling input images

/*
   cout << "image_feature_info.size() = " << image_feature_info.size()
        << endl;
   for (unsigned int i=0; i<image_feature_info.size(); i++)
   {
      cout << "i = " << i << " image_feature_info[i].size() = "
           << image_feature_info[i].size() << endl;
   }
   outputfunc::enter_continue_char();
*/
}

// ---------------------------------------------------------------------
// Member function extract_HOG_features() imports the image specified
// by the input filename.  It sets up a lattice of U,V sites which
// covers the image and whose dimensions are supplied as input
// parameters.  Looping over each lattice site, this method calls the 
// histogram-of-oriented gradients binary to compute 128-dimensional
// descriptor vectors.  Feature UV coordinates and 128-dimensional
// values are returned within STL vector curr_image_feature_info.

void sift_detector::extract_HOG_features(
   string image_filename,unsigned int n_columns,unsigned int n_rows,
   vector<feature_pair>& currimage_feature_info)
{
//   cout << "inside sift_detector::extract_HOG_features()" << endl;

// First generate grid of keypoints:

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
      image_filename,NULL);
   int pixel_height=texture_rectangle_ptr->getHeight();

   double Umin=texture_rectangle_ptr->get_minU();
   double Umax=texture_rectangle_ptr->get_maxU();
   double Vmin=texture_rectangle_ptr->get_minV();
   double Vmax=texture_rectangle_ptr->get_maxV();
   double dU=(Umax-Umin)/(n_columns+1);
   double dV=(Vmax-Vmin)/(n_rows+1);
//   cout << "dU = " << dU << " dV = " << dV << endl;
   
   string keypoints_filename="/tmp/keypoints.kps";
   ofstream keypts_stream;
   filefunc::openfile(keypoints_filename,keypts_stream);
  
// Generate HOG locations along U,V lattice which is displaced inwards
// from the border of the image:
 
   unsigned int pu,pv;
   for (unsigned int r=1; r<=n_rows; r++)
   {
      double V=Vmin+r*dV;
      for (unsigned int c=1; c<=n_columns; c++)
      {
         double U=Umin+c*dU;
//         cout << "r = " << r << " c = " << c
//              << " U = " << U << " V = " << V << endl;
         texture_rectangle_ptr->get_pixel_coords(U,V,pu,pv);
         keypts_stream << pu << "  " << pv << endl;
      }
   }
   filefunc::closefile(keypoints_filename,keypts_stream);
   delete texture_rectangle_ptr;

   string hog_features_filename="/tmp/hog.features";
   string unix_cmd="hog -i "+image_filename+
      " -k "+keypoints_filename+" -p 64 -o "+hog_features_filename;
   sysfunc::unix_command(unix_cmd);

// Parse HOG features:

   filefunc::ReadInfile(hog_features_filename);
   int linenumber=0;
   
//   unsigned int feature_size=stringfunc::string_to_number(
//      filefunc::text_line[linenumber++]);
   unsigned int n_features=stringfunc::string_to_number(
      filefunc::text_line[linenumber++]);

//   cout << "feature_size = " << feature_size << endl;
//   cout << "n_features = " << n_features << endl;

   currimage_feature_info.clear();
   for (unsigned int f=0; f<n_features; f++)
   {
      descriptor* F_ptr=new descriptor(f_dims);
      F_ptr->put(0,feature_counter++);
      F_ptr->put(10,f);		// feature index
      F_ptr->put(11,-1);	// image ID

      vector<double> feature_keys=stringfunc::string_to_numbers(
         filefunc::text_line[linenumber++]);
                                                 
//      cout << "feature = " << f
//           << " pu = " << feature_keys[0]
//           << " pv = " << feature_keys[1]
//           << " pixel_height = " << pixel_height
//           << endl;

      double U=double(feature_keys[0])/double(pixel_height);
      double V=1-double(feature_keys[1])/double(pixel_height);
      
      if (U < min_allowed_U || U > max_allowed_U ||
          V < min_allowed_V || V > max_allowed_V) continue;

      F_ptr->put(1,U);
      F_ptr->put(2,V);
      F_ptr->put(3,feature_keys[2]);	// patch size

// For HOG features, 5th component of *F_ptr holds dummy value:

      F_ptr->put(4,-1);	

// 6th component of *F_ptr acts as counter indicating number of images
// in which feature exists:

      F_ptr->put(5,1);

// If current photograph's camera is calibrated, compute backprojected
// 3D ray corresponding to (U,V):

/*
      camera* camera_ptr=photograph_ptr->get_camera_ptr();
      if (camera_ptr->get_calibration_flag())
      {
         threevector n_hat=camera_ptr->pixel_ray_direction(U,V);
         F_ptr->put(6,n_hat.get(0));
         F_ptr->put(7,n_hat.get(1));
         F_ptr->put(8,n_hat.get(2));
      }
      else
*/

      {
         F_ptr->put(6,NEGATIVEINFINITY);
         F_ptr->put(7,NEGATIVEINFINITY);
         F_ptr->put(8,NEGATIVEINFINITY);
      }

      d_dims=128;
      descriptor* D_ptr=new descriptor(d_dims);
      for (unsigned int d=0; d<d_dims; d++)
      {
         D_ptr->put(d,feature_keys[3+d]);
      }

      pair<descriptor*,descriptor*> P(F_ptr,D_ptr);
//       cout << "*F_ptr = " << *F_ptr << endl;
//       cout << "*D_ptr = " << *D_ptr << endl;
      currimage_feature_info.push_back(P);

   } // loop over index f labeling curr image's features
}

// ---------------------------------------------------------------------
// Member function extract_CHOG_features() loops over each image within
// *photogroup_ptr. It calls the Compressed Histogram of Features
// binary to extract feature ID, U,V,scale and orientation information
// along with 91-dimensional descriptors for each feature.  This CHOG
// information is stored for each image into member STL vector of
// vectors image_feature_info.

void sift_detector::extract_CHOG_features(int n_requested_features)
{
   string banner="Extracting Compressed HOG features:";
   outputfunc::write_banner(banner);

   vector<feature_pair> currimage_feature_info;
   for (unsigned int i=0; i<n_images; i++)
   {
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);

      n_duplicate_features=0;
      extract_CHOG_features(
         photograph_ptr->get_filename(),n_requested_features,
         currimage_feature_info);
      add_image_feature_info(i,currimage_feature_info);

      cout << "image i = " << i << " had " << n_duplicate_features
           << " duplicate features" << endl;
      cout << "currimage_feature_info.size() = " 
           << currimage_feature_info.size() << endl;
      
      export_feature_tracks(i);
   } // loop over index i labeling input images

/*
   cout << "image_feature_info.size() = " << image_feature_info.size()
        << endl;
   for (unsigned int i=0; i<image_feature_info.size(); i++)
   {
      cout << "i = " << i << " image_feature_info[i].size() = "
           << image_feature_info[i].size() << endl;
   }
   outputfunc::enter_continue_char();
*/
}

// ---------------------------------------------------------------------
// Member function extract_CHOG_features() imports the image specified
// by the input filename as well as a desired number of features to
// detect.  It calls the Compressed Histogram-of-Gradients binary to
// compute 91-dimensional feature vectors.  Feature UV coordinates and
// 91-dimensional values are returned within STL vector
// currimage_feature_info.

void sift_detector::extract_CHOG_features(
   string image_filename,int n_requested_features,
   vector<feature_pair>& currimage_feature_info)
{
//   cout << "inside sift_detector::extract_CHOG_features()" << endl;

   unsigned int pixel_width,pixel_height;
   imagefunc::get_image_width_height(image_filename,pixel_width,pixel_height);

   string pgm_filename=imagefunc::convert_image_to_pgm(image_filename);
   string chog_features_filename="/tmp/chog.features";

   string unix_cmd="chog-release -m 1 -n "+stringfunc::number_to_string(
      n_requested_features)+" "+pgm_filename+" "+chog_features_filename;
   sysfunc::unix_command(unix_cmd);
   filefunc::deletefile(pgm_filename);

// Parse CHOG features:

   filefunc::ReadInfile(chog_features_filename);


   unsigned int n_features=filefunc::text_line.size();
//   cout << "n_features = " << n_features << endl;
   currimage_feature_info.clear();
   for (unsigned int f=0; f<n_features; f++)
   {
      descriptor* F_ptr=new descriptor(f_dims);
      F_ptr->put(0,feature_counter++);
      F_ptr->put(10,f);		// feature index
      F_ptr->put(11,-1);	// image ID


      vector<double> feature_keys=stringfunc::string_to_numbers(
         filefunc::text_line[f]);
                                                 
//      cout << "feature = " << f
//           << " pu = " << feature_keys[0]
//           << " pv = " << feature_keys[1]
//           << " pixel_height = " << pixel_height
//           << endl;

      double U=double(feature_keys[0])/double(pixel_height);
      double V=1-double(feature_keys[1])/double(pixel_height);
      
      if (U < min_allowed_U || U > max_allowed_U ||
          V < min_allowed_V || V > max_allowed_V) continue;

      F_ptr->put(1,U);
      F_ptr->put(2,V);
      F_ptr->put(3,feature_keys[2]);	// scale
      F_ptr->put(4,feature_keys[3]);	// orientation

// For CHOG features, feature_keys[4] = hessian value (whatever that is..._

// 6th component of *F_ptr acts as counter indicating number of images
// in which feature exists:

      F_ptr->put(5,1);

// If current photograph's camera is calibrated, compute backprojected
// 3D ray corresponding to (U,V):

//      camera* camera_ptr=photograph_ptr->get_camera_ptr();
//      if (camera_ptr->get_calibration_flag())
//      {
//         threevector n_hat=camera_ptr->pixel_ray_direction(U,V);
//         F_ptr->put(6,n_hat.get(0));
//         F_ptr->put(7,n_hat.get(1));
//         F_ptr->put(8,n_hat.get(2));
//      }
//      else

      {
         F_ptr->put(6,NEGATIVEINFINITY);
         F_ptr->put(7,NEGATIVEINFINITY);
         F_ptr->put(8,NEGATIVEINFINITY);
      }

      unsigned int d_dims=feature_keys.size()-5;
//      cout << "d_dims = " << d_dims << endl;
      descriptor* D_ptr=new descriptor(d_dims);

      double Dsum=0;
      for (unsigned int d=0; d<d_dims; d++)
      {
         D_ptr->put(d,feature_keys[5+d]);
         Dsum += feature_keys[5+d];
      }
//      cout << "f = " << f << " Dsum = " << Dsum << endl;

/*
// Renormalize *D_ptr so that its components' values sum to one.  This
// normalization condition is necessary if *D_ptr is to be used for
// Kullback-Leibler divergence computations:

      for (unsigned int d=0; d<d_dims; d++)
      {
         D_ptr->put(d,D_ptr->get(d)/Dsum);
      }
*/

      pair<descriptor*,descriptor*> P(F_ptr,D_ptr);
//      cout << "*F_ptr = " << *F_ptr << endl;
//      cout << "*D_ptr = " << *D_ptr << endl;
      currimage_feature_info.push_back(P);

   } // loop over index f labeling curr image's features
}

// =========================================================================
// SURF feature extraction member functions
// =========================================================================

// Member function extract_SURF_features() loops over each image within
// *photogroup_ptr. It calls DLIB's SURF feature extraction method
// to extract feature ID, U,V,scale and orientation information
// along with 64-dimensional descriptors for each feature.  This SURF
// information is stored for each image into member STL vector of
// vectors image_feature_info.

void sift_detector::extract_SURF_features(string SURF_keys_subdir)
{
   string banner="Extracting SURF features:";
   outputfunc::write_banner(banner);

   filefunc::dircreate(SURF_keys_subdir);

   for (unsigned int i=0; i<n_images; i++)
   {
      outputfunc::update_progress_fraction(i,50,n_images);
//      cout << "i = " << i << " n_images = " << n_images << endl;
      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(i);
      extract_SURF_features(
         SURF_keys_subdir,photograph_ptr->get_filename());

   } // loop over index i labeling input images
}

// ---------------------------------------------------------------------
// This overloaded version of extract_SURF_features() works with a set
// of input image filenames rather than a photogroup object.

void sift_detector::extract_SURF_features(
   string SURF_keys_subdir,vector<string>& image_filenames)
{
   string banner="Extracting SURF features:";
   outputfunc::write_banner(banner);

   n_images=image_filenames.size();
   for (unsigned int i=0; i<n_images; i++)
   {
      outputfunc::update_progress_fraction(i,100,n_images);
      extract_SURF_features(
         SURF_keys_subdir,image_filenames[i]);
   } // loop over index i labeling input images
}

// ---------------------------------------------------------------------
// Member function extract_SURF_features() imports the image specified
// by the input filename.  It calls the dlib's get_surf_points()
// method to compute SURF interest points and their 64-dimensional
// descriptors.  DLIB's output is serialized and exported to a binary
// output file within SURF_keys_subdir.

void sift_detector::extract_SURF_features(
   string SURF_keys_subdir,string image_filename)
{
//   cout << "inside sift_detector::extract_SURF_features()" << endl;

// Here we declare an image object that can store rgb_pixels.  Note
// that in dlib there is no explicit image object, just a 2D array and
// various pixel types.

   dlib::array2d<dlib::rgb_pixel> img;

// Now load the image file into our image.  If something is wrong then
// load_image() will throw an exception.  Also, if you linked with
// libpng and libjpeg then load_image() can load PNG and JPEG files in
// addition to BMP files:

   dlib::load_image(img, image_filename.c_str());

// Get SURF points from the image.  Note that get_surf_points() has
// some optional arguments that allow you to control the number of
// points you get back:

   int n_points=50000;
//   double threshold=1E-4;
   double threshold=0.01;
//   double threshold=1;
   vector<dlib::surf_point> sp = 
      dlib::get_surf_points(img,n_points,threshold);

// Serialize and save SURF features:

   string image_prefix=filefunc::getprefix(image_filename);
   string surf_features_filename=SURF_keys_subdir+image_prefix
      +".surf_features";
   ofstream fout(surf_features_filename.c_str(),ios::binary);
   dlib::serialize(sp,fout);
   fout.close();
}

// ---------------------------------------------------------------------
// Member function import_SURF_features() imports a serialized 
// DLIB object which contains SURF feature for the specified input image.
// Interest point UV coordinates and 64-dimensional
// descriptors are returned within STL vector currimage_feature_info.

void sift_detector::import_SURF_features(
   string SURF_keys_subdir,string image_filename,
   vector<feature_pair>& currimage_feature_info)
{
//   cout << "inside sift_detector::extract_SURF_features()" << endl;

   unsigned int pixel_width,pixel_height;
   imagefunc::get_image_width_height(image_filename,pixel_width,pixel_height);

// Deserialize previously saveed SURF features:

   string image_prefix=filefunc::getprefix(image_filename);
   string surf_features_filename=SURF_keys_subdir+image_prefix
      +".surf_features";
   ifstream fin(surf_features_filename.c_str(),ios::binary);

   vector<dlib::surf_point> sp;
   dlib::deserialize(sp, fin);

   unsigned int n_features=sp.size();
   currimage_feature_info.clear();
   for (unsigned int f=0; f<n_features; f++)
   {
      descriptor* F_ptr=new descriptor(f_dims);
      F_ptr->put(0,feature_counter++);
      F_ptr->put(10,f);		// feature index
      F_ptr->put(11,-1);	// image ID

      double U=sp[f].p.center.x()/double(pixel_height);
      double V=1-sp[f].p.center.y()/double(pixel_height);
    
      if (U < min_allowed_U || U > max_allowed_U ||
      V < min_allowed_V || V > max_allowed_V) continue;
      
      F_ptr->put(1,U);
      F_ptr->put(2,V);
      F_ptr->put(3,sp[f].p.scale);       	// scale
      F_ptr->put(4,sp[f].angle);		// orientation
//      double score=sp[f].p.score;
//      double laplacian=sp[f].p.laplacian;

//      cout << "f = " << f
//           << " U = " << U << " V = " << V
//           << " scale = " << F_ptr->get(3)
//           << " angle = " << F_ptr->get(4)*180/PI 
//           << " laplacian = " << laplacian 
//           << endl;
      
// 6th component of *F_ptr acts as counter indicating number of images
// in which feature exists:

      F_ptr->put(5,1);

      F_ptr->put(6,NEGATIVEINFINITY);
      F_ptr->put(7,NEGATIVEINFINITY);
      F_ptr->put(8,NEGATIVEINFINITY);

      unsigned int d_dims=sp[f].des.size();	// 64

// Note added on 11/5/13: sp[f].des is normalized to unity, and its
// coordinates can assume negative values. To conform with SIFT
// descriptor conventions, we multiply all components within by 256:

      descriptor* D_ptr=new descriptor(d_dims);
      for (unsigned int d=0; d<d_dims; d++)
      {
         D_ptr->put(d,128*(1+sp[f].des(d)));
      }

      pair<descriptor*,descriptor*> P(F_ptr,D_ptr);
//      cout << "*F_ptr = " << *F_ptr << endl;
//      cout << "*D_ptr = " << *D_ptr << endl;
//      cout << "D_ptr->magnitude() = " << D_ptr->magnitude() << endl;
      currimage_feature_info.push_back(P);

   } // loop over index f labeling SURF features
}

// =========================================================================
// CHOG feature matching via fundamental matrix member functions:
// =========================================================================

void sift_detector::identify_CHOG_feature_matches_via_fundamental_matrix(
   double max_ratio,double worst_frac_to_reject,double max_scalar_product)
{ 
   cout << "inside sift_detector::identify_CHOG_feature_matches_via_fundamental_matrix()" 
        << endl;

   unsigned int istart=0;
   unsigned int istop=n_images-1;
   
   for (unsigned int i=istart; i<istop; i++)
   {
      for (unsigned int j=i+1; j<n_images; j++)
      {
         identify_CHOG_feature_matches_via_fundamental_matrix(
            i,j,max_ratio,worst_frac_to_reject,max_scalar_product);
      } // loop over index j labeling images
      export_feature_tracks(i);
   } // loop over index i labeling images
   export_feature_tracks(istop);
}

// ---------------------------------------------------------------------
// Member function
// identify_CHOG_feature_matches_via_fundamental_matrix() takes
// in image indices i and j.  It first instantiates VPtrees (Di,Dj)
// feature information for image j and image i, respectively. This
// method then searches for tiepoint pairings between the features.

bool sift_detector::identify_CHOG_feature_matches_via_fundamental_matrix(
   int i,int j,double max_ratio,double worst_frac_to_reject,
   double max_scalar_product)
{ 
   cout << "inside sift_detector::identify_CHOG_feature_matches_via_fundamental_matrix()" 
        << endl;
   cout << "i = " << i << " j = " << j << endl;
   
   vector<feature_pair> currimage_feature_info=image_feature_info[i];
   vector<feature_pair> nextimage_feature_info=image_feature_info[j];
   
// Loop over features in image i and search for counterparts in images
// jstart through jstop:
   
   identify_CHOG_feature_matches_for_image_pair(
      currimage_feature_info,nextimage_feature_info,max_ratio);

   int n_candidate_tiepoints=candidate_tiepoint_pairs.size();
   const int min_candidate_tiepoints=8;
   if (n_candidate_tiepoints < min_candidate_tiepoints) return false;

   cout << "Before entering RANSAC loop" << endl;

   max_n_inliers=0;
   int n_good_iters=0;
   int max_n_good_RANSAC_iters=500;
   vector<twovector> tiepoint_UV,tiepoint_UVmatch;
   store_tiepoint_twovectors(
      candidate_tiepoint_pairs,tiepoint_UV,tiepoint_UVmatch);

   double min_RANSAC_cost=POSITIVEINFINITY;
   inlier_tiepoint_pairs.clear();
   while (n_good_iters < max_n_good_RANSAC_iters)
   {
      if (!compute_candidate_fundamental_matrix()) 
         continue;
      if (n_good_iters%10==0) cout << n_good_iters/10 << " " << flush;

      identify_inliers_via_fundamental_matrix(
         max_scalar_product,tiepoint_UV,tiepoint_UVmatch,
         candidate_tiepoint_pairs,inlier_tiepoint_pairs,fundamental_ptr,
         min_RANSAC_cost,i,j);
      n_good_iters++;
   }
   cout << endl;
   cout << "After exiting RANSAC loop" << endl;

   cout << "n_inliers = " << max_n_inliers 
        << " n_candidate_tiepoints = " << n_candidate_tiepoints
        << " n_inliers/n_candidate_tiepoints = " 
        << double(max_n_inliers)/double(n_candidate_tiepoints)
        << endl;

   compute_inlier_fundamental_matrix(
      worst_frac_to_reject,inlier_tiepoint_pairs,fundamental_ptr,i,j);

// Iteratively refine fundamental matrix:

   unsigned int n_iters=3;
   for (unsigned int iter=0; iter<n_iters; iter++)
   {
      threevector e0_hat=fundamental_ptr->get_null_vector();
      fundamental_ptr->solve_for_fundamental(e0_hat);
   } // loop over iter index

   fundamental_ptr->renormalize_F_entries();

   identify_inliers_via_fundamental_matrix(
      max_scalar_product,tiepoint_UV,tiepoint_UVmatch,
      candidate_tiepoint_pairs,inlier_tiepoint_pairs,fundamental_ptr,
      min_RANSAC_cost,i,j);

   rename_feature_IDs(i,j);
   return true;
}

// ---------------------------------------------------------------------
// Member function identify_CHOG_feature_matches_for_image_pair()
// takes in image indices i and j.  It utilizes the Kdtrees
// within members *ANN_ptr and *inverse_ANN_ptr which hold (F,D)
// feature information for image j and image i, respectively.  This
// method then searches for one-to-one and onto pairings between the
// features for image i and image j.  Such bijective pairs are
// temporarily stored in member STL vector candidate_tiepoint_pairs
// for subsequent RANSAC processing.

void sift_detector::identify_CHOG_feature_matches_for_image_pair(
   const vector<feature_pair>& currimage_feature_info,
   const vector<feature_pair>& nextimage_feature_info,
   double max_ratio)
{
   cout << "inside sift_detector::identify_CHOG_feature_matches_for_image_pair()"
        << endl;

// Form STL map of next image's D_ptrs vs F_ptrs.  We'll later need
// this map in order to find a next image *F_ptr corresponding to
// a next image *D_ptr:

   typedef map<descriptor*,descriptor*> FEATURE_PAIR_MAP;

   cout << "Filling feature pair maps" << endl;
   FEATURE_PAIR_MAP currimage_feature_pair_map,nextimage_feature_pair_map;
   for (unsigned int f=0; f<currimage_feature_info.size(); f++)
   {
      descriptor* Fcurr_ptr=currimage_feature_info[f].first;
      descriptor* Dcurr_ptr=currimage_feature_info[f].second;
      currimage_feature_pair_map[Dcurr_ptr]=Fcurr_ptr;
   } // loop over index f labeling curr image's features

   for (unsigned int f=0; f<nextimage_feature_info.size(); f++)
   {
      descriptor* Fnext_ptr=nextimage_feature_info[f].first;
      descriptor* Dnext_ptr=nextimage_feature_info[f].second;
      nextimage_feature_pair_map[Dnext_ptr]=Fnext_ptr;
   } // loop over index f labeling next image's features

// Instantiate Vantage Point Trees to hold current and next image
// features:

   vptree* curr_vptree_ptr=new vptree;
//   curr_vptree_ptr->set_KL_distance_flag(false);
   curr_vptree_ptr->set_KL_distance_flag(true);
//   curr_vptree_ptr->set_sqrd_Euclidean_distance_flag(true);
   cout << "currimage_feature_info.size() = "
        << currimage_feature_info.size() << endl;
   curr_vptree_ptr->construct_tree(currimage_feature_info);

   vptree* next_vptree_ptr=new vptree;
//   next_vptree_ptr->set_KL_distance_flag(false);
   next_vptree_ptr->set_KL_distance_flag(true);
//   next_vptree_ptr->set_sqrd_Euclidean_distance_flag(true);
   cout << "nextimage_feature_info.size() = "
        << nextimage_feature_info.size() << endl;
   next_vptree_ptr->construct_tree(nextimage_feature_info);

   candidate_tiepoint_pairs.clear();
   unsigned int n_curr_features=currimage_feature_info.size();
//   unsigned int n_next_features=nextimage_feature_info.size();

//   cout << "n_curr_features = " << n_curr_features << endl;
//   outputfunc::enter_continue_char();   

// Loop over all features within current image.  Find a candidate
// feature within the next image as its tiepoint match.  Then inverse
// map next image feature back to current image.  Only declare
// candidate tiepoint pair if matching is one-to-one and onto:

   vector<int> nearest_neighbor_node_IDs;
   vector<double> query_to_neighbor_distances;
   vector<double> separation_distance_ratios;
   vector<descriptor*> metric_space_element_ptrs;
   for (unsigned int f=0; f<n_curr_features; f++)
   {
//      cout << "f = " << f << endl;
      if (f%100==0) cout << f << " " << flush;
      descriptor* Fcurr_ptr=currimage_feature_info[f].first;
      descriptor* Dcurr_ptr=currimage_feature_info[f].second;

//      cout << "f = " << f
//           << " Fcurr_ptr = " << Fcurr_ptr 
//           << " Dcurr_ptr = " << Dcurr_ptr 
//           << endl;

      next_vptree_ptr->incrementally_find_nearest_nodes(
         2,Dcurr_ptr,nearest_neighbor_node_IDs,query_to_neighbor_distances,
         metric_space_element_ptrs);
      double neighbor_distance_ratio=
         query_to_neighbor_distances[0]/query_to_neighbor_distances[1];
      separation_distance_ratios.push_back(neighbor_distance_ratio);

//      cout << "f = " << f 
//           << " sep distance 0 = " << query_to_neighbor_distances[0] 
//           << " sep distance 1 = " << query_to_neighbor_distances[1] 
//           << " ratio = " << separation_distance_ratios.back()
//           << endl;

      if (neighbor_distance_ratio > max_ratio) continue;

      descriptor* Dnext_ptr=metric_space_element_ptrs[0];
      FEATURE_PAIR_MAP::iterator iter=
         nextimage_feature_pair_map.find(Dnext_ptr);
      if (iter == nextimage_feature_pair_map.end())
      {
         cout << "Error!" << endl;
         exit(-1);
      }
      descriptor* Fnext_ptr=iter->second;

      curr_vptree_ptr->incrementally_find_nearest_nodes(
         2,Dnext_ptr,nearest_neighbor_node_IDs,query_to_neighbor_distances,
         metric_space_element_ptrs);
      neighbor_distance_ratio=
         query_to_neighbor_distances[0]/query_to_neighbor_distances[1];

//      cout << "f = " << f 
//           << " sep distance 0 = " << query_to_neighbor_distances[0] 
//           << " sep distance 1 = " << query_to_neighbor_distances[1] 
//           << endl;

      if (neighbor_distance_ratio > max_ratio) continue;

      descriptor* Dcurrent_ptr=metric_space_element_ptrs[0];
      iter=currimage_feature_pair_map.find(Dcurrent_ptr);
      if (iter == currimage_feature_pair_map.end())
      {
         cout << "Error!" << endl;
         exit(-1);
      }
      descriptor* Fcurrent_ptr=iter->second;

      if (Fcurr_ptr != Fcurrent_ptr) continue;

      candidate_tiepoint_pairs.push_back(feature_pair(Fcurr_ptr,Fnext_ptr));

//      cout << "f = " << f 
//           << " candidate_tiepoint_pair.size() = "
//           << candidate_tiepoint_pairs.size()
//           << " Dmatch_ptr = " << Dmatch_ptr 
//           << " Fmatch_ptr = " << Fmatch_ptr << endl;

   } // loop over index f labeling features within currimage
//   cout << endl;
   delete curr_vptree_ptr;
   delete next_vptree_ptr;

   prob_distribution prob(separation_distance_ratios,100);
   prob.writeprobdists(false);

   cout << endl;
//   cout << "At end of sift_detector::identify_CHOG_feature_matches_for_image_pair()" << endl;
}

// =========================================================================
// Candidate SIFT feature matching member functions
// =========================================================================

// Member function prepare_all_SIFT_descriptors() sequentially imports
// SIFT feature information for each image within STL vector member
// image_feature_info into FLANN matrix
// *SIFT_descriptors_matrix_ptr of *akm_ptr.  It then instantiates and
// a new KDtree index within the STL vector kdtree_index_ptrs member
// of *akm_ptr.

void sift_detector::prepare_all_SIFT_descriptors()
{ 
   cout << "inside sift_detector::prepare_all_SIFT_descriptors()" << endl;

   timefunc::initialize_timeofday_clock();
   for (unsigned int i=0; i<image_feature_info.size(); i++)
   {
      outputfunc::print_elapsed_time();
      vector<feature_pair>* currimage_feature_info_ptr=
         &(image_feature_info[i]);
      akm_ptr->load_SIFT_descriptors(currimage_feature_info_ptr);
      akm_ptr->pushback_SIFT_descriptors_matrix_ptr();
      akm_ptr->initialize_randomized_kdtree_index();
   } // loop over index i labeling images

//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function prepare_SIFT_descriptors[2]() imports SIFT feature
// information for image i into FLANN matrix
// *SIFT_descriptors[2]_matrix_ptr within *akm_ptr.  It also
// initializes the backward [forward] KDtrees in *akm_ptr.

void sift_detector::prepare_SIFT_descriptors(int i)
{ 
//   cout << "inside sift_detector::prepare_SIFT_descriptors()" << endl;
   vector<feature_pair>* currimage_feature_info_ptr=&(image_feature_info[i]);
   prepare_SIFT_descriptors(currimage_feature_info_ptr);
}

void sift_detector::prepare_SIFT_descriptors(
   vector<feature_pair>* currimage_feature_info_ptr)
{ 
//   cout << "inside sift_detector::prepare_SIFT_descriptors()" << endl;

   akm_ptr->load_SIFT_descriptors(currimage_feature_info_ptr);

// As of 4/30/13, we no longer perform "bijective"/"backward" SIFT
// matching for speed reasons!

//   akm_ptr->initialize_backward_SIFT_feature_search();
}

void sift_detector::prepare_SIFT_descriptors2(
   vector<feature_pair>* nextimage_feature_info_ptr)
{ 
//   cout << "inside sift_detector::prepare_SIFT_descriptors2()" << endl;

   akm_ptr->load_SIFT_descriptors2(nextimage_feature_info_ptr);
   akm_ptr->initialize_forward_SIFT_feature_search();
}

// Next overloaded version of prepare_SIFT_descriptors() is for
// parallelized running:

void sift_detector::prepare_SIFT_descriptors(
   int i,int j,akm* curr_akm_ptr)
{ 
//   cout << "inside sift_detector::prepare_SIFT_descriptors()" << endl;
   curr_akm_ptr->load_SIFT_descriptors(&(image_feature_info[i]));
   curr_akm_ptr->load_SIFT_descriptors2(&(image_feature_info[j]));

   if (forward_feature_matching_flag)
   {
      curr_akm_ptr->initialize_forward_SIFT_feature_search();
   }
   else
   {
      curr_akm_ptr->initialize_backward_SIFT_feature_search();
   }
}

// ---------------------------------------------------------------------
// Member function
// identify_candidate_feature_matches_via_fundamental_matrix() loops over
// all image pairs labeled by indices i and j.  For each pair, it
// identifies candidate one-to-one and onto feature matches based upon
// Lowe's ratio test.  It then bins the candidate matching features
// into quadrants surrounding the median point within image i.
// 4-point homographies are iteratively constructed from candidate
// tiepoint pairs randomly pulled from different quadrants.  Following
// the RANSAC procedure, we use the homographies to define inlier
// tiepoint pairs.  A final homography relating image i to image j is
// calculated based upon just inlier tiepoint pairs.  IDs for SIFT
// features extracted from image i and j are renamed so that they
// reflect tiepoint relationships.

// Feature track information is iteratively exported to output files
// as the processing for each image is finished.

// ---------------------------------------------------------------------
// Streamlined version of
// identify_candidate_feature_matches_via_Lowe_ratio_test() as of
// Tues, Apr 30, 2013:

bool sift_detector::identify_candidate_feature_matches_via_Lowe_ratio_test(
   int i,int j,double sqrd_max_ratio)
{ 
   cout << "inside sift_detector::identify_candidate_feature_matches_via_Lowe_ratio_test()" 
        << endl;

   prepare_SIFT_descriptors2(get_image_feature_info_ptr(j));

   candidate_tiepoint_pairs.clear();
   int n_matches=identify_candidate_FLANN_feature_matches_for_image_pair(
      akm_ptr,sqrd_max_ratio,
      get_image_feature_info_ptr(i),get_image_feature_info_ptr(j),
      candidate_tiepoint_pairs);
   return (n_matches > 0);
}

// ---------------------------------------------------------------------
// Working version of
// identify_candidate_feature_matches_via_Lowe_ratio_test() as of
// Friday, April 26, 2013:

bool sift_detector::identify_candidate_feature_matches_via_Lowe_ratio_test(
   int i,int jstart,int jstop,double sqrd_max_ratio)
{ 
   cout << "inside sift_detector::identify_candidate_feature_matches_via_Lowe_ratio_test()" 
        << endl;
//   cout << "i = " << i << " jstart = " << jstart
//        << " jstop = " << jstop << endl;
   if (jstart > jstop) return false;

   if (!FLANN_flag)
   {
      cout << "FLANN_flag must equal true!" << endl;
      exit(-1);
   }

   vector<feature_pair>* currimage_feature_info_ptr=&(image_feature_info[i]);
//   cout << "currimage_feature_info_ptr->size() = "
//        << currimage_feature_info_ptr->size() << endl;

// Load all features for images jstart through jstop into
// cumimage_feature_info:

   vector<feature_pair> cumimage_feature_info;
   vector<feature_pair>* nextimage_feature_info_ptr=NULL;

   if (jstop > jstart)
   {
      for (int j=jstart; j<=jstop; j++)
      {
         vector<feature_pair> nextimage_feature_info=image_feature_info[j];
         for (unsigned int k=0; k<nextimage_feature_info.size(); k++)
         {
            cumimage_feature_info.push_back(nextimage_feature_info[k]);
         }
      }
//      cout << "cumimage_feature_info.size() = "
//           << cumimage_feature_info.size() << endl;
      prepare_SIFT_descriptors2(&cumimage_feature_info);
   }
   else
   {
      nextimage_feature_info_ptr=&(image_feature_info[jstart]);
//      cout << "nextimage_feature_info_ptr->size() = "
//           << nextimage_feature_info_ptr->size() << endl;
      prepare_SIFT_descriptors2(nextimage_feature_info_ptr);
   }

   int n_matches=0;
   candidate_tiepoint_pairs.clear();
   int c_start=0;
   int c_stop=0;
   candidate_tiepoint_pairs_start_stop_images_map.clear();

   if (jstop > jstart)
   {
      n_matches += identify_candidate_FLANN_feature_matches_for_image_pair(
         c_start,c_stop,
         currimage_feature_info_ptr,&cumimage_feature_info,sqrd_max_ratio);
   }
   else
   {
      n_matches += identify_candidate_FLANN_feature_matches_for_image_pair(
         c_start,c_stop,
         currimage_feature_info_ptr,nextimage_feature_info_ptr,
         sqrd_max_ratio);
   }
   cout << "n_candidate_matches = " << n_matches << endl;

   return (n_matches > 0);
}

// ---------------------------------------------------------------------
// Member function
// identify_candidate_FLANN_feature_matches_for_image_pair()

int sift_detector::identify_candidate_FLANN_feature_matches_for_image_pair(
   int& c_start,int& c_stop,
   const vector<feature_pair>* currimage_feature_info_ptr,
   const vector<feature_pair>* nextimage_feature_info_ptr,
   double max_distance_ratio)
{
//   cout << "inside sift_detector::identify_candidate_FLANN_feature_matches_for_image_pair()"
//        << endl;
//   outputfunc::print_elapsed_time();

//   cout << "Finding only forward SIFT matches()" << endl;
   akm_ptr->find_only_forward_SIFT_matches(max_distance_ratio);

   vector<pair<int,int> >* initial_SIFT_matches_ptr=
      akm_ptr->get_initial_SIFT_matches_ptr();

   unsigned int n_initial_matches=initial_SIFT_matches_ptr->size();
//   cout << "n_initial_matches = " << n_initial_matches << endl;

   const int min_SOH_corner_angle_matches=3;
//   const int min_SOH_corner_angle_matches=4;

   int n_hamming_rejects=0;
//   const int max_hamming_dist=9;
//   const int max_hamming_dist=10;
//   const int max_hamming_dist=11;
   const int max_hamming_dist=12;
//   const int max_hamming_dist=25;
//   const int max_hamming_dist=50;

   int n_entropy_rejects=0;

   int n_candidate_matches=0;
   for (unsigned int t=0; t<n_initial_matches; t++)
   {
//      if (t%1000==0) cout << t/1000 << " " << flush;
      int f=initial_SIFT_matches_ptr->at(t).first;
      int g=initial_SIFT_matches_ptr->at(t).second;

// As of 4/27/13, we require at least 3 of 4 SOH corner angles to
// match before a putative tiepoint pair will be accepted as a genuine
// candidate match:

      descriptor* Dcurr_ptr=currimage_feature_info_ptr->at(f).second;
      descriptor* Dnext_ptr=nextimage_feature_info_ptr->at(g).second;

      int n_corner_angle_matches=
         count_SOH_corner_angle_matches(Dcurr_ptr,Dnext_ptr);
      if (n_corner_angle_matches < min_SOH_corner_angle_matches) continue;

// Reject tiepoint pair if its Hamming distance exceeds maximum
// threshold:

      int hamming_dist=binaryfunc::hamming_distance(Dcurr_ptr,Dnext_ptr);
      if (hamming_dist > max_hamming_dist)
      {
         n_hamming_rejects++;
         continue;
      }

// As of 8/25/13, we experiment with rejecting a putative tiepoint pair
// if either of its descriptors has a low entropy as defined by Wei
// Dong in his Princeton Ph.D. thesis:

      const double min_descriptor_entropy=4.4;
      if (Dcurr_ptr->entropy() < min_descriptor_entropy ||
          Dnext_ptr->entropy() < min_descriptor_entropy)
      {
         n_entropy_rejects++;
         continue;
      }

      descriptor* Fcurr_ptr=currimage_feature_info_ptr->at(f).first;

//      int curr_feature_ID=Fcurr_ptr->get(0);
//      candidate_tiepoint_currfeature_ids_iter=
//         candidate_tiepoint_currfeature_ids_map.find(curr_feature_ID);
//      if (candidate_tiepoint_currfeature_ids_iter != 
//      candidate_tiepoint_currfeature_ids_map.end()) continue;
//      candidate_tiepoint_currfeature_ids_map[curr_feature_ID]=true;

      descriptor* Fnext_ptr=nextimage_feature_info_ptr->at(g).first;

      candidate_tiepoint_pairs.push_back(feature_pair(Fcurr_ptr,Fnext_ptr));
      n_candidate_matches++;

/*
      if (candidate_tiepoint_pairs.size() <=1) continue;
      
      int next_image_ID=Fnext_ptr->get(11);

      int n_candidate_tiepoints=candidate_tiepoint_pairs.size();
      int previous_next_image_ID=candidate_tiepoint_pairs[
         n_candidate_tiepoints-2].second->get(11);
      if (next_image_ID==previous_next_image_ID) c_stop++;

      if (next_image_ID != previous_next_image_ID || t==n_matches-1)
      {
         pair<int,int> P(c_start,c_stop);
         candidate_tiepoint_pairs_start_stop_images_map[
            previous_next_image_ID]=P;
         c_start=c_stop+1;
         c_stop=c_start;
      }
*/

   } // loop over index t labeling tiepoint pairs
//   cout << endl;

   double hamming_rejection_frac=double(n_hamming_rejects)/
      n_initial_matches;
   double entropy_rejection_frac=double(n_entropy_rejects)/
      n_initial_matches;
   cout << "n_hamming_rejects = " << n_hamming_rejects << endl;
   cout << "hamming_rejection_frac = " << hamming_rejection_frac << endl;
   cout << "n_entropy_rejects = " << n_entropy_rejects << endl;
   cout << "entropy_rejection_frac = " << entropy_rejection_frac << endl;
//   outputfunc::enter_continue_char();

   return n_candidate_matches;
}

// ---------------------------------------------------------------------
// Streamlined version of member function
// identify_candidate_FLANN_feature_matches_for_image_pair()

int sift_detector::identify_candidate_FLANN_feature_matches_for_image_pair(
   akm* curr_akm_ptr,double max_distance_ratio,
   const vector<feature_pair>* currimage_feature_info_ptr,
   const vector<feature_pair>* nextimage_feature_info_ptr,
   vector<feature_pair>& curr_candidate_tiepoint_pairs)
{
//   cout << "inside sift_detector::identify_candidate_FLANN_feature_matches_for_image_pair()"
//        << endl;
//   outputfunc::print_elapsed_time();

   if (forward_feature_matching_flag)
   {
//      cout << "Finding only forward SIFT matches" << endl;
      curr_akm_ptr->find_only_forward_SIFT_matches(max_distance_ratio);
   }
   else
   {
//      cout << "Finding only backward SIFT matches" << endl;
      curr_akm_ptr->find_only_backward_SIFT_matches(max_distance_ratio);
   }

   vector<pair<int,int> >* initial_SIFT_matches_ptr=
      curr_akm_ptr->get_initial_SIFT_matches_ptr();
   unsigned int n_initial_matches=initial_SIFT_matches_ptr->size();
//   cout << "n_initial_matches = " << n_initial_matches << endl;

   const int min_SOH_corner_angle_matches=3;
//   const int min_SOH_corner_angle_matches=4;

   int n_hamming_rejects=0;
//   const int max_hamming_dist=9;
//   const int max_hamming_dist=10;
   const int max_hamming_dist=12;

   int n_entropy_rejects=0;

   curr_candidate_tiepoint_pairs.clear();
   for (unsigned int t=0; t<n_initial_matches; t++)
   {
//      if (t%1000==0) cout << t/1000 << " " << flush;

      int f,g;
      if (forward_feature_matching_flag)
      {

// forward searching (matching image i with image j > i):

         f=initial_SIFT_matches_ptr->at(t).first;
         g=initial_SIFT_matches_ptr->at(t).second;
      }
      else
      {
// backward searching (matching image j > i with image i):

         g=initial_SIFT_matches_ptr->at(t).first;
         f=initial_SIFT_matches_ptr->at(t).second;
      }

      descriptor* Dcurr_ptr=currimage_feature_info_ptr->at(f).second;
      descriptor* Dnext_ptr=nextimage_feature_info_ptr->at(g).second;

      if (perform_SOH_corner_angle_test_flag)
      {

// As of 4/27/13, we require at least 3 of 4 SOH corner angles to
// match before a putative tiepoint pair will be accepted as a genuine
// candidate match:

         int n_corner_angle_matches=
            count_SOH_corner_angle_matches(Dcurr_ptr,Dnext_ptr);
         if (n_corner_angle_matches < min_SOH_corner_angle_matches) continue;
      }

      if (perform_Hamming_distance_test_flag)
      {
      
// Reject tiepoint pair if its Hamming distance exceeds maximum
// threshold:

         int hamming_dist=binaryfunc::hamming_distance(Dcurr_ptr,Dnext_ptr);
         if (hamming_dist > max_hamming_dist)
         {
            n_hamming_rejects++;
            continue;
         }
      }

      if (perform_min_descriptor_entropy_test_flag)
      {
      
// As of 8/25/13, we experiment with rejecting a putative tiepoint pair
// if either of its descriptors has a low entropy as defined by Wei
// Dong in his Princeton Ph.D. thesis:

         const double min_descriptor_entropy=4.4;
         if (Dcurr_ptr->entropy() < min_descriptor_entropy ||
         Dnext_ptr->entropy() < min_descriptor_entropy)
         {
            n_entropy_rejects++;
            continue;
         }
      }
      
      descriptor* Fcurr_ptr=currimage_feature_info_ptr->at(f).first;
      descriptor* Fnext_ptr=nextimage_feature_info_ptr->at(g).first;
      curr_candidate_tiepoint_pairs.push_back(
         feature_pair(Fcurr_ptr,Fnext_ptr));
   } // loop over index t labeling tiepoint pairs
//   cout << endl;

//   double entropy_rejection_frac=double(n_entropy_rejects)/n_initial_matches;
//   cout << "n_entropy_rejects = " << n_entropy_rejects << endl;
//   cout << "entropy_rejection_frac = " << entropy_rejection_frac << endl;
//   outputfunc::enter_continue_char();

   return curr_candidate_tiepoint_pairs.size();
}
 
// ---------------------------------------------------------------------
// Member function instantiate_FLANN_index() takes in image indices i
// and j=j_start through j_stop.  It instantiates an approximate
// k-means object corresponding to images i and j and stores its
// pointer within *akm_map_ptr.  The new akm object is then loaded
// with SIFT descriptors for images i and j.  Finally, a FLANN index
// is constructed within the akm object.

// In July 2013, Marius Muja (the inventor of the FLANN library) wrote
// to us that FLANN index construction is NOT thread safe whereas
// FLANN search is thread safe.  So we must instantiate FLANN indices
// serially and NOT inside a parallel-for loop.

void sift_detector::instantiate_FLANN_index(int i,int j_start,int j_stop)
{
//   cout << "inside sift_detector::instantiate_FLANN_index(), i = " 
//        << i << endl;

   for (int j=j_start; j<j_stop; j++)
   {
      akm* curr_akm_ptr=new akm(FLANN_flag);      
      twovector IJ(i,j);
      (*akm_map_ptr)[IJ]=curr_akm_ptr;

      curr_akm_ptr->load_SIFT_descriptors(&(image_feature_info[i]));
      curr_akm_ptr->load_SIFT_descriptors2(&(image_feature_info[j]));
      if (forward_feature_matching_flag)
      {
         curr_akm_ptr->initialize_forward_SIFT_feature_search();
      }
      else
      {
         curr_akm_ptr->initialize_backward_SIFT_feature_search();
      }
   } // loop over index j labeling input images
}

// ---------------------------------------------------------------------
// Member function match_image_pair_features() matches SIFT/ASIFT
// features extracted from the ith images with those from j=j_start to
// j_stop.  

void sift_detector::match_image_pair_features(
   int i,int j_start,int j_stop,double sqrd_max_ratio,
   double worst_frac_to_reject,double max_scalar_product,
   int max_n_good_RANSAC_iters,int min_candidate_tiepoints,
   int minimal_number_of_inliers,string bundler_IO_subdir,
   map_unionfind* map_unionfind_ptr,ofstream& ntiepoints_stream)
{
//   cout << "inside sift_detector::match_image_pair_features()" << endl;

   for (int j=j_start; j<j_stop; j++)
   {
      akm* curr_akm_ptr=new akm();      
      curr_akm_ptr->load_SIFT_descriptors(get_image_feature_info_ptr(i));
      curr_akm_ptr->load_SIFT_descriptors2(get_image_feature_info_ptr(j));

      if (forward_feature_matching_flag)
      {

// FORWARD SIFT searches match SIFT features from image i with those
// from image j > i :
         curr_akm_ptr->initialize_forward_SIFT_feature_search();
      }
      else
      {

// BACKWARD SIFT searches match SIFT features from image j > i with
// those from image i:
         
         curr_akm_ptr->initialize_backward_SIFT_feature_search();
      }

      outputfunc::print_elapsed_time();
      string banner="Matching features from image "+
         stringfunc::number_to_string(i)+" with those from image "+
         stringfunc::number_to_string(j);
      outputfunc::write_banner(banner);

      vector<feature_pair> curr_candidate_tiepoint_pairs;
      int n_candidate_matches=
         identify_candidate_FLANN_feature_matches_for_image_pair(
            curr_akm_ptr,sqrd_max_ratio,
            get_image_feature_info_ptr(i),get_image_feature_info_ptr(j),
            curr_candidate_tiepoint_pairs);

      cout << "******************************************************" 
           << endl;
      cout << "i = " << i << " j = " << j 
           << " n_candidate_matches = " << n_candidate_matches << endl;
      cout << "******************************************************" 
           << endl;

      vector<feature_pair> curr_inlier_tiepoint_pairs;
      if (n_candidate_matches > 0)
      {
         fundamental curr_fundamental;

         if (identify_inlier_matches_via_fundamental_matrix(
            i,j,curr_candidate_tiepoint_pairs,curr_inlier_tiepoint_pairs,
            &curr_fundamental,
            worst_frac_to_reject,max_scalar_product,max_n_good_RANSAC_iters,
            min_candidate_tiepoints,minimal_number_of_inliers)) 
         {
            export_fundamental_matrix(&curr_fundamental,bundler_IO_subdir,i,j);
            link_matching_node_IDs(
               i,j,curr_inlier_tiepoint_pairs,map_unionfind_ptr);
            
            int n_duplicate_features=0;
            string tiepoints_subdir=bundler_IO_subdir+"tiepoints/";
            export_feature_tiepoints(
               i,j,curr_inlier_tiepoint_pairs,tiepoints_subdir,
               photogroup_ptr,n_duplicate_features,
               map_unionfind_ptr,ntiepoints_stream);
         }
      } // n_candidate_matches > 0 conditional
      
      delete curr_akm_ptr;

   } // loop over index j labeling input images
}

// ---------------------------------------------------------------------
// On 5/21/13, Davis King reminded us that the following structure is
// completely independent of the sift_detector class!  So to call
// member functions of sift_detector, we must pass in a copy of *this
// or equivalently the "this" pointer:

struct function_object_match_image_pair_features
{
   function_object_match_image_pair_features( 
      bool export_fundamental_matrices_flag_,
      int i_,double sqrd_max_ratio_,
      double worst_frac_to_reject_,double max_scalar_product_,
      int max_n_good_RANSAC_iters_,int min_candidate_tiepoints_,
      int minimal_number_of_inliers_,string bundler_IO_subdir_,
      map_unionfind* map_unionfind_ptr_,
      ofstream& ntiepoints_stream_,
      sift_detector* this_ptr_) :
      export_fundamental_matrices_flag(export_fundamental_matrices_flag_),
         i(i_),sqrd_max_ratio(sqrd_max_ratio_),
         worst_frac_to_reject(worst_frac_to_reject_),
         max_scalar_product(max_scalar_product_),
         max_n_good_RANSAC_iters(max_n_good_RANSAC_iters_),
         min_candidate_tiepoints(min_candidate_tiepoints_),
         minimal_number_of_inliers(minimal_number_of_inliers_),
         bundler_IO_subdir(bundler_IO_subdir_),
         map_unionfind_ptr(map_unionfind_ptr_),
         ntiepoints_stream(ntiepoints_stream_),
	 this_ptr(this_ptr_)  {}

      bool export_fundamental_matrices_flag;
      int i,max_n_good_RANSAC_iters,min_candidate_tiepoints,
         minimal_number_of_inliers;
      double sqrd_max_ratio,worst_frac_to_reject,max_scalar_product;
      string bundler_IO_subdir;      
      map_unionfind* map_unionfind_ptr;
      sift_detector* this_ptr;
      ofstream& ntiepoints_stream;
//      dlib::mutex m;

      void operator() (long j) const
      {
//         cout << "inside function_object_match_image_pair_features()"
//              << endl;
//         cout << "i = " << i << " j = " << j << endl;


         akm* curr_akm_ptr=new akm();      
         curr_akm_ptr->load_SIFT_descriptors(
            this_ptr->get_image_feature_info_ptr(i));
         curr_akm_ptr->load_SIFT_descriptors2(
            this_ptr->get_image_feature_info_ptr(j));

         if (this_ptr->get_forward_feature_matching_flag())
         {
// FORWARD SIFT searches match SIFT features from image i with those
// from image j > i :

            curr_akm_ptr->initialize_forward_SIFT_feature_search();
         }
         else
         {
// BACKWARD SIFT searches match SIFT features from image j > i with
// those from image i:

            curr_akm_ptr->initialize_backward_SIFT_feature_search();
         }

//         outputfunc::print_elapsed_time();
         string banner="Matching features from image "+
            stringfunc::number_to_string(i)+" with those from image "+
            stringfunc::number_to_string(j);
         outputfunc::write_banner(banner);

         vector<sift_detector::feature_pair> curr_candidate_tiepoint_pairs;
         int n_candidate_matches=
            this_ptr->identify_candidate_FLANN_feature_matches_for_image_pair(
               curr_akm_ptr,sqrd_max_ratio,
               this_ptr->get_image_feature_info_ptr(i),
               this_ptr->get_image_feature_info_ptr(j),
               curr_candidate_tiepoint_pairs);

         cout << "******************************************************" 
              << endl;
         cout << "i = " << i << " j = " << j 
              << " n_candidate_matches = " << n_candidate_matches << endl;
         cout << "******************************************************" 
              << endl;

         vector<sift_detector::feature_pair> curr_inlier_tiepoint_pairs;
         if (n_candidate_matches > 0)
         {
            fundamental curr_fundamental;

            if (this_ptr->identify_inlier_matches_via_fundamental_matrix(
               i,j,curr_candidate_tiepoint_pairs,curr_inlier_tiepoint_pairs,
               &curr_fundamental,worst_frac_to_reject,
               max_scalar_product,max_n_good_RANSAC_iters,
               min_candidate_tiepoints,minimal_number_of_inliers)) 
            {
               if (export_fundamental_matrices_flag)
                  this_ptr->export_fundamental_matrix(
                     &curr_fundamental,bundler_IO_subdir,i,j);
               this_ptr->link_matching_node_IDs(
                  i,j,curr_inlier_tiepoint_pairs,map_unionfind_ptr);

               int n_duplicate_features=0;
               string tiepoints_subdir=bundler_IO_subdir+"tiepoints/";
               this_ptr->export_feature_tiepoints(
                  i,j,curr_inlier_tiepoint_pairs,tiepoints_subdir,
                  this_ptr->get_photogroup_ptr(),n_duplicate_features,
                  map_unionfind_ptr,ntiepoints_stream);
            }

         } // n_candidate_matches > 0 conditional

         delete curr_akm_ptr;
//         cout << "Finished matching i = " << i << " with j = " << j 
//              << endl;
      }
};

// ---------------------------------------------------------------------
void sift_detector::parallel_match_image_pair_features(
   bool export_fundamental_matrices_flag,
   int i,int j_start,int j_stop,double sqrd_max_ratio,
   double worst_frac_to_reject,double max_scalar_product,
   int max_n_good_RANSAC_iters,int min_candidate_tiepoints,
   int minimal_number_of_inliers,string bundler_IO_subdir,
   map_unionfind* map_unionfind_ptr,ofstream& ntiepoints_stream)
{
   function_object_match_image_pair_features funct(
      export_fundamental_matrices_flag,
      i,sqrd_max_ratio,worst_frac_to_reject,
      max_scalar_product,max_n_good_RANSAC_iters,min_candidate_tiepoints,
      minimal_number_of_inliers,
      bundler_IO_subdir,map_unionfind_ptr,ntiepoints_stream,this);
   dlib::parallel_for(num_threads, j_start, j_stop, funct);
}

// ---------------------------------------------------------------------
// Member function match_successive_image_features() matches
// SIFT/ASIFT or SURF features extracted from images i and i+1.

void sift_detector::match_successive_image_features(
   int i,int j,double sqrd_max_ratio,
   double worst_frac_to_reject,double max_scalar_product,
   int max_n_good_RANSAC_iters,int min_candidate_tiepoints,
   int minimal_number_of_inliers,string bundler_IO_subdir,
   map_unionfind* map_unionfind_ptr,ofstream& ntiepoints_stream)
{
   cout << "inside sift_detector::match_successive_image_features()" << endl;
//   int j=i+1;

   outputfunc::print_elapsed_time();
   string banner="Matching features from image "+
      stringfunc::number_to_string(i)+" with those from image "+
      stringfunc::number_to_string(j);
   outputfunc::write_big_banner(banner);

   akm* curr_akm_ptr=new akm();      
//   cout << "i = " << i << endl;

   bool valid_input_image_pair_flag=true;
   if (!curr_akm_ptr->load_SIFT_descriptors(get_image_feature_info_ptr(i)))
   {
      valid_input_image_pair_flag=false;
   }
   
   if (!curr_akm_ptr->load_SIFT_descriptors2(get_image_feature_info_ptr(j)))
   {
      valid_input_image_pair_flag=false;
   }

   int n_candidate_matches=0;
   vector<feature_pair> curr_candidate_tiepoint_pairs;
   if (valid_input_image_pair_flag)
   {
      if (forward_feature_matching_flag)
      {

// FORWARD SIFT searches match SIFT features from image i with those
// from image j > i :
         curr_akm_ptr->initialize_forward_SIFT_feature_search();
      }
      else
      {

// BACKWARD SIFT searches match SIFT features from image j > i with
// those from image i:
         
         curr_akm_ptr->initialize_backward_SIFT_feature_search();
      }

      n_candidate_matches=
         identify_candidate_FLANN_feature_matches_for_image_pair(
            curr_akm_ptr,sqrd_max_ratio,
            get_image_feature_info_ptr(i),get_image_feature_info_ptr(j),
            curr_candidate_tiepoint_pairs);
   } // valid_input_image_pair_flag conditional
   delete curr_akm_ptr;   

   cout << "******************************************************" 
        << endl;
   cout << "i = " << i << " j = " << j 
        << " n_candidate_matches = " << n_candidate_matches << endl;
   cout << "******************************************************" 
        << endl;

   vector<feature_pair> curr_inlier_tiepoint_pairs;
   if (n_candidate_matches > 0)
   {
      fundamental curr_fundamental;
      if (identify_inlier_matches_via_fundamental_matrix(
             i,j,curr_candidate_tiepoint_pairs,curr_inlier_tiepoint_pairs,
             &curr_fundamental,
             worst_frac_to_reject,max_scalar_product,max_n_good_RANSAC_iters,
             min_candidate_tiepoints,minimal_number_of_inliers)) 
      {
         export_fundamental_matrix(&curr_fundamental,bundler_IO_subdir,i,j);
         link_matching_node_IDs(
            i,j,curr_inlier_tiepoint_pairs,map_unionfind_ptr);

         int n_duplicate_features=0;
         string tiepoints_subdir=bundler_IO_subdir+"tiepoints/";
         export_feature_tiepoints(
            i,j,curr_inlier_tiepoint_pairs,tiepoints_subdir,
            photogroup_ptr,n_duplicate_features,
            map_unionfind_ptr,ntiepoints_stream);
      } // inlier matches successfully identified conditional
   } // n_candidate_matches > 0 conditional
}


// ---------------------------------------------------------------------
void sift_detector::restricted_match_image_pair_features(
   int i,int j_start,int j_stop,vector<camera*>& camera_ptrs,
   photogroup* photogroup_ptr,double max_camera_angle_separation,
   double sqrd_max_ratio,double worst_frac_to_reject,double max_scalar_product,
   int max_n_good_RANSAC_iters,int min_candidate_tiepoints,
   int minimal_number_of_inliers,string bundler_IO_subdir,
   map_unionfind* map_unionfind_ptr,ofstream& ntiepoints_stream)
{
   cout << "inside sift_detector::match_image_pair_features()" << endl;

//   photograph* photo_i_ptr=photogroup_ptr->get_photograph_ptr(i);
   camera* camera_i_ptr=camera_ptrs[i];
   threevector point_i_hat=-camera_i_ptr->get_What();

   for (int j=j_start; j<j_stop; j++)
   {
//      photograph* photo_j_ptr=photogroup_ptr->get_photograph_ptr(j);
      camera* camera_j_ptr=camera_ptrs[j];
      threevector point_j_hat=-camera_j_ptr->get_What();

// As of 2/10/13, we believe that any two cameras whose pointing
// directions are more than (approximately)
// max_camera_angle_separation degs apart have negligible number of
// ASIFT matches:

      double dotproduct=point_i_hat.dot(point_j_hat);
      double curr_theta=acos(dotproduct);
//      cout << "Raw angle between cameras = " << curr_theta*180/PI << endl;
      if (curr_theta < max_camera_angle_separation)
      {
         akm* curr_akm_ptr=new akm();      
         curr_akm_ptr->load_SIFT_descriptors(
            get_image_feature_info_ptr(i));
         curr_akm_ptr->load_SIFT_descriptors2(
            get_image_feature_info_ptr(j));

         if (forward_feature_matching_flag)
         {

// FORWARD SIFT searches match SIFT features from image i with those
// from image j > i :

            curr_akm_ptr->initialize_forward_SIFT_feature_search();
         }
         else
         {
// BACKWARD SIFT searches match SIFT features from image j > i with
// those from image i:

            curr_akm_ptr->initialize_backward_SIFT_feature_search();
         }

//         outputfunc::print_elapsed_time();
         string banner="Matching features from image "+
            stringfunc::number_to_string(i)+" with those from image "+
            stringfunc::number_to_string(j);
         outputfunc::write_banner(banner);

         vector<feature_pair> curr_candidate_tiepoint_pairs;
         int n_candidate_matches=
            identify_candidate_FLANN_feature_matches_for_image_pair(
               curr_akm_ptr,sqrd_max_ratio,
               get_image_feature_info_ptr(i),get_image_feature_info_ptr(j),
               curr_candidate_tiepoint_pairs);

         vector<feature_pair> curr_inlier_tiepoint_pairs;
         if (n_candidate_matches > 0)
         {
            fundamental curr_fundamental;

            if (identify_inlier_matches_via_fundamental_matrix(
               i,j,curr_candidate_tiepoint_pairs,curr_inlier_tiepoint_pairs,
               &curr_fundamental,worst_frac_to_reject,
               max_scalar_product,max_n_good_RANSAC_iters,
               min_candidate_tiepoints,minimal_number_of_inliers)) 
            {
               export_fundamental_matrix(
                  &curr_fundamental,bundler_IO_subdir,i,j);
               link_matching_node_IDs(
                  i,j,curr_inlier_tiepoint_pairs,map_unionfind_ptr);

               int n_duplicate_features=0;
               string tiepoints_subdir=bundler_IO_subdir+"tiepoints/";
               export_feature_tiepoints(
                  i,j,curr_inlier_tiepoint_pairs,tiepoints_subdir,
                  photogroup_ptr,n_duplicate_features,curr_theta*180/PI,
                  map_unionfind_ptr,ntiepoints_stream);
            }
         } // n_candidate_matches > 0 conditional
      
         delete curr_akm_ptr;
      } // curr_theta < max_camera_angle_separation
      
   } // loop over index j labeling input images
}

// ---------------------------------------------------------------------
// On 5/21/13, Davis King reminded us that the following structure is
// completely independent of the sift_detector class!  So to call
// member functions of sift_detector, we must pass in a copy of *this
// or equivalently the "this" pointer:

struct function_object_restricted_match_image_pair_features
{
   function_object_restricted_match_image_pair_features( 
      bool export_fundamental_matrices_flag_,
      int i_,vector<camera*>& camera_ptrs_,photogroup* photogroup_ptr_,
      double max_camera_angle_separation_,double sqrd_max_ratio_,
      double worst_frac_to_reject_,double max_scalar_product_,
      int max_n_good_RANSAC_iters_,int min_candidate_tiepoints_,
      int minimal_number_of_inliers_,
      string bundler_IO_subdir_,map_unionfind* map_unionfind_ptr_,
      ofstream& ntiepoints_stream_,sift_detector* this_ptr_) :
         export_fundamental_matrices_flag(export_fundamental_matrices_flag_),
	 i(i_),camera_ptrs(camera_ptrs_),photogroup_ptr(photogroup_ptr_),
         max_camera_angle_separation(max_camera_angle_separation_),
         sqrd_max_ratio(sqrd_max_ratio_),
         worst_frac_to_reject(worst_frac_to_reject_),
         max_scalar_product(max_scalar_product_),
         max_n_good_RANSAC_iters(max_n_good_RANSAC_iters_),
         min_candidate_tiepoints(min_candidate_tiepoints_),
         minimal_number_of_inliers(minimal_number_of_inliers_),
         bundler_IO_subdir(bundler_IO_subdir_),
         map_unionfind_ptr(map_unionfind_ptr_),
         ntiepoints_stream(ntiepoints_stream_),
         this_ptr(this_ptr_)  {}

      bool export_fundamental_matrices_flag;
      int i,max_n_good_RANSAC_iters,min_candidate_tiepoints,
         minimal_number_of_inliers;
      double max_camera_angle_separation,sqrd_max_ratio;
      double worst_frac_to_reject,max_scalar_product;
      string bundler_IO_subdir;      
      photogroup* photogroup_ptr;
      map_unionfind* map_unionfind_ptr;
      ofstream& ntiepoints_stream;
      sift_detector* this_ptr;
      vector<camera*>& camera_ptrs;
//      dlib::mutex m;

      void operator() (long j) const
      {
//         photograph* photo_i_ptr=photogroup_ptr->get_photograph_ptr(i);
         camera* camera_i_ptr=camera_ptrs[i];
         threevector point_i_hat=-camera_i_ptr->get_What();

//         photograph* photo_j_ptr=photogroup_ptr->get_photograph_ptr(j);
         camera* camera_j_ptr=camera_ptrs[j];
         threevector point_j_hat=-camera_j_ptr->get_What();

// As of 2/10/13, we believe that any two cameras whose pointing
// directions are more than (approximately)
// max_camera_angle_separation degs apart have negligible number of
// ASIFT matches:

         double dotproduct=point_i_hat.dot(point_j_hat);
         double curr_theta=acos(dotproduct);
//         cout << "Raw angle between cameras = " << curr_theta*180/PI << endl;

         if (curr_theta < max_camera_angle_separation)
         {
            akm* curr_akm_ptr=new akm();      
            curr_akm_ptr->load_SIFT_descriptors(
               this_ptr->get_image_feature_info_ptr(i));
            curr_akm_ptr->load_SIFT_descriptors2(
               this_ptr->get_image_feature_info_ptr(j));

            if (this_ptr->get_forward_feature_matching_flag())
            {
// FORWARD SIFT searches match SIFT features from image i with those
// from image j > i :
               
               curr_akm_ptr->initialize_forward_SIFT_feature_search();
            }
            else
            {

// BACKWARD SIFT searches match SIFT features from image j > i with
// those from image i:

               curr_akm_ptr->initialize_backward_SIFT_feature_search();
            }
            
//            outputfunc::print_elapsed_time();
            string banner="Matching features from image "+
               stringfunc::number_to_string(i)+" with those from image "+
               stringfunc::number_to_string(j);
            outputfunc::write_banner(banner);

            vector<sift_detector::feature_pair> curr_candidate_tiepoint_pairs;
            int n_candidate_matches=
               this_ptr->
               identify_candidate_FLANN_feature_matches_for_image_pair(
                  curr_akm_ptr,sqrd_max_ratio,
                  this_ptr->get_image_feature_info_ptr(i),
                  this_ptr->get_image_feature_info_ptr(j),
                  curr_candidate_tiepoint_pairs);

//            cout << "n_candidate_matches = " << n_candidate_matches 
//                 << " i = " << i << " j = " << j << endl;

            vector<sift_detector::feature_pair> curr_inlier_tiepoint_pairs;
            if (n_candidate_matches > 0)
            {
               fundamental curr_fundamental;

               if (this_ptr->identify_inlier_matches_via_fundamental_matrix(
                  i,j,curr_candidate_tiepoint_pairs,curr_inlier_tiepoint_pairs,
                  &curr_fundamental,worst_frac_to_reject,
                  max_scalar_product,max_n_good_RANSAC_iters,
                  min_candidate_tiepoints,minimal_number_of_inliers)) 
               {
                  if (export_fundamental_matrices_flag)
                     this_ptr->export_fundamental_matrix(
                        &curr_fundamental,bundler_IO_subdir,i,j);
                  this_ptr->link_matching_node_IDs(
                     i,j,curr_inlier_tiepoint_pairs,map_unionfind_ptr);

                  int n_duplicate_features=0;
                  string tiepoints_subdir=bundler_IO_subdir+"tiepoints/";
                  this_ptr->export_feature_tiepoints(
                     i,j,curr_inlier_tiepoint_pairs,tiepoints_subdir,
                     this_ptr->get_photogroup_ptr(),n_duplicate_features,
                     curr_theta*180/PI,map_unionfind_ptr,ntiepoints_stream);
               }
            } // n_candidate_matches > 0 conditional

            delete curr_akm_ptr;
//            cout << "Finished matching i = " << i << " with j = " << j 
//                 << endl;
         } // curr_theta < max_camera_angle_separation conditional

//         cout << "At end of function_object_restricted_match_image_pair_features()" << endl;
      }
};

// ---------------------------------------------------------------------
void sift_detector::parallel_restricted_match_image_pair_features(
   bool export_fundamental_matrices_flag,
   int i,int j_start,int j_stop,
   vector<camera*>& camera_ptrs,photogroup* photogroup_ptr,
   double max_camera_angle_separation,double sqrd_max_ratio,
   double worst_frac_to_reject,double max_scalar_product,
   int max_n_good_RANSAC_iters,int min_candidate_tiepoints,
   int minimal_number_of_inliers,string bundler_IO_subdir,
   map_unionfind* map_unionfind_ptr,ofstream& ntiepoints_stream)
{
   function_object_restricted_match_image_pair_features funct(
      export_fundamental_matrices_flag,
      i,camera_ptrs,photogroup_ptr,max_camera_angle_separation,
      sqrd_max_ratio,worst_frac_to_reject,
      max_scalar_product,max_n_good_RANSAC_iters,
      min_candidate_tiepoints,minimal_number_of_inliers,
      bundler_IO_subdir,map_unionfind_ptr,ntiepoints_stream,this);
   dlib::parallel_for(num_threads, j_start, j_stop, funct);
 
//   cout << "At end of sift_detector::parallel_restricted_match_image_pair_features()" << endl;
}

// ---------------------------------------------------------------------
// Member function reorder_parallelized_tiepoints_file() imports an
// "ntiepoints.dat" file which we assume was generated via some
// threaded program.  It reorders the lines within the tiepoints file
// based upon image_i,image_j duples.  The reordered ntiepoints file
// is exported to tiepoints_subdir and may be easily compared with
// serial-processing counterparts.

void sift_detector::reorder_parallelized_tiepoints_file(
   string bundler_IO_subdir)
{
   string tiepoints_subdir=bundler_IO_subdir+"tiepoints/";
   string tiepoints_filename=tiepoints_subdir+"ntiepoints.dat";
   
   filefunc::ReadInfile(tiepoints_filename);

   vector<vector<string> > all_substrings=filefunc::ReadInSubstrings(
      tiepoints_filename);
   unsigned int n_lines=all_substrings.size();

   typedef std::map<DUPLE,string,ltduple> FILELINES_MAP;
   FILELINES_MAP filelines_map;
   FILELINES_MAP::iterator iter;

   for (unsigned int i=0; i<n_lines; i++)
   {
      int p=stringfunc::string_to_number(all_substrings[i].at(0));
      int q=stringfunc::string_to_number(all_substrings[i].at(1));
      DUPLE duple(p,q);
      filelines_map[duple]=filefunc::text_line[i];
   } // loop over index i labeling input file lines
   
   string output_filename=tiepoints_subdir+"reordered_ntiepoints.dat";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   outstream << "# Image i Image j N_tiepoints N_duplicates sensor_separation_angle filename_i   filename_j" << endl;
   outstream << endl;

   for (iter=filelines_map.begin(); iter != filelines_map.end(); iter++)
   {
      string curr_line=iter->second;
      outstream << curr_line << endl;
   }
   
   filefunc::closefile(output_filename,outstream);      
} 

// =========================================================================
// Inlier feature identification via fundamental matrix member functions
// =========================================================================

// Member function identify_inlier_matches_via_fundamental_matrix()

bool sift_detector::identify_inlier_matches_via_fundamental_matrix(
   int i,int j,vector<feature_pair>& curr_candidate_tiepoint_pairs,
   vector<feature_pair>& curr_inlier_tiepoint_pairs,
   fundamental* curr_fundamental_ptr,
   double worst_frac_to_reject,double max_scalar_product,
   int max_n_good_RANSAC_iters,int min_candidate_tiepoints,
   int minimal_number_of_inliers,bool ignore_triple_roots_flag)
{ 
//   cout << "inside sift_detector::identify_inlier_matches_via_fundamental_mat//rix() i = " << i << " j = " << j << endl;
//   cout << "i = " << i << " j = " << j 
//        << " curr_candidate_tiepoint_pairs.size() = "
//        << curr_candidate_tiepoint_pairs.size() << endl;

// Don't bother attempting to compute inlier feature matches if
// number of candidate features is too small:

   int n_candidate_tiepoints=curr_candidate_tiepoint_pairs.size();
   if (n_candidate_tiepoints < min_candidate_tiepoints) return false;

   vector<twovector> tiepoint_UV,tiepoint_UVmatch;
   store_tiepoint_twovectors(
      curr_candidate_tiepoint_pairs,tiepoint_UV,tiepoint_UVmatch);

   double min_RANSAC_cost=POSITIVEINFINITY;
   curr_inlier_tiepoint_pairs.clear();
   if (!compute_fundamental_matrix_via_RANSAC(
      tiepoint_UV,tiepoint_UVmatch,curr_candidate_tiepoint_pairs,
      curr_inlier_tiepoint_pairs,curr_fundamental_ptr,
      max_scalar_product,worst_frac_to_reject,
      ignore_triple_roots_flag,max_n_good_RANSAC_iters,
      minimal_number_of_inliers,min_RANSAC_cost,i,j))
   {
      cout << "Too few inliers found to reliably compute fundamental matrix"
           << endl;
      return false;
   }
   
   identify_inliers_via_fundamental_matrix(
      max_scalar_product,tiepoint_UV,tiepoint_UVmatch,
      curr_candidate_tiepoint_pairs,curr_inlier_tiepoint_pairs,
      curr_fundamental_ptr,min_RANSAC_cost,i,j);

// As of 7/26/13, we experiment with fitting a 2D affine
// transformation to provide a further check on tiepoint inliers:

   int n_affine_RANSAC_iters=500;
   vector<feature_pair> affine_inlier_tiepoint_pairs;
   int n_affine_inliers=identify_tiepoint_inliers_via_affine_transformation(
      n_affine_RANSAC_iters,curr_inlier_tiepoint_pairs,
      affine_inlier_tiepoint_pairs);

   if (n_affine_inliers < minimal_number_of_inliers) return false;

// Only return tiepoints which pass affine transformation constraint
// as genuine inlier pairs:
   
   curr_inlier_tiepoint_pairs.clear();
   for (unsigned int a=0; a<affine_inlier_tiepoint_pairs.size(); a++)
   {
      curr_inlier_tiepoint_pairs.push_back(affine_inlier_tiepoint_pairs[a]);
   }

   cout << "i = " << i << " j = " << j 
        << " n_inliers = " << max_n_inliers 
        << " n_affine_inliers = " << n_affine_inliers 
        << " n_candidate_tiepoints = " << n_candidate_tiepoints
        << " n_inliers/n_candidate_tiepoints = " 
        << double(max_n_inliers)/double(n_candidate_tiepoints)
        << endl;
//   cout << "At end of sift_detector::identify_inlier_matches_via_fundamental_matrix(), i = " << i << " j = " << j << endl;

//   outputfunc::enter_continue_char();
   
   return true;
}

// ---------------------------------------------------------------------
// Member function compute_fundamental_matrix_via_RANSAC() iteratively
// updates the fundamental matrix and inlier tiepoints via a RANSAC
// approach.  If a minimal number of inlier tiepoint pairs is not
// found, this boolean method returns false.

bool sift_detector::compute_fundamental_matrix_via_RANSAC(
   const vector<twovector>& tiepoint_UV,
   const vector<twovector>& tiepoint_UVmatch,
   vector<feature_pair>& curr_candidate_tiepoint_pairs,
   vector<feature_pair>& curr_inlier_tiepoint_pairs,
   fundamental* curr_fundamental_ptr,
   double max_scalar_product,double worst_frac_to_reject,
   bool ignore_triple_roots_flag,int max_n_good_RANSAC_iters,
   int minimal_number_of_inliers,
   double& min_RANSAC_cost,int thread_i,int thread_j)
{ 
//   cout << "inside sift_detector::compute_fundamental_matrix_via_RANSAC(), i = " << thread_i << " j = " << thread_j << endl;

//   cout << "Before entering RANSAC loop" << endl;

   max_n_inliers=0;
   int n_iters=0;
   int n_good_iters=0;
   int n_7pt_failures=0;
   while (n_good_iters < max_n_good_RANSAC_iters)
   {
      n_iters++;
      double seed=n_iters;
      dlib::random_subset_selector<feature_pair> tiepoint_samples=
         dlib::randomly_subsample(
            curr_candidate_tiepoint_pairs,7,seed);

      if (!compute_seven_point_fundamental_matrix(
         tiepoint_samples,curr_fundamental_ptr,thread_i,thread_j))
      {
         n_7pt_failures++;
//         if (thread_i==0 && thread_j==1)
//         {
//            cout << "i = " << thread_i << " j = " << thread_j
//                 << " n_7pt_failures = " << n_7pt_failures << endl;
//         }
         continue;
      }
    
//      if (n_good_iters%10==0) cout << n_good_iters/10 << " " << flush;

      identify_inliers_via_fundamental_matrix(
         max_scalar_product,tiepoint_UV,tiepoint_UVmatch,
         curr_candidate_tiepoint_pairs,curr_inlier_tiepoint_pairs,
         curr_fundamental_ptr,min_RANSAC_cost,thread_i,thread_j);

      n_good_iters++;
//      cout << "n_good_iters = " << n_good_iters << endl;
   } // while loop
   cout << endl;

//   int minimal_number_of_inliers=10;
//   int minimal_number_of_inliers=25;
//   int minimal_number_of_inliers=40;
   cout << "i = " << thread_i << " j = " << thread_j
        << " max_n_inliers = " << max_n_inliers << endl;
   if (max_n_inliers < minimal_number_of_inliers) return false;

   compute_inlier_fundamental_matrix(
      worst_frac_to_reject,curr_inlier_tiepoint_pairs,curr_fundamental_ptr,
      thread_i,thread_j);
   return true;
}

// ---------------------------------------------------------------------
// Member function compute_seven_point_fundamental_matrix()
// fills *curr_fundamental_ptr with the fundamental matrix relating
// 7 pairs of UV and U'V' coordinates randomly sampled from
// member STL vector candidate_tiepoint_pairs.

bool sift_detector::compute_seven_point_fundamental_matrix(
   const vector<int>& tiepoint_indices,
   const vector<twovector>& tiepoint_UV,
   const vector<twovector>& tiepoint_UVmatch,
   vector<feature_pair>& curr_candidate_tiepoint_pairs,
   vector<feature_pair>& curr_inlier_tiepoint_pairs,
   fundamental* curr_fundamental_ptr,
   double max_scalar_product,bool ignore_triple_roots_flag,
   double& min_RANSAC_cost,int thread_i,int thread_j)
{
//   cout << "inside sift_detector::compute_seven_point_fundamental_matrix()" 
//        << endl;
//   cout << "candidate_tiepoint_pairs.size() = " 
//        << candidate_tiepoint_pairs.size() << endl;

   vector<double> X,Y,U,V;
   for (unsigned int q=0; q<tiepoint_indices.size(); q++)
   {
      int random_index=tiepoint_indices[q];
//      cout << "random_index = " << random_index << endl;
      descriptor* F_ptr=curr_candidate_tiepoint_pairs[random_index].first;
      descriptor* Fmatch_ptr=curr_candidate_tiepoint_pairs[random_index].second;

//      cout << "q = " << q
//           << " *F_ptr = " << *F_ptr
//           << " *Fmatch_ptr = " << *Fmatch_ptr << endl;

      X.push_back(F_ptr->get(1));
      Y.push_back(F_ptr->get(2));
      U.push_back(Fmatch_ptr->get(1));
      V.push_back(Fmatch_ptr->get(2));
//      cout << "X = " << X.back() << " Y = " << Y.back()
//           << " U = " << U.back() << " V = " << V.back() << endl;
   } // loop over index q labeling quadrants

   if (!curr_fundamental_ptr->parse_fundamental_inputs(X,Y,U,V,X.size()))
   {
      return false;
   }

   unsigned int n_fundamental_candidates=
      curr_fundamental_ptr->seven_point_algorithm(
         ignore_triple_roots_flag);

// Rate-limiting step for SIFT feature matching with RANSAC is AFTER here...

// If ignore_triple_roots_flag==true, then n_fundamental_candidates=-1
// when triple root solution is encountered.  So this method should
// return false if n_fundamental_candidates < 0:

   if (n_fundamental_candidates < 0) 
   {
      return false;
   }
   else if (n_fundamental_candidates > 1)
   {
      vector<genmatrix*> F_ptrs=curr_fundamental_ptr->get_F_ptrs();

      vector<int> n_inliers;
      for (unsigned int i=0; i<n_fundamental_candidates; i++)
      {
         curr_fundamental_ptr->set_F_values(*(F_ptrs[i]));
         int n_curr_inliers=identify_inliers_via_fundamental_matrix(
            max_scalar_product,tiepoint_UV,tiepoint_UVmatch,
            curr_candidate_tiepoint_pairs,curr_inlier_tiepoint_pairs,
            curr_fundamental_ptr,min_RANSAC_cost,thread_i,thread_j);
         n_inliers.push_back(n_curr_inliers);
      } // loop over index i labeling fundamental matrix candidate

      templatefunc::Quicksort_descending(n_inliers,F_ptrs);
      curr_fundamental_ptr->set_F_values((*F_ptrs[0]));
   }

   bool fundamental_matrix_OK_flag=true;
   return fundamental_matrix_OK_flag;
}

// ---------------------------------------------------------------------
// Member function compute_seven_point_fundamental_matrix()
// fills *curr_fundamental_ptr with the fundamental matrix relating
// 7 pairs of UV and U'V' coordinates randomly sampled from
// member STL vector candidate_tiepoint_pairs.

bool sift_detector::compute_seven_point_fundamental_matrix(
   dlib::random_subset_selector<feature_pair>& tiepoint_samples,
   fundamental* curr_fundamental_ptr,int thread_i,int thread_j)
{
//   cout << "inside sift_detector::compute_seven_point_fundamental_matrix()" 
//        << endl;
//   cout << "candidate_tiepoint_pairs.size() = " 
//        << candidate_tiepoint_pairs.size() << endl;

   vector<double> X,Y,U,V;
   for (unsigned int q=0; q<tiepoint_samples.size(); q++)
   {
//      cout << "random_index = " << random_index << endl;
      descriptor* F_ptr=tiepoint_samples[q].first;
      descriptor* Fmatch_ptr=tiepoint_samples[q].second;

      X.push_back(F_ptr->get(1));
      Y.push_back(F_ptr->get(2));
      U.push_back(Fmatch_ptr->get(1));
      V.push_back(Fmatch_ptr->get(2));
//      cout << "X = " << X.back() << " Y = " << Y.back()
//           << " U = " << U.back() << " V = " << V.back() << endl;
   } // loop over index q labeling tiepoint samples

   if (!curr_fundamental_ptr->parse_fundamental_inputs(X,Y,U,V,X.size()))
   {
      return false;
   }

   bool ignore_triple_roots_flag=true;
   int n_fundamental_candidates=curr_fundamental_ptr->seven_point_algorithm(
      ignore_triple_roots_flag);

// Since ignore_triple_roots_flag==true, n_fundamental_candidates=-1
// when triple root solution is encountered.  So this method should
// return false if n_fundamental_candidates < 0:

   if (n_fundamental_candidates < 0) 
   {
      return false;
   }
   else
   {
      return true;
   }
}

// ---------------------------------------------------------------------
// Member function compute_candidate_fundamental_matrix()
// fills member *fundamental_ptr with the fundamental matrix relating
// 8 pairs of UV and U'V' coordinates randomly sampled from member STL
// vector candidate_tiepoint_pairs.

// Note added on 7/8/2013: This method is deprecated.  Use
// compute_seven_point_fundamental_matrix() instead!

bool sift_detector::compute_candidate_fundamental_matrix()
{
//   cout << "inside compute_candidate_fundamental_matrix()" << endl;
//   cout << "candidate_tiepoint_pairs.size() = " 
//        << candidate_tiepoint_pairs.size() << endl;

   vector<int> tiepoint_indices=mathfunc::random_sequence(
      candidate_tiepoint_pairs.size(),8);
   
   vector<twovector> candidate_features,matching_candidate_features;

   for (unsigned int q=0; q<8; q++)
   {
      int random_index=tiepoint_indices[q];
//      cout << "random_index = " << random_index << endl;
      descriptor* F_ptr=candidate_tiepoint_pairs[random_index].first;
      descriptor* Fmatch_ptr=candidate_tiepoint_pairs[random_index].second;
//      cout << "q = " << q
//           << " *F_ptr = " << *F_ptr
//           << " *Fmatch_ptr = " << *Fmatch_ptr << endl;

      twovector UV=recover_UV_from_F(F_ptr);
      candidate_features.push_back(UV);

      twovector UVmatch=recover_UV_from_F(Fmatch_ptr);
      matching_candidate_features.push_back(UVmatch);

//      cout << "UV = " << UV << " UVmatch = " << UVmatch << endl;
   } // loop over index q labeling quadrants

   bool fundamental_matrix_OK_flag=true;

   fundamental_ptr->parse_fundamental_inputs(
      candidate_features,matching_candidate_features);

   if (!fundamental_ptr->compute_fundamental_matrix())
   {
      fundamental_matrix_OK_flag=false;
      return fundamental_matrix_OK_flag;
   }

   if (!fundamental_ptr->check_fundamental_matrix(
      candidate_features,matching_candidate_features))
   {
      fundamental_matrix_OK_flag=false;
   }

//   cout << "fundamental_ptr->det = " 
//        << fundamental_ptr->get_F_ptr()->determinant() << endl;

   return fundamental_matrix_OK_flag;
}

// ---------------------------------------------------------------------
// Member function identify_inliers_via_fundamental_matrix() loops
// over entries within STL vector curr_candidate_tiepoint_pairs.  It
// computes the fundamental matrix scalar product for each candidate
// tiepoint pair.  If the magnitude of the scalar product is less than
// max_scalar_product, the tiepoint pair is marked as an inlier. The
// number of inliers found for the current fundamental matrix is
// returned.

// If n_inliers > max_n_inliers, this method also refills 
// curr_inlier_tiepoint_pairs with candidate_tiepoint_pairs and
// resets curr_fundamental_ptr->Fbest_ptr to equal
// curr_fundamental_ptr->F_ptr.

int sift_detector::identify_inliers_via_fundamental_matrix(
   double max_scalar_product,
   const vector<twovector>& tiepoint_UV,
   const vector<twovector>& tiepoint_UVmatch,
   const vector<feature_pair>& curr_candidate_tiepoint_pairs,
   vector<feature_pair>& curr_inlier_tiepoint_pairs,
   fundamental* curr_fundamental_ptr,double& min_RANSAC_cost,
   int thread_i,int thread_j)
{
//      cout << "inside sift_detector::identify_inliers_via_fundamental_matrix(), i = " << thread_i << " j  = " << thread_j << endl;
//      cout << "tiepoint_UV.size() = " << tiepoint_UV.size() << endl;
//      cout << "tiepoint_UVmatch.size() = " << tiepoint_UVmatch.size() << endl;
   
  unsigned int n_candidate_tiepoints=tiepoint_UVmatch.size();
   
// As of 4/27/13, we replace the number of inliers as the RANSAC score
// function with the robust error term in eqn 17 of "MLESAC: A new
// robust estimator with application to estimating image geometry" by
// Torr and Zisserman (1996):

   double curr_RANSAC_cost=0;
   vector<int> inlier_feature_indices;
   for (unsigned int f=0; f<n_candidate_tiepoints; f++)
   {
      double dperp=-1;
      if (sampson_error_flag)
      {
         dperp=curr_fundamental_ptr->sampson_error(
            tiepoint_UV[f],tiepoint_UVmatch[f]);
      }
      else
      {
         dperp=curr_fundamental_ptr->reprojection_error(
            tiepoint_UV[f],tiepoint_UVmatch[f]);
      }
//      cout << "f = " << f << " dperp = " << dperp
//           << " max_scalar_product = " << max_scalar_product << endl;
//      cout << "UV = " << UV[f] << endl;
//      cout << "UVmatch = " << UVmatch[f] << endl;
      
      if (dperp < max_scalar_product)
      {
         inlier_feature_indices.push_back(f);
         curr_RANSAC_cost += sqr(dperp);
//         curr_RANSAC_cost += 0;
      }
      else
      {
         curr_RANSAC_cost += sqr(max_scalar_product);
//         curr_RANSAC_cost += 1;
      }
   } // loop over index f labeling candidate features
   
   unsigned int n_inliers=inlier_feature_indices.size();
//   cout << "i = " << thread_i << " j = " << thread_j 
//        << " n_inliers = " << n_inliers 
//        << " max_n_inliers = " << max_n_inliers << endl;
//   cout << "curr_RANSAC_cost = " << curr_RANSAC_cost
//        << " min_RANSAC_cost = " << min_RANSAC_cost << endl;
   
   if (curr_RANSAC_cost < min_RANSAC_cost)
   {
      min_RANSAC_cost=curr_RANSAC_cost;
      max_n_inliers=n_inliers;
      curr_inlier_tiepoint_pairs.clear();
      for (unsigned int f=0; f<n_inliers; f++)
      {
         curr_inlier_tiepoint_pairs.push_back(
            curr_candidate_tiepoint_pairs[inlier_feature_indices[f]]);
      }
      curr_fundamental_ptr->set_Fbest(curr_fundamental_ptr->get_F_ptr());

//      cout << "i = " << thread_i << " j = " << thread_j 
//           << " min_RANSAC_cost = " << min_RANSAC_cost
//           << " n_inliers = " << max_n_inliers 
//           << " n_candidate_tiepoints = " << n_candidate_tiepoints
//           << " inliers/n_candidate_tiepoints = " 
//           << double(max_n_inliers)/double(n_candidate_tiepoints)
//           << endl;
//      cout << "*curr_fundamental_ptr = " << *curr_fundamental_ptr << endl;
   } // n_inliers > max_n_inliers conditional

   return n_inliers;
}

// ---------------------------------------------------------------------
void sift_detector::store_tiepoint_twovectors(
   const vector<feature_pair>& curr_candidate_tiepoint_pairs,
   vector<twovector>& tiepoint_UV,vector<twovector>& tiepoint_UVmatch)
{
//   cout << "inside sift_detector::store_tiepoint_twovectors()" 
//        << endl;

   for (unsigned int f=0; f<curr_candidate_tiepoint_pairs.size(); f++)
   {
      descriptor* F_ptr=curr_candidate_tiepoint_pairs[f].first;
      tiepoint_UV.push_back(recover_UV_from_F(F_ptr));
      
      descriptor* Fmatch_ptr=curr_candidate_tiepoint_pairs[f].second;
      tiepoint_UVmatch.push_back(recover_UV_from_F(Fmatch_ptr));
   }
}

// ---------------------------------------------------------------------
// Member function recover_inlier_tiepoints() extracts XY and UV
// coordinates for tiepoint pairs from inlier_tiepoint_pairs.  Results
// are returned within member STL vectors inlier_XY and inlier_UV.

int sift_detector::recover_inlier_tiepoints()
{
//   cout << "inside sift_detector::recover_inlier_tiepoints()" << endl;

   unsigned int n_inliers(inlier_tiepoint_pairs.size());
   cout << "n_inliers = " << n_inliers << endl;

   inlier_tiepoint_ID.clear();
   inlier_XY.clear();
   inlier_UV.clear();
   for (unsigned int i=0; i<n_inliers; i++)
   {
      descriptor* F_ptr=inlier_tiepoint_pairs[i].first;
      descriptor* Fmatch_ptr=inlier_tiepoint_pairs[i].second;

//    cout << " *F_ptr = " << *F_ptr
//           << " *Fmatch_ptr = " << *Fmatch_ptr << endl;

      int tiepoint_ID=F_ptr->get(0);

      twovector UV=recover_UV_from_F(F_ptr);
      twovector UVmatch=recover_UV_from_F(Fmatch_ptr);

      inlier_tiepoint_ID.push_back(tiepoint_ID);
      inlier_XY.push_back(UV);
      inlier_UV.push_back(UVmatch);
   }  // loop over index i labeling inlier tiepoint pairs

   return n_inliers;
}

// ---------------------------------------------------------------------
// Member function compute_inlier_fundamental_matrix() recomputes
// *fundamental_ptr using just those feature pairs stored within
// member STL vector curr_inlier_tiepoint_pairs.  It returns the
// number of inliers used to compute the fundamental matrix.

int sift_detector::compute_inlier_fundamental_matrix(
   double worst_frac_to_reject,
   vector<feature_pair>& curr_inlier_tiepoint_pairs,
   fundamental* curr_fundamental_ptr,
   int thread_i,int thread_j)
{
//   cout << "inside sift_detector::compute_inlier_fundamental_matrix(), i = " << thread_i << " j = " << thread_j << endl;
//   cout << "Initially, *curr_fundamental_ptr = " << *curr_fundamental_ptr << endl;

//   string banner="Computing inlier fundamental matrix";
//   outputfunc::write_banner(banner);

   unsigned int n_inliers=curr_inlier_tiepoint_pairs.size();
//   cout << "n_inliers = " << n_inliers << " i = " << thread_i
//        << " j = " << thread_j << endl;

   vector<double> inlier_X,inlier_Y,inlier_U,inlier_V;
   for (unsigned int i=0; i<n_inliers; i++)
   {
      descriptor* F_ptr=curr_inlier_tiepoint_pairs[i].first;
      descriptor* Fmatch_ptr=curr_inlier_tiepoint_pairs[i].second;

      inlier_X.push_back(F_ptr->get(1));
      inlier_Y.push_back(F_ptr->get(2));
      inlier_U.push_back(Fmatch_ptr->get(1));
      inlier_V.push_back(Fmatch_ptr->get(2));
   }  // loop over index i labeling inlier tiepoint pairs

   curr_fundamental_ptr->parse_fundamental_inputs(
      inlier_X,inlier_Y,inlier_U,inlier_V,n_inliers);

//   bool print_flag=true;
   bool print_flag=false;
   curr_fundamental_ptr->compute_fundamental_matrix(print_flag);

//   cout << "Before checking fundamental matrix" << endl;
//   curr_fundamental_ptr->check_fundamental_matrix(inlier_XY,inlier_UV,print_flag);
 
   curr_fundamental_ptr->renormalize_F_entries();
   return n_inliers;
}

// ---------------------------------------------------------------------
// Member function
// refine_inliers_identification_via_fundamental_matrix() loops over
// all candidate inlier tiepoint pairs.  For each pair, it computes
// the fundamental matrix scalar product for UV and UVmatch.  If 
// the scalar product's magnitude is too large, the inlier
// tiepoint pair is rejected.

void sift_detector::refine_inliers_identification_via_fundamental_matrix(
   double max_scalar_product)
{
   cout << "inside sift_detector::refine_inliers_identification_via_fundamental_matrix()" << endl;
   string banner="Refining inliers identification";
   outputfunc::write_banner(banner);

   int n_inliers(inlier_tiepoint_pairs.size());
   cout << "n_inliers = " << n_inliers << endl;

   vector<feature_pair> cleaned_inlier_tiepoint_pairs;
   for (unsigned int i=0; i<inlier_tiepoint_pairs.size(); i++)
   {
      descriptor* F_ptr=inlier_tiepoint_pairs[i].first;
      twovector UV=recover_UV_from_F(F_ptr);

      descriptor* Fmatch_ptr=inlier_tiepoint_pairs[i].second;
      twovector UVmatch=recover_UV_from_F(Fmatch_ptr);

//    cout << " *F_ptr = " << *F_ptr
//           << " *Fmatch_ptr = " << *Fmatch_ptr << endl;
//      cout << "UV = " << UV << " UVmatch = " << UVmatch << endl;

      if (fabs(fundamental_ptr->scalar_product(UV,UVmatch))
          < max_scalar_product)
      {
         cleaned_inlier_tiepoint_pairs.push_back(inlier_tiepoint_pairs[i]);
      }
   }  // loop over index i labeling candidate inlier tiepoint pairs

   cout << "Before cleaning, inlier_tiepoint_pairs.size() = "
        << inlier_tiepoint_pairs.size() << endl;

   inlier_tiepoint_pairs.clear();
   for (unsigned int i=0; i<cleaned_inlier_tiepoint_pairs.size(); i++)
   {
      inlier_tiepoint_pairs.push_back(cleaned_inlier_tiepoint_pairs[i]);
   }

   cout << "After cleaning, inlier_tiepoint_pairs.size() = "
        << inlier_tiepoint_pairs.size() << endl;
   cout << "max_scalar_product = " << max_scalar_product << endl;
}

// =========================================================================
// Inlier feature identification via affine transformation member functions
// =========================================================================

// Member function identify_tiepoint_inliers_via_affine_transformation()

int sift_detector::identify_tiepoint_inliers_via_affine_transformation(
   unsigned int n_affine_RANSAC_iters,
   const vector<feature_pair>& curr_inlier_tiepoint_pairs,
   vector<feature_pair>& affine_inlier_tiepoint_pairs)
{
//   cout << "inside sift_detector::identify_tiepoint_inliers_via_affine_trans(//)"
//        << endl;

// First load all tiepoint pairs' U,V coordinates into STL vectors Q
// and P:

   vector<twovector> Q_vecs,P_vecs;
   for (unsigned int t=0; t<curr_inlier_tiepoint_pairs.size(); t++)
   {
      descriptor* F_ptr=curr_inlier_tiepoint_pairs[t].first;
      descriptor* Fmatch_ptr=curr_inlier_tiepoint_pairs[t].second;
         
      twovector curr_q(F_ptr->get(1),F_ptr->get(2));
      twovector curr_p(Fmatch_ptr->get(1),Fmatch_ptr->get(2));
      Q_vecs.push_back(curr_q);
      P_vecs.push_back(curr_p);
   } // loop over index t labeling sample tiepoint pairs

// Perform RANSAC loop to find best affine fit to Q_vecs & P_vecs:

   int n_samples=3;
//   const double max_delta_UV=0.1;
//   const double max_delta_UV=0.05;
   const double max_delta_UV=0.025;
   affine_transform* affine_transform_ptr=new affine_transform();

   for (unsigned int iter=0; iter<n_affine_RANSAC_iters; iter++)
   {
      double seed=iter;
      dlib::random_subset_selector<feature_pair> tiepoint_samples=
         dlib::randomly_subsample(
            curr_inlier_tiepoint_pairs,n_samples,seed);

// Load sample tiepoint pairs' U,V coordinates into STL vectors q_vecs
// and p_vecs:

      vector<twovector> q_vecs,p_vecs;
      for (unsigned int s=0; s<tiepoint_samples.size(); s++)
      {
         descriptor* F_ptr=tiepoint_samples[s].first;
         descriptor* Fmatch_ptr=tiepoint_samples[s].second;

         twovector curr_q(F_ptr->get(1),F_ptr->get(2));
         twovector curr_p(Fmatch_ptr->get(1),Fmatch_ptr->get(2));
         q_vecs.push_back(curr_q);
         p_vecs.push_back(curr_p);
      } // loop over index i labeling sample tiepoint pairs

      affine_transform_ptr->parse_affine_transform_inputs(q_vecs,p_vecs);
      affine_transform_ptr->fit_affine_transformation();
//      double RMS_residual_3=
//         affine_transform_ptr->check_affine_transformation(q_vecs,p_vecs);
//      cout << "RMS_residual_3 = " << RMS_residual_3 << endl;
//      double RMS_residual_all=
//         affine_transform_ptr->check_affine_transformation(Q_vecs,P_vecs);
//      cout << "RMS_residual_all = " << RMS_residual_all << endl;

// Count number of affine inliers among entries in Q_vecs & P_vecs:

//      int n_inliers=
         affine_transform_ptr->count_affine_inliers(
            Q_vecs,P_vecs,max_delta_UV);

   } // loop over index iter labeing RANSAC iteration

   int max_n_inliers=affine_transform_ptr->get_max_n_inliers();
   vector<int> affine_inlier_indices=
      affine_transform_ptr->export_affine_inlier_indices(
         Q_vecs,P_vecs,max_delta_UV);
   delete affine_transform_ptr;

//   cout << "max_n_inliers = " << max_n_inliers
//        << " affine_inlier_indices.size() = " 
//        << affine_inlier_indices.size() << endl;

   affine_inlier_tiepoint_pairs.clear();
   for (unsigned int j=0; j<affine_inlier_indices.size(); j++)
   {
      int curr_index=affine_inlier_indices[j];
      affine_inlier_tiepoint_pairs.push_back(
         curr_inlier_tiepoint_pairs[curr_index]);
   }

   return max_n_inliers;
}

// =========================================================================
// Subregion orientation histogram member functions
// =========================================================================

// Member function check_tiepoint_corner_angles() takes in integers
// i,j labeling input images.  Scanning through all features
// (F_ptr,D_ptr) for image i and image j, it identifies those which do
// [NOT] represent tiepoint pairs.  For this subset of tiepoint pairs
// [NOT], this method computes the 4 subregion orientation histogram
// corner angles defined in 

// "VF-SIFT: Very Fast SIFT Feature Matching" by F. Alhwarin,
// D. Ristic-Durrant and A. Graser, 2010, pp 222-231.

// According to this paper, the corner angles should [NOT] match for
// genuine tiepoint pairs [NOT].  So this method tabulates the
// fractions of tiepoint pairs [NOT] which have 0, 1, 2, 3 and 4
// matching corner angles.  

// As of 4/20/13, we have empirically
// confirmed that at least 90% of genuine tiepoint pairs have 3 or 4
// matching corner angles.  In contrast, over 90% of non-tiepoint
// pairs have fewer than 3 matching corner angles!

void sift_detector::check_tiepoint_corner_angles(int i,int j)
{
   cout << "inside sift_detector::check_tiepoint_corner_angles()" << endl;

   vector<feature_pair>* currimage_feature_info_ptr=&(image_feature_info[i]);
   vector<feature_pair>* nextimage_feature_info_ptr=&(image_feature_info[j]);

   typedef map<int,feature_pair> ID_FEATUREPAIR_MAP;
// Independent int = feature_ID
// Dependent feature_pair = (F_ptr,D_ptr)   

   ID_FEATUREPAIR_MAP* curr_ID_featurepair_map_ptr=new ID_FEATUREPAIR_MAP;
   ID_FEATUREPAIR_MAP* next_ID_featurepair_map_ptr=new ID_FEATUREPAIR_MAP;
   ID_FEATUREPAIR_MAP::iterator curr_iter,next_iter;

   for (unsigned int f=0; f<currimage_feature_info_ptr->size(); f++)
   {
      descriptor* F_ptr=currimage_feature_info_ptr->at(f).first;
      int currfeature_ID=F_ptr->get(0);
      (*curr_ID_featurepair_map_ptr)[currfeature_ID]=
         currimage_feature_info_ptr->at(f);
   }

   for (unsigned int g=0; g<nextimage_feature_info_ptr->size(); g++)
   {
      descriptor* F_ptr=nextimage_feature_info_ptr->at(g).first;
      int nextfeature_ID=F_ptr->get(0);
      (*next_ID_featurepair_map_ptr)[nextfeature_ID]=
         nextimage_feature_info_ptr->at(g);
   }

   vector<int> number_matching_corners;
   for (unsigned int c=0; c<=4; c++)
   {
      number_matching_corners.push_back(0);
   }

   int n_tiepoints=0;
   for (curr_iter=curr_ID_featurepair_map_ptr->begin();
        curr_iter != curr_ID_featurepair_map_ptr->end(); curr_iter++)
   {
      int curr_feature_ID=curr_iter->first;
      next_iter=next_ID_featurepair_map_ptr->find(curr_feature_ID);

      if (next_iter==next_ID_featurepair_map_ptr->end()) continue;

      descriptor* curr_D_ptr=curr_iter->second.second;
      descriptor* next_D_ptr=next_iter->second.second;

      int n_matching_corners=count_SOH_corner_angle_matches(
         curr_D_ptr,next_D_ptr);
      number_matching_corners[n_matching_corners]=
         number_matching_corners[n_matching_corners]+1;
      n_tiepoints++;
   } // loop over curr_iter

   delete curr_ID_featurepair_map_ptr;
   delete next_ID_featurepair_map_ptr;

   for (unsigned int c=0; c<=4; c++)
   {
      double matching_frac=double(number_matching_corners[c])/
         n_tiepoints;
      cout << "c = " << c
           << " number_matching_corners[c] = "
           << number_matching_corners[c] 
           << " matching_frac = " << matching_frac << endl;
   }
}

// ---------------------------------------------------------------------
// Member function count_SOH_corner_angle_matches() takes in two
// 128-dim SIFT descriptors.  It computes 4 sub-region orientation
// histogram angles from both descriptors.  This method returns the
// number of SOH angles (0-4) that lie within 36 degrees of each
// other.

int sift_detector::count_SOH_corner_angle_matches(
   descriptor* curr_D_ptr,descriptor* next_D_ptr)
{
//   cout << "inside sift_detector::count_SOH_corner_angle_matches()" << endl;

   double curr_theta00=compute_SOH_angle(0,0,curr_D_ptr);
   double next_theta00=compute_SOH_angle(0,0,next_D_ptr);
   next_theta00=basic_math::phase_to_canonical_interval(
      next_theta00,curr_theta00-PI,curr_theta00+PI);
   double dtheta00=fabs(curr_theta00-next_theta00);

   double curr_theta03=compute_SOH_angle(0,3,curr_D_ptr);
   double next_theta03=compute_SOH_angle(0,3,next_D_ptr);
   next_theta03=basic_math::phase_to_canonical_interval(
      next_theta03,curr_theta03-PI,curr_theta03+PI);
   double dtheta03=fabs(curr_theta03-next_theta03);

   double curr_theta30=compute_SOH_angle(3,0,curr_D_ptr);
   double next_theta30=compute_SOH_angle(3,0,next_D_ptr);
   next_theta30=basic_math::phase_to_canonical_interval(
      next_theta30,curr_theta30-PI,curr_theta30+PI);
   double dtheta30=fabs(curr_theta30-next_theta30);

   double curr_theta33=compute_SOH_angle(3,3,curr_D_ptr);
   double next_theta33=compute_SOH_angle(3,3,next_D_ptr);
   next_theta33=basic_math::phase_to_canonical_interval(
      next_theta33,curr_theta33-PI,curr_theta33+PI);
   double dtheta33=fabs(curr_theta33-next_theta33);

// Count number of corner pairs which lie close to each other in
// angle space:

   int n_matching_SOH_corners=0;
   if (dtheta00 < 36*PI/180) n_matching_SOH_corners++;
   if (dtheta03 < 36*PI/180) n_matching_SOH_corners++;
   if (dtheta30 < 36*PI/180) n_matching_SOH_corners++;
   if (dtheta33 < 36*PI/180) n_matching_SOH_corners++;

//   cout << "Feature ID = " << curr_feature_ID << endl;
//      cout << "curr_theta00 = " << curr_theta00*180/PI
//           << " next_theta00 = " << next_theta00*180/PI << endl;

//      cout << "curr_theta03 = " << curr_theta03*180/PI
//           << " next_theta03 = " << next_theta03*180/PI << endl;
//      cout << "curr_theta30 = " << curr_theta30*180/PI
//           << " next_theta30 = " << next_theta30*180/PI << endl;
//      cout << "curr_theta33 = " << curr_theta33*180/PI
//           << " next_theta33 = " << next_theta33*180/PI << endl;
//   cout << "dtheta00 = " << dtheta00*180/PI << endl;
//   cout << "dtheta03 = " << dtheta03*180/PI << endl;
//   cout << "dtheta30 = " << dtheta30*180/PI << endl;
//   cout << "dtheta33 = " << dtheta33*180/PI << endl;
//   cout << "n_matching_SOH_corners = " << n_matching_SOH_corners << endl;
//   cout << endl;

   return n_matching_SOH_corners;
}

// ---------------------------------------------------------------------
// Member function compute_SOH_angle() takes in SIFT descriptor *D_ptr
// along with indices 0 <= i,j <= 3 that label one of its 16
// subregions.  This method computes the descriptor subregion's angle
// as defined in eqn (1) of "VF-SIFT: Very Fast SIFT Feature Matching"
// by F. Alhwarin et al.  The returned SOH angle is forced to lie
// within the interval [-PI,PI].

double sift_detector::compute_SOH_angle(int i,int j,descriptor* D_ptr)
{
//   cout << "inside sift_detector::compute_SOH_angle()" << endl;
   
   double numer=0;
   double denom=0;
   int start_index=8*(i*4+j);
   for (unsigned int d=0; d<8; d++)
   {
      double mag=D_ptr->get(start_index+d);
      numer += mag*sin_orientation[d];
      denom += mag*cos_orientation[d];
   } // loop over index d labeling 8 subregion orientations
   
   double theta=atan2(numer,denom);
   theta=basic_math::phase_to_canonical_interval(theta,-PI,PI);
   if (nearly_equal(theta,PI,1E-8)) theta=-PI;

   return theta;
}

// ---------------------------------------------------------------------
// Member function quantize_SOH_corner_angles() takes in a SIFT
// feature (F_ptr,D_ptr) within input current_feature_pair.  It
// computes the descriptor's four corner angles.  Each angle is then
// quantized into 1 of SOH_binsize=24 15-degree bins.  The four
// quantized bin integers are returned within a quadruple data
// structure.

void sift_detector::quantize_SOH_corner_angles(
   feature_pair& current_feature_pair,quadruple& current_quadruple)
{
//   cout << "inside sift_detector::quantize_SOH_corner_angles()" << endl;

   descriptor* D_ptr=current_feature_pair.second;

   vector<double> theta;
   theta.push_back(compute_SOH_angle(0,0,D_ptr));
   theta.push_back(compute_SOH_angle(0,3,D_ptr));
   theta.push_back(compute_SOH_angle(3,0,D_ptr));
   theta.push_back(compute_SOH_angle(3,3,D_ptr));

   current_quadruple.first=basic_math::mytruncate((theta[0]-(-PI))/
      SOH_binsize);
   current_quadruple.second=basic_math::mytruncate((theta[1]-(-PI))/
      SOH_binsize);
   current_quadruple.third=basic_math::mytruncate((theta[2]-(-PI))/
      SOH_binsize);
   current_quadruple.fourth=basic_math::mytruncate((theta[3]-(-PI))/
      SOH_binsize);
}

// ---------------------------------------------------------------------
// Member function quantize_currimage_feature_info() loops over all
// (F_ptr,D_ptr) feature pairs within input STL vector
// currimage_feature_info.  For each input SIFT feature, it computes
// quantized integer indices for the descriptor's 4 SOH corners.  The
// entire set of input feature pairs are stored as functions of quantized 
// quadruple SOH corner indices in STL map member
// curr_SOH_corner_descriptor_map.

void sift_detector::quantize_currimage_feature_info(
   vector<feature_pair>& currimage_feature_info)
{
   cout << "inside sift_detector::quantize_currimage_feature_info()" << endl;

   curr_SOH_corner_descriptor_map.clear();

   quadruple curr_quadruple;
   for (unsigned int f=0; f<currimage_feature_info.size(); f++)
   {
      quantize_SOH_corner_angles(currimage_feature_info[f],curr_quadruple);
//      cout << "f = " << f << " curr_quadruple = "
//           << curr_quadruple << endl;

      curr_SOH_corner_iter=curr_SOH_corner_descriptor_map.find(curr_quadruple);
      if (curr_SOH_corner_iter==curr_SOH_corner_descriptor_map.end())
      {
         vector<feature_pair> V;
         V.push_back(currimage_feature_info[f]);
         curr_SOH_corner_descriptor_map[curr_quadruple]=V;
      }
      else
      {
         curr_SOH_corner_iter->second.push_back(currimage_feature_info[f]);
      }
   } // loop over index f

   cout << "curr_SOH_corner_descriptor_map.size() = "
        << curr_SOH_corner_descriptor_map.size() << endl;

   vector<double> number_features;
   for (curr_SOH_corner_iter=curr_SOH_corner_descriptor_map.begin();
        curr_SOH_corner_iter != curr_SOH_corner_descriptor_map.end();
        curr_SOH_corner_iter++)
   {
      curr_quadruple=curr_SOH_corner_iter->first;

      int n_features=curr_SOH_corner_iter->second.size();
//      cout << "curr_quadruple = " << curr_quadruple
//           << " n_features = " << n_features << endl;

      number_features.push_back(n_features);
   }

   prob_distribution prob_features(number_features,100);
   prob_features.writeprobdists(false);   
//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function quantize_cumimage_feature_info() loops over all
// (F_ptr,D_ptr) feature pairs within input STL vector
// cumimage_feature_info.  For each input SIFT feature, it computes
// quantized integer indices for the descriptor's 4 SOH corners.  The
// entire set of input feature pairs are stored as functions of quantized 
// quadruple SOH corner indices in STL map member
// cum_SOH_corner_descriptor_map.

void sift_detector::quantize_cumimage_feature_info(
   vector<feature_pair>& cumimage_feature_info)
{
   cout << "inside sift_detector::quantize_cumimage_feature_info()" << endl;

   cum_SOH_corner_descriptor_map.clear();

   quadruple cum_quadruple;
   for (unsigned int f=0; f<cumimage_feature_info.size(); f++)
   {
      quantize_SOH_corner_angles(cumimage_feature_info[f],cum_quadruple);

      cum_SOH_corner_iter=cum_SOH_corner_descriptor_map.find(cum_quadruple);
      if (cum_SOH_corner_iter==cum_SOH_corner_descriptor_map.end())
      {
         vector<feature_pair> V;
         V.push_back(cumimage_feature_info[f]);
         cum_SOH_corner_descriptor_map[cum_quadruple]=V;
      }
      else
      {
         cum_SOH_corner_iter->second.push_back(cumimage_feature_info[f]);
      }
   } // loop over index f

   cout << "cum_SOH_corner_descriptor_map.size() = "
        << cum_SOH_corner_descriptor_map.size() << endl;
}

// ---------------------------------------------------------------------
// Member function
// identify_candidate_SOH_feature_matches_for_image_pair()

int sift_detector::identify_candidate_SOH_feature_matches_for_image_pair(
   const vector<feature_pair>& currimage_feature_info,
   const vector<feature_pair>& nextimage_feature_info)
{
//   cout << "inside sift_detector::identify_candidate_SOH_feature_matches_for_image_pair()"
//        << endl;
//   outputfunc::print_elapsed_time();

   int n_matches=0;
   unsigned int max_hamming_dist=1;
//   int max_hamming_dist=5;
//   int max_hamming_dist=12;
   for (unsigned int f=0; f<currimage_feature_info.size(); f++)
   {
      descriptor* Fcurr_ptr=currimage_feature_info[f].first;
      descriptor* Dcurr_ptr=currimage_feature_info[f].second;
      for (unsigned int g=0; g<nextimage_feature_info.size(); g++)
      {
         descriptor* Fnext_ptr=nextimage_feature_info[g].first;
         descriptor* Dnext_ptr=nextimage_feature_info[g].second;

         unsigned long hamming_dist=binaryfunc::hamming_distance(
            Dcurr_ptr,Dnext_ptr);
         if (hamming_dist > max_hamming_dist) continue;
         
/*
         double delta_Dmagnitude=(*Dcurr_ptr - *Dnext_ptr).magnitude();
//         const double max_delta_Dmagnitude=50;
         const double max_delta_Dmagnitude=100;
//         const double max_delta_Dmagnitude=150;
//         const double max_delta_Dmagnitude=200;
         if (delta_Dmagnitude > max_delta_Dmagnitude) continue;

         double dotproduct=Dcurr_ptr->dot(*Dnext_ptr);
//         const double min_dotproduct=250000.0;
         const double min_dotproduct=255000.0;
         if (dotproduct < min_dotproduct) continue;

         double max_coordinate_diff=0;
         for (unsigned int d=0; d<128; d++)
         {
            double curr_coordinate_diff=
               fabs(Dcurr_ptr->get(d)-Dnext_ptr->get(d));
            max_coordinate_diff=basic_math::max(
               max_coordinate_diff,curr_coordinate_diff);
         }
         const double maximum_coord_diff=50;
//         const double maximum_coord_diff=60;
         if (max_coordinate_diff > maximum_coord_diff) continue;
*/

/*       
         int previous_next_image_ID=-1;
         if (candidate_tiepoint_pairs.size() >= 1)
         {
            previous_next_image_ID=
               candidate_tiepoint_pairs.back().second->get(11);
         }
*/
         candidate_tiepoint_pairs.push_back(feature_pair(Fcurr_ptr,Fnext_ptr));

         n_matches++;

      } // loop over index g
   } // loop over index f

//   cout << "n_matches = " << n_matches << endl;
//   cout << endl;

   return n_matches;
}

// ---------------------------------------------------------------------
// Member function renormalize_quadruple_indices() takes in
// curr_quadruple and resets all its entries to range from 0 to
// max_quadruple_index-1.

void sift_detector::renormalize_quadruple_indices(quadruple& curr_quadruple)
{
   for (unsigned int c=0; c<4; c++)
   {
      curr_quadruple.first=modulo(curr_quadruple.first,max_quadruple_index);
      curr_quadruple.second=modulo(curr_quadruple.second,max_quadruple_index);
      curr_quadruple.third=modulo(curr_quadruple.third,max_quadruple_index);
      curr_quadruple.fourth=modulo(curr_quadruple.fourth,max_quadruple_index);
   }
}

// ---------------------------------------------------------------------
// Member function quadruple_neighborhood() takes in quadruple
// labeling some sub-orientation histogram cell.  It adds/subtracts
// unity from each of the quadruple's 4-dimensional integer indices.
// The set of 8 neighboring quadruples are returned within an STL
// vector of quadruples.

vector<quadruple> sift_detector::quadruple_neighborhood(
   quadruple& curr_quadruple)
{
   renormalize_quadruple_indices(curr_quadruple);

   vector<quadruple> quadruple_neighbors;

   for (int c=-1; c<=1; c += 2)
   {
      quadruple neighbor_quadruple=curr_quadruple;
      neighbor_quadruple.first=neighbor_quadruple.first+c;
      renormalize_quadruple_indices(neighbor_quadruple);
      quadruple_neighbors.push_back(neighbor_quadruple);

      neighbor_quadruple=curr_quadruple;
      neighbor_quadruple.first=neighbor_quadruple.second+c;
      renormalize_quadruple_indices(neighbor_quadruple);
      quadruple_neighbors.push_back(neighbor_quadruple);

      neighbor_quadruple=curr_quadruple;
      neighbor_quadruple.first=neighbor_quadruple.third+c;
      renormalize_quadruple_indices(neighbor_quadruple);
      quadruple_neighbors.push_back(neighbor_quadruple);

      neighbor_quadruple=curr_quadruple;
      neighbor_quadruple.first=neighbor_quadruple.fourth+c;
      renormalize_quadruple_indices(neighbor_quadruple);
      quadruple_neighbors.push_back(neighbor_quadruple);
   }

   return quadruple_neighbors;
}

// ---------------------------------------------------------------------
// Member function curr_feature_SOH_neighborhood() takes in the quadruple
// for some particular sub-orientation histogram cell.  If the cell is
// nonempty, it fills output STL vector curr_feature_neighborhood_info
// with all feature pairs corresponding to the cell and its immediate
// 4D neighbors.  

int sift_detector::curr_feature_SOH_neighborhood(
   quadruple& curr_quadruple,
   vector<feature_pair>& curr_feature_neighborhood_info)
{
//   cout << "inside sift_detector::curr_feature_SOH_neighborhood()" << endl;

   curr_feature_neighborhood_info.clear();

   SOH_CORNER_DESCRIPTOR_MAP::iterator iter=
      curr_SOH_corner_descriptor_map.find(curr_quadruple);
   if (iter==curr_SOH_corner_descriptor_map.end()) return 0;

   curr_feature_neighborhood_info=iter->second;

/*
   vector<quadruple> quadruple_neighbors=quadruple_neighborhood(
      curr_quadruple);
   for (unsigned int i=0; i<quadruple_neighbors.size(); i++)
   {
      iter=curr_SOH_corner_descriptor_map.find(
         quadruple_neighbors[i]);
      if (iter==curr_SOH_corner_descriptor_map.end()) continue;

      for (unsigned int j=0; j<iter->second.size(); j++)
      {
         curr_feature_neighborhood_info.push_back(iter->second.at(j));
      }
   } // loop over index i labeling neighboring quadruples
*/

   return curr_feature_neighborhood_info.size();
}

// ---------------------------------------------------------------------
int sift_detector::cum_feature_SOH_neighborhood(
   quadruple& cum_quadruple,
   vector<feature_pair>& cum_feature_neighborhood_info)
{
//   cout << "inside sift_detector::cum_feature_SOH_neighborhood()" << endl;

   cum_feature_neighborhood_info.clear();

   SOH_CORNER_DESCRIPTOR_MAP::iterator iter=
      cum_SOH_corner_descriptor_map.find(cum_quadruple);
   if (iter==cum_SOH_corner_descriptor_map.end()) return 0;

   cum_feature_neighborhood_info=iter->second;

/*
   vector<quadruple> quadruple_neighbors=quadruple_neighborhood(
      cum_quadruple);
   for (unsigned int i=0; i<quadruple_neighbors.size(); i++)
   {
      iter=cum_SOH_corner_descriptor_map.find(
         quadruple_neighbors[i]);
      if (iter==cum_SOH_corner_descriptor_map.end()) continue;

      for (unsigned int j=0; j<iter->second.size(); j++)
      {
         cum_feature_neighborhood_info.push_back(iter->second.at(j));
      }
   } // loop over index i labeling neighboring quadruples
*/

   return cum_feature_neighborhood_info.size();
}

// =========================================================================
// SIFT feature matching via homography member functions:
// =========================================================================

// Member function identify_candidate_feature_matches loops over all
// image pairs labeled by indices i and j.  For each pair, it
// identifies candidate one-to-one and onto feature matches based upon
// Lowe's ratio test.  It then bins the candidate matching features
// into quadrants surrounding the median point within image i.
// 4-point homographies are iteratively constructed from candidate
// tiepoint pairs randomly pulled from different quadrants.  Following
// the RANSAC procedure, we use the homographies to define inlier
// tiepoint pairs.  A final homography relating image i to image j is
// calculated based upon just inlier tiepoint pairs.  IDs for SIFT
// features extracted from image i and j are renamed so that they
// reflect tiepoint relationships.

// Feature track information is iteratively exported to output files
// as the processing for each image is finished.

void sift_detector::identify_candidate_feature_matches_via_homography(
   int n_min_quadrant_features,double sqrd_max_ratio,
   double worst_frac_to_reject,double max_sqrd_delta)
{ 
   cout << "inside sift_detector::identify_candidate_feature_matches_via_homography()" 
        << endl;

   unsigned int istart=0;
   unsigned int istop=n_images-1;
   
   for (unsigned int i=istart; i<istop; i++)
   {
      for (unsigned int j=i+1; j<n_images; j++)
      {
         identify_candidate_feature_matches_via_homography(
            i,j,j,
            n_min_quadrant_features,sqrd_max_ratio,
            worst_frac_to_reject,max_sqrd_delta);
      } // loop over index j labeling images
      export_feature_tracks(i);
   } // loop over index i labeling images
   export_feature_tracks(istop);
}

// ---------------------------------------------------------------------
// Member function identify_candidate_feature_matches_via_homography() 
// takes in image indices i and j.  It first instantiates Kdtrees
// within members *ANN_ptr and *inverse_ANN_ptr to hold (F,D) feature
// information for image j and image i, respectively.  This method
// then searches for one-to-one and onto pairings between the features

bool sift_detector::identify_candidate_feature_matches_via_homography(
   int i,int jstart,int jstop,
   int n_min_quadrant_features,double sqrd_max_ratio,
   double worst_frac_to_reject,double max_sqrd_delta)
{ 
   cout << "inside sift_detector::identify_candidate_feature_matches_via_homography()" 
        << endl;
   cout << "i = " << i << " jstart = " << jstart
        << " jstop = " << jstop << endl;
   
// First load features for image i into *inverse_ANN_ptr:

   vector<feature_pair> currimage_feature_info=image_feature_info[i];
   inverse_ANN_ptr->load_data_points(currimage_feature_info);

// Next load all features for images jstart through jstop into
// *ANN_ptr:

   vector<feature_pair> cumimage_feature_info;
   for (int j=jstart; j<=jstop; j++)
   {
      vector<feature_pair> nextimage_feature_info=image_feature_info[j];
      for (unsigned int k=0; k<nextimage_feature_info.size(); k++)
      {
         cumimage_feature_info.push_back(nextimage_feature_info[k]);
      }
   }
   ANN_ptr->load_data_points(cumimage_feature_info);

// Loop over features in image i and search for counterparts in images
// jstart through jstop:
   
   cout << "identifying candidate features matches for image pair" 
        << endl;
   identify_candidate_feature_matches_for_image_pair(
      currimage_feature_info,cumimage_feature_info,sqrd_max_ratio);
 
   int n_candidate_tiepoints=candidate_tiepoint_pairs.size();
   if (jstart==jstop)
   {
      cout << "image i = " << i << " image j = " << jstart << endl;
   }
   else
   {
      cout << "image i = " << i << " image jstart = " << jstart 
           << " image jstop = " << jstop << endl;
   }
   cout << "candidate_tiepoint_pairs.size() = " 
        << candidate_tiepoint_pairs.size() << endl;
   const unsigned int min_n_candidate_tiepoints=10;
   if (candidate_tiepoint_pairs.size() < min_n_candidate_tiepoints) 
      return false;

//   outputfunc::enter_continue_char();

   double bbox_area=bin_features_into_quadrants(n_min_quadrant_features);
   if (nearly_equal(bbox_area,0)) return false;

   cout << "Before entering RANSAC loop" << endl;

   max_n_inliers=0;
   int n_iters=0;
   int n_good_iters=0;
//   int max_n_good_RANSAC_iters=100;
//   int max_n_good_RANSAC_iters=300;
//   int max_n_good_RANSAC_iters=500;
//   int max_n_good_RANSAC_iters=1000;
//   int max_n_good_RANSAC_iters=5000;
   int max_n_good_RANSAC_iters=10000;
   inlier_tiepoint_pairs.clear();
   while (n_good_iters < max_n_good_RANSAC_iters)
   {

// Try to avoid RANSAC from entering effective infinite loop.  If a
// candidate homography has not been found after 10*max_n_good_RANSAC_iters,
// then reduce bbox_area by factor of 2 and try again:

      n_iters++;
      if (n_iters > 10*max_n_good_RANSAC_iters)
      {
         n_iters=0;
         bbox_area *= 0.5;
         cout << "Bbox area reduced to " << bbox_area << endl;
      }
      
      if (!compute_candidate_homography(bbox_area)) continue;
      if (n_good_iters%10==0) cout << n_good_iters/10 << " " << flush;
      bool print_flag=false;
      identify_inliers_via_homography(
         candidate_tiepoint_pairs,max_sqrd_delta,print_flag);
      n_good_iters++;
   }
   cout << endl;

   cout << "After exiting RANSAC loop" << endl;

   cout << "n_inliers = " << max_n_inliers 
        << " n_candidate_tiepoints = " << n_candidate_tiepoints
        << " n_inliers/n_candidate_tiepoints = " 
        << double(max_n_inliers)/double(n_candidate_tiepoints)
        << endl;

   compute_inlier_homography(worst_frac_to_reject);
   refine_inliers_identification(max_sqrd_delta);

   for (int j=jstart; j<=jstop; j++)
   {
      rename_feature_IDs(i,j);
   }

//   outputfunc::enter_continue_char();
   return true;
}

// ---------------------------------------------------------------------
// Member function identify_inlier_matches_via_homography()

bool sift_detector::identify_inlier_matches_via_homography(
   int i,int j,int max_n_good_RANSAC_iters,
   double worst_frac_to_reject,double max_sqrd_delta)
{ 
   return identify_inlier_matches_via_homography(
      i,j,max_n_good_RANSAC_iters,worst_frac_to_reject,max_sqrd_delta,
      candidate_tiepoint_pairs);
}

// ---------------------------------------------------------------------
// Member function identify_inlier_matches_via_homography()

bool sift_detector::identify_inlier_matches_via_homography(
   int i,int j,int max_n_good_RANSAC_iters,
   double worst_frac_to_reject,double max_sqrd_delta,
   vector<feature_pair>& curr_candidate_tiepoint_pairs)
{ 
   cout << "inside sift_detector::identify_feature_matches_via_homography()" 
        << endl;
   cout << "i = " << i << " j = " << j << endl;
   
   if (!compute_homography_via_RANSAC(
      curr_candidate_tiepoint_pairs,max_n_good_RANSAC_iters,max_sqrd_delta))
   {
      cout << "Too few inliers found to reliably compute homography"
           << endl;
      return false;
   }
 
   identify_inliers_via_homography(
      curr_candidate_tiepoint_pairs,max_sqrd_delta);

   int n_candidate_tiepoints=candidate_tiepoint_pairs.size();
   cout << "n_inliers = " << max_n_inliers 
        << " n_candidate_tiepoints = " << n_candidate_tiepoints 
        << " n_inliers/n_candidate_tiepoints = " 
        << double(max_n_inliers)/double(n_candidate_tiepoints)
        << endl;

   rename_feature_IDs(i,j);
   return true;
}

// ---------------------------------------------------------------------
// Member function compute_homography_via_RANSAC

bool sift_detector::compute_homography_via_RANSAC(
   vector<feature_pair>& curr_candidate_tiepoint_pairs,
   int max_n_good_RANSAC_iters,double max_sqrd_delta)
{
   cout << "inside compute_homography_via_RANSAC()" << endl;

   max_n_inliers=0;
   int n_good_iters=0;

//   int max_n_good_RANSAC_iters=100;	       
//   int max_n_good_RANSAC_iters=1000;	       
//   int max_n_good_RANSAC_iters=5000;	       
//   int max_n_good_RANSAC_iters=15000;	       
//   int max_n_good_RANSAC_iters=25000;	       
   inlier_tiepoint_pairs.clear();

   while (n_good_iters < max_n_good_RANSAC_iters)
   {
      compute_four_point_homography(curr_candidate_tiepoint_pairs);
      identify_inliers_via_homography(
         curr_candidate_tiepoint_pairs,max_sqrd_delta);
      n_good_iters++;
//      cout << "n_good_iters = " << n_good_iters << endl;
   } // while loop
   cout << endl;

   int minimal_number_of_inliers=10;
//   int minimal_number_of_inliers=25;
   if (max_n_inliers < minimal_number_of_inliers) return false;

   double worst_frac_to_reject=0;
   compute_inlier_homography(worst_frac_to_reject);
   return true;
}

// ---------------------------------------------------------------------
// Member function compute_four_point_homography()

void sift_detector::compute_four_point_homography(
   vector<feature_pair>& curr_candidate_tiepoint_pairs)
{
//   cout << "inside sift_detector::compute_four_point_homography()" << endl;

   if (curr_candidate_tiepoint_pairs.size() < 4)
   {
      cout << "Error in sift_detector::compute_four_point_homography()!" 
           << endl;
      cout << "curr_candidate_tiepoint_pairs.size() = "
           << curr_candidate_tiepoint_pairs.size() << endl;
      exit(-1);
   }
   
   vector<int> tiepoint_indices=mathfunc::random_sequence(
      curr_candidate_tiepoint_pairs.size(),4);
   
   vector<twovector> XY,UV;
   for (unsigned int q=0; q<tiepoint_indices.size(); q++)
   {
      int random_index=tiepoint_indices[q];
//      cout << "random_index = " << random_index << endl;
      descriptor* F_ptr=curr_candidate_tiepoint_pairs[random_index].first;
      descriptor* Fmatch_ptr=
         curr_candidate_tiepoint_pairs[random_index].second;
//      cout << "q = " << q
//           << " *F_ptr = " << *F_ptr
//           << " *Fmatch_ptr = " << *Fmatch_ptr << endl;

      XY.push_back(twovector(F_ptr->get(1),F_ptr->get(2)));
      UV.push_back(twovector(Fmatch_ptr->get(1),Fmatch_ptr->get(2)));
   } // loop over index q labeling quadrants

   H_ptr->parse_homography_inputs(XY,UV);
   H_ptr->compute_homography_matrix();
}

// ---------------------------------------------------------------------
// Member function identify_inliers_via_homography() loops over all
// candidate features within image i and computes their projections
// into image j via homography member *H_ptr.  If the squared distance
// between the projection and its candidate tiepoint counterpart is
// less than sqrt(max_sqrd_delta), the tiepoint pair is counted as an
// inlier.

void sift_detector::identify_inliers_via_homography(
   vector<feature_pair>& curr_candidate_tiepoint_pairs,
   double max_sqrd_delta,bool print_flag,bool feature_ray_matching_flag)
{
//   cout << "inside identify_inliers_via_homography()" << endl;

   unsigned int n_candidate_tiepoints=curr_candidate_tiepoint_pairs.size();
   vector<int> inlier_feature_indices;
   double sqrd_delta;
   for (unsigned int f=0; f<n_candidate_tiepoints; f++)
   {
      descriptor* F_ptr=curr_candidate_tiepoint_pairs[f].first;
      twovector UV(recover_UV_from_F(F_ptr));
      
      if (feature_ray_matching_flag)
      {
         threevector n_hat(recover_nhat_from_F(F_ptr));
         twovector UVprojected=
            H_ptr->project_ray_to_image_plane(n_hat);
         sqrd_delta=(UV-UVprojected).sqrd_magnitude();
      }
      else
      {
         descriptor* Fmatch_ptr=curr_candidate_tiepoint_pairs[f].second;
         twovector UVmatch=recover_UV_from_F(Fmatch_ptr);
         
         twovector UVprojected=
            H_ptr->project_world_plane_to_image_plane(UV);
         sqrd_delta=(UVmatch-UVprojected).sqrd_magnitude();
      }

      if (sqrd_delta < max_sqrd_delta)
      {
         inlier_feature_indices.push_back(f);
      }
   } // loop over index f labeling candidate features
   
   int n_inliers=int(inlier_feature_indices.size());
//   cout << "n_inliers = " << n_inliers << endl;

   if (n_inliers > max_n_inliers)
   {
      max_n_inliers=n_inliers;
      inlier_tiepoint_pairs.clear();
      for (unsigned int f=0; f<inlier_feature_indices.size(); f++)
      {
         inlier_tiepoint_pairs.push_back(
            curr_candidate_tiepoint_pairs[inlier_feature_indices[f]]);
      }

      if (print_flag)
      {
         cout << "max_n_inliers = " << max_n_inliers 
              << " n_candidate_tiepoints = " << n_candidate_tiepoints 
              << " inliers/candidate_tiepoints = " 
              << double(max_n_inliers)/double(n_candidate_tiepoints)
              << endl;
      }
   } // n_inliers > max_n_inliers conditional
}

// ---------------------------------------------------------------------
// Member function compute_inlier_homography() recomputes homography
// *H_ptr using just those feature pairs stored within member STL
// vector inlier_tiepoint_pairs. 

void sift_detector::compute_inlier_homography(
   double worst_frac_to_reject,bool feature_ray_matching_flag)
{
//   cout << "inside sift_detector::compute_inlier_homography()" << endl;
   string banner="Computing inlier homography";
   outputfunc::write_banner(banner);

   unsigned int n_inliers(inlier_tiepoint_pairs.size());
   cout << "n_inliers = " << n_inliers << endl;

   inlier_XY.clear();
   inlier_UV.clear();
   vector<feature_pair> features_rays;
   for (unsigned int i=0; i<n_inliers; i++)
   {
      descriptor* F_ptr=inlier_tiepoint_pairs[i].first;
      descriptor* Fmatch_ptr=inlier_tiepoint_pairs[i].second;

//    cout << " *F_ptr = " << *F_ptr
//           << " *Fmatch_ptr = " << *Fmatch_ptr << endl;

      if (feature_ray_matching_flag)
      {
         features_rays.push_back(feature_pair(F_ptr,Fmatch_ptr));
      }
      else
      {
         twovector UV=recover_UV_from_F(F_ptr);
         twovector UVmatch=recover_UV_from_F(Fmatch_ptr);

         inlier_XY.push_back(UV);
         inlier_UV.push_back(UVmatch);
      }
   }  // loop over index i labeling inlier tiepoint pairs

   bool print_flag=false;
   if (feature_ray_matching_flag)
   {
      double input_frac_to_use=1.0;
      bool check_ray_feature_homography_flag=true;
      compute_ray_feature_homography(&features_rays,input_frac_to_use,
                                     check_ray_feature_homography_flag);
   }
   else
   {
      H_ptr->parse_homography_inputs(inlier_XY,inlier_UV);
      H_ptr->compute_homography_matrix();
      H_ptr->check_homography_matrix(
         inlier_XY,inlier_UV,print_flag);
   }

// Recompute inlier homography after rejecting worst feature pairs:

   double best_frac_to_keep=1-worst_frac_to_reject;
   int reduced_n_inliers=best_frac_to_keep*n_inliers;

   if (feature_ray_matching_flag)
   {
      compute_ray_feature_homography(&features_rays,best_frac_to_keep);
   }
   else
   {
      H_ptr->parse_homography_inputs(
         H_ptr->get_XY_sorted(),H_ptr->get_UV_sorted(),reduced_n_inliers);
      H_ptr->compute_homography_matrix();
      H_ptr->check_homography_matrix(
         inlier_XY,inlier_UV,reduced_n_inliers,print_flag);
   }

   H_ptr->enforce_unit_determinant();
   H_ptr->compute_homography_inverse();

   cout << "n_inliers = " << n_inliers
        << " reduced_n_inliers = " << reduced_n_inliers << endl;
//   cout << "*H_ptr = " << *H_ptr << endl;
//   cout << "H_ptr->det = " << H_ptr->get_H_ptr()->determinant() << endl;
//   cout << "Hinv = " << *(H_ptr->get_Hinv_ptr()) << endl;
}

// ---------------------------------------------------------------------
// Member function identify_candidate_feature_matches_for_image_pair()
// takes in image indices i and j.  It utilizes the Kdtrees
// within members *ANN_ptr and *inverse_ANN_ptr which hold (F,D)
// feature information for image j and image i, respectively.  This
// method then searches for one-to-one and onto pairings between the
// features for image i and image j.  Such bijective pairs are
// temporarily stored in member STL vector candidate_tiepoint_pairs
// for subsequent RANSAC processing.

// For video/panorama matching, currimage_feature_info should contain
// instantaneous video frame's features.  nextimage_feature_info
// should contain cumulative features for all panorama component images.

int sift_detector::identify_candidate_feature_matches_for_image_pair(
   const vector<feature_pair>& currimage_feature_info,
   const vector<feature_pair>& nextimage_feature_info,
   double sqrd_max_ratio)
{
   cout << "inside sift_detector::identify_candidate_feature_matches_for_image_pair()"
        << endl;

   unsigned int n_features=currimage_feature_info.size();

   cout << "currimage_feature_info.size() = " 
        << currimage_feature_info.size() << endl;
   cout << "nextimage_feature_info.size() = " 
        << nextimage_feature_info.size() << endl;

   feature_pair matching_feature_pair;
   for (unsigned int f=0; f<n_features; f++)
   {
//      cout << "f = " << f << endl;
//      if (f%1000==0) cout << f/1000 << " " << flush;
      descriptor* F_ptr=currimage_feature_info[f].first;
      descriptor* D_ptr=currimage_feature_info[f].second;

// For each feature within currimage, search for forward neighboring
// counterpart within nextimage:

      if (ANN_ptr->match_feature_descriptor(
             D_ptr,nextimage_feature_info,sqrd_max_ratio,
             matching_feature_pair))
      {

// Test whether backward neighbor counterpart to forward matching
// feature yields initial feature.  If so, tiepoint pair is one-to-one
// and onto:

         feature_pair inverse_matching_feature_pair=
            inverse_ANN_ptr->match_feature_descriptor(
               matching_feature_pair.second,currimage_feature_info,
               sqrd_max_ratio);

         if ( inverse_matching_feature_pair.first != NULL &&
              nearly_equal(F_ptr->get(0),
                           inverse_matching_feature_pair.first->get(0)) )
         {
            descriptor* Fmatch_ptr=matching_feature_pair.first;
            candidate_tiepoint_pairs.push_back(
               feature_pair(F_ptr,Fmatch_ptr));

//            double u=F_ptr->get(1);
//            double v=F_ptr->get(2);
//            double uprime=Fmatch_ptr->get(1);
//            double vprime=Fmatch_ptr->get(2);

//            cout << "feature ID = " << F_ptr->get(0)
//                 << " u = " << u << " v = " << v 
//                 << " scale = " << F_ptr->get(3)
//                 << " orient = " << F_ptr->get(4)
//                 << endl;
            
//            cout << "feature = " << F_ptr->get(0)
//                 << " inverse_feature = "
//                 << inverse_matching_feature_pair.first->get(0)
//                 << endl;

//               cout << "mfeature ID = " << Fmatch_ptr->get(0)
//                    << " u' = " << uprime << " v' = " << vprime
//                    << " scale = " << Fmatch_ptr->get(3)
//                    << " orient = " << Fmatch_ptr->get(4)
//                    << endl << endl;

//         cout << "Matching feature = "
//              << *(matching_feature_pair.first) << endl;

/*            
// Look for patterns in matching feature scales and orientations:
         
            double curr_scale_ratio=F_ptr->get(3)/Fmatch_ptr->get(3);
            scale_ratios.push_back(curr_scale_ratio);

            double matching_orientation=mathfunc::phase_to_continuous_value(
               Fmatch_ptr->get(4),F_ptr->get(4));
            double curr_orientation_diff=F_ptr->get(4)-matching_orientation;
            orientation_differences.push_back(curr_orientation_diff);
*/

         } // backward matching consistent with forward matching conditional
      }
      else
      {
//               cout << "No matching feature found" << endl;
      }

   } // loop over index f labeling features within currimage
//   cout << endl;
  
/*
   const int min_tiepoint_pairs=5;
   if (scale_ratios.size() > min_tiepoint_pairs)
   {
      double mean_scale_ratio=mathfunc::mean(scale_ratios);
      double sigma_scale_ratio=mathfunc::std_dev(scale_ratios);
      cout << "scale ratio = " << mean_scale_ratio << " +/- "
           << sigma_scale_ratio << endl;
   }
   
   if (orientation_differences.size() > min_tiepoint_pairs)
   {
      double mean_orientation_diff=mathfunc::mean(orientation_differences);
      double sigma_orientation_diff=mathfunc::std_dev(
         orientation_differences);
      cout << "orientation diff = " << mean_orientation_diff*180/PI << " +/- "
           << sigma_orientation_diff*180/PI << " degs" << endl;
   }
*/

   return candidate_tiepoint_pairs.size();
}

// ---------------------------------------------------------------------
// Member function maximal_number_forward_feature_matches takes in
// index i labeling some dynamic video frame and indices jstart &
// jstop labeling static panoramic photos.  This method counts numbers
// of forward feature matches between the former with each of the
// photos in the specified interval.  It returns a pair of integers
// where the first contains the maximal number of feature matches and
// the second contains the corresponding panoramic photo index.

pair<int,int> sift_detector::maximal_number_forward_feature_matches(
   int i,int jstart,int jstop,double sqrd_max_ratio)
{ 
   cout << "inside sift_detector::maximal_number_feature_matches()" 
        << endl;
//   cout << "nimages = " << n_images << endl;

   int max_n_feature_matches=-1;
   int j_maximal_overlap=-1;
   for (int j=jstart; j<jstop; j++)
   {
      ANN_ptr->load_data_points(image_feature_info[j]);
      int n_feature_matches=count_candidate_forward_feature_matches(
         image_feature_info[i],image_feature_info[j],sqrd_max_ratio);
      if (n_feature_matches > max_n_feature_matches)
      {
         max_n_feature_matches=n_feature_matches;
         j_maximal_overlap=j;
      }
      
//      cout << "j = " << j
//           << " n_feature_matches with image i="
//           << n_feature_matches << endl;
   } // loop over index j labeling images matching against image i

//   cout << "max_n_feature_matches = " << max_n_feature_matches << endl;
//   cout << 
//      "Panoramic component image whose overlap with dynamic video frame is maximal = " 
//        << j_maximal_overlap << endl;
   return pair<int,int>(max_n_feature_matches,j_maximal_overlap);
}

// ---------------------------------------------------------------------
// Member function count_candidate_forward_feature_matches() takes in
// STL vector currimage_feature_info containing feature_pairs for a
// single photograph as well as nextimage_feature_info.  This method
// returns the number of potential tiepoint matches between the former
// with the latter.

int sift_detector::count_candidate_forward_feature_matches(
   const vector<feature_pair>& currimage_feature_info,
   const vector<feature_pair>& nextimage_feature_info,
   double sqrd_max_ratio)
{
//   cout << "inside sift_detector::count_candidate_forward_feature_matches()"
//        << endl;

   int n_candidate_forward_matches=0;
//   cout << "currimage_feature_info.size() = " 
//        << currimage_feature_info.size() << endl;

// In order to speed up figuring out which static panoramic component
// should be matched against the input dynamic video frame, we only
// forward project nmax_features rather than all
// currimage_feature_info.size():

   int nmax_features=200;
//   int nmax_features=400;
   int fstep=currimage_feature_info.size()/nmax_features;

   feature_pair matching_feature_pair;
   descriptor* D_ptr;
   
   for (unsigned int f=0; f<currimage_feature_info.size(); f += fstep)
   {
//      if (f%1000==0) cout << f/1000 << " " << flush;

      D_ptr=currimage_feature_info[f].second;

// For each feature within currimage, search for forward neighboring
// counterpart within nextimage:

      if (ANN_ptr->match_feature_descriptor(
             D_ptr,nextimage_feature_info,sqrd_max_ratio,
             matching_feature_pair))
      {
         n_candidate_forward_matches++;
      }
   } // loop over index f labeling features within currimage
   return n_candidate_forward_matches;
}

// ---------------------------------------------------------------------
vector<int> sift_detector::identify_overlapping_images(
   int i,int jstart,int jstop,int max_n_feature_matches,
   double frac_max_n_feature_matches,double sqrd_max_ratio)
{ 
   cout << "inside sift_detector::identify_overlapping_images()" << endl;
//   cout << "nimages = " << n_images << endl;

   vector<int> overlapping_images;
   for (int j=jstart; j<jstop; j++)
   {
      ANN_ptr->load_data_points(image_feature_info[j]);
      int n_feature_matches=count_candidate_forward_feature_matches(
         image_feature_info[i],image_feature_info[j],sqrd_max_ratio);
      if (n_feature_matches > frac_max_n_feature_matches*
          max_n_feature_matches)
      {
         overlapping_images.push_back(j);
         cout << "image i=" << i << " overlaps image j=" << j << " with "
              << n_feature_matches << " feature matches" << endl;
      }
   } // loop over index j labeling images to be compared with image i
   return overlapping_images;
}

// ---------------------------------------------------------------------
// Member function identify_candidate_feature_ray_matches() takes in
// integer i labeling a dynamic video frame whose features are to be
// matched onto the subset of panoramic stills whose indices are
// contained within input STL vector overlapping_image_indices.  This
// method forms an STL map of the video frame's features.  It then
// loops over each panoramic still and erases from the STL map those
// features which it matches.  The feature tiepoints are returned
// within member STL vector candidate_tiepoint_pairs.

// We have intentionally tried to streamline this member function so
// that it runs as fast as possible.

void sift_detector::identify_candidate_feature_ray_matches(
   int i,const vector<int>& overlapping_image_indices,double sqrd_max_ratio)
{ 
   cout << "inside sift_detector::identify_candidate_feature_ray_matches()" 
        << endl;
   
// First load features for image i into *inverse_ANN_ptr:

//   cout << "Loading features for video frame into *inverse_ANN_ptr" << endl;
   inverse_ANN_ptr->load_data_points(image_feature_info[i]);

// Fill STL map with image i's feature pairs:

   typedef map<int,feature_pair> FEATURE_PAIR_MAP;
   FEATURE_PAIR_MAP currimage_feature_pair_map;
   for (unsigned int f=0; f<image_feature_info[i].size(); f++)
   {
      currimage_feature_pair_map[f]=image_feature_info[i].at(f);
   } // loop over index f labeling image i's features

   vector<FEATURE_PAIR_MAP::iterator> iters_to_delete;

// Loop over panoramic still images jstart through jstop:

   descriptor *F_ptr,*D_ptr,*Fmatch_ptr;
   feature_pair matching_feature_pair,inverse_matching_feature_pair;
   candidate_tiepoint_pairs.clear();
   for (unsigned int k=0; k<overlapping_image_indices.size(); k++)
   {
      int j=overlapping_image_indices[k];

      ANN_ptr->load_data_points(image_feature_info[j]);

// Loop over features in video frame i and search for counterparts in
// panoramic stills jstart through jstop:

      iters_to_delete.clear();
      for (FEATURE_PAIR_MAP::iterator iter=currimage_feature_pair_map.begin();
           iter != currimage_feature_pair_map.end(); iter++)
      {

// For each feature within currimage, search for forward neighboring
// counterpart within nextimage:

         D_ptr=iter->second.second;

         if (ANN_ptr->match_feature_descriptor(
                D_ptr,image_feature_info[j],sqrd_max_ratio,
                matching_feature_pair))
         {

// Test whether backward neighbor counterpart to forward matching
// feature yields initial feature.  If so, tiepoint pair is one-to-one
// and onto:

            F_ptr=iter->second.first;

            if (inverse_ANN_ptr->match_feature_descriptor(
                   matching_feature_pair.second,image_feature_info[i],
                   sqrd_max_ratio,inverse_matching_feature_pair) &&
                nearly_equal(F_ptr->get(0),
                             inverse_matching_feature_pair.first->get(0)) )
            {

// Copy 3D ray from *Fmatch_ptr to *F_ptr:

               Fmatch_ptr=matching_feature_pair.first;
               F_ptr->put(6,Fmatch_ptr->get(6));
               F_ptr->put(7,Fmatch_ptr->get(7));
               F_ptr->put(8,Fmatch_ptr->get(8));

//               cout << " nx = " << F_ptr->get(6)
//                    << " ny = " << F_ptr->get(7)
//                    << " nz = " << F_ptr->get(8) << endl;
                   
               candidate_tiepoint_pairs.push_back(iter->second);

// Add current feature pair to an STL vector for later deletion from
// currimage_feature_pair_map prior to searching next panoramic still:

               iters_to_delete.push_back(iter);

            } // backward matching consistent w/ forward matching conditional
         } // matching_feature_pair.first != NULL conditional
      } // loop over FEATURE_MAP iterator

// Loop over features in video frame i and delete those from
// currimage_feature_pair_map which were successfully matched against
// panoramic still j:

      for (unsigned int d=0; d<iters_to_delete.size(); d++)
      {
         currimage_feature_pair_map.erase(iters_to_delete[d]);
      }

      cout << "k = " << k << " image j = " << j 
           << " currimage_feature_pair_map.size() = "
           << currimage_feature_pair_map.size() << endl;

   } // loop over index k labeling panoramic still photos

   cout << "candidate_tiepoint_pairs.size() = "
        << candidate_tiepoint_pairs.size() << endl;
}

// ---------------------------------------------------------------------
// Member function homography_match_image_pair_features() matches
// SIFT/ASIFT features extracted from the ith images with those from
// j=j_start to j_stop.  

void sift_detector::homography_match_image_pair_features(
   int i,int j,int max_n_good_RANSAC_iters,double sqrd_max_ratio,
   double worst_frac_to_reject,double max_sqrd_delta,
   string features_subdir)
{
   cout << "inside sift_detector::homography_match_image_pair_features()" 
        << endl;

   akm* curr_akm_ptr=new akm(FLANN_flag);      
   prepare_SIFT_descriptors(i,j,curr_akm_ptr);

   outputfunc::print_elapsed_time();
   string banner="Matching features from image "+
      stringfunc::number_to_string(i)+" with those from image "+
      stringfunc::number_to_string(j);
   outputfunc::write_big_banner(banner);

   vector<feature_pair> curr_candidate_tiepoint_pairs;
   int n_candidate_matches=
      identify_candidate_FLANN_feature_matches_for_image_pair(
         curr_akm_ptr,sqrd_max_ratio,
         get_image_feature_info_ptr(i),get_image_feature_info_ptr(j),
         curr_candidate_tiepoint_pairs);

   vector<feature_pair> curr_inlier_tiepoint_pairs;
   if (n_candidate_matches > 0)
   {
      if (identify_inlier_matches_via_homography(
         i,j,max_n_good_RANSAC_iters,worst_frac_to_reject,max_sqrd_delta,
         curr_candidate_tiepoint_pairs))
      {
         rename_feature_IDs(i,j); // Rename tiepoint pair labels
         export_feature_tracks(i,features_subdir);
         export_feature_tracks(j,features_subdir);
      }
         
   } // n_candidate_matches > 0 conditional
      
   delete curr_akm_ptr;
}

// =========================================================================
// RANSAC member functions
// =========================================================================

// Member function identify_inlier_feature_ray_matches partitions the
// candidate tiepoint features within STL vector member
// candidate_tiepoint_pairs into quadrants and computes their
// enclosing bounding box.  This method then performs multiple RANSAC
// iterations.  For each one, it selects four (UV,ray) coordinates
// from the LL,UL,LR,UR quadrants and computes a homography that maps
// the rays to the UV coordinates.  This method counts the number of
// feature tiepoints which are inliers according to the candidate
// homography.  The homography which maximizes the inlier count is
// used to perform a final feature inlier identification.  The inlier
// results are returned via member STL vector inlier_tiepoint_pairs.

void sift_detector::identify_inlier_feature_ray_matches(
   int n_min_quadrant_features,
   double worst_frac_to_reject,double max_sqrd_delta)
{ 
   cout << "inside sift_detector::identify_inlier_feature_ray_matches()" 
        << endl;

   double bbox_area=bin_features_into_quadrants(n_min_quadrant_features);

   max_n_inliers=0;
   int n_iters=0;
   int max_n_iters=100;
//   int max_n_iters=300;
//   int max_n_iters=500;
//   int max_n_iters=1000;
   inlier_tiepoint_pairs.clear();

   bool print_flag=false;
   bool feature_ray_matching_flag=true;
   while (n_iters < max_n_iters)
   {
//            if (n_iters%100==0) cout << n_iters/100 << " " << flush;

      if (!compute_candidate_homography(bbox_area,feature_ray_matching_flag)) 
      {
         continue;
      }
      const double max_sqrd_delta=sqr(0.005);
      identify_inliers_via_homography(
         candidate_tiepoint_pairs,
         max_sqrd_delta,print_flag,feature_ray_matching_flag);
      n_iters++;
   }
   cout << endl;

   compute_inlier_homography(worst_frac_to_reject,feature_ray_matching_flag);
}

// ---------------------------------------------------------------------
// Member function bin_features_into_quadrants loops over all features
// within member STL vector candidate_tiepoint_pairs.  It first
// computes a 2D center-of-mass for the input feature distribution.
// This method next decomposes the moment-of-inertia tensor for the
// feature distribution in order to compute its principle symmetry
// directions.  Each feature in the distribution is then binned
// according to whether it lies within the upper/lower and left/right
// quadrants wrt the COM as defined by the principle symmetry
// directions.  This method also returns the area of the bounding box
// enclosing all of the features which is aligned with the principle
// symmetry directions.

double sift_detector::bin_features_into_quadrants(int n_min_quadrant_features)
{
//   cout << "inside sift_detector::bin_features_into_quadrants()" << endl;

// First compute 2D COM of candidate features in image i:

   unsigned int n_features=candidate_tiepoint_pairs.size();

   vector<double> U,V;
   vector<twovector> R;
   for (unsigned int f=0; f<n_features; f++)
   {
      descriptor* F_ptr=candidate_tiepoint_pairs[f].first;
      R.push_back( recover_UV_from_F(F_ptr) );
      U.push_back(R.back().get(0));
      V.push_back(R.back().get(1));

//      cout << "f = " << f << " U = " << U.back() << " V = " << V.back()
//           << endl;
   }

// At least 4 features must be found in order to construct a
// homography:

   if (R.size() < 4)
   {
      double bbox_area=0;
      return bbox_area;
   }

//   double U_avg=mathfunc::mean(U);
//   double V_avg=mathfunc::mean(V);
//   twovector origin(U_avg,V_avg);
   double U_median=mathfunc::median_value(U);
   double V_median=mathfunc::median_value(V);
   twovector origin(U_median,V_median);
//   cout << "U_median = " << U_median << " V_median = " << V_median <<  endl;

// Compute and decompose moment-of-inertia tensor for 2D feature
// distribution:

   double Imin,Imax;
   twovector Imin_hat,Imax_hat;
   mathfunc::moment_of_inertia_2D(
      origin,Imin,Imax,Imin_hat,Imax_hat,R);

//   cout << "sqrt(Imin) = " << sqrt(Imin) 
//        << " sqrt(Imax) = " << sqrt(Imax) << endl;
//   cout << "Imin_hat = " << Imin_hat << " Imax_hat = " << Imax_hat
//        << " Imin_hat.dot(Imax_hat) = " << Imin_hat.dot(Imax_hat) 
//        << endl;

   double bbox_area=mathfunc::moment_of_inertia_bbox_area(
      origin,Imin_hat,Imax_hat,R);

// Use feature distribution's COM and its principle symmetry
// directions to define left & right and lower & upper:
   
   LL_candidate_tiepoint_pairs.clear();
   LR_candidate_tiepoint_pairs.clear();
   UR_candidate_tiepoint_pairs.clear();
   UL_candidate_tiepoint_pairs.clear();

   for (unsigned int f=0; f<n_features; f++)
   {
      descriptor* F_ptr=candidate_tiepoint_pairs[f].first;
      descriptor* Fprime_ptr=candidate_tiepoint_pairs[f].second;
      feature_pair curr_candidate_tiepoint(F_ptr,Fprime_ptr);

      twovector delta=R[f]-origin;
      double alpha=delta.dot(Imin_hat);
      double beta=delta.dot(Imax_hat);

      if (alpha < 0 && beta < 0)
      {
         LL_candidate_tiepoint_pairs.push_back(curr_candidate_tiepoint);
      }
      else if (alpha >= 0 && beta < 0)
      {
         LR_candidate_tiepoint_pairs.push_back(curr_candidate_tiepoint);
      }
      else if (alpha >= 0 && beta >= 0)
      {
         UR_candidate_tiepoint_pairs.push_back(curr_candidate_tiepoint);
      }
      else if (alpha < 0 && beta >= 0)
      {
         UL_candidate_tiepoint_pairs.push_back(curr_candidate_tiepoint);
      }
   } // loop over index f labeling features in image i

   int LL=LL_candidate_tiepoint_pairs.size();
   int LR=LR_candidate_tiepoint_pairs.size();
   int UR=UR_candidate_tiepoint_pairs.size();
   int UL=UL_candidate_tiepoint_pairs.size();
   
//   cout << "U_median = " << U_median << " V_median = " << V_median << endl;
//   cout << "LL = " << LL << " LR = " << LR << endl;
//   cout << "UR = " << UR << " UL = " << UL << endl;
//   cout << "LL+UL = " << LL+UL << " LR+UR = " << LR+UR << endl;
//   cout << "LL+LR = " << LL+LR << " UL+UR = " << UL+UR << endl;
//   cout << endl;

// If LL, LR, UR or UL is too small, feature matching between pairs of
// images will be corrupted.  In this case, return a bbox_area of zero
// to indicate failure:

   if (LL < n_min_quadrant_features ||
       LR < n_min_quadrant_features ||
       UR < n_min_quadrant_features ||
       UL < n_min_quadrant_features)
   {
      bbox_area=0;
   }

//   cout << "bbox_area = " << bbox_area << endl;
   return bbox_area;
}

// ---------------------------------------------------------------------
// Member function compute_candidate_homography randomly selects one
// feature from the LL, LR, UR and UL quadrant member STL vectors.  It
// checks whether the quadrilateral spanned by these 4 selected UV
// twovectors is some reasonable fraction of the input bbox area.  If
// not, this boolean method returns false.  If input argument
// feature_ray_matching_flag==true [false], this method fills member
// homography *H_ptr with the homography that maps from the first
// feature's UV coordinates to the corresponding first feature's n_hat
// [second feature's UV] coordinates stored in member STL vector
// candidate_tiepoint_pairs.

bool sift_detector::compute_candidate_homography(
   double bbox_area,bool feature_ray_matching_flag)
{
//   cout << "inside compute_candidate_homography()" << endl;

   int ll_index=basic_math::mytruncate(
      LL_candidate_tiepoint_pairs.size()*0.99*nrfunc::ran1());
   int lr_index=basic_math::mytruncate(
      LR_candidate_tiepoint_pairs.size()*0.99*nrfunc::ran1());
   int ur_index=basic_math::mytruncate(
      UR_candidate_tiepoint_pairs.size()*0.99*nrfunc::ran1());
   int ul_index=basic_math::mytruncate(
      UL_candidate_tiepoint_pairs.size()*0.99*nrfunc::ran1());

//   ll_index=basic_math::min(ll_index,int(LL_candidate_tiepoint_pairs.size()-1));
//   lr_index=basic_math::min(lr_index,int(LR_candidate_tiepoint_pairs.size()-1));
//   ur_index=basic_math::min(ur_index,int(UR_candidate_tiepoint_pairs.size()-1));
//   ul_index=basic_math::min(ul_index,int(UL_candidate_tiepoint_pairs.size()-1));

//   cout << "ll_index = " << ll_index
//        << " lr_index = " << lr_index
//        << " ur_index = " << ur_index
//        << " ul_index = " << ul_index << endl;
   
   descriptor *F_ptr,*Fmatch_ptr;
   vector<twovector> candidate_features,matching_candidate_features;
   vector<feature_pair> features_rays;

   for (unsigned int q=0; q<4; q++)
   {
      if (q==0)
      {
         F_ptr=LL_candidate_tiepoint_pairs[ll_index].first;
         Fmatch_ptr=LL_candidate_tiepoint_pairs[ll_index].second;
      }
      else if (q==1)
      {
         F_ptr=LR_candidate_tiepoint_pairs[lr_index].first;
         Fmatch_ptr=LR_candidate_tiepoint_pairs[lr_index].second;
      }
      else if (q==2)
      {
         F_ptr=UR_candidate_tiepoint_pairs[ur_index].first;
         Fmatch_ptr=UR_candidate_tiepoint_pairs[ur_index].second;
      }
      else if (q==3)
      {
         F_ptr=UL_candidate_tiepoint_pairs[ul_index].first;
         Fmatch_ptr=UL_candidate_tiepoint_pairs[ul_index].second;
      }
   
//      cout << "q = " << q
//           << " *F_ptr = " << *F_ptr
//           << " *Fmatch_ptr = " << *Fmatch_ptr << endl;

      twovector UV=recover_UV_from_F(F_ptr);
      candidate_features.push_back(UV);

      if (feature_ray_matching_flag)
      {
         features_rays.push_back(feature_pair(F_ptr,Fmatch_ptr));
      }
      else
      {
         twovector UVmatch=recover_UV_from_F(Fmatch_ptr);
         matching_candidate_features.push_back(UVmatch);
      }

//      cout << "UV = " << UV << " UVmatch = " << UVmatch << endl;
   } // loop over index q labeling quadrants

// Compute area of quadrilateral spanned by 4 candidate features.
// Reject candidate feature set if quadrilateral's area is less than
// some minimal fraction of input bounding box area:

   bool homography_OK_flag=true;
   double lower_right_triangle_area=geometry_func::compute_triangle_area(
      candidate_features[0],candidate_features[1],candidate_features[2]);
   double upper_left_triangle_area=geometry_func::compute_triangle_area(
      candidate_features[2],candidate_features[3],candidate_features[0]);
   double quadrilateral_area=lower_right_triangle_area+
      upper_left_triangle_area;
   double area_ratio=quadrilateral_area/bbox_area;
//   const double min_area_ratio=0.20;
   const double min_area_ratio=0.25;

   if (area_ratio < min_area_ratio)
   {
      homography_OK_flag=false;
      return homography_OK_flag;
   }
//   cout << "area_ratio = " << area_ratio << endl;
   
   if (feature_ray_matching_flag)
   {
      compute_ray_feature_homography(&features_rays);
   }
   else
   {
      H_ptr->parse_homography_inputs(
         candidate_features,matching_candidate_features);
      H_ptr->compute_homography_matrix();
   }

//   H_ptr->check_homography_matrix(
//      candidate_features,matching_candidate_features);
//   H_ptr->enforce_unit_determinant();
//   H_ptr->compute_homography_inverse();

//   cout << "*H_ptr = " << *H_ptr << endl;
//   cout << "H_ptr->det = " << H_ptr->get_H_ptr()->determinant() << endl;
//   cout << "Hinv = " << *(H_ptr->get_Hinv_ptr()) << endl;

   return homography_OK_flag;
}

// ---------------------------------------------------------------------
// Member function compute_n_iters implements formula (4.18) in
// Multiple View Geometry in Computer Vision by Hartley and Zisserman,
// 2nd edition.  This equation returns the number of iterations
// required to ensure, with probability p, that at least one
// homography has no outliers for an input number of tiepoints
// n_tiepoints and a proportion of outliers eps.

int sift_detector::compute_n_iters(double eps,int n_tiepoints)
{
//   cout << "inside sift_detector::compute_iters()" << endl;
   const double p=0.999;
   n_iters=log(1-p)/log(1-(pow(1-eps,n_tiepoints)));
   cout << "n_iters = " << n_iters << endl;

   return n_iters;
}

// ---------------------------------------------------------------------
// Member function refine_inliers_identification loops over all
// candidate inlier tiepoint pairs.  For each pair, it computes the
// forward [backward] projection of UV [UVmatch] and compares with
// UVmatch [UV].  If the difference between the projected feature and
// its candidate tiepoint counterpart is too large, the candidate is
// rejected.

void sift_detector::refine_inliers_identification(
   double max_sqrd_delta,bool feature_ray_matching_flag)
{
//   cout << "inside sift_detector::refine_inliers_identification()" << endl;
   string banner="Refining inliers identification";
   outputfunc::write_banner(banner);
   outputfunc::enter_continue_char();

   int n_inliers(inlier_tiepoint_pairs.size());
   cout << "n_inliers = " << n_inliers << endl;

   double forward_sqrd_delta,backward_sqrd_delta;
   vector<feature_pair> cleaned_inlier_tiepoint_pairs;
   for (unsigned int i=0; i<inlier_tiepoint_pairs.size(); i++)
   {
      descriptor* F_ptr=inlier_tiepoint_pairs[i].first;
      twovector UV=recover_UV_from_F(F_ptr);

      if (feature_ray_matching_flag)
      {
         threevector n_hat(recover_nhat_from_F(F_ptr));
         twovector UVprojected=
            H_ptr->project_ray_to_image_plane(n_hat);
         forward_sqrd_delta=(UV-UVprojected).sqrd_magnitude();
         backward_sqrd_delta=0;
      }
      else
      {
         descriptor* Fmatch_ptr=inlier_tiepoint_pairs[i].second;
         twovector UVmatch=recover_UV_from_F(Fmatch_ptr);

//    cout << " *F_ptr = " << *F_ptr
//           << " *Fmatch_ptr = " << *Fmatch_ptr << endl;
//      cout << "UV = " << UV << " UVmatch = " << UVmatch << endl;

         twovector UV_projected=H_ptr->project_world_plane_to_image_plane(UV);
         twovector UVmatch_projected=
            H_ptr->project_image_plane_to_world_plane(UVmatch);
         forward_sqrd_delta=(UV_projected-UVmatch).sqrd_magnitude();
         backward_sqrd_delta=(UVmatch_projected-UV).sqrd_magnitude();
      }

      if (forward_sqrd_delta < max_sqrd_delta &&
         backward_sqrd_delta < max_sqrd_delta)
      {
         cleaned_inlier_tiepoint_pairs.push_back(inlier_tiepoint_pairs[i]);
      }
   }  // loop over index i labeling candidate inlier tiepoint pairs

//   cout << "Before cleaning, inlier_tiepoint_pairs.size() = "
//        << inlier_tiepoint_pairs.size() << endl;

   inlier_tiepoint_pairs.clear();
   for (unsigned int i=0; i<cleaned_inlier_tiepoint_pairs.size(); i++)
   {
      inlier_tiepoint_pairs.push_back(cleaned_inlier_tiepoint_pairs[i]);
   }

//   cout << "After cleaning, inlier_tiepoint_pairs.size() = "
//        << inlier_tiepoint_pairs.size() << endl;
}

// ---------------------------------------------------------------------
// Member function rename_feature_IDs takes in image indices i and j.
// It then resets the IDs for inlier tiepoint pairs in next image j
// and curr image i to their minimal value.  This guarantees that all
// matching SIFT features across multiple images have the same feature
// ID (only true for SERIAL processing!).  All other singleton
// features should have unique IDs.

// This method also propagates valid 3D backprojected ray information
// among different images.

// In April 2013, we empirically observed that inlier_tiepoint_pairs
// can contain redundant copies of genuine tiepoints.  So n_features
// is generally less than inlier_tiepoint_pairs.size().

void sift_detector::rename_feature_IDs(int i,int j)
{
   rename_feature_IDs(i,j,inlier_tiepoint_pairs);
}

void sift_detector::rename_feature_IDs(
   int i,int j,const vector<feature_pair>& curr_inlier_tiepoint_pairs)
{
   cout << "inside sift_detector::rename_feature_IDs(i,j)" << endl;
   cout << "i = " << i << " j = " << j << endl;
   string banner="Renaming feature IDs";
   outputfunc::write_banner(banner);

// First reset feature IDs for curr image i and next image j which
// belong to an inlier tiepoint pair to their minimal value:

   cout << "i = " << i << " j = " << j 
        << " curr_inlier_tiepoint_pairs.size() = "
        << curr_inlier_tiepoint_pairs.size() << endl;
   for (unsigned int t=0; t<curr_inlier_tiepoint_pairs.size(); t++)
   {
      descriptor* T_ptr=curr_inlier_tiepoint_pairs[t].first;
      descriptor* Tmatch_ptr=curr_inlier_tiepoint_pairs[t].second;

      int T_ID=T_ptr->get(0);
      int Tmatch_ID=Tmatch_ptr->get(0);
      int new_ID=basic_math::min(T_ID,Tmatch_ID);
      T_ptr->put(0,new_ID);
      Tmatch_ptr->put(0,new_ID);

// Copy 3D backprojected rays from *T_ptr [*Tmatch_ptr] onto
// *Tmatch_ptr [*T_ptr] if latter doesn't already contain valid ray
// values:

      if (fabs(T_ptr->get(6)) < 0.5*POSITIVEINFINITY &&
          fabs(Tmatch_ptr->get(6)) > 0.5*POSITIVEINFINITY)
      {
         Tmatch_ptr->put(6,T_ptr->get(6));
         Tmatch_ptr->put(7,T_ptr->get(7));
         Tmatch_ptr->put(8,T_ptr->get(8));
      }
      else if (fabs(Tmatch_ptr->get(6)) < 0.5*POSITIVEINFINITY &&
               fabs(T_ptr->get(6)) > 0.5*POSITIVEINFINITY)
      {
         T_ptr->put(6,Tmatch_ptr->get(6));
         T_ptr->put(7,Tmatch_ptr->get(7));
         T_ptr->put(8,Tmatch_ptr->get(8));
      }

//      cout << "t = " << t 
//           << " T_ptr->get(0) = " << T_ptr->get(0)
//           << " Tmatch_ptr->get(0) = " << Tmatch_ptr->get(0)
//           << endl;
   } // loop over index t labeling inlier tiepoint pairs
//   outputfunc::enter_continue_char();

// Use STL map in order to identify feature track elements:

   cout << "Identifying feature track elements:" << endl;

   vector<feature_pair>* currimage_feature_info_ptr=&(image_feature_info[i]);
   vector<feature_pair>* nextimage_feature_info_ptr=&(image_feature_info[j]);
   cout << "i = " << i << " j = " << j 
        << " currimage_info.size = "
        << currimage_feature_info_ptr->size() 
        << " nextimage_info.size = "
        << nextimage_feature_info_ptr->size()  << endl;

   typedef map<int,int> ID_INDEX_MAP;
   ID_INDEX_MAP* ID_index_map_ptr=new ID_INDEX_MAP;

   for (unsigned int n=0; n<nextimage_feature_info_ptr->size(); n++)
   {
      descriptor* Fmatch_ptr=nextimage_feature_info_ptr->at(n).first;
      int nextimage_ID=Fmatch_ptr->get(0);
      (*ID_index_map_ptr)[nextimage_ID]=n;
   }
   cout << "i = " << i << " j = " << j 
        << " ID_index_map_ptr->size() = "
        << ID_index_map_ptr->size() << endl;
   
   int n_feature_matches=0;
   for (unsigned int c=0; c<currimage_feature_info_ptr->size(); c++)
   {
      descriptor* F_ptr=currimage_feature_info_ptr->at(c).first;
      int currimage_ID=F_ptr->get(0);

      ID_INDEX_MAP::iterator iter=ID_index_map_ptr->find(currimage_ID);
      if (iter == ID_index_map_ptr->end()) continue;

      int n=iter->second;
      descriptor* Fmatch_ptr=nextimage_feature_info_ptr->at(n).first;

// Increment component #5 of *F_ptr that counts number of images in
// which feature exists:

      F_ptr->put(5,F_ptr->get(5)+1);
      Fmatch_ptr->put(5,Fmatch_ptr->get(5)+1);

      n_feature_matches++;
   } // loop over index c labeling features in curr image i

   delete ID_index_map_ptr;

//   cout << "image i = " << i << " image j = " << j << endl;
   cout << "n_feature_matches = " << n_feature_matches << endl << endl;

//   outputfunc::enter_continue_char();
}

// =========================================================================
// map_unionfind member functions
// =========================================================================

/*
// Member function get_node_ID() takes in integer index i labeling an
// image and integer index F_ID labeling a feature for image I.  It
// returns a double which serves as an ID for a node within a
// map_unionfind object.

double sift_detector::get_node_ID(int i,int F_ID) const
{
//   cout << "inside sift_detector::get_node_ID(), i = " << i 
//        << " F_ID = " << F_ID << endl;
//   cout.precision(12);
   double node_ID=F_ID+1E-4*i;
//   cout << "NODE_ID = " << node_ID << endl;
   return node_ID;
}

int sift_detector::get_feature_ID(double node_ID) const
{
//   cout << "inside sift_detector::get_feature_ID()" << endl;
//   cout.precision(12);
   int F_ID=basic_math::mytruncate(node_ID);
   return F_ID;
}

int sift_detector::get_image_number(double node_ID) const
{
//   cout << "inside sift_detector::get_image_number()" << endl;
//   cout.precision(12);
   double image_frac=node_ID-get_feature_ID(node_ID);
   int image_number=1E4*image_frac;
   return image_number;
}
*/

// ---------------------------------------------------------------------
// Member function load_node_IDs() takes in integer index i labeling
// an image.  It retrieves all features which have been extracted for
// this image and computes their corresponding map_unionfind node
// IDs.  This method loads those node IDs into *map_unionfind_ptr.

void sift_detector::load_node_IDs(
   int i,map_unionfind* map_unionfind_ptr)
{
//   cout << "inside sift_detector::load_node_IDs(), i = " << i << endl;

   for (unsigned int f=0; f<image_feature_info[i].size(); f++)
   {
      descriptor* F_ptr=get_image_feature_info_ptr(i)->at(f).first;
      int F_ID=F_ptr->get(0);
      map_unionfind_ptr->MakeSet(map_unionfind_ptr->form_node_ID(i,F_ID));
   } // loop over index f labeling extracted features for image i

//   cout << "map_unionfind.get_n_nodes = "
//        << map_unionfind_ptr->get_n_nodes() << endl;
}

// ---------------------------------------------------------------------
// Member function link_matching_node_IDs() takes in image IDs i and j
// along with an STL vector containing inlier tiepoint pairs.  For
// each tiepoint pair, this method computes map_unionfind node IDs
// from the image and feature labels.  It then links the nodes
// corresponding to these IDs within input *map_unionfind_ptr.  It
// also resets the descriptor's ID for the root node corresponding to
// each tiepoint pair.

void sift_detector::link_matching_node_IDs(
   int i,int j,const vector<feature_pair>& curr_inlier_tiepoint_pairs,
   map_unionfind* map_unionfind_ptr)
{
//   cout << "inside sift_detector::link_matching_node_IDs(i,j)" << endl;
//   cout << "i = " << i << " j = " << j << endl;
   
   for (unsigned int t=0; t<curr_inlier_tiepoint_pairs.size(); t++)
   {
      descriptor* T_ptr=curr_inlier_tiepoint_pairs[t].first;
      descriptor* Tmatch_ptr=curr_inlier_tiepoint_pairs[t].second;
      int T_ID=T_ptr->get(0);
      int Tmatch_ID=Tmatch_ptr->get(0);

      DUPLE node_ID=map_unionfind_ptr->form_node_ID(i,T_ID);
      DUPLE nodematch_ID=map_unionfind_ptr->form_node_ID(j,Tmatch_ID);

      if (!map_unionfind_ptr->set_data_ptr(node_ID,T_ptr))
      {
         cout << "i = " << i << " j = " << j 
              << " node_ID = " << node_ID.first << "," << node_ID.second
              << endl;
         cout << "T_ID = " << T_ID << " Tmatch_ID = " << Tmatch_ID << endl;
         outputfunc::enter_continue_char();
      }

      if (!map_unionfind_ptr->set_data_ptr(nodematch_ID,Tmatch_ptr))
      {
         cout << "i = " << i << " j = " << j 
              << " nodematch_ID = " << nodematch_ID.first << ","
              << nodematch_ID.second << endl;
         cout << "T_ID = " << T_ID << " Tmatch_ID = " << Tmatch_ID << endl;
         outputfunc::enter_continue_char();
      }

      DUPLE root_ID=map_unionfind_ptr->Link(node_ID,nodematch_ID);
      map_unionfind::NODE* root_node_ptr=map_unionfind_ptr->
         get_node_ptr(root_ID);

      descriptor* Troot_ptr=NULL;
      if (root_node_ptr != NULL)
      {
         Troot_ptr=static_cast<descriptor*>(root_node_ptr->third);
      }

      if (Troot_ptr != NULL) 
      {
         int root_node_feature_number=root_node_ptr->first.second;
         Troot_ptr->put(0,root_node_feature_number);

// Increment component #5 of *F_ptr that counts number of images in
// which feature exists:

         Troot_ptr->put(5,Troot_ptr->get(5)+1);
      }
      
   } // loop over index t labeling inlier tiepoint pairs
}

// ---------------------------------------------------------------------
// Member function rename_feature_IDs()

void sift_detector::rename_feature_IDs(map_unionfind* map_unionfind_ptr)
{
   cout << "inside sift_detector::rename_feature_IDs()" << endl;

   map_unionfind::NODES_MAP* nodes_map_ptr=map_unionfind_ptr->
      get_nodes_map_ptr();
//   map_unionfind::NODE* node_ptr=NULL;

   int tiepoint_counter=0;
   for (map_unionfind::NODES_MAP::iterator iter=
           nodes_map_ptr->begin(); iter != nodes_map_ptr->end(); iter++)
   {
      DUPLE node_ID=iter->first;
//      cout << "node_ID = " << node_ID << endl;
      map_unionfind::NODE* node_ptr=iter->second;
      descriptor* F_ptr=static_cast<descriptor*>(node_ptr->third);
      if (F_ptr==NULL) continue;

      DUPLE root_ID=map_unionfind_ptr->Find(node_ID);
//      cout << "root_ID = " << root_ID << endl;
      map_unionfind::NODE* root_node_ptr=map_unionfind_ptr->get_node_ptr(
         root_ID);
      descriptor* Froot_ptr=static_cast<descriptor*>(root_node_ptr->third);

//       int orig_F_ID=F_ptr->get(0);
      int Froot_ID=Froot_ptr->get(0);
      F_ptr->put(0,Froot_ID);
      F_ptr->put(5,Froot_ptr->get(5));

//      cout << "orig F_ID = " << orig_F_ID 
//           << " Froot_ID = " << Froot_ID
//           << " n_matches = " << F_ptr->get(5) 
//           << endl;

      tiepoint_counter++;
   } // loop over iter index 
   cout << "tiepoint_counter = " << tiepoint_counter 
        << " nodes_map_ptr->size() = " << nodes_map_ptr->size() 
        << endl;
}

// =========================================================================
// Feature export member functions
// =========================================================================

// Member export_feature_tiepoints() takes in a set of inlier tiepoint
// pairs corresponding to images labeled by indices i and j.  It first
// appends to the ntiepoints_stream one line which contains the
// indices i and j for the images, their number of tiepoints, the
// number of feature duplicates and the images' file names.
// This method next creates a new subdirectory within tiepoints_subdir
// that is labeled by i and j.  It exports two feature text files to
// this labeled subdirectory which contain all tiepoint pairs found
// for images i and j.  The name of the first tiepoint features filename is
// returned by this method.

string sift_detector::export_feature_tiepoints(
   int i,int j,const vector<feature_pair>& curr_inlier_tiepoint_pairs,
   string tiepoints_subdir,photogroup* curr_photogroup_ptr,
   int n_duplicate_features,map_unionfind* map_unionfind_ptr,
   ofstream& ntiepoints_stream) const
{
   return export_feature_tiepoints(
      i,j,curr_inlier_tiepoint_pairs,tiepoints_subdir,
      curr_photogroup_ptr,n_duplicate_features,-1,
      map_unionfind_ptr,ntiepoints_stream);
}

string sift_detector::export_feature_tiepoints(
   int i,int j,const vector<feature_pair>& curr_inlier_tiepoint_pairs,
   string tiepoints_subdir,photogroup* curr_photogroup_ptr,
   int n_duplicate_features,double sensor_separation_angle,
   map_unionfind* map_unionfind_ptr,ofstream& ntiepoints_stream) const
{
//   cout << "inside sift_detector::export_feature_tiepoints, i = " << i << " j = " << j << endl;

//   string banner="Export tiepoints for image "+stringfunc::number_to_string(i)
//      +" and image "+stringfunc::number_to_string(j);
//   outputfunc::write_big_banner(banner);

   vector<int> ID,freq,feature_index;
   vector<int> ID_match,freq_match,feature_index_match;
   vector<double> U,V,U_match,V_match;

// Set up n_horiz_blocks x n_vert_blocks grid on photo(i):

   photograph* photo_ptr=photogroup_ptr->get_photograph_ptr(i);
   double Umax=photo_ptr->get_maxU();
   
   unsigned int n_horiz_blocks,n_vert_blocks;
   if (Umax > 1)
   {
      n_vert_blocks=4;
      n_horiz_blocks=Umax*n_vert_blocks;
   }
   else
   {
      n_horiz_blocks=4;
      n_vert_blocks=n_horiz_blocks/Umax;
   }
   double block_deltaU=Umax/n_horiz_blocks;
   double block_deltaV=1.0/n_vert_blocks;

// Instantiate genmatrix which counts number of tiepoints that fall
// into grid blocks on photo(i):

   genmatrix* blocks_tiepoint_matrix_ptr=
      new genmatrix(n_vert_blocks,n_horiz_blocks);
   blocks_tiepoint_matrix_ptr->clear_values();

   for (unsigned int t=0; t<curr_inlier_tiepoint_pairs.size(); t++)
   {
      descriptor* F_ptr=curr_inlier_tiepoint_pairs[t].first;
      descriptor* Fmatch_ptr=curr_inlier_tiepoint_pairs[t].second;

      int F_ID=F_ptr->get(0);
      int Fmatch_ID=Fmatch_ptr->get(0);

//      cout << "t = " << t 
//           << " F_ID = " << F_ID 
//           << " Fmatch_ID = " << Fmatch_ID << endl;
//      cout << "F_ptr = " << F_ptr << endl;
//      cout << "Fmatch_ptr = " <<  Fmatch_ptr << endl;

      DUPLE node_ID=map_unionfind_ptr->form_node_ID(i,F_ID);
//      cout << " node_ID = " << node_ID.first << ","
//           << node_ID.second << endl;

      DUPLE nodematch_ID=
         map_unionfind_ptr->form_node_ID(j,Fmatch_ID);
//      cout << " nodematch_ID = " << nodematch_ID.first << ","
//           << nodematch_ID.second << endl;

      DUPLE root_ID=map_unionfind_ptr->Find(node_ID);
//      cout << " root_ID = " << root_ID.first << ","
//           << root_ID.second << endl;
      DUPLE rootmatch_ID=map_unionfind_ptr->Find(nodematch_ID);
//      cout << "rootmatch_ID = " << rootmatch_ID.first << ","
//           << rootmatch_ID.second << endl;

      if (root_ID.first != rootmatch_ID.first ||
          root_ID.second != rootmatch_ID.second)
      {
         cout << "t = " << t 
              << " F_ID = " << F_ID
              << " Fmatch_ID = " << Fmatch_ID
              << " node_ID = " << node_ID.first << ","
              << node_ID.second
              << " nodematch_ID = " << nodematch_ID.first << ","
              << nodematch_ID.second << endl;
         cout << "root_ID = " << root_ID.first << "," 
              << root_ID.second << endl;
         cout << "rootmatch_ID = " << rootmatch_ID.first << "," 
              << rootmatch_ID.second << endl;
         outputfunc::enter_continue_char();
         continue;
      }
      
      ID.push_back(root_ID.second);
      U.push_back(F_ptr->get(1));
      V.push_back(F_ptr->get(2));
      freq.push_back(F_ptr->get(5));
      feature_index.push_back(F_ptr->get(10));

      ID_match.push_back(rootmatch_ID.second);
      U_match.push_back(Fmatch_ptr->get(1));
      V_match.push_back(Fmatch_ptr->get(2));
      freq_match.push_back(Fmatch_ptr->get(5));
      feature_index_match.push_back(Fmatch_ptr->get(10));

// Increment grid block containing current tiepoint within photo(i):

      int column=U.back()/block_deltaU;
      int row=n_vert_blocks-V.back()/block_deltaV;
      blocks_tiepoint_matrix_ptr->put(
         row,column,blocks_tiepoint_matrix_ptr->get(row,column)+1);

   } // loop over index f labeling all SIFT features in image i

// Compute fraction of non-empty grid blocks for photo(i):

//   cout << "*blocks_tiepoint_matrix_ptr = " 
//        << *blocks_tiepoint_matrix_ptr << endl;
   int n_nonzero_tiepoint_blocks=0;
   for (unsigned int r=0; r<n_vert_blocks; r++)
   {
      for (unsigned int c=0; c<n_horiz_blocks; c++)
      {
         if (blocks_tiepoint_matrix_ptr->get(r,c) > 0)
         {
            n_nonzero_tiepoint_blocks++;
         }
      }
   }
   delete blocks_tiepoint_matrix_ptr;
   
   double nonzero_tiepoint_blocks_frac=
      double(n_nonzero_tiepoint_blocks)/double(n_vert_blocks*n_horiz_blocks);
//   cout << "nonzero_tiepoint_blocks_frac = "
//        << nonzero_tiepoint_blocks_frac << endl;

// Export 1-line file containing number of tiepoints between images i
// and j:

   string image_i_basename=filefunc::getbasename(
      photogroup_ptr->get_photograph_ptr(i)->get_filename());
   string image_j_basename=filefunc::getbasename(
      photogroup_ptr->get_photograph_ptr(j)->get_filename());

   int n_tiepoints=ID.size();
   ntiepoints_stream 
      << i << "   " << j << "   " << n_tiepoints << "   "
      << n_duplicate_features << "     " 
      << sensor_separation_angle << "     " 
      << nonzero_tiepoint_blocks_frac << "     "
      << image_i_basename << "    " << image_j_basename 
      << endl;

// Now export tiepoint features for photo(i) and photo(j):

   templatefunc::Quicksort(ID,U,V,freq,feature_index);
   templatefunc::Quicksort(
      ID_match,U_match,V_match,freq_match,feature_index_match);

   string curr_tiepoints_subdir=tiepoints_subdir+"tiepoints_"+
      stringfunc::integer_to_string(i,5)+"_"+
      stringfunc::integer_to_string(j,5)+"/";
   filefunc::dircreate(curr_tiepoints_subdir);

   string unix_cmd="ln -s ../../images/"+image_i_basename+" "
      +curr_tiepoints_subdir;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="ln -s ../../images/"+image_j_basename+" "+curr_tiepoints_subdir;
   sysfunc::unix_command(unix_cmd);

   string features_i_filename=curr_tiepoints_subdir+"features_2D_"+
      stringfunc::prefix(image_i_basename)+".txt";
   string features_j_filename=curr_tiepoints_subdir+"features_2D_"+
      stringfunc::prefix(image_j_basename)+".txt";

   ofstream outstream,match_outstream;
   filefunc::openfile(features_i_filename,outstream);
   filefunc::openfile(features_j_filename,match_outstream);

   outstream << "# time feature-ID image-ID U V Freq Curr_image_feature_index" 
             << endl;
   match_outstream 
      << "# time feature-ID image-ID U V Freq Curr_image_feature_index" 
      << endl;
   outstream << endl;
   match_outstream << endl;
      
   for (unsigned int f=0; f<ID.size(); f++)
   {
      outstream << "0.00  " 
                << ID[f] << "  "
                << i << "  "
                << U[f] << "  "
                << V[f] << "  "
                << freq[f] << "  "
                << feature_index[f] << "  "
                << endl;

      match_outstream << "0.00  " 
                      << ID_match[f] << "  "
                      << j << "  "
                      << U_match[f] << "  "
                      << V_match[f] << "  "
                      << freq_match[f] << "  "
                      << feature_index_match[f] << "  "
                      << endl;
   }

   filefunc::closefile(features_i_filename,outstream);
   filefunc::closefile(features_j_filename,match_outstream);

   return features_i_filename;

//   cout << "At end of sift_detector::export_feature_tiepoints()" << endl;
}

// ---------------------------------------------------------------------
// Member function export_feature_tracks() writes feature information
// to output text files which can be read in by programs VIDEO and
// PANORAMA.  Only features which appear in at least 2 photos are
// exported.  This method returns the number of exported feature tracks.

int sift_detector::export_feature_tracks(int i)
{
//   cout << "inside sift_detector::export_feature_tracks() #1" << endl;
   vector<feature_pair>* currimage_feature_info_ptr=
      &(image_feature_info[i]);
   string features_subdir="./features/";
   return export_feature_tracks(i,features_subdir,currimage_feature_info_ptr);
}

int sift_detector::export_feature_tracks(int i,string features_subdir)
{
//   cout << "inside sift_detector::export_feature_tracks() #2" << endl;
   vector<feature_pair>* currimage_feature_info_ptr=
      &(image_feature_info[i]);
   return export_feature_tracks(i,features_subdir,currimage_feature_info_ptr);
}

int sift_detector::export_feature_tracks(
   int i,string features_subdir,
   vector<feature_pair>* currimage_feature_info_ptr)
{
//   cout << "inside sift_detector::export_feature_tracks() #3" << endl;

   string curr_filename=photogroup_ptr->get_photograph_ptr(i)->
      get_filename();
   string feature_filename=features_subdir+"features_2D_"+
      stringfunc::prefix(filefunc::getbasename(curr_filename))+".txt";

   string banner="Exporting feature tracks to output file "+feature_filename;
   outputfunc::write_big_banner(banner);

   return export_all_feature_tracks(
      i,feature_filename,currimage_feature_info_ptr);
}

// ---------------------------------------------------------------------
// This variant of member function export_feature_tracks()
// writes feature information for image labeled by input index i to
// output text file feature_filename.  It returns the number of
// exported feature tracks.

int sift_detector::export_all_feature_tracks(
   int i,string features_filename,
   vector<feature_pair>* currimage_feature_info_ptr)
{
//   cout << "inside sift_detector::export_all_feature_tracks()" << endl;
//   cout << "image index i = image ID = " << i << endl;
//   cout << "features_filename = " << features_filename << endl;

   ofstream outstream;
   filefunc::openfile(features_filename,outstream);

// Sort tiepoint features according to their feature IDs:

   vector<int> ID,IDcopy,feature_index,freq;
   vector<double> U,V,nx,ny,nz;

   const int f_skip=1;
   const int n_counterpart_photos=1;
//   const int f_skip=10;		// viewgraph generation
//   const int n_counterpart_photos=0;	// viewgraph generation

//   cout << "currimage_feature_info_ptr->size() = "
//        << currimage_feature_info_ptr->size() << endl;
 
   for (unsigned int f=0; f<currimage_feature_info_ptr->size(); f += f_skip)
   {
      descriptor* F_ptr=currimage_feature_info_ptr->at(f).first;
      
      if (F_ptr->get(5) > n_counterpart_photos)
      {
         int curr_ID=F_ptr->get(0);
         bool continue_flag=false;
         for (unsigned int j=0; j<ID.size(); j++)
         {
            if (ID[j]==curr_ID) continue_flag=true;
         }
         if (continue_flag) continue;

         ID.push_back(F_ptr->get(0));
         feature_index.push_back(F_ptr->get(10));
                           
         U.push_back(F_ptr->get(1));
         V.push_back(F_ptr->get(2));
         freq.push_back(F_ptr->get(5));

         IDcopy.push_back(F_ptr->get(0));
         nx.push_back(F_ptr->get(6));
         ny.push_back(F_ptr->get(7));
         nz.push_back(F_ptr->get(8));
      } // conditional indicating feature belongs to track
   } // loop over index f labeling all SIFT features in image i

   templatefunc::Quicksort(ID,feature_index,U,V,freq);
//   templatefunc::Quicksort(IDcopy,nx,ny,nz);
   outstream << "# time feature-ID image-ID U V Freq Curr_image_feature_index" 
             << endl;
   outstream << endl;
      
   outstream.precision(12);
   for (unsigned int f=0; f<ID.size(); f++)
   {
      outstream << "0.00  " 
                << ID[f] << "  "
                << i << "  "
                << U[f] << "  "
                << V[f] << "  "
                << freq[f] << "  "
                << feature_index[f] << "  ";
//      if (fabs(nx[f]) < 0.5*POSITIVEINFINITY)
//      {
//         outstream << nx[f] << "  "
//                   << ny[f] << "  "
//                   << nz[f] << "  ";
//      }
      outstream << endl;
   }

   filefunc::closefile(features_filename,outstream);
   return ID.size();
}

// ---------------------------------------------------------------------
// Member function generate_features_map()

void sift_detector::generate_features_map()
{
//   cout << "inside sift_detector::generate_feature_map()" << endl;
   string banner="Generating features map:";
   outputfunc::write_banner(banner);

   for (unsigned int i=0; i<n_images; i++)
   {
      generate_features_map(i);
   } // loop over index i labeling images
}

void sift_detector::generate_features_map(int i)
{
   int pass_number=i;
   vector<feature_pair>* currimage_feature_info_ptr=
      &(image_feature_info[i]);
   for (unsigned int f=0; f<currimage_feature_info_ptr->size(); f++)
   {
      descriptor* F_ptr=currimage_feature_info_ptr->at(f).first;
      if (F_ptr->get(5) > 1)
      {
         int feature_ID=F_ptr->get(0);
         double U=F_ptr->get(1);
         double V=F_ptr->get(2);

         fourvector feature_coords(pass_number,U,V);
         FEATURES_MAP::iterator feature_iter=
            features_map.find(feature_ID);
         if (feature_iter != features_map.end())
         {
            feature_iter->second.push_back(feature_coords);
         }
         else
         {
            vector<fourvector> fV;
            fV.push_back(feature_coords);
            features_map[ feature_ID ] = fV;
         }

      } // conditional indicating feature belongs to track
   } // loop over index f labeling all SIFT features in image i
}

// ---------------------------------------------------------------------
// Member function export_fundamental_matrix() takes in integer
// indices i and j that label the current photo pair.  It writes out
// the 3x3 fundamental matrix corresponding for this pair to a text
// file whose name contains those of the two photos.

void sift_detector::export_fundamental_matrix(
   fundamental* curr_fundamental_ptr,string bundler_IO_subdir,int i,int j)
{
//   cout << "inside sift_detector::export_fundamental_matrix" << endl;

   string fundamental_subdir=bundler_IO_subdir+"fundamental_matrices/";
   filefunc::dircreate(fundamental_subdir);
   
   string curr_filename=photogroup_ptr->get_photograph_ptr(i)->
      get_filename();
   string next_filename=photogroup_ptr->get_photograph_ptr(j)->
      get_filename();

   string fundamental_filename=fundamental_subdir+"fundamental_____"+
      filefunc::getbasename(curr_filename)+"_____"+
      filefunc::getbasename(next_filename)+".dat";
//   string banner="Exporting fundamental matrix to output file "+
//      fundamental_filename;
//   outputfunc::write_big_banner(banner);

   curr_fundamental_ptr->export_best_matrix(i,j,fundamental_filename);
}

// ---------------------------------------------------------------------
// Member function export_homography_matrix() takes in integer
// indices i and j that label the current photo pair.  It writes out
// the 3x3 homography matrix corresponding for this pair to a text
// file whose name contains those of the two photos.

void sift_detector::export_homography_matrix(
   string bundler_IO_subdir,int i,int j)
{
//   cout << "inside sift_detector::export_homography_matrix" << endl;

   string homography_subdir=bundler_IO_subdir+"homography_matrices/";
   filefunc::dircreate(homography_subdir);
   
   string curr_filename=photogroup_ptr->get_photograph_ptr(i)->
      get_filename();
   string next_filename=photogroup_ptr->get_photograph_ptr(j)->
      get_filename();

   string homography_filename=homography_subdir+"homography_____"+
      filefunc::getbasename(curr_filename)+"_____"+
      filefunc::getbasename(next_filename)+".dat";
   string banner="Exporting homography matrix to output file "+
      homography_filename;
   outputfunc::write_big_banner(banner);

   H_ptr->export_matrix(homography_filename);
}

// ---------------------------------------------------------------------
// Member function export_features_to_Lowe_keyfile() generates an
// output text file which follows Lowe's SIFT keyfile conventions.  We
// wrote this method so that we could export SIFT + ASIFT features to
// a keyfile which could subsequently be imported into BUNDLER.

void sift_detector::export_features_to_Lowe_keyfile(
   int photo_ydim,string output_keyfilename,
   const vector<feature_pair>& currimage_feature_info)
{
//   cout << "inside sift_detector::export_features_as_Lowe_keyfile()"
//        << endl;
//   cout << "output_keyfilename = " << output_keyfilename << endl;

   unsigned int n_features=currimage_feature_info.size();
//   cout << "n_features = " << n_features << endl;
   d_dims=128;

   ofstream outstream;
   filefunc::openfile(output_keyfilename,outstream);
   outstream << n_features << " " << d_dims << endl;

   for (unsigned int f=0; f<n_features; f++)
   {
      descriptor* F_ptr=currimage_feature_info[f].first;

      double U=F_ptr->get(1);
      double V=F_ptr->get(2);
      double py=(1-V)*photo_ydim;
      double px=U*photo_ydim;
      
      outstream << py << " " << px << " " << F_ptr->get(3) << " "
                << F_ptr->get(4) << endl;

      descriptor* D_ptr=currimage_feature_info[f].second;
      for (unsigned int d=0; d<d_dims; d++)
      {
         int curr_D=D_ptr->get(d);
         outstream << curr_D << " ";
         if ((d+1)%20==0) outstream << endl;
      }
      outstream << endl;
      
   } // loop over index f labeling feature info

   filefunc::closefile(output_keyfilename,outstream);
}

// =========================================================================
// Horn's relative orientation and baseline determination member functions
// =========================================================================

// Member function convert_matching_binocular_features_into_rays()
// takes in "left" and "right" cameras whose projection matrices have
// trivial extrinsic camera parameter values.  It loops over all
// entries within the features_map and converts their 2D (U,V)
// coordinates into corresponding ray direction vectors.  This method
// returns an STL vector of "left" and "right" ray direction vector
// pairs.

vector<pair<threevector,threevector> >
sift_detector::convert_matching_binocular_features_into_rays(
   camera* cameraL_ptr,camera* cameraR_ptr)
{
   cout << "inside sift_detector::convert_matching_binocular_features_into_rays()"
        << endl;

   vector<pair<threevector,threevector> > LR_rays;
   
   for (FEATURES_MAP::iterator iter=features_map.begin();
        iter != features_map.end(); iter++)
   {
      int feature_ID=iter->first;
      vector<fourvector> feature_coords=iter->second;

      cout << "feature: ID = " << feature_ID << endl;
      for (unsigned int i=0; i<feature_coords.size(); i++)
      {
         cout << " pass = " << feature_coords[i].get(0)
              << " U = " << feature_coords[i].get(1)
              << " V = " << feature_coords[i].get(2) << endl << endl;
      }

      twovector UV_L(feature_coords[0].get(1),feature_coords[0].get(2));
      threevector rhat_L=cameraL_ptr->pixel_ray_direction(UV_L);
      twovector UV_R(feature_coords[1].get(1),feature_coords[1].get(2));
      threevector rhat_R=cameraR_ptr->pixel_ray_direction(UV_R);

      cout << "rhat_L = " << rhat_L << endl;
      cout << "rhat_R = " << rhat_R << endl;

      pair<threevector,threevector> P(rhat_L,rhat_R);
      LR_rays.push_back(P);
   } // loop over iter index
   return LR_rays;
}

// =========================================================================
// 3x3 projection matrix member functions
// =========================================================================

// Member function compute_ray_feature_homography() reconstructs the
// 3x3 homogeneous matrix H which maps 3D panoramic rays to 2D (U,V)
// features.  This method returns the number of RMS residual between
// the rays projected into the image plane and their UV tiepoint
// counterparts.

double sift_detector::compute_ray_feature_homography(
   vector<feature_pair>* feature_ray_ptr,
   double input_frac_to_use,bool check_ray_projection_flag)
{
//   cout << "inside sift_detector::compute_ray_feature_homography" << endl;

   unsigned int n_features=feature_ray_ptr->size();

// First extract 3D rays and 2D features which form tiepoint pairs
// into STL vectors:

   vector<threevector> nhat;
   vector<twovector> UV;

   for (unsigned int f=0; f<n_features; f++)
   {
      descriptor* F_ptr=feature_ray_ptr->at(f).first;
      if (fabs(F_ptr->get(6)) < 0.5*POSITIVEINFINITY)
      {
//         int ID=F_ptr->get(0);
         UV.push_back(recover_UV_from_F(F_ptr));
         nhat.push_back(recover_nhat_from_F(F_ptr));
      }
   } // loop over index f labeling features

   return compute_ray_feature_homography(
      UV,nhat,input_frac_to_use,check_ray_projection_flag);
}

// ---------------------------------------------------------------------
double sift_detector::compute_ray_feature_homography(
   const vector<twovector>& UV,const vector<threevector>& nhat,
   double input_frac_to_use,bool check_ray_projection_flag)
{
//   cout << "inside sift_detector::compute_ray_feature_homography" << endl;

// Solve for the 3x3 projection matrix H that maps 3D rays onto
// their 2D (U,V) feature counterparts:
   
   H_ptr->parse_projection_inputs(nhat,UV);
   H_ptr->compute_homography_matrix();

// Check mapping of 3D rays onto 2D features via 3x3 homogeneous
// matrix H.  Sort features according to the residuals between the UV
// and projected UV coordinates:

   double RMS_residual=-1;
   if (check_ray_projection_flag)
   {
      bool print_flag=false;
//      bool print_flag=true;
      RMS_residual=
         H_ptr->check_ray_projection(nhat,UV,input_frac_to_use,print_flag);
   }
   return RMS_residual;
}

// ---------------------------------------------------------------------
// Member function compute_projection_matrix() reconstructs the 3x4
// projection matrices for an uncalibrated input photograph.

void sift_detector::compute_projection_matrix(photograph* photograph_ptr)
{
//   cout << "inside sift_detector::compute_projection_matrix()" << endl;
   string banner="Computing 3x4 projection matrix:";
   outputfunc::write_banner(banner);

   camera* camera_ptr=photograph_ptr->get_camera_ptr();
   if (!camera_ptr->get_calibration_flag())
   {
      cout << "photograph_ptr->get_filename() = "
           << photograph_ptr->get_filename() << endl;

      genmatrix P(3,4);
      compute_projection_matrix(camera_ptr->get_world_posn(),&P);
      camera_ptr->set_projection_matrix(P);
      camera_ptr->decompose_projection_matrix();
//      cout << "*camera_ptr = " << *camera_ptr << endl;
      camera_ptr->print_external_and_internal_params();
   }
}

// ---------------------------------------------------------------------
// Member function compute_projection_matrix() takes in from member
// homography *H_ptr the 3x3 homogeneous matrix which maps 3D
// panoramic rays to 2D (U,V) features in photograph i.  If the camera
// lies at (0,0,0), this method returns 3x4 projection matrix 
// P = [H | 0].

void sift_detector::compute_projection_matrix(
   const threevector& camera_world_posn,genmatrix* P_ptr)
{
//   cout << "inside sift_detector::compute_projection_matrix()" << endl;
//   cout << "camera_world_posn = " << camera_world_posn << endl;

// Fill entries of input 3x4 projection matrix *P_ptr with those from
// member homography *H_ptr.  

   genmatrix* H3x3_ptr=H_ptr->get_H_ptr();
//   cout << "H3x3 = " << *H3x3_ptr << endl;
   threevector p4=- *H3x3_ptr * camera_world_posn;
//   cout << "p4 = " << p4 << endl;

   for (unsigned int i=0; i<3; i++)
   {
      for (unsigned int j=0; j<3; j++)
      {
         P_ptr->put(i,j,H3x3_ptr->get(i,j));
      }
      P_ptr->put(i,3,p4.get(i));
   }
}

// ---------------------------------------------------------------------
// Member function write_projection_package_file takes in photograph
// *photograph_ptr whose camera we assume has already been calibrated
// and whose 3x4 projection matrix *P_ptr has already been calculated.
// This method writes out a package file containing the photograph's
// filename and P_ptr.  The package file can be read in by program
// NEW_FOV in order to display the photograph as a 3D OBSFRUSTUM.

void sift_detector::write_projection_package_file(
   double frustum_sidelength,double downrange_distance,
   string output_package_subdir,photograph* photograph_ptr)
{
   camera* camera_ptr=photograph_ptr->get_camera_ptr();
   const genmatrix* P_ptr=camera_ptr->get_P_ptr();
   string photograph_filename=photograph_ptr->get_filename();
   string basename=filefunc::getbasename(photograph_filename);

   string package_filename=
      output_package_subdir+stringfunc::prefix(basename)+".pkg";

   string banner="Writing projection package file to "+package_filename;
   outputfunc::write_banner(banner);

   ofstream outstream;
   outstream.precision(12);
   
   filefunc::openfile(package_filename,outstream);
   outstream << photograph_filename << endl;
   outstream << "--frustum_sidelength "+stringfunc::number_to_string(
      frustum_sidelength) << endl;
   outstream << "--downrange_distance "+stringfunc::number_to_string(
      downrange_distance) << endl;
   outstream << "--projection_matrix" << endl;
   for (unsigned int i=0; i<3; i++)
   {
      for (unsigned int j=0; j<4; j++)
      {
         outstream << P_ptr->get(i,j) << "   ";
      }
      outstream << endl;
   }
   filefunc::closefile(package_filename,outstream);
}



// =========================================================================



// ---------------------------------------------------------------------
bool sift_detector::identify_candidate_feature_matches_via_Lowe_ratio_test_2(
   int i,int j,double sqrd_max_ratio)
{ 
   cout << "inside sift_detector::identify_candidate_feature_matches_via_Lowe_ratio_test_2()" 
        << endl;

// Load features for image i into currimage_feature_info:

   vector<feature_pair> currimage_feature_info=image_feature_info[i];
   cout << "currimage_feature_info.size() = "
        << currimage_feature_info.size() << endl;
   quantize_currimage_feature_info(currimage_feature_info);

// Load all features for image j into nextimage_feature_info:

   vector<feature_pair> nextimage_feature_info=image_feature_info[j];
   cout << "nextimage_feature_info.size() = "
        << nextimage_feature_info.size() << endl;
   quantize_cumimage_feature_info(nextimage_feature_info);

   int n_matches=0;
   candidate_tiepoint_pairs.clear();

   int n_curr_SOH_candidates=0;
   int n_next_SOH_candidates=0;

   timefunc::initialize_timeofday_clock();
   for (curr_SOH_corner_iter=curr_SOH_corner_descriptor_map.begin();
        curr_SOH_corner_iter != curr_SOH_corner_descriptor_map.end();
        curr_SOH_corner_iter++)
   {
      quadruple curr_quadruple=curr_SOH_corner_iter->first;
      vector<feature_pair> curr_feature_info=curr_SOH_corner_iter->second;

      for (unsigned int corner=0; corner<4; corner++)
      {
         for (int k=0; k<=0; k++)
//         for (int k=-1; k<=1; k++)
//         for (int k=-2; k<=2; k++)
         {
            quadruple cum_quadruple=curr_quadruple;
            if (corner==0)
            {
               cum_quadruple.first += k;
            }
            else if (corner==1)
            {
               cum_quadruple.second += k;
            }
            else if (corner==2)
            {
               cum_quadruple.third += k;
            }
            else if (corner==3)
            {
               cum_quadruple.fourth += k;
            }

            cum_SOH_corner_iter=cum_SOH_corner_descriptor_map.find(
               cum_quadruple);
            if (cum_SOH_corner_iter==cum_SOH_corner_descriptor_map.end())
               continue;

            vector<feature_pair> next_feature_info=cum_SOH_corner_iter->second;

            n_curr_SOH_candidates += curr_feature_info.size();
            n_next_SOH_candidates += next_feature_info.size();
            

// Loop over features in image i and search for counterparts in image j:
   
            n_matches += 
               identify_candidate_SOH_feature_matches_for_image_pair(
                  curr_feature_info,next_feature_info);

//            cout << "n_candidate_matches = " << n_matches << endl;

         } // loop over index k 
      } // loop over corner index
   } // loop over curr_SOH_corner_iter iterator

   cout << "n_curr_SOH_candidates = " << n_curr_SOH_candidates << endl;
   cout << "n_next_SOH_candidates = " << n_next_SOH_candidates << endl;

//   cout << "n_candidate_matches = " << n_matches << endl;
   outputfunc::print_elapsed_time();
   outputfunc::enter_continue_char();

   return (n_matches > 0);
}

// =========================================================================
// Binary SIFT member functions
// =========================================================================

// Member function compute_descriptor_component_medians() loops over
// all d_dim components of all descriptors.  It computes the median
// value for each component and stores the results within member STL
// vector median_d_values.

vector<int>& sift_detector::compute_descriptor_component_medians() 
{
   cout << "inside sift_detector::compute_descriptor_component_medians()"
        << endl;

   vector<int> d_values;
   d_values.reserve(n_images*50000);

   int n_images=get_image_feature_info().size();
   cout << "n_images = " << n_images << endl;

   double min_midpoint_dist=POSITIVEINFINITY;

   median_d_values.clear();
   for (unsigned int d=0; d<d_dims; d++)
   {
      d_values.clear();

      for (int i=0; i<n_images; i++)
      {
         vector<feature_pair>* currimage_feature_info_ptr=
            get_image_feature_info_ptr(i);

         int n_features=currimage_feature_info_ptr->size();
         for (int j=0; j<n_features; j++)
         {
            descriptor* D_ptr=currimage_feature_info_ptr->at(j).second;
            d_values.push_back(D_ptr->get(d));

//            cout << "d = " << d << " i = " << i << " j = " << j 
//                 << " d_value = " << d_values.back() << endl;
         } // loop over index j labeling current image features
      } // loop over index i labeling input images

      std::sort(d_values.begin(),d_values.end());
      median_d_values.push_back(d_values[d_values.size()/2]);
      cout << "d = " << d 
           << "  d_median = " << median_d_values.back() 
           << "  d_values.size() = " << d_values.size() 
           << endl;

// Keep track of median_d_value closest to 256/2=128:

      double curr_midpoint_dist=fabs(median_d_values.back()-128);
      if (curr_midpoint_dist < min_midpoint_dist)
      {
         min_midpoint_dist=curr_midpoint_dist;
         d_index_1=d;
      }

      cout << "d_index_1 = " << d_index_1
           << " min_midpoint_dist = " << min_midpoint_dist << endl;

   } // loop over index d labeling SIFT descriptor components

/*
   std::sort(median_d_values.begin(),median_d_values.end());
   for (int d=0; d<d_dims; d++)
   {
      cout << "d = " << d << " Sorted median_d_values = "
           << median_d_values[d] << endl;
   }
*/

   cout << "d_index_1 = " << d_index_1
        << " min_midpoint_dist = " << min_midpoint_dist << endl;

   return median_d_values;
}

// ---------------------------------------------------------------------
// Member function binary_quantize_SIFT_descriptors() loops over all
// features within all images.  It compares each component of each
// feature descriptor with the component's median value stored in
// median_d_values.  A binary 1 or 0 value is assigned based upon the
// component being greater or less than the median value.  Binary
// quantized feature descriptors are stored within member STL vector 
// Dbinary_ptrs.

void sift_detector::binary_quantize_SIFT_descriptors()
{
   cout << "inside sift_detector::binary_quantize_SIFT_descriptors()"
        << endl;

   int n_images=get_image_feature_info().size();
   vector<descriptor*> curr_Dbinary_ptrs;
   for (int i=0; i<n_images; i++)
   {
      cout << "Quantizing descriptors for image i = " << i << endl;
      vector<feature_pair>* currimage_feature_info_ptr=
         get_image_feature_info_ptr(i);

      curr_Dbinary_ptrs.clear();
      
      int n_features=currimage_feature_info_ptr->size();
      for (int j=0; j<n_features; j++)
      {
         descriptor* D_ptr=currimage_feature_info_ptr->at(j).second;

// Store quantized version of D_ptr within a new d-dimensional
// descriptor *Dbinary_ptr:

         descriptor* Dbinary_ptr=new descriptor(d_dims);
         curr_Dbinary_ptrs.push_back(Dbinary_ptr);
         for (unsigned int d=0; d<d_dims; d++)
         {
            int d_value=D_ptr->get(d);
            if (d_value > median_d_values[d])
            {
               Dbinary_ptr->put(d,1);
            }
            else
            {
               Dbinary_ptr->put(d,0);
            }
         }
      } // loop over index j labeling current image features
      Dbinary_ptrs.push_back(curr_Dbinary_ptrs);

   } // loop over index i labeling input images
}

// ---------------------------------------------------------------------
// Member function generate_VPtrees()

void sift_detector::generate_VPtrees()
{
   cout << "inside sift_detector::generate_VPtrees()" << endl;

   int n_images=get_image_feature_info().size();
   for (int i=0; i<n_images; i++)
   {
      cout << "Generating VPtree for image " << i << endl;
      vptree* vptree_ptr=new vptree();
      vptree_ptrs.push_back(vptree_ptr);

      vptree_ptr->set_hamming_distance_flag(true);
      vptree_ptr->construct_tree(*(get_Dbinary_ptrs_ptr(i)));
   } // loop over index i labeling images
}

/*
// ---------------------------------------------------------------------
// Member function 

void sift_detector::match_binary_SIFT_descriptors()
{
   cout << "inside sift_detector::match_binary_SIFT_descriptors()"
        << endl;

   int n_images=get_image_feature_info().size();

   for (int i=0; i<n_images; i++)
   {
      vector<unsigned long >* currimage_binary_descriptor_left_ptr=
         get_binary_descriptor_left_ptr(i);
      vector<unsigned long >* currimage_binary_descriptor_right_ptr=
         get_binary_descriptor_right_ptr(i);
      int n_curr_binary_descriptors=currimage_binary_descriptor_left_ptr->
         size();

      for (int j=i+1; j<n_images; j++)
      {
         cout << "Matching binary descriptors for image i = " << i 
              << " and image j = " << j << endl;
         vector<unsigned long >* nextimage_binary_descriptor_left_ptr=
            get_binary_descriptor_left_ptr(j);
         vector<unsigned long >* nextimage_binary_descriptor_right_ptr=
            get_binary_descriptor_right_ptr(j);
         int n_next_binary_descriptors=nextimage_binary_descriptor_left_ptr->
            size();

         cout << "n_curr_binary_descriptors = " 
              << n_curr_binary_descriptors << endl;
         cout << "n_next_binary_descriptors = " 
              << n_next_binary_descriptors << endl;

         for (int a=0; a<n_curr_binary_descriptors; a++)
         {
            if (a%1000==0) cout << a << endl;
            for (int b=0; b<n_next_binary_descriptors; b++)
            {
               unsigned long left_hamming_dist=binaryfunc::hamming_distance(
                  currimage_binary_descriptor_left_ptr->at(a),
                  nextimage_binary_descriptor_left_ptr->at(b));
               unsigned long right_hamming_dist=binaryfunc::hamming_distance(
                  currimage_binary_descriptor_right_ptr->at(a),
                  nextimage_binary_descriptor_right_ptr->at(b));
            } // loop over index b labeling next binary descriptors
         } // loop over index a labeling current binary descriptors
         
      } // loop over index j labeling next image
   } // loop over index i labeling curr image
}
*/

// ---------------------------------------------------------------------
// Member function check_hamming_distances()

void sift_detector::check_hamming_distances(int i,int j)
{
   cout << "inside sift_detector::check_hamming_distances()" << endl;
   
   vector<feature_pair>* currimage_feature_info_ptr=&(image_feature_info[i]);
   vector<feature_pair>* nextimage_feature_info_ptr=&(image_feature_info[j]);

   typedef map<int,feature_pair> ID_FEATUREPAIR_MAP;
// Independent int = feature_ID
// Dependent feature_pair = (F_ptr,D_ptr)   

   ID_FEATUREPAIR_MAP* curr_ID_featurepair_map_ptr=new ID_FEATUREPAIR_MAP;
   ID_FEATUREPAIR_MAP* next_ID_featurepair_map_ptr=new ID_FEATUREPAIR_MAP;
   ID_FEATUREPAIR_MAP::iterator curr_iter,next_iter;

   for (unsigned int f=0; f<currimage_feature_info_ptr->size(); f++)
   {
      descriptor* F_ptr=currimage_feature_info_ptr->at(f).first;
      int currfeature_ID=F_ptr->get(0);
      (*curr_ID_featurepair_map_ptr)[currfeature_ID]=
         currimage_feature_info_ptr->at(f);
   }

   for (unsigned int g=0; g<nextimage_feature_info_ptr->size(); g++)
   {
      descriptor* F_ptr=nextimage_feature_info_ptr->at(g).first;
      int nextfeature_ID=F_ptr->get(0);
      (*next_ID_featurepair_map_ptr)[nextfeature_ID]=
         nextimage_feature_info_ptr->at(g);
   }

   vector<double> hamming_dists;
   for (curr_iter=curr_ID_featurepair_map_ptr->begin();
        curr_iter != curr_ID_featurepair_map_ptr->end(); curr_iter++)
   {
      int curr_feature_ID=curr_iter->first;
      next_iter=next_ID_featurepair_map_ptr->find(curr_feature_ID);

      if (next_iter==next_ID_featurepair_map_ptr->end()) continue;

      descriptor* curr_D_ptr=curr_iter->second.second;
      descriptor* next_D_ptr=next_iter->second.second;

      descriptor* curr_Dbinary_ptr=new descriptor(d_dims);
      descriptor* next_Dbinary_ptr=new descriptor(d_dims);

      for (unsigned int d=0; d<d_dims; d++)
      {
         if (curr_D_ptr->get(d) > median_d_values[d])
         {
            curr_Dbinary_ptr->put(d,1);
         }
         else
         {
            curr_Dbinary_ptr->put(d,0);
         }

         if (next_D_ptr->get(d) > median_d_values[d])
         {
            next_Dbinary_ptr->put(d,1);
         }
         else
         {
            next_Dbinary_ptr->put(d,0);
         }
      } // loop over index d labeling descriptor components

      hamming_dists.push_back(
         binaryfunc::hamming_distance(curr_Dbinary_ptr,next_Dbinary_ptr));

   } // loop over curr_iter

   delete curr_ID_featurepair_map_ptr;
   delete next_ID_featurepair_map_ptr;

   prob_distribution prob_hamming(hamming_dists,100);
   prob_hamming.writeprobdists(false);
}

// ---------------------------------------------------------------------
// Member function check_binary_matches() works with "genuine"
// SIFT/ASIFT feature matches.  It binary quantizes the d-dim
// descriptors for all matching features for image i and j.  This
// method then compares the binary quantized values for the special
// bin labeled by d_index_1 with median_d_values[d_index_1].  It
// computes the fraction of tiepoint pairs for which the
// binary-quantized descriptors are either both greater than or less
// than median_d_values[d_index_1].  On 4/30/13, we empirically found
// that this fraction exceeds 80% and is often close to 90%!

void sift_detector::check_binary_matches(int i,int j)
{
   cout << "inside sift_detector::check_binary_matches()" << endl;
   
   vector<feature_pair>* currimage_feature_info_ptr=&(image_feature_info[i]);
   vector<feature_pair>* nextimage_feature_info_ptr=&(image_feature_info[j]);

   typedef map<int,feature_pair> ID_FEATUREPAIR_MAP;
// Independent int = feature_ID
// Dependent feature_pair = (F_ptr,D_ptr)   

   ID_FEATUREPAIR_MAP* curr_ID_featurepair_map_ptr=new ID_FEATUREPAIR_MAP;
   ID_FEATUREPAIR_MAP* next_ID_featurepair_map_ptr=new ID_FEATUREPAIR_MAP;
   ID_FEATUREPAIR_MAP::iterator curr_iter,next_iter;

   for (unsigned int f=0; f<currimage_feature_info_ptr->size(); f++)
   {
      descriptor* F_ptr=currimage_feature_info_ptr->at(f).first;
      int currfeature_ID=F_ptr->get(0);
      (*curr_ID_featurepair_map_ptr)[currfeature_ID]=
         currimage_feature_info_ptr->at(f);
   }

   for (unsigned int g=0; g<nextimage_feature_info_ptr->size(); g++)
   {
      descriptor* F_ptr=nextimage_feature_info_ptr->at(g).first;
      int nextfeature_ID=F_ptr->get(0);
      (*next_ID_featurepair_map_ptr)[nextfeature_ID]=
         nextimage_feature_info_ptr->at(g);
   }

   int hits_1=0;
   int misses_1=0;
   for (curr_iter=curr_ID_featurepair_map_ptr->begin();
        curr_iter != curr_ID_featurepair_map_ptr->end(); curr_iter++)
   {
      int curr_feature_ID=curr_iter->first;
      next_iter=next_ID_featurepair_map_ptr->find(curr_feature_ID);

      if (next_iter==next_ID_featurepair_map_ptr->end()) continue;

      descriptor* curr_D_ptr=curr_iter->second.second;
      descriptor* next_D_ptr=next_iter->second.second;

      descriptor* curr_Dbinary_ptr=new descriptor(d_dims);
      descriptor* next_Dbinary_ptr=new descriptor(d_dims);

      for (unsigned int d=0; d<d_dims; d++)
      {
         if (curr_D_ptr->get(d) > median_d_values[d])
         {
            curr_Dbinary_ptr->put(d,1);
         }
         else
         {
            curr_Dbinary_ptr->put(d,0);
         }

         if (next_D_ptr->get(d) > median_d_values[d])
         {
            next_Dbinary_ptr->put(d,1);
         }
         else
         {
            next_Dbinary_ptr->put(d,0);
         }
      } // loop over index d labeling descriptor components

      if (curr_Dbinary_ptr->get(d_index_1)==next_Dbinary_ptr->get(d_index_1))
      {
         hits_1++;
      }
      else
      {
         misses_1++;
      }
   } // loop over curr_iter

   delete curr_ID_featurepair_map_ptr;
   delete next_ID_featurepair_map_ptr;

   int n_comparisons=hits_1+misses_1;
   double hits_1_frac=double(hits_1)/double(n_comparisons);
   cout << "hits_1_frac = " << hits_1_frac << endl;
}


