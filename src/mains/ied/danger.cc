// ==========================================================================
// Program DANGER_MAP
// ==========================================================================
// Last updated on 5/22/05; 4/24/06; 8/5/06; 12/4/10
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include "image/connectfuncs.h"
#include "image/displayfuncs.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "ladar/featurefuncs.h"
#include "general/filefuncs.h"
#include "ladar/groundfuncs.h"
#include "datastructures/Hashtable.h"
#include "image/imagefuncs.h"
#include "ladar/ladarfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/parallelogram.h"
#include "image/recursivefuncs.h"
#include "general/sysfuncs.h"
#include "urban/tree_cluster.h"
#include "image/TwoDarray.h"
#include "urban/urbanfuncs.h"
#include "urban/urbanimage.h"
#include "geometry/voronoifuncs.h"
#include "threeDgraphics/xyzpfuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::ios;
   using std::istream;
   using std::ofstream;
   using std::ostream;
   using std::string;
   std::set_new_handler(sysfunc::out_of_memory);
  
   bool input_param_file;
   unsigned int ninputlines,currlinenumber;
   string inputline[200];

   filefunc::parameter_input(
      argc,argv,input_param_file,inputline,ninputlines);
   currlinenumber=0;

// Read contents of binary xyzp file into 1D x, y, z and p arrays:

   cout << "Enter refined feature image:" << endl;
   urbanimage cityimage;
//   cityimage.set_public_software(true);
   cityimage.initialize_image(input_param_file,inputline,currlinenumber);
   cityimage.parse_and_store_input_data(true,false,false);

// Eliminate junk nearby edges of data bounding box:

   cityimage.compute_data_bbox(cityimage.get_z2Darray_ptr(),false);
   ladarfunc::crop_data_inside_bbox(0.01,cityimage.get_data_bbox_ptr(),
                                    cityimage.get_z2Darray_ptr());
   ladarfunc::crop_data_inside_bbox(0.01,cityimage.get_data_bbox_ptr(),
                                    cityimage.get_p2Darray_ptr());

   twoDarray const *ztwoDarray_ptr=cityimage.get_z2Darray_ptr();
   twoDarray const *features_twoDarray_ptr=cityimage.get_p2Darray_ptr();

   string features_filename=cityimage.get_imagedir()+"features.xyzp";   
//   xyzpfunc::write_xyzp_data(
//      ztwoDarray_ptr,features_twoDarray_ptr,features_filename);
//   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
//      features_twoDarray_ptr,features_filename);

   twoDarray const *features_and_heights_twoDarray_ptr=
      urbanfunc::color_feature_heights(
      ztwoDarray_ptr,features_twoDarray_ptr);

   

   string features_and_heights_filename=
      cityimage.get_imagedir()+"features_and_heights.xyzp";   
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,features_and_heights_twoDarray_ptr,
      features_and_heights_filename);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      features_and_heights_twoDarray_ptr,features_and_heights_filename);
   filefunc::gunzip_file(features_and_heights_filename);

   threevector vertex[20];
   vertex[0]=threevector(15,260);
   vertex[1]=threevector(151,268);
   vertex[2]=threevector(144,338);
   vertex[3]=threevector(329,350);
   vertex[4]=threevector(308,439);
   vertex[5]=threevector(339,277);
   vertex[6]=threevector(465,266);
   vertex[7]=threevector(349,205);

   vertex[8]=threevector(356,162);
   vertex[9]=threevector(419,146);
   vertex[10]=threevector(167,55);
   vertex[11]=threevector(151,194);
   vertex[12]=threevector(297,96);
   vertex[13]=threevector(65,191);

   vertex[14]=threevector(129,124);
   vertex[15]=threevector(157,121);
   vertex[16]=threevector(326,132);

   linesegment l0(vertex[0],vertex[1]);
   linesegment l1(vertex[2],vertex[1]);
   linesegment l2(vertex[2],vertex[3]);
   linesegment l3(vertex[4],vertex[3]);
   linesegment l4(vertex[5],vertex[3]);
   linesegment l5(vertex[5],vertex[1]);

   linesegment l6(vertex[5],vertex[7]);
   linesegment l7(vertex[5],vertex[6]);
   linesegment l8(vertex[9],vertex[6]);
   linesegment l9(vertex[7],vertex[8]);
   linesegment l10(vertex[8],vertex[16]);   
   linesegment l11(vertex[12],vertex[10]);
   linesegment l12(vertex[10],vertex[15]);
   linesegment l13(vertex[7],vertex[11]);
   linesegment l14(vertex[11],vertex[13]);

   linesegment l15(vertex[14],vertex[15]);
   linesegment l16(vertex[15],vertex[16]);
   linesegment l17(vertex[15],vertex[11]);
   linesegment l18(vertex[12],vertex[16]);


   linesegment l[30];
   l[0]=l0;
   l[1]=l1;
   l[2]=l2;
   l[3]=l3;
   l[4]=l4;
   l[5]=l5;
   l[6]=l6;
   l[7]=l7;
   l[8]=l8;
   l[9]=l9;
   l[10]=l10;
   l[11]=l11;
   l[12]=l12;
   l[13]=l13;
   l[14]=l14;
   l[15]=l15;
   l[16]=l16;
   l[17]=l17;
   l[18]=l18;

   int nlines=19;
   for (int n=0; n<nlines; n++)
   {
      draw3Dfunc::draw_line(l[n],features_and_heights_filename,
                            draw3Dfunc::annotation_value1);
   }
   
// Generate danger probability map:

   twoDarray* danger_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
   danger_twoDarray_ptr->initialize_values(1.0);

   for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
   {
      for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
      {
         double curr_f=features_twoDarray_ptr->get(px,py);
         if (nearly_equal(curr_f,featurefunc::tree_sentinel_value) ||
             nearly_equal(curr_f,featurefunc::building_sentinel_value))
         {
            danger_twoDarray_ptr->put(px,py,0);
         }
         else
         {
            double min_dist=POSITIVEINFINITY;
            threevector currpoint;
            ztwoDarray_ptr->pixel_to_point(px,py,currpoint);
            for (int n=0; n<nlines; n++)
            {

               double curr_dist=l[n].point_to_line_segment_distance(
                  currpoint);
               min_dist=basic_math::min(min_dist,curr_dist);
            } // loop over index l labeling road line segments
            
            double p_danger=0;
            double lethal_range[3];
            lethal_range[0]=8;
            lethal_range[1]=13;
            lethal_range[2]=16;
            if (min_dist < lethal_range[0])
            {
               p_danger=1.0;
            }
            else if (min_dist < lethal_range[1])
            {
               p_danger=0.7;
            }
            else if (min_dist < lethal_range[2])
            {
               p_danger=0.35;
            }
            danger_twoDarray_ptr->put(px,py,p_danger);
         }
         
      } // loop over py index
   } // loop over px index


/*
// Ground surface IED danger map:

   for (int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
   {
      for (int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
      {
         double curr_f=features_twoDarray_ptr->get(px,py);
         if (nearly_equal(curr_f,featurefunc::tree_sentinel_value))
         {
            danger_twoDarray_ptr->put(px,py,0.6);
         }
         else if (nearly_equal(curr_f,featurefunc::building_sentinel_value))
         {
            danger_twoDarray_ptr->put(px,py,0);
         }
      }
   }
*/


/*
//   threevector center_point(ztwoDarray_ptr->center_point());
//   double x0=center_point.e[0];
//   double y0=center_point.e[1];

// "Final" IED danger map:
   double x0=344;
   double y0=164;
   double sigma0=10;
   displayfunc::twoD_gaussian_peak(x0,y0,sigma0,danger_twoDarray_ptr);

   double x1=154;
   double y1=277;
   double sigma1=5;
   displayfunc::twoD_gaussian_peak(x1,y1,sigma1,danger_twoDarray_ptr);

   double x2=309;
   double y2=408;
   double sigma2=8;
   displayfunc::twoD_gaussian_peak(x2,y2,sigma2,danger_twoDarray_ptr);
*/

// Fuze together height and danger probability maps:

   const double p_hi=0.0;
   const double p_lo=1.0;
   twoDarray* fused_twoDarray_ptr=
      urbanfunc::fuse_height_feature_and_prob_images(
      p_hi,p_lo,ztwoDarray_ptr,features_and_heights_twoDarray_ptr,
      danger_twoDarray_ptr);
   string fuse_filenamestr=cityimage.get_imagedir()+"danger.xyzp";
   xyzpfunc::write_xyzp_data(
      ztwoDarray_ptr,fused_twoDarray_ptr,fuse_filenamestr);
   draw3Dfunc::append_fake_xyzp_points_in_twoDarray_middle(
      fused_twoDarray_ptr,fuse_filenamestr);
   delete fused_twoDarray_ptr;

   delete danger_twoDarray_ptr;
   delete features_and_heights_twoDarray_ptr;
 
   cityimage.summarize_results();
   cityimage.update_logfile("danger");
}


