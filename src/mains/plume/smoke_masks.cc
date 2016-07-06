// ==========================================================================
// Program SMOKE_MASKS employs the ViBe background subtraction
// algorithm in order to classify pixels as "smoke" or "non-smoke" for
// t > 0 plume images.  By varying the ViBe radius threshold, we
// generate increasingly strict "smoke" classes.  SMOKE_MASKS
// eliminates small connected components whose number of pixels is
// less than a min_n_pixels_per_cluster threshold whose value varies
// with the Vibe radius threshold.  After the multiple masks are
// consolidated, median filling is performed to minimize small holes
// in smoke plumes (caused by foreground occluders).

// SMOKE_MASKS exports three outputs for each mask.  Firstly, it
// writes out a binary file containing pixel_width * pixel_height
// 2-byte integers.  Each 2-byte integer ranging from 0 to 65535
// encodes a mask value ranging from 0 to 1.  The total byte size of
// the binary mask file = 2 * pixel_width * pixel_height.  The binary
// mask files are bzipped to save disk space.

// SMOKE_MASKS secondly exports each mask as a colored JPG to files with names
// "Colored_TimeAvged_0_ ...  . jpg".  Red [blue] pixels in these
// colored jpgs indicate regions with maximum [minimum] "smokiness"
// confidence.  Finally, SMOKE_MASKS outputs another JPG with name
// "fused_classification_ ... .jpg".  It shows the colored smoke mask
// superposed on a greyscale version of the original image.  


/*

./smoke_masks \
--region_filename ./bundler/plume/Nov2012/Day2_video/rover2K9K_fixed/packages/peter_inputs.pkg \
--GIS_layer ./packages/plume_metadata.pkg 

*/


// ==========================================================================
// Last updated on 1/23/13; 1/24/13; 1/25/13; 1/26/13
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
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"


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
   std::set_new_handler(sysfunc::out_of_memory);

   timefunc::initialize_timeofday_clock();

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   string image_list_filename=passes_group.get_image_list_filename();
//   cout << " image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
//   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   vector<int> GISlayer_IDs=passes_group.get_GISlayer_IDs();

// Instantiate postgis database objects to send data to and retrieve
// data from external Postgres database:

   postgis_databases_group* postgis_databases_group_ptr=
      new postgis_databases_group;
   postgis_database* postgis_db_ptr=postgis_databases_group_ptr->
      generate_postgis_database_from_GISlayer_IDs(
         passes_group,GISlayer_IDs);
//   cout << "postgis_db_ptr = " << postgis_db_ptr << endl;

   int mission_ID=22;
   cout << "Enter mission ID:" << endl;
   cin >> mission_ID;

   int fieldtest_ID=plumedatabasefunc::retrieve_fieldtest_ID_given_mission_ID(
      postgis_db_ptr,mission_ID);
//   cout << "fieldtest_ID = " << fieldtest_ID << endl;

   int day_number;
   string experiment_label;
   plumedatabasefunc::retrieve_mission_metadata_from_database(
      postgis_db_ptr,fieldtest_ID,mission_ID,
      day_number,experiment_label);

   string start_timestamp;
   plumedatabasefunc::retrieve_fieldtest_metadata_from_database(
      postgis_db_ptr,fieldtest_ID,start_timestamp);
//   cout << "start_timestamp = " << start_timestamp << endl;
   Clock clock;
   cout.precision(13);

   bool UTC_flag=true;
   double epoch=
      clock.timestamp_string_to_elapsed_secs(start_timestamp,UTC_flag);
   int year=clock.get_year();
   string month_name=clock.get_month_name();

   string experiment_subdir="/data/ImageEngine/plume/";
   experiment_subdir += month_name+stringfunc::number_to_string(year)+"/Day";
   experiment_subdir += stringfunc::number_to_string(day_number)+"/"+
      experiment_label+"/";
//   cout << "experiment_subdir = " << experiment_subdir << endl;

   string bksub_subdir=experiment_subdir+"bksub/time_slices/";
   cout << "bksub_subdir = " << bksub_subdir << endl;
   filefunc::dircreate(bksub_subdir);

   int initial_frame_number=1;
//   cout << "Enter starting frame number:" << endl;
//   cin >> initial_frame_number;

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

/*
   radius.push_back(7.5);
   radius.push_back(12.5);
   radius.push_back(17.5);
   radius.push_back(22.5);
   radius.push_back(27.5);
*/

   radius.push_back(10);
   radius.push_back(15);
   radius.push_back(20);
   radius.push_back(25);
   radius.push_back(30);

   vector<int> min_n_pixels_per_cluster;   
   min_n_pixels_per_cluster.push_back(35);
   min_n_pixels_per_cluster.push_back(30);
   min_n_pixels_per_cluster.push_back(25);
   min_n_pixels_per_cluster.push_back(20);
   min_n_pixels_per_cluster.push_back(15);

   string bksub_dir="/data_third_disk/plume/Nov_2012/video/Day2/H/bksub/";
   filefunc::dircreate(bksub_dir);

// ===========================================================================
// Loop over sensor ID starts here

   int start_sensor_ID=1;
   int stop_sensor_ID=10;
   for (int sensor_ID=start_sensor_ID; sensor_ID <= stop_sensor_ID;
        sensor_ID++)
   {
      string sensor_subdir=bksub_dir+"camera_"+
         stringfunc::integer_to_string(sensor_ID,2)+"/";
      filefunc::dircreate(sensor_subdir);

      vector<string> frame_filenames=
         plumedatabasefunc::retrieve_photo_URLs_from_database(
            postgis_db_ptr,fieldtest_ID,mission_ID,sensor_ID);

      int n_frames=frame_filenames.size();
      cout << "n_frames = " << n_frames << endl;

// Note added on 1/26/13: Falling "shrapnel" are visible in Video 4
// for Nov 2012 Expt 2H around frame 350.  So we can safely ignore any
// video frame beyond 500 for all 10 video cameras for Expt 2H:

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

      int background=0;
      int foreground=255;
      double znull=0;
      double zfill=1;

      connected_components* connected_components_ptr=
         new connected_components();
      twoDarray* pbinary_twoDarray_ptr=NULL;
      twoDarray* ptwoDarray_ptr=NULL;
      twoDarray* classification_twoDarray_ptr=NULL;
      int xdim,ydim;

// ===========================================================================
// Loop over video frames starts here

      int i_step=10;
//      int i_step=30;
      for (int i_start=initial_frame_number; i_start<initial_frame_number+
              i_step; i_start++)
      {
         for (int i=i_start; i<n_frames_max; i += i_step)
         {
            cout << endl;
            cout << "Sensor ID = " << sensor_ID 
                 << " : Processing frame " << i << endl;
            outputfunc::print_elapsed_time();

            string curr_slice_subdir=bksub_subdir+
               "t"+stringfunc::integer_to_string(i,5)+"/";
            filefunc::dircreate(curr_slice_subdir);

            int n_iters=radius.size();
            for (int iter=0; iter<n_iters; iter++)
            {
               int sqrd_radius=sqr(radius[iter]);  
               // number of close samples for being part of background (bg)

               texture_rectangle_ptr->import_photo_from_file(
                  frame_filenames[i]);

               if (i==i_start)
               {
                  initialize_samples(
                     N,texture_rectangle_ptr,Rsamples_ptr,Gsamples_ptr,
                     Bsamples_ptr);
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
                     classification_twoDarray_ptr=new twoDarray(
                        cc_twoDarray_ptr);
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
                  double class_label=double(radius[iter])/
                     double(radius.back());
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
      
//      imagefunc::median_fill(3,classification_twoDarray_ptr,znull);
            imagefunc::median_fill(5,classification_twoDarray_ptr,znull);
//            imagefunc::median_fill(7,classification_twoDarray_ptr,znull);
//      imagefunc::median_fill(9,classification_twoDarray_ptr,znull);

// Export binary version of *classification_twoDarray_ptr:

            string basename=filefunc::getbasename(frame_filenames[i]);
            string prefix=stringfunc::prefix(basename);
            string binary_mask_filename=sensor_subdir+prefix+"_bksub.bin";

            ofstream binary_outstream;
            filefunc::open_binaryfile(binary_mask_filename,binary_outstream);

            for (int py=0; py<ydim; py++)
            {
               for (int px=0; px<xdim; px++)
               {
                  double z=classification_twoDarray_ptr->get(px,py);
                  unsigned short z_short=65535*z;
                  filefunc::writeobject(binary_outstream,z_short);
               }
            }
            binary_outstream.close();
            filefunc::bzip2_file(binary_mask_filename);
            string bzipped_filename=binary_mask_filename+".bz2";
            string unix_cmd="mv "+bzipped_filename+" "+curr_slice_subdir;
            sysfunc::unix_command(unix_cmd);

// Export classification mask as colored JPG:

            string colored_mask_filename=sensor_subdir+"Colored_TimeAvgd_0_"+
               prefix+"_bksub.bin.jpg";
            classification_texture_rectangle_ptr->
               initialize_RGB_twoDarray_image(classification_twoDarray_ptr);
            classification_texture_rectangle_ptr->fill_twoDarray_image(
               classification_twoDarray_ptr,3);
            classification_texture_rectangle_ptr->
               convert_grey_values_to_hues();
            classification_texture_rectangle_ptr->write_curr_frame(
               colored_mask_filename);
            unix_cmd="cp "+colored_mask_filename+" "+curr_slice_subdir;
            sysfunc::unix_command(unix_cmd);

// Fuse hue-colored classification image with grey colored video
// frame:

// In order to conserve disk space, only generate fused images for
// video cameras #1, #4 and #6:

//            if (!(sensor_ID==1 || sensor_ID==4 || sensor_ID==6)) continue;

            texture_rectangle_ptr->import_photo_from_file(frame_filenames[i]);
            texture_rectangle_ptr->convert_color_image_to_greyscale();

            double h,s,v,hc,sc,vc;
            string fused_classification_filename=sensor_subdir+
               "fused_classification_"+prefix+"_bksub.jpg";
            for (int py=0; py<ydim; py++)
            {
               for (int px=0; px<xdim; px++)
               {
                  texture_rectangle_ptr->get_pixel_hsv_values(px,py,h,s,v);
                  classification_texture_rectangle_ptr->get_pixel_hsv_values(
                     px,py,hc,sc,vc);
                  s=basic_math::max(s,sc);
                  texture_rectangle_ptr->set_pixel_hsv_values(px,py,hc,s,v);
               } // loop over px
            } // loop over py

            texture_rectangle_ptr->write_curr_frame(
               fused_classification_filename);
            unix_cmd="cp "+fused_classification_filename+" "+curr_slice_subdir;
            sysfunc::unix_command(unix_cmd);
      
         } // loop over index i labeling video frame

      } // loop over i_start 

// ===========================================================================
   
      delete [] Rsamples_ptr;
      delete [] Gsamples_ptr;
      delete [] Bsamples_ptr;
      delete [] segMap_ptr;

      delete texture_rectangle_ptr;
      delete classification_texture_rectangle_ptr;
      delete connected_components_ptr;

   } // loop over sensor_ID

}
