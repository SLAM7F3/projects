// ==========================================================================
// Program VIBE implements the background subtraction algorithm from
// O. Barnich and M.V. Droogenbroeck, "ViBe: A Universal background
// subtraction algorithm for video sequences, IEEE Trans Image Processing
// 20 (6), 1709-1724, June 2011.
// ==========================================================================
// Last updated on 1/23/13; 1/24/13; 6/7/14
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "math/basic_math.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"
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

   int N=20;	// number of samples per pixel
//   int radius=20;  
   int sqrd_radius=sqr(20);  
	       // number of close samples for being part of background (bg)
   int hash_min=2;	// amount of random subsampling
   int phi=16;
   
   string GoPro_subdir=
      "/data_third_disk/plume/Nov_2012/video/GoPro_movies/";
   string frames_subdir=GoPro_subdir+"camera1/png/";
   vector<string> frame_filenames=filefunc::image_files_in_subdir(
      frames_subdir);

   int n_frames=frame_filenames.size();
   cout << "n_frames = " << n_frames << endl;

   unsigned int width=1920;
   unsigned int height=1080;
   imagefunc::get_image_width_height(frame_filenames[0],width,height);
   
   int* Rsamples_ptr=new int[width*height*N];
   int* Gsamples_ptr=new int[width*height*N];
   int* Bsamples_ptr=new int[width*height*N];
   int* segMap_ptr=new int[width*height];
   
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
      width,height,1,3,NULL);

   int i_start=1000;
   cout << "Enter starting frame number:" << endl;
   cin >> i_start;

   int background=0;
   int foreground=255;

   for (int i=i_start; i<n_frames; i++)
   {
      cout << "Processing frame " << i << endl;
      texture_rectangle_ptr->import_photo_from_file(frame_filenames[i]);
//      texture_rectangle_ptr->convert_color_image_to_luminosity();

      if (i==i_start)
      {
         initialize_samples(
            N,texture_rectangle_ptr,Rsamples_ptr,Gsamples_ptr,Bsamples_ptr);
      }

      int R,G,B;
      int px_neighbor,py_neighbor;
      for (unsigned int py=0; py<height; py++)
      {
         for (unsigned int px=0; px<width; px++)
         {
            int py_width_px=py*width+px;
            
            texture_rectangle_ptr->get_pixel_RGB_values(px,py,R,G,B);

// 1.  Compare pixel to background model

            int count=0;
            int index=0;
            while ( (count < hash_min) && (index < N) )
            {
               int arg1=index*height*width+py_width_px;

               int dr=R-Rsamples_ptr[arg1];
               int dg=G-Gsamples_ptr[arg1];
               int db=B-Bsamples_ptr[arg1];
               int sqrd_dist=dr*dr+dg*dg+db*db;
//               int dist=sqrt(sqrd_dist);

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

      if (i%5==0) 
      {
         for (unsigned int py=0; py<height; py++)
         {
            for (unsigned int px=0; px<width; px++)
            {
               texture_rectangle_ptr->set_pixel_RGB_values(
                  px,py,segMap_ptr[py*width+px],0,0);
            }
         }

         string change_map_filename=
            "change_map_"+stringfunc::integer_to_string(i,3)+".jpg";
         texture_rectangle_ptr->write_curr_frame(change_map_filename);
      }

      
   } // loop over index i labeling video frame
   
   delete [] Rsamples_ptr;
   delete [] Gsamples_ptr;
   delete [] Bsamples_ptr;
   delete [] segMap_ptr;

   delete texture_rectangle_ptr;
}
