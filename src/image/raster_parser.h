// ==========================================================================
// Header file for raster_parser class 
// ==========================================================================
// Last modified on 12/12/09; 10/20/10; 2/3/12; 4/5/14
// ==========================================================================

#ifndef RASTER_PARSER_H
#define RASTER_PARSER_H

#include <string>
#include "cpl_string.h"
#include "image/myimage.h"
#include "image/TwoDarray.h"
#include "math/twovector.h"

class GDALDataset;
class GDALRasterBand;
class genmatrix;

class raster_parser
{

  public:

   raster_parser();
   raster_parser(const raster_parser& d);
   void null_ztwoDarray_ptr();
   ~raster_parser();
   raster_parser& operator= (const raster_parser& d);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const raster_parser& d);

// Set and get member functions:

   void set_n_channels(unsigned int n);
   unsigned int get_n_channels() const;
   int get_raster_Xsize() const;
   int get_raster_Ysize() const;

   bool get_northern_hemisphere_flag() const;
   int get_specified_UTM_zonenumber() const;

   double get_min_z() const;
   double get_max_z() const;
   twoDarray* get_ztwoDarray_ptr();
   const twoDarray* get_ztwoDarray_ptr() const;

   twoDarray* get_RtwoDarray_ptr();
   const twoDarray* get_RtwoDarray_ptr() const;
   twoDarray* get_GtwoDarray_ptr();
   const twoDarray* get_GtwoDarray_ptr() const;
   twoDarray* get_BtwoDarray_ptr();
   const twoDarray* get_BtwoDarray_ptr() const;
   twoDarray* get_AtwoDarray_ptr();
   const twoDarray* get_AtwoDarray_ptr() const;

// GDAL image import member functions:

   bool open_image_file(std::string image_filename,
                        bool predelete_ztwoDarray_ptr_flag=true);
   void close_image_file();
   void fetch_raster_band(int channel_ID);
   void read_raster_data(twoDarray* ztwoDarray_ptr);
   twoDarray* read_single_channel_image(std::string image_filename);

// GDAL image export member functions:

   void write_raster_data(
      bool output_floats_flag,std::string geotiff_filename,
      int output_UTM_zonenumber,bool output_northern_hemisphere_flag,
      twoDarray* ztwoDarray_ptr);
   void write_raster_data(
      bool output_floats_flag,std::string geotiff_filename,
      int output_UTM_zonenumber,bool output_northern_hemisphere_flag,
      twoDarray* ztwoDarray_ptr,double z_min,double z_max);
   void write_colored_raster_data(
      std::string geotiff_filename,
      int output_UTM_zonenumber,bool output_northern_hemisphere_flag,
      twoDarray* RtwoDarray_ptr,twoDarray* GtwoDarray_ptr,
      twoDarray* BtwoDarray_ptr,twoDarray* AtwoDarray_ptr);
   void write_colored_raster_data(
      std::string geotiff_filename,
      int output_UTM_zonenumber,bool output_northern_hemisphere_flag,
      twoDarray* RtwoDarray_ptr,twoDarray* GtwoDarray_ptr,
      twoDarray* BtwoDarray_ptr,twoDarray* AtwoDarray_ptr,
      double z_min,double z_max);

// Linear array value filling member functions:

   void fill_Z_unsigned_char_array(
      double z_min,double z_max,const twoDarray* ztwoDarray_ptr,
      unsigned char* Z,bool rescale_zvalues_flag);
   void fill_Z_GUInt_array(
      double z_min,double z_max,const twoDarray* ztwoDarray_ptr,GUInt16* Z);
   void fill_Z_float_array(const twoDarray* ztwoDarray_ptr,float* Z);
   void fill_Z_double_array(const twoDarray* ztwoDarray_ptr,double* Z);
   void convert_GUInts_to_doubles(
      double z_min,double z_max,twoDarray* ztwoDarray_ptr);

// Geo value extraction member functions:

   bool get_Zvalue(double longitude,double latitude,double& Zvalue);

  private: 

   unsigned int n_channels;
   double min_z,max_z,min_r,max_r,min_g,max_g,min_b,max_b;
   bool northern_hemisphere_flag;
   int specified_UTM_zonenumber;
   std::string projection_WKT;
   GDALDataset* poDataset_ptr;
   GDALRasterBand* poBand_ptr;
   genmatrix* M_ptr;
   twovector trans;
   twoDarray *ztwoDarray_ptr,*RtwoDarray_ptr,*GtwoDarray_ptr,*BtwoDarray_ptr,
      *AtwoDarray_ptr;

   double* scanline_buffer;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const raster_parser& d);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void raster_parser::set_n_channels(unsigned int n)
{
   n_channels=n;
}

inline unsigned int raster_parser::get_n_channels() const
{
   return n_channels;
}

inline bool raster_parser::get_northern_hemisphere_flag() const
{
   return northern_hemisphere_flag;
}

inline int raster_parser::get_specified_UTM_zonenumber() const
{
   return specified_UTM_zonenumber;
}

inline double raster_parser::get_min_z() const
{
   return min_z;
}

inline double raster_parser::get_max_z() const
{
   return max_z;
}

inline twoDarray* raster_parser::get_ztwoDarray_ptr()
{
   return ztwoDarray_ptr;
}

inline const twoDarray* raster_parser::get_ztwoDarray_ptr() const
{
   return ztwoDarray_ptr;
}

inline twoDarray* raster_parser::get_RtwoDarray_ptr()
{
   return RtwoDarray_ptr;
}

inline const twoDarray* raster_parser::get_RtwoDarray_ptr() const
{
   return RtwoDarray_ptr;
}

inline twoDarray* raster_parser::get_GtwoDarray_ptr()
{
   return GtwoDarray_ptr;
}

inline const twoDarray* raster_parser::get_GtwoDarray_ptr() const
{
   return GtwoDarray_ptr;
}

inline twoDarray* raster_parser::get_BtwoDarray_ptr()
{
   return BtwoDarray_ptr;
}

inline const twoDarray* raster_parser::get_BtwoDarray_ptr() const
{
   return BtwoDarray_ptr;
}

inline twoDarray* raster_parser::get_AtwoDarray_ptr()
{
   return AtwoDarray_ptr;
}

inline const twoDarray* raster_parser::get_AtwoDarray_ptr() const
{
   return AtwoDarray_ptr;
}

#endif  // raster_parser.h
