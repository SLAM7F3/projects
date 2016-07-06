// As of 11/1/2012, we have to insert ugly C-preprocessor definitions
// and conditionals in order to compile our code tree on the TOC12
// laptop which does NOT have libgdal installed!

// #define TOC12_LAPTOP_FLAG

// =========================================================================
// Raster_Parser class member function definitions 
// =========================================================================
// Last modified on 12/6/10; 11/15/11; 2/3/12; 4/5/14
// =========================================================================

#include <iostream>
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "math/genmatrix.h"
#include "image/raster_parser.h"

using std::cout;
using std::endl;
using std::flush;
using std::ostream;
using std::ofstream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void raster_parser::allocate_member_objects()
{
   M_ptr=new genmatrix(2,2);

//   const int mdim_max=5000;
   const int mdim_max=10001;
   scanline_buffer=new double[mdim_max];
}

void raster_parser::initialize_member_objects()
{
   n_channels=0;
   ztwoDarray_ptr=NULL;
   RtwoDarray_ptr=NULL;
   GtwoDarray_ptr=NULL;
   BtwoDarray_ptr=NULL;
   AtwoDarray_ptr=NULL;

   poDataset_ptr=NULL;
   poBand_ptr=NULL;

#ifndef TOC12_LAPTOP_FLAG

   GDALAllRegister();

#endif

}		 

// ---------------------------------------------------------------------
raster_parser::raster_parser()
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

raster_parser::raster_parser(const raster_parser& d)
{
   docopy(d);
}

void raster_parser::null_ztwoDarray_ptr()
{
//   cout << "inside raster_parser::null_ztwoDarray_ptr()" << endl;
   ztwoDarray_ptr=NULL;
}

raster_parser::~raster_parser()
{
//   cout << "inside raster_parser destructor" << endl;

   close_image_file();

   delete ztwoDarray_ptr;

   delete RtwoDarray_ptr;
   delete GtwoDarray_ptr;
   delete BtwoDarray_ptr;
   delete AtwoDarray_ptr;

   delete M_ptr;
   delete scanline_buffer;
}

// ---------------------------------------------------------------------
void raster_parser::docopy(const raster_parser& d)
{
//   cout << "inside raster_parser::docopy()" << endl;
//   outputfunc::enter_continue_char();
}

// Overload = operator:

raster_parser& raster_parser::operator= (const raster_parser& d)
{
   if (this==&d) return *this;
   docopy(d);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const raster_parser& d)
{
   outstream << "northern_hemisphere_flag = " << d.northern_hemisphere_flag
             << endl;
   outstream << "specified_UTM_zonenumber = " << d.specified_UTM_zonenumber
             << endl;
   
   outstream << endl;
   return outstream;
}

// =========================================================================
// Set & get member functions
// =========================================================================

int raster_parser::get_raster_Xsize() const
{
#ifndef TOC12_LAPTOP_FLAG
   return poDataset_ptr->GetRasterXSize();
#endif
}

int raster_parser::get_raster_Ysize() const
{
#ifndef TOC12_LAPTOP_FLAG
   return poDataset_ptr->GetRasterYSize();
#endif   
}

// =========================================================================
// GDAL image import member functions
// =========================================================================

bool raster_parser::open_image_file(
   string image_filename,bool predelete_ztwoDarray_ptr_flag)
{

#ifndef TOC12_LAPTOP_FLAG

//   cout << "inside raster_parse::open_image_file()" << endl;
//   cout << "image_filename = " << image_filename << endl;
   bool imagefile_successfully_opened=true;

   poDataset_ptr = (GDALDataset *) GDALOpen( 
      image_filename.c_str(), GA_ReadOnly );
   if ( poDataset_ptr == NULL )
   {
//      cout << "Could not open input image file" << endl;
      return false;
   }
      
//   cout << "Driver = " 
//        << poDataset_ptr->GetDriver()->GetDescription()
//        << poDataset_ptr->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME )
//        << endl;

   n_channels=poDataset_ptr->GetRasterCount();
//   cout << "n_channels = " << n_channels << endl;

   if (n_channels==1)
   {

// On Dec 11, 2009, we discovered the hard and painful way that the
// following delete ztwoDarray_ptr line caused a segmentation fault
// when this method was called from
// TilesGroup::update_avg_LOS_tiles().  For speed purposes, we set TWO
// pointers equal to this class' ztwoDarray_ptr.  But in this case,
// the TilesGroup class must become responsible for explicitly
// deleting the pointers after they're no longer needed.  So
// predelete_ztwoDarray_flag = false for this special case.
// Otherwise, we generally do want to delete ztwoDarray_ptr prior to
// instantiating a new ztwoDarray:

      if (predelete_ztwoDarray_ptr_flag) delete ztwoDarray_ptr;
      
      ztwoDarray_ptr=new twoDarray(
         poDataset_ptr->GetRasterXSize(),poDataset_ptr->GetRasterYSize());
   }
   else
   {
//      cout << "Xsize = " << poDataset_ptr->GetRasterXSize() << endl;
//      cout << "Ysize = " << poDataset_ptr->GetRasterYSize() << endl;
      
      delete RtwoDarray_ptr;
      delete GtwoDarray_ptr;
      delete BtwoDarray_ptr;
      delete AtwoDarray_ptr;
      RtwoDarray_ptr=new twoDarray(
         poDataset_ptr->GetRasterXSize(),poDataset_ptr->GetRasterYSize());
      GtwoDarray_ptr=new twoDarray(
         poDataset_ptr->GetRasterXSize(),poDataset_ptr->GetRasterYSize());
      BtwoDarray_ptr=new twoDarray(
         poDataset_ptr->GetRasterXSize(),poDataset_ptr->GetRasterYSize());
      AtwoDarray_ptr=new twoDarray(
         poDataset_ptr->GetRasterXSize(),poDataset_ptr->GetRasterYSize());
      ztwoDarray_ptr=RtwoDarray_ptr;
   }

//   cout << "Image size = " << ztwoDarray_ptr->get_xdim() 
//        << " x " << ztwoDarray_ptr->get_ydim() << " x " 
//        << n_channels << endl;
   
   if ( poDataset_ptr->GetProjectionRef()  != NULL )
   {
      projection_WKT=poDataset_ptr->GetProjectionRef();
//      cout << "Image projection is " << projection_WKT << endl;

      OGRSpatialReference oSRS;
      char* projection_WKT_cstr=const_cast<char *>(projection_WKT.c_str());
      oSRS.importFromWkt( &projection_WKT_cstr );

      int pbNorth;
      specified_UTM_zonenumber=oSRS.GetUTMZone(&pbNorth);
      northern_hemisphere_flag=static_cast<bool>(pbNorth);
//      cout << "specified_UTM_zonenumber = " << specified_UTM_zonenumber 
//           << endl;
//      cout << "northern_hemisphere_flag = " << northern_hemisphere_flag
//           << endl;

   }
//   outputfunc::enter_continue_char();

   double adfGeoTransform[6];

//    adfGeoTransform[0] /* top left x */
//    adfGeoTransform[1] /* w-e pixel resolution */
//    adfGeoTransform[2] /* rotation, 0 if image is "north up" */
//    adfGeoTransform[3] /* top left y */
//    adfGeoTransform[4] /* rotation, 0 if image is "north up" */
//    adfGeoTransform[5] /* n-s pixel resolution */

   if ( poDataset_ptr->GetGeoTransform( adfGeoTransform ) == CE_None )
   {
//      cout << "Origin = ( " << adfGeoTransform[0] << " , " 
//           << adfGeoTransform[3] << " ) " << endl;

      trans=twovector(adfGeoTransform[0],adfGeoTransform[3]);
      M_ptr->put(0,0,adfGeoTransform[1]);
      M_ptr->put(0,1,adfGeoTransform[2]);
      M_ptr->put(1,0,adfGeoTransform[4]);
      M_ptr->put(1,1,adfGeoTransform[5]);

      ztwoDarray_ptr->set_xlo(adfGeoTransform[0]);
      ztwoDarray_ptr->set_deltax(adfGeoTransform[1]);
      ztwoDarray_ptr->set_xhi(
         ztwoDarray_ptr->get_xlo()
         +(ztwoDarray_ptr->get_mdim()-1)*ztwoDarray_ptr->get_deltax());

      ztwoDarray_ptr->set_yhi(adfGeoTransform[3]);
      ztwoDarray_ptr->set_deltay(-adfGeoTransform[5]);      
      ztwoDarray_ptr->set_ylo(
         ztwoDarray_ptr->get_yhi()
         -(ztwoDarray_ptr->get_ndim()-1)*ztwoDarray_ptr->get_deltay());

//      cout.precision(12);
//      cout << "xlo,xhi = " << ztwoDarray_ptr->get_xlo() << " , "
//           << ztwoDarray_ptr->get_xhi() << endl;
//      cout << "ylo,yhi = " << ztwoDarray_ptr->get_ylo() << " , "
//           << ztwoDarray_ptr->get_yhi() << endl;
//      cout << "Pixel size = ( " 
//           << ztwoDarray_ptr->get_deltax() << " , "
//           << ztwoDarray_ptr->get_deltay() << " ) " << endl;

//      cout << "adfGeoTransform[2]  = " << adfGeoTransform[2]
//           << " adfGeoTransform[4] = " << adfGeoTransform[4] << endl;
   }

   if (n_channels==3) ztwoDarray_ptr=NULL;

//   cout << "imagefile_successfully_opened = "
//        << imagefile_successfully_opened << endl;
   return imagefile_successfully_opened;

#endif

}		 

// ---------------------------------------------------------------------
void raster_parser::close_image_file()
{
   if (poDataset_ptr != NULL)
   {
      delete poDataset_ptr;
      poDataset_ptr=NULL;
   }
}

// ---------------------------------------------------------------------
// Member function fetch_raster_band

// At this time access to raster data via GDAL is done one band at a
// time. Also, metadata, blocksizes, color tables, and various other
// information are available on a band by band basis. The following
// codes fetches a GDALRasterBand object from the dataset (numbered 1
// through GetRasterCount()) and displays a little information about
// it.

void raster_parser::fetch_raster_band(int channel_ID)
{
//    cout << "inside raster_parser::fetch_raster_band()" << endl;
   
#ifndef TOC12_LAPTOP_FLAG

   int nBlockXSize, nBlockYSize;
   poBand_ptr = poDataset_ptr->GetRasterBand( channel_ID+1 );
   poBand_ptr->GetBlockSize( &nBlockXSize, &nBlockYSize );

//   cout << "Block = " << nBlockXSize << " x " << nBlockYSize
//        << " Type = " << GDALGetDataTypeName(poBand_ptr->GetRasterDataType())
//        << " ColorInterp = " << GDALGetColorInterpretationName(
//           poBand_ptr->GetColorInterpretation()) << endl;

   int bGotMin, bGotMax;
   double adfMinMax[2];
   adfMinMax[0] = poBand_ptr->GetMinimum( &bGotMin );
   adfMinMax[1] = poBand_ptr->GetMaximum( &bGotMax );
   if ( ! (bGotMin && bGotMax) )
      GDALComputeRasterMinMax((GDALRasterBandH)poBand_ptr, TRUE, adfMinMax);

   min_z=adfMinMax[0];
   max_z=adfMinMax[1];
//   cout << "Min = " << min_z << " Max = " << max_z << endl;

   if ( poBand_ptr->GetOverviewCount() > 0 )
   {
      cout << "Band has " << poBand_ptr->GetOverviewCount()
           << " overviews" << endl;
   }

   if ( poBand_ptr->GetColorTable() != NULL )
   {
//      cout << "Band has a color table with " 
//           << poBand_ptr->GetColorTable()->GetColorEntryCount() << " entries"
//           << endl;
   }

#endif

}

// ---------------------------------------------------------------------
// Member function read_raster_data

// There are a few ways to read raster data, but the most common is
// via the GDALRasterBand::RasterIO() method. This method will
// automatically take care of data type conversion, up/down sampling
// and windowing. The following code will read the first scanline of
// data into a similarly sized buffer, converting it to floating point
// as part of the operation.



void raster_parser::read_raster_data(twoDarray* ztwoDarray_ptr)
{
//   cout << "inside raster_parser::read_raster_data()" << endl;
   ztwoDarray_ptr->clear_values();

   unsigned int mdim=ztwoDarray_ptr->get_mdim();
   unsigned int ndim=ztwoDarray_ptr->get_ndim();
//   cout << "mdim = " << mdim << " ndim = " << ndim << endl;

   int nXoffset=0;
   for (unsigned int py=0; py<ndim; py++)
   {
//      if (py%1000==0) cout << py << " " << flush;

#ifndef TOC12_LAPTOP_FLAG

      poBand_ptr->RasterIO( 
         GF_Read, nXoffset, py , mdim, 1, 
         scanline_buffer, mdim, 1, GDT_Float64, 
         0, 0 );

#endif

//      memcpy(
//         ztwoDarray_ptr->get_e_ptr()+py*mdim,
//         scanline_buffer,mdim*sizeof(double));

      for (unsigned int px=0; px<mdim; px++)
      {
//         cout << "px=" << px << " py = " << py 
//              << " z.get(py,px) = " << ztwoDarray_ptr->get(py,px)
//              << " buffer[px]=" 
//              << scanline_buffer[px] << endl;

         ztwoDarray_ptr->put(px,py,scanline_buffer[px]);
      }
   }
//   cout << endl;
}



// ---------------------------------------------------------------------
// Member function read_single_channel_image() imports the contents
// from a grey-scale image with a single channel rather than an RGB
// image with 3 channels.  It instantiates and returns a twoDarray
// containing the grey-scale image information.  This method is
// intended to be high-level and self-contained.

twoDarray* raster_parser::read_single_channel_image(std::string image_filename)
{
//   cout << "inside raster_parser::read_single_channel_image()" << endl;
//   cout << "image_filename = " << image_filename << endl;

   twoDarray* zcopy_twoDarray_ptr=NULL;

   bool file_read_flag=open_image_file(image_filename);
//   cout << "file_read_flag = " << file_read_flag << endl;
   
   if (file_read_flag)
   {
      int channel_ID=0;
      fetch_raster_band(channel_ID);

      read_raster_data(ztwoDarray_ptr);
      zcopy_twoDarray_ptr=new twoDarray(ztwoDarray_ptr);
      ztwoDarray_ptr->copy(zcopy_twoDarray_ptr);
      close_image_file();
   }
   
   return zcopy_twoDarray_ptr;
}

// =========================================================================
// GDAL image export member functions
// =========================================================================

// Member function write_raster_data takes in twoDarray
// *ztwoDarray_ptr.  It generates WGS-84 coordinate system metadata
// output for the single-channel imagery.  If input boolean
// output_float_flag==true, this method writes floats to the output
// geotiff file.  Otherwise, it rescales and quantizes the Z values so
// that they range from 0 to 2**16-1.  As Ross Anderson reminded us on
// 4/24/07, many standard 2D image views (e.g. GIMP, display, xview,
// etc) have difficulty displaying an array of floats.  So we follow
// Ross' advice and convert the output data to an array of short
// integers.

void raster_parser::write_raster_data(
   bool output_floats_flag,string geotiff_filename,
   int output_UTM_zonenumber,bool output_northern_hemisphere_flag,
   twoDarray* ztwoDarray_ptr)
{
//   cout << "inside raster_parser::write_raster_data()" << endl;
   double z_min,z_max;
   ztwoDarray_ptr->minmax_values(
      z_min,0.5*NEGATIVEINFINITY,z_max,0.5*POSITIVEINFINITY);
//   cout << "z_min = " << z_min << " z_max = " << z_max << endl;
   write_raster_data(
      output_floats_flag,geotiff_filename,
      output_UTM_zonenumber,output_northern_hemisphere_flag,
      ztwoDarray_ptr,z_min,z_max);
}

// ---------------------------------------------------------------------
// We wrote this next overloaded version of write_raster_data which
// takes in z_min and z_max parameters in order to convert multiple
// input TDP tiles to geotiffs on a common height scale:

void raster_parser::write_raster_data(
   bool output_floats_flag,string geotiff_filename,
   int output_UTM_zonenumber,bool output_northern_hemisphere_flag,
   twoDarray* ztwoDarray_ptr,double z_min,double z_max)
{

#ifndef TOC12_LAPTOP_FLAG

//   cout << "inside raster_parser::write_raster_data() #2" << endl;
//   cout << "z_min = " << z_min << " z_max = " << z_max << endl;
//   cout << "output_floats_flag = " << output_floats_flag << endl;

// First set some raster_parser member variables equal to input
// parameters:

   specified_UTM_zonenumber=output_UTM_zonenumber;
   northern_hemisphere_flag=output_northern_hemisphere_flag;

   const char* pszFormat = "GTiff";
   GDALDriver* poDriver_ptr = 
      GetGDALDriverManager()->GetDriverByName(pszFormat);

   if ( poDriver_ptr == NULL ) 
   {
      cout << "poDriver_ptr = NULL!" << endl;
      exit(1);
   }

/*
   char** papszMetadata = poDriver_ptr->GetMetadata();
   if ( CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATE, FALSE ) )
   {
      cout << "Driver " << pszFormat << " supports Create() method"
           << endl;
   }

   if ( CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATECOPY, FALSE ) )
   {
      cout << "Driver " << pszFormat << " supports CreateCopy() method"
           << endl;
   }
*/

   int xdim=ztwoDarray_ptr->get_xdim();
   int ydim=ztwoDarray_ptr->get_ydim();
   n_channels=1;
   char** papszOptions = NULL;
   GDALDataset* poDstDS_ptr =NULL;
   if (output_floats_flag)
   {
      poDstDS_ptr = poDriver_ptr->Create( 
         geotiff_filename.c_str(), 
         xdim, ydim, n_channels, GDT_Float32, papszOptions );
   }
   else
   {
      poDstDS_ptr = poDriver_ptr->Create( 
      geotiff_filename.c_str(), 
      xdim, ydim, n_channels, GDT_UInt16, papszOptions );
   }

   int xlo=static_cast<int>(ztwoDarray_ptr->get_xlo());
//   int xhi=static_cast<int>(ztwoDarray_ptr->get_xhi());
//   int ylo=static_cast<int>(ztwoDarray_ptr->get_ylo());
   int yhi=static_cast<int>(ztwoDarray_ptr->get_yhi());
   double delta_x=ztwoDarray_ptr->get_deltax();
   double delta_y=ztwoDarray_ptr->get_deltay();
   
//   cout << "xdim = " << xdim << " ydim = " << ydim << endl;
//   cout << "xlo = " << xlo << " xhi = " << xhi << endl;
//   cout << "ylo = " << ylo << " yhi = " << yhi << endl;
//   cout << "delta_x = " << delta_x << " delta_y = " << delta_y << endl;

// Set up spatial coordinate system for geotiff imagery output:

   double adfGeoTransform[6] = { 
      xlo,delta_x,0,
      yhi,0,-delta_y};
   poDstDS_ptr->SetGeoTransform( adfGeoTransform );

   OGRSpatialReference oSRS;
   oSRS.SetUTM( output_UTM_zonenumber, output_northern_hemisphere_flag );
   oSRS.SetWellKnownGeogCS( "WGS84" );

   char* pszSRS_WKT = NULL;
   oSRS.exportToWkt( &pszSRS_WKT );
//   cout << "psqSRS_WKT  = " << pszSRS_WKT << endl;

   poDstDS_ptr->SetProjection( pszSRS_WKT );
   CPLFree( pszSRS_WKT );

//   cout << "n_channels = " << n_channels << endl;
//   cout << "xdim = " << xdim << " ydim = " << ydim << endl;
   for (unsigned int n=0; n<n_channels; n++)
   {
      GDALRasterBand* poBand_ptr = poDstDS_ptr->GetRasterBand(n+1);

      if (output_floats_flag)
      {
         float* Z=new float[xdim*ydim];
         fill_Z_float_array(ztwoDarray_ptr,Z);
         poBand_ptr->RasterIO( GF_Write, 0, 0, xdim, ydim, 
                               Z, xdim, ydim, GDT_Float32, 0, 0 );    
         delete [] Z;      
      }
      else
      {
         GUInt16* Z=new GUInt16[xdim*ydim];
         fill_Z_GUInt_array(z_min,z_max,ztwoDarray_ptr,Z);
         poBand_ptr->RasterIO( GF_Write, 0, 0, xdim, ydim, 
                               Z, xdim, ydim, GDT_UInt16, 0, 0 );    
         delete [] Z;      
      }
//      else
//      {
//         unsigned char* Z=new unsigned char[xdim*ydim];
//         fill_Z_unsigned_char_array(z_min,z_max,ztwoDarray_ptr,Z,true);
//         poBand_ptr->RasterIO( GF_Write, 0, 0, xdim, ydim, 
//                               Z, xdim, ydim, GDT_Byte, 0, 0 );    
//         delete [] Z;      
//      }

   } // loop over index n labeling color channel
   
   delete poDstDS_ptr;

#endif

}

// ---------------------------------------------------------------------
void raster_parser::write_colored_raster_data(
   string geotiff_filename,
   int output_UTM_zonenumber,bool output_northern_hemisphere_flag,
   twoDarray* RtwoDarray_ptr,twoDarray* GtwoDarray_ptr,
   twoDarray* BtwoDarray_ptr,twoDarray* AtwoDarray_ptr)
{
//   cout << "inside raster_parser::write_colored_raster_data()" << endl;

   double z_min=0;
   double z_max=1;
   write_colored_raster_data(
      geotiff_filename,output_UTM_zonenumber,output_northern_hemisphere_flag,
      RtwoDarray_ptr,GtwoDarray_ptr,BtwoDarray_ptr,AtwoDarray_ptr,
      z_min,z_max);
}

// ---------------------------------------------------------------------
// We wrote this next overloaded version of write_raster_data which
// takes in z_min and z_max parameters in order to convert multiple
// input TDP tiles to geotiffs on a common height scale:

void raster_parser::write_colored_raster_data(
   string geotiff_filename,
   int output_UTM_zonenumber,bool output_northern_hemisphere_flag,
   twoDarray* RtwoDarray_ptr,twoDarray* GtwoDarray_ptr,
   twoDarray* BtwoDarray_ptr,twoDarray* AtwoDarray_ptr,
   double z_min,double z_max)
{

#ifndef TOC12_LAPTOP_FLAG

//   cout << "inside raster_parser::write_colored_raster_data()" << endl;
//   cout << "z_min = " << z_min << " z_max = " << z_max << endl;

// First set some raster_parser member variables equal to input
// parameters:

   specified_UTM_zonenumber=output_UTM_zonenumber;
   northern_hemisphere_flag=output_northern_hemisphere_flag;
   
   const char* pszFormat = "GTiff";
   GDALDriver* poDriver_ptr = 
      GetGDALDriverManager()->GetDriverByName(pszFormat);

   if ( poDriver_ptr == NULL ) 
   {
      cout << "poDriver_ptr = NULL!" << endl;
      exit(1);
   }

   int xdim=RtwoDarray_ptr->get_xdim();
   int ydim=RtwoDarray_ptr->get_ydim();
   n_channels=4;
   char** papszOptions = NULL;
   GDALDataset* poDstDS_ptr = poDriver_ptr->Create( 
      geotiff_filename.c_str(), 
      xdim, ydim, n_channels, GDT_Byte, papszOptions );

   int xlo=static_cast<int>(RtwoDarray_ptr->get_xlo());
//   int xhi=static_cast<int>(RtwoDarray_ptr->get_xhi());
//   int ylo=static_cast<int>(RtwoDarray_ptr->get_ylo());
   int yhi=static_cast<int>(RtwoDarray_ptr->get_yhi());
   double delta_x=RtwoDarray_ptr->get_deltax();
   double delta_y=RtwoDarray_ptr->get_deltay();
   
//   cout << "xdim = " << xdim << " ydim = " << ydim << endl;
//   cout << "xlo = " << xlo << endl;
//   cout << " xhi = " << xhi << endl;
//   cout << "ylo = " << ylo << endl;
//   cout << " yhi = " << yhi << endl;
//   cout << "delta_x = " << delta_x << " delta_y = " << delta_y << endl;

// Set up spatial coordinate system for geotiff imagery output:

   double adfGeoTransform[6] = { 
      xlo,delta_x,0,
      yhi,0,-delta_y};
   poDstDS_ptr->SetGeoTransform( adfGeoTransform );

   OGRSpatialReference oSRS;
   oSRS.SetUTM( output_UTM_zonenumber, output_northern_hemisphere_flag );
   oSRS.SetWellKnownGeogCS( "WGS84" );

   char* pszSRS_WKT = NULL;
   oSRS.exportToWkt( &pszSRS_WKT );
   poDstDS_ptr->SetProjection( pszSRS_WKT );
   CPLFree( pszSRS_WKT );

   for (unsigned int n=0; n<n_channels; n++)
   {
      GDALRasterBand* poBand_ptr = poDstDS_ptr->GetRasterBand(n+1);
      twoDarray* ztwoDarray_ptr=NULL;
      if (n==0)
      {
         ztwoDarray_ptr=RtwoDarray_ptr;
      }
      else if (n==1)
      {
         ztwoDarray_ptr=GtwoDarray_ptr;
      }
      else if (n==2)
      {
         ztwoDarray_ptr=BtwoDarray_ptr;
      }
      else if (n==3)
      {
         ztwoDarray_ptr=AtwoDarray_ptr;
      }
      
      unsigned char* Z=new unsigned char[xdim*ydim];
      bool rescale_zvalues_flag=true;
      fill_Z_unsigned_char_array(
         z_min,z_max,ztwoDarray_ptr,Z,rescale_zvalues_flag);
      poBand_ptr->RasterIO( 
         GF_Write, 0, 0, xdim, ydim, 
         Z, xdim, ydim, GDT_Byte, 1 , 0 );    
      delete [] Z;      

   } // loop over index n labeling color channel
   
   delete poDstDS_ptr;

// Do not return from this method until colored geotif file is
// actually written out to disk...

   bool colored_geotif_exists_flag=false;
   while (!colored_geotif_exists_flag)
   {
      cout << "Waiting for export of colored_geotif " 
           << geotiff_filename << " to finish" << endl;
      colored_geotif_exists_flag=filefunc::fileexist(geotiff_filename);
   }

#endif

}

// =========================================================================
// Linear array value filling member functions
// =========================================================================

void raster_parser::fill_Z_unsigned_char_array(
   double z_min,double z_max,const twoDarray* ztwoDarray_ptr,unsigned char* Z,
   bool rescale_zvalues_flag)
{
//   cout << "inside raster_parser::fill_Z_unsigned_char_array()" << endl;
   
   int counter=0;
   for (unsigned int py=0; py<ztwoDarray_ptr->get_ydim(); py++)
   {
      for (unsigned int px=0; px<ztwoDarray_ptr->get_xdim(); px++)
      {
         double curr_z=basic_math::max(0.0,ztwoDarray_ptr->get(px,py));
         
// Rescale Z values so that they range from 0 to 2**8-1=255:

         int z_rescaled;
         if (rescale_zvalues_flag)
         {
            const double factor=255;
            z_rescaled=static_cast<int>(
               factor*(curr_z-z_min)/(z_max-z_min));
            z_rescaled=basic_math::max(0,z_rescaled);
            z_rescaled=basic_math::min(255,z_rescaled);
         }
         else
         {
            z_rescaled=curr_z;
         }
         
         unsigned char z_char=stringfunc::ascii_integer_to_unsigned_char(
            z_rescaled);

//         cout << "px = " << px << " py = " << py 
//              << " curr_z = " << curr_z 
//              << " z_rescaled = " << z_rescaled << endl;

         Z[counter++]=z_char;
      }
   }
}

// ---------------------------------------------------------------------
// Member function fill_Z_GUInt_array() maps z values within the
// interval [z_min, z_max] onto the unsigned 2-byte integer interval
// [0, 2**16-1=65535].  The results are returned within output array
// Z.

void raster_parser::fill_Z_GUInt_array(
   double z_min,double z_max,const twoDarray* ztwoDarray_ptr,GUInt16* Z)
{
//   cout << "inside raster_parser::fill_Z_GUInt_array()" << endl;
   
   int counter=0;
   for (unsigned int py=0; py<ztwoDarray_ptr->get_ydim(); py++)
   {
      for (unsigned int px=0; px<ztwoDarray_ptr->get_xdim(); px++)
      {
//         double curr_z=basic_math::max(0.0,ztwoDarray_ptr->get(px,py));
         double curr_z=ztwoDarray_ptr->get(px,py);
         
// Rescale Z values so that they range from 0 to 2**16-1:

         const double factor=65535;
         GUInt16 z_quantized=static_cast<GUInt16>(
            factor*(curr_z-z_min)/(z_max-z_min));

//         if (z_quantized > 0)
//         {
//            cout << "px = " << px << " py = " << py 
//                 << " curr_z = " << curr_z 
//                 << " z_quant = " << z_quantized << endl;
//         }

         Z[counter++]=z_quantized;
      }
   }
}

// ---------------------------------------------------------------------
void raster_parser::fill_Z_float_array(
   const twoDarray* ztwoDarray_ptr,float* Z)
{
//   cout << "inside raster_parser::fill_Z_float_array()" << endl;
   double min_Z=POSITIVEINFINITY;
   double max_Z=NEGATIVEINFINITY;

   int counter=0;
   for (unsigned int py=0; py<ztwoDarray_ptr->get_ydim(); py++)
   {
      for (unsigned int px=0; px<ztwoDarray_ptr->get_xdim(); px++)
      {
         double curr_z=ztwoDarray_ptr->get(px,py);
         Z[counter++]=curr_z;

/*
         if (curr_z < 0.5*NEGATIVEINFINITY)
         {
            cout << "px = " << px << " py = " << py 
                 << " curr_z = " << curr_z << endl;
         }
*/
         min_Z=basic_math::min(min_Z,curr_z);
         max_Z=basic_math::max(max_Z,curr_z);
      }
   }

//   cout << "At end of raster_parser::fill_Z_float_array()" << endl;
//   cout << "min_Z = " << min_Z << " max_Z = " << max_Z << endl;
//   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
void raster_parser::fill_Z_double_array(
   const twoDarray* ztwoDarray_ptr,double* Z)
{
   int counter=0;
   for (unsigned int py=0; py<ztwoDarray_ptr->get_ydim(); py++)
   {
      for (unsigned int px=0; px<ztwoDarray_ptr->get_xdim(); px++)
      {
         Z[counter++]=ztwoDarray_ptr->get(px,py);
      }
   }
}

// ---------------------------------------------------------------------
// Member function convert_GUInts_to_doubles() maps quantized 2-byte
// integer *ztwoDarray_ptr entries within interval [0, 2**16-1=65535]
// onto continuous values within interval [z_min, z_max].

void raster_parser::convert_GUInts_to_doubles(
   double z_min,double z_max,twoDarray* ztwoDarray_ptr)
{
   for (unsigned int py=0; py<ztwoDarray_ptr->get_ydim(); py++)
   {
      for (unsigned int px=0; px<ztwoDarray_ptr->get_xdim(); px++)
      {
         double z_quantized=ztwoDarray_ptr->get(px,py);
         const double factor=65535.0;
         double z_continuous=z_min+(z_max-z_min)*z_quantized/factor;
         ztwoDarray_ptr->put(px,py,z_continuous);

//         if (z_quantized > 0)
//         {
//            cout << "py = " << py << " px = " << px
//                 << " z_quant = " << z_quantized
//                 << " z_cont = " << z_continuous << endl;
//         }
         
      }
   }
}

// =========================================================================
// Geo value extraction member functions
// =========================================================================

bool raster_parser::get_Zvalue(double longitude,double latitude,
                               double& Zvalue)
{
   unsigned int px,py;
   bool point_inside_region=
      ztwoDarray_ptr->point_to_pixel(longitude,latitude,px,py);
   if (point_inside_region)
   {
      Zvalue=ztwoDarray_ptr->get(px,py);
   }
   return point_inside_region;
}
