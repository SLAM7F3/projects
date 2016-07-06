// ==========================================================================
// Program SMOKE_MASK employs the ViBe background subtraction
// algorithm in order to classify smoke vs non-smoke pixels in t > 0
// plume images.  After ViBe classification is performed, SMOKE_MASK
// eliminates small connected components within the resulting masks.
// It also performs recursive filling in order to eliminate small
// holes surrounded by oceans of smoke pixels.
// ==========================================================================
// Last updated on 1/23/13; 1/24/13
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "math/basic_math.h"
#include "image/binaryimagefuncs.h"
#include "video/connected_components.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"
#include "passes/PassesGroup.h"
#include "postgres/plumedatabasefuncs.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "image/recursivefuncs.h"
#include "general/stringfuncs.h"
#include "video/texture_rectangle.h"


int getRandomNeighborXCoordinate(int px,int width)
{
   int random_int=mathfunc::getRandomInteger(3)-1;
   int px_new=px+random_int;
   if (px_new < 0) px_new=0;
   if (px_new >= width) px_new=width-1;
   return px_new;
}

int getRandomNeighborYCoordinate(int py,int height)
{
   int random_int=mathfunc::getRandomInteger(3)-1;
   int py_new=py+random_int;
   if (py_new < 0) py_new=0;
   if (py_new >= height) py_new=height-1;
   return py_new;
}

void getRandomNeighborCoordinates(
   int px,int py,int width,int height,int& px_neighbor,int& py_neighbor)
{
   do 
   {
      px_neighbor=getRandomNeighborXCoordinate(px,width);
      py_neighbor=getRandomNeighborYCoordinate(py,height);
   }
   while (px_neighbor==0 && py_neighbor==0);
}

void initialize_samples(
   int N,texture_rectangle* texture_rectangle_ptr,
   int* Rsamples_ptr,int* Gsamples_ptr,int* Bsamples_ptr)
{
   int px_neighbor,py_neighbor;
   int R,G,B;
   int width=texture_rectangle_ptr->getWidth();
   int height=texture_rectangle_ptr->getHeight();
   for (int index=0; index<N; index++)
   {
      for (int px=0; px<width; px++)
      {
         for (int py=0; py<height; py++)
         {
            getRandomNeighborCoordinates(
               px,py,width,height,px_neighbor,py_neighbor);

            texture_rectangle_ptr->get_pixel_RGB_values(
               px_neighbor,py_neighbor,R,G,B);

            int arg=index*height*width+py*width+px;
            Rsamples_ptr[arg]=R;
            Gsamples_ptr[arg]=G;
            Bsamples_ptr[arg]=B;
         } // loop over py index
      } // loop over px index
   }
}

// ==========================================================================
int main (int argc, char* argv[])
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::string;
   using std::vector;

   int N=20;   		// number of samples per pixel
   int hash_min=2;	// amount of random subsampling
   int phi=16;

//   int radius=20;  
//   cout << "Enter radius:" << endl;
//   cin >> radius;

//   int min_n_pixels_per_cluster=10;
//   cout << "Enter min number pixels per cluster" << endl;
//   cin >> min_n_pixels_per_cluster;

   vector<int> radius;
   radius.push_back(7.5);
   radius.push_back(12.5);
   radius.push_back(17.5);
   radius.push_back(22.5);
   radius.push_back(27.5);
//   radius.push_back(30);

   vector<int> min_n_pixels_per_cluster;   
   min_n_pixels_per_cluster.push_back(35);
   min_n_pixels_per_cluster.push_back(30);
   min_n_pixels_per_cluster.push_back(25);
   min_n_pixels_per_cluster.push_back(20);
   min_n_pixels_per_cluster.push_back(15);
//   min_n_pixels_per_cluster.push_back(10);

   string GoPro_subdir=
      "/data_third_disk/plume/Nov_2012/video/GoPro_movies/";
   string frames_subdir=GoPro_subdir+"camera1/png/";
   vector<string> frame_filenames=filefunc::image_files_in_subdir(
      frames_subdir);

   int n_frames=frame_filenames.size();
   cout << "n_frames = " << n_frames << endl;
   int n_frames_max=500;
   
   int width=1920;
   int height=1080;
   imagefunc::get_image_width_height(frame_filenames[0],width,height);
   
   int* Rsamples_ptr=new int[width*height*N];
   int* Gsamples_ptr=new int[width*height*N];
   int* Bsamples_ptr=new int[width*height*N];
   int* segMap_ptr=new int[width*height];
   
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
      width,height,1,3,NULL);
   texture_rectangle* classification_texture_rectangle_ptr=
      new texture_rectangle(width,height,1,3,NULL);

   int i_start=1000;
   cout << "Enter starting frame number:" << endl;
   cin >> i_start;

   int background=0;
   int foreground=255;
   double znull=0;
   double zfill=1;

   connected_components* connected_components_ptr=new connected_components();
   twoDarray* pbinary_twoDarray_ptr=NULL;
   twoDarray* ptwoDarray_ptr=NULL;
   twoDarray* classification_twoDarray_ptr=NULL;
   int xdim,ydim;

// ===========================================================================
// Loop over video frames starts here

   for (int i=i_start; i<n_frames; i++)
   {
      cout << "Processing frame " << i << endl;

      int n_iters=radius.size();
      for (int iter=0; iter<n_iters; iter++)
      {
         int sqrd_radius=sqr(radius[iter]);  
         // number of close samples for being part of background (bg)

         texture_rectangle_ptr->import_photo_from_file(frame_filenames[i]);

         if (i==i_start)
         {
            initialize_samples(
               N,texture_rectangle_ptr,Rsamples_ptr,Gsamples_ptr,Bsamples_ptr);
         }

         int R,G,B;
         int px_neighbor,py_neighbor;
         for (int py=0; py<height; py++)
         {
            for (int px=0; px<width; px++)
            {
               int py_width_px=py*width+px;
            
               texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);

// 1.  Compare pixel to background model

               int count=0;
               int index=0;
               int dist=0;
               while ( (count < hash_min) && (index < N) )
               {
                  int arg1=index*height*width+py_width_px;

                  int dr=R-Rsamples_ptr[arg1];
                  int dg=G-Gsamples_ptr[arg1];
                  int db=B-Bsamples_ptr[arg1];
                  int sqrd_dist=dr*dr+dg*dg+db*db;
                  int dist=sqrt(sqrd_dist);

                  if (sqrd_dist < sqrd_radius) count++;
//               if (dist < radius) count++;
                  index++;
//               cout << "dist = " << dist << " count = " << count
//                   << " index = " << index << endl;
               } // while loop

// 2.  Classify pixel and update model

//            cout << "count = " << count << endl;
               if (count >= hash_min)
               {

// Classify current pixel as background:

                  segMap_ptr[py_width_px]=background;
               
// 3.  Update current pixel model 

// Get random number between 0 and phi-1 (or alternatively, random
// number between 0 and 1 and ask if its value is less than 1/phi):

                  int random_int1=mathfunc::getRandomInteger(phi-1);
                  if (random_int1==0)	// random subsampling
                  {

// Replace randomly chosen sample

                     int random_int2=mathfunc::getRandomInteger(N-1);
                     int arg2=random_int2*height*width+py_width_px;
                     Rsamples_ptr[arg2]=R;
                     Gsamples_ptr[arg2]=G;
                     Bsamples_ptr[arg2]=B;
                  }
               
// 4.  Update neighboring pixel model

                  int random_int3=mathfunc::getRandomInteger(phi-1);
                  if (random_int3==0)	// random subsampling
                  {
// Choose neighboring pixel randomly

                     getRandomNeighborCoordinates(
                        px,py,width,height,px_neighbor,py_neighbor);

// Replace randomly chosen sample:

                     int random_int4=mathfunc::getRandomInteger(N-1);
                     int arg4=random_int4*height*width+py_neighbor*width+
                        px_neighbor;
                     Rsamples_ptr[arg4]=R;
                     Gsamples_ptr[arg4]=G;
                     Bsamples_ptr[arg4]=B;
                  
                  } // random_int3==0 conditional

               }
               else
               {
// Classify current pixel as foreground:

                  segMap_ptr[py_width_px]=foreground;
               
               } // count >= hashmin conditional

            } // loop over px index
         } // loop over py index

         if (i%1==0) 
         {
            for (int py=0; py<height; py++)
            {
               for (int px=0; px<width; px++)
               {
                  int R=segMap_ptr[py*width+px];
                  texture_rectangle_ptr->set_pixel_RGB_values(
                     px,py,R,R,R);
               }
            }

            string change_map_filename=
               "change_map_"+stringfunc::integer_to_string(i,3)+"_"
               +stringfunc::integer_to_string(iter,2)+".jpg";
            texture_rectangle_ptr->write_curr_frame(change_map_filename);

            int color_channel_ID=-1;
            int image_counter=0;
            connected_components_ptr->reset_image(
               change_map_filename,color_channel_ID,image_counter);
            filefunc::deletefile(change_map_filename);

            int threshold=128;
            int n_components=connected_components_ptr->
               compute_connected_components(
                  threshold,min_n_pixels_per_cluster[iter]);
            cout << "n_components = " << n_components << endl;

            string connected_regions_filename=
               "ccs_"+stringfunc::integer_to_string(i,3)+"_"
               +stringfunc::integer_to_string(iter,2)+".jpg";
//            connected_components_ptr->color_connected_components(
//               connected_regions_filename);

            twoDarray* cc_twoDarray_ptr=connected_components_ptr->
               get_cc_twoDarray_ptr();

            if (pbinary_twoDarray_ptr==NULL)
            {
               pbinary_twoDarray_ptr=new twoDarray(cc_twoDarray_ptr);
               ptwoDarray_ptr=new twoDarray(cc_twoDarray_ptr);
               classification_twoDarray_ptr=new twoDarray(cc_twoDarray_ptr);
               xdim=ptwoDarray_ptr->get_xdim();
               ydim=ptwoDarray_ptr->get_ydim();
            }
            binaryimagefunc::binary_threshold(
               0.5,cc_twoDarray_ptr,pbinary_twoDarray_ptr,znull,zfill);

/*
  int pxlo,pxhi,pylo,pyhi;
  imagefunc::compute_pixel_borders(
  pbinary_twoDarray_ptr,pxlo,pxhi,pylo,pyhi);

  int nrecursion_max=10;
  double zempty_value=0;
  double zfill_value=1;
  recursivefunc::binary_fill(
  nrecursion_max,pxlo,pxhi,pylo,pyhi,
  zempty_value,zfill_value,pbinary_twoDarray_ptr);
*/

            pbinary_twoDarray_ptr->copy(cc_twoDarray_ptr);

            string binary_map_filename="binary_"+
               stringfunc::integer_to_string(i,3)+"_"+
               stringfunc::integer_to_string(iter,2)+".jpg";

            bool uniform_color_flag=false;
//         bool uniform_color_flag=true;
//            connected_components_ptr->color_connected_components(
//               binary_map_filename,uniform_color_flag);

            if (iter==0) classification_twoDarray_ptr->clear_values();
            double class_label=double(radius[iter])/double(radius.back());
            for (int py=0; py<ydim; py++)
            {
               for (int px=0; px<xdim; px++)
               {
                  double curr_p=pbinary_twoDarray_ptr->get(px,py);
                  if (nearly_equal(curr_p,0)) continue;
                  classification_twoDarray_ptr->put(px,py,class_label);
               } // loop over px
            } // loop over py
         } // i%1==0 conditional
      
      } // loop over iter index
      
      imagefunc::median_fill(3,classification_twoDarray_ptr,znull);

      classification_texture_rectangle_ptr->initialize_RGB_twoDarray_image(
        classification_twoDarray_ptr);
      classification_texture_rectangle_ptr->fill_twoDarray_image(
         classification_twoDarray_ptr,3);
      classification_texture_rectangle_ptr->convert_grey_values_to_hues();

      texture_rectangle_ptr->import_photo_from_file(frame_filenames[i]);
      texture_rectangle_ptr->convert_color_image_to_greyscale();

      string classification_subdir="./classification/";
      filefunc::dircreate(classification_subdir);
      string classification_filename=classification_subdir+
         "classification_"+stringfunc::integer_to_string(i,3)+".jpg";
      cout << "classification_filename = " << classification_filename
           << endl;
      classification_texture_rectangle_ptr->write_curr_frame(
         classification_filename);

// Fuse hue-colored classification image with grey colored video
// frame:

      double h,s,v,hc,sc,vc;
      for (int py=0; py<ydim; py++)
      {
         for (int px=0; px<xdim; px++)
         {
            texture_rectangle_ptr->get_pixel_hsv_values(px,py,h,s,v);
            classification_texture_rectangle_ptr->get_pixel_hsv_values(
               px,py,hc,sc,vc);
            s=basic_math::max(s,sc);
            texture_rectangle_ptr->set_pixel_hsv_values(px,py,hc,s,v);
//            texture_rectangle_ptr->set_pixel_hsv_values(px,py,hc,sc,vc);
         } // loop over px
      } // loop over py

      string frame_classification_filename=classification_subdir+
         "frame_classification_"+stringfunc::integer_to_string(i,3)+".jpg";
      texture_rectangle_ptr->write_curr_frame(frame_classification_filename);

   } // loop over index i labeling video frame
   
   delete [] Rsamples_ptr;
   delete [] Gsamples_ptr;
   delete [] Bsamples_ptr;
   delete [] segMap_ptr;

   delete texture_rectangle_ptr;
   delete classification_texture_rectangle_ptr;
   delete connected_components_ptr;
}
