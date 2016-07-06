// ==========================================================================
// TDPFUNCS stand-alone methods
// ==========================================================================
// Last modified on 3/5/13; 6/13/13; 8/25/13
// ==========================================================================

#include <osg/Geometry>
#include <iostream>
#include <libtdp/tdp.h>
#include <libtdp/point_data.conf.h>
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "image/TwoDarray.h"
#include "threeDgraphics/xyzpfuncs.h"

#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::pair;
using std::string;
using std::vector;

namespace tdpfunc
{

   unsigned int max_points_per_iter=1000000;

// ==========================================================================
// TDP file input methods:
// ==========================================================================

// Method npoints_in_tdpfile takes in a tdp file's name and returns
// the number of XYZ points which it contains.

   unsigned int npoints_in_tdpfile(string tdp_filename)
      {
         Tdp_file tdp_file;
         if (!tdp_file.file_open(tdp_filename))
         {
            cout << "Unable to open TDP file " << tdp_filename << endl;
            exit(1);
         }

         const int nfloats_per_point=3;
         unsigned int n_points= tdp_file.klv_length( 
            TdpKeyXYZ_POINT_DATA, 0 ) / sizeof(float) / nfloats_per_point;
//         cout << "n_points = " << n_points << endl;

         tdp_file.file_close();
         return n_points;
      }

// ---------------------------------------------------------------------
// Method parse_UTM_info

   bool parse_UTM_info(
      Tdp_file& tdp_file,string& UTMzone,threevector& UTM_offset)
      {
//         cout << "inside tdpfunc::parse_UTM_info()" << endl;
         bool UTMzone_exists=tdp_file.klv_exists(TdpKeyUTMZone,0);

//         cout << "UTMzone_exists = " << UTMzone_exists << endl;
         if (UTMzone_exists)
         {
            char zone[8];
            tdp_file.klv_read(TdpKeyUTMZone,0,(char *) zone,
                              8*sizeof(char));
            UTMzone=string(zone);
//            cout << "UTMzone = " << UTMzone << endl;
         }

         bool UTMoffset_exists=tdp_file.klv_exists(TdpKeyUTMOffset,0);
//         cout << "UTMoffset_exists = " << UTMoffset_exists << endl;
         if (UTMoffset_exists)
         {
            osg::Vec3d offset;
            tdp_file.klv_read(TdpKeyUTMOffset,0,offset.ptr(), 
                              3*sizeof(real64_t));
            UTM_offset=threevector(offset);
//            cout << "UTMoffset = " << UTM_offset << endl;
         }
         return (UTMzone_exists || UTMoffset_exists);
      }

// ---------------------------------------------------------------------
   void compute_extremal_XYZ_points_in_tdpfile(
      string tdp_filename,threevector& XYZ_min,threevector& XYZ_max)
      {
//         cout << "inside tdpfunc::compute_extremal_XYZ_points_in_tdpfile()" 
//              << endl;

         unsigned int n_points=npoints_in_tdpfile(tdp_filename);
//         cout << "n_points = " << n_points << endl;
         int n_iters=get_n_iters(n_points);
//         cout << "n_iters = " << n_iters << endl;
         
         Tdp_file tdp_file;
         if (!tdp_file.file_open(tdp_filename))
         {
            cout << "Unable to open TDP file " << tdp_filename << endl;
            exit(1);
         }

         string UTMzone;
         threevector UTM_offset;
         parse_UTM_info(tdp_file,UTMzone,UTM_offset);
//         cout << "UTM_offset = " << UTM_offset << endl;

         int xyz_byte_offset=0;
         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_read=basic_math::min(max_points_per_iter,n_points);
            compute_extremal_XYZ_values(
               npoints_to_read,tdp_file,xyz_byte_offset,UTM_offset,
               XYZ_min,XYZ_max);
            n_points -= npoints_to_read;
         } // loop over iter index
         
         tdp_file.file_close();
      }

// ---------------------------------------------------------------------
   void compute_extremal_XYZ_values(
      int npoints_to_read,Tdp_file& tdp_file,int& xyz_byte_offset,
      const threevector& UTM_offset,threevector& XYZ_min,threevector& XYZ_max)
      {
         const int nfloats_per_point=3;
         int n_xyz_bytes=npoints_to_read*nfloats_per_point*sizeof(real32_t);
         real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];
         uint64_t klv_index = 0;
         tdp_file.klv_read( TdpKeyXYZ_POINT_DATA, klv_index, rel_xyz_data,
                            n_xyz_bytes,tdp_data,xyz_byte_offset);

         double xmin=XYZ_min.get(0);
         double xmax=XYZ_max.get(0);
         double ymin=XYZ_min.get(1);
         double ymax=XYZ_max.get(1);
         double zmin=XYZ_min.get(2);
         double zmax=XYZ_max.get(2);
         
         for (int n=0; n<npoints_to_read; n++)
         {
            xmin=basic_math::min(xmin,rel_xyz_data[3*n+0]+UTM_offset.get(0));
            xmax=basic_math::max(xmax,rel_xyz_data[3*n+0]+UTM_offset.get(0));
            ymin=basic_math::min(ymin,rel_xyz_data[3*n+1]+UTM_offset.get(1));
            ymax=basic_math::max(ymax,rel_xyz_data[3*n+1]+UTM_offset.get(1));
            zmin=basic_math::min(zmin,rel_xyz_data[3*n+2]+UTM_offset.get(2));
            zmax=basic_math::max(zmax,rel_xyz_data[3*n+2]+UTM_offset.get(2));
         } // loop over index n labeling current iteration's points
            
         delete [] rel_xyz_data;

         XYZ_min=threevector(xmin,ymin,zmin);
         XYZ_max=threevector(xmax,ymax,zmax);
         xyz_byte_offset += n_xyz_bytes;
      }
 
// ---------------------------------------------------------------------
// Method read_curr_XYZ_points() takes in an already opened TDP file
// along with some number of points to read starting at input
// byte_offset location.  It appends the input data to STL vectors X,
// Y and Z:

   void read_curr_XYZ_points(
      int npoints_to_read,Tdp_file& tdp_file,int& byte_offset,
      const threevector& UTM_offset,
      vector<double>& X,vector<double>& Y,vector<double>& Z)
      {
//         cout << "inside tdpfunc::read_curr_XYZ_points()" << endl;
//         cout.precision(10);

         const int nfloats_per_point=3;
         int n_xyz_bytes=npoints_to_read*nfloats_per_point*sizeof(real32_t);
         real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];
         uint64_t klv_index = 0;
         tdp_file.klv_read( TdpKeyXYZ_POINT_DATA, klv_index, rel_xyz_data,
                            n_xyz_bytes,tdp_data,byte_offset);

         for (int n=0; n<npoints_to_read; n++)
         {
            double curr_x=rel_xyz_data[3*n+0]+UTM_offset.get(0);
            double curr_y=rel_xyz_data[3*n+1]+UTM_offset.get(1);
            double curr_z=rel_xyz_data[3*n+2]+UTM_offset.get(2);
            X.push_back(curr_x);
            Y.push_back(curr_y);
            Z.push_back(curr_z);
//            cout << "n = " << n 
//                 << " x = " << curr_x
//                 << " y = " << curr_y
//                 << " z = " << curr_z << endl;
            
         } // loop over index n labeling current iteration's points
            
         delete [] rel_xyz_data;
         byte_offset += n_xyz_bytes;

//         cout << "UTM_offset = " << UTM_offset << endl;
//         outputfunc::enter_continue_char();
      }


// ---------------------------------------------------------------------
// Method read_curr_XYZP_points() takes in an already opened TDP file
// along with some number of points to read starting at input
// byte_offset location.  It appends the input data to STL vectors X,
// Y, Z and P:

   void read_curr_XYZP_points(
      int npoints_to_read,Tdp_file& tdp_file,int& p_byte_offset,
      const threevector& UTM_offset,vector<double>& X,vector<double>& Y,
      vector<double>& Z,vector<double>& P)
   {
//      cout << "inside tdpfunc::read_curr_XYZP_points()" << endl;
      
         const int nfloats_per_point=3;
         int n_xyz_bytes=npoints_to_read*nfloats_per_point*sizeof(real32_t);
         int n_p_bytes=npoints_to_read*sizeof(real32_t);
         real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];
         real32_t* p_data=new real32_t[n_p_bytes];

         int xyz_byte_offset=nfloats_per_point*p_byte_offset;

         uint64_t klv_index = 0;
         tdp_file.klv_read( 
            TdpKeyXYZ_POINT_DATA, klv_index, rel_xyz_data,
            n_xyz_bytes,tdp_data,xyz_byte_offset);
         tdp_file.klv_read( 
            TdpKeyMETADATA_PROBABILITY_OF_DETECTION, klv_index, p_data,
            n_p_bytes,tdp_data,p_byte_offset);
         
         for (int n=0; n<npoints_to_read; n++)
         {
            X.push_back(rel_xyz_data[3*n+0]+UTM_offset.get(0));
            Y.push_back(rel_xyz_data[3*n+1]+UTM_offset.get(1));
            Z.push_back(rel_xyz_data[3*n+2]+UTM_offset.get(2));
            P.push_back(p_data[n]);
//            cout << "n = " << n << " p_data[n] = " << p_data[n] << endl;
         } // loop over index n labeling current iteration's points
            
         delete [] rel_xyz_data;
         delete [] p_data;
         p_byte_offset += n_p_bytes;
      }

// ---------------------------------------------------------------------
// Method read_XYZ_points_from_tdpfile() extracts all XYZ point info
// from the TDP file specified by its filename argument and returns
// the data within STL vectors X, Y and Z.

   void read_XYZ_points_from_tdpfile(
      string tdp_filename,
      vector<double>& X,vector<double>& Y,vector<double>& Z)
      {
         cout << "inside tdpfunc::read_XYZ_points_from_tdpfile()" << endl;
         
         outputfunc::write_banner("Reading XYZ points from TDP file:");

         unsigned int n_points=npoints_in_tdpfile(tdp_filename);
         int n_iters=get_n_iters(n_points);
         
         Tdp_file tdp_file;
         if (!tdp_file.file_open(tdp_filename))
         {
            cout << "Unable to open TDP file " << tdp_filename << endl;
            exit(1);
         }

         string UTMzone;
         threevector UTM_offset;
         parse_UTM_info(tdp_file,UTMzone,UTM_offset);
         cout << "UTM_offset = " << UTM_offset << endl;

         int byte_offset=0;
         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_read=basic_math::min(max_points_per_iter,n_points);
            read_curr_XYZ_points(
               npoints_to_read,tdp_file,byte_offset,UTM_offset,X,Y,Z);
            n_points -= npoints_to_read;
         } // loop over iter index
         
         tdp_file.file_close();
      }

// ---------------------------------------------------------------------
   void read_XYZP_points_from_tdpfile(
      string tdp_filename,
      vector<double>& X,vector<double>& Y,vector<double>& Z,vector<double>& P)
      {
//         cout << "inside tdpfunc::read_XYZP_points_from_tdpfile()" << endl;
         outputfunc::write_banner("Reading XYZP points from TDP file:");

         unsigned int n_points=npoints_in_tdpfile(tdp_filename);
//         cout << "n_points = " << n_points << endl;
         int n_iters=get_n_iters(n_points);
//         cout << "n_iters = " << n_iters << endl;
         
         X.reserve(n_points);
         Y.reserve(n_points);
         Z.reserve(n_points);
         P.reserve(n_points);

         Tdp_file tdp_file;
         if (!tdp_file.file_open(tdp_filename))
         {
            cout << "Unable to open TDP file " << tdp_filename << endl;
            exit(1);
         }

         string UTMzone;
         threevector UTM_offset;
         parse_UTM_info(tdp_file,UTMzone,UTM_offset);
//         cout << "UTM_offset = " << UTM_offset << endl;

         int p_byte_offset=0;
         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_read=basic_math::min(max_points_per_iter,n_points);
//            cout << "iter = " << iter 
//                 << " npoints_to_read = " << npoints_to_read << endl;
            read_curr_XYZP_points(
               npoints_to_read,tdp_file,p_byte_offset,UTM_offset,X,Y,Z,P);
            n_points -= npoints_to_read;
         } // loop over iter index
         
         tdp_file.file_close();
      }

// ---------------------------------------------------------------------
// Method read_XYZRGBA_points_from_tdpfile extracts all XYZ and RGBA
// point info from the TDP file specified by its filename argument and
// returns the data within STL vectors X, Y, Z, R, G and B.

   void read_XYZRGB_points_from_tdpfile(
      string tdp_filename,
      vector<double>& X,vector<double>& Y,vector<double>& Z,
      vector<int>& R,vector<int>& G,vector<int>& B)
      {
         outputfunc::write_banner("Reading XYZRGB points from TDP file:");

         int n_orig_points=npoints_in_tdpfile(tdp_filename);
         unsigned int n_points=n_orig_points;
         int n_iters=get_n_iters(n_points);

         Tdp_file tdp_file;
         if (!tdp_file.file_open(tdp_filename))
         {
            cout << "Unable to open TDP file " << tdp_filename << endl;
            exit(1);
         }

         string UTMzone;
         threevector UTM_offset;
         parse_UTM_info(tdp_file,UTMzone,UTM_offset);
         cout << "UTM_offset = " << UTM_offset << endl;

         int byte_offset=0;
         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_read=basic_math::min(max_points_per_iter,n_points);
            read_curr_XYZ_points(
               npoints_to_read,tdp_file,byte_offset,UTM_offset,X,Y,Z);
            n_points -= npoints_to_read;
         } // loop over iter index

         n_points=n_orig_points;
         osg::Vec4ubArray* colors_ptr=new osg::Vec4ubArray();
         colors_ptr->reserve(n_points);
         parse_pointcloud_color_data(n_points,tdp_file,colors_ptr);

         for (unsigned int n=0; n<colors_ptr->size(); n++)
         {
            osg::Vec4ub curr_RGBA(colors_ptr->at(n));
            R.push_back(stringfunc::unsigned_char_to_ascii_integer(
               curr_RGBA.r()));
            G.push_back(stringfunc::unsigned_char_to_ascii_integer(
               curr_RGBA.g()));
            B.push_back(stringfunc::unsigned_char_to_ascii_integer(
               curr_RGBA.b()));
         }

         tdp_file.file_close();
      }

// ---------------------------------------------------------------------
// Method parse_pointcloud_color_data fills up a dynamically generated STL
// vector with fourvector containing XYZP data read from the input
// .tdp file specified by tdp_filename.  This boolean method returns
// true if the input TDP file contains RGBA color information.

   bool parse_pointcloud_color_data(
      int n_points,Tdp_file& tdp_file,osg::Vec4ubArray* colors)
      {
         cout << "inside tdpfunc::parse_pointcloud_color_data()" << endl;
//         cout << "n_points = " << n_points << endl;

         bool xyz_data_exists=tdp_file.klv_exists(TdpKeyXYZ_POINT_DATA,0);
//         bool pdata_exists=tdp_file.klv_exists(
//            TdpKeyMETADATA_PROBABILITY_OF_DETECTION,0);
//         bool coverage_data_exists=tdp_file.klv_exists(
//            TdpKeyMETADATA_COVERAGE_DENSITY,0);
         bool rgba_data_exists=tdp_file.klv_exists(
            TdpKeyRGBA_COLOR_8,0);

         cout << "xyz_data_exists = " << xyz_data_exists << endl;
//         cout << "pdata_exists = " << pdata_exists << endl;
//         cout << "coverage_data_exists = " << coverage_data_exists << endl;
         cout << "rgba_data_exists = " << rgba_data_exists << endl;

         if (xyz_data_exists && rgba_data_exists)
         {
            const int nchars_per_point=4;
            int n_color_bytes=n_points*nchars_per_point*sizeof(char8_t);
            char8_t* rgba_data=new char8_t[n_color_bytes];
            tdp_file.klv_read( 
               TdpKeyRGBA_COLOR_8,0,rgba_data,n_color_bytes);
            for (int n=0; n<n_points; n++)
            {
               colors->push_back(osg::Vec4ub(
                  static_cast<unsigned char>(rgba_data[4*n+0]),
                  static_cast<unsigned char>(rgba_data[4*n+1]),
                  static_cast<unsigned char>(rgba_data[4*n+2]),
                  static_cast<unsigned char>(rgba_data[4*n+3])));
            } // loop over index n labeling xyz points

         } // xyz_data_exists && RGBA_data_exists conditional
         return rgba_data_exists;
      }

// ==========================================================================
// TwoDarray input methods
// ==========================================================================

// Method generate_ztwoDarray_from_tdpfile first scans over all XYZ
// points within an input tdp file and determines their extremal
// extents.  It then dynamically instantiates a twoDarray and fills
// its contents with Z=Z(X,Y).  A pointer to this height array is
// returned by this method.

   twoDarray* generate_ztwoDarray_from_tdpfile(
      string tdp_filename,double delta_x,double delta_y)
      {
         outputfunc::write_banner("Generating ztwoDarray from TDP file:");

         unsigned int n_points= npoints_in_tdpfile(tdp_filename);
         int n_iters=get_n_iters(n_points);
         cout << "n_points = " << n_points << " n_iters = " << n_iters << endl;

         Tdp_file tdp_file;
         if (!tdp_file.file_open(tdp_filename))
         {
            cout << "Unable to open TDP file " << tdp_filename << endl;
            exit(1);
         }

         string UTMzone;
         threevector UTM_offset;
         parse_UTM_info(tdp_file,UTMzone,UTM_offset);
         cout << "UTM_offset = " << UTM_offset << endl;

// First read through data and determine extremal XYZ extents:

         int byte_offset=0;
         double min_x=POSITIVEINFINITY;
         double max_x=NEGATIVEINFINITY;
         double min_y=POSITIVEINFINITY;
         double max_y=NEGATIVEINFINITY;         
         double min_z=POSITIVEINFINITY;
         double max_z=NEGATIVEINFINITY;         

         vector<double> X,Y,Z;
         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_read=basic_math::min(max_points_per_iter,n_points);
            read_curr_XYZ_points(
               npoints_to_read,tdp_file,byte_offset,UTM_offset,X,Y,Z);

            for (int n=0; n<npoints_to_read; n++)
            {
               min_x=basic_math::min(min_x,X[n]);
               max_x=basic_math::max(max_x,X[n]);
               min_y=basic_math::min(min_y,Y[n]);
               max_y=basic_math::max(max_y,Y[n]);
               min_z=basic_math::min(min_z,Z[n]);
               max_z=basic_math::max(max_z,Z[n]);
            } // loop over index n labeling current iteration's points
            X.clear();
            Y.clear();
            Z.clear();
          
            n_points -= npoints_to_read;
         } // loop over iter index

         cout << "min_x = " << min_x << " max_x = " << max_x << endl;
         cout << "min_y = " << min_y << " max_y = " << max_y << endl;
         cout << "min_z = " << min_z << " max_z = " << max_z << endl;

// Instantiate *ztwoDarray_ptr:
         
         int mdim=basic_math::round((max_x-min_x)/delta_x)+1;
         int ndim=basic_math::round((max_y-min_y)/delta_y)+1;
         cout << "mdim = " << mdim << " ndim = " << ndim << endl;

         twoDarray* ztwoDarray_ptr=new twoDarray(mdim,ndim);
         ztwoDarray_ptr->set_deltax(delta_x);
         ztwoDarray_ptr->set_xlo(min_x);
         ztwoDarray_ptr->set_xhi(min_x+(mdim-1)*delta_x);
         ztwoDarray_ptr->set_deltay(delta_y);
         ztwoDarray_ptr->set_ylo(min_y);
         ztwoDarray_ptr->set_yhi(min_y+(ndim-1)*delta_y);

         ztwoDarray_ptr->initialize_values(xyzpfunc::null_value);

         fill_ztwoDarray_from_tdpfile(tdp_filename,ztwoDarray_ptr);
         
         return ztwoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method fill_ztwoDarray_from_tdpfile fills the contents of twoDarray
// *ztwoDarray_ptr with data read in from the specified input TDP
// file.

   void fill_ztwoDarray_from_tdpfile(
      string tdp_filename,twoDarray* ztwoDarray_ptr)
      {
         outputfunc::write_banner("Filling ztwoDarray from TDP file:");

         unsigned int n_points=npoints_in_tdpfile(tdp_filename);
         int n_iters=get_n_iters(n_points);

         cout << "n_points = " << n_points << " n_iters = " << n_iters << endl;
         
         Tdp_file tdp_file;
         if (!tdp_file.file_open(tdp_filename))
         {
            cout << "Unable to open TDP file " << tdp_filename << endl;
            exit(1);
         }

         string UTMzone;
         threevector UTM_offset;
         parse_UTM_info(tdp_file,UTMzone,UTM_offset);
//         cout << "UTM_offset = " << UTM_offset << endl;

// Now read through data again and extract XYZ points:

         uint64_t klv_index = 0;
         const int nfloats_per_point=3;
         unsigned int px,py,n_coincidence=0;
         int byte_offset=0;
         n_points= tdp_file.klv_length( 
            TdpKeyXYZ_POINT_DATA, 0 ) / sizeof(float) / nfloats_per_point;
         n_iters=get_n_iters(n_points);
         for (int iter=0; iter<n_iters; iter++)
         {
//            cout << iter << " " << flush;
            int npoints_to_read=basic_math::min(max_points_per_iter,n_points);
            int n_xyz_bytes=npoints_to_read*nfloats_per_point*
               sizeof(real32_t);
            real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];
            tdp_file.klv_read( TdpKeyXYZ_POINT_DATA, klv_index, rel_xyz_data,
                               n_xyz_bytes,tdp_data,byte_offset);

            for (int n=0; n<npoints_to_read; n++)
            {
               double curr_x=rel_xyz_data[3*n+0]+UTM_offset.get(0);
               double curr_y=rel_xyz_data[3*n+1]+UTM_offset.get(1);
               double curr_z=rel_xyz_data[3*n+2]+UTM_offset.get(2);

// Note added on Sat, Jan 9, 2010 at 1:05 pm:

// After several hours of debugging, we learned the hard way that the
// raw DTED tiles e39n28 and e39n29 are totally corrupted!  So we
// decided to approximate the heights of these tiles with a reasonable
// constant value of 676 meters:

//               curr_z=676;
               
               if (ztwoDarray_ptr->point_to_pixel(curr_x,curr_y,px,py))
               {
                  if (ztwoDarray_ptr->get(px,py) > 0) n_coincidence++;

// Since delta_x & delta_y spacing in ztwoDarray is generally coarser
// than that in the cloud's *vertices_ptr, multiple voxels are mapped
// to a single pixel.  We take the pixel's z value to equal the
// maximum of the voxels'.

                  ztwoDarray_ptr->put(
                     px,py,basic_math::max(curr_z,ztwoDarray_ptr->get(px,py)));

//                  cout << "px = " << px << " py = " << py
//                       << " ztwoDarray = " << ztwoDarray_ptr->get(px,py) 
//                       << endl;
               }
            } // loop over index n labeling current iteration's points
            
            delete [] rel_xyz_data;
            n_points -= npoints_to_read;
            byte_offset += n_xyz_bytes;
         } // loop over iter index
//         cout << endl;
         cout << "n_coincidences = " << n_coincidence << endl;

         tdp_file.file_close();
      }

// ---------------------------------------------------------------------
// Method generate_ztwoDarray_and_ptwoDarray_from_tdpfile first scans
// over all XYZP points within an input tdp file and determines their
// extremal extents.  It then dynamically instantiates 2 twoDarrays
// and fills their contents with Z=Z(X,Y) and P=P(X,Y).  Pointers to
// the height and probability arrays are returned by this method.

   pair<twoDarray*,twoDarray*> 
      generate_ztwoDarray_and_ptwoDarray_from_tdpfile(
      string tdp_filename,double delta_x,double delta_y)
      {
         outputfunc::write_banner(
            "Generating ztwoDarray and ptwoDarray from TDP file:");
         
         unsigned int n_points=npoints_in_tdpfile(tdp_filename);
         int n_iters=get_n_iters(n_points);

         Tdp_file tdp_file;
         if (!tdp_file.file_open(tdp_filename))
         {
            cout << "Unable to open TDP file " << tdp_filename << endl;
            exit(1);
         }

         string UTMzone;
         threevector UTM_offset;
         parse_UTM_info(tdp_file,UTMzone,UTM_offset);
         cout << "UTM_offset = " << UTM_offset << endl;

// First read through data and determine number of points in TDP file
// plus data's extremal XYZP extents:

         const int nfloats_per_point=3;
         int xyz_byte_offset=0;
         int p_byte_offset=0;
         double min_x=POSITIVEINFINITY;
         double max_x=NEGATIVEINFINITY;
         double min_y=POSITIVEINFINITY;
         double max_y=NEGATIVEINFINITY;         
         double min_z=POSITIVEINFINITY;
         double max_z=NEGATIVEINFINITY;         
         double min_p=POSITIVEINFINITY;
         double max_p=NEGATIVEINFINITY;         

         uint64_t klv_index = 0;
         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_read=basic_math::min(max_points_per_iter,n_points);
            int n_xyz_bytes=npoints_to_read*nfloats_per_point*
               sizeof(real32_t);
            int n_p_bytes=npoints_to_read*sizeof(real32_t);
            real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];
            real32_t* p_data=new real32_t[n_p_bytes];
            tdp_file.klv_read( 
               TdpKeyXYZ_POINT_DATA, klv_index, rel_xyz_data,
               n_xyz_bytes,tdp_data,xyz_byte_offset);
            tdp_file.klv_read( 
               TdpKeyMETADATA_PROBABILITY_OF_DETECTION, klv_index, p_data,
               n_p_bytes,tdp_data,p_byte_offset);

            for (int n=0; n<npoints_to_read; n++)
            {
               double curr_x=rel_xyz_data[3*n+0]+UTM_offset.get(0);
               double curr_y=rel_xyz_data[3*n+1]+UTM_offset.get(1);
               double curr_z=rel_xyz_data[3*n+2]+UTM_offset.get(2);
               double curr_p=p_data[n];
               min_x=basic_math::min(min_x,curr_x);
               max_x=basic_math::max(max_x,curr_x);
               min_y=basic_math::min(min_y,curr_y);
               max_y=basic_math::max(max_y,curr_y);
               min_z=basic_math::min(min_z,curr_z);
               max_z=basic_math::max(max_z,curr_z);
               min_p=basic_math::min(min_p,curr_p);
               max_p=basic_math::max(max_p,curr_p);
            } // loop over index n labeling current iteration's points
            
            delete [] rel_xyz_data;
            delete [] p_data;
            n_points -= npoints_to_read;
            xyz_byte_offset += n_xyz_bytes;
            p_byte_offset += n_p_bytes;
         } // loop over iter index

         cout << "min_x = " << min_x << " max_x = " << max_x << endl;
         cout << "min_y = " << min_y << " max_y = " << max_y << endl;
         cout << "min_z = " << min_z << " max_z = " << max_z << endl;
         cout << "min_p = " << min_p << " max_p = " << max_p << endl;

// Instantiate ztwoDarray_ptr & ptwoDarray_ptr:
         
         int mdim=basic_math::round((max_x-min_x)/delta_x)+1;
         int ndim=basic_math::round((max_y-min_y)/delta_y)+1;
         cout << "mdim = " << mdim << " ndim = " << ndim << endl;

         twoDarray* ztwoDarray_ptr=new twoDarray(mdim,ndim);
         ztwoDarray_ptr->set_deltax(delta_x);
         ztwoDarray_ptr->set_xlo(min_x);
         ztwoDarray_ptr->set_xhi(min_x+(mdim-1)*delta_x);
         ztwoDarray_ptr->set_deltay(delta_y);
         ztwoDarray_ptr->set_ylo(min_y);
         ztwoDarray_ptr->set_yhi(min_y+(ndim-1)*delta_y);
         ztwoDarray_ptr->initialize_values(0);

         twoDarray* ptwoDarray_ptr=new twoDarray(mdim,ndim);
         ptwoDarray_ptr->set_deltax(delta_x);
         ptwoDarray_ptr->set_xlo(min_x);
         ptwoDarray_ptr->set_xhi(min_x+(mdim-1)*delta_x);
         ptwoDarray_ptr->set_deltay(delta_y);
         ptwoDarray_ptr->set_ylo(min_y);
         ptwoDarray_ptr->set_yhi(min_y+(ndim-1)*delta_y);
         ptwoDarray_ptr->initialize_values(-1);

// Now read through data again and extract XYZP points:

         unsigned int px,py,n_coincidence=0;
         xyz_byte_offset=p_byte_offset=0;
         n_points=tdp_file.klv_length( 
            TdpKeyXYZ_POINT_DATA, 0 ) / sizeof(float) / nfloats_per_point;
         n_iters=get_n_iters(n_points);
         for (int iter=0; iter<n_iters; iter++)
         {
//            cout << iter << " " << flush;
            int npoints_to_read=basic_math::min(max_points_per_iter,n_points);
            int n_xyz_bytes=npoints_to_read*nfloats_per_point*
               sizeof(real32_t);
            int n_p_bytes=npoints_to_read*sizeof(real32_t);
            real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];
            real32_t* p_data=new real32_t[n_p_bytes];
            tdp_file.klv_read( 
               TdpKeyXYZ_POINT_DATA, klv_index, rel_xyz_data,
               n_xyz_bytes,tdp_data,xyz_byte_offset);
            tdp_file.klv_read( 
               TdpKeyMETADATA_PROBABILITY_OF_DETECTION, klv_index, p_data,
               n_p_bytes,tdp_data,p_byte_offset);

            for (int n=0; n<npoints_to_read; n++)
            {
               double curr_x=rel_xyz_data[3*n+0]+UTM_offset.get(0);
               double curr_y=rel_xyz_data[3*n+1]+UTM_offset.get(1);
               double curr_z=rel_xyz_data[3*n+2]+UTM_offset.get(2);
               double curr_p=p_data[n];
               
               if (ztwoDarray_ptr->point_to_pixel(curr_x,curr_y,px,py))
               {
                  if (ztwoDarray_ptr->get(px,py) > 0) n_coincidence++;

// Since delta_x & delta_y spacing in ztwoDarray is generally coarser
// than that in the cloud's *vertices_ptr, multiple voxels are mapped
// to a single pixel.  We take the pixel's z value to equal the
// maximum of the voxels'.

                  ztwoDarray_ptr->put(
                     px,py,basic_math::min(curr_z,ztwoDarray_ptr->get(px,py)));
                  ptwoDarray_ptr->put(
                     px,py,basic_math::min(
                        curr_p,ptwoDarray_ptr->get(px,py)));
               }
            } // loop over index n labeling current iteration's points
            
            delete [] rel_xyz_data;
            delete [] p_data;
            n_points -= npoints_to_read;
            xyz_byte_offset += n_xyz_bytes;
            p_byte_offset += n_p_bytes;
         } // loop over iter index
//         cout << endl;
         cout << "n_coincidences = " << n_coincidence << endl;

         tdp_file.file_close();

         return pair<twoDarray*,twoDarray*>(ztwoDarray_ptr,ptwoDarray_ptr);
      }

// ==========================================================================
// TDP file output initialiation methods
// ==========================================================================

// Method get_n_iters() returns the number of iterations which need to
// be performed in order for the TDP library to successfully write out
// all n_points to file:

   int get_n_iters(int n_points)
      {
         int n_iters=n_points/max_points_per_iter+1;
//         cout << "n_iters = " << n_iters << endl;
         return n_iters;
      }

// ---------------------------------------------------------------------
   void initialize_output_tdpfile(
      string tdp_filename,string UTMzone,const osg::Vec3Array* vertices_ptr,
      Tdp_file& tdp_file,osg::Vec3& zeroth_xyz)
      {
//         cout << "inside tdpfunc::initialize_output_tdpfile()" << endl;
//         cout << "max_points_per_iter = " << max_points_per_iter << endl;
         
         tdp_file.file_open( tdp_filename, "w+b" );
         zeroth_xyz=osg::Vec3(vertices_ptr->at(0));
         write_UTM_zone_and_offset(tdp_file,UTMzone,threevector(zeroth_xyz));
      }

// ---------------------------------------------------------------------
// We wrote this next overloaded version of initialize_output_tdpfile
// to take in zeroth_xyz with double precision. 

   void initialize_output_tdpfile(
      string tdp_filename,string UTMzone,
      Tdp_file& tdp_file,const threevector& zeroth_xyz)
      {
//         cout << "inside tdpfunc::initialize_output_tdpfile()" << endl;
         
         tdp_file.file_open( tdp_filename, "w+b" );
         write_UTM_zone_and_offset(tdp_file,UTMzone,zeroth_xyz);
      }

// ---------------------------------------------------------------------
// Method write_UTM_zone_and_offset first converts input UTMzone
// string into an 8 byte character array and stores it within the TDP
// file header.  It also saves the double precision entries within
// zeroth_XYZ_vertex into the TDP header.

   void write_UTM_zone_and_offset(Tdp_file& tdp_file,string UTMzone,
                                  const threevector& zeroth_XYZ)
      {
//         cout << "inside tdpfunc::write_UTM_zone_and_offset()" << endl;

// On 7/24/06, we definitely observed bad quantization effects from
// setting all UTMoffsets to zero and storing full easting and
// northing information into OSG vertices within the scenegraph.  So
// we have no choice but to store residual eastings and northings
// relative to some origin for each tile...

//         cout << "zeroth_XYZ = " << zeroth_XYZ << endl;

         double UTMOffset[3];
         UTMOffset[0]=zeroth_XYZ.get(0);
         UTMOffset[1]=zeroth_XYZ.get(1);
         UTMOffset[2]=zeroth_XYZ.get(2);
         tdp_file.klv_create_and_write(
            TdpKeyUTMOffset,UTMOffset,3*sizeof(real64_t));

         if (UTMzone=="")
         {
//            cout << "Warning in tdpfunc::write_UTM_zone_and_offset()" << endl;
//            cout << "Input UTMzone has no value !!!" << endl;
            return;
         }
         char* cstr_ptr=const_cast<char*>(UTMzone.c_str());
         char* char_array_ptr=new char[8];
         for (unsigned int i=0; i<basic_math::min(size_t(8),UTMzone.size()); 
              i++)
         {
            char_array_ptr[i]=cstr_ptr[i];
         }
         tdp_file.klv_create_and_write(TdpKeyUTMZone,char_array_ptr,8);
         delete [] char_array_ptr;
      }

// ==========================================================================
// Relative vertex output methods
// ==========================================================================

// Method write_relative_xyz_data() takes in osg::Vec3Array
// *vertices_ptr.  It takes the zeroth point in this array as the
// origin for all others.  The origin's X, Y and Z values are saved
// within the output TDP file's header.  The input UTMzone information
// is also saved in the header output.  All other point's X, Y and Z
// displacements relative to the origin are saved as real_32s.

   void write_relative_xyz_data(
      string tdp_filename,string UTMzone,const osg::Vec3Array* vertices_ptr)
      {
//         cout << "inside tdpfunc::write_relative_xyz_data() #1" << endl;
         
         osg::Vec3 zeroth_xyz;
         Tdp_file tdp_file;
         initialize_output_tdpfile(
            tdp_filename,UTMzone,vertices_ptr,tdp_file,zeroth_xyz);
         
         write_relative_xyz_data(zeroth_xyz,tdp_file,vertices_ptr);
      }

   void write_relative_xyz_data(
      string tdp_filename,string UTMzone,
      const osg::Vec3& specified_origin,const osg::Vec3Array* vertices_ptr)
      {
//         cout << "inside tdpfunc::write_relative_xyz_data #2()" << endl;
         
         osg::Vec3 zeroth_xyz;
         Tdp_file tdp_file;
         initialize_output_tdpfile(
            tdp_filename,UTMzone,vertices_ptr,tdp_file,zeroth_xyz);

         write_relative_xyz_data(specified_origin,tdp_file,vertices_ptr);
      }

   void write_relative_xyz_data(
      const osg::Vec3& origin,Tdp_file& tdp_file,
      const osg::Vec3Array* vertices_ptr)
      {
//         cout << "inside tdpfunc::write_relative_xyz_data() #3" << endl;
         
         const int nfloats_per_point=3;
         unsigned int n_points=vertices_ptr->size();
         int n_iters=get_n_iters(n_points);
         int point_offset=0;
         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_write=basic_math::min(max_points_per_iter,n_points);
            cout << "iter = " << iter
                 << " npoints_to_write = " << npoints_to_write << endl;
            int n_xyz_bytes=npoints_to_write*nfloats_per_point*
               sizeof(real32_t);
            real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];

// Write valid XYZ points relative to origin as floats to TDP file:

            for (int i=0; i<npoints_to_write; i++)
            {
               osg::Vec3 curr_xyz( vertices_ptr->at(i+point_offset) );
               if (curr_xyz.z() > 0.5*NEGATIVEINFINITY)
               {
                  rel_xyz_data[3*i+0]=curr_xyz.x()-origin.x();
                  rel_xyz_data[3*i+1]=curr_xyz.y()-origin.y();
                  rel_xyz_data[3*i+2]=curr_xyz.z()-origin.z();
               }

               if (i < 20)
               {
                  cout << "i = " << i 
                       << " curr_xyz.x() = " << curr_xyz.x()
                       << " origin.x() = " << origin.x() << endl;
                  cout << "  curr_xyz.y() = " << curr_xyz.y()
                       << " origin.y() = " << origin.y() << endl;
               }
               
            }

            if (iter==0)
            {
               tdp_file.klv_create_and_write( 
                  TdpKeyXYZ_POINT_DATA,rel_xyz_data,n_xyz_bytes);
            }
            else
            {
               const uint64_t klv_index = 0;
               tdp_file.klv_append( 
                  TdpKeyXYZ_POINT_DATA,klv_index,rel_xyz_data,n_xyz_bytes);
            }

            delete [] rel_xyz_data;
            n_points -= npoints_to_write;
            point_offset += npoints_to_write;
         }

         tdp_file.file_close();
      }

// ---------------------------------------------------------------------
// This overloaded version of write_relative_xyz_data() imports XYZ
// information from input STL vectors.  After converting the input
// data to osg::Vec3Array, it calls the first
// write_relative_xyz_data() method.

   void write_relative_xyz_data(
      string tdp_filename,
      const vector<double>& X,const vector<double>& Y,const vector<double>& Z)
      {
//         cout << "inside tdpfunc::write_relative_xyz_data() #4" << endl;
         
         unsigned int n_points=X.size();
         threevector zeroth_xyz(X[0],Y[0],Z[0]);
         cout << "n_points = " << n_points << endl;
         cout << "zeroth_xyz = " << zeroth_xyz << endl;

// Recall that osg::Vec3Array can only hold 4-byte floats and not
// 8-byte doubles.  So we need to store relative and not absolute XYZ
// vertex coordinates within Vec3 objects!

         osg::Vec3Array* rel_vertices_ptr=new osg::Vec3Array();
         rel_vertices_ptr->reserve(n_points);

         osg::Vec3 rel_XYZ;
         for (unsigned int i=0; i<n_points; i++)
         {
            rel_XYZ[0]=X[i]-zeroth_xyz.get(0);
            rel_XYZ[1]=Y[i]-zeroth_xyz.get(1);
            rel_XYZ[2]=Z[i]-zeroth_xyz.get(2);
            rel_vertices_ptr->push_back(rel_XYZ);

            if (i < 10)
            {
               cout << "i = " << i 
                    << " rel_X = " << rel_XYZ[0]
                    << " rel_Y = " << rel_XYZ[1]
                    << " rel_Z = " << rel_XYZ[2]
                    << endl;
            }
            
         }

         osg::Vec3 origin(zeroth_xyz.get(0),zeroth_xyz.get(1),
         zeroth_xyz.get(2));
         string UTMzone="";
         write_relative_xyz_data(tdp_filename,UTMzone,origin,rel_vertices_ptr);
      }

// ---------------------------------------------------------------------
// This next version of write_relative_xyz_data() should theoretically
// work with double precision.

   void write_relative_xyz_data(
      string tdp_filename,const vector<threevector>& vertices)
      {
//         cout << "inside tdpfunc::write_relative_xyz_data() #5" << endl;
//         cout << "vertices.size() = " << vertices.size() << endl;

         if (vertices.size()==0)
         {
            cout << "Error in tdpfunc::write_relative_xyz_data()" << endl;
            cout << "vertices.size() = 0 !!!" << endl;
            exit(-1);
         }

         threevector zeroth_xyz=vertices.front();
         write_relative_xyz_data(tdp_filename,zeroth_xyz,vertices);
      }

   void write_relative_xyz_data(
      string tdp_filename,const threevector& origin,
      const vector<threevector>& vertices)
      {
//         cout << "inside tdpfunc::write_relative_xyz_data() #5" << endl;
         string UTMzone="";
         Tdp_file tdp_file;
         initialize_output_tdpfile(tdp_filename,UTMzone,tdp_file,origin);
         write_relative_xyz_data(origin,tdp_file,vertices);
      }
   
// ---------------------------------------------------------------------
   void write_relative_xyz_data(
      const threevector& origin,Tdp_file& tdp_file,
      const vector<threevector>& vertices)
      {
//         cout << "inside tdpfunc::write_relative_xyz_data() #6" << endl;
         
         const int nfloats_per_point=3;
         unsigned int n_points=vertices.size();
         int n_iters=get_n_iters(n_points);
         int point_offset=0;
         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_write=basic_math::min(max_points_per_iter,n_points);
//            cout << "iter = " << iter
//                 << " npoints_to_write = " << npoints_to_write << endl;
            int n_xyz_bytes=npoints_to_write*nfloats_per_point*
               sizeof(real32_t);
            real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];

// Write valid XYZ points relative to origin as floats to TDP file:

            for (int i=0; i<npoints_to_write; i++)
            {
               threevector curr_XYZ(vertices.at(i+point_offset));
               if (curr_XYZ.get(2) > 0.5*NEGATIVEINFINITY)
               {
                  rel_xyz_data[3*i+0]=curr_XYZ.get(0)-origin.get(0);
                  rel_xyz_data[3*i+1]=curr_XYZ.get(1)-origin.get(1);
                  rel_xyz_data[3*i+2]=curr_XYZ.get(2)-origin.get(2);
               }

//               if (i < 20)
//               {
//                  cout << "i = " << i 
//                       << " curr_XYZ.x = " << curr_XYZ.get(0)
//                       << " origin.x() = " << origin.get(0) << endl;
//                  cout << "  curr_XYZ.y() = " << curr_XYZ.get(1)
//                       << " origin.y() = " << origin.get(1) << endl;
//               }
            }

            if (iter==0)
            {
               tdp_file.klv_create_and_write( 
                  TdpKeyXYZ_POINT_DATA,rel_xyz_data,n_xyz_bytes);
            }
            else
            {
               const uint64_t klv_index = 0;
               tdp_file.klv_append( 
                  TdpKeyXYZ_POINT_DATA,klv_index,rel_xyz_data,n_xyz_bytes);
            }

            delete [] rel_xyz_data;
            n_points -= npoints_to_write;
            point_offset += npoints_to_write;
         }

         tdp_file.file_close();
      }

// ---------------------------------------------------------------------
// Method write_relative_xyzp_data takes in osg::Vec3Array
// *vertices_ptr and model::Metadata *metadata_ptr.  It takes the
// zeroth point in *vertices_ptr as the spatial origin for all others.
// The origin's X, Y and Z values are saved within the output TDP
// file's header.  The input UTMzone information is also saved in the
// header output.  All other point's X, Y and Z displacements relative
// to the origin are saved as real_32s.  P values are also written out
// to the TDP file.

   void write_relative_xyzp_data(
      string tdp_filename,string UTMzone,const osg::Vec3Array* vertices_ptr,
      const model::Metadata* metadata_ptr)
      {
         cout << "inside tdpfunc::write_relative_xyzp_data()" << endl;

         osg::Vec3 zeroth_xyz;
         Tdp_file tdp_file;
         initialize_output_tdpfile(
            tdp_filename,UTMzone,vertices_ptr,tdp_file,zeroth_xyz);

         const int nfloats_per_point=3;
         unsigned int n_points=vertices_ptr->size();
         int n_iters=get_n_iters(n_points);
         int point_offset=0;
         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_write=basic_math::min(max_points_per_iter,n_points);
            cout << "iter = " << iter
                 << " npoints_to_write = " << npoints_to_write << endl;
            int n_xyz_bytes=npoints_to_write*nfloats_per_point*
               sizeof(real32_t);
            real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];

// Write valid XYZ points relative to origin as floats to TDP file:

            for (int i=0; i<npoints_to_write; i++)
            {
               osg::Vec3 curr_xyz( vertices_ptr->at(i+point_offset) );

               if (curr_xyz.z() > 0.5*NEGATIVEINFINITY)
               {
                  rel_xyz_data[3*i+0]=curr_xyz.x()-zeroth_xyz.x();
                  rel_xyz_data[3*i+1]=curr_xyz.y()-zeroth_xyz.y();
                  rel_xyz_data[3*i+2]=curr_xyz.z()-zeroth_xyz.z();
               }
               else
               {
                  rel_xyz_data[3*i+0]=0;
                  rel_xyz_data[3*i+1]=0;
                  rel_xyz_data[3*i+2]=0;
               }
            }

            if (iter==0)
            {
               tdp_file.klv_create_and_write( 
                  TdpKeyXYZ_POINT_DATA,rel_xyz_data,n_xyz_bytes);
            }
            else
            {
               uint64_t klv_index = 0;
               tdp_file.klv_append( 
                  TdpKeyXYZ_POINT_DATA,klv_index,rel_xyz_data,n_xyz_bytes);
            }

            delete [] rel_xyz_data;
            n_points -= npoints_to_write;
            point_offset += npoints_to_write;
         } // loop over iter index

// Next iteratively write out P-data:

         point_offset=0;
         n_points=vertices_ptr->size();
         n_iters=get_n_iters(n_points);
         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_write=basic_math::min(max_points_per_iter,n_points);
            int n_p_bytes=npoints_to_write*sizeof(real32_t);
            real32_t* p_data=new real32_t[n_p_bytes];

            for (int i=0; i<npoints_to_write; i++)
            {
               osg::Vec3 curr_xyz( vertices_ptr->at(i+point_offset) );
               if (curr_xyz.z() > 0.5*NEGATIVEINFINITY)
               {

// In mid April 2007, we empirically observed that raw ALIRT-A [RTV]
// Manhattan p-values effectively ranged from 0 to 5.0 [50.0] .  So
// we set an upper threshold on the raw p-values and renormalize them
// so that they range from 0 to 1.0:

                  double max_raw_p_value=1.0;	// properly normalized p-data
//                  double max_raw_p_value=5.0;	// ALIRT-A NYC data
//                  double max_raw_p_value=50.0;	// RTV NYC data

                  double false_p=metadata_ptr->get(i+point_offset,0);
                  false_p=basic_math::min(max_raw_p_value,false_p);
                  p_data[i]=false_p/max_raw_p_value;

//                  p_data[i]=metadata_ptr->get(i,0);

//                  cout << "i = " << i 
//                       << " false_p = " << false_p
//                       << " p = " << p_data[i] << endl;
               }
               else
               {
                  p_data[i]=0;
               }
            } // loop over index i labeling vertices

            if (iter==0)
            {
               tdp_file.klv_create_and_write( 
                  TdpKeyMETADATA_PROBABILITY_OF_DETECTION,p_data,n_p_bytes);
            }
            else
            {
               uint64_t klv_index = 0;
               tdp_file.klv_append( 
                  TdpKeyMETADATA_PROBABILITY_OF_DETECTION,klv_index,
                  p_data,n_p_bytes);
            }

            delete [] p_data;
            n_points -= npoints_to_write;
            point_offset += npoints_to_write;
         } // loop over iter index

         tdp_file.file_close();
      }

// ---------------------------------------------------------------------
// This overloaded version of write_relative_xyzp_data is a minor
// variant of the preceding method.  It takes in a Vec3Array
// containing XYZ points and a counterpart FloatArray containing P
// data.  

   void write_relative_xyzp_data(
      string tdp_filename,string UTMzone,const osg::Vec3Array* vertices_ptr,
      const osg::FloatArray* probs_ptr)
      {
//         cout << "inside tdpfunc::write_relative_xyzp_data()" << endl;

         osg::Vec3 zeroth_xyz;
         Tdp_file tdp_file;
         initialize_output_tdpfile(
            tdp_filename,UTMzone,vertices_ptr,tdp_file,zeroth_xyz);

         const int nfloats_per_point=3;
         unsigned int n_points=vertices_ptr->size();
         int n_iters=get_n_iters(n_points);
         int point_offset=0;
         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_write=basic_math::min(max_points_per_iter,n_points);
            cout << "iter = " << iter
                 << " npoints_to_write = " << npoints_to_write << endl;
            int n_xyz_bytes=npoints_to_write*nfloats_per_point*
               sizeof(real32_t);
            real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];

// Write valid XYZ points relative to origin as floats to TDP file:

            for (int i=0; i<npoints_to_write; i++)
            {
               osg::Vec3 curr_xyz( vertices_ptr->at(i+point_offset) );
               if (curr_xyz.z() > 0.5*NEGATIVEINFINITY)
               {
                  rel_xyz_data[3*i+0]=curr_xyz.x()-zeroth_xyz.x();
                  rel_xyz_data[3*i+1]=curr_xyz.y()-zeroth_xyz.y();
                  rel_xyz_data[3*i+2]=curr_xyz.z()-zeroth_xyz.z();
               }
               else
               {
                  rel_xyz_data[3*i+0]=0;
                  rel_xyz_data[3*i+1]=0;
                  rel_xyz_data[3*i+2]=0;
               }
            }

            if (iter==0)
            {
               tdp_file.klv_create_and_write( 
                  TdpKeyXYZ_POINT_DATA,rel_xyz_data,n_xyz_bytes);
            }
            else
            {
               uint64_t klv_index = 0;
               tdp_file.klv_append( 
                  TdpKeyXYZ_POINT_DATA,klv_index,rel_xyz_data,n_xyz_bytes);
            }

            delete [] rel_xyz_data;
            n_points -= npoints_to_write;
            point_offset += npoints_to_write;
         } // loop over iter index

// Next iteratively write out P-data:

         point_offset=0;
         n_points=vertices_ptr->size();
         n_iters=get_n_iters(n_points);
         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_write=basic_math::min(max_points_per_iter,n_points);
            int n_p_bytes=npoints_to_write*sizeof(real32_t);
            real32_t* p_data=new real32_t[n_p_bytes];

            for (int i=0; i<npoints_to_write; i++)
            {
               osg::Vec3 curr_xyz( vertices_ptr->at(i+point_offset) );
               if (curr_xyz.z() > 0.5*NEGATIVEINFINITY)
               {

// In mid April 2007, we empirically observed that raw ALIRT-A [RTV]
// Manhattan p-values effectively ranged from 0 to 5.0 [50.0] .  So
// we set an upper threshold on the raw p-values and renormalize them
// so that they range from 0 to 1.0:

                  double max_raw_p_value=1.0;	// properly normalized p-data
//                  double max_raw_p_value=5.0;	// ALIRT-A NYC data
//                  double max_raw_p_value=50.0;	// RTV NYC data

                  double false_p=probs_ptr->at(i+point_offset);
                  false_p=basic_math::min(max_raw_p_value,false_p);
                  p_data[i]=false_p/max_raw_p_value;

//                  cout << "i = " << i 
//                       << " false_p = " << false_p
//                       << " p = " << p_data[i] << endl;
               }
               else
               {
                  p_data[i]=0;
               }
            } // loop over index i labeling vertices

            if (iter==0)
            {
               tdp_file.klv_create_and_write( 
                  TdpKeyMETADATA_PROBABILITY_OF_DETECTION,p_data,n_p_bytes);
            }
            else
            {
               uint64_t klv_index = 0;
               tdp_file.klv_append( 
                  TdpKeyMETADATA_PROBABILITY_OF_DETECTION,klv_index,
                  p_data,n_p_bytes);
            }

            delete [] p_data;
            n_points -= npoints_to_write;
            point_offset += npoints_to_write;
         } // loop over iter index

         tdp_file.file_close();
      }

// ==========================================================================
// Relative XYZRGBA output methods
// ==========================================================================

// Method write_relative_xyzrgba_data takes in osg::Vec3Array
// *vertices_ptr and osg::Vec4ubArray *colors_ptr.  It takes the
// zeroth point in *vertices_ptr as the spatial origin for all others.
// The origin's X, Y and Z values are saved within the output TDP
// file's header.  The input UTMzone information is also saved in the
// header output.  All other point's X, Y and Z displacements relative
// to the origin are saved as real_32s.  RGBA values are also written out
// to the TDP file.

   void write_relative_xyzrgba_data(
      string tdp_filename,string UTMzone,const osg::Vec3Array* vertices_ptr,
      const osg::Vec4ubArray* colors_ptr)
      {
//         cout << "inside tdpfunc::write_relative_xyzrgba_data()" << endl;
//         cout << "vertices_ptr->size() = " << vertices_ptr->size() << endl;
//         cout << "colors_ptr->size() = " << colors_ptr->size() << endl;

         osg::Vec3 zeroth_xyz;
         Tdp_file tdp_file;
         initialize_output_tdpfile(
            tdp_filename,UTMzone,vertices_ptr,tdp_file,zeroth_xyz);

// First write XYZ vertices to output tdp file:

         const int nfloats_per_point=3;
         const int nchars_per_point=4;
         unsigned int n_points=vertices_ptr->size();
         int n_iters=get_n_iters(n_points);

         int point_offset=0;
         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_write=basic_math::min(max_points_per_iter,n_points);
//         cout << "iter = " << iter
//              << " npoints_to_write = " << npoints_to_write << endl;
            int n_xyz_bytes=npoints_to_write*nfloats_per_point*
               sizeof(real32_t);
            real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];

            for (int i=0; i<npoints_to_write; i++)
            {
               osg::Vec3 curr_xyz( vertices_ptr->at(i+point_offset) );
               if (curr_xyz.z() > 0.5*NEGATIVEINFINITY)
               {
                  rel_xyz_data[3*i+0]=curr_xyz.x()-zeroth_xyz.x();
                  rel_xyz_data[3*i+1]=curr_xyz.y()-zeroth_xyz.y();
                  rel_xyz_data[3*i+2]=curr_xyz.z()-zeroth_xyz.z();
               }
               else
               {
                  rel_xyz_data[3*i+0]=0;
                  rel_xyz_data[3*i+1]=0;
                  rel_xyz_data[3*i+2]=0;
               }

//               cout << "x = " << curr_xyz.x()
//                    << " y = " << curr_xyz.y() << endl;
//               cout << "rel_x = " << rel_xyz_data[3*i+0] 
//                    << " rel_y = " << rel_xyz_data[3*i+1] << "  " << flush;

            } // loop over index i labeling vertices

            if (iter==0)
            {
               tdp_file.klv_create_and_write( 
                  TdpKeyXYZ_POINT_DATA,rel_xyz_data,n_xyz_bytes);
            }
            else
            {
               uint64_t klv_index = 0;
               tdp_file.klv_append( 
                  TdpKeyXYZ_POINT_DATA,klv_index,rel_xyz_data,n_xyz_bytes);
            }
            
            delete [] rel_xyz_data;
            n_points -= npoints_to_write;
            point_offset += npoints_to_write;

         } // loop over iter index
         
// Next write RGBA color information to output tdp file:

         point_offset=0;
         n_points=vertices_ptr->size();
         n_iters=get_n_iters(n_points);

//         cout << "n_points = " << n_points << endl;
//         cout << "n_iters = " << n_iters << endl;

         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_write=basic_math::min(max_points_per_iter,n_points);
            int n_color_bytes=npoints_to_write*nchars_per_point*
               sizeof(char8_t);
            char8_t* rgba_data=new char8_t[n_color_bytes];

            for (int i=0; i<npoints_to_write; i++)
            {
               osg::Vec3 curr_xyz( vertices_ptr->at(i+point_offset) );
               if (curr_xyz.z() > 0.5*NEGATIVEINFINITY)
               {
                  osg::Vec4ub curr_RGBA(colors_ptr->at(i+point_offset));
                  rgba_data[4*i+0]=static_cast<char8_t>(curr_RGBA.r());
                  rgba_data[4*i+1]=static_cast<char8_t>(curr_RGBA.g());
                  rgba_data[4*i+2]=static_cast<char8_t>(curr_RGBA.b());
                  rgba_data[4*i+3]=static_cast<char8_t>(curr_RGBA.a());
               }
               else
               {
                  rgba_data[4*i+0]=0;
                  rgba_data[4*i+1]=0;
                  rgba_data[4*i+2]=0;
                  rgba_data[4*i+3]=0;
               }

/*
               if ( i < 5)
               {
                  cout << "i = " << i
                       << " r = " << stringfunc::char_to_ascii_integer(
                          rgba_data[4*i+0])
                       << " g = " << stringfunc::char_to_ascii_integer(
                          rgba_data[4*i+1])
                       << " b = " << stringfunc::char_to_ascii_integer(
                          rgba_data[4*i+2])
                       << " a = " << stringfunc::char_to_ascii_integer(
                          rgba_data[4*i+3])
                       << endl;
               }
*/

            } // loop over index i labeling vertices

            if (iter==0)
            {
               tdp_file.klv_create_and_write( 
                  TdpKeyRGBA_COLOR_8,rgba_data,n_color_bytes);
            }
            else
            {
               uint64_t klv_index = 0;
               tdp_file.klv_append( 
                  TdpKeyRGBA_COLOR_8,klv_index,rgba_data,n_color_bytes);
            }

            delete [] rgba_data;
            n_points -= npoints_to_write;
            point_offset += npoints_to_write;
         } // loop over iter index

         tdp_file.file_close();
      }

// ---------------------------------------------------------------------
// This overloaded version of write_relative_xyzrgba_data() takes in a
// double precision zeroth_xyz origin as well as relative XYZ as OSG
// Vec3 objects.

   void write_relative_xyzrgba_data(
      string tdp_filename,string UTMzone,const threevector& zeroth_xyz,
      const osg::Vec3Array* rel_vertices_ptr,
      const osg::Vec4ubArray* colors_ptr)
      {
//         cout << "inside tdpfunc::write_relative_xyzrgba_data()" << endl;
//         cout << "zeroth_xyz = " << zeroth_xyz << endl;
//         cout << "colors_ptr->size() = " << colors_ptr->size() << endl;

         unsigned int n_points=rel_vertices_ptr->size();
         int n_iters=get_n_iters(n_points);
         Tdp_file tdp_file;
         initialize_output_tdpfile(tdp_filename,UTMzone,tdp_file,zeroth_xyz);

// First write XYZ vertices to output tdp file:

         const int nfloats_per_point=3;
         const int nchars_per_point=4;
         int point_offset=0;
         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_write=basic_math::min(max_points_per_iter,n_points);
//         cout << "iter = " << iter
//              << " npoints_to_write = " << npoints_to_write << endl;
            int n_xyz_bytes=npoints_to_write*nfloats_per_point*
               sizeof(real32_t);
            real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];
            
            for (int i=0; i<npoints_to_write; i++)
            {
               osg::Vec3 rel_xyz(rel_vertices_ptr->at(i+point_offset));
               double curr_z=zeroth_xyz.get(2)+rel_xyz.z();
               if (curr_z > 0.5*NEGATIVEINFINITY)
               {
                  rel_xyz_data[3*i+0]=rel_xyz.x();
                  rel_xyz_data[3*i+1]=rel_xyz.y();
                  rel_xyz_data[3*i+2]=rel_xyz.z();
               }
               else
               {
                  rel_xyz_data[3*i+0]=0;
                  rel_xyz_data[3*i+1]=0;
                  rel_xyz_data[3*i+2]=0;
               }

//               cout << "x = " << curr_xyz.get(0)
//                    << " y = " << curr_xyz.get(1) << endl;
//               cout << "rel_x = " << rel_xyz_data[3*i+0] 
//               cout << " rel_y = " << rel_xyz_data[3*i+1] << endl;

            } // loop over index i labeling vertices

            if (iter==0)
            {
               tdp_file.klv_create_and_write( 
                  TdpKeyXYZ_POINT_DATA,rel_xyz_data,n_xyz_bytes);
            }
            else
            {
               uint64_t klv_index = 0;
               tdp_file.klv_append( 
                  TdpKeyXYZ_POINT_DATA,klv_index,rel_xyz_data,n_xyz_bytes);
            }
            
            delete [] rel_xyz_data;
            n_points -= npoints_to_write;
            point_offset += npoints_to_write;

         } // loop over iter index
         
// Next write RGBA color information to output tdp file:

         point_offset=0;
         n_points=rel_vertices_ptr->size();
         n_iters=get_n_iters(n_points);

//         cout << "n_points = " << n_points << endl;
//         cout << "n_iters = " << n_iters << endl;

         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_write=basic_math::min(max_points_per_iter,n_points);
            int n_color_bytes=npoints_to_write*nchars_per_point*
               sizeof(char8_t);
            char8_t* rgba_data=new char8_t[n_color_bytes];

            for (int i=0; i<npoints_to_write; i++)
            {
               osg::Vec3 rel_xyz(rel_vertices_ptr->at(i+point_offset));
               double curr_z=zeroth_xyz.get(2)+rel_xyz.z();
               if (curr_z > 0.5*NEGATIVEINFINITY)
               {
                  osg::Vec4ub curr_RGBA(colors_ptr->at(i+point_offset));
                  rgba_data[4*i+0]=static_cast<char8_t>(curr_RGBA.r());
                  rgba_data[4*i+1]=static_cast<char8_t>(curr_RGBA.g());
                  rgba_data[4*i+2]=static_cast<char8_t>(curr_RGBA.b());
                  rgba_data[4*i+3]=static_cast<char8_t>(curr_RGBA.a());
               }
               else
               {
                  rgba_data[4*i+0]=0;
                  rgba_data[4*i+1]=0;
                  rgba_data[4*i+2]=0;
                  rgba_data[4*i+3]=0;
               }
            } // loop over index i labeling vertices

            if (iter==0)
            {
               tdp_file.klv_create_and_write( 
                  TdpKeyRGBA_COLOR_8,rgba_data,n_color_bytes);
            }
            else
            {
               uint64_t klv_index = 0;
               tdp_file.klv_append( 
                  TdpKeyRGBA_COLOR_8,klv_index,rgba_data,n_color_bytes);
            }

            delete [] rgba_data;
            n_points -= npoints_to_write;
            point_offset += npoints_to_write;
         } // loop over iter index

         tdp_file.file_close();
      }

// ---------------------------------------------------------------------
// This overloaded version of write_relative_xyzrgba_data takes XYZ
// and RGB information within input STL vectors.  After converting the
// input data to osg::Vec3Array and osg::Vec4ubArray formats, it calls
// the previous write_relative_xyzrgba_data() method.

   void write_relative_xyzrgba_data(
      string UTMzone,string tdp_filename,
      const vector<double>& X,const vector<double>& Y,const vector<double>& Z,
      const vector<int>& R,const vector<int>& G,const vector<int>& B)
      {
         unsigned int n_points=X.size();
         threevector zeroth_xyz(X[0],Y[0],Z[0]);

// Recall that osg::Vec3Array can only hold 4-byte floats and not
// 8-byte doubles.  So we need to store relative and not absolute XYZ
// vertex coordinates within Vec3 objects!

         osg::Vec3Array* rel_vertices_ptr=new osg::Vec3Array();
         rel_vertices_ptr->reserve(n_points);

         osg::Vec4ubArray* colors_ptr=new osg::Vec4ubArray();
         colors_ptr->reserve(n_points);

         osg::Vec3 rel_XYZ;
         osg::Vec4ub RGBA;
         const unsigned char alpha_byte=static_cast<unsigned char>(
            stringfunc::ascii_integer_to_char(255));

         for (unsigned int i=0; i<n_points; i++)
         {
            rel_XYZ[0]=X[i]-zeroth_xyz.get(0);
            rel_XYZ[1]=Y[i]-zeroth_xyz.get(1);
            rel_XYZ[2]=Z[i]-zeroth_xyz.get(2);
            rel_vertices_ptr->push_back(rel_XYZ);

            RGBA=osg::Vec4ub(
               static_cast<unsigned char>(static_cast<unsigned int>(R[i])),
               static_cast<unsigned char>(static_cast<unsigned int>(G[i])),
               static_cast<unsigned char>(static_cast<unsigned int>(B[i])),
               alpha_byte);
            colors_ptr->push_back(RGBA);
         }
         write_relative_xyzrgba_data(
            tdp_filename,UTMzone,zeroth_xyz,rel_vertices_ptr,colors_ptr);
      }

// ---------------------------------------------------------------------
// This overloaded version of write_relative_xyzrgba_data() takes XYZ
// information in as double precision threevectors rather than as OSG
// Vec3 objects.

   void write_relative_xyzrgba_data(
      string tdp_filename,string UTMzone,const threevector& zeroth_xyz,
      const vector<threevector>* vertices_ptr,
      const osg::Vec4ubArray* colors_ptr)
      {
//         cout << "inside tdpfunc::write_relative_xyzrgba_data()" << endl;
//         cout << "colors_ptr->size() = " << colors_ptr->size() << endl;

         unsigned int n_points=vertices_ptr->size();
         int n_iters=get_n_iters(n_points);
         
         Tdp_file tdp_file;
         initialize_output_tdpfile(tdp_filename,UTMzone,tdp_file,zeroth_xyz);

// First write XYZ vertices to output tdp file:

         double zeroth_x=zeroth_xyz.get(0);
         double zeroth_y=zeroth_xyz.get(1);
         double zeroth_z=zeroth_xyz.get(2);

         const int nfloats_per_point=3;
         const int nchars_per_point=4;
         int point_offset=0;
         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_write=basic_math::min(max_points_per_iter,n_points);
//         cout << "iter = " << iter
//              << " npoints_to_write = " << npoints_to_write << endl;
            int n_xyz_bytes=npoints_to_write*nfloats_per_point*
               sizeof(real32_t);
            real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];
            
            for (int i=0; i<npoints_to_write; i++)
            {
               threevector curr_xyz(vertices_ptr->at(i+point_offset));
               double curr_z=curr_xyz.get(2);
               if (curr_z > 0.5*NEGATIVEINFINITY)
               {
                  rel_xyz_data[3*i+0]=curr_xyz.get(0)-zeroth_x;
                  rel_xyz_data[3*i+1]=curr_xyz.get(1)-zeroth_y;
                  rel_xyz_data[3*i+2]=curr_z-zeroth_z;
               }
               else
               {
                  rel_xyz_data[3*i+0]=0;
                  rel_xyz_data[3*i+1]=0;
                  rel_xyz_data[3*i+2]=0;
               }

//               cout << "x = " << curr_xyz.get(0)
//                    << " y = " << curr_xyz.get(1) << endl;
//               cout << "rel_x = " << rel_xyz_data[3*i+0] 
//               cout << " rel_y = " << rel_xyz_data[3*i+1] << endl;

            } // loop over index i labeling vertices

            if (iter==0)
            {
               tdp_file.klv_create_and_write( 
                  TdpKeyXYZ_POINT_DATA,rel_xyz_data,n_xyz_bytes);
            }
            else
            {
               uint64_t klv_index = 0;
               tdp_file.klv_append( 
                  TdpKeyXYZ_POINT_DATA,klv_index,rel_xyz_data,n_xyz_bytes);
            }
            
            delete [] rel_xyz_data;
            n_points -= npoints_to_write;
            point_offset += npoints_to_write;

         } // loop over iter index
         
// Next write RGBA color information to output tdp file:

         point_offset=0;
         n_points=vertices_ptr->size();
         n_iters=get_n_iters(n_points);

//         cout << "n_points = " << n_points << endl;
//         cout << "n_iters = " << n_iters << endl;

         for (int iter=0; iter<n_iters; iter++)
         {
            int npoints_to_write=basic_math::min(max_points_per_iter,n_points);
            int n_color_bytes=npoints_to_write*nchars_per_point*
               sizeof(char8_t);
            char8_t* rgba_data=new char8_t[n_color_bytes];

            for (int i=0; i<npoints_to_write; i++)
            {
               threevector curr_xyz(vertices_ptr->at(i+point_offset));
               double curr_z=curr_xyz.get(2);
               if (curr_z > 0.5*NEGATIVEINFINITY)
               {
                  osg::Vec4ub curr_RGBA(colors_ptr->at(i+point_offset));
                  rgba_data[4*i+0]=static_cast<char8_t>(curr_RGBA.r());
                  rgba_data[4*i+1]=static_cast<char8_t>(curr_RGBA.g());
                  rgba_data[4*i+2]=static_cast<char8_t>(curr_RGBA.b());
                  rgba_data[4*i+3]=static_cast<char8_t>(curr_RGBA.a());
               }
               else
               {
                  rgba_data[4*i+0]=0;
                  rgba_data[4*i+1]=0;
                  rgba_data[4*i+2]=0;
                  rgba_data[4*i+3]=0;
               }
            } // loop over index i labeling vertices

            if (iter==0)
            {
               tdp_file.klv_create_and_write( 
                  TdpKeyRGBA_COLOR_8,rgba_data,n_color_bytes);
            }
            else
            {
               uint64_t klv_index = 0;
               tdp_file.klv_append( 
                  TdpKeyRGBA_COLOR_8,klv_index,rgba_data,n_color_bytes);
            }

            delete [] rgba_data;
            n_points -= npoints_to_write;
            point_offset += npoints_to_write;
         } // loop over iter index

         tdp_file.file_close();
      }

// ---------------------------------------------------------------------
// This overloaded version of write_relative_xyzrgba_data() takes in a
// set of red, green and blue twoDarrays which are assumed to contain
// EO tif image information.  It extracts X and Y values from the
// arrays and writes them (along with a constant Z=0 height) into the
// TDP file specified by its input filename.  This method also
// extracts corresponding RGB colors and writes them into the output
// TDP file.  If the number output points is very large, this method
// writes tdp output to a numbered sequence of tdp files.

   void write_relative_xyzrgba_data(
      string tdp_filename,string UTMzone,const twoDarray* RtwoDarray_ptr,
      const twoDarray* GtwoDarray_ptr,const twoDarray* BtwoDarray_ptr,
      int nodata_value)
      {
//         cout << "inside tdpfunc::write_relative_xyzrgba_data()" << endl;

         int mdim=RtwoDarray_ptr->get_mdim();
         int ndim=RtwoDarray_ptr->get_ndim();

//         cout << "mdim = " << mdim << " ndim = " << ndim << endl;
//         cout << " mdim*ndim = " << mdim*ndim << endl;

         unsigned int n_points=mdim*ndim;
         int n_iters=get_n_iters(n_points);
         int n_xyz_triples_actually_written=0;
         int n_colors_actually_written=0;

// On 32-bit machines with no more than 4 Gbytes of RAM, we run out of
// memory if we attempt to write out TDP files which are too huge.  So
// generate new output TDP files in this case:

         const int nmax_iters_per_tdp_file=30;
         int n_output_tdp_files=n_iters/nmax_iters_per_tdp_file+1;
         int tdp_file_point_offset=0;

         double zeroth_x,zeroth_y,zeroth_z=0;
         RtwoDarray_ptr->pixel_to_point(0,0,zeroth_x,zeroth_y);
         threevector zeroth_xyz(zeroth_x,zeroth_y,zeroth_z);

         for (int n_output_file=0; n_output_file<n_output_tdp_files; 
              n_output_file++)
         {
            string output_tdp_filename=tdp_filename;
            if (n_output_tdp_files > 1)
            {
               string prefix=stringfunc::prefix(tdp_filename);
               string suffix=stringfunc::suffix(tdp_filename);
               output_tdp_filename=prefix+"_"+stringfunc::number_to_string(
                  n_output_file)+"."+suffix;
               cout << "n_output_file = " << n_output_file 
                    << " output_tdp_filename = " << output_tdp_filename 
                    << endl;
            }
            
            Tdp_file tdp_file;
            initialize_output_tdpfile(
               output_tdp_filename,UTMzone,tdp_file,zeroth_xyz);

// First write XYZ vertices to output tdp file:

            const int nfloats_per_point=3;
            const int nchars_per_point=4;
            int point_offset=tdp_file_point_offset;
            for (int iter=0; iter<basic_math::min(
               n_iters,nmax_iters_per_tdp_file); 
                 iter++)
            {

// Note: Last iteration generally contains some fraction less than
// unity of max_points_per_iter to write.  So we need to explicitly
// cap npoints_to_write by n_remaining_points:

               unsigned int n_remaining_points=
                  mdim*ndim-n_xyz_triples_actually_written;
               int npoints_to_write=basic_math::min(
                  max_points_per_iter,n_points,
                  n_remaining_points);
//               cout << "iter = " << iter
//                    << " npoints_to_write = " << npoints_to_write << endl;
               int n_xyz_bytes=npoints_to_write*nfloats_per_point*
                  sizeof(real32_t);
               real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];
            
               int i_actual=0;
               double x,y,z=0;
               for (int i=0; i<npoints_to_write; i++)
               {
                  int px,py;
                  RtwoDarray_ptr->index_to_indices(point_offset+i,px,py);

// Check whether current XYZ point corresponds to "nodata" color RGB
// values.  If so, do NOT include the XYZ info into rel_xyz_data:

                  int r=RtwoDarray_ptr->get(px,py);
                  int g=GtwoDarray_ptr->get(px,py);
                  int b=BtwoDarray_ptr->get(px,py);

                  RtwoDarray_ptr->pixel_to_point(px,py,x,y);               
                  if (r==nodata_value && g==nodata_value && b==nodata_value)
                  {
                  }
                  else
                  {
                     rel_xyz_data[i_actual++]=x-zeroth_x;
                     rel_xyz_data[i_actual++]=y-zeroth_y;
                     rel_xyz_data[i_actual++]=z-zeroth_z;
                  }
               } // loop over index i

               n_xyz_bytes=i_actual*sizeof(real32_t);
               if (iter==0)
               {
                  tdp_file.klv_create_and_write( 
                     TdpKeyXYZ_POINT_DATA,rel_xyz_data,n_xyz_bytes);
               }
               else
               {
                  uint64_t klv_index = 0;
                  tdp_file.klv_append( 
                     TdpKeyXYZ_POINT_DATA,klv_index,rel_xyz_data,n_xyz_bytes);
               }
            
               delete [] rel_xyz_data;
               n_points -= npoints_to_write;
               point_offset += npoints_to_write;
               n_xyz_triples_actually_written += npoints_to_write;

//               if (n_xyz_triples_actually_written >= mdim*ndim)
//               {
//                  cout << "n_xyz_triples_actually_written=" 
//                       << n_xyz_triples_actually_written
//                       << " mdim*ndim=" << mdim*ndim
//                       << " iter=" << iter << " n_iters=" << n_iters
//                       << endl;
//               }

            } // loop over iter index
         
// Next write RGBA color information to output tdp file:

            point_offset=tdp_file_point_offset;
            n_points=mdim*ndim;

//         cout << "n_points = " << n_points << endl;
//         cout << "n_iters = " << n_iters << endl;
            unsigned char achar=stringfunc::ascii_integer_to_unsigned_char(
               255);

            for (int iter=0; iter<basic_math::min(
               n_iters,nmax_iters_per_tdp_file); 
                 iter++)
            {
               unsigned int n_remaining_points=
                  mdim*ndim-n_colors_actually_written;
               int npoints_to_write=basic_math::min(
                  max_points_per_iter,n_points,
                  n_remaining_points);
//               cout << "iter = " << iter
//                    << " n_remaining_points = " << n_remaining_points
//                    << " n_points_to_write = " << npoints_to_write
//                    << endl;

               int i_actual=0;
               int n_color_bytes=npoints_to_write*nchars_per_point*
                  sizeof(char8_t);
               char8_t* rgba_data=new char8_t[n_color_bytes];
   
               for (int i=0; i<npoints_to_write; i++)
               {
                  int px,py;
                  RtwoDarray_ptr->index_to_indices(point_offset+i,px,py);

// Check whether current RGB point corresponds to "nodata" color RGB
// values.  If so, do NOT include the RGB info into rgba_data:

                  int r=RtwoDarray_ptr->get(px,py);
                  int g=GtwoDarray_ptr->get(px,py);
                  int b=BtwoDarray_ptr->get(px,py);

                  if (r==nodata_value && g==nodata_value && b==nodata_value)
                  {
                  }
                  else
                  {
//                cout << "r = " << r << " g = " << g << " b = " << b << endl;
                     unsigned rchar=
                        stringfunc::ascii_integer_to_unsigned_char(r);
                     unsigned gchar=
                        stringfunc::ascii_integer_to_unsigned_char(g);
                     unsigned bchar=
                        stringfunc::ascii_integer_to_unsigned_char(b);

                     rgba_data[i_actual++]=static_cast<char8_t>(rchar);
                     rgba_data[i_actual++]=static_cast<char8_t>(gchar);
                     rgba_data[i_actual++]=static_cast<char8_t>(bchar);
                     rgba_data[i_actual++]=static_cast<char8_t>(achar);
                  }
               } // loop over index i labeling vertices

               n_color_bytes=i_actual*sizeof(char8_t);
               if (iter==0)
               {
                  tdp_file.klv_create_and_write( 
                     TdpKeyRGBA_COLOR_8,rgba_data,n_color_bytes);
               }
               else
               {
                  uint64_t klv_index = 0;
                  tdp_file.klv_append( 
                     TdpKeyRGBA_COLOR_8,klv_index,rgba_data,n_color_bytes);
               }

               delete [] rgba_data;
               n_points -= npoints_to_write;
               point_offset += npoints_to_write;
               n_colors_actually_written += npoints_to_write;

//               if (n_colors_actually_written >= mdim*ndim)
//               {
//                  cout << "n_colors_actually_written=" 
//                       << n_colors_actually_written
//                       << " mdim*ndim=" << mdim*ndim
 //                      << " iter=" << iter << " n_iters=" << n_iters
//                       << endl;
//               }

            } // loop over iter index

            tdp_file.file_close();

            n_iters -= nmax_iters_per_tdp_file;
            tdp_file_point_offset=point_offset;

         } // loop over n_output_file index

//         cout << "at end of tdpfunc::write_relative_xyzrgba_data()" << endl;
//         cout << "n_xyz_triples_actually_written = "
//              << n_xyz_triples_actually_written << endl;
//         cout << "n_colors_actually_written = "
//              << n_colors_actually_written << endl;
      }
   
// ---------------------------------------------------------------------
// This overloaded version of write_relative_xyzrgba_data() takes in a
// height ZtwoDarray along with a set of red, green and blue
// twoDarrays which are assumed to contain EO tif image information.
// It extracts X, Y and Z values from *ztwoDarray_ptr and writes them
// into the TDP file specified by its input filename.  This method
// also extracts corresponding RGB colors and writes them into the
// output TDP file.  If the number output points is very large, this
// method writes tdp output to a numbered sequence of tdp files.

   void write_relative_xyzrgba_data(
      string tdp_filename,string UTMzone,
      const twoDarray* ztwoDarray_ptr,const twoDarray* RtwoDarray_ptr,
      const twoDarray* GtwoDarray_ptr,const twoDarray* BtwoDarray_ptr)
      {
//         cout << "inside tdpfunc::write_relative_xyzrgba_data()" << endl;

         int mdim=ztwoDarray_ptr->get_mdim();
         int ndim=ztwoDarray_ptr->get_ndim();

//         cout << "mdim = " << mdim << " ndim = " << ndim << endl;
//         cout << " mdim*ndim = " << mdim*ndim << endl;

         unsigned int n_points=mdim*ndim;
         int n_iters=get_n_iters(n_points);
         int n_xyz_triples_actually_written=0;
         int n_colors_actually_written=0;

// On 32-bit machines with no more than 4 Gbytes of RAM, we run out of
// memory if we attempt to write out TDP files which are too huge.  So
// generate new output TDP files in this case:

         const int nmax_iters_per_tdp_file=30;
         int n_output_tdp_files=n_iters/nmax_iters_per_tdp_file+1;
         int tdp_file_point_offset=0;

         double zeroth_x,zeroth_y;
         ztwoDarray_ptr->pixel_to_point(0,0,zeroth_x,zeroth_y);
//         double zeroth_z=ztwoDarray_ptr->get(0,0);
         double zeroth_z=0;
         threevector zeroth_xyz(zeroth_x,zeroth_y,zeroth_z);
         cout << "zeroth_xyz = " << zeroth_xyz << endl;

         for (int n_output_file=0; n_output_file<n_output_tdp_files; 
              n_output_file++)
         {
            string output_tdp_filename=tdp_filename;
            if (n_output_tdp_files > 1)
            {
               string prefix=stringfunc::prefix(tdp_filename);
               string suffix=stringfunc::suffix(tdp_filename);
               output_tdp_filename=prefix+"_"+stringfunc::number_to_string(
                  n_output_file)+"."+suffix;
               cout << "n_output_file = " << n_output_file 
                    << " output_tdp_filename = " << output_tdp_filename 
                    << endl;
            }
            
            Tdp_file tdp_file;
            initialize_output_tdpfile(
               output_tdp_filename,UTMzone,tdp_file,zeroth_xyz);

// First write XYZ vertices to output tdp file:

            const int nfloats_per_point=3;
            const int nchars_per_point=4;
            int point_offset=tdp_file_point_offset;
            for (int iter=0; iter<basic_math::min(
               n_iters,nmax_iters_per_tdp_file); iter++)
            {

// Note: Last iteration generally contains some fraction less than
// unity of max_points_per_iter to write.  So we need to explicitly
// cap npoints_to_write by n_remaining_points:

               unsigned int n_remaining_points=
                  mdim*ndim-n_xyz_triples_actually_written;
               int npoints_to_write=basic_math::min(
                  max_points_per_iter,n_points,n_remaining_points);
//               cout << "iter = " << iter
//                    << " npoints_to_write = " << npoints_to_write << endl;
               int n_xyz_bytes=npoints_to_write*nfloats_per_point*
                  sizeof(real32_t);
               real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];
            
               double x,y;
               for (int i=0; i<npoints_to_write; i++)
               {
                  int px,py;
                  ztwoDarray_ptr->index_to_indices(point_offset+i,px,py);
                  ztwoDarray_ptr->pixel_to_point(px,py,x,y);
                  double z=ztwoDarray_ptr->get(px,py);

                  if (z > 0.5*NEGATIVEINFINITY)
                  {
                     rel_xyz_data[3*i+0]=x-zeroth_x;
                     rel_xyz_data[3*i+1]=y-zeroth_y;
                     rel_xyz_data[3*i+2]=z-zeroth_z;
                  }
                  else
                  {
                     rel_xyz_data[3*i+0]=0;
                     rel_xyz_data[3*i+1]=0;
                     rel_xyz_data[3*i+2]=0;
                  }

               } // loop over index i
            
               if (iter==0)
               {
                  tdp_file.klv_create_and_write( 
                     TdpKeyXYZ_POINT_DATA,rel_xyz_data,n_xyz_bytes);
               }
               else
               {
                  uint64_t klv_index = 0;
                  tdp_file.klv_append( 
                     TdpKeyXYZ_POINT_DATA,klv_index,rel_xyz_data,n_xyz_bytes);
               }
            
               delete [] rel_xyz_data;
               n_points -= npoints_to_write;
               point_offset += npoints_to_write;
               n_xyz_triples_actually_written += npoints_to_write;

//               if (n_xyz_triples_actually_written >= mdim*ndim)
//               {
//                  cout << "n_xyz_triples_actually_written=" 
//                       << n_xyz_triples_actually_written
//                       << " mdim*ndim=" << mdim*ndim
//                       << " iter=" << iter << " n_iters=" << n_iters
//                       << endl;
//               }

            } // loop over iter index
         
// Next write RGBA color information to output tdp file:

            point_offset=tdp_file_point_offset;
            n_points=mdim*ndim;

//         cout << "n_points = " << n_points << endl;
//         cout << "n_iters = " << n_iters << endl;
            unsigned char achar=stringfunc::ascii_integer_to_unsigned_char(
               255);

            for (int iter=0; iter<basic_math::min(
                    n_iters,nmax_iters_per_tdp_file); iter++)
            {
               unsigned int n_remaining_points=
                  mdim*ndim-n_colors_actually_written;
               int npoints_to_write=basic_math::min(
                  max_points_per_iter,n_points,n_remaining_points);
//               cout << "iter = " << iter
//                    << " n_remaining_points = " << n_remaining_points
//                    << " n_points_to_write = " << npoints_to_write
//                    << endl;

               int n_color_bytes=npoints_to_write*nchars_per_point*
                  sizeof(char8_t);
               char8_t* rgba_data=new char8_t[n_color_bytes];
   
               for (int i=0; i<npoints_to_write; i++)
               {
                  int px,py;
                  RtwoDarray_ptr->index_to_indices(point_offset+i,px,py);
                  int r=RtwoDarray_ptr->get(px,py);
                  int g=GtwoDarray_ptr->get(px,py);
                  int b=BtwoDarray_ptr->get(px,py);
//                cout << "r = " << r << " g = " << g << " b = " << b << endl;
                  unsigned rchar=
                     stringfunc::ascii_integer_to_unsigned_char(r);
                  unsigned gchar=
                     stringfunc::ascii_integer_to_unsigned_char(g);
                  unsigned bchar=
                     stringfunc::ascii_integer_to_unsigned_char(b);

                  rgba_data[4*i+0]=static_cast<char8_t>(rchar);
                  rgba_data[4*i+1]=static_cast<char8_t>(gchar);
                  rgba_data[4*i+2]=static_cast<char8_t>(bchar);
                  rgba_data[4*i+3]=static_cast<char8_t>(achar);
               } // loop over index i labeling vertices

               if (iter==0)
               {
                  tdp_file.klv_create_and_write( 
                     TdpKeyRGBA_COLOR_8,rgba_data,n_color_bytes);
               }
               else
               {
                  uint64_t klv_index = 0;
                  tdp_file.klv_append( 
                     TdpKeyRGBA_COLOR_8,klv_index,rgba_data,n_color_bytes);
               }

               delete [] rgba_data;
               n_points -= npoints_to_write;
               point_offset += npoints_to_write;
               n_colors_actually_written += npoints_to_write;

//               if (n_colors_actually_written >= mdim*ndim)
//               {
//                  cout << "n_colors_actually_written=" 
//                       << n_colors_actually_written
//                       << " mdim*ndim=" << mdim*ndim
 //                      << " iter=" << iter << " n_iters=" << n_iters
//                       << endl;
//               }

            } // loop over iter index

            tdp_file.file_close();

            n_iters -= nmax_iters_per_tdp_file;
            tdp_file_point_offset=point_offset;

         } // loop over n_output_file index

//         cout << "at end of tdpfunc::write_relative_xyzrgba_data()" << endl;
//         cout << "n_xyz_triples_actually_written = "
//              << n_xyz_triples_actually_written << endl;
//         cout << "n_colors_actually_written = "
//              << n_colors_actually_written << endl;
      }

// ---------------------------------------------------------------------
// Method write_zp_twoDarrays

   void write_zp_twoDarrays(
      string tdp_filename,string UTMzone,
      const twoDarray* ztwoDarray_ptr,const twoDarray* ptwoDarray_ptr,
      bool insert_fake_coloring_points_flag)
      {
         outputfunc::write_banner(
            "Writing out ztwoDarray & ptwoDarray data:");

         int npoints=ztwoDarray_ptr->get_mdim()*ztwoDarray_ptr->get_ndim();
         osg::Vec3Array* vertices_ptr=new osg::Vec3Array;
         osg::FloatArray* probs_ptr=new osg::FloatArray;
         vertices_ptr->reserve(npoints);
         probs_ptr->reserve(npoints);

         int pnt_counter=0;
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               if (ztwoDarray_ptr->get(px,py) > 0.99*xyzpfunc::null_value)
               {
                  double x,y;
                  ztwoDarray_ptr->pixel_to_point(px,py,x,y);
                  vertices_ptr->push_back(osg::Vec3(
                     static_cast<float>(x),
                     static_cast<float>(y),
                     static_cast<float>(ztwoDarray_ptr->get(px,py))));
                  float pfloat=static_cast<float>(ptwoDarray_ptr->get(px,py));
                  probs_ptr->push_back(pfloat);
                  pnt_counter++;
               } // ztwoDarray(px,py) > xyzpfunc::null_value conditional
            } // loop over index py
         } // loop over index px

// Insert fake XYZP points at very start of twoDarrays for p-data
// coloring purposes:

         if (insert_fake_coloring_points_flag)
         {
            for (int i=0; i<=20; i++)
            {
               vertices_ptr->push_back(vertices_ptr->at(0));
               probs_ptr->push_back(i*0.05);
            }
         }
         
         write_relative_xyzp_data(
            tdp_filename,UTMzone,vertices_ptr,probs_ptr);
      }
   
// ==========================================================================
// XYZ output methods
// ==========================================================================

   void write_xyz_data(
      string tdp_filename,vector<double>* X_ptr,
      vector<double>* Y_ptr,vector<double>* Z_ptr)
   {
      string UTMzone="";
      threevector zeroth_XYZ(X_ptr->at(0),Y_ptr->at(0),Z_ptr->at(0));
      write_xyz_data(tdp_filename,UTMzone,zeroth_XYZ,X_ptr,Y_ptr,Z_ptr);
   }

   void write_xyz_data(
      string tdp_filename,string UTMzone,const threevector& zeroth_XYZ,
      vector<double>* X_ptr,vector<double>* Y_ptr,vector<double>* Z_ptr)
      {
//         cout << "inside tdpfunc::write_xyz_data()" << endl;
         
         Tdp_file tdp_file;
         tdp_file.file_open( tdp_filename, "w+b" );
         write_UTM_zone_and_offset(tdp_file,UTMzone,zeroth_XYZ);

// Write xyz point data to file all at once:

         int npoints=X_ptr->size();
         const int nfloats_per_point=3;
         int n_xyz_bytes=npoints*nfloats_per_point*sizeof(real32_t);
         real32_t* xyz_data=new real32_t[n_xyz_bytes];
         for (int i=0; i<npoints; i++)
         {
            xyz_data[3*i+0]=X_ptr->at(i)-zeroth_XYZ.get(0);
            xyz_data[3*i+1]=Y_ptr->at(i)-zeroth_XYZ.get(1);
            xyz_data[3*i+2]=Z_ptr->at(i)-zeroth_XYZ.get(2);
         }

         tdp_file.klv_create_and_write( 
            TdpKeyXYZ_POINT_DATA,xyz_data,n_xyz_bytes);
         delete [] xyz_data;

         tdp_file.file_close();
      }

// ---------------------------------------------------------------------
// Member function write_xyz_data was written on 4/6/07 when we
// realized that there must be a serious bug in our viewer code which
// cannot read OSGA files containing non-trivial p-data.  As a quick
// hack, we ignore p-data within this method.  

   void write_xyz_data(
      string UTMzone,string tdp_filename,vector<fourvector>* xyzp_pnt_ptr)
      {
         osg::Vec3Array* vertices_ptr=new osg::Vec3Array();

         osg::Vec3 XYZ;
         vertices_ptr->reserve(xyzp_pnt_ptr->size());
         for (int i=0; i<int(xyzp_pnt_ptr->size()); i++)
         {
            XYZ[0]=xyzp_pnt_ptr->at(i).get(0);
            XYZ[1]=xyzp_pnt_ptr->at(i).get(1);
            XYZ[2]=xyzp_pnt_ptr->at(i).get(2);
            vertices_ptr->push_back(XYZ);
         }
         write_relative_xyz_data(tdp_filename,UTMzone,vertices_ptr);
      }

// ---------------------------------------------------------------------
// This overloaded version of write_xyz_data takes in STL vectors
// X,Y,Z and boolean STL vector include_point_flag.  It writes to an
// output TDP file whose name is passed as an input argument those XYZ
// points for which include_point_flag==true.

   void write_xyz_data(
      string UTMzone,string tdp_filename,
      const vector<double>& X,const vector<double>& Y,const vector<double>& Z,
      const vector<bool> include_orig_point)
      {
         osg::Vec3Array* vertices_ptr=new osg::Vec3Array();

         osg::Vec3 XYZ;
         vertices_ptr->reserve(X.size());
         for (unsigned int i=0; i<X.size(); i++)
         {
            if (include_orig_point[i])
            {
               XYZ[0]=X[i];
               XYZ[1]=Y[i];
               XYZ[2]=Z[i];
               vertices_ptr->push_back(XYZ);
            }
         }
         write_relative_xyz_data(tdp_filename,UTMzone,vertices_ptr);
      }

// ---------------------------------------------------------------------
// This overloaded version of write_xyz_data writes the contents
// *ztwoDarray_ptr to the specified output tdp file.

   void write_xyz_data(
      string UTMzone,string tdp_filename,const twoDarray* ztwoDarray_ptr)
      {
         osg::Vec3Array* vertices_ptr=new osg::Vec3Array();

         int n_twoDarray_points=
            ztwoDarray_ptr->get_mdim()*ztwoDarray_ptr->get_ndim();
         vertices_ptr->reserve(n_twoDarray_points);

// Load XYZ info from *ztwoDarray_ptr into *vertices_ptr:

         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               if (ztwoDarray_ptr->get(px,py) > 0.99*xyzpfunc::null_value)
               {
                  double x,y;
                  ztwoDarray_ptr->pixel_to_point(px,py,x,y);
                  vertices_ptr->push_back(osg::Vec3(
                     static_cast<float>(x),
                     static_cast<float>(y),
                     static_cast<float>(ztwoDarray_ptr->get(px,py))));
               }
            }
         }
         
         write_relative_xyz_data(tdp_filename,UTMzone,vertices_ptr);
      }

// ---------------------------------------------------------------------
// This overloaded version of write_xyz_data was hacked together for
// the special purpose of combining median filled NYC TDP files with
// wall content XYZ points from the original NYC tiles.  It takes in
// the median-filled data from input *ztwoDarray_ptr and the
// skyscraper wall points within input STL vectors X,Y,Z.

   void write_xyz_data(
      string UTMzone,string tdp_filename,
      const twoDarray* ztwoDarray_ptr,
      const vector<double>& X,const vector<double>& Y,const vector<double>& Z,
      const vector<bool> include_orig_point)
      {
         osg::Vec3Array* vertices_ptr=new osg::Vec3Array();

         int n_twoDarray_points=
            ztwoDarray_ptr->get_mdim()*ztwoDarray_ptr->get_ndim();
         vertices_ptr->reserve(X.size()+n_twoDarray_points);

// First load *vertices_ptr with XYZ info from X, Y and Z STL vectors:

         osg::Vec3 XYZ;
         for (unsigned int i=0; i<X.size(); i++)
         {
            if (include_orig_point[i])
            {
               XYZ[0]=X[i];
               XYZ[1]=Y[i];
               XYZ[2]=Z[i];
               vertices_ptr->push_back(XYZ);
            }
         }

// Next append XYZ info from *ztwoDarray_ptr into *vertices_ptr:

         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               if (ztwoDarray_ptr->get(px,py) > 0.99*xyzpfunc::null_value)
               {
                  double x,y;
                  ztwoDarray_ptr->pixel_to_point(px,py,x,y);
                  vertices_ptr->push_back(osg::Vec3(
                     static_cast<float>(x),
                     static_cast<float>(y),
                     static_cast<float>(ztwoDarray_ptr->get(px,py))));
               }
            }
         }
         
         write_relative_xyz_data(tdp_filename,UTMzone,vertices_ptr);
      }

// ==========================================================================
// XYZP output methods
// ==========================================================================

   void write_xyzp_data(
      string tdp_filename,vector<threevector>* XYZ_ptr,vector<double>* P_ptr)
   {
      string UTMzone="";
      vector<double>* X_ptr=new vector<double>;
      vector<double>* Y_ptr=new vector<double>;
      vector<double>* Z_ptr=new vector<double>;

      for (unsigned int i=0; i<XYZ_ptr->size(); i++)
      {
         threevector curr_XYZ=XYZ_ptr->at(i);
         X_ptr->push_back(curr_XYZ.get(0));
         Y_ptr->push_back(curr_XYZ.get(1));
         Z_ptr->push_back(curr_XYZ.get(2));
      }
      threevector zeroth_XYZ(X_ptr->at(0),Y_ptr->at(0),Z_ptr->at(0));
      write_xyzp_data(tdp_filename,UTMzone,zeroth_XYZ,X_ptr,Y_ptr,Z_ptr,P_ptr);

      delete X_ptr;
      delete Y_ptr;
      delete Z_ptr;
   }

   void write_xyzp_data(
      string tdp_filename,vector<double>* X_ptr,
      vector<double>* Y_ptr,vector<double>* Z_ptr,vector<double>* P_ptr)
   {
      string UTMzone="";
      threevector zeroth_XYZ(X_ptr->at(0),Y_ptr->at(0),Z_ptr->at(0));
      write_xyzp_data(tdp_filename,UTMzone,zeroth_XYZ,X_ptr,Y_ptr,Z_ptr,P_ptr);
   }

   void write_xyzp_data(
      string tdp_filename,string UTMzone,const threevector& zeroth_XYZ,
      vector<double>* X_ptr,vector<double>* Y_ptr,
      vector<double>* Z_ptr,vector<double>* P_ptr)
      {
//         cout << "inside tdpfunc::write_xyzp_data()" << endl;
         
         Tdp_file tdp_file;
         tdp_file.file_open( tdp_filename, "w+b" );
         write_UTM_zone_and_offset(tdp_file,UTMzone,zeroth_XYZ);

// Write xyz point data to file all at once:

         int npoints=X_ptr->size();
         const int nfloats_per_point=3;
         int n_xyz_bytes=npoints*nfloats_per_point*sizeof(real32_t);
         int n_p_bytes=npoints*sizeof(real32_t);
         real32_t* xyz_data=new real32_t[n_xyz_bytes];
         real32_t* p_data=new real32_t[n_p_bytes];
         for (int i=0; i<npoints; i++)
         {
            xyz_data[3*i+0]=X_ptr->at(i)-zeroth_XYZ.get(0);
            xyz_data[3*i+1]=Y_ptr->at(i)-zeroth_XYZ.get(1);
            xyz_data[3*i+2]=Z_ptr->at(i)-zeroth_XYZ.get(2);
            if (P_ptr==NULL)
            {
               p_data[i]=0;
            }
            else
            {
               p_data[i]=P_ptr->at(i);
            }
//            cout << "i = " << i << " p_data[i] = " << p_data[i] << endl;
         }

         tdp_file.klv_create_and_write( 
            TdpKeyXYZ_POINT_DATA,xyz_data,n_xyz_bytes);
         delete [] xyz_data;

         tdp_file.klv_create_and_write( 
            TdpKeyMETADATA_PROBABILITY_OF_DETECTION,p_data,n_p_bytes);
         delete [] p_data;
         
         tdp_file.file_close();
      }

// ---------------------------------------------------------------------
   void write_xyzp_data(
      string tdp_filename,vector<fourvector>* xyzp_pnt_ptr)
      {
         string UTMzone="0A";
         threevector zeroth_XYZ(0,0,0);
         write_xyzp_data(tdp_filename,UTMzone,zeroth_XYZ,xyzp_pnt_ptr);
      }

   void write_xyzp_data(
      string tdp_filename,string UTMzone,const threevector& zeroth_XYZ,
      vector<fourvector>* xyzp_pnt_ptr)
      {
         Tdp_file tdp_file;
         tdp_file.file_open( tdp_filename, "w+b" );
         write_UTM_zone_and_offset(tdp_file,UTMzone,zeroth_XYZ);

// Write xyz point data to file all at once:

         int npoints=xyzp_pnt_ptr->size();
         const int nfloats_per_point=3;
         int n_xyz_bytes=npoints*nfloats_per_point*sizeof(real32_t);
         int n_p_bytes=npoints*sizeof(real32_t);
         real32_t* xyz_data=new real32_t[n_xyz_bytes];
         real32_t* p_data=new real32_t[n_p_bytes];
         for (int i=0; i<npoints; i++)
         {
            fourvector curr_xyzp( xyzp_pnt_ptr->at(i) );
            xyz_data[3*i+0]=curr_xyzp.get(0)-zeroth_XYZ.get(0);
            xyz_data[3*i+1]=curr_xyzp.get(1)-zeroth_XYZ.get(1);
            xyz_data[3*i+2]=curr_xyzp.get(2)-zeroth_XYZ.get(2);
            p_data[i]=curr_xyzp.get(3);
         }

         tdp_file.klv_create_and_write( 
            TdpKeyXYZ_POINT_DATA,xyz_data,n_xyz_bytes);
         delete [] xyz_data;

         tdp_file.klv_create_and_write( 
            TdpKeyMETADATA_PROBABILITY_OF_DETECTION,p_data,n_p_bytes);
         delete [] p_data;
         
         tdp_file.file_close();
      }




} // tdpfunc namespace
