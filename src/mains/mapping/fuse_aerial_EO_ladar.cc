// ========================================================================
// Program FUSE_AERIAL_EO_LADAR is a specialized utility which we
// wrote in Dec 2009 in order to trivially drape colored aerial EO
// imagery onto TEC 2004 ladar data.  We first used program ROSS_TILE
// in order to chop apart 434 mbyte Boston_TEC.x0.y0.tdp into tiles
// which align with Geotif files that we created in Aug 2009 from
// MassGIS data.  FUSE_AERIAL_EO_LADAR then iterates over all points
// within each TDP tile, extracts its UTM easting and northing
// coordinates, looks up the counterpart RGB color values within the
// corresponding geotif tile, and exports a fused TDP file.  This
// program then calls LODTREE to convert the fused TDP data into an
// OSGA file.

// When 500 Gbyte Disk #2 is mounted on touchy2, EO tif files sit in
// /media/disk/touchy2_Ubuntu7/mapping/boston/tif_files/UTM_tif_files

// On touchy2, TEC 2004 ladar data sits in

// /data/ladar/TEC/TEC_2004/Boston_Downtown_MSL_2004/Boston_TEC.x0.y0.tdp

// and /data/ladar/TEC/TEC_2004/Boston_Downtown_MSL_2004/Boston_TEC.osga


//			    fuse_aerial_EO_ladar

// ========================================================================
// Last updated on 12/22/09; 12/23/09; 5/4/11
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "image/raster_parser.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::string;
   using std::vector;

/*
   string general_tdp_subdir=
      "/data/ladar/TEC/TEC_2004/Boston_Downtown_MSL_2004/tdp_tiles/compatible_EO_tiles/";

   string geotif_subdir=
      "/media/disk/touchy2_Ubuntu7/mapping/boston/tif_files/UTM_tif_files/";
*/

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input TDP file containing XYZ cloud information:

   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();

   cout.precision(12);

   int start_row_ID=4;
   int stop_row_ID=5;
   int start_column_ID=2;
   int stop_column_ID=4;

   double hue_start=180;	// cyan
//   cout << "Enter starting hue in degrees:" << endl;
//   cin >> hue_start;

   int n_wrap=3;
//   cout << "Enter n_wrap:" << endl;
//   cin >> n_wrap;

   double s_min=0.37;
//   cout << "Enter s_min:" << endl;
//   cin >> s_min;
   double s_max=1.0;

//   double z_min=-12;
   double z_min=0;
   double z_max=200;
//   double z_max=244;

   for (int row_ID=start_row_ID; row_ID <= stop_row_ID; row_ID++)
   {
      for (int column_ID=start_column_ID; column_ID <= stop_column_ID;
           column_ID++)
      {

/*
         string particular_tdp_subdir=general_tdp_subdir
            +"r"+stringfunc::number_to_string(row_ID)
            +"c"+stringfunc::number_to_string(column_ID)+"/";
         string init_tdp_filename=particular_tdp_subdir+"tile.x0.y0.tdp";
         string final_tdp_filename=particular_tdp_subdir+"tile.x"
            +stringfunc::number_to_string(row_ID)+".y"
            +stringfunc::number_to_string(column_ID)+".tdp";

         string unix_command="cp "+init_tdp_filename+" "+final_tdp_filename;
         cout << unix_command << endl;
         sysfunc::unix_command(unix_command);
         string tdp_filename=final_tdp_filename;
*/

         string z_tdp_subdir=
            "/media/66368D22368CF3F9/TOC11/FOB_Blessing/FOB_Blessing_tiles/z_tif_tiles/tiles/filtered_Z_tiles/tdp/";
         string tdp_filename="filtered_tile_"+
            stringfunc::number_to_string(row_ID)+"_"+
            stringfunc::number_to_string(column_ID)+".tdp";
         tdp_filename=z_tdp_subdir+tdp_filename;

         cout << "Z tdp_filename = " << tdp_filename << endl;

         vector<double> X,Y,Z;
         tdpfunc::read_XYZ_points_from_tdpfile(tdp_filename,X,Y,Z);
   
// Next read in EO GEOTIF files:

         string EO_geotif_subdir=
            "/media/66368D22368CF3F9/TOC11/FOB_Blessing/Buckeye/CMB_AOI_MP/mrsid_files/s3_tif_konar16/";

         string EO_geotif_filename=EO_geotif_subdir+"tile_"
            +stringfunc::number_to_string(row_ID)+"_"
            +stringfunc::number_to_string(column_ID)+".tif";
         cout << "EO_geotif_filename = " << EO_geotif_filename << endl;

         raster_parser RasterParser;
         RasterParser.open_image_file(EO_geotif_filename);

         int n_channels=RasterParser.get_n_channels();
//         cout << "n_channels = " << n_channels << endl;

         twoDarray *RtwoDarray_ptr,*GtwoDarray_ptr,*BtwoDarray_ptr;
         double x_lo,x_hi,y_lo,y_hi;

         for (int channel_ID=0; channel_ID<n_channels; channel_ID++)
         {
            RasterParser.fetch_raster_band(channel_ID);

            if (n_channels==3)
            {
//               cout << "channel_ID = " << channel_ID << endl;
               if (channel_ID==0)
               {
                  RtwoDarray_ptr=RasterParser.get_RtwoDarray_ptr();
//                  cout << "RtwoDarray_ptr = " << RtwoDarray_ptr << endl;
                  RasterParser.read_raster_data(RtwoDarray_ptr);

                  x_lo=RtwoDarray_ptr->get_xlo();
                  x_hi=RtwoDarray_ptr->get_xhi();
                  y_lo=RtwoDarray_ptr->get_ylo();
                  y_hi=RtwoDarray_ptr->get_yhi();
               }
               else if (channel_ID==1)
               {
                  GtwoDarray_ptr=RasterParser.get_GtwoDarray_ptr();
//            cout << "GtwoDarray_ptr = " << GtwoDarray_ptr << endl;
                  RasterParser.read_raster_data(GtwoDarray_ptr);
               }
               else if (channel_ID==2)
               {
                  BtwoDarray_ptr=RasterParser.get_BtwoDarray_ptr();
//            cout << "BtwoDarray_ptr = " << BtwoDarray_ptr << endl;
                  RasterParser.read_raster_data(BtwoDarray_ptr);
               }
            } // n_channels conditional
         } // loop over channel_ID labeling RGB channels

         RasterParser.close_image_file();

         cout << "x_lo = " << x_lo << " x_hi = " << x_hi << endl;
         cout << "y_lo = " << y_lo << " y_hi = " << y_hi << endl;

         int n_points=X.size();
         int px,py;
         vector<int> R,G,B;
         for (int n=0; n<n_points; n++)
         {
            if (n%100000==0) cout << n/100000 << " " << flush;

//      cout << "n = " << n << " X = " << X[n]
//           << " Y = " << Y[n] << " Z = " << Z[n] << endl;

            double zfrac=(Z[n]-z_min)/(z_max-z_min);
//               const double hue_start=180;	// cyan
//               const double hue_start=150;	// green-cyan
//               const double hue_start=120;	// green

            const double hue_stop=hue_start-n_wrap*360;
            double h=hue_start+zfrac*(hue_stop-hue_start);

            double s=s_max;
            double v=1;
            double r,g,b;
            if (RtwoDarray_ptr->point_to_pixel(X[n],Y[n],px,py))
            {
               r=RtwoDarray_ptr->get(px,py)/255.0;
               g=GtwoDarray_ptr->get(px,py)/255.0;
               b=BtwoDarray_ptr->get(px,py)/255.0;
               colorfunc::RGB_to_hsv(r,g,b,h,s,v);
               s=s_min+s*(s_max-s_min);
            }
            else
            {
               cout << "******  No RGB counterpart found!!! ******" << endl;

               cout << "X = " << X[n] << " Y = " << Y[n] << endl;
               double dY=1;	// meter
               if (RtwoDarray_ptr->point_to_pixel(X[n],Y[n]+dY,px,py))
               {
                  cout << "RGB counterpart found for Y+dY" << endl;
               }
               if (RtwoDarray_ptr->point_to_pixel(X[n],Y[n]-dY,px,py))
               {
                  cout << "RGB counterpart found for Y-dY" << endl;
               }
               outputfunc::enter_continue_char();
               
            }

            colorfunc::hsv_to_RGB(h,s,v,r,g,b);
            R.push_back(basic_math::round(255*r));
            G.push_back(basic_math::round(255*g));
            B.push_back(basic_math::round(255*b));
         } // loop over index n labeling XYZ points

// Write colored XYZ points to output tdp file:
   
         bool northern_hemisphere_flag=true;
//         int specified_UTM_zonenumber=19;	// Boston
         int specified_UTM_zonenumber=42;	// Afghanistan
         string UTMzone=stringfunc::number_to_string(
            specified_UTM_zonenumber);

         string prefix=stringfunc::prefix(tdp_filename);
         string output_tdp_filename=prefix+"_rgb.tdp";
         string output_osga_filename=prefix+"_rgb.osga";
         cout << "Output tdp_filename = " << output_tdp_filename << endl;

         tdpfunc::write_relative_xyzrgba_data(
            UTMzone,output_tdp_filename,X,Y,Z,R,G,B);

         string lodtree_cmd=
            "/home/cho/programs/c++/svn/projects/src/mains/mapping/lodtree ";
         string unix_command=lodtree_cmd+" -o "+z_tdp_subdir+" "
            +output_tdp_filename;
         cout << "lodtree command = " << unix_command << endl;
         sysfunc::unix_command(unix_command);

         string final_osga_subdir=
//            "/data/ladar/TEC/TEC_2004/Boston_Downtown_MSL_2004/fused_osga_tiles/";
            "/media/66368D22368CF3F9/TOC11/FOB_Blessing/FOB_Blessing_tiles/fused_tiles/EO_Z/";

         unix_command="mv "+output_osga_filename+" "+final_osga_subdir;
         cout << unix_command << endl;
         sysfunc::unix_command(unix_command);

//         unix_command="/bin/rm "+output_tdp_filename;
         unix_command="mv "+output_tdp_filename+" "+final_osga_subdir;
         cout << unix_command << endl;
         sysfunc::unix_command(unix_command);

      } // loop over column_ID index
   } // loop over row_ID index

   cout << "hue_start = " << hue_start << endl;
   cout << "n_wrap = " << n_wrap << endl;
   cout << "s_min = " << s_min << endl;
}
