// ==========================================================================
// Program TERRAIN_HEIGHTS is a specialized utility for computing the
// residual distribution for Lubbock ladar vs PMVS point cloud heights.

//				terrain_heights

// ==========================================================================
// Last updated on 3/1/11
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <set>
#include <string>
#include <vector>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "general/filefuncs.h"
#include "astro_geo/geofuncs.h"
#include "passes/PassesGroup.h"
#include "math/prob_distribution.h"
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
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

   const double min_altitude=800;	// meters above sea level
   const double max_altitude=1200;	// meters above sea level
   vector<double> delta_z;

// First read in dense PMVS point cloud into *zlh_twoDarray_ptr:

   string banner="Processing PMVS heights:";
   outputfunc::write_big_banner(banner);

   string lighthawk_subdir=
      "/home/cho/programs/c++/svn/projects/src/mains/photosynth/bundler/lighthawk/";
   string lighthawk_tdpfilename=
      lighthawk_subdir+"recolored_pmvs_options.txt.10.ladar.tdp";

   double delta_x=1.0;		// meters      
   double delta_y=1.0;		// meters      
   twoDarray* zlh_twoDarray_ptr=tdpfunc::generate_ztwoDarray_from_tdpfile(
      lighthawk_tdpfilename,delta_x,delta_y);
   cout << "*zlh_twoDarray_ptr = " << *zlh_twoDarray_ptr << endl;

// Next read input TDP files containing Lubbock ladar height data:

   vector<int> tile_number;
   tile_number.push_back(503);
   tile_number.push_back(504);
   tile_number.push_back(505);
   tile_number.push_back(506);
   tile_number.push_back(529);
   tile_number.push_back(530);
   tile_number.push_back(531);
   tile_number.push_back(532);
   tile_number.push_back(533);

   string ladar_subdir=
      "/media/10E8-0ED2/Bluegrass/ladar/tdp_files/median_filled/";
   vector<double> X,Y,Z;
   vector<threevector> XYZ_ladar;
   for (unsigned int t=0; t<tile_number.size(); t++)
   {
      string banner="Processing tile "+stringfunc::number_to_string(
         tile_number[t]);
      outputfunc::write_big_banner(banner);

      string ladar_tdp_filename=ladar_subdir+"tile_"+
         stringfunc::number_to_string(tile_number[t])+"_filled.tdp";
      tdpfunc::read_XYZ_points_from_tdpfile(ladar_tdp_filename,X,Y,Z);

      unsigned int qx,qy;
      for (unsigned int i=0; i<X.size(); i++)
      {
         if (Z[i] < min_altitude || Z[i] > max_altitude) continue;

         if (!zlh_twoDarray_ptr->point_to_pixel(X[i],Y[i],qx,qy)) continue;
         double z_lh=zlh_twoDarray_ptr->get(qx,qy);
         double dz=z_lh-Z[i];
//      cout << "dz = " << dz << endl;

// Reject height outliers in either lighthawk or ladar data:

         if (fabs(dz) > 100) continue;
      
         delta_z.push_back(dz);
      }
      
      X.clear();
      Y.clear();
      Z.clear();
      
      cout << "delta_z.size() = " << delta_z.size() << endl;

      double mu_dz=mathfunc::mean(delta_z);
      double sigma_dz=mathfunc::std_dev(delta_z);
      double median_dz=mathfunc::median_value(delta_z);

      cout << "mu_dz = " << mu_dz << endl;
      cout << "sigma_dz = " << sigma_dz << endl;
      cout << "median_dz = " << median_dz << endl;

   } // loop over index t labeling ladar tiles

   prob_distribution prob_dz(delta_z,1000);
   prob_dz.set_xmin(-4);
   prob_dz.set_xmax(4);
   prob_dz.set_xtic(1);
   prob_dz.set_xsubtic(1);

   cout << "peak density value = " << prob_dz.peak_density_value()
        << endl;

   bool gzip_flag=false;
   prob_dz.writeprobdists(gzip_flag);

/*

// Tile 531:

mu_dz = -0.307417
sigma_dz = 2.21758
median_dz = -0.398766

// Tile 532: 

mu_dz = 1.29532
sigma_dz = 2.23626
median_dz = 1.21738

// Tiles 503, 504, 505, 506, 529, 530, 531, 532, 533

mu_dz = 0.70772
sigma_dz = 2.99168
median_dz = 0.256171

*/


}
