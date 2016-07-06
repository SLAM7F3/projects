// ========================================================================
// Program EOVSIR works with a set of FLIR video frames which were
// either gathered in the EO or IR frequency bands.  Looping over such
// a set of JPEG video frames, this program tests the RGB content
// within a small lattice of pixels.  If the absolute discrepancy
// between RG, GB or BR values exceeds some threshold, the image is
// declared to have been collected by the FLIR's EO camera.  The EO
// classified images are added as entries to the image_attributes
// table in the IMAGERY database.
// ========================================================================
// Last updated on 4/14/12; 5/4/12; 5/17/12
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "video/imagesdatabasefuncs.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();
//   cout << "GISlayer_IDs.size() = " << GISlayer_IDs.size() << endl;

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
//   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;

   int hierarchy_ID=2;	// Grand Canyon
//   int hierarchy_ID=5;	// Tstorm
   cout << "Enter graph hierarchy ID which contains FLIR video frames:"
        << endl;
   cin >> hierarchy_ID;

   int graph_ID=0;

   vector<int> campaign_IDs,mission_IDs,image_IDs,datum_IDs;
   vector<string> URLs;
   
   imagesdatabasefunc::get_image_URLs(
      postgis_db_ptr,hierarchy_ID,graph_ID,
      campaign_IDs,mission_IDs,image_IDs,datum_IDs,URLs);

/*
// FAKE FAKE:  Sat Apr 14, 2012 at 2:30 pm

   URLs.clear();
   URLs.push_back(
      "/data/ImageEngine/GrandCanyon/images/GCFLIR00242.jpg");
//      "/data/ImageEngine/GrandCanyon/images/GCFLIR00243.jpg");
//   URLs.push_back(
//      "/data/ImageEngine/GrandCanyon/images/GC_FLIR_clip5_-01537.jpg");
//      "/data/ImageEngine/GrandCanyon/images/GC_FLIR_clip5_-01311.jpg");
//   URLs.push_back(
//      "/data/ImageEngine/GrandCanyon/images/GC_FLIR_clip5_-01324.jpg");
//   URLs.push_back(
//      "/data/ImageEngine/GrandCanyon/images/GC_FLIR_clip5_-01324.jpg");
//   URLs.push_back(
//      "/data/ImageEngine/GrandCanyon/images/GC_FLIR_clip5_-01537.jpg");
//   URLs.push_back(
//      "/data/ImageEngine/GrandCanyon/images/GC_FLIR_clip5_-01938.jpg");
//      "/data/ImageEngine/GrandCanyon/images/GC_FLIR_clip5_-01311.jpg");
//   URLs.push_back("/data/ImageEngine/GrandCanyon/images/GCFLIR00029.jpg");
//   URLs.push_back("/data/ImageEngine/GrandCanyon/images/GCFLIR00051.jpg");
//   URLs.push_back("/data/ImageEngine/GrandCanyon/images/GC_FLIR_clip5_-01312.jpg");
*/

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
      URLs[0],NULL);

//   int npu=texture_rectangle_ptr->getWidth();
//   int npv=texture_rectangle_ptr->getHeight();
//   double umin=0;
//   double umax=double(npu)/double(npv);
//   double vmin=0;
//   double vmax=1;

//   int n_skip=101;
//   int dpu=npu/double(n_skip);
//   int dpv=npv/double(n_skip);
//   int pu_start=0+dpu;
//   int pv_start=0+dpv;
//   int pu_stop=npu-dpu;
//   int pv_stop=npv-dpv;

   int n_EO_images=0;
   int n_IR_images=0;
//   int R,G,B;
   vector<double> intensities;

//   const int max_greyscale_delta=7;
   for (unsigned int i=0; i<URLs.size(); i++)
   {
      if (i%100==0) cout << i << " " << flush;

//      cout << "i = " << i << " URLs[i] = "
//           << URLs[i] << endl;

// For Grand Canyon FLIR images, "FLIR" exists within input filenames:

      string jpg_filename=filefunc::getbasename(URLs[i]);
      int FLIR_posn=stringfunc::first_substring_location(jpg_filename,"FLIR");
//      cout << "jpg_filename = " << jpg_filename
//           << " FLIR_posn = " << FLIR_posn << endl;
      if (FLIR_posn < 0) continue;
      
// First check whether entry corresponding to URL exists within nodes
// table of IMAGERY database:

      int campaign_ID,mission_ID,image_ID,datum_ID;
      imagesdatabasefunc::get_image_metadata_given_URL(
         postgis_db_ptr,URLs[i],
         campaign_ID,mission_ID,image_ID,datum_ID);

//      cout << "campaign_ID = " << campaign_ID
//           << " mission_ID = " << mission_ID
//           << " image_ID = " << image_ID
//           << " datum_ID = " << datum_ID << endl;

// Ignore any video frame which does not exist within nodes table of
// IMAGERY database.  Recall not every Tstorm image is loaded into the
// nodes table...

      if (image_ID < 0 || datum_ID < 0)
      {
         cout << "Error!  image_ID = " << image_ID << " datum_ID = "
              << datum_ID << endl;
         exit(-1);
      }
      
      texture_rectangle_ptr->reset_texture_content(URLs[i]);

      bool generate_quantized_image_flag=false;
      string output_filename="junk.jpg";
      
      vector<double> color_histogram=videofunc::generate_color_histogram(
         generate_quantized_image_flag,texture_rectangle_ptr,
         output_filename);

      vector<string> color_label;
      color_label.push_back("red");
      color_label.push_back("orange");
      color_label.push_back("yellow");
      color_label.push_back("green");
      color_label.push_back("blue");
      color_label.push_back("purple");
      color_label.push_back("black");
      color_label.push_back("white");
      color_label.push_back("grey");
      color_label.push_back("brown");

      double bw_frac_sum=0;
      bw_frac_sum += color_histogram[6]+color_histogram[7]+color_histogram[8];


      bool RGB_colored_flag=true;
      if (bw_frac_sum > 0.97)
      {
         RGB_colored_flag=false;
      }
      
      
/*
      bool RGB_colored_flag=true;
      int colored_pixel_counter=0;
      int n_sampled_pixels=0;
      int n_grey_sampled_pixels=0;
      for (int pu=pu_start; pu<pu_stop; pu += dpu)
      {
         for (int pv=pv_start; pv<pv_stop; pv += dpv)
         {
            n_sampled_pixels++;
            texture_rectangle_ptr->get_pixel_RGB_values(pu,pv,R,G,B);
                     

// On 2/25/12, we empirically found that "greyscale" IR FLIR images do
// NOT necessarily have R=G=B.  But |R-G|, |G-B| and |B-R| are limited
// for IR video frames:

            int d_RG=fabs(R-G);
            int d_GB=fabs(G-B);
            int d_BR=fabs(B-R);
            
            if (d_RG > max_greyscale_delta ||
            d_GB > max_greyscale_delta ||
            d_BR > max_greyscale_delta)
            {
//               cout << "R = " << R 
//                    << " G = " << G
//                    << " B = " << B << endl;
//               cout << "d_RG = " << d_RG
//                    << " d_GB = " << d_GB
//                    << " d_BR = " << d_BR << endl;
               colored_pixel_counter++;
            }
            else
            {
               double r=R/255.0;
               double g=G/255.0;
               double b=B/255.0;
            
               r=basic_math::max(0.0,r);
               g=basic_math::max(0.0,g);
               b=basic_math::max(0.0,b);
               double h,s,v;
               colorfunc::RGB_to_hsv(r,g,b,h,s,v);
               intensities.push_back(v);
               int quantized_color_bin=
                  colorfunc::assign_hsv_to_color_histogram_bin(h,s,v);
               if (quantized_color_bin==8)
               {
                  n_grey_sampled_pixels++;
               }
            }
         } // loop over pv index
      } // loop over pu index
//      cout << "RGB_colored_flag = " << RGB_colored_flag << endl;

//      cout << "n_sampled_pixels = " << n_sampled_pixels << endl;
      double colored_pixel_fraction=double(colored_pixel_counter)/
         n_sampled_pixels;

      double grey_pixel_fraction=
         double(n_grey_sampled_pixels)/n_sampled_pixels;
//      cout << "colored_pixel_frac = " << colored_pixel_fraction 
//           << " grey_pixel_frac = " << grey_pixel_fraction << endl;

// On 4/14/12, we empirically observed that median intentsities for
// Grand Canyon IR flir frames is noticeably larger than for GC EO
// frames which are nearly grey:

      double median_intensity=mathfunc::median_value(intensities);
//      cout << "median_intensity = " << median_intensity << endl;

      RGB_colored_flag=true;
      if (grey_pixel_fraction > 0.9 || colored_pixel_fraction < 0.04)
      {
         if (median_intensity > 0.3)
         {
            RGB_colored_flag=false;
         }
      }
*/

      cout << i
           << " filename=" << filefunc::getbasename(URLs[i])
           << " RGB_colored_flag=" << RGB_colored_flag 
           << " bw_sum = " << bw_frac_sum 
           << endl;

// As of 2/26/12, default EO_vs_IR_imagery attribute value = EO.  So
// ignore any images for which RGB_color_flag=true...
      
      string attribute_key="Frequency_band";
      string attribute_value;
      if (!RGB_colored_flag) 
      {
         attribute_value="IR";
         n_IR_images++;
      }
      else
      {
         attribute_value="EO";
         n_EO_images++;
      }
      
//      cout << "hierarchy_ID = " << hierarchy_ID
//           << " campaign_ID = " << campaign_ID
//           << " mission_ID = " << mission_ID
//           << " image_ID = " << image_ID 
//           << " datum_ID = " << datum_ID 
//           << endl;
//      cout << i << " of " << URLs.size() 
//           << "  URL = " << filefunc::getbasename(URLs[i]) << endl;
      
//      imagesdatabasefunc::insert_image_attribute(
//         postgis_db_ptr,campaign_ID,mission_ID,image_ID,datum_ID,
//         attribute_key,attribute_value);

      imagesdatabasefunc::update_image_attribute(
         postgis_db_ptr,campaign_ID,mission_ID,image_ID,datum_ID,
         attribute_key,attribute_value);

//      if (!RGB_colored_flag) outputfunc::enter_continue_char();

   } // loop over index i labeling input jpeg files

   delete texture_rectangle_ptr;

   cout << endl;
   cout << "Total number of FLIR images analyzed = "<< URLs.size() << endl;
   cout << "Number of images classified as EO = " << n_EO_images << endl;
   cout << "Number of images classified as IR = " << n_IR_images << endl;
   double EO_frac=double(n_EO_images)/(n_EO_images+n_IR_images);
   double IR_frac=double(n_IR_images)/(n_EO_images+n_IR_images);
   cout << "EO fraction = " << EO_frac << endl;
   cout << "IR fraction = " << IR_frac << endl;
   
}
