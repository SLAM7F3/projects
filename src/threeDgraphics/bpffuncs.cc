// ==========================================================================
// BPFFUNCS stand-alone methods
// ==========================================================================
// Last modified on 11/18/11; 11/28/11; 12/9/11; 4/5/14
// ==========================================================================

#include <iostream>
#include "threeDgraphics/bpffuncs.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;

namespace bpffunc
{

// Boolean method is_bpf3_file() takes in an ifstream which is assumed
// to have already been opened to some input Binary Point Format (BPF)
// file.  It returns true if the file's format is BPF3, and false if
// the format is BPF1 or BPF2.

   bool is_bpf3_file(ifstream& binary_instream)
   {
//      cout << "inside is_bpf3_file()" << endl;
      
      bool bpf3_flag=false;
      binary_instream.seekg(0,ios::beg);

// Parse base header for BPF3 file:

      char* char_buffer=new char[4];
      binary_instream.read(char_buffer,4);
      string magic_number(char_buffer);
//      cout << "magic_number = " << magic_number << endl;

      binary_instream.seekg(0,ios::beg);

// On 11/16/11, we encountered trouble with the magic number string
// being read as having extra characters beyond BPF!.  So we will only
// focus upon the first 4 chars in magic_number to determine if the
// input file is BPF3 format or not:

      string substr=magic_number.substr(0,4);
      if (magic_number.substr(0,4)=="BPF!")
      {
         bpf3_flag=true;
      }

      return bpf3_flag;
   }

// --------------------------------------------------------------------------
// Method parse_bpf12_points() reads in X,Y,Z and P values from an
// binary stream which is assumed to correspond to an input BPF1
// or BPF2 file.  This method returns those values within output STL
// vectors along with the number of the UTM zone in which they were
// collected.

   void parse_bpf12_points(
      ifstream& binary_instream,vector<double>* X_ptr,vector<double>* Y_ptr,
      vector<double>* Z_ptr,vector<double>* NWC_ptr,vector<double>* P_ptr,
      int& UTM_zonenumber)
   {
      cout << "inside bpffunc::parse_bpf12_points()" << endl;
      
      int32_t header_length;
      binary_instream.read((char *) &header_length,sizeof(int32_t)); 
      cout << "header_length = " << header_length << endl;

      int32_t format_version;
      binary_instream.read((char *) &format_version,sizeof(int32_t));
      cout << "format_version = " << format_version << endl;

      bool interleaved_flag=false;
      if (format_version==2) interleaved_flag=true;

      int32_t point_count;
      binary_instream.read((char *) &point_count,sizeof(int32_t)); 
      cout << "point_count = " << point_count << endl;

      int32_t n_metadims;
      binary_instream.read((char *) &n_metadims,sizeof(int32_t)); 
      cout << "n_metadims = " << n_metadims << endl;

      int32_t coordspace_type;
      binary_instream.read((char *) &coordspace_type,sizeof(int32_t)); 
//      cout << "coordspace_type = " << coordspace_type << endl;

      int32_t utm_zonenumber;
      binary_instream.read((char *) &utm_zonenumber,sizeof(int32_t)); 
      UTM_zonenumber=utm_zonenumber;
      cout << "UTM_zonenumber = " << UTM_zonenumber << endl;

      float point_spacing;
      binary_instream.read((char *) &point_spacing,sizeof(float)); 
      cout << "point_spacing = " << point_spacing << endl;

      double Xoffset;
      binary_instream.read((char *) &Xoffset,sizeof(double)); 
      cout << "Xoffset = " << Xoffset << endl;

      double Yoffset;
      binary_instream.read((char *) &Yoffset,sizeof(double)); 
      cout << "Yoffset = " << Yoffset << endl;

      double Zoffset;
      binary_instream.read((char *) &Zoffset,sizeof(double)); 
     cout << "Zoffset = " << Zoffset << endl;

      double Xmin;
      binary_instream.read((char *) &Xmin,sizeof(double)); 
      cout << "Xmin = " << Xmin << endl;

      double Xmax;
      binary_instream.read((char *) &Xmax,sizeof(double)); 
      cout << "Xmax = " << Xmax << endl;

      double Ymin;
      binary_instream.read((char *) &Ymin,sizeof(double)); 
      cout << "Ymin = " << Ymin << endl;

      double Ymax;
      binary_instream.read((char *) &Ymax,sizeof(double)); 
      cout << "Ymax = " << Ymax << endl;

      double Zmin;
      binary_instream.read((char *) &Zmin,sizeof(double)); 
      cout << "Zmin = " << Zmin << endl;

      double Zmax;
      binary_instream.read((char *) &Zmax,sizeof(double)); 
      cout << "Zmax = " << Zmax << endl;

// Parse metadata subheader for BPF1 or BPF2 files:

      vector<double> value_offset,min_value,max_value;
      vector<string> value_label;

      value_offset.push_back(Xoffset);
      value_offset.push_back(Yoffset);
      value_offset.push_back(Zoffset);
   
      min_value.push_back(Xmin);
      min_value.push_back(Ymin);
      min_value.push_back(Zmin);
   
      max_value.push_back(Xmax);
      max_value.push_back(Ymax);
      max_value.push_back(Zmax);
   
      value_label.push_back("X");
      value_label.push_back("Y");
      value_label.push_back("Z");

      cout.precision(10);   
      for (int d=0; d<n_metadims; d++)
      {
         double offset;
         binary_instream.read((char *) &offset,sizeof(double)); 
         cout << "d = " << d << " offset = " << offset << endl;
         value_offset.push_back(offset);
      }
      for (int d=0; d<n_metadims; d++)
      {
         double minimum_value;
         binary_instream.read((char *) &minimum_value,sizeof(double)); 
         cout << "d = " << d << " minimum_value = " << minimum_value << endl;
         min_value.push_back(minimum_value);
      }

      for (int d=0; d<n_metadims; d++)
      {
         double maximum_value;
         binary_instream.read((char *) &maximum_value,sizeof(double)); 
         cout << "d = " << d << " maximum_value = " << maximum_value << endl;
         max_value.push_back(maximum_value);
      }

      char* big_char_buffer=new char[32];
      for (int d=0; d<n_metadims; d++)
      {
         binary_instream.read(big_char_buffer,32);
         string label(big_char_buffer);
         cout << "d = " << d << " label = " << label << endl;
         value_label.push_back(label);
      }
      delete [] big_char_buffer;

      cout << endl;
      X_ptr->reserve(point_count);
      Y_ptr->reserve(point_count);
      Z_ptr->reserve(point_count);

      if (P_ptr != NULL) P_ptr->reserve(point_count);
      if (NWC_ptr != NULL) NWC_ptr->reserve(point_count);

// Seek to starting location of actual data points:

      binary_instream.seekg(0,ios::beg);

      char dummy_char;
      for (int i=0; i<header_length; i++)
      {
         binary_instream.read((char *) &dummy_char,sizeof(char)); 
      }


      if (interleaved_flag)
      {
         for (int npnts=0; npnts<point_count; npnts++)
         {
            for (int d=0; d<3+n_metadims; d++)
            {
               float curr_value;
               binary_instream.read((char *) &curr_value,sizeof(float)); 
               curr_value += value_offset[d];

               if (d==0)
               {
                  X_ptr->push_back(curr_value);
               }
               else if (d==1)
               {
                  Y_ptr->push_back(curr_value);
               }
               else if (d==2)
               {
                  Z_ptr->push_back(curr_value);
               }
               else if (d==5 && P_ptr != NULL)
               {
                  P_ptr->push_back(curr_value);
               }

               if (npnts%1000==0)
               {
                  cout << "npnts = " << npnts 
                       << " d = " << d 
                       << " label = " << value_label[d] 
                       << " curr_val = " << curr_value  << endl;
               }
            
               if (curr_value < min_value[d]) 
               {
                  cout << "ERROR: min_value = " << min_value[d] << endl;
//                  outputfunc::enter_continue_char();
               }
               if (curr_value > max_value[d]) 
               {
                  cout << "ERROR: max_value = " << max_value[d] << endl;
//                  outputfunc::enter_continue_char();
               }

            } // loop over d index

//         outputfunc::enter_continue_char();
         } // loop over npnts index

      }
      else if (!interleaved_flag)
      {
         for (int d=0; d<3+n_metadims; d++)
         {
            for (int npnts=0; npnts<point_count; npnts++)
            {
               float curr_value;
               binary_instream.read((char *) &curr_value,sizeof(float)); 
               curr_value += value_offset[d];

               if (d==0)
               {
                  X_ptr->push_back(curr_value);
               }
               else if (d==1)
               {
                  Y_ptr->push_back(curr_value);
               }
               else if (d==2)
               {
                  Z_ptr->push_back(curr_value);
               }
               else if (d==3 && NWC_ptr != NULL)
               {
                  NWC_ptr->push_back(curr_value);
               }
               else if (d==5 && P_ptr != NULL)
               {
                  P_ptr->push_back(curr_value);
               }

               if (npnts%1000==0)
               {
//               cout << "d = " << d 
//                    << " label = " << value_label[d] 
//                    << " curr_val = " << curr_value  << endl;

                  if (curr_value < min_value[d]) 
                  {
                     cout << "ERROR: min_value = " << min_value[d] << endl;
//                  outputfunc::enter_continue_char();
                  }
                  if (curr_value > max_value[d]) 
                  {
                     cout << "ERROR: max_value = " << max_value[d] << endl;
//                  outputfunc::enter_continue_char();
                  }
               } // npnts%1000==0 conditional

            } // loop over npnts index
         } // loop over d index
      }
   }

// --------------------------------------------------------------------------
// Method parse_bpf3_points() reads in X,Y,Z and P values from an
// binary stream which is assumed to correspond to an input BPF3 file.
// This method returns those values within output STL
// vectors along with the number of the UTM zone in which they were
// collected.

   int parse_L1_bpf3_points(
      ifstream& binary_instream,vector<double>* X_ptr,vector<double>* Y_ptr,
      vector<double>* Z_ptr,vector<int>* pixel_number_ptr,int& UTM_zonenumber)
   {
      cout << "bpffunc::inside parse_bpf3_points()" << endl;
      int n_frames;
      parse_bpf3_points(
         binary_instream,X_ptr,Y_ptr,Z_ptr,NULL,NULL,pixel_number_ptr,
         UTM_zonenumber,n_frames);
      return n_frames;
   }
   
   void parse_bpf3_points(
      ifstream& binary_instream,vector<double>* X_ptr,vector<double>* Y_ptr,
      vector<double>* Z_ptr,vector<double>* NWC_ptr,vector<double>* P_ptr,
      vector<int>* pixel_number_ptr,int& UTM_zonenumber,int& n_frames)
   {
      cout << "bpffunc::inside parse_bpf3_points()" << endl;

      char* char_buffer=new char[4];
      binary_instream.read(char_buffer,4);
      string magic_number(char_buffer);
      cout << "magic_number = " << magic_number << endl;

      binary_instream.read(char_buffer,4);
      string format_version(char_buffer);
      cout << "format_version = " << format_version << endl;

      delete [] char_buffer;

      int32_t header_length;
      binary_instream.read((char *) &header_length,sizeof(int32_t)); 
      cout << "header_length = " << header_length << endl;

      unsigned char uchar;
      binary_instream.read((char *) &uchar,sizeof(unsigned char)); 
      int n_dims=stringfunc::unsigned_char_to_ascii_integer(uchar);
      cout << "ndims = " << n_dims << endl;

      binary_instream.read((char *) &uchar,sizeof(unsigned char)); 
      int interleaved=stringfunc::unsigned_char_to_ascii_integer(uchar);
      cout << "interleaved = " << interleaved << endl;

      binary_instream.read((char *) &uchar,sizeof(unsigned char)); 
      binary_instream.read((char *) &uchar,sizeof(unsigned char)); 

      int32_t point_count;
      binary_instream.read((char *) &point_count,sizeof(int32_t)); 
      cout << "point_count = " << point_count << endl;

      int32_t coordspace_type;
      binary_instream.read((char *) &coordspace_type,sizeof(int32_t)); 
      cout << "coordspace_type = " << coordspace_type << endl;

      int32_t utm_zonenumber;
      binary_instream.read((char *) &utm_zonenumber,sizeof(int32_t)); 
      UTM_zonenumber=utm_zonenumber;
      cout << "UTM_zonenumber = " << UTM_zonenumber << endl;

      float point_spacing;
      binary_instream.read((char *) &point_spacing,sizeof(float)); 
      cout << "point_spacing = " << point_spacing << endl;

      double M11,M12,M13,M14;
      double M21,M22,M23,M24;
      double M31,M32,M33,M34;
      double M41,M42,M43,M44;
   
      binary_instream.read((char *) &M11,sizeof(double)); 
      binary_instream.read((char *) &M12,sizeof(double)); 
      binary_instream.read((char *) &M13,sizeof(double)); 
      binary_instream.read((char *) &M14,sizeof(double)); 

      binary_instream.read((char *) &M21,sizeof(double)); 
      binary_instream.read((char *) &M22,sizeof(double)); 
      binary_instream.read((char *) &M23,sizeof(double)); 
      binary_instream.read((char *) &M24,sizeof(double)); 

      binary_instream.read((char *) &M31,sizeof(double)); 
      binary_instream.read((char *) &M32,sizeof(double)); 
      binary_instream.read((char *) &M33,sizeof(double)); 
      binary_instream.read((char *) &M34,sizeof(double)); 

      binary_instream.read((char *) &M41,sizeof(double)); 
      binary_instream.read((char *) &M42,sizeof(double)); 
      binary_instream.read((char *) &M43,sizeof(double)); 
      binary_instream.read((char *) &M44,sizeof(double)); 

      genmatrix M(4,4);
      M.put(0,0,M11);
      M.put(0,1,M12);
      M.put(0,2,M13);
      M.put(0,3,M14);

      M.put(1,0,M21);
      M.put(1,1,M22);
      M.put(1,2,M23);
      M.put(1,3,M24);

      M.put(2,0,M31);
      M.put(2,1,M32);
      M.put(2,2,M33);
      M.put(2,3,M34);

      M.put(3,0,M41);
      M.put(3,1,M42);
      M.put(3,2,M43);
      M.put(3,3,M44);
   
      cout << "M = " << M << endl;
      cout.precision(12);

      double start_time;
      binary_instream.read((char *) &start_time,sizeof(double)); 
      cout << "start_time = " << start_time << endl;

      double end_time;
      binary_instream.read((char *) &end_time,sizeof(double)); 
      cout << "end_time = " << end_time << endl;

// Parse metadata subheader for BPF3 file:

      vector<double> value_offset,min_value,max_value;
      vector<string> value_label;

      cout.precision(10);
      for (int d=0; d<n_dims; d++)
      {
         double offset;
         binary_instream.read((char *) &offset,sizeof(double)); 
         cout << "d = " << d << " offset = " << offset << endl;
         value_offset.push_back(offset);
      }
      cout << endl;
      
      for (int d=0; d<n_dims; d++)
      {
         double minimum_value;
         binary_instream.read((char *) &minimum_value,sizeof(double)); 
         cout << "d = " << d << " minimum_value = " << minimum_value << endl;
         min_value.push_back(minimum_value);
      }
      cout << endl;

      for (int d=0; d<n_dims; d++)
      {
         double maximum_value;
         binary_instream.read((char *) &maximum_value,sizeof(double)); 
         cout << "d = " << d << " maximum_value = " << maximum_value << endl;
         max_value.push_back(maximum_value);
      }
      cout << endl;

      char* big_char_buffer=new char[32];
      for (int d=0; d<n_dims; d++)
      {
         binary_instream.read(big_char_buffer,32);
         string label(big_char_buffer);
         cout << "d = " << d << " label = " << label << endl;
         value_label.push_back(label);
      }
      delete [] big_char_buffer;

      cout << endl;
//      outputfunc::enter_continue_char();

      X_ptr->reserve(point_count);
      Y_ptr->reserve(point_count);
      Z_ptr->reserve(point_count);
      if (NWC_ptr != NULL) NWC_ptr->reserve(point_count);
      if (P_ptr != NULL) P_ptr->reserve(point_count);
      if (pixel_number_ptr != NULL) pixel_number_ptr->reserve(point_count);

// For level-1 raw data, max[min] frame numbers are stored in
// max[min]_value[3]:

      n_frames=0;
      if (P_ptr==NULL)
      {
         n_frames=max_value[3]-min_value[3];
      }

// Seek to starting location of actual data points:

      binary_instream.seekg(0,ios::beg);

      char dummy_char;
      for (int i=0; i<header_length; i++)
      {
         binary_instream.read((char *) &dummy_char,sizeof(char)); 
      }

      if (interleaved==1)
      {
         const unsigned int n_iters=64;
         float curr_value[n_dims*n_iters];
//         float* curr_value=new float[n_dims*n_iters];
         
         for (int npnts=0; npnts<point_count; npnts += n_iters)
         {
            outputfunc::update_progress_fraction(
               npnts,2000000/n_iters,point_count);

            binary_instream.read(
               (char *) &curr_value,n_dims*n_iters*sizeof(float)); 
//            binary_instream.read(
//               (char *) curr_value,n_dims*n_iters*sizeof(float)); 

            for (unsigned int iter=0; iter<n_iters; iter++)
            {
               X_ptr->push_back(curr_value[iter*n_dims+0] + value_offset[0]);
               Y_ptr->push_back(curr_value[iter*n_dims+1] + value_offset[1]);
               Z_ptr->push_back(curr_value[iter*n_dims+2] + value_offset[2]);
               if (NWC_ptr != NULL)
               {
                  NWC_ptr->push_back(
                     (curr_value[iter*n_dims+3]+value_offset[3])/max_value[3]);
               }
               if (pixel_number_ptr != NULL)
               {
                  pixel_number_ptr->push_back(
                     static_cast<int>(curr_value[iter*n_dims+4]));
               }
               if (P_ptr != NULL)
               {
                  P_ptr->push_back(curr_value[iter*n_dims+5]);
               }

            } // loop over iter index
         } // loop over npnts index

//         delete curr_value;
      }
      else if (interleaved==0)
      {
         for (int d=0; d<n_dims; d++)
         {
            for (int npnts=0; npnts<point_count; npnts++)
            {
               float curr_value;
               binary_instream.read((char *) &curr_value,sizeof(float)); 
               curr_value += value_offset[d];

               if (d==0)
               {
                  X_ptr->push_back(curr_value);
               }
               else if (d==1)
               {
                  Y_ptr->push_back(curr_value);
               }
               else if (d==2)
               {
                  Z_ptr->push_back(curr_value);
               }
               else if (d==3 && NWC_ptr != NULL)
               {
                  NWC_ptr->push_back(curr_value/max_value[3]);
               }
               else if (d==5 && P_ptr != NULL)
               {
                  P_ptr->push_back(curr_value);
               }

               if (npnts%1000==0)
               {
//               cout << "d = " << d 
//                    << " label = " << value_label[d] 
//                    << " curr_val = " << curr_value  << endl;

                  if (curr_value < min_value[d]) 
                  {
                     cout << "ERROR: min_value = " << min_value[d] << endl;
//                  outputfunc::enter_continue_char();
                  }
                  if (curr_value > max_value[d]) 
                  {
                     cout << "ERROR: max_value = " << max_value[d] << endl;
//                  outputfunc::enter_continue_char();
                  }
               } // npnts%1000==0 conditional

            } // loop over npnts index
         } // loop over d index
      }

      cout << endl;
   }

// --------------------------------------------------------------------------
// Method read_bfp_L1_data() reads in X,Y,Z from a raw "level 1" BPF file.
// This method returns those values within output STL vectors along
// with the number of the UTM zone in which they were collected.

   int read_bpf_L1_data(
      string bpf_filename,vector<double>* X_ptr,vector<double>* Y_ptr,
      vector<double>* Z_ptr,vector<int>* pixel_number_ptr,
      int& UTM_zonenumber)
   {
//      cout << "inisde bpffunc::read_bpf_L1_data()" << endl;
      
      ifstream binary_instream;
      filefunc::open_binaryfile(bpf_filename,binary_instream);

      int n_frames=0;
      if (bpffunc::is_bpf3_file(binary_instream))
      {
         cout << "BPF3 format" << endl;
         n_frames=bpffunc::parse_L1_bpf3_points(
            binary_instream,X_ptr,Y_ptr,Z_ptr,pixel_number_ptr,
            UTM_zonenumber);
      }
      else
      {
         cout << "BPF1 or BPF2 format" << endl;
         bpffunc::parse_bpf12_points(
            binary_instream,X_ptr,Y_ptr,Z_ptr,NULL,NULL,UTM_zonenumber);
      }
      binary_instream.close();  

//      int n_points=X_ptr->size();
//      cout << "Finished reading in " << n_points << " BPF data points" 
//           << endl;
//      return n_points;

      return n_frames;
   }
   
} // bpffunc namespace






