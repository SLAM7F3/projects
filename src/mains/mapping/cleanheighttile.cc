// ==========================================================================
// Program CLEANHEIGHTTILE performs some simple cleaning operations on
// ALIRT height imagery geotif files.  It first identifies and
// eliminates isolated height outlier pixels.  CLEANHEIGHTTILE next
// iteratively searches and median fills null-valued pixels.  Filtered
// geotif, tdp and osga files are written out to subdirectories of
// ./filtered_Z_tiles. 

// 	       	       cleanheighttile tile_0_5.tif

// ==========================================================================
// Last updated on 5/1/11; 5/2/11; 4/6/14
// ==========================================================================

#include <iostream>
#include <string>
#include "image/imagefuncs.h"
#include "passes/PassesGroup.h"
#include "image/raster_parser.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "image/TwoDarray.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;

// ==========================================================================
int main (int argc, char * argv[])
{
   std::set_new_handler(sysfunc::out_of_memory);
   
// Use an ArgumentParser object to manage the program arguments:
   
   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   Pass* pass_ptr=passes_group.get_pass_ptr(0);
   string image_filename=pass_ptr->get_first_filename();
   cout << "image_filename = " << image_filename << endl;

   raster_parser RasterParser;
   RasterParser.open_image_file(image_filename);

   int channel_ID=0;
   RasterParser.fetch_raster_band(channel_ID);

   twoDarray* ztwoDarray_ptr=RasterParser.get_ztwoDarray_ptr();
   RasterParser.read_raster_data(ztwoDarray_ptr);

   twoDarray* ztwoDarray_filtered_ptr=new twoDarray(ztwoDarray_ptr);
   ztwoDarray_ptr->copy(ztwoDarray_filtered_ptr);

   unsigned int mdim=ztwoDarray_ptr->get_mdim();
   unsigned int ndim=ztwoDarray_ptr->get_ndim();
   cout << "mdim = " << mdim << " ndim = " << ndim << endl;

   const double min_Z=10;	// meters

// First scan through Z values and search for isolated height
// outliers.  Replace them with median of immediate surrounding
// neighbor values:

   int n_outliers=0;
   for (unsigned int px=0; px<mdim; px++)
   {
      for (unsigned int py=0; py<ndim; py++)
      {
         double curr_Z=ztwoDarray_ptr->get(px,py);
         if (curr_Z < min_Z) continue;

         unsigned int nsize=3;
         if (imagefunc::median_filter(
            px,py,nsize,ztwoDarray_ptr,
            ztwoDarray_filtered_ptr,min_Z))
         {
            double median_Z=ztwoDarray_filtered_ptr->get(px,py);
            const double max_z_difference=250;	// meters
            if (fabs(median_Z-curr_Z) > max_z_difference)
            {
               ztwoDarray_ptr->put(px,py,median_Z);
               n_outliers++;
            }
         }
         
      } // loop over py index
   } // loop over px index
   cout << "n_outliers = " << n_outliers << endl;
   ztwoDarray_ptr->copy(ztwoDarray_filtered_ptr);   

// Next scan through Z values and search for null-valued pixels.  Fill
// their Z values with median of neighboring values.  Perform multiple
// iterations of median filling in order to close holes/gaps in ladar
// height map:
   
   int n_iters=50;
   for (int iter=0; iter<n_iters; iter++)
   {
      int n_missing_pixels=0;
      int n_missing_filtered_pixels=0;

      const double null_value=NEGATIVEINFINITY;
      for (int px=0; px<mdim; px++)
      {
         if (px%1000==0) cout << px << " " << flush;
         for (int py=0; py<ndim; py++)
         {
            double curr_Z=ztwoDarray_ptr->get(px,py);

// Ignore any pixel whose Z value lies below some minimal threshold:

            if (curr_Z < min_Z)
            {
               n_missing_pixels++;
               int nsize=7;
               if (imagefunc::median_filter(
                  px,py,nsize,ztwoDarray_ptr,
                  ztwoDarray_filtered_ptr,min_Z))
               {
                  double filtered_Z=ztwoDarray_filtered_ptr->get(px,py);
               }
               else
               {
                  ztwoDarray_filtered_ptr->put(px,py,null_value);
                  n_missing_filtered_pixels++;
               }
            }
         }
      }
      cout << endl;

      cout << "n_missing_pixels = " << n_missing_pixels << endl;
      cout << "n_missing_filtered_pixels = "
           << n_missing_filtered_pixels << endl;
      
      ztwoDarray_filtered_ptr->copy(ztwoDarray_ptr);

   } // loop over iter index

   bool output_floats_flag=true;
   bool northern_hemisphere_flag=true;
   int output_UTM_zonenumber=42;	// Afghanistan
   string filtered_geotif_filename="filtered_"+image_filename;

   RasterParser.write_raster_data(
      output_floats_flag,filtered_geotif_filename,
      output_UTM_zonenumber,northern_hemisphere_flag,
      ztwoDarray_filtered_ptr);

// Convert filtered geotif file into TDP output.  Then run Ross'
// LODTREE program to convert TDP file into OSGA file:

   string unix_cmd=
      "/home/cho/programs/c++/svn/projects/src/mains/mapping/tif2tdp "+
      filtered_geotif_filename;
   sysfunc::unix_command(unix_cmd);

   string basename=filefunc::getbasename(filtered_geotif_filename);
   string prefix=stringfunc::prefix(basename);
   string tdp_filename=prefix+".tdp";
   unix_cmd="lodtree "+tdp_filename;
   sysfunc::unix_command(unix_cmd);

// Move all filtered height files into subdirectories of
// ./filtered_Z_tiles:

   unix_cmd="mv "+filtered_geotif_filename+" ./filtered_Z_tiles/tif/";
   sysfunc::unix_command(unix_cmd);

   unix_cmd="mv "+tdp_filename+" ./filtered_Z_tiles/tdp/";
   sysfunc::unix_command(unix_cmd);

   string osga_filename=prefix+".osga";
   unix_cmd="mv "+osga_filename+" ./filtered_Z_tiles/osga/";
   sysfunc::unix_command(unix_cmd);
} 

