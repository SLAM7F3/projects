// ==========================================================================
// Program COLOR_DISTS imports a set of manually selected bounding
// boxes for some input image.  For all pixels within each bounding
// box, it computes the R, G and B color distributions.  Density
// and/or cumulative color distributions are exported.  COLOR_DISTS
// also generates annotated versions of the input images containing
// bboxes with their IDs.

//				./color_dists

// ==========================================================================
// Last updated on 2/19/14; 2/20/14; 4/14/14
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>
#include "geometry/bounding_box.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "math/prob_distribution.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);

   vector<string> image_basenames;

   image_basenames.push_back("Boston/resized_images/shore1.jpg");
   image_basenames.push_back("Boston/resized_images/shore3.jpg");
   image_basenames.push_back("Boston/resized_images/marina_1.jpg");
   image_basenames.push_back("Boston/resized_images/marina_2.jpg");
   image_basenames.push_back("Boston/resized_images/marina_3.jpg");
   image_basenames.push_back("Boston/resized_images/marina_4.jpg");
   image_basenames.push_back("Boston/resized_images/marina_5.jpg");

   image_basenames.push_back(
      "000175/resized_images/000175-071112163915-Cam1.JPG");
   image_basenames.push_back(
      "000175/resized_images/000175-071112163915-Cam2.JPG");
   image_basenames.push_back(
      "000175/resized_images/000175-071112163915-Cam4.JPG");
   image_basenames.push_back(
      "000175/resized_images/000175-071112163915-Cam5.JPG");

   image_basenames.push_back(
      "HalfMoonBay/resized_images/009014-080712000452-Cam6.JPG");
   image_basenames.push_back(
      "HalfMoonBay/resized_images/009853-080712002012-Cam8.JPG");
   image_basenames.push_back(
      "HalfMoonBay/resized_images/009816-080712002043-Cam6.JPG");
   image_basenames.push_back(
      "HalfMoonBay/resized_images/009857-080712002009-Cam7.JPG");

   image_basenames.push_back(
      "Toronto/resized_images/036051-070512141231-Cam1.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/036051-070512141231-Cam3.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/036051-070512141231-Cam5.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/036051-070512141231-Cam7.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/036051-070512141231-Cam9.JPG");

   image_basenames.push_back(
      "Toronto/resized_images/036838-070512135926-Cam9.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/042750-072912155521-Cam1.JPG");
   
   image_basenames.push_back(
      "Toronto/resized_images/042432-072912154633-Cam1.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/042432-072912154633-Cam2.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/042432-072912154633-Cam3.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/042432-072912154633-Cam4.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/042432-072912154633-Cam5.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/042432-072912154633-Cam6.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/042432-072912154633-Cam7.JPG");
   image_basenames.push_back(
      "Toronto/resized_images/042432-072912154633-Cam8.JPG");

   for (unsigned int i=0; i<image_basenames.size(); i++)
   {
      cout << i << " --> " << filefunc::getbasename(image_basenames[i])
           << endl;
   }
   cout << endl;

   cout << "Enter input image index:" << endl;
   int image_index;
   cin >> image_index;

   string images_subdir="./images/";
   string image_filename=images_subdir+image_basenames[image_index];
   cout << "image_filename = " << image_filename << endl;

   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle* annotated_texture_rectangle_ptr=new texture_rectangle();
   texture_rectangle_ptr->import_photo_from_file(image_filename);
   annotated_texture_rectangle_ptr->import_photo_from_file(image_filename);

   int width=texture_rectangle_ptr->getWidth();
   int height=texture_rectangle_ptr->getHeight();
   cout << "width = " << width
        << " height = " << height << endl;
   cout << "width*height = " << width*height << endl;

// Import manually selected opposing corners defining 2D bounding
// boxes:

   string bboxes_subdir=filefunc::getdirname(image_filename);
   string prefix=filefunc::getprefix(image_filename);
   string bboxes_filename=bboxes_subdir+"bboxes_2D_"+prefix+".txt";
   cout << "bboxes_filename = " << bboxes_filename << endl;
   if (!filefunc::fileexist(bboxes_filename))
   {
      cout << "No file containing bboxes found in bboxes_filename = "
           << bboxes_filename << endl;
      exit(-1);
   }

   vector<vector<double> > row_values=filefunc::ReadInRowNumbers(
      bboxes_filename);


   vector<string> text_lines;
   vector<twovector> xy_start;
   vector<colorfunc::Color> text_colors;
   vector<bounding_box> bboxes;
   for (unsigned int r=0; r<row_values.size(); r += 2)
   {
      double curr_u=row_values[r].at(3);
      double curr_v=row_values[r].at(4);
      double next_u=row_values[r+1].at(3);
      double next_v=row_values[r+1].at(4);

      double Ulo=basic_math::min(curr_u,next_u);
      double Uhi=basic_math::max(curr_u,next_u);
      double Vlo=basic_math::min(curr_v,next_v);
      double Vhi=basic_math::max(curr_v,next_v);
      bboxes.push_back(bounding_box(Ulo,Uhi,Vlo,Vhi));

      int bbox_ID=r/2;
      text_lines.push_back(stringfunc::number_to_string(bbox_ID));
      text_colors.push_back(colorfunc::yellow);

      unsigned int pu_mean,pv_mean;
      texture_rectangle_ptr->get_pixel_coords(
         0.495*(Ulo+Uhi),1-0.495*(Vlo+Vhi),pu_mean,pv_mean);
      xy_start.push_back(twovector(pu_mean,pv_mean));
   }

// Compute different color channel probability distributions for
// pixel regions contained inside bounding boxes:

   vector<double> S_25,V_75;

   vector<double> n_bbox_pixels;
   for (unsigned int b=0; b<bboxes.size(); b++)
   {
      bounding_box curr_bbox(bboxes[b]);
      double Ulo=curr_bbox.get_xmin();
      double Uhi=curr_bbox.get_xmax();
      double Vlo=curr_bbox.get_ymin();
      double Vhi=curr_bbox.get_ymax();

      unsigned int pu_lo,pu_hi,pv_lo,pv_hi;
      texture_rectangle_ptr->get_pixel_coords(Ulo,Vlo,pu_lo,pv_hi);
      texture_rectangle_ptr->get_pixel_coords(Uhi,Vhi,pu_hi,pv_lo);

      n_bbox_pixels.push_back((pu_hi-pu_lo)*(pv_hi-pv_lo));
      cout << "b = " << b << " pu_lo = " << pu_lo << " pu_hi = " << pu_hi
           << " pv_lo = " << pv_lo << " pv_hi = " << pv_hi << endl;
      cout << "n_bbox_pixels = " << n_bbox_pixels.back() << endl;

      vector<double> reds,greens,blues,hues,saturations,values;
      for (unsigned int pv=pv_lo; pv <= pv_hi; pv++)
      {
         for (unsigned int pu=pu_lo; pu <= pu_hi; pu++)
         {
            int R,G,B;
            double h,s,v;
            texture_rectangle_ptr->get_pixel_RGBhsv_values(pu,pv,R,G,B,h,s,v);
            reds.push_back(R/255.0);
            greens.push_back(G/255.0);
            blues.push_back(B/255.0);
//            if (h >= 0) hues.push_back(h/360.0);
            if (h >= 0) hues.push_back(h);
            saturations.push_back(s);
            values.push_back(v);

         } // loop over pv
      } // loop over pu

      vector<string> prob_jpg_filenames;

      string density_filename_prefix=bboxes_subdir+"reds_dens_bbox_"+
         stringfunc::number_to_string(b);
      prob_distribution prob_reds(reds,100,0);
      prob_reds.set_xmin(0);
      prob_reds.set_xmax(1);
      prob_reds.set_xtic(0.1);
      prob_reds.set_xsubtic(0.002);
      prob_reds.set_densityfilenamestr(density_filename_prefix+".meta");
      prob_reds.write_density_dist(false,true);
      prob_jpg_filenames.push_back(density_filename_prefix+".jpg");

      density_filename_prefix=bboxes_subdir+"greens_dens_bbox_"+
         stringfunc::number_to_string(b);
      prob_distribution prob_greens(greens,100,0);
      prob_greens.set_xmin(0);
      prob_greens.set_xmax(1);
      prob_greens.set_xtic(0.1);
      prob_greens.set_xsubtic(0.002);
      prob_greens.set_densityfilenamestr(density_filename_prefix+".meta");
      prob_greens.write_density_dist(false,true);
      prob_jpg_filenames.push_back(density_filename_prefix+".jpg");

      density_filename_prefix=bboxes_subdir+"blues_dens_bbox_"+
         stringfunc::number_to_string(b);
      prob_distribution prob_blues(blues,100,0);
      prob_blues.set_xmin(0);
      prob_blues.set_xmax(1);
      prob_blues.set_xtic(0.1);
      prob_blues.set_xsubtic(0.002);
      prob_blues.set_densityfilenamestr(density_filename_prefix+".meta");
      prob_blues.write_density_dist(false,true);
      prob_jpg_filenames.push_back(density_filename_prefix+".jpg");

/*
      density_filename_prefix=bboxes_subdir+"hues_dens_bbox_"+
         stringfunc::number_to_string(b);
      prob_distribution prob_hues(hues,100,0);
      prob_hues.set_xmin(0);
      prob_hues.set_xmax(360);
      prob_blues.set_xtic(60);
      prob_blues.set_xsubtic(20);
      prob_hues.set_densityfilenamestr(density_filename_prefix+".meta");
      prob_hues.write_density_dist(false,true);
      prob_jpg_filenames.push_back(density_filename_prefix+".jpg");

      string density_filename_prefix=bboxes_subdir+"saturations_dens_bbox_"+
         stringfunc::number_to_string(b);
      string cumulative_filename_prefix=bboxes_subdir+"saturations_cum_bbox_"+
         stringfunc::number_to_string(b);
      prob_distribution prob_saturations(saturations,100,0);
      prob_saturations.set_xmin(0);
      prob_saturations.set_xmax(1);
      prob_saturations.set_xtic(0.1);
      prob_saturations.set_xsubtic(0.002);
      prob_saturations.set_densityfilenamestr(
         density_filename_prefix+".meta");
      prob_saturations.set_cumulativefilenamestr(
         cumulative_filename_prefix+".meta");
      prob_saturations.writeprobdists(false,true);
      prob_jpg_filenames.push_back(density_filename_prefix+".jpg");
      prob_jpg_filenames.push_back(cumulative_filename_prefix+".jpg");
      S_25.push_back(prob_saturations.find_x_corresponding_to_pcum(0.25));

      density_filename_prefix=bboxes_subdir+"values_dens_bbox_"+
         stringfunc::number_to_string(b);
      cumulative_filename_prefix=bboxes_subdir+"values_cum_bbox_"+
         stringfunc::number_to_string(b);
      prob_distribution prob_values(values,100,0);
      prob_values.set_xmin(0);
      prob_values.set_xmax(1);
      prob_values.set_xtic(0.1);
      prob_values.set_xsubtic(0.002);
      prob_values.set_densityfilenamestr(
         density_filename_prefix+".meta");
      prob_values.set_cumulativefilenamestr(
         cumulative_filename_prefix+".meta");
      prob_values.writeprobdists(false,true);
      prob_jpg_filenames.push_back(density_filename_prefix+".jpg");
      prob_jpg_filenames.push_back(cumulative_filename_prefix+".jpg");
      V_75.push_back(prob_values.find_x_corresponding_to_pcum(0.75));
*/

      string script_filename=bboxes_subdir+"view_dists_"+
         stringfunc::number_to_string(b);
      ofstream scriptstream;
      filefunc::openfile(script_filename,scriptstream);

/*
      string unix_cmd="montageview "+
         filefunc::getbasename(prob_jpg_filenames[0])+" "+
         filefunc::getbasename(prob_jpg_filenames[2])+" "+
         filefunc::getbasename(prob_jpg_filenames[1])+" "+
         filefunc::getbasename(prob_jpg_filenames[3]);
      scriptstream << unix_cmd << endl;
*/

      for (unsigned int r=0; r<1; r++)
//      for (unsigned int r=0; r<2; r++)
      {
         string unix_cmd="montageview "+
            filefunc::getbasename(prob_jpg_filenames[3*r+0])+" "+
            filefunc::getbasename(prob_jpg_filenames[3*r+1])+" "+
            filefunc::getbasename(prob_jpg_filenames[3*r+2]);
         scriptstream << unix_cmd << endl;
      }

      filefunc::closefile(script_filename,scriptstream);
      filefunc::make_executable(script_filename);

   } // loop over index b labeling bounding boxes

/*
// Export 25/75 percentile results for saturation/values as function
// of bbox ID:

   string output_filename=bboxes_subdir+"SV_results_"+prefix+".dat";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   for (unsigned int b=0; b<S_25.size(); b++)
   {
      outstream << "bbox ID = " << b << " S_25 = " << S_25[b]
                << " V_75 = " << V_75[b] << endl;
   }
   filefunc::closefile(output_filename,outstream);
   string banner="Exported "+output_filename;
   outputfunc::write_banner(banner);
*/

// Annotate image with bounding boxes labeled with IDs:

   int bbox_color_index=colorfunc::purple;
   int line_thickness=1;
   videofunc::display_bboxes(
      bboxes,texture_rectangle_ptr,bbox_color_index,line_thickness);

//   int fontsize=10;   
   int fontsize=15;   
//   int fontsize=20;   
   videofunc::annotate_image_with_text(
      texture_rectangle_ptr,
      annotated_texture_rectangle_ptr,fontsize,
      text_lines,xy_start,text_colors);

   string annotated_image_filename=bboxes_subdir+prefix+"_annotated.jpg";
   annotated_texture_rectangle_ptr->write_curr_frame(annotated_image_filename);

   string banner="Exported "+annotated_image_filename;
   outputfunc::write_banner(banner);

// Compute average number of pixels per bbox:

   double mean_bbox_pixels=mathfunc::mean(n_bbox_pixels);
   cout << "mean(bbox_pixels) = " << mean_bbox_pixels
        << " sqrt(mean(bbox_pixels)) = " << sqrt(mean_bbox_pixels)
        << endl;

   delete texture_rectangle_ptr;
   delete annotated_texture_rectangle_ptr;

}

