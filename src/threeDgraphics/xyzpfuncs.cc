// ==========================================================================
// XYZPFUNCS stand-alone methods
// ==========================================================================
// Last modified on 11/20/11; 1/29/12; 4/2/12; 4/5/14
// ==========================================================================

#include <set>
#include "math/basic_math.h"
#include "math/constant_vectors.h"
#include "kdtree/kdtreefuncs.h"
#include "math/threevector.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "image/TwoDarray.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

namespace xyzpfunc
{

// Declare null_value to indicate missing z and/or p data:

   const double null_value=NEGATIVEINFINITY;

// ==========================================================================
// XYZP file input methods:
// ==========================================================================

// Method npoints_inside_xyzp_file [npoints_inside_xyz_file] returns
// the number of (x,y,z,p) [(x,y,z)] points which are contained within
// Group 94 binary xyzp [xyz] float [double] files:
   
   unsigned int npoints_inside_p_file(string p_filename,int p_size)
      {
         long long nbytes=0;
         ifstream binary_instream;
         filefunc::open_binaryfile(p_filename,binary_instream,nbytes);
         binary_instream.close();
         unsigned int npoints=nbytes/p_size;
         cout << "Number of points within input binary p file = " 
              << npoints << endl;
         return npoints;
      }

   unsigned int npoints_inside_xyz_file(string xyz_filename)
      {
         long long nbytes=0;
         ifstream binary_instream;
         filefunc::open_binaryfile(xyz_filename,binary_instream,nbytes);
         binary_instream.close();
         int nfloats=nbytes/sizeof(float);
         unsigned int npoints=nfloats/3;
         cout << "Number of points within input binary xyz file = " 
              << npoints << endl;
         return npoints;
      }

   unsigned int npoints_inside_xyzp_file(string xyzp_filename)
      {
         long long nbytes=0;
         ifstream binary_instream;
         if (filefunc::open_binaryfile(xyzp_filename,binary_instream,nbytes))
         {
            binary_instream.close();
            int nfloats=nbytes/sizeof(float);
            unsigned int npoints=nfloats/4;
            outputfunc::newline();
            cout << "Number of points within input binary xyzp file = " 
                 << npoints << endl;
            return npoints;
         }
         else
         {
            cout << "Error in xyzpfunc::npoints_inside_xyzp_file()" << endl;
            cout << "Cannot open binary file!" << endl;
            exit(-1);
         }
      }

// ---------------------------------------------------------------------
// Method read_xyzp_float_data takes in the name of some Group 94 xyzp
// binary file along with its number of points.  It then fills up 1D
// arrays x[], y[], z[] (and p[]) with the data from this binary file.

   void read_xyz_float_data(unsigned int npoints,string xyz_filename,
                            double x[],double y[],double z[])
      {
         ifstream binary_instream;
         filefunc::open_binaryfile(xyz_filename,binary_instream);

         float curr_x,curr_y,curr_z,curr_p;
         for (unsigned int n=0; n<npoints; n++)
         {
            filefunc::readobject(binary_instream,curr_x);
            filefunc::readobject(binary_instream,curr_y);
            filefunc::readobject(binary_instream,curr_z);
            filefunc::readobject(binary_instream,curr_p);
            x[n]=curr_x;
            y[n]=curr_y;
            z[n]=curr_z;
         }
         binary_instream.close();  
      }

   vector<threevector>* read_xyz_float_data(string xyz_filename)
      {
         ifstream binary_instream;

         bool file_opened=false;
         do
         {
            filefunc::gunzip_file_if_gzipped(xyz_filename);
            file_opened=filefunc::open_binaryfile(
               xyz_filename,binary_instream);
         }
         while (!file_opened);
         unsigned int npoints=npoints_inside_xyz_file(xyz_filename);

         float curr_x,curr_y,curr_z;
         vector<threevector>* xyz_pnt_ptr=new vector<threevector>;
         xyz_pnt_ptr->reserve(npoints);
         for (unsigned int n=0; n<npoints; n++)
         {
            filefunc::readobject(binary_instream,curr_x);
            filefunc::readobject(binary_instream,curr_y);
            filefunc::readobject(binary_instream,curr_z);
            xyz_pnt_ptr->push_back(threevector(curr_x,curr_y,curr_z));
         } // loop over index n labeling input points

         binary_instream.close();  
         return xyz_pnt_ptr;
      }

// ---------------------------------------------------------------------
   void read_xyzp_float_data(unsigned int npoints,string xyzp_filename,
                             double x[],double y[],double z[],double p[])
      {
         ifstream binary_instream;
         filefunc::open_binaryfile(xyzp_filename,binary_instream);

         int n_null_values=0;
         float curr_x,curr_y,curr_z,curr_p;
         for (unsigned int n=0; n<npoints; n++)
         {
            filefunc::readobject(binary_instream,curr_x);
            filefunc::readobject(binary_instream,curr_y);
            filefunc::readobject(binary_instream,curr_z);
            filefunc::readobject(binary_instream,curr_p);
            x[n]=curr_x;
            y[n]=curr_y;
            z[n]=curr_z;

// In cleaned versions of xyzp datafiles, we indicate null
// probabilities with xyzpfunc::null_value values.  So when reading
// back in cleaned files, we interpret any negative probability value
// as null...

            if (curr_p >= 0)
            {

// As of 1/20/05, we no longer follow the old Group 94 convention of
// multiplying all p-values by 10 before writing them out to XYZP
// files.  So we no longer need to divide p-values by 10 when reading
// them in from XYZP files:

               p[n]=curr_p;
//               p[n]=0.1*curr_p;
            }
            else
            {
               n_null_values++;
               p[n]=xyzpfunc::null_value;
            }
         }
         binary_instream.close();  
      }

// ---------------------------------------------------------------------
   int read_xyzp_float_data(
      string xyzp_filename,vector<double>* X_ptr,
      vector<double>* Y_ptr,vector<double>* Z_ptr,vector<double>* P_ptr)
      {
//         cout << "inside xyzpfunc::read_xyzp_float_data()" << endl;
         
         ifstream binary_instream;
         filefunc::open_binaryfile(xyzp_filename,binary_instream);

         unsigned int n_points=npoints_inside_xyzp_file(xyzp_filename);
         X_ptr->clear();
         Y_ptr->clear();
         Z_ptr->clear();
         P_ptr->clear();
         
         X_ptr->reserve(n_points);
         Y_ptr->reserve(n_points);
         Z_ptr->reserve(n_points);
         P_ptr->reserve(n_points);

         float curr_x,curr_y,curr_z,curr_p;
         for (unsigned int n=0; n<n_points; n++)
         {
            filefunc::readobject(binary_instream,curr_x);
            filefunc::readobject(binary_instream,curr_y);
            filefunc::readobject(binary_instream,curr_z);
            filefunc::readobject(binary_instream,curr_p);
            X_ptr->push_back(curr_x);
            Y_ptr->push_back(curr_y);
            Z_ptr->push_back(curr_z);
            P_ptr->push_back(curr_p);
         }
         binary_instream.close();  
         return n_points;
      }

// ---------------------------------------------------------------------
   void read_xyzp_float_data(
      std::string xyzp_filename,std::vector<threevector>& XYZ,
      std::vector<double>& P)
      {
         outputfunc::write_banner("Reading xyzp float data:");

         ifstream binary_instream;
         filefunc::open_binaryfile(xyzp_filename,binary_instream);

         int n_null_values=0;
         float curr_x,curr_y,curr_z,curr_p;
         int n=0;
         while (!binary_instream.eof())
         {
            filefunc::readobject(binary_instream,curr_x);
            filefunc::readobject(binary_instream,curr_y);
            filefunc::readobject(binary_instream,curr_z);
            XYZ.push_back(threevector(curr_x,curr_y,curr_z));

// In cleaned versions of xyzp datafiles, we indicate null
// probabilities with xyzpfunc::null_value values.  So when reading
// back in cleaned files, we interpret any negative probability value
// as null...

            filefunc::readobject(binary_instream,curr_p);
            if (curr_p >= 0)
            {

// As of 1/20/05, we no longer follow the old Group 94 convention of
// multiplying all p-values by 10 before writing them out to XYZP
// files.  So we no longer need to divide p-values by 10 when reading
// them in from XYZP files:

               P.push_back(curr_p);
//               P.push_back(0.1*curr_p);
            }
            else
            {
               n_null_values++;
               P.push_back(xyzpfunc::null_value);
            }
            n++;
            
         } // loop over index n labeling XYZP points
         binary_instream.close();  
      }

// ---------------------------------------------------------------------
// This next overloaded version of read_xyzp_float_data fills up a
// dynamically generated STL vector with fourvector containing XYZP
// data read from the input .xyzp file specified by xyzp_filename:

   vector<fourvector>* read_xyzp_float_data(
      string xyzp_filename,double pnull_threshold)
      {
         vector<fourvector>* xyzp_pnt_ptr=new vector<fourvector>;
         read_xyzp_float_data(xyzp_filename,pnull_threshold,xyzp_pnt_ptr);
         return xyzp_pnt_ptr;
      }

// ---------------------------------------------------------------------
   void read_xyzp_float_data(
      string xyzp_filename,double pnull_threshold,
      vector<fourvector>* xyzp_pnt_ptr)
      {
         ifstream binary_instream;

         bool file_opened=false;
         do
         {
            filefunc::gunzip_file_if_gzipped(xyzp_filename);
            file_opened=filefunc::open_binaryfile(
               xyzp_filename,binary_instream);
         }
         while (!file_opened);

         unsigned int npoints=npoints_inside_xyzp_file(xyzp_filename);
         xyzp_pnt_ptr->reserve(npoints+xyzp_pnt_ptr->size());

         int n_null_values=0;
         float curr_x,curr_y,curr_z,curr_p;
         fourvector curr_point;
         for (unsigned int n=0; n<npoints; n++)
         {
            filefunc::readobject(binary_instream,curr_x);
            filefunc::readobject(binary_instream,curr_y);
            filefunc::readobject(binary_instream,curr_z);
            filefunc::readobject(binary_instream,curr_p);

            curr_point.put(0,curr_x);
            curr_point.put(1,curr_y);
            curr_point.put(2,curr_z);

// In cleaned versions of xyzp datafiles, we indicate null
// probabilities with xyzpfunc::null_value values.  So when reading
// back in cleaned files, we interpret any probability value less than
// pnull_threshold as null...

            if (curr_p >= pnull_threshold)
            {
               curr_point.put(3,curr_p);
            }
            else
            {
               n_null_values++;
               curr_point.put(3,xyzpfunc::null_value);
            }
            xyzp_pnt_ptr->push_back(curr_point);
         } // loop over index n labeling input points
         binary_instream.close();  

         cout << "Number of null values within input XYZP file = "
              << n_null_values << endl;
      }

// ---------------------------------------------------------------------
// Method read_just_xyz_float_data fills up a dynamically generated
// STL vector with threevectors containing XYZ data read from the
// input .xyzp file specified by xyzp_filename.  P information is
// discarded by this method.

   vector<threevector>* read_just_xyz_float_data(string xyzp_filename)
      {
         ifstream binary_instream;

         bool file_opened=false;
         do
         {
            filefunc::gunzip_file_if_gzipped(xyzp_filename);
            file_opened=filefunc::open_binaryfile(
               xyzp_filename,binary_instream);
         }
         while (!file_opened);
         unsigned int npoints=npoints_inside_xyzp_file(xyzp_filename);

         float curr_x,curr_y,curr_z,curr_p;
         vector<threevector>* xyz_pnt_ptr=new vector<threevector>;
         xyz_pnt_ptr->reserve(npoints);
         for (unsigned int n=0; n<npoints; n++)
         {
            filefunc::readobject(binary_instream,curr_x);
            filefunc::readobject(binary_instream,curr_y);
            filefunc::readobject(binary_instream,curr_z);
            filefunc::readobject(binary_instream,curr_p);
            xyz_pnt_ptr->push_back(threevector(curr_x,curr_y,curr_z));
         } // loop over index n labeling input points

         binary_instream.close();  
         return xyz_pnt_ptr;
      }

// ---------------------------------------------------------------------
// Method convert_xyzrgba_to_xyzp_data reads in an XYZRGBA file which
// is assumed to contain grey scale information.  For voxels with
// R=G=B, this method sets the output p value equal to R/256.  For all
// other voxels, p is set equal to null_p_value.

   vector<fourvector>* convert_xyzrgba_to_xyzp_data(
      string xyzrgba_filename,double null_p_value)
      {
         ifstream binary_instream;

         bool file_opened=false;
         do
         {
            filefunc::gunzip_file_if_gzipped(xyzrgba_filename);
            file_opened=filefunc::open_binaryfile(
               xyzrgba_filename,binary_instream);
         }
         while (!file_opened);
         unsigned int npoints=npoints_inside_xyzp_file(xyzrgba_filename);

         typedef union RGBAStruct
         {
               float p;
               unsigned char rgba[4];
         };
         union RGBAStruct RGBA;

         float curr_x,curr_y,curr_z,curr_p;
         fourvector curr_point;
         vector<fourvector>* xyzp_pnt_ptr=new vector<fourvector>;
         xyzp_pnt_ptr->reserve(npoints);
         for (unsigned int n=0; n<npoints; n++)
         {
            filefunc::readobject(binary_instream,curr_x);
            filefunc::readobject(binary_instream,curr_y);
            filefunc::readobject(binary_instream,curr_z);
            filefunc::readobject(binary_instream,RGBA.p);

            float r=static_cast<float>(RGBA.rgba[0]);
            float g=static_cast<float>(RGBA.rgba[1]);
            float b=static_cast<float>(RGBA.rgba[2]);
            if (nearly_equal(r,g) && nearly_equal(g,b))
            {
               curr_p=r/256.0;
            }
            else
            {
               curr_p=null_p_value;
            }
            curr_point.put(0,curr_x);
            curr_point.put(1,curr_y);
            curr_point.put(2,curr_z);
            curr_point.put(3,curr_p);

            xyzp_pnt_ptr->push_back(curr_point);
         } // loop over index n labeling input points

         binary_instream.close();  
         return xyzp_pnt_ptr;
      }

// ---------------------------------------------------------------------
   void read_xyz_double_data(unsigned int npoints,string xyzp_filename,
                             double x[],double y[],double z[])
      {
         ifstream binary_instream;
         filefunc::open_binaryfile(xyzp_filename,binary_instream);
         
         double curr_x,curr_y,curr_z;
         for (unsigned int n=0; n<npoints; n++)
         {
            filefunc::readobject(binary_instream,curr_x);
            filefunc::readobject(binary_instream,curr_y);
            filefunc::readobject(binary_instream,curr_z);
            x[n]=curr_x;
            y[n]=curr_y;
            z[n]=curr_z;
         }
         binary_instream.close();  
      }

// ---------------------------------------------------------------------
// Method fill_image_with_z_and_p_values first initializes all pixels
// to null_value.  It then scans through the input x[], y[] and z[]
// arrays and distributes their contents into the *ztwoDarray_ptr
// twoDarray.  This method similarly fills up the *ptwoDarray_ptr
// array with probability information contained in the input p[]
// array.  This method returns the number of coincidences which it
// encounters as it is filling up the twoDarrays.

   int fill_image_with_z_and_p_values(
      unsigned int npoints,double x[],double y[],double z[],double p[],
      twoDarray* ztwoDarray_ptr,twoDarray* ptwoDarray_ptr)
      {
         ztwoDarray_ptr->initialize_values(xyzpfunc::null_value);
         ptwoDarray_ptr->initialize_values(xyzpfunc::null_value);
         unsigned int px,py;
         int n_coincidence=0;

         for (unsigned int n=0; n<npoints; n++)
         {

// Recall "snow" points within the raw input 3D point cloud have had
// their z values reset to null_value and should not be included
// within the ladar image:

            if (ztwoDarray_ptr->point_to_pixel(x[n],y[n],px,py) && 
                z[n] > xyzpfunc::null_value)
            {
               double currz=ztwoDarray_ptr->get(px,py);
               if (currz != xyzpfunc::null_value) n_coincidence++;
               ztwoDarray_ptr->put(px,py,z[n]);
               ptwoDarray_ptr->put(px,py,p[n]);
            }
         }
   
         outputfunc::newline();
         cout << "Number of chimney point coincidences = " 
              << n_coincidence << endl;
         outputfunc::newline();
   
         return n_coincidence;
      }

   int fill_image_with_z_and_p_values(
      const vector<threevector>& XYZ,const vector<double>& p,
      twoDarray* ztwoDarray_ptr,twoDarray* ptwoDarray_ptr)
      {
         outputfunc::write_banner("Filling image with z & p values:");
         ztwoDarray_ptr->initialize_values(xyzpfunc::null_value);
         ptwoDarray_ptr->initialize_values(xyzpfunc::null_value);

         int n_coincidence=0;
         for (unsigned int n=0; n<XYZ.size(); n++)
         {

// Recall "snow" points within the raw input 3D point cloud have had
// their z values reset to null_value and should not be included
// within the ladar image:

            unsigned int px,py;
            if (ztwoDarray_ptr->point_to_pixel(
               XYZ[n].get(0),XYZ[n].get(1),px,py) &&
                XYZ[n].get(2) > xyzpfunc::null_value)
            {
               double currz=ztwoDarray_ptr->get(px,py);
               if (currz != xyzpfunc::null_value) n_coincidence++;
               ztwoDarray_ptr->put(px,py,XYZ[n].get(2));
               ptwoDarray_ptr->put(px,py,p[n]);
            }
         }
   
         outputfunc::newline();
         cout << "Number of chimney point coincidences = " 
              << n_coincidence << endl;
         outputfunc::newline();
   
         return n_coincidence;
      }

// ---------------------------------------------------------------------
   int fill_image_with_z_and_p_values(
      osg::Vec3Array* vertices_ptr,const osg::FloatArray* probs_ptr,
      twoDarray* ztwoDarray_ptr,twoDarray* ptwoDarray_ptr)
      {
//         cout << "inside xyzpfunc::fill_image_with_z_and_p_values()"
//              << endl;

         ztwoDarray_ptr->initialize_values(xyzpfunc::null_value);
         ptwoDarray_ptr->initialize_values(xyzpfunc::null_value);
         unsigned int px,py;
         int n_coincidence=0;

         for (unsigned int n=0; n<vertices_ptr->size(); n++)
         {

// Recall "snow" points within the raw input 3D point cloud have had
// their z values reset to null_value and should not be included
// within the ladar image:

            if (ztwoDarray_ptr->point_to_pixel(
                vertices_ptr->at(n).x(),vertices_ptr->at(n).y(),px,py) &&
                vertices_ptr->at(n).z() > xyzpfunc::null_value)
            {
               double currz=ztwoDarray_ptr->get(px,py);
               if (currz != xyzpfunc::null_value) n_coincidence++;

// Since deltax & deltay spacing in ztwoDarray is generally coarser
// than that in the cloud's *vertices_ptr, multiple voxels are mapped
// to a single pixel.  We take the pixel's z value to equal the
// maximum of the voxels'.

               ztwoDarray_ptr->put(
                  px,py,basic_math::max(static_cast<double>(vertices_ptr->at(n).z()),
                            currz));
               if (probs_ptr != NULL)
               {
                  ptwoDarray_ptr->put(px,py,probs_ptr->at(n));
               }
            }
         }
   
         outputfunc::newline();
         cout << "Number of chimney point coincidences = " 
              << n_coincidence << endl;
         outputfunc::newline();
   
         return n_coincidence;
      }

// ---------------------------------------------------------------------
   int fill_image_with_z_values(
      osg::Vec3Array* vertices_ptr,twoDarray* ztwoDarray_ptr)
      {
//         cout << "inside xyzpfunc::fill_image_with_z_values()" << endl;

         ztwoDarray_ptr->initialize_values(xyzpfunc::null_value);
         unsigned int px,py;
         int n_coincidence=0;

         for (unsigned int n=0; n<vertices_ptr->size(); n++)
         {

// Recall "snow" points within the raw input 3D point cloud have had
// their z values reset to null_value and should not be included
// within the ladar image:

            if (ztwoDarray_ptr->point_to_pixel(
                vertices_ptr->at(n).x(),vertices_ptr->at(n).y(),px,py) &&
                vertices_ptr->at(n).z() > xyzpfunc::null_value)
            {
               double currz=ztwoDarray_ptr->get(px,py);
               if (currz != xyzpfunc::null_value) n_coincidence++;

// Since deltax & deltay spacing in ztwoDarray is generally coarser
// than that in the cloud's *vertices_ptr, multiple voxels are mapped
// to a single pixel.  We take the pixel's z value to equal the
// maximum of the voxels'.

               ztwoDarray_ptr->put(
                  px,py,basic_math::max(static_cast<double>(vertices_ptr->at(n).z()),
                            currz));
            }
         }
   
         outputfunc::newline();
         cout << "Number of chimney point coincidences = " 
              << n_coincidence << endl;
         outputfunc::newline();
   
         return n_coincidence;
      }

// ==========================================================================
// XYZP file output methods
// ==========================================================================

// Method write_xyz_data takes in STL vectors X,Y,Z.  It writes to an
// output XYZ file whose name is passed as an input argument.  We
// hacked together this method in Jan 2009 for sending NYC points to
// Noah Snavely.

   void write_xyz_ascii_data(
      const vector<fourvector>* xyzp_pnt_ptr,string xyz_filename)
      {
         ofstream outstream;
         outstream.precision(12);
         filefunc::openfile(xyz_filename,outstream);

         for (unsigned int n=0; n<xyzp_pnt_ptr->size(); n++)
         {
            if (n%1000000==0) cout << n/1000000 << " " << flush;
            fourvector curr_xyzp(xyzp_pnt_ptr->at(n));
            if (curr_xyzp.get(2) > xyzpfunc::null_value)
            {
               outstream << curr_xyzp.get(0) << "   "
                         << curr_xyzp.get(1) << "   "
                         << curr_xyzp.get(2) << endl;
            } // curr_Z > xyzpfunc::null_value conditional
         } // loop over index n labeling points within input STL vectors
         cout << endl;
         
         filefunc::closefile(xyz_filename,outstream);

         cout << "Number of points written to output ascii file = " 
              << xyzp_pnt_ptr->size() << endl;
      }

   void write_xyz_data(
      const vector<double>& X,const vector<double>& Y,const vector<double>& Z,
      string xyz_filename)
      {
         ofstream binary_outstream;
         filefunc::open_binaryfile(xyz_filename,binary_outstream);

         for (unsigned int n=0; n<X.size(); n++)
         {
            if (Z[n] > xyzpfunc::null_value)
            {
               threevector curr_xyz_point(X[n],Y[n],Z[n]);
               write_single_xyz_point(
                  binary_outstream,curr_xyz_point);
            } // Z[n] > xyzpfunc::null_value conditional
         } // loop over index n labeling points within input STL vectors
            
         binary_outstream.close();  

         cout << "Number of points written to output binary file = " 
              << X.size() << endl;
      }

// ---------------------------------------------------------------------
// Method write_xyz_data takes in STL vectors X,Y,Z and boolean STL
// vector include_point_flag.  It writes to an output XYZP file whose
// name is passed as an input argument those XYZ points for which
// include_point_flag==true.

   void write_xyz_data(
      const vector<double>& X,const vector<double>& Y,const vector<double>& Z,
      const vector<bool>& include_point_flag,string xyzp_filename)
      {
         ofstream binary_outstream;
         filefunc::open_binaryfile(xyzp_filename,binary_outstream);

         unsigned int npoints_written=0;
         float zmax=2*NEGATIVEINFINITY;
         float zmin=2*POSITIVEINFINITY;
         float pmax=2*NEGATIVEINFINITY;
         float pmin=2*POSITIVEINFINITY;

         float pfloat=1;
         for (unsigned int n=0; n<X.size(); n++)
         {
            if (Z[n] > xyzpfunc::null_value && include_point_flag[n])
            {
               float xfloat=X[n];
               float yfloat=Y[n];
               float zfloat=Z[n];
               write_single_xyzp_point(
                  binary_outstream,xfloat,yfloat,zfloat,pfloat,
                  zmax,zmin,pmax,pmin,npoints_written);
            } // Z[n] > xyzpfunc::null_value && include_point_flag conditional
         } // loop over index n labeling points within input STL vectors
            
         binary_outstream.close();  

         cout << "Maximum z value written out = " << zmax << endl;
         cout << "Minimum z value written out = " << zmin << endl;
         cout << "Maximum p value written out = " << pmax << endl;
         cout << "Minimum p value written out = " << pmin << endl;
         cout << "Number of points written to output binary file = " 
              << npoints_written << endl;
      }

// ---------------------------------------------------------------------
// Method write_xyzp_data takes in a twoDarray which contains
// z=z(x,y).  It writes out a binary xyzp file which can be read in by
// the Group 94 dataviewer.  The output file is gzipped in order to
// reduce storage space.

   void write_xyzp_data(
      const twoDarray* ztwoDarray_ptr,const twoDarray* ptwoDarray_ptr,
      string xyzp_filename,bool p_represents_genuine_prob,
      bool gzip_output_file)
      {
         outputfunc::write_banner("Writing xyzp data:");
         cout.precision(10);

         cout << "*ptwoDarray_ptr = " << *ptwoDarray_ptr << endl;
         
         ofstream binary_outstream;
         filefunc::open_binaryfile(xyzp_filename,binary_outstream);

         unsigned int npoints_written=0;
         float zmax=2*NEGATIVEINFINITY;
         float zmin=2*POSITIVEINFINITY;
         float pmax=2*NEGATIVEINFINITY;
         float pmin=2*POSITIVEINFINITY;

         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {

// On 6/8/04, we realized (the hard way!) that the following
// double conditional effectively prohibits us from reading in some
// output xyzp file and blindly using it to continue some long
// computation which we often want to break apart.  Recall that median
// filling of ALIRT data gaps leads to a smooth z-surface.  But no
// corresponding filling of p-data is performed early on in the
// algorithm flow.  The following double conditional prevents a point
// with a filled z-value but null_valued p from being written out.
// Yet such points are important for tree/building voxel
// classification...

// Experiment at 2:24 on Tuesday, June 8: Try to write out xyzp points
// to file even if p-value=NEGATIVEINFINITY.  Altered dataviewer to
// reset such quadruples to (0,0,0,0).  Do this for output-input xyzp
// file purposes...

               if (ztwoDarray_ptr->get(px,py) > 0.99*xyzpfunc::null_value)
               {
                  double x,y;
                  ztwoDarray_ptr->pixel_to_point(px,py,x,y);
                  float xfloat=static_cast<float>(x);
                  float yfloat=static_cast<float>(y);
                  float zfloat=static_cast<float>(
                     ztwoDarray_ptr->get(px,py));
                  float pfloat=static_cast<float>(
                     ptwoDarray_ptr->get(px,py));

// As of June 03, Group 94 xyzp files have probability values which
// range from 0 to 10 rather than from 0 to 1:
                  
                  if (p_represents_genuine_prob)
                  {
                     pfloat=basic_math::max(float(0.0),pfloat);
                     pfloat=basic_math::min(float(1.0),pfloat);

// As of 1/20/05, we no longer follow the old Group 94 convention of
// multiplying all p-values by 10 before writing them out to XYZP
// files:

//                      if (pfloat >= 0) pfloat *= 10;

                     write_single_xyzp_point(
                        binary_outstream,xfloat,yfloat,zfloat,pfloat,
                        zmax,zmin,pmax,pmin,npoints_written);
                  }
                  else
                  {
                     write_single_xyzp_point(
                        binary_outstream,xfloat,yfloat,zfloat,pfloat,
                        zmax,zmin,pmax,pmin,npoints_written);
                  }
               } // ztwoDarray(px,py) > xyzpfunc::null_value conditional
            } // loop over index py
         } // loop over index px
         binary_outstream.close();  
         if (gzip_output_file) filefunc::gzip_file(xyzp_filename);
         
         cout << "mdim*ndim = " << ztwoDarray_ptr->get_mdim()*
            ztwoDarray_ptr->get_ndim() << endl;
         cout << "Maximum z value written out = " << zmax << endl;
         cout << "Minimum z value written out = " << zmin << endl;
         cout << "Maximum p value written out = " << pmax << endl;
         cout << "Minimum p value written out = " << pmin << endl;
         cout << "Number of points written to output binary file = " 
              << npoints_written << endl;
      }

// ---------------------------------------------------------------------
// This overloaded version of method write_xyzp_data takes in separate
// one-dimensional x, y, z and p arrays.  It writes out a binary xyzp
// file which can be read in by the Group 94 dataviewer.  The output
// file is gzipped in order to reduce storage space.

   void write_xyzp_data(
      string xyzp_filename,vector<double>* X_ptr,vector<double>* Y_ptr,
      vector<double>* Z_ptr,vector<double>* P_ptr)
      {
         ofstream binary_outstream;
         filefunc::open_binaryfile(xyzp_filename,binary_outstream);

         for (unsigned int n=0; n<Z_ptr->size(); n++)
         {
            float xfloat=X_ptr->at(n);
            float yfloat=Y_ptr->at(n);
            float zfloat=Z_ptr->at(n);
            float pfloat=P_ptr->at(n);

            write_single_xyzp_point(
               binary_outstream,xfloat,yfloat,zfloat,pfloat);
         } // loop over index n labeling point within 3D cloud
            
         binary_outstream.close();  
         filefunc::gzip_file(xyzp_filename);
      }

   void write_xyzp_data(
      unsigned int n_xyz_points,double x[],double y[],double z[],double p[],
      string xyzp_filename,bool p_represents_genuine_prob)
      {
         ofstream binary_outstream;
         filefunc::open_binaryfile(xyzp_filename,binary_outstream);

         unsigned int npoints_written=0;
         float zmax=2*NEGATIVEINFINITY;
         float zmin=2*POSITIVEINFINITY;
         float pmax=2*NEGATIVEINFINITY;
         float pmin=2*POSITIVEINFINITY;

         for (unsigned int n=0; n<n_xyz_points; n++)
         {
            if (z[n] > xyzpfunc::null_value)
            {
               float xfloat=x[n];
               float yfloat=y[n];
               float zfloat=z[n];
               float pfloat=p[n];

// As of June 03, Group 94 xyzp files have probability values which
// range from 0 to 10 rather than from 0 to 1:
                  
               if (p_represents_genuine_prob)
               {
//                  if (pfloat >= 0) pfloat *= 10;

                  pfloat=basic_math::max(float(0.0),pfloat);
                  pfloat=basic_math::min(float(1.0),pfloat);
                  if (pfloat >= 0) pfloat *= 10;

                  write_single_xyzp_point(
                     binary_outstream,xfloat,yfloat,zfloat,pfloat,
                     zmax,zmin,pmax,pmin,npoints_written);
               }
               else
               {
                  if (pfloat > xyzpfunc::null_value)
                  {
                     write_single_xyzp_point(
                        binary_outstream,xfloat,yfloat,zfloat,pfloat,
                        zmax,zmin,pmax,pmin,npoints_written);
                  }
               }
            } // z[n] > xyzpfunc::null_value conditional
         } // loop over index n labeling point within 3D cloud
            
         binary_outstream.close();  
         filefunc::gzip_file(xyzp_filename);

         cout << "Maximum z value written out = " << zmax << endl;
         cout << "Minimum z value written out = " << zmin << endl;
         cout << "Maximum p value written out = " << pmax << endl;
         cout << "Minimum p value written out = " << pmin << endl;
         cout << "Number of points written to output binary file = " 
              << npoints_written << endl;
      }

   void write_xyzp_data(
      const vector<threevector>& XYZ,const vector<double>& P,
      string xyzp_filename,bool p_represents_genuine_prob,
      bool gzip_output_file)
      {
         cout << "inside xyzpfunc::write_xyzp_data()" << endl;
         ofstream binary_outstream;
         filefunc::open_binaryfile(xyzp_filename,binary_outstream);

         cout << "XYZ.size() = " << XYZ.size() << endl;
         cout << "P.size() = " << P.size() << endl;

         unsigned int npoints_written=0;
         float zmax=2*NEGATIVEINFINITY;
         float zmin=2*POSITIVEINFINITY;
         float pmax=2*NEGATIVEINFINITY;
         float pmin=2*POSITIVEINFINITY;

         for (unsigned int n=0; n<XYZ.size(); n++)
         {
            if (XYZ[n].get(2) > xyzpfunc::null_value)
            {
               float xfloat=XYZ[n].get(0);
               float yfloat=XYZ[n].get(1);
               float zfloat=XYZ[n].get(2);
               float pfloat=P[n];

// As of June 03, Group 94 xyzp files have probability values which
// range from 0 to 10 rather than from 0 to 1:
                  
               if (p_represents_genuine_prob)
               {
//                  if (pfloat >= 0) pfloat *= 10;

                  pfloat=basic_math::max(float(0.0),pfloat);
                  pfloat=basic_math::min(float(1.0),pfloat);
                  if (pfloat >= 0) pfloat *= 10;

//                  cout << "n = " << n 
//                       << " x = " << xfloat << " y = " << yfloat
//                       << " z = " << zfloat << " p = " << pfloat
//                       << endl;
                  
                  write_single_xyzp_point(
                     binary_outstream,xfloat,yfloat,zfloat,pfloat,
                     zmax,zmin,pmax,pmin,npoints_written);
               }
               else
               {
                  if (pfloat > xyzpfunc::null_value)
                  {
                     write_single_xyzp_point(
                        binary_outstream,xfloat,yfloat,zfloat,pfloat,
                        zmax,zmin,pmax,pmin,npoints_written);
                  }
               }
            } // z[n] > xyzpfunc::null_value conditional
         } // loop over index n labeling point within 3D cloud
            
         binary_outstream.close();  
         if (gzip_output_file) filefunc::gzip_file(xyzp_filename);

         cout << "Maximum z value written out = " << zmax << endl;
         cout << "Minimum z value written out = " << zmin << endl;
         cout << "Maximum p value written out = " << pmax << endl;
         cout << "Minimum p value written out = " << pmin << endl;
         cout << "Number of points written to output binary file = " 
              << npoints_written << endl;
      }

// ---------------------------------------------------------------------
// This overloaded version of write_xyzp_data takes in an STL vector
// containing fourvector of XYZP points.  It writes each one to the
// output xyzp file.

   void write_xyz_data(
      string xyz_filename,vector<threevector>* xyz_pnt_ptr,
      bool gzip_output_file)
      {
         ofstream binary_outstream;
         binary_outstream.open(xyz_filename.c_str(),ios::app|ios::binary);

         for (unsigned int n=0; n<xyz_pnt_ptr->size(); n++)
         {
            write_single_xyz_point(binary_outstream,(*xyz_pnt_ptr)[n]);
         } // loop over index n labeling point within 3D cloud
         binary_outstream.close();  
         if (gzip_output_file) filefunc::gzip_file(xyz_filename);
      }

   void write_xyzp_data(
      string xyzp_filename,vector<fourvector>* xyzp_pnt_ptr,
      bool gzip_output_file)
      {
         ofstream binary_outstream;
         binary_outstream.open(xyzp_filename.c_str(),ios::app|ios::binary);

         for (unsigned int n=0; n<xyzp_pnt_ptr->size(); n++)
         {
            write_single_xyzp_point(
               binary_outstream,(*xyzp_pnt_ptr)[n]);
         } // loop over index n labeling point within 3D cloud
         binary_outstream.close();  
         if (gzip_output_file) filefunc::gzip_file(xyzp_filename);
      }

   void write_xyzp_data(
      string xyzp_filename,vector<genvector>* xyzp_pnt_ptr,
      bool gzip_output_file)
      {
         ofstream binary_outstream;
         binary_outstream.open(xyzp_filename.c_str(),ios::app|ios::binary);

         for (unsigned int n=0; n<xyzp_pnt_ptr->size(); n++)
         {
            write_single_xyzp_point(
               binary_outstream,(*xyzp_pnt_ptr)[n]);
         } // loop over index n labeling point within 3D cloud
         binary_outstream.close();  
         if (gzip_output_file) filefunc::gzip_file(xyzp_filename);
      }

// ---------------------------------------------------------------------
// Method write_local_xyzp_data takes in a twoDarray which contains
// z=z(x,y).  It also takes in an origin point (x0,y0) as well as a
// radius r.  It writes to an output binary xyzp file only those
// points which lie within a circle of radius r from the origin.

   void write_local_xyzp_data(
      double x0,double y0,double r0,
      const twoDarray* ztwoDarray_ptr,const twoDarray* ptwoDarray_ptr,
      string xyzp_filename,bool p_represents_genuine_prob)
      {
         ofstream binary_outstream;
         filefunc::open_binaryfile(xyzp_filename,binary_outstream);

         unsigned int npoints_written=0;
         float zmax=2*NEGATIVEINFINITY;
         float zmin=2*POSITIVEINFINITY;
         float pmax=2*NEGATIVEINFINITY;
         float pmin=2*POSITIVEINFINITY;

         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {

// On 6/8/04, we realized (the hard way!) that the following
// double conditional effectively prohibits us from reading in some
// output xyzp file and blindly using it to continue some long
// computation which we often want to break apart.  Recall that median
// filling of ALIRT data gaps leads to a smooth z-surface.  But no
// corresponding filling of p-data is performed early on in the
// algorithm flow.  The following double conditional prevents a point
// with a filled z-value but null_valued p from being written out.
// Yet such points are important for tree/building voxel
// classification...

// Experiment at 2:24 on Tuesday, June8: Try to write out xyzp points
// to file even if p-value=NEGATIVEINFINITY.  Altered dataveiwer to
// reset such quadruples to (0,0,0,0).  Do this for output-input xyzp
// file purposes...

               if (ztwoDarray_ptr->get(px,py) > 0.99*xyzpfunc::null_value)
//               if (ztwoDarray_ptr->get(px,py) > 0.99*xyzpfunc::null_value &&
//                   ptwoDarray_ptr->get(px,py) > 0.99*xyzpfunc::null_value)
               {
                  double x,y;
                  ztwoDarray_ptr->pixel_to_point(px,py,x,y);
                  float xfloat=x;
                  float yfloat=y;
                  float zfloat=ztwoDarray_ptr->get(px,py);
                  float pfloat=ptwoDarray_ptr->get(px,py);

                  float curr_radius=sqrt(sqr(x-x0)+sqr(y-y0));
                  if (curr_radius < r0)
                  {
                     
// As of June 03, Group 94 xyzp files have probability values which
// range from 0 to 10 rather than from 0 to 1:
                  
                     if (p_represents_genuine_prob)
                     {
                        pfloat=basic_math::max(float(0.0),pfloat);
                        pfloat=basic_math::min(float(1.0),pfloat);
                        if (pfloat >= 0) pfloat *= 10;

                        write_single_xyzp_point(
                           binary_outstream,xfloat,yfloat,zfloat,pfloat,
                           zmax,zmin,pmax,pmin,npoints_written);
                     }
                     else
                     {
                        write_single_xyzp_point(
                           binary_outstream,xfloat,yfloat,zfloat,pfloat,
                           zmax,zmin,pmax,pmin,npoints_written);
                     }
                  } // curr_radius < r0 conditional
               } // ztwoDarray(px,py) > xyzpfunc::null_value conditional
            } // loop over index py
         } // loop over index px
         binary_outstream.close();  
         filefunc::gzip_file(xyzp_filename);

         cout << "mdim*ndim = " << ztwoDarray_ptr->get_mdim()*
            ztwoDarray_ptr->get_ndim() << endl;
         cout << "Maximum z value written out = " << zmax << endl;
         cout << "Minimum z value written out = " << zmin << endl;
         cout << "Maximum p value written out = " << pmax << endl;
         cout << "Minimum p value written out = " << pmin << endl;
         cout << "Number of points written to output binary file = " 
              << npoints_written << endl;
      }

// ---------------------------------------------------------------------
// This overloaded version of write_local_xyzp_data takes in an STL
// vector containing fourvector of XYZP points.  It writes each one
// which lies within some 2D radius rho of some specified 3D position
// to the output xyzp file.

   void write_local_xyzp_data(
      const threevector& center_posn,double rho,
      string xyzp_filename,vector<fourvector>* xyzp_pnt_ptr,
      bool gzip_output_file)
      {
         unsigned int npoints_written=0;
         
         ofstream binary_outstream;
         binary_outstream.open(xyzp_filename.c_str(),ios::app|ios::binary);

         for (unsigned int n=0; n<xyzp_pnt_ptr->size(); n++)
         {
            fourvector curr_point;
            curr_point=(*xyzp_pnt_ptr)[n];
            double curr_rhosq=sqr(center_posn.get(0)-curr_point.get(0))
               +sqr(center_posn.get(1)-curr_point.get(1));
            if (curr_rhosq < sqr(rho))
            {
               write_single_xyzp_point(binary_outstream,(*xyzp_pnt_ptr)[n]);
               npoints_written++;
            }
         } // loop over index n labeling point within 3D cloud
         binary_outstream.close();  

//         cout << "npoints_written = " << npoints_written << endl;
         if (gzip_output_file) filefunc::gzip_file(xyzp_filename);
      }

   void write_local_xyzp_data(
      const threevector& center_posn,double rho,
      string xyzp_filename,vector<genvector>* xyzp_pnt_ptr,
      bool gzip_output_file)
      {
         unsigned int npoints_written=0;
         
         ofstream binary_outstream;
         binary_outstream.open(xyzp_filename.c_str(),ios::app|ios::binary);

         for (unsigned int n=0; n<xyzp_pnt_ptr->size(); n++)
         {
            genvector curr_point(4);
            curr_point=(*xyzp_pnt_ptr)[n];
            double curr_rhosq=sqr(center_posn.get(0)-curr_point.get(0))
               +sqr(center_posn.get(1)-curr_point.get(1));
            if (curr_rhosq < sqr(rho))
            {
               write_single_xyzp_point(binary_outstream,(*xyzp_pnt_ptr)[n]);
               npoints_written++;
            }
         } // loop over index n labeling point within 3D cloud
         binary_outstream.close();  

//         cout << "npoints_written = " << npoints_written << endl;
         if (gzip_output_file) filefunc::gzip_file(xyzp_filename);
      }

// ---------------------------------------------------------------------
// Method write_annotated_xyzp_data initially copies z data within
// input twoDarray *ztwoDarray_ptr into a temporary *ftwoDarray_ptr
// twoDarray.  It then adds a constant height offset to any ftwoDarray
// element whose corresponding entry within input twoDarray
// *ptwoDarray_ptr exceeds 999.  This method writes (x,y,z,f)
// quadruples to an output binary file.

   void write_annotated_xyzp_data(
      const twoDarray* ztwoDarray_ptr,const twoDarray* ptwoDarray_ptr,
      string xyzp_filename,double z_threshold)
      {
         twoDarray* ftwoDarray_ptr=new twoDarray(ptwoDarray_ptr);
         ztwoDarray_ptr->copy(ftwoDarray_ptr);
         
         const double constant_height=50;	// meters
         
         for (unsigned int px=0; px<ztwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ztwoDarray_ptr->get_ndim(); py++)
            {
               if (ptwoDarray_ptr->get(px,py) > 999 &&
                   ztwoDarray_ptr->get(px,py) < z_threshold)
               {
                  ftwoDarray_ptr->put(px,py,constant_height);
               }
            }
         }
         write_xyzp_data(
            ztwoDarray_ptr,ftwoDarray_ptr,xyzp_filename,false);
      }

// This overloaded version of write_annotated_xyzp_data takes in
// unmarked z and p data within twoDarrays *ztwoDarray_ptr and
// *ptwoDarray_ptr.  It also takes in annotation information within
// input twoDarray *ftwoDarray_ptr.  This method annotates the p-data
// image rather than the z-data image.  

   void write_annotated_xyzp_data(
      const twoDarray* ztwoDarray_ptr,const twoDarray* ptwoDarray_ptr,
      const twoDarray* ftwoDarray_ptr,string xyzp_filename,
      double z_threshold)
      {
         twoDarray* gtwoDarray_ptr=new twoDarray(ptwoDarray_ptr);
         ptwoDarray_ptr->copy(gtwoDarray_ptr);
         
         const double constant_annotation_value=1.5;	
         
         for (unsigned int px=0; px<ptwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ptwoDarray_ptr->get_ndim(); py++)
            {
               if (ftwoDarray_ptr->get(px,py) > 999 &&
                   ztwoDarray_ptr->get(px,py) < z_threshold)
               {
                  gtwoDarray_ptr->put(px,py,constant_annotation_value);
               }
            }
         }
         write_xyzp_data(
            ztwoDarray_ptr,gtwoDarray_ptr,xyzp_filename,false);
      }
   
// ---------------------------------------------------------------------
// Method write_single_xyz_point writes one (x,y,z) triple to the
// output binary_outstream.

   void write_single_xyz_point(
      ofstream& binary_outstream,float x,float y,float z)
      {
         filefunc::writeobject(binary_outstream,x);
         filefunc::writeobject(binary_outstream,y);
         filefunc::writeobject(binary_outstream,z);
      }
   void write_single_xyz_point(
      ofstream& binary_outstream,const threevector& curr_point)
      {
         float x=static_cast<float>(curr_point.get(0));
         float y=static_cast<float>(curr_point.get(1));
         float z=static_cast<float>(curr_point.get(2));
         write_single_xyz_point(binary_outstream,x,y,z);
      }

// ---------------------------------------------------------------------
// Method write_single_xyzp_point writes one (x,y,z,p) quadruple to
// the output binary_outstream.

   void write_single_xyzp_point(
      ofstream& binary_outstream,float x,float y,float z,float p)
      {
         filefunc::writeobject(binary_outstream,x);
         filefunc::writeobject(binary_outstream,y);
         filefunc::writeobject(binary_outstream,z);
         filefunc::writeobject(binary_outstream,p);
      }

   void write_single_xyzp_point(
      ofstream& binary_outstream,float x,float y,float z,float p,
      float& zmax,float& zmin,float& pmax,float& pmin,
      unsigned int& npoints_written)
      {
         write_single_xyzp_point(binary_outstream,x,y,z,p);
         zmax=basic_math::max(zmax,z);
         zmin=basic_math::min(zmin,z);
         pmax=basic_math::max(pmax,p);
         pmin=basic_math::min(pmin,p);
         npoints_written++;
      }

   void write_single_xyzp_point(
      ofstream& binary_outstream,const threevector& curr_point,double curr_p)
      {
         float x=static_cast<float>(curr_point.get(0));
         float y=static_cast<float>(curr_point.get(1));
         float z=static_cast<float>(curr_point.get(2));
         float p=static_cast<float>(curr_p);
         write_single_xyzp_point(binary_outstream,x,y,z,p);
      }

// These next overloaded versions of write_single_xyzp_point takes in
// a fourvector (or fourvector and writes its contents to
// binary_outstream:

   void write_single_xyzp_point(
      ofstream& binary_outstream,const fourvector& curr_point)
      {
         float x=static_cast<float>(curr_point.get(0));
         float y=static_cast<float>(curr_point.get(1));
         float z=static_cast<float>(curr_point.get(2));
         float p=static_cast<float>(curr_point.get(3));
         write_single_xyzp_point(binary_outstream,x,y,z,p);
      }

   void write_single_xyzp_point(
      ofstream& binary_outstream,const genvector& curr_point)
      {
         float x=static_cast<float>(curr_point.get(0));
         float y=static_cast<float>(curr_point.get(1));
         float z=static_cast<float>(curr_point.get(2));
         float p=static_cast<float>(curr_point.get(3));
         write_single_xyzp_point(binary_outstream,x,y,z,p);
      }

// ---------------------------------------------------------------------
// Method write_single_xyzrgba_point takes in XYZ and RGB information.
// If the color values range between [0,1], input boolean flag
// normalized_input_RGB_values should be set to true.  This method
// writes out the XYZ info as 3 floats.  It then writes out 4 bytes
// corresponding to RGB and alpha which we set equal to one.

   void write_single_xyzrgba_point(
      ofstream& binary_outstream,const threevector& curr_point,
      colorfunc::RGB curr_RGB,bool normalized_input_RGB_values)
      {
         float x=static_cast<float>(curr_point.get(0));
         float y=static_cast<float>(curr_point.get(1));
         float z=static_cast<float>(curr_point.get(2));
         filefunc::writeobject(binary_outstream,x);
         filefunc::writeobject(binary_outstream,y);
         filefunc::writeobject(binary_outstream,z);

         colorfunc::RGB_bytes curr_RGB_bytes=
            colorfunc::RGB_to_bytes(curr_RGB,normalized_input_RGB_values);
         const unsigned char alpha_byte=
            static_cast<unsigned char>(stringfunc::ascii_integer_to_char(
               255));

//         cout << curr_RGB.first*256 << " " 
//              << curr_RGB.second*256 << " " 
//              << curr_RGB.third*256 << endl;
//         cout << "R char = " 
//              << stringfunc::unsigned_char_to_ascii_integer(
//                 curr_RGB_bytes.first) 
//              << " G char = " 
//              << stringfunc::unsigned_char_to_ascii_integer(
//                 curr_RGB_bytes.second)
//              << " B char = " 
//              << stringfunc::unsigned_char_to_ascii_integer(
//                 curr_RGB_bytes.third) << endl;
            
         filefunc::writeobject(binary_outstream,curr_RGB_bytes.first);
         filefunc::writeobject(binary_outstream,curr_RGB_bytes.second);
         filefunc::writeobject(binary_outstream,curr_RGB_bytes.third);
         filefunc::writeobject(binary_outstream,alpha_byte);
      }

   void write_single_xyzrgba_point(
      ofstream& binary_outstream,const threevector& curr_point,
      osg::Vec4ub& curr_RGB)
      {
         float x=static_cast<float>(curr_point.get(0));
         float y=static_cast<float>(curr_point.get(1));
         float z=static_cast<float>(curr_point.get(2));
         filefunc::writeobject(binary_outstream,x);
         filefunc::writeobject(binary_outstream,y);
         filefunc::writeobject(binary_outstream,z);

         filefunc::writeobject(binary_outstream,curr_RGB.r());
         filefunc::writeobject(binary_outstream,curr_RGB.g());
         filefunc::writeobject(binary_outstream,curr_RGB.b());
         filefunc::writeobject(binary_outstream,curr_RGB.a());
      }

   void write_single_xyzrgba_point(
      ofstream& binary_outstream,const osg::Vec3& curr_point,
      osg::Vec4ub& curr_RGB)
      {
         float x=curr_point.x();
         float y=curr_point.y();
         float z=curr_point.z();
         filefunc::writeobject(binary_outstream,x);
         filefunc::writeobject(binary_outstream,y);
         filefunc::writeobject(binary_outstream,z);

         filefunc::writeobject(binary_outstream,curr_RGB.r());
         filefunc::writeobject(binary_outstream,curr_RGB.g());
         filefunc::writeobject(binary_outstream,curr_RGB.b());
         filefunc::writeobject(binary_outstream,curr_RGB.a());
      }

// ==========================================================================
// Height and intensity image fusion methods:
// ==========================================================================

// Method fuse_z_and_p_images takes in a height image *ztwoDarray_ptr
// along with a probability image *ptwoDarray_ptr whose values are
// assumed to range between 0.0 and 1.0.  This method returns a fused
// image which is meant to be viewed with our big hue and value
// colormap.  P information is indicated by coarse hue variations
// ranging from reds (p_lo) to blues (p_hi).  Fine hue variations are
// used to cyclically convey height information.

// We specifically implemented this method in July 04 in order to
// construct IED "danger maps".  The output from this method looks
// much better than that from the next "fuse_z_and_p_images" which
// tries to use hue+intensity or hue+saturation to simultaneously
// convey heights and probabilities.

   twoDarray* fuse_z_and_p_images(
      double p_hi,double p_lo,
      twoDarray const *ztwoDarray_ptr,twoDarray const *ptwoDarray_ptr)
      {
         outputfunc::write_banner("Fusing z and p images:");
         twoDarray* ftwoDarray_ptr=new twoDarray(ztwoDarray_ptr);   

         const double delta_z=0.2;	// meter
         const double delta_p=0.1251;
         const double delta_f=0.0035;
         const int nbins=basic_math::round((0.1-0.00)/delta_f)+1;

         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               double z=ztwoDarray_ptr->get(px,py);
               double p=p_lo+ptwoDarray_ptr->get(px,py)*(p_hi-p_lo);

               double f=xyzpfunc::null_value;
               if (z > 0.99*xyzpfunc::null_value  &&
                   p > 0.99*xyzpfunc::null_value)
               {
                  int p_bin=basic_math::mytruncate(p/delta_p);  
						// pbin=0,1,..,7
                  int z_bin=basic_math::round(z/delta_z);
                  int n=modulo(z_bin,nbins);
                  f=0.1*p_bin+n*delta_f;

//                  cout << "px = " << px << " py = " << py << " p = " << p
//                       << endl;
//                  cout << "pbin = " << p_bin 
//                       << " z_bin = " << z_bin << " n = " 
//                       << n << " f = " << f << endl << endl;

               }
               ftwoDarray_ptr->put(px,py,f);
            } // loop over index py
         } // loop over index px
         return ftwoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method fuse_z_and_binary_p_images takes in height and binary mask
// arrays *ztwoDarray_ptr and *ptwoDarray_ptr.  Low-p value regions
// are indicated by "cold" colors (cyan, blue, purple), while high-p
// value regions are indicated by "hot" colors (red, orange, yellow).
// Colors vary according to height.

   twoDarray* fuse_z_and_binary_p_images(
      twoDarray const *ztwoDarray_ptr,twoDarray const *ptwoDarray_ptr)
      {
         outputfunc::write_banner("Fusing z and binary p images:");
         twoDarray* ftwoDarray_ptr=new twoDarray(ztwoDarray_ptr);   
//         cout << "*ftwoDarray_ptr = " << *ftwoDarray_ptr << endl;

         const double delta_z=0.10;	// meter
//         const double delta_z=0.15;	// meter
//         const double delta_f=0.0035;
         const double delta_f=0.005;

// Map low p value onto "cold" colors (cyan,blue,purple):

         const double min_cold=0.60;
         const double max_cold=0.90;
         const int n_cold_bins=basic_math::round(
            (max_cold-min_cold)/delta_f)+1;

// Map high p value onto "hot" colors (red, orange, yellow):

         const double min_hot=0.0;
         const double max_hot=0.25;
         const int n_hot_bins=basic_math::round(
            (max_hot-min_hot)/delta_f)+1;

         cout << "n_cold_bins = " << n_cold_bins << endl;
         cout << "n_hot_bins = " << n_hot_bins << endl;

         int n_null_values=0;
//         const double p_lo=0.0;
//         const double p_hi=0.95;
         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               double z=ztwoDarray_ptr->get(px,py);
//               double p=p_lo+ptwoDarray_ptr->get(px,py)*(p_hi-p_lo);
               double p=ptwoDarray_ptr->get(px,py);

               double f=xyzpfunc::null_value;
               if (z > 0.99*xyzpfunc::null_value  &&
                   p > 0.99*xyzpfunc::null_value)
               {
                  int z_bin=basic_math::round(z/delta_z);

                  int n=modulo(z_bin,n_cold_bins);
                  f=min_cold+n*delta_f;
                  if (p > 0.5)
                  {
                     n=modulo(z_bin,n_hot_bins);
                     f=min_hot+n*delta_f;
                  }

//                  cout << "px = " << px << " py = " << py 
//                       << " z = " << z << " p = " << p 
//                       << " f = " << f << endl;
               }
               else
               {
                  n_null_values++;
               }
               ftwoDarray_ptr->put(px,py,f);
            } // loop over index py
         } // loop over index px

         cout << "n_null_values = " << n_null_values << endl;

         return ftwoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method fuse_z_and_p_images fuses height and detection probability
// data into a single image.  Hue and intensity [or saturation]
// properties of colors are used to display 4D data in a colored 2D
// image.  The results are returned within a dynamically generated
// twoDarray.

   twoDarray* fuse_z_and_p_images(
      double v_hi,double v_lo,double zmax,double zmin,
      twoDarray const *ztwoDarray_ptr,twoDarray const *ptwoDarray_ptr,
      bool map_prob_onto_intensity_rather_than_saturation)
      {
         outputfunc::write_banner("Fusing z and p images:");
         twoDarray* ftwoDarray_ptr=new twoDarray(ztwoDarray_ptr);   

         double h,s,v,f;
         double fmin=POSITIVEINFINITY;
         double fmax=NEGATIVEINFINITY;
         for (unsigned int px=0; px<ftwoDarray_ptr->get_mdim(); px++)
         {
            for (unsigned int py=0; py<ftwoDarray_ptr->get_ndim(); py++)
            {
               double z=ztwoDarray_ptr->get(px,py);
               double p=ptwoDarray_ptr->get(px,py);

               if (map_prob_onto_intensity_rather_than_saturation)
               {
                  convert_zp_to_hue_and_intensity(
                     zmax,zmin,z,p,v_hi,v_lo,h,v);
                  convert_hue_and_intensity_to_f(h,v,f);
               }
               else
               {
                  double s_hi=v_hi;	// map p_hi=1 to saturation s_hi
                  double s_lo=v_lo;     // map p_lo=0 to saturation s_lo
                  convert_zp_to_hue_and_saturation(
                     z,p,zmax,zmin,s_hi,s_lo,h,s);
                  convert_hue_and_saturation_to_f(h,s,f);
               }
               
               if (f > xyzpfunc::null_value)
               {
                  fmin=basic_math::min(fmin,f);
               }
               fmax=basic_math::max(fmax,f);
//               if (z > xyzpfunc::null_value)
//               {
//                  cout << "z = " << z << " p = " << p
//                       << " h = " << h << " v = " << v << " f = " << f
//                       << endl;
//               }

               ftwoDarray_ptr->put(px,py,f);
            } // loop over index py
         } // loop over index px
         cout << "fmin = " << fmin << " fmax = " << fmax << endl;
         return ftwoDarray_ptr;
      }

// ---------------------------------------------------------------------
// Method convert_zp_to_hue_intensity_and_saturation() takes in ladar
// height z information and returns a corresponding hue value h.  It
// also takes in ladar detection probability p and returns a
// saturation/intensity value s/v.  We attempt to exploit color
// degrees of information in order to display 4D information in a 2D
// image!

   void convert_zp_to_hue_intensity_and_saturation(
      double wrap_frac,double zmax,double zmin,double z,double p,
      double v_hi,double v_lo,double& h,double& v,double& s)
      {
//         cout << "inside xyzpfunc::convert_zp_to_hue_and_intensity()" << endl;
         
         double h_hi=360*wrap_frac;	// degs
         double h_lo=0;			// degs		(red)
         if (z==xyzpfunc::null_value || z < zmin || z > zmax)
         {
            h=xyzpfunc::null_value;
         }
         else
         {
            h=h_lo+(h_hi-h_lo)*(z-zmin)/(zmax-zmin);
//            cout << "zmin = " << zmin << " z = " << z
//                 << " zmax = " << zmax << " h = " << h << endl;
            h=basic_math::phase_to_canonical_interval(h,0,360);
         }

         if (p < 0)
         {
//            cout << "Warning in xyzpfunc::convert_zp_to_hue_and_intensity()"
//                 << endl;
//            cout << "z = " << z << " p = " << p << endl;
            v=v_lo;    
         }
         else if (p > 1)
         {
//            cout << "Warning in xyzpfunc::convert_zp_to_hue_and_intensity()"
//                 << endl;
//            cout << "z = " << z << " p = " << p << endl;
            v=v_hi;
         }
         else
         {
            v=v_lo+p*(v_hi-v_lo);

            double s_hi=1;
//            double s_lo=0.15;
            double s_lo=0.05;
            s=s_hi+p*(s_lo-s_hi);
         }

//         if (z > xyzpfunc::null_value)
//         {
//            cout << "z = " << z << " p = " << p 
//                 << " hue = " << h << " val = " << v << endl;
//         }
      }

   void convert_zp_to_hue_and_intensity(
      double zmax,double zmin,double z,double p,double v_hi,double v_lo,
      double& h,double& v,double hue_hi,double hue_lo)
      {
//         double hue_lo=0;		// degs		(red)
         if (z==xyzpfunc::null_value)
         {
            h=xyzpfunc::null_value;
         }
         else if (z < zmin)
         {
            h=hue_hi;
         }
         else if (z > zmax)
         {
            h=hue_lo;
         }
         else
         {
            h=hue_lo+(hue_hi-hue_lo)*(zmax-z)/(zmax-zmin);
         }

         if (p < 0)
         {
//            cout << "Warning in xyzpfunc::convert_zp_to_hue_and_intensity()"
//                 << endl;
//            cout << "z = " << z << " p = " << p << endl;
            v=v_lo;    
         }
         else if (p > 1)
         {
//            cout << "Warning in xyzpfunc::convert_zp_to_hue_and_intensity()"
//                 << endl;
//            cout << "z = " << z << " p = " << p << endl;
            v=v_hi;
         }
         else
         {
            v=v_lo+p*(v_hi-v_lo);
         }

//         if (z > xyzpfunc::null_value)
//         {
//            cout << "z = " << z << " p = " << p 
//                 << " hue = " << h << " val = " << v << endl;
//         }
      }

// ---------------------------------------------------------------------
// Method convert_zp_to_hue_and_saturation maps height values onto
// hues and probabilities onto saturation values.  P=0 [p=1] is mapped
// onto s=s_lo [s=s_hi].  

   void convert_zp_to_hue_and_saturation(
      double z,double p,double zmax,double zmin,
      double s_hi,double s_lo,double& h,double& s)
      {
         double h_hi=300;	// degs		(magenta)
         double h_lo=0;		// degs		(red)
         if (z==xyzpfunc::null_value || z < zmin || z > zmax)
         {
            h=xyzpfunc::null_value;
         }
         else
         {
            h=h_lo+(h_hi-h_lo)*(zmax-z)/(zmax-zmin);
         }

         if (p < 0)
         {
            s=s_lo;    
         }
         else if (p > 1)
         {
            s=s_hi;
         }
         else
         {
            s=s_lo+p*(s_hi-s_lo);
         }
//         cout << "inside convert_zp_to_hue_and_saturation" << endl;
//         cout << "p = " << p << " s = " << s << endl;
      }

// ---------------------------------------------------------------------
// Method convert_hue_and_intensity_to_f maps 2D input hue color and
// intensity information onto a 1D output "fused" function f.

   void convert_hue_and_intensity_to_f(double h,double v,double& f)
      {
         const int M=24;
         const int N=64;

         double h_hi=300;	// degs		(magenta)
         double h_lo=0;		// degs		(red)
         double dh=(h_hi-h_lo)/(N-1);

         double v_hi=1;
         double v_lo=0.0;
         double dv=(v_hi-v_lo)/(M-1);

         if (h==xyzpfunc::null_value)
         {
            f=xyzpfunc::null_value;
         }
         else
         {
            int n=basic_math::round((h-h_lo)/dh);
            double m=(v-v_lo)/dv;

            if (is_even(n))
            {
               f=n*M+m;
            }
            else
            {
               f=n*M+(M-m-1);
            }
         }
      }

// ---------------------------------------------------------------------
// Method convert_hue_and_saturation_to_f maps 2D input hue color and
// saturation information onto a 1D output "fused" function f.  Output
// from this method should be viewed using the "bighs" dataviewer
// colormap.

// Note: On 7/5/04, we spent quite a bit of time studying the colormap
// generated by this method.  For reasons we don't understand, the
// "saturation" appearance is unfortunately a strong function of hue
// index n.  In plain English, this means that two adjacent colors
// with the same saturation index m values but hue indices n and n+1
// can look markedly different.  The one with hue index n may appear
// to be nearly white.  Its neighbor with hue index n+1 may appear to
// have substantial hue content to it.  

// As of result of this unfortunate hue dependence of apparent
// saturation, we cannot use the hue-saturation colormap to fuse z and
// p information together in a satisfactory way.  Boo hoo...

   void convert_hue_and_saturation_to_f(double h,double s,double& f)
      {
         const int M=24;
         const int N=64;

         double h_hi=300;	// degs		(magenta)
         double h_lo=0;		// degs		(red)
         double dh=(h_hi-h_lo)/double(N-1);

         double s_hi=1;
         double s_lo=0.0;
         double ds=(s_hi-s_lo)/double(M-1);

         if (nearly_equal(h,xyzpfunc::null_value))
         {
            f=xyzpfunc::null_value;
         }
         else
         {
            int n=basic_math::round((h-h_lo)/dh);
            double m=(s-s_lo)/ds;

            if (is_even(n))
            {
               f=n*M+m;
            }
            else
            {
               f=n*M+(M-m-1);
            }

//            cout << "h = " << h << " s = " << s 
//                 << " n = " << n << " m = " << m << " f = " << f << endl;
         }
      }

// ==========================================================================
// XYZ point cloud properties
// ==========================================================================

// Method interpoint_RMS_distance forms a KDTree from all of the XYZ
// threevectors.  It then loops over each XYZ point and finds its
// nearest neighbor.  Neighbors which are located too close are
// ignored.  Otherwise, this method returns the RMS nearest neighbor
// distance.

   double interpoint_RMS_distance(
      double minimal_separation,vector<fourvector>* XYZP_ptr)
      {
         outputfunc::write_banner(
            "Computing approximate interpoint RMS distance:");

         KDTree::KDTree<3,fourvector>* xyz_kdtree_ptr=
            kdtreefunc::generate_3D_kdtree(*XYZP_ptr);

         cout << "Original KDtree size = " << xyz_kdtree_ptr->size() << endl;

         int n_inputs=0;
         double closest_node_sqr_dist=0;

         unsigned int n_closest_nodes=2;
         const double SQR_TINY=sqr(minimal_separation);
         double rho=basic_math::max(minimal_separation,0.005);	// meter
         vector<fourvector> closest_node;
         for (unsigned int i=0; i<xyz_kdtree_ptr->size(); i++)
         {
            if (i%1000==0) cout << i << " " << flush;

            fourvector curr_xyzp=(*XYZP_ptr)[i];

            closest_node.clear();
            kdtreefunc::find_closest_nodes(
               xyz_kdtree_ptr,curr_xyzp,rho,n_closest_nodes,closest_node);

            threevector delta(closest_node[1]-curr_xyzp);
            double sqr_dist=delta.sqrd_magnitude();
            if (sqr_dist > SQR_TINY)
            {
               closest_node_sqr_dist += sqr_dist;
               n_inputs++;
            }
         } // loop over index i labeling XYZ points

         delete xyz_kdtree_ptr;

         closest_node_sqr_dist /= n_inputs;
         return sqrt(closest_node_sqr_dist);
      }

// ---------------------------------------------------------------------
   threevector center_of_mass(vector<fourvector> const *xyzp_point_ptr)
      {
         threevector xyz_sum;
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            fourvector curr_pnt=(*xyzp_point_ptr)[i];
            xyz_sum += threevector(curr_pnt.get(0),curr_pnt.get(1),
                                curr_pnt.get(2));
         }
         return xyz_sum/double(xyzp_point_ptr->size());
      }

   threevector center_of_mass(vector<genvector> const *xyzp_point_ptr)
      {
         threevector xyz_sum;
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            genvector curr_pnt=(*xyzp_point_ptr)[i];
            xyz_sum += threevector(curr_pnt.get(0),curr_pnt.get(1),
                                curr_pnt.get(2));
         }
         return xyz_sum/double(xyzp_point_ptr->size());
      }

// ---------------------------------------------------------------------
   double zdist_percentile_height(
      vector<genvector> const *xyzp_point_ptr,double cum_prob)
      {
         vector<double> z_values;
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            genvector curr_pnt=(*xyzp_point_ptr)[i];
            z_values.push_back(curr_pnt.get(2));
         }
         prob_distribution prob(z_values,100);
         return prob.find_x_corresponding_to_pcum(cum_prob);
      }

// ---------------------------------------------------------------------
   void plot_p_distribution(vector<genvector> const *xyzp_point_ptr)
      {
         vector<double> p_values;
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            genvector curr_pnt=(*xyzp_point_ptr)[i];
            p_values.push_back(curr_pnt.get(3));
         }
         prob_distribution prob(p_values,100);
         prob.set_densityfilenamestr("./raw_p_dist.meta");
         prob.set_xlabel("P value");
         prob.write_density_dist();
      }

// ---------------------------------------------------------------------
   void compute_extremal_XYZ_points_in_xyzpfile(
      string xyzp_filename,threevector& XYZ_min,threevector& XYZ_max)
      {
         vector<double>* X_ptr=new vector<double>;
         vector<double>* Y_ptr=new vector<double>;
         vector<double>* Z_ptr=new vector<double>;
         vector<double>* P_ptr=new vector<double>;
   
         unsigned int n_points=
            read_xyzp_float_data(xyzp_filename,X_ptr,Y_ptr,Z_ptr,P_ptr);

         double xmin=XYZ_min.get(0);
         double ymin=XYZ_min.get(1);
         double zmin=XYZ_min.get(2);

         double xmax=XYZ_max.get(0);
         double ymax=XYZ_max.get(1);
         double zmax=XYZ_max.get(2);

         for (unsigned int i=0; i<n_points; i++)
         {
            xmin=basic_math::min(xmin,X_ptr->at(i));
            xmax=basic_math::max(xmax,X_ptr->at(i));

            ymin=basic_math::min(ymin,Y_ptr->at(i));
            ymax=basic_math::max(ymax,Y_ptr->at(i));

            zmin=basic_math::min(zmin,Z_ptr->at(i));
            zmax=basic_math::max(zmax,Z_ptr->at(i));
         }

         delete X_ptr;
         delete Y_ptr;
         delete Z_ptr;
         delete P_ptr;

         XYZ_min=threevector(xmin,ymin,zmin);
         XYZ_max=threevector(xmax,ymax,zmax);
      }

// ==========================================================================
// XYZ point manipulation methods
// ==========================================================================

   void translate(const threevector& trans,vector<fourvector>* xyzp_point_ptr)
      {
         fourvector trans_vector;
         trans_vector.put(0,trans.get(0));
         trans_vector.put(1,trans.get(1));
         trans_vector.put(2,trans.get(2));
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            (*xyzp_point_ptr)[i] += trans_vector;
         }
      }

   void translate(const threevector& trans,
                  vector<genvector>* xyzp_point_ptr)
      {
         genvector trans_vector(4);
         trans_vector.put(0,trans.get(0));
         trans_vector.put(1,trans.get(1));
         trans_vector.put(2,trans.get(2));
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            (*xyzp_point_ptr)[i] += trans_vector;
         }
      }

// ---------------------------------------------------------------------
// Method scale multiplies each X, Y and Z value within
// *xyzp_point_ptr by input scalar sfactor.

   void scale(const double sfactor,vector<genvector>* xyzp_point_ptr)
      {
         genvector curr_xyzp_point(4);
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            curr_xyzp_point=(*xyzp_point_ptr)[i];
            curr_xyzp_point.put(0,sfactor*curr_xyzp_point.get(0));
            curr_xyzp_point.put(1,sfactor*curr_xyzp_point.get(1));
            curr_xyzp_point.put(2,sfactor*curr_xyzp_point.get(2));
            (*xyzp_point_ptr)[i]=curr_xyzp_point;
         }
      }

// ---------------------------------------------------------------------
   void rotate(const threevector& rotation_origin,
               vector<fourvector>* xyzp_point_ptr,
               double thetax,double thetay,double thetaz)
      {
         rotation R(thetax,thetay,thetaz);
         rotate(rotation_origin,R,xyzp_point_ptr);
      }

   void rotate(const threevector& rotation_origin,
               vector<genvector>* xyzp_point_ptr,
               double thetax,double thetay,double thetaz)
      {
         rotation R(thetax,thetay,thetaz);
         rotate(rotation_origin,R,xyzp_point_ptr);
      }

   void rotate(const rotation& R,vector<fourvector>* xyzp_point_ptr)
      {
         rotate(Zero_vector,R,xyzp_point_ptr);
      }

   void rotate(const rotation& R,vector<genvector>* xyzp_point_ptr)
      {
         rotate(Zero_vector,R,xyzp_point_ptr);
      }

   void rotate(const threevector& rotation_origin,const rotation& R,
               vector<fourvector>* xyzp_point_ptr)
      {
         cout << "inside rotate, rotation_origin = " << rotation_origin
              << endl;
         fourvector curr_xyzp_point,rotated_xyzp_point;
         threevector xyz_point;
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            curr_xyzp_point=(*xyzp_point_ptr)[i];
            xyz_point.put(0,curr_xyzp_point.get(0));
            xyz_point.put(1,curr_xyzp_point.get(1));
            xyz_point.put(2,curr_xyzp_point.get(2));
            threevector dv(xyz_point-rotation_origin);
            xyz_point=rotation_origin+R*dv;
            rotated_xyzp_point.put(0,xyz_point.get(0));
            rotated_xyzp_point.put(1,xyz_point.get(1));
            rotated_xyzp_point.put(2,xyz_point.get(2));
            rotated_xyzp_point.put(3,curr_xyzp_point.get(3));
            (*xyzp_point_ptr)[i]=rotated_xyzp_point;
         }
      }

   void rotate(const threevector& rotation_origin,const rotation& R,
               vector<genvector>* xyzp_point_ptr)
      {
         genvector curr_xyzp_point(4),rotated_xyzp_point(4);
         threevector xyz_point;
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            curr_xyzp_point=(*xyzp_point_ptr)[i];
            xyz_point.put(0,curr_xyzp_point.get(0));
            xyz_point.put(1,curr_xyzp_point.get(1));
            xyz_point.put(2,curr_xyzp_point.get(2));
            threevector dv(xyz_point-rotation_origin);
            xyz_point=rotation_origin+R*dv;
            rotated_xyzp_point.put(0,xyz_point.get(0));
            rotated_xyzp_point.put(1,xyz_point.get(1));
            rotated_xyzp_point.put(2,xyz_point.get(2));
            rotated_xyzp_point.put(3,curr_xyzp_point.get(3));
            (*xyzp_point_ptr)[i]=rotated_xyzp_point;
         }
      }

// ---------------------------------------------------------------------
// Method XY0P_projection takes in a set of XYZP points within STL
// vector *xyzp_point_ptr.  It zeros out all Z values and returns the
// results within a new dynamically generated STL vector.

   vector<fourvector>* XY0P_projection(
      vector<fourvector> const *xyzp_point_ptr)
      {
         fourvector projected_xy0p_point;
         vector<fourvector>* xy0p_point_ptr=new vector<fourvector>;
         xy0p_point_ptr->reserve(xyzp_point_ptr->size());
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            projected_xy0p_point=(*xyzp_point_ptr)[i];
//            projected_xy0p_point.put(0,curr_xyzp_point.get(0));
//            projected_xy0p_point.put(1,curr_xyzp_point.get(1));
            projected_xy0p_point.put(2,0);
//            projected_xy0p_point.put(3,curr_xyzp_point.get(3));
            xy0p_point_ptr->push_back(projected_xy0p_point);
         }
         return xy0p_point_ptr;
      }

// ---------------------------------------------------------------------
// Method subdivide_xy_spacing takes in STL vector *xyzp_point_ptr
// which contains fourvector XYZP points whose xy spacing equals
// ds_old.  It also takes in a central position along with 2D radius
// rho.  For those XYZP points within *xyzp_point_ptr lying closer
// than distance rho to the central position, this method generates an
// r x r X-Y lattice of points with the same Z and P values as the
// original point.  Here r = nearest ODD integer >= the ratio
// ds_old/ds_new.  All such lattice points are written to the output
// XYZP file specified by input string subdivided_xyzp_filename.  We
// devised this method in Feb 05 in order to merge dense ground photos
// of our favorite Lowell apartment building which less dense ALIRT
// imagery of this same structure.

   void subdivide_xy_spacing(
      string subdivided_xyzp_filename,double ds_old,double ds_new,
      const threevector& center_posn,double rho,
      vector<fourvector>* xyzp_point_ptr)
      {
         ofstream binary_outstream;
         filefunc::open_binaryfile(subdivided_xyzp_filename,binary_outstream);

         unsigned int orig_size=xyzp_point_ptr->size();
         int r=basic_math::round(ds_old/ds_new);
         if (is_even(r)) r++;

         fourvector curr_point,new_point;         
         for (unsigned int i=0; i<orig_size; i++)
         {
            if (i%10000==0) cout << i << " " << flush;
            curr_point=new_point=(*xyzp_point_ptr)[i];

            double curr_rhosq=sqr(center_posn.get(0)-curr_point.get(0))
               +sqr(center_posn.get(1)-curr_point.get(1));
            if (curr_rhosq < sqr(rho))
            {
               for (int px=-r/2; px <= r/2; px++)
               {
                  new_point.put(0,curr_point.get(0)+px*ds_new);
                  for (int py=-r/2; py <= r/2; py++)
                  {
                     new_point.put(1,curr_point.get(1)+py*ds_new);
                     write_single_xyzp_point(binary_outstream,new_point);
                  } // loop over py index
               } // loop over px index
            } // curr_rhosq < sqr(rho) conditional
         } // loop over index i labeling XYZP points
         binary_outstream.close();
      }

   void subdivide_xy_spacing(
      string subdivided_xyzp_filename,double ds_old,double ds_new,
      const threevector& center_posn,double rho,
      vector<genvector>* xyzp_point_ptr)
      {
         ofstream binary_outstream;
         filefunc::open_binaryfile(subdivided_xyzp_filename,binary_outstream);

         unsigned int orig_size=xyzp_point_ptr->size();
         int r=basic_math::round(ds_old/ds_new);
         if (is_even(r)) r++;

         genvector curr_point(4),new_point(4);         
         for (unsigned int i=0; i<orig_size; i++)
         {
            if (i%10000==0) cout << i << " " << flush;
            curr_point=new_point=(*xyzp_point_ptr)[i];

            double curr_rhosq=sqr(center_posn.get(0)-curr_point.get(0))
               +sqr(center_posn.get(1)-curr_point.get(1));
            if (curr_rhosq < sqr(rho))
            {
               for (int px=-r/2; px <= r/2; px++)
               {
                  new_point.put(0,curr_point.get(0)+px*ds_new);
                  for (int py=-r/2; py <= r/2; py++)
                  {
                     new_point.put(1,curr_point.get(1)+py*ds_new);
                     write_single_xyzp_point(binary_outstream,new_point);
                  } // loop over py index
               } // loop over px index
            } // curr_rhosq < sqr(rho) conditional
         } // loop over index i labeling XYZP points
         binary_outstream.close();
      }

// ==========================================================================
// P value manipulation methods
// ==========================================================================

// Method renormalize_p_values takes in an STL vector of XYZP points.
// It rescales all of the p-values such that their minimum and maximum
// values equal those passed as input parameters to this method.

   void renormalize_p_values(
      double min_p,double max_p,vector<fourvector>* xyzp_point_ptr)
      {
         renormalize_p_values(min_p,max_p,NEGATIVEINFINITY,xyzp_point_ptr);
      }

// This overloaded version takes in a threshold p value.  Any
// probabilities which are less than this threshold are ignored by the
// renormalization procedure.  

   void renormalize_p_values(
      double min_p,double max_p,double p_threshold,
      vector<fourvector>* xyzp_point_ptr)
      {

// Compute minimum and maximum p values within incoming XYZP STL vector:

         double minimum_p=POSITIVEINFINITY;
         double maximum_p=NEGATIVEINFINITY;
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            fourvector curr_pnt=(*xyzp_point_ptr)[i];
            minimum_p=basic_math::min(minimum_p,curr_pnt.get(3));
            maximum_p=basic_math::max(maximum_p,curr_pnt.get(3));
         }
         minimum_p=basic_math::max(minimum_p,p_threshold);

         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            fourvector curr_pnt=(*xyzp_point_ptr)[i];
            double p_orig=curr_pnt.get(3);
            double p_new=p_orig;
            if (p_orig > p_threshold)
            {
               p_new=min_p+(max_p-min_p)/(maximum_p-minimum_p)*
                  (p_orig-minimum_p);
            }
            (*xyzp_point_ptr)[i].put(3,p_new);
         }
      }

// ---------------------------------------------------------------------
// Method reset_null_p_values takes in an STL vector of XYZP points.
// It resets any p which is less than null_value to new_null_value.

   void reset_null_p_values(double null_value,double new_null_value,
                            vector<fourvector>* xyzp_point_ptr)
      {
         int n_null_values=0;
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            if (  (*xyzp_point_ptr)[i].get(3) < null_value )
            {
               (*xyzp_point_ptr)[i].put(3,new_null_value);
               n_null_values++;
            }
         }
         cout << "Number of reset null values = " << n_null_values
              << endl;
      }

   void reset_null_p_values(
      double min_value,double max_value,double new_null_value,
      vector<fourvector>* xyzp_point_ptr)
      {
         int n_null_values=0;
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            if (  (*xyzp_point_ptr)[i].get(3) > min_value &&
                  (*xyzp_point_ptr)[i].get(3) < max_value )
            {
               (*xyzp_point_ptr)[i].put(3,new_null_value);
               n_null_values++;
            }
         }
         cout << "Number of reset null values = " << n_null_values
              << endl;
      }



// ---------------------------------------------------------------------
// Method threshold_p_values takes in an STL vector of XYZP points.
// It discards all points whose p-values fall below input parameter
// min_p.

   void threshold_p_values(double min_p,vector<genvector>*& xyzp_point_ptr)
      {
         vector<genvector>* new_xyzp_pnt_ptr=new vector<genvector>;
         new_xyzp_pnt_ptr->reserve(xyzp_point_ptr->size());
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            genvector curr_pnt=(*xyzp_point_ptr)[i];
            if (curr_pnt.get(3) > min_p)
            {
               new_xyzp_pnt_ptr->push_back(curr_pnt);
            }
         }
         delete xyzp_point_ptr;
         xyzp_point_ptr=new_xyzp_pnt_ptr;
      }

// ---------------------------------------------------------------------
// Method recolor_p_values_for_RGB_colormap takes in an STL vector
// which is assumed to be filled with XYZP values where the
// probabilities are intended to be displayed with the group 94/106
// dataviewer using our "Hue+value" colormap.  It converts the
// p-values so that they can be viewed using our "RGB" colormap.  This
// method was created in Feb 2005 for ALIRT ladar imagery/color photo
// fusion purposes.

   void recolor_p_values_for_RGB_colormap(
      string xyzp_filename,string RGB_xyzp_filename)
      {
         outputfunc::write_banner("Recoloring p-values for RGB colormap:");

         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         vector<fourvector>* xyzp_point_ptr=
            read_xyzp_float_data(xyzp_filename);
         filefunc::gzip_file_if_gunzipped(xyzp_filename);

         recolor_p_values_for_RGB_colormap(xyzp_point_ptr);
         write_xyzp_data(RGB_xyzp_filename,xyzp_point_ptr);
         delete xyzp_point_ptr;
      }

   void recolor_p_values_for_RGB_colormap(vector<fourvector>* xyzp_point_ptr)
      {
         colorfunc::initialize_hue_value_colormap();	
					// ALIRT Lowell images
//         colorfunc::initialize_big_hue_value_colormap(); 
					// JIGSAW static images
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            genvector curr_pnt=(*xyzp_point_ptr)[i];
            double p_hue_value=curr_pnt.get(3);
            colorfunc::RGB curr_RGB=colorfunc::dataviewer_colormap_to_RGB(
               p_hue_value);
            double p_RGB=colorfunc::rgb_colormap_value(curr_RGB);
            (*xyzp_point_ptr)[i].put(3,p_RGB);
         }
         colorfunc::delete_dataviewer_colormap();
      }

// ---------------------------------------------------------------------
// Method scale_p_values_to_heights linearly interpolates all p values
// within input STL vector *xyzp_point_ptr so that they are
// proportional to their z values and range from p_min <= p <= p_max.

   void scale_p_values_to_heights(
      double z_min,double z_max,double p_min,double p_max,
      vector<genvector>* xyzp_point_ptr)
      {
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            genvector curr_pnt=(*xyzp_point_ptr)[i];
            double z=curr_pnt.get(2);
            double p_new=p_min+(z-z_min)*(p_max-p_min)/(z_max-z_min);
            (*xyzp_point_ptr)[i].put(3,p_new);
         }
      }

// ---------------------------------------------------------------------
// Method write_p_values_as_RGBA_bytes takes in the name of some XYZP
// file which we assume should be viewed using the Group 94/106
// dataviewer with our "Hue+value" colormap.  This method first
// converts each p value into the corresponding RGB values.  It next
// converts the RGB values into their byte forms.  It writes float x,
// float y, float z, char R, char G, char B and char alpha to the new
// output file specified by RGBA_xyzp_filename.  This output file
// should be readable by an OpenSceneGraph viewer.

   void write_p_values_as_RGBA_bytes(
      string xyzp_filename,string RGBA_xyzp_filename)
      {
         outputfunc::write_banner("Writing out p-values as RGBA bytes:");

         filefunc::gunzip_file_if_gzipped(xyzp_filename);
         vector<fourvector>* xyzp_point_ptr=
            read_xyzp_float_data(xyzp_filename);
         filefunc::gzip_file_if_gunzipped(xyzp_filename);

         ofstream binary_outstream;
         filefunc::open_binaryfile(RGBA_xyzp_filename,binary_outstream);
         write_p_values_as_RGBA_bytes(xyzp_point_ptr,binary_outstream);
         binary_outstream.close();

         delete xyzp_point_ptr;
      }

   void write_p_values_as_RGBA_bytes(
      vector<fourvector>* xyzp_point_ptr,ofstream& binary_outstream)
      {
         const unsigned char alpha_byte=
            static_cast<unsigned char>(stringfunc::ascii_integer_to_char(
               255));

         colorfunc::initialize_hue_value_colormap();	
					// ALIRT Lowell images
//         colorfunc::initialize_big_hue_value_colormap(); 
					// JIGSAW static images
         for (unsigned int i=0; i<xyzp_point_ptr->size(); i++)
         {
            fourvector curr_point=(*xyzp_point_ptr)[i];
            double p_hue_value=curr_point.get(3);
            colorfunc::RGB curr_RGB=colorfunc::dataviewer_colormap_to_RGB(
               p_hue_value);
            colorfunc::RGB_bytes curr_RGB_bytes=
               colorfunc::RGB_to_bytes(curr_RGB);

            float x=static_cast<float>(curr_point.get(0));
            float y=static_cast<float>(curr_point.get(1));
            float z=static_cast<float>(curr_point.get(2));
            filefunc::writeobject(binary_outstream,x);
            filefunc::writeobject(binary_outstream,y);
            filefunc::writeobject(binary_outstream,z);

            filefunc::writeobject(binary_outstream,curr_RGB_bytes.first);
            filefunc::writeobject(binary_outstream,curr_RGB_bytes.second);
            filefunc::writeobject(binary_outstream,curr_RGB_bytes.third);
            filefunc::writeobject(binary_outstream,alpha_byte);
         }
         colorfunc::delete_dataviewer_colormap();
      }

// ---------------------------------------------------------------------
// Method read_xyzrgba_data reads in an XYZRGBA file.

   void read_xyzrgba_data(
      string xyzrgba_filename,vector<threevector>* xyz_pnt_ptr,
      vector<colorfunc::RGB>* rgb_pnt_ptr)
      {
         ifstream binary_instream;

         bool file_opened=false;
         do
         {
            filefunc::gunzip_file_if_gzipped(xyzrgba_filename);
            file_opened=filefunc::open_binaryfile(
               xyzrgba_filename,binary_instream);
         }
         while (!file_opened);
         unsigned int npoints=npoints_inside_xyzp_file(xyzrgba_filename);

         typedef union RGBAStruct
         {
               float p;
               unsigned char rgba[4];
         };
         union RGBAStruct RGBA;

         float curr_x,curr_y,curr_z;
         colorfunc::RGB curr_rgb;
         for (unsigned int n=0; n<npoints; n++)
         {
            filefunc::readobject(binary_instream,curr_x);
            filefunc::readobject(binary_instream,curr_y);
            filefunc::readobject(binary_instream,curr_z);
            xyz_pnt_ptr->push_back(threevector(curr_x,curr_y,curr_z));
            
            filefunc::readobject(binary_instream,RGBA.p);
            curr_rgb.first=static_cast<double>(RGBA.rgba[0]);
            curr_rgb.second=static_cast<double>(RGBA.rgba[1]);
            curr_rgb.third=static_cast<double>(RGBA.rgba[2]);
            rgb_pnt_ptr->push_back(curr_rgb);

         } // loop over index n labeling input points

         binary_instream.close();  
      }

// ---------------------------------------------------------------------
// Method density_filter()

   void density_filter(
      double maximum_separation,vector<fourvector>* XYZP_ptr)
      {
         cout << "inside xyzpfunc::density_filter()" << endl;

         KDTree::KDTree<3,fourvector>* xyz_kdtree_ptr=
            kdtreefunc::generate_3D_kdtree(*XYZP_ptr);

         cout << "Original KDtree size = " << xyz_kdtree_ptr->size() << endl;

/*
         int n_inputs=0;
         double closest_node_sqr_dist=0;

         unsigned int n_closest_nodes=2;
         const double SQR_TINY=sqr(minimal_separation);
         double rho=basic_math::max(minimal_separation,0.005);	// meter
         vector<fourvector> closest_node;
         for (unsigned int i=0; i<xyz_kdtree_ptr->size(); i++)
         {
            if (i%1000==0) cout << i << " " << flush;

            fourvector curr_xyzp=(*XYZP_ptr)[i];

            closest_node.clear();
            kdtreefunc::find_closest_nodes(
               xyz_kdtree_ptr,curr_xyzp,rho,n_closest_nodes,closest_node);

            threevector delta(closest_node[1]-curr_xyzp);
            double sqr_dist=delta.sqrd_magnitude();
            if (sqr_dist > SQR_TINY)
            {
               closest_node_sqr_dist += sqr_dist;
               n_inputs++;
            }
         } // loop over index i labeling XYZ points
*/

         delete xyz_kdtree_ptr;

//         closest_node_sqr_dist /= n_inputs;
//         return sqrt(closest_node_sqr_dist);
      }


// ---------------------------------------------------------------------
// Method npoints_inside_Hokoyu_file()

   vector<threevector>* parse_Hokoyu_binary_datafile(
      string hokoyu_filename,threevector& robo_coords)
      {
         long long nbytes=0;
         ifstream binary_instream;
         filefunc::open_binaryfile(hokoyu_filename,binary_instream,nbytes);
         binary_instream.close();

         int nfloats=nbytes/sizeof(float);
         unsigned int npoints=(nfloats-4)/3;
         cout << "Number of XYZ points within input binary hokoyu file = " 
              << npoints << endl;

         filefunc::open_binaryfile(hokoyu_filename,binary_instream,nbytes);

         float x,y,z,theta;
         filefunc::readobject(binary_instream,x);
         filefunc::readobject(binary_instream,y);
         filefunc::readobject(binary_instream,theta);
         robo_coords=threevector(x,y,theta);
         cout << "robo_coords = " << robo_coords << endl;

         unsigned int number_pnts;
         filefunc::readobject(binary_instream,number_pnts);

         vector<threevector>* xyz_pnt_ptr=new vector<threevector>;
         xyz_pnt_ptr->reserve(number_pnts);
         for (unsigned int n=0; n<number_pnts; n++)
         {
            filefunc::readobject(binary_instream,x);
            filefunc::readobject(binary_instream,y);
            filefunc::readobject(binary_instream,z);
            xyz_pnt_ptr->push_back(threevector(x,y,z));
         } // loop over index n labeling input points

         binary_instream.close();
         return xyz_pnt_ptr;
      }

   
} // xyzpfunc namespace






