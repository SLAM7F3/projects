// ========================================================================
// Program FIX_LADARMAP iterates over all pixels within an ALIRT map
// geotif.  It compares each z value with the median height within
// some tile surrounding the current pixel.  If the discrepancy
// between the two height values is greater than 50 meters, the
// outlier value is replaced with the median value.  FIX_LADARMAP
// continues with such median filtering until no pixel within the
// ALIRT map is altered.
// ========================================================================
// Last updated on 12/26/11
// ========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "osg/osgGraphicals/AnimationController.h"
#include "osg/osgGeometry/ArrowsGroup.h"
#include "osg/Custom2DManipulator.h"
#include "osg/osgOrganization/Decorations.h"
#include "osg/osgEarth/EarthRegionsGroup.h"
#include "osg/osgFusion/FusionGroup.h"
#include "osg/osgFusion/FusionKeyHandler.h"
#include "osg/osgGrid/GridKeyHandler.h"
#include "osg/osgGrid/LatLongGridsGroup.h"
#include "messenger/Messenger.h"
#include "osg/ModeController.h"
#include "osg/ModeKeyHandler.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"
#include "osg/osgSceneGraph/MyDatabasePager.h"
#include "osg/osgOperations/Operations.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "osg/osg3D/PointCloudKeyHandler.h"
#include "osg/osgGraphicals/PointFinder.h"
#include "osg/osgGeometry/PolygonsGroup.h"
#include "osg/osgGeometry/PolyhedraGroup.h"
#include "osg/osgGeometry/PolyLinesGroup.h"
#include "osg/osgGIS/postgis_databases_group.h"
#include "image/raster_parser.h"
#include "osg/osgTiles/ray_tracer.h"
#include "osg/osg3D/Terrain_Manipulator.h"
#include "osg/osgTiles/TilesGroup.h"
#include "time/timefuncs.h"
#include "osg/osgWindow/ViewerManager.h"

#include "general/outputfuncs.h"

#include "geometry/polyline.h"
#include "osg/osg3D/tdpfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Instantiate TilesGroup to perform raytracing computations:

   TilesGroup* TilesGroup_ptr=new TilesGroup();
   string map_countries_name="HAFB";
   string geotif_subdir="/data/DTED/"+map_countries_name+"/geotif/";
   TilesGroup_ptr->set_geotif_subdir(geotif_subdir);
   TilesGroup_ptr->set_ladar_height_data_flag(true);
   TilesGroup_ptr->purge_tile_files();

   string ladar_tile_filename=geotif_subdir+"Ztiles/"+
      "orig_larger_flightfacility_ladarmap.tif";
   TilesGroup_ptr->load_ladar_height_data_into_ztwoDarray(ladar_tile_filename);

   twoDarray* DTED_ztwoDarray_ptr=TilesGroup_ptr->get_DTED_ztwoDarray_ptr();
   cout << "*DTED_ztwoDarray_ptr = " << *DTED_ztwoDarray_ptr << endl;

   int n_size=5;
   int mdim=DTED_ztwoDarray_ptr->get_mdim();
   int ndim=DTED_ztwoDarray_ptr->get_ndim();

   int px_lo=0;
   int px_hi=mdim-1;
   int py_lo=0;
   int py_hi=ndim-1;

   int n_iters=20;
   for (int iter=0; iter<n_iters; iter++)
   {
      string banner="Iteration number "+stringfunc::number_to_string(iter);
      outputfunc::write_big_banner(banner);
      cout << "px_lo = " << px_lo << " px_hi = " << px_hi << endl;
      cout << "py_lo = " << py_lo << " py_hi = " << py_hi << endl;
      
      double median,quartile;
      vector<double> z_values;
      vector<threevector> median_fix;
      for (int px=px_lo; px<=px_hi; px++)
      {
         outputfunc::update_progress_fraction(px,100,mdim);
         for (int py=py_lo; py<=py_hi; py++)
         {
            double z=DTED_ztwoDarray_ptr->get(px,py);
            if ( z < 0) continue;

            int mlo=px-n_size/2;
            mlo=basic_math::max(mlo,0);
            int mhi=px+n_size/2;
            mhi=basic_math::min(mhi,mdim-1);
         
            int nlo=py-n_size/2;
            nlo=basic_math::max(nlo,0);
            int nhi=py+n_size/2;
            nhi=basic_math::min(nhi,ndim-1);

            z_values.clear();
            for (int qx=mlo; qx<=mhi; qx++)
            {
               for (int qy=nlo; qy<=nhi; qy++)
               {
                  double curr_z=DTED_ztwoDarray_ptr->get(qx,qy);
                  if (curr_z < 0) continue;
                  z_values.push_back(curr_z);
               } // loop over qy index
            } // loop over qx index

            mathfunc::median_value_and_quartile_width(
               z_values,median,quartile);
            double ratio=fabs(z-median)/quartile;
            if (z > median+50)
            {
               median_fix.push_back(threevector(px,py,median));
               cout << "mdim = " << mdim << " ndim = " << ndim << endl;
               cout << "px = " << px << " py = " << py << endl;
               cout << "z = " << z << " median = " << median 
                    << " quartile = " << quartile 
                    << " ratio = " << ratio 
                    << " n_outliers = " << median_fix.size()
                    << endl;
//            outputfunc::enter_continue_char();
            }
//         cout << "px = " << px << " py = " << py << " z = " << z << endl;

         } // loop over py index
      } // loop over px index
      cout << endl;

      int n_outliers=median_fix.size();
      banner="Number of outliers replaced by median values = "+
         stringfunc::number_to_string(median_fix.size());
      outputfunc::write_banner(banner);

      if (n_outliers==0) break;

      int qx_lo=2*mdim;
      int qy_lo=2*ndim;
      int qx_hi=0;
      int qy_hi=0;

      for (unsigned int i=0; i<median_fix.size(); i++)
      {
         threevector curr_median_fix=median_fix[i];
         int qx=curr_median_fix.get(0);
         int qy=curr_median_fix.get(1);
         double z=curr_median_fix.get(2);
         DTED_ztwoDarray_ptr->put(qx,qy,z);

         qx_lo=basic_math::min(qx_lo,qx);
         qy_lo=basic_math::min(qy_lo,qy);

         qx_hi=basic_math::max(qx_hi,qx);
         qy_hi=basic_math::max(qy_hi,qy);

      } // loop over index i labeling median fixes

      px_lo=basic_math::max(qx_lo-5,0);
      py_lo=basic_math::max(qy_lo-5,0);
      
      px_hi=basic_math::min(qx_hi+5,mdim-1);
      py_hi=basic_math::min(qy_hi+5,ndim-1);
      
   } // loop over iter index

// Export cleaned version of ladar map to new tif file:

   bool output_floats_flag=true;
   int output_UTM_zonenumber=19;
   bool output_northern_hemisphere_flag=true;
   string corrected_ladar_tile_filename=
      "corrected_larger_flightfacility_ladarmap.tif";

   raster_parser RP;
   RP.write_raster_data(
      output_floats_flag,corrected_ladar_tile_filename,
      output_UTM_zonenumber,output_northern_hemisphere_flag,
      DTED_ztwoDarray_ptr);
}


