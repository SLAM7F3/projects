// ==========================================================================
// Program MERGE_TILES is a quick-n-dirty utility which we wrote in
// order to merge together the geotiff files for the RTV NYC ladar
// data set.  The total size of the consolidated file is ~250 Mbytes.  
// ==========================================================================
// Last updated on 7/6/09
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "image/raster_parser.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "image/TwoDarray.h"

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
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
   
// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input TDP file containing XYZ cloud information:

   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   cout << "cloudpass_ID = " << cloudpass_ID << endl;
   vector<string> tdp_filenames=
      passes_group.get_pass_ptr(cloudpass_ID)->get_filenames();

// Compute extremal x,y values for all input TDP tiles:

   double xmin=POSITIVEINFINITY;
   double xmax=NEGATIVEINFINITY;
   double ymin=POSITIVEINFINITY;
   double ymax=NEGATIVEINFINITY;

   double delta_x=1.0;		// meters	NYC
   double delta_y=1.0;		// meters	NYC

/*
   for (int t=0; t<tdp_filenames.size(); t++)
   {
      cout << "t = " << t << " tdp_filename = " << tdp_filenames[t] << endl;
      twoDarray* curr_ztwoDarray_ptr=
         tdpfunc::generate_ztwoDarray_from_tdpfile(
            tdp_filenames[t],delta_x,delta_y);

      xmin=min(xmin,curr_ztwoDarray_ptr->get_xlo());
      xmax=max(xmax,curr_ztwoDarray_ptr->get_xhi());
      ymin=min(ymin,curr_ztwoDarray_ptr->get_ylo());
      ymax=max(ymax,curr_ztwoDarray_ptr->get_yhi());

      delete curr_ztwoDarray_ptr;
   }
*/

   cout << "xmin = " << xmin << " xmax = " << xmax << endl;
   cout << "ymin = " << ymin << " ymax = " << ymax << endl;

// Hard coded extremal values for NYC ladar data:
   
   xmin = 582789.125;
   xmax = 590686.25;
   ymin = 4505938;
   ymax = 4522753.5;

// Generate new consolidated tile with same delta_x,delta_y as
// individual input tiles:

   int mtotal=(xmax-xmin)/delta_x+1;
   int ntotal=(ymax-ymin)/delta_y+1;
   
   twoDarray* consolidated_ztwoDarray_ptr=new twoDarray(mtotal,ntotal);
   consolidated_ztwoDarray_ptr->init_coord_system(xmin,xmax,ymin,ymax);

   cout << "mtotal = " << mtotal << " ntotal = " << ntotal << endl;
   cout << "cons xlo = " << consolidated_ztwoDarray_ptr->get_xlo() << endl;
   cout << "cons xhi = " << consolidated_ztwoDarray_ptr->get_xhi() << endl;
   cout << "cons ylo = " << consolidated_ztwoDarray_ptr->get_ylo() << endl;
   cout << "cons yhi = " << consolidated_ztwoDarray_ptr->get_yhi() << endl;

// Loop over each individual input tile and place its contents into
// the consolidated tile:

   for (int t=0; t<tdp_filenames.size(); t++)
   {
      cout << "t = " << t << " tdp_filename = " << tdp_filenames[t] << endl;
      twoDarray* curr_ztwoDarray_ptr=
         tdpfunc::generate_ztwoDarray_from_tdpfile(
            tdp_filenames[t],delta_x,delta_y);

// Iterate over curr ztwoDarray's pixels and place its contents into
// the consllidated ztwoDarray:

      unsigned int qx,qy;
      int mdim=curr_ztwoDarray_ptr->get_mdim();
      int ndim=curr_ztwoDarray_ptr->get_ndim();
      cout << "mdim = " << mdim << " ndim = " << ndim << endl;
      
      for (unsigned int px=0; px<mdim; px++)
      {
         double x=curr_ztwoDarray_ptr->fast_px_to_x(px);
         consolidated_ztwoDarray_ptr->x_to_px(x,qx);
         for (unsigned int py=0; py<ndim; py++)
         {
            double y=curr_ztwoDarray_ptr->fast_py_to_y(py);
            consolidated_ztwoDarray_ptr->y_to_py(y,qy);
            double curr_z=curr_ztwoDarray_ptr->fast_XY_to_Z(x,y);
//            cout << "qx = " << qx << " qy = " << qy
//                 << " curr_z = " << curr_z << endl;
            consolidated_ztwoDarray_ptr->put(qx,qy,curr_z);
         } // loop over py labeling pixels in *curr_ztwoDarray_ptr
      } // loop over px labeling pixels in *curr_ztwoDarray_ptr
      
   } // loop over index t labeling input tiles

// Write consolidated tile to geotiff output file:

   raster_parser RasterParser;

   string geotiff_filename="consolidated.tif";
   int output_UTM_zonenumber=18;	// NYC
   bool output_northern_hemisphere_flag=true;

   bool output_floats_flag=true;
   if (output_floats_flag)
   {
      RasterParser.write_raster_data(
         output_floats_flag,geotiff_filename,
         output_UTM_zonenumber,output_northern_hemisphere_flag,
         consolidated_ztwoDarray_ptr);
   }
   else
   {

// NYC z bounds:

      double z_min = 0;
      double z_max = 396;

      RasterParser.write_raster_data(
         output_floats_flag,geotiff_filename,
         output_UTM_zonenumber,output_northern_hemisphere_flag,
         consolidated_ztwoDarray_ptr,z_min,z_max);
   }

}
