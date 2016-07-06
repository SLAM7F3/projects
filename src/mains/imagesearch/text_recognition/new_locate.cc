// ==========================================================================
// Program NEW_LOCATE imports the probabilistic classifier
// function computed with a gaussian kernel exported by
// program SVM_SHAPE.  It also imports the probabilistic classified
// function computed with a linear kernel exported by program TEXT.  
// LOCATE_CHARS first computes connected components within
// binary thresholded versions of an input image for thresholds
// ranging from 255 down to 0.  Following Neumann and Matas,
// "Real-time scene text localization and recognition", CVPR 2012, we
// compute aspect_ratio, compactness, n_holes and median horizontal
// crossings for each candidate extremal region.  These features are
// mapped into a probability that the candidate extremal region
// corresponds to a text character.

// Coates-Ng text localization is subsequently performed within
// bounding boxes nominated via the extremal region shape procedure.  

// Finally, bounding boxes are placed around detected text characters.
// A composite displaying the locations
// of all candidate text bounding boxes is exported as a jpeg image.

//				new_locate

// ==========================================================================
// Last updated on 8/3/12; 8/4/12; 8/5/12; 11/11/15
// ==========================================================================

#include  <fstream>
#include  <iostream>
#include  <map>
#include  <string>
#include  <vector>
#include "dlib/svm.h"

#include "image/binaryimagefuncs.h"
#include "image/compositefuncs.h"
#include "video/connected_components.h"
#include "image/connectfuncs.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "image/graphicsfuncs.h"
#include "image/imagefuncs.h"
#include "math/ltfourvector.h"
#include "numrec/nrfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "classification/text_detector.h"
#include "video/texture_rectangle.h"
#include "math/threevector.h"
#include "time/timefuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::map;
using std::string;
using std::vector;

using namespace dlib;

int main(int argc, char* argv[])
{
   cout.precision(12);

// The svm functions use column vectors to contain a lot of the
// data on which they operate. So the first thing we do here is
// declare a convenient typedef.

// This typedef declares a matrix with K rows and 1 column.  It will
// be the object that contains each of our K dimensional samples. 

//   const int K_shapes=4;
//   const int K_shapes=7;
   const int K_shapes=11;
   typedef matrix<double, K_shapes, 1> shapes_sample_type;
   shapes_sample_type shapes_sample;

// This is a typedef for the type of kernel we are going to use in
// this example.  In this case I have selected the gaussian kernel that
// can operate on our K-dim shapes_sample_type objects

   typedef radial_basis_kernel<shapes_sample_type> shapes_kernel_type;

// Another thing that is worth knowing is that just about everything
// in dlib is serializable. So for example, you can save the
// learned_pfunct object to disk and recall it later like so:

   typedef decision_function<shapes_kernel_type> shapes_dec_funct_type;
   typedef normalized_function<shapes_dec_funct_type> shapes_funct_type;
   shapes_funct_type shapes_funct;

   string projects_rootdir = getenv("PROJECTSROOT");
   string home_subdir=projects_rootdir+"/src/mains/imagesearch/text_recognition/";

   string learned_funcs_subdir=home_subdir+"learned_functions/";
   string shapes_binary_filename=learned_funcs_subdir+
      "shapes_bifunc_15938_10824_11features.dat";            
//      "shapes_bifunc_6408_5165_11features.dat";
//      "shapes_bifunc_6408_5165_7features.dat";
//      "shapes_bifunc_6408_5165.dat";
//      "shapes_bifunc_1143_3568.dat";
//      "shapes_bifunc_1555_3131.dat";
   cout << "shapes_binary_filename = " << shapes_binary_filename << endl;
   ifstream fin1(shapes_binary_filename.c_str(),ios::binary);
   deserialize(shapes_funct, fin1);

   typedef probabilistic_decision_function<shapes_kernel_type> 
      shapes_probabilistic_funct_type;  
   typedef normalized_function<shapes_probabilistic_funct_type> 
      shapes_pfunct_type;
   shapes_pfunct_type shapes_pfunct;

   string shapes_pfunct_filename=learned_funcs_subdir+
      "shapes_pfunct_15938_10824_11features.dat";		// text
//      "shapes_pfunct_6408_5165_11features.dat";
//      "shapes_pfunct_6408_5165_7features.dat";
//      "shapes_pfunct_6408_5165.dat";
//      "shapes_pfunct_1143_3568.dat";
//      "shapes_pfunct_1555_3131.dat";
   cout << "shapes_pfunct_filename = " << shapes_binary_filename << endl;
   ifstream fin2(shapes_pfunct_filename.c_str(),ios::binary);
   deserialize(shapes_pfunct, fin2);

/*
// Import binary and probabilistic decision functions trained on text
// and non-text randomness features:

   const int K_random=1;
//   const int K_random=6;
   typedef matrix<double, K_random, 1> random_sample_type;
   random_sample_type random_sample;

// This is a typedef for the type of kernel we are going to use in
// this example.  In this case I have selected the gaussian kernel that
// can operate on our K-dim shapes_sample_type objects

   typedef radial_basis_kernel<random_sample_type> random_kernel_type;

// Another thing that is worth knowing is that just about everything
// in dlib is serializable. So for example, you can save the
// learned_pfunct object to disk and recall it later like so:

   typedef decision_function<random_kernel_type> random_dec_funct_type;
   typedef normalized_function<random_dec_funct_type> random_funct_type;
   random_funct_type random_funct;

   string random_binary_filename=learned_funcs_subdir+
      "randomness_bifunc_2524_1640.dat";
   cout << "random_binary_filename = " << random_binary_filename << endl;
   ifstream fin3(random_binary_filename.c_str(),ios::binary);
   deserialize(random_funct, fin3);

   typedef probabilistic_decision_function<random_kernel_type> 
      random_probabilistic_funct_type;  
   typedef normalized_function<random_probabilistic_funct_type> 
      random_pfunct_type;
   random_pfunct_type random_pfunct;

   string random_pfunct_filename=learned_funcs_subdir+
      "randomness_pfunct_2524_1640.dat";
   cout << "random_pfunct_filename = " << random_binary_filename << endl;
   ifstream fin4(random_pfunct_filename.c_str(),ios::binary);
   deserialize(random_pfunct, fin4);
*/

// Import binary and probabilistic decision functions generated by an
// SVM with a linear kernel on 50K text char and non-char images:

   const int K_Ng=512;
   const int nineK=9*K_Ng;
   typedef matrix<double, nineK, 1> Ng_sample_type;
   typedef linear_kernel<Ng_sample_type> Ng_kernel_type;
   Ng_sample_type Ng_sample;

   typedef decision_function<Ng_kernel_type> Ng_dec_funct_type;
   typedef normalized_function<Ng_dec_funct_type> Ng_funct_type;
   Ng_funct_type Ng_funct;

   typedef probabilistic_decision_function<Ng_kernel_type> 
      Ng_probabilistic_funct_type;  
   typedef normalized_function<Ng_probabilistic_funct_type> 
      Ng_pfunct_type;
   Ng_pfunct_type Ng_pfunct;

   string learned_Ng_binary_filename=learned_funcs_subdir+
      "Ng_bifunc_50K.dat";
//      "Ng_bifunc_19871.dat";
   cout << "learned_Ng_binary_filename = " 
        << learned_Ng_binary_filename << endl;
   ifstream fin5(learned_Ng_binary_filename.c_str(),ios::binary);
   deserialize(Ng_funct, fin5);

   string learned_Ng_pfunct_filename=learned_funcs_subdir+
      "Ng_pfunct_50K.dat";
//      "Ng_pfunct_19871.dat";
   cout << "learned_Ng_pfunct_filename = "
        << learned_Ng_pfunct_filename << endl;
   ifstream fin6(learned_Ng_pfunct_filename.c_str(),ios::binary);
   deserialize(Ng_pfunct, fin6);
 
   double Ng_char_threshold=0.5;
//   cout << "Enter Ng char threshold:" << endl;
//   cin >> Ng_char_threshold;

// Import dictionary trained on 50K text and non-text images:

   string dictionary_subdir=home_subdir+"training_data/dictionary/";
   text_detector* text_detector_ptr=new text_detector(K_Ng,dictionary_subdir);
   text_detector_ptr->import_mean_and_covar_matrices();
   text_detector_ptr->import_dictionary();
   texture_rectangle* texture_rectangle_ptr=text_detector_ptr->
      get_texture_rectangle_ptr();

// ----------------------------------------------------------------------
// Sample images hardwired for testing and development purposes:   

   cout << "0: light C against dark background" << endl;
   cout << "1: light FAVOURITE against dark background" << endl;
   cout << "2: light WWW against dark background" << endl;
   cout << "3: light library against dark background" << endl;
   cout << "4: light library against dark background" << endl;
   cout << "5: light open against dark background" << endl;
   cout << "6: light ROGERS against red background" << endl;
   cout << "7: kermit000" << endl;
   cout << "8: Whole foods interior" << endl;
   cout << "9: Pork store front" << endl;
   cout << "10: Garden Room awning" << endl;
   cout << "11: GREGGS store window" << endl;
   cout << "12: Street signs" << endl;
   cout << "13: Main Street sign" << endl;
   cout << "14: Do Not Block sign" << endl;
   cout << "15: Dilemma Cafe" << endl;
   cout << "16: News hour frame" << endl;
   cout << "17: Many signs" << endl;
   cout << "18: Cropped stop sign" << endl;
   cout << "19: Pork store sign" << endl;
   cout << "20: simulated image" << endl;
   cout << "21: Enter image filename from curr subdir" << endl;

   int image_number=0;
//   int image_number=7;
   cout << "Enter image number:" << endl;
   cin >> image_number;

   string image_subdir;
   if (image_number==0)
   {
      image_subdir=home_subdir+
         "training_data/char_intensity/bright_chars_dark_background/";
   }
   else if (image_number >= 1 && image_number <= 5)
   {
      image_subdir=home_subdir+"training_data/icdar03/words/test/2/";
   }
   else if (image_number >=6 && image_number <= 6)
   {
      image_subdir=home_subdir+
         "training_data/icdar03/words/test/3/";
   }
   else if (image_number >= 7 && image_number <= 7)
   {
      image_subdir="/data/ImageEngine/kermit/";
   }
   else if (image_number >=8 && image_number <= 11)
   {
      image_subdir=home_subdir+"training_data/stores/";
   }
   else if (image_number >=12 && image_number <= 14)
   {
      image_subdir=home_subdir+"training_data/street_signs/";
   }
   else if (image_number >=15 && image_number <= 15)
   {
      image_subdir=home_subdir+"training_data/text_database/";
   }
   else if (image_number >=16 && image_number <= 16)
   {
      image_subdir=home_subdir+"training_data/NewsWrap/";
   }
   else if (image_number >=17 && image_number <= 18)
   {
      image_subdir=home_subdir+"training_data/street_signs/";
   }
   else if (image_number >=19 && image_number <= 19)
   {
      image_subdir=home_subdir+"training_data/stores/";
   }
   else if (image_number >=20 && image_number <= 21)
   {
      image_subdir="./";
   }
   
   string image_filename;
   if (image_number==0)
   {
      image_filename=image_subdir+"00285.jpg"; // light C/dark bkgnd
   }
   else if (image_number==1)
   {
      image_filename=image_subdir+"120.jpg"; // light FAVOURITE/dark bkgnd
   }
   else if (image_number==2)
   {
      image_filename=image_subdir+"122.jpg"; // light www on dark bkgnd
   }
   else if (image_number==3)
   {
      image_filename=image_subdir+"152.jpg"; // light library on dark bkgnd
   }
   else if (image_number==4)
   {
      image_filename=image_subdir+"157.jpg"; // light library on dark bkgnd
   }
   else if (image_number==5)
   {
      image_filename=image_subdir+"158.jpg"; // light open on dark bkgnd
   }
   else if (image_number==6)
   {
      image_filename=image_subdir+"227.jpg"; // 
   }
   else if (image_number==7)
   {
      image_filename=image_subdir+"kermit000.jpg"; // 
   }
   else if (image_number==8)
   {
      image_filename=image_subdir+"produce.jpg";
   }
   else if (image_number==9)
   {
      image_filename=image_subdir+"pork_store.jpg";
   }
   else if (image_number==10)
   {
      image_filename=image_subdir+"garden_room.jpg";
   }
   else if (image_number==11)
   {
      image_filename=image_subdir+"reduced_greggs.jpg";
   }
   else if (image_number==12)
   {
      image_filename=image_subdir+"P6180940.JPG";
   }
   else if (image_number==13)
   {
      image_filename=image_subdir+"MainStreet.jpg";
   }
   else if (image_number==14)
   {
      image_filename=image_subdir+"DoNotBlock.jpg";
   }
   else if (image_number==15)
   {
      image_filename=image_subdir+"reduced_text_img0251.png";
   }
   else if (image_number==16)
   {
      image_filename=image_subdir+"frame-0011.jpg";
   }
   else if (image_number==17)
   {
      image_filename=image_subdir+"000_0360.jpg";
   }
   else if (image_number==18)
   {
      image_filename=image_subdir+"stop_sign.jpg";
   }
   else if (image_number==19)
   {
      image_filename=image_subdir+"pork_store_sign.jpg";
   }
   else if (image_number==20)
   {
      image_filename=image_subdir+"./sim_image.jpg";
   }
   else if (image_number==21)
   {
      cout << "Enter image filename from current subdirectory:" << endl;
      cin >> image_filename;
      image_filename=image_subdir+image_filename;
   }

   std::vector<polygon> all_polygons;

   string bboxes_subdir=home_subdir+"bboxes/";
//   string unix_cmd="/bin/rm -r -f "+bboxes_subdir;
//   cout << "unix_cmd = " << unix_cmd << endl;
//   sysfunc::unix_command(unix_cmd);
   filefunc::dircreate(bboxes_subdir);


   int color_channel_ID=-1;
   cout << "Enter color channel ID:" << endl;
   cout << "-1 = value, 0 = saturation, 1 = red, 2 = green, 3 = blue" << endl;
//   cin >> color_channel_ID;
   
   timefunc::initialize_timeofday_clock();

   double max_sigma_z_threshold=0;
//   cout << "Enter max sigma z threshold:" << endl;
//   cin >> max_sigma_z_threshold;

   double max_skew_z_threshold=1.35;
//   cout << "Enter max skew z threshold:" << endl;
//   cin >> max_skew_z_threshold;

   double max_dimensionless_quartic_z_threshold=4;
//   cout << "Enter max dimensionless quartic z threshold:" << endl;
//   cin >> max_dimensionless_quartic_z_threshold;

   int counter=0;
   int index=0;
   int n_detections=0;
   int n_rejections=0;
   int n_sigmaz_rejections=0;
   int n_skewz_rejections=0;
   int n_quartic_z_rejections=0;
   int n_Ng_rejections=0;

   connected_components* connected_components_ptr=NULL;
   connectfunc::create_extremal_region_pooled_memory();

// Instantiate and fill STL map with bounding box corner coordinates:

   typedef std::map<fourvector,int,ltfourvector > BBOXES_MAP;
   BBOXES_MAP* bboxes_map_ptr=NULL;

// =======================================================================
// Outermost loop over bright vs dark characters starts here:

//   for (int brightness_iter=0; brightness_iter < 1; brightness_iter++)
//   for (int brightness_iter=1; brightness_iter < 2; brightness_iter++)
   for (int brightness_iter=0; brightness_iter < 2; brightness_iter++)
   {
      bool invert_binary_values_flag;
      int threshold_start,threshold_stop,d_threshold;

      if (brightness_iter==0)	// bright chars dark background
      {
         invert_binary_values_flag=false;
         threshold_start=254;
         threshold_stop=1;
         d_threshold=-1;
      }
      else if (brightness_iter==1) // dark chars bright background
      {
         invert_binary_values_flag=true; 
         threshold_start=1;
         threshold_stop=254;
         d_threshold=1;
      }

      delete connected_components_ptr;   
      connected_components_ptr=new connected_components();
      connected_components_ptr->set_shapes_pfunct_ptr(&shapes_pfunct);
      connected_components_ptr->reset_image(image_filename,color_channel_ID);

// Instantiate and fill STL map with bounding box corner coordinates:
      
      delete bboxes_map_ptr;
      BBOXES_MAP* bboxes_map_ptr=new BBOXES_MAP;
      // *bboxes_map_ptr independent var = left bottom & top right bbox corners
      // dependent var = dummy integer

      int min_cc_label=0;

// ----------------------------------------------------------------------
      for (int threshold=threshold_start; threshold != threshold_stop; 
           threshold += d_threshold)
      {
         int level=-1;
         if (!invert_binary_values_flag)
         {
            level=255-threshold;
         }
         else
         {
            level=threshold;
         }


         string region_type;
         if (brightness_iter==0)
         {
            region_type="bright_region";
         }
         else
         {
            region_type="dark_region";
         }

         cout << region_type 
//              << " threshold = " << threshold 
              << "  level=" << level 
              << endl;

         int n_connected_components=
            connected_components_ptr->new_compute_connected_components(
               index,threshold,level,invert_binary_values_flag);
//         cout << "n_connected_components = " << n_connected_components
//              << endl;

         int n_candidate_regions=0;
         for (int n=0; n<connected_components_ptr->get_n_extremal_regions(); 
              n++)
         {
            extremal_region* extremal_region_ptr=
               connected_components_ptr->get_extremal_region_ptr(n);
            if (extremal_region_ptr==NULL) continue;

            int ID=extremal_region_ptr->get_ID();
            if (ID==99999 || ID <= min_cc_label) continue;

//            cout << "ID = " << ID << endl;

/*
            int pixel_area=extremal_region_ptr->get_pixel_area();
            int pixel_perimeter=extremal_region_ptr->get_pixel_perim();
            int euler_number=extremal_region_ptr->get_Euler_number();

            int left_pu,bottom_pv,right_pu,top_pv;
            extremal_region_ptr->get_bbox(
               left_pu,bottom_pv,right_pu,top_pv);
            int pixel_width=right_pu-left_pu;
            int pixel_height=top_pv-bottom_pv;

            double aspect_ratio=extremal_region_ptr->get_aspect_ratio();
            double compactness=extremal_region_ptr->get_compactness();
            int n_holes=extremal_region_ptr->get_n_holes();
*/

            int median_horiz_crossings=extremal_region_ptr->
               get_n_horiz_crossings();

// Median horiz crossings are only calculated for extremal regions
// which have changed since the previous level.  Otherwise, they have
// default -1 values.  So checking for median horiz crossing > 0 is
// tantamount to determining which extremal regions have changed since
// the previous level.  Ignore any extremal region which has NOT
// changed:

            if (median_horiz_crossings < 0) continue;

// Reject candidate extremal regions whose pixel height or width are
// less than reasonable threshold values:
         
            if (extremal_region_ptr->region_too_small()) continue;

            extremal_region_ptr->compute_shape_text_prob(&shapes_pfunct);

/*
            double prob=extremal_region_ptr->get_object_prob();
            if (prob > 0)
            {
               cout << "node: ID = " << ID
                    << " area = " << pixel_area
                    << " perim = " << pixel_perimeter
                    << " euler = " << euler_number
                    << " horiz_crossings = " << median_horiz_crossings << endl;
               cout << "  left_pu = " << left_pu
                    << " right_pu = " << right_pu
                    << " bottom_pv = " << bottom_pv
                    << " top_pv = " << top_pv << endl;
            
//            cout << "  n = " << n << " node_ID = " << ID << endl;
//            cout << "  aspect_ratio = " << aspect_ratio << endl;
//            cout << "  compactness = " << compactness << endl;
//            cout << "  n_holes = " << n_holes << endl;
//            cout << "  median_horiz_crossings = " << median_horiz_crossings
//                 << endl;

               cout << "  px_sum = " << extremal_region_ptr->get_px_sum()
                    << " py_sum = " << extremal_region_ptr->get_py_sum() 
                    << " z_sum = " << extremal_region_ptr->get_z_sum()
                    << endl;
               cout << "  sqr_px_sum = " 
                    << extremal_region_ptr->get_sqr_px_sum()
                    << " sqr_py_sum = " 
                    << extremal_region_ptr->get_sqr_py_sum() 
                    << " px_py_sum = " << extremal_region_ptr->get_px_py_sum() 
                    << " sqr_z_sum = " << extremal_region_ptr->get_sqr_z_sum()
                    << endl;
               cout << "  cube_px_sum = " 
                    << extremal_region_ptr->get_cube_px_sum()
                    << " sqr_px_py_sum = " 
                    << extremal_region_ptr->get_sqr_px_py_sum() 
                    << " sqr_py_px_sum = " 
                    << extremal_region_ptr->get_sqr_py_px_sum() 
                    << " cube_py_sum = " 
                    << extremal_region_ptr->get_cube_py_sum() 
                    << " cube_z_sum = " 
                    << extremal_region_ptr->get_cube_z_sum()
                    << endl;

               cout << "  prob = " << extremal_region_ptr->get_object_prob()
                    << endl;
            } // prob > 0 conditional
*/
         
// As of Fri July 13, 2012, we no longer believe trying to using
// sigma_z as a text char feature helps much at all.  On the other
// hand, skew_z and dimensionless_quartic_z are features which yield
// modest rejectin of non-text extremal regions:

/*
//            double sigma_z=extremal_region_ptr->get_sigma_z();
//            cout << "sigma_z = " << sigma_z << endl;
            if (extremal_region_ptr->get_sigma_z() > max_sigma_z_threshold)
            {
               extremal_region_ptr->set_object_prob(-1);
               n_sigmaz_rejections++;
            }
*/

            double skew_z=extremal_region_ptr->get_skew_z();
//            cout << "skew_z = " << skew_z << endl;
            if (fabs(extremal_region_ptr->get_skew_z()) > max_skew_z_threshold)
            {
               extremal_region_ptr->set_object_prob(-1);
               n_skewz_rejections++;
            }

            double dimensionless_quartic_z=extremal_region_ptr->
               get_dimensionless_quartic_z();
//            cout << "dimensionless quartic z = " 
//                 << dimensionless_quartic_z << endl;
            if (dimensionless_quartic_z > 
            max_dimensionless_quartic_z_threshold)
            {
               extremal_region_ptr->set_object_prob(-1);
               n_quartic_z_rejections++;
            }

            if (extremal_region_ptr->get_object_prob() > 0)
            {
               n_candidate_regions++;
            }

         } // loop over index n labeling current extremal region
//         cout << "n_candidate_regions = " << n_candidate_regions << endl;

         std::vector<extremal_region*> extremal_regions_vector=
            connected_components_ptr->get_stable_regions();

// Save bbox corners from each candidate extremal region into
// *bboxes_map_ptr:

         for (int n=0; n<extremal_regions_vector.size(); n++)
         {
            extremal_region* extremal_region_ptr=extremal_regions_vector[n];

            int left_pu,bottom_pv,right_pu,top_pv;
            extremal_region_ptr->get_bbox(left_pu,bottom_pv,right_pu,top_pv);
            right_pu++;
            top_pv++;
         
            fourvector bbox_corners(left_pu,bottom_pv,right_pu,top_pv);
            BBOXES_MAP::iterator iter=bboxes_map_ptr->find(bbox_corners);
            if (iter==bboxes_map_ptr->end())
            {
               (*bboxes_map_ptr)[bbox_corners]=1;

//               cout << "level = " << level
//                    << " char prob = " << basic_math::round(100*char_prob)
//                    << " bottom left: pu = " << left_pu
//                    << " pv = " << bottom_pv
//                    << " top right: pu = " << right_pu
//                    << " pv = " << top_pv << endl;
            }
         } // loop over index n labeling stable region nodes 
   
//         cout << "Initially, bboxes_map_ptr->size() = " 
//              << bboxes_map_ptr->size() << endl;

// Merge bounding boxes whose sizes are nearly equal:
   
//   const int bbox_distance_toler=1;	// pixels
         const int bbox_distance_toler=2;	// pixels
//      const int bbox_distance_toler=3;	// pixels
//      const int bbox_distance_toler=4;	// pixels

         bool changed_flag=false;
         do 
         {
            changed_flag=false;

            for (BBOXES_MAP::iterator iter=bboxes_map_ptr->begin();
                 iter != bboxes_map_ptr->end(); iter++)
            {
               fourvector curr_bbox_corners=iter->first;
               if (iter->second < 0) continue;

               double left_pu=curr_bbox_corners.get(0);
               double bottom_pv=curr_bbox_corners.get(1);
               double right_pu=curr_bbox_corners.get(2);
               double top_pv=curr_bbox_corners.get(3);
      
               for (BBOXES_MAP::iterator iter2=bboxes_map_ptr->begin();
                    iter2 != bboxes_map_ptr->end(); iter2++)
               {
                  if (iter2==iter) continue;
                  if (iter2->second < 0) continue;
            
                  fourvector next_bbox_corners=iter2->first;
                  if ( (curr_bbox_corners-next_bbox_corners).magnitude() > 
                  bbox_distance_toler) continue;

                  left_pu=basic_math::min(
                     left_pu,next_bbox_corners.get(0));
                  bottom_pv=basic_math::min(
                     bottom_pv,next_bbox_corners.get(1));
                  right_pu=basic_math::max(
                     right_pu,next_bbox_corners.get(2));
                  top_pv=basic_math::max(
                     top_pv,next_bbox_corners.get(3));
                  fourvector new_bbox_corners
                     (left_pu,bottom_pv,right_pu,top_pv);
                  (*bboxes_map_ptr)[new_bbox_corners]=iter->second;
                  iter2->second=-1;
                  changed_flag=true;
               } // loop over iter2
            } // loop over iter
         }
         while (changed_flag);

//         cout << "After bbox merging, bboxes_map_ptr->size() = " 
//              << bboxes_map_ptr->size() << endl;

// Draw rescaled candidate text character bounding boxes:

         texture_rectangle_ptr->import_photo_from_file(image_filename);

         int bbox_counter=0;
         for (BBOXES_MAP::iterator iter=bboxes_map_ptr->begin();
              iter != bboxes_map_ptr->end(); iter++)
         {
            if (iter->second < 0) continue;

            int left_pu=iter->first.get(0);
            int right_pu=iter->first.get(2)-1;
            int bottom_pv=iter->first.get(3)-1;
            int top_pv=iter->first.get(1);

//            cout << "left_pu = " << left_pu << " right_pu = " << right_pu
//                 << " bottom_pv = " << bottom_pv << " top_pv = " << top_pv
//                 << endl;
            
// Copy current bounding box chip into *qtwoDarray_ptr.  Then rescale
// chip's size so that new version stored in *qnew_twoDarray_ptr has
// height or width precisely equal to 32 pixels in size:

            int width=right_pu-left_pu+1;
            int height=bottom_pv-top_pv+1;
            double aspect_ratio=double(width)/double(height);

            int new_width,new_height;
            if (aspect_ratio > 1)
            {
               new_width=32;
               new_height=new_width/aspect_ratio;
            }
            else
            {
               new_height=32;
               new_width=aspect_ratio*new_height;
            }

            texture_rectangle_ptr->import_photo_from_file(image_filename);
            texture_rectangle_ptr->refresh_ptwoDarray_ptr();
            twoDarray* qtwoDarray_ptr=texture_rectangle_ptr->
               export_sub_twoDarray(left_pu,right_pu,top_pv,bottom_pv);
            qtwoDarray_ptr->init_coord_system(0,1,0,1);

            twoDarray* qnew_twoDarray_ptr=compositefunc::downsample(
               new_width,new_height,qtwoDarray_ptr);
            delete qtwoDarray_ptr;

            texture_rectangle_ptr->instantiate_ptwoDarray_ptr();
            *(texture_rectangle_ptr->get_ptwoDarray_ptr()) = 
               *qnew_twoDarray_ptr;

            text_detector_ptr->destroy_window_features_maps();
            text_detector_ptr->set_window_width(new_width);
            text_detector_ptr->set_window_height(new_height);
            text_detector_ptr->initialize_window_features_maps();

/*
  double Ng_char_prob=1;
  bool flag=text_detector_ptr->average_window_features(0,0);
  if (flag)
  {
  float* window_histogram=text_detector_ptr->
  get_nineK_window_descriptor();
  for (int k=0; k<nineK; k++)
  {
  Ng_sample(k)=window_histogram[k];
//               cout << "k = " << k << " window_histogram[k] = "
//                    << window_histogram[k] << endl;
} // loop over index k labeling dictionary descriptors
Ng_char_prob=Ng_pfunct(Ng_sample);
cout << "Ng char probability = " << Ng_char_prob << endl;
//            outputfunc::enter_continue_char();
}
*/

            delete qnew_twoDarray_ptr;

/*
  if (Ng_char_prob < Ng_char_threshold)
  {
  n_Ng_rejections++;
  continue;
  }
*/

            double left_u,top_v,right_u,bottom_v;
            texture_rectangle_ptr->get_uv_coords(
               left_pu,top_pv,left_u,top_v);
            texture_rectangle_ptr->get_uv_coords(
               right_pu,bottom_pv,right_u,bottom_v);

            std::vector<threevector> vertices;
            vertices.push_back(threevector(left_u,top_v));
            vertices.push_back(threevector(left_u,bottom_v));
            vertices.push_back(threevector(right_u,bottom_v));
            vertices.push_back(threevector(right_u,top_v));
            polygon bbox(vertices);
            std::vector<polygon> polygons;
            polygons.push_back(bbox);
            all_polygons.push_back(bbox);

/*
  string output_filename=bboxes_subdir+"bbox_bright_chars";
  if (brightness_iter==1) output_filename=bboxes_subdir+
  "bbox_dark_chars";
  output_filename += stringfunc::integer_to_string(bbox_counter++,3)+
  ".jpg";
         
  int contour_color_index=1;
  int line_thickness=2;
  videofunc::display_polygons(
  polygons,texture_rectangle_ptr,output_filename,
  contour_color_index,line_thickness);
*/

            n_detections++;

         } // loop over *bboxes_map_ptr

//         outputfunc::enter_continue_char();

         min_cc_label += n_connected_components;

      } // loop over threshold index
// ----------------------------------------------------------------------

   } // loop over brightness_iter
// =======================================================================

   delete connected_components_ptr;   
   delete bboxes_map_ptr;
   delete text_detector_ptr;

   cout << "n_detections = " << n_detections << endl;
   cout << "n_rejections = " << n_rejections << endl;
   cout << "n_sigmaz_rejections = " << n_sigmaz_rejections << endl;
   cout << "n_skewz_rejections = " << n_skewz_rejections << endl;
   cout << "n_quartic_z_rejections = " << n_quartic_z_rejections << endl;
   cout << "n_Ng_rejections = " << n_Ng_rejections << endl;
   double rejection_frac=
      double(n_rejections)/double(n_rejections+n_detections);
   cout << "R/(R+D) = " << rejection_frac << endl;

// Display all text character bounding boxes within a single,
// composite image:

   texture_rectangle* composite_texture_rectangle_ptr=new texture_rectangle();
   composite_texture_rectangle_ptr->import_photo_from_file(image_filename);

   string composite_output_filename=bboxes_subdir+"composite_char_bboxes.jpg";
   int contour_color_index=-1;
   int thickness=0;
   videofunc::display_polygons(
      all_polygons,composite_texture_rectangle_ptr,composite_output_filename,
      contour_color_index,thickness);

   delete composite_texture_rectangle_ptr;
   connectfunc::delete_extremal_region_pooled_memory();

   double total_time=timefunc::elapsed_timeofday_time();
   cout << "TOTAL PROCESSING TIME = " << total_time << " secs = " 
        << total_time / 60.0 << " minutes" << endl;
}

