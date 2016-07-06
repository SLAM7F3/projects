/*---------------------------------------------------------------------------
 *                        Copyright © 2005-2010
 *                   Massachusetts Institute of Technology
 *                         All rights reserved
 *---------------------------------------------------------------------------
 *
 * Description: Kakadu support routines.  In particular, GeoTIFF smart header
 *     box for specifying the geolocation of images.
 *
 * Author: Herb DaSilva - June 9, 2006
 *         Cindy Fang - Edits for variable precision and signage, precincts,
 *                     and fixes for compressed size.  Feb.2007
 *---------------------------------------------------------------------------*/

#include "KakaduSupport.h"
#include "kdu_arch.h"

#include <memory>

static int doubleBufferSize = 16;
static int numCPUs = 1;

static kd_core_message_collector err_collector( true );
static kdu_message_formatter err_formatter( &err_collector, 256 );



/***************************************************************************
 * function KDUInit
 *
 * Initialize the number of threads to use when multi-threaded-enabled
 * routines in this module are called.  If this routine is not called, then
 * the number of threads defaults to 1.  If a non-zero 'limit' is supplied,
 * then no more than that number of threads will be used, otherwise, the
 * number of CPUs determines the number of threads.
 ***************************************************************************/
void KDUInit( int limit )
{
    KakaduInit(limit);
}

void KakaduInit( int limit )
{
  static int setup = 0;


  if (setup != 0)
    return;

  // If there is more than one CPU, Create Multi-threading environment.
  numCPUs = kdu_get_num_processors();
  PutLog( "KDUInit (KakaduSupport) - %d CPUs detected\n", numCPUs );

  // Test for user-imposed limit.
  if (limit > 0) {
    if (limit < numCPUs)
      numCPUs = limit;
  }
  setup = 1;

} // end KDUInit



/***************************************************************************
 * function KDUGetCPUCount
 *
 * Returns the number of CPUs detected by KDUInit above.
 * If the application didn't call KDUInit to set up multithreading,
 * then we'll return 1.
 ***************************************************************************/

int KDUGetCPUCount( void )
{
  return numCPUs;
}



/*
 * GeoTIFF header encoding stuff begins.
 *
 * The GeoTIFF header is a composite entity comprising a standard TIFF header
 * sequence, and then a degenerate GeoTIFF image specification.  The trick is
 * that the GeoTIFF header is actually written into the JP2 as a UUID box,
 * which is ignored by JP2 readers that don't understand it, but is decoded
 * by applications (like ERDAS Imagine) that do understand it, and reveals
 * the geographical location of the image.  In our case, we're just encoding
 * the UTM Zone, upper-left-hand corner UTM location, and the proper scaling,
 * which should allow the application reading it to determine the UTM
 * lower-right-hand corner.
 *
 * Yes, there are a LOT of magic numbers in here.  The GeoTIFF specification
 * takes an already cumbersome specification (TIFF), and makes it just a bit
 * worse.  In particular, the TIFF and GeoTIFF specs *love* offsets, and most
 * of the game in getting the specification right is in correctly computing
 * the offsets.  So, modify the following code at your peril!  ;-)
 *
 * If they still exist, the documents that I found on the Internet that
 * really helped put this all together are: 
 * 
 * http://www.lizardtech.com/support/kb/docs/geotiff_box.txt
 * http://partners.adobe.com/public/developer/en/tiff/TIFF6.pdf
 * http://www.remotesensing.org/geotiff/spec/contents.html
 *
 * for the JP2 UUID, TIFF, and GeoTIFF specifications, respectively.
 */


// This is the UUID JP2 box that wraps up the GeoTIFF info.
#define TIFFBOXSIZE 16
static unsigned char geoTiffBox[TIFFBOXSIZE] = {
  0xb1, 0x4b, 0xf8, 0xbd,
  0x08, 0x3d, 0x4b, 0x43,
  0xa5, 0xae, 0x8c, 0xd7,
  0xd5, 0xa6, 0xce, 0x03
};

// TIFF/GeoTIFF header key/tag type definitions.
#define BYTE  0x01
#define ASCII 0x02
#define SHORT 0x03
#define LONG  0x04
#define RATNL 0x05
#define DOUBL 0x0C

// Standard TIFF tag directory entry structure.
#define NUMTIFFTAGS 14
typedef struct tifftagA {
  unsigned short tag;
  unsigned short type;
  unsigned int   length;
  unsigned int   valueOrOffset;
} TIFFTagA;

// Standard TIFF file header stream.
#define HEADERCHARSIZE 11
static unsigned char headerChars[HEADERCHARSIZE] = {
  0x49, 0x49, 0x2A, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, NUMTIFFTAGS, 0x00 };

// GeoTIFF UTM zone encoding definitions.
#define UTM_NORTH_START 32601  // WGS84 UTM Zone 1N, 60N = 32660
#define UTM_SOUTH_START 32701  // WGS84 UTM Zone 1S, 60S = 32760

// GeoTIFF key entries.
#define GEOKEYSIZE 16
static unsigned short geoKeyData[GEOKEYSIZE] = {
  1, 1, 0, 3,    // Mandatory GeoKey Header - KeyDirRev,KeyVer,KeyMinor, #Keys
  1024, 0, 1, 1, // Model Type: Projected
  1025, 0, 1, 1, // Raster Space: Pixel Is Area
  3072, 0, 1, UTM_NORTH_START // Projected Type Key - WGS84 -
                 // UTM_NORTH_START must be replaced with actual zone below!
};

#define DUMMY_NORTHING 0.0
#define DUMMY_EASTING 0.0
#define DUMMY_XSCALE 2000.0
#define DUMMY_YSCALE 2000.0
#define NUMPIXSCALE 3
#define NUMTIEPOINT 6
// GeoTIFF double-sized value stream. 
// Replace dummy values with the real data in the code below.
#define GEODOUBLESIZE 9
static double geoDoubles[GEODOUBLESIZE] = { 
  DUMMY_XSCALE, DUMMY_YSCALE, 0.0,                  // Pixel Spacing
  0.0, 0.0, 0.0, DUMMY_EASTING, DUMMY_NORTHING, 0.0 // Upper-left UTM
};

// Date MUST be of the format "YYYY:MM:DD HH:MM:SS"
#define DATESIZE 20
static char date[DATESIZE];

// TIFF header directory entries.  The degenerate GeoTIFF is very specific.
// This includes standard TIFF tags, plus the GeoTIFF tags that point to the
// GeoTIFF key and value tables.
TIFFTagA tags[NUMTIFFTAGS] = {
  { 256, SHORT, 1, 1 }, // Width
  { 257, SHORT, 1, 1 }, // Height
  { 258, SHORT, 1, 8 }, // BitsPerSample
  { 262, SHORT, 1, 1 }, // PhotometricInterp
  { 273, SHORT, 1, 8 }, // StripOffsets
  { 277, SHORT, 1, 1 }, // SamplesPerPixel
  { 278, SHORT, 1, 1 }, // RowsPerStrip
  { 279, SHORT, 1, 1 }, // StripByteCounts
  { 284, SHORT, 1, 1 }, // PlanarConfiguration
  { 296, SHORT, 1, 1 }, // ResolutionUnit
  { 306, ASCII, DATESIZE, NUMTIFFTAGS * 12 + HEADERCHARSIZE +
    GEOKEYSIZE * sizeof(unsigned short) + GEODOUBLESIZE * sizeof(double) }, // Date/Time
  { 33550, DOUBL, NUMPIXSCALE, NUMTIFFTAGS * 12 + HEADERCHARSIZE +
    GEOKEYSIZE * sizeof(unsigned short) }, // PixelScale
  { 33922, DOUBL, NUMTIEPOINT, NUMTIFFTAGS * 12 + HEADERCHARSIZE +
    GEOKEYSIZE * sizeof(unsigned short) + NUMPIXSCALE * sizeof(double) }, // TiePoint GeoreferenceModel
  { 34735, SHORT, GEOKEYSIZE, NUMTIFFTAGS * 12 + HEADERCHARSIZE } // GeoKeyDir
};



/***************************************************************************
 * function GetUTMIndex
 *
 * Determine the proper GeoTIFF UTM index from the supplied UTM zone string.
 * The zone string is expected to be 1 or 2 ASCII numbers and one letter.
 ***************************************************************************/

static int GetUTMIndex( char zone[ZONE_LEN] )
{
  int  code;
  int  zNum;
  char zChar;
  char northCodes[] = "NPQRSTUVWX";


  sscanf( zone, "%d%c", &zNum, &zChar );
  if (strchr( northCodes, zChar ) == NULL)
    code = zNum - 1 + UTM_SOUTH_START;
  else
    code = zNum - 1 + UTM_NORTH_START;

  PutLog( "GetUTMIndex - zone: %s, code: %d\n", zone, code );
  return code;

} /* end GetUTMIndex */



/***************************************************************************
 * function FormatTime
 *
 * Create TIFF-legal time string from time nugget.
 * If time64 is 0, a blank string of the correct length is returned.
 ***************************************************************************/

static void FormatTime( __int64 time64, char str[] )
{
  time_t time;
  tm *tstruct;


  time = (time_t)(time64 / 1000000);
  if (time == 0)
    sprintf( str, "%19s", " " );
  else {
    tstruct = gmtime( &time ); 
    sprintf( str, "%d:%02d:%02d %02d:%02d:%02d", tstruct->tm_year + 1900,
	     tstruct->tm_mon + 1, tstruct->tm_mday, tstruct->tm_hour,
	     tstruct->tm_min, tstruct->tm_sec );
  }
} /* end FormatTime */



/***************************************************************************
 * function KDUWriteJP2RGB24
 *
 * Write a "Smart" JPEG2000 file that is compatible with ERDAS Imagine.
 * The input image must be a 24-bit RGB image with 8 bits of each color.
 * 'filename' is the output file name to save into, which must already be
 * a known, valid file and/or path name.  'sizeX' and 'sizeY' are the size
 * of the image in pixels, and there must be 3 bytes for each pixel in RGB
 * order.  'utm' is a RECT containing the upper-left and lower-right corners
 * of the image to the nearest meter.  'zone' is the UTM zone (1 or 2-digit
 * ASCII number and one letter).  'res' is the resolution of the image in
 * meters/pixel.  'time64' is a UTC time nugget which indicates the time
 * of the image.  If no time is appropriate for the image, use 0.
 *
 * Returns 1 on success, and 0 on failure.
 ***************************************************************************/

int KDUWriteJP2RGB24( char filename[], unsigned char *rgb,
			 int sizeX, int sizeY, RECT utm,
			 char zone[ZONE_LEN], double res, __int64 time64 )
{
  int numComponents = 3;
  unsigned short dummyData[4] = { 0xADDE, 0xEFBE, 0xADDE, 0xEFBE };
  jp2_target     jp2_out;
  jp2_family_tgt jp2_ultimate_tgt;
  siz_params  siz;
  kdu_params *siz_ref = &siz;
  kdu_codestream codestream;
  kdu_stripe_compressor  compressor;
  kdu_compressed_target *output = NULL;


  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  /* Try to open the jp2 output file.
   */
  output = &jp2_out;
  jp2_ultimate_tgt.open( filename );
  jp2_out.open( &jp2_ultimate_tgt );

  /* Construct codestream object
   */
  siz.set( Scomponents, 0, 0, numComponents );
  siz.set( Sdims, 0, 0, sizeY );
  siz.set( Sdims, 0, 1, sizeX );
  siz.set( Sprecision, 0, 0, 8 );  // Image samples have original bit-depth of 8
  siz.set( Ssigned, 0, 0, false ); // Image samples are originally unsigned
  siz_ref->finalize();

  codestream.create( &siz, output );
  codestream.access_siz()->parse_string( "Clayers=1" );
  codestream.access_siz()->parse_string( "Creversible=yes" );
  codestream.access_siz()->finalize_all();

  /* Write the JP2 header
   */
  if (jp2_ultimate_tgt.exists()) {
    jp2_dimensions dimensions;
    dimensions = jp2_out.access_dimensions();
    dimensions.init( codestream.access_siz() );
    jp2_colour colour = jp2_out.access_colour();
    colour.init( JP2_sRGB_SPACE );
    jp2_out.write_header();

    /* This is where the "smart" data box goes.
     * Create a GeoTIFF data blob and stuff it in the box.
     */
    kdu_uint32 boxType = jp2_uuid_4cc;
    jp2_output_box smartBox;
    smartBox.open( &jp2_ultimate_tgt, boxType );
    smartBox.write( geoTiffBox, TIFFBOXSIZE );
    smartBox.write( headerChars, HEADERCHARSIZE );
    smartBox.write( (unsigned char *)tags, sizeof(TIFFTagA) * NUMTIFFTAGS );
    geoKeyData[15] = GetUTMIndex( zone );
    smartBox.write( (unsigned char *)geoKeyData, sizeof(unsigned short) * GEOKEYSIZE);
    geoDoubles[0] = res;
    geoDoubles[1] = res;
    geoDoubles[6] = utm.left;
    geoDoubles[7] = utm.top;
    smartBox.write( (unsigned char *)geoDoubles, sizeof(double) * GEODOUBLESIZE );
    FormatTime( time64, date );
    smartBox.write( (unsigned char *)date, sizeof(unsigned char) * DATESIZE);
    smartBox.write( (unsigned char *)dummyData, sizeof(unsigned short) * 4);
    smartBox.close();

    /* Compress the image in memory in one shot.
     */
    jp2_out.open_codestream( true );
    int height[3] = { sizeY, sizeY, sizeY };
    compressor.start( codestream );
    // In theory, the image should go out in one hit, but loop anyway.
    while (compressor.push_stripe( rgb, height ));

    // Clean up.
    compressor.finish();
    codestream.destroy();
    output->close();
    if (jp2_ultimate_tgt.exists())
      jp2_ultimate_tgt.close();

    return 1;
  } else
    return 0;

} /* end KDUWriteJP2RGB24 */



/***************************************************************************
 * function KDUWriteJP2Mono8
 *
 * Write a "Smart" JPEG2000 file that is compatible with ERDAS Imagine.
 * The input image must be an 8-bit grayscale image.
 * 'filename' is the output file name to save into, which must already be
 * a known, valid file and/or path name.  'sizeX' and 'sizeY' are the size
 * of the image in pixels, and there must be 1 byte for each pixel.
 * 'utm' is a RECT containing the upper-left and lower-right corners of the
 * image to the nearest meter.  'zone' is the UTM zone (1 or 2-digit
 * ASCII number and one letter).  'res' is the resolution of the image in
 * meters/pixel.  'time64' is a UTC time nugget which indicates the time of
 * the image.  If no time is appropriate for the image, use 0.
 * 'targetSizeBytes' requests the target size of the data body (note that
 * this does NOT include the headers, so the file will be 20-40k larger than
 * 'targetSizeBytes').  If 0, no compression is used and the image is lossless.
 *
 * Returns 1 on success, and 0 on failure.
 ***************************************************************************/

int KDUWriteJP2Mono8( char filename[], unsigned char *data, int sizeX,
			 int sizeY, RECT utm, char zone[ZONE_LEN], double res,
			 __int64 time64, long targetSizeBytes )
{
  int numComponents = 1;
  unsigned short dummyData[4] = { 0xADDE, 0xEFBE, 0xADDE, 0xEFBE };
  jp2_target     jp2_out;
  jp2_family_tgt jp2_ultimate_tgt;
  siz_params  siz;
  kdu_params *siz_ref = &siz;
  kdu_codestream codestream;
  kdu_stripe_compressor  compressor;
  kdu_compressed_target *output = NULL;
  kdu_thread_env threadEnv;
  kdu_thread_env *threadEnvRef = NULL;


  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Set up multi-threaded environment.
  if (numCPUs > 1) {
    int nt = numCPUs;
    threadEnv.create();
    for (int ct = 1; ct < nt; ct++)
      if (!threadEnv.add_thread()) {
	nt = ct; // Unable to create all the threads requested
	PutLog( "KDUWriteJP2Mono8 - Tried to create %d threads, but created %d instead\n",
		numCPUs, nt );
      }
    threadEnvRef = &threadEnv;
  }

  /* Try to open the jp2 output file.
   */
  output = &jp2_out;
  jp2_ultimate_tgt.open( filename );
  jp2_out.open( &jp2_ultimate_tgt );

  /* Construct codestream object
   */
  siz.set( Scomponents, 0, 0, numComponents );
  siz.set( Sdims, 0, 0, sizeY );
  siz.set( Sdims, 0, 1, sizeX );
  siz.set( Sprecision, 0, 0, 8 );  // Image samples have original bit-depth of 8
  siz.set( Ssigned, 0, 0, false ); // Image samples are originally unsigned
  siz.set( Stiles, 0, 0, 512 );
  siz.set( Stiles, 0, 1, 512 );
  siz_ref->finalize();

  codestream.create( &siz, output );
  codestream.access_siz()->parse_string( "Clayers=1" );
  if(targetSizeBytes==0)
    codestream.access_siz()->parse_string( "Creversible=yes" );  // to be lossless, must be reversible

  kdu_params *cod = codestream.access_siz()->access_cluster(COD_params);
  cod->set(Corder, 0, 0, Corder_PCRL);
  cod->parse_string("Cprecincts={512,512},{256,256}");

  kdu_params *org = codestream.access_siz()->access_cluster(ORG_params);
  org->set(ORGgen_tlm, 0, 0, 1);
  codestream.access_siz()->finalize_all();

  /* Write the JP2 header
   */
  if (jp2_ultimate_tgt.exists()) {
    jp2_dimensions dimensions;
    dimensions = jp2_out.access_dimensions();
    dimensions.init( codestream.access_siz() );
    jp2_colour colour = jp2_out.access_colour();
    colour.init( JP2_sLUM_SPACE );
    jp2_out.write_header();

    /* This is where the "smart" data box goes.
     * Create a GeoTIFF data blob and stuff it in the box.
     */
    kdu_uint32 boxType = jp2_uuid_4cc;
    jp2_output_box smartBox;
    smartBox.open( &jp2_ultimate_tgt, boxType );
    smartBox.write( geoTiffBox, TIFFBOXSIZE );
    smartBox.write( headerChars, HEADERCHARSIZE );
    smartBox.write( (unsigned char *)tags, sizeof(TIFFTagA) * NUMTIFFTAGS );
    geoKeyData[15] = GetUTMIndex( zone );
    smartBox.write( (unsigned char *)geoKeyData, sizeof(unsigned short) * GEOKEYSIZE);
    geoDoubles[0] = res;
    geoDoubles[1] = res;
    geoDoubles[6] = utm.left;
    geoDoubles[7] = utm.top;
    smartBox.write( (unsigned char *)geoDoubles, sizeof(double) * GEODOUBLESIZE );
    FormatTime( time64, date );
    smartBox.write( (unsigned char *)date, sizeof(unsigned char) * DATESIZE);
    smartBox.write( (unsigned char *)dummyData, sizeof(unsigned short) * 4);
    smartBox.close();

    /* Compress the image in memory in one shot.
     */
    jp2_out.open_codestream( true );
    int height[3] = { sizeY, sizeY, sizeY };
    kdu_long layer_sizes[1] = { (kdu_long)targetSizeBytes };

    compressor.start( codestream, 1, layer_sizes, NULL, 0, false,
		      false, true, 0.0, numComponents, false, threadEnvRef,
		      NULL, doubleBufferSize );

    // In theory, the image should go out in one hit, but loop anyway.
    while (compressor.push_stripe( data, height ));

    // Clean up.
    compressor.finish();
    codestream.destroy();
    output->close();
    if (threadEnv.exists())
      threadEnv.destroy();
    if (jp2_ultimate_tgt.exists())
      jp2_ultimate_tgt.close();

    return 1;
  } else
    return 0;

} /* end KDUWriteJP2Mono8 */



/***************************************************************************
 * function KDUWriteJP2Mono8NoGeo
 *
 * The input image must be an 8-bit grayscale image.
 * 'filename' is the output file name to save into, which must already be
 * a known, valid file and/or path name.  'sizeX' and 'sizeY' are the size
 * of the image in pixels, and there must be 1 byte for each pixel.
 * 'targetSizeBytes' requests the target size of the data body (note that
 * this does NOT include the headers, so the file will be 20-40k larger than
 * 'targetSizeBytes').  If 0, no compression is used and the image is lossless.
 *
 * Returns 1 on success, and 0 on failure.
 ***************************************************************************/

int KDUWriteJP2Mono8NoGeo( char filename[], unsigned char *data,
			      int sizeX, int sizeY, long targetSizeBytes )
{
  int numComponents = 1;
  jp2_target     jp2_out;
  jp2_family_tgt jp2_ultimate_tgt;
  siz_params  siz;
  kdu_params *siz_ref = &siz;
  kdu_codestream codestream;
  kdu_stripe_compressor  compressor;
  kdu_compressed_target *output = NULL;
  kdu_thread_env threadEnv;
  kdu_thread_env *threadEnvRef = NULL;


  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Set up multi-threaded environment.
  if (numCPUs > 1) {
    int nt = numCPUs;
    threadEnv.create();
    for (int ct = 1; ct < nt; ct++)
      if (!threadEnv.add_thread()) {
	nt = ct; // Unable to create all the threads requested
	PutLog( "KDUWriteJP2Mono8 - Tried to create %d threads, but created %d instead\n",
		numCPUs, nt );
      }
    threadEnvRef = &threadEnv;
  }

  /* Try to open the jp2 output file.
   */
  output = &jp2_out;
  jp2_ultimate_tgt.open( filename );
  jp2_out.open( &jp2_ultimate_tgt );

  /* Construct codestream object
   */
  siz.set( Scomponents, 0, 0, numComponents );
  siz.set( Sdims, 0, 0, sizeY );
  siz.set( Sdims, 0, 1, sizeX );
  siz.set( Sprecision, 0, 0, 8 );  // Image samples have original bit-depth of 8
  siz.set( Ssigned, 0, 0, false ); // Image samples are originally unsigned
  siz.set( Stiles, 0, 0, 512 );
  siz.set( Stiles, 0, 1, 512 );
  siz_ref->finalize();

  codestream.create( &siz, output );
  codestream.access_siz()->parse_string( "Clayers=1" );
  codestream.access_siz()->parse_string( "Creversible=yes" );

  kdu_params *cod = codestream.access_siz()->access_cluster(COD_params);
  cod->set(Corder, 0, 0, Corder_PCRL);
  cod->parse_string("Cprecincts={512,512},{256,256},{128,128},{64,64},{32,32}");

  codestream.access_siz()->finalize_all();

  /* Write the JP2 header
   */
  if (jp2_ultimate_tgt.exists()) {
    jp2_dimensions dimensions;
    dimensions = jp2_out.access_dimensions();
    dimensions.init( codestream.access_siz() );
    jp2_colour colour = jp2_out.access_colour();
    colour.init( JP2_sLUM_SPACE );
    jp2_out.write_header();

    /* Compress the image in memory in one shot.
     */
    jp2_out.open_codestream( true );
    int height[3] = { sizeY, sizeY, sizeY };
    kdu_long *layer_sizes = new kdu_long[1];
    layer_sizes[0] = (kdu_long)targetSizeBytes;

    compressor.start( codestream, 1, layer_sizes, NULL, 0, false,
		      false, true, 0.0, numComponents, false, threadEnvRef,
		      NULL, doubleBufferSize );

    // In theory, the image should go out in one hit, but loop anyway.
    while (compressor.push_stripe( data, height ));

    // Clean up.
    compressor.finish();
    codestream.destroy();
    output->close();
    if (threadEnv.exists())
      threadEnv.destroy();
    if (jp2_ultimate_tgt.exists())
      jp2_ultimate_tgt.close();

    delete[] layer_sizes;
    return 1;
  } else
    return 0;

} /* end KDUWriteJP2Mono8 */



/***************************************************************************
 * function KDUWriteJP2Mono16
 *
 * Write a "Smart" JPEG2000 file that is compatible with ERDAS Imagine.
 * The input image must be an 10 to 16-bit grayscale image.
 * 'filename' is the output file name to save into, which must already be
 * a known, valid file and/or path name.  'sizeX' and 'sizeY' are the size
 * of the image in pixels, and there must be 2 bytes for each pixel.
 * 'utm' is a RECT containing the upper-left and lower-right corners of the
 * image to the nearest meter.  'zone' is the UTM zone (1 or 2-digit
 * ASCII number and one letter).  'res' is the resolution of the image in
 * meters/pixel.  'time64' is a UTC time nugget which indicates the time of
 * the image.  If no time is appropriate for the image, use 0.
 * 'targetSizeBytes' requests the target size of the data body (note that
 * this does NOT include the headers, so the file will be 20-40k larger than
 * 'targetSizeBytes').  If 0, no compression is used and the image is lossless.
 *
 * Returns 1 on success, and 0 on failure.
 ***************************************************************************/

int KDUWriteJP2Mono16( char filename[], short *gray, int sizeX, int sizeY,
			  RECT utm, char zone[ZONE_LEN], double res,
			  __int64 time64, long targetSizeBytes,
			  bool isSigned, int precision )
{
  int numComponents = 1;
  unsigned short dummyData[4] = { 0xADDE, 0xEFBE, 0xADDE, 0xEFBE };
  jp2_target     jp2_out;
  jp2_family_tgt jp2_ultimate_tgt;
  siz_params  siz;
  kdu_params *siz_ref = &siz;
  kdu_codestream codestream;
  kdu_stripe_compressor  compressor;
  kdu_compressed_target *output = NULL;
  kdu_thread_env threadEnv;
  kdu_thread_env *threadEnvRef = NULL;


  // Check inputs
  if (precision < 10 || precision > 16)
    return 0;

  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Set up multi-threaded environment.
  if (numCPUs > 1) {
    int nt = numCPUs;
    threadEnv.create();
    for (int ct = 1; ct < nt; ct++)
      if (!threadEnv.add_thread()) {
	nt = ct; // Unable to create all the threads requested
	PutLog( "KDUWriteJP2Mono16 - Tried to create %d threads, but created %d instead\n",
		numCPUs, nt );
      }
    threadEnvRef = &threadEnv;
  }

  /* Try to open the jp2 output file.
   */
  output = &jp2_out;
  jp2_ultimate_tgt.open( filename );
  jp2_out.open( &jp2_ultimate_tgt );

  /* Construct codestream object
   */
  siz.set( Scomponents, 0, 0, numComponents );
  siz.set( Sdims, 0, 0, sizeY );
  siz.set( Sdims, 0, 1, sizeX );
  siz.set( Sprecision, 0, 0, precision );
  siz.set( Ssigned, 0, 0, isSigned );
  siz.set( Stiles, 0, 0, 512 );
  siz.set( Stiles, 0, 1, 512 );
  siz_ref->finalize();

  codestream.create( &siz, output );
  codestream.access_siz()->parse_string( "Clayers=1" );
  codestream.access_siz()->parse_string( "Creversible=no" );

  kdu_params *cod = codestream.access_siz()->access_cluster(COD_params);
  cod->set(Corder, 0, 0, Corder_PCRL);
  cod->parse_string("Cprecincts={512,512},{256,256}");

  codestream.access_siz()->finalize_all();

  /* Write the JP2 header
   */
  if (jp2_ultimate_tgt.exists()) {
    jp2_dimensions dimensions;
    dimensions = jp2_out.access_dimensions();
    dimensions.init( codestream.access_siz() );
    jp2_colour colour = jp2_out.access_colour();
    colour.init( JP2_sLUM_SPACE );
    jp2_out.write_header();

    /* This is where the "smart" data box goes.
     * Create a GeoTIFF data blob and stuff it in the box.
     */
    kdu_uint32 boxType = jp2_uuid_4cc;
    jp2_output_box smartBox;
    smartBox.open( &jp2_ultimate_tgt, boxType );
    smartBox.write( geoTiffBox, TIFFBOXSIZE );
    smartBox.write( headerChars, HEADERCHARSIZE );
    smartBox.write( (unsigned char *)tags, sizeof(TIFFTagA) * NUMTIFFTAGS );
    geoKeyData[15] = GetUTMIndex( zone );
    smartBox.write( (unsigned char *)geoKeyData, sizeof(unsigned short) * GEOKEYSIZE);
    geoDoubles[0] = res;
    geoDoubles[1] = res;
    geoDoubles[6] = utm.left;
    geoDoubles[7] = utm.top;
    smartBox.write( (unsigned char *)geoDoubles, sizeof(double) * GEODOUBLESIZE );
    FormatTime( time64, date );
    smartBox.write( (unsigned char *)date, sizeof(unsigned char) * DATESIZE);
    smartBox.write( (unsigned char *)dummyData, sizeof(unsigned short) * 4);
    smartBox.close();

    /* Compress the image in memory using the stripe compressor.
     */
    int precisions[1] = {precision};
    int stripe_heights[1];
    int max_stripe_heights[1];
    bool signs[1] = { isSigned };
    short *gray1 = NULL;
    short *bufptr;
    jp2_out.open_codestream( true );
    kdu_long layer_sizes[1] = { (kdu_long)targetSizeBytes };

    compressor.start( codestream, 1, layer_sizes, NULL, 0, false,
		      false, true, 0.0, numComponents, false, threadEnvRef,
		      NULL, doubleBufferSize );
    compressor.get_recommended_stripe_heights( 8, 1024, stripe_heights, max_stripe_heights );

    // Convert data into signed representation
    // The internal data representation *must* be signed, even if the
    // data values themselves are unsigned, or Kakadu will do the wrong
    // thing and wash out the data.
    if (!isSigned) {
      gray1 = (short *)malloc( sizeX * sizeY * sizeof(short) );
      int offset = (1 << precision ) >> 1;
      for (int i = 0; i < sizeX * sizeY ;i++)
	gray1[i] = gray[i] - offset;
      bufptr = gray1;
    } else
      bufptr = gray;

    while (compressor.push_stripe( bufptr, stripe_heights, NULL, NULL, NULL, precisions, signs )) {
      compressor.get_recommended_stripe_heights( 8, 1024, stripe_heights, NULL );
      bufptr += stripe_heights[0] * sizeX;
    }

    // Clean up.
    compressor.finish();
    codestream.destroy();
    output->close();
    if (gray1)
      free( gray1 );
    if (threadEnv.exists())
      threadEnv.destroy();
    if (jp2_ultimate_tgt.exists())
      jp2_ultimate_tgt.close();

    return 1;
  } else
    return 0;

} /* end KDUWriteJP2Mono16 */



/***************************************************************************
 * function KDUWriteJP2Mono16NoGeo
 *
 * Compress data into a regular JPEG2000 file without Geographical info.
 * 'targetSizeBytes' requests the target size of the data body.  If 0, no
 * compression is used and the image is lossless.
 *
 * Returns 1 on success, and 0 on failure.
 ***************************************************************************/

int KDUWriteJP2Mono16NoGeo( char filename[], short *gray, int sizeX, int sizeY,
			       long targetSizeBytes, bool isSigned, int precision )
{
  int numComponents = 1;  // Grayscale
  jp2_target     jp2_out;
  jp2_family_tgt jp2_ultimate_tgt;
  siz_params  siz;
  kdu_params *siz_ref = &siz;
  kdu_codestream codestream;
  kdu_stripe_compressor  compressor;
  kdu_compressed_target *output = NULL;
  kdu_thread_env threadEnv;
  kdu_thread_env *threadEnvRef = NULL;


  // Check inputs
  if (precision < 10 || precision > 16)
    return 0;

  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Set up multi-threaded environment.
  if (numCPUs > 1) {
    int nt = numCPUs;
    threadEnv.create();
    for (int ct = 1; ct < nt; ct++)
      if (!threadEnv.add_thread()) {
	nt = ct; // Unable to create all the threads requested
	PutLog( "KDUWriteJP2Mono16 - Tried to create %d threads, but created %d instead\n",
		numCPUs, nt );
      }
    threadEnvRef = &threadEnv;
  }

  /* Try to open the jp2 output file.
   */
  output = &jp2_out;
  jp2_ultimate_tgt.open( filename );
  jp2_out.open( &jp2_ultimate_tgt );

  /* Construct codestream object
   */
  siz.set( Scomponents, 0, 0, numComponents );
  siz.set( Sdims, 0, 0, sizeY );
  siz.set( Sdims, 0, 1, sizeX );
  siz.set( Sprecision, 0, 0, precision );
  siz.set( Ssigned, 0, 0, isSigned );
  siz.set( Stiles, 0, 0, 512 );
  siz.set( Stiles, 0, 1, 512 );
  siz_ref->finalize();

  codestream.create( &siz, output );
  codestream.access_siz()->parse_string( "Clayers=1" );
  codestream.access_siz()->parse_string( "Creversible=no" );

  kdu_params *cod = codestream.access_siz()->access_cluster(COD_params);
  cod->set(Corder, 0, 0, Corder_PCRL);
  cod->parse_string("Cprecincts={512,512},{256,256}");

  codestream.access_siz()->finalize_all();

  /* Write the JP2 header
   */
  if (jp2_ultimate_tgt.exists()) {
    jp2_dimensions dimensions;
    dimensions = jp2_out.access_dimensions();
    dimensions.init( codestream.access_siz() );
    jp2_colour colour = jp2_out.access_colour();
    colour.init( JP2_sLUM_SPACE );
    jp2_out.write_header();

    /* Compress the image in memory using the stripe compressor.
     */
    int precisions[1] = {precision};
    int stripe_heights[1];
    int max_stripe_heights[1];
    bool signs[1] = { isSigned };
    short *gray1 = NULL;
    short *bufptr;
    jp2_out.open_codestream( true );
    kdu_long layer_sizes[1] = { (kdu_long)targetSizeBytes };

    compressor.start( codestream, 1, layer_sizes, NULL, 0, false,
		      false, true, 0.0, numComponents, false, threadEnvRef,
		      NULL, doubleBufferSize );
    compressor.get_recommended_stripe_heights( 8, 1024, stripe_heights, max_stripe_heights );

    // Convert data into signed representation.
    // The internal data representation *must* be signed, even if the
    // data values themselves are unsigned, or Kakadu will do the wrong
    // thing and wash out the data.
    //if (!isSigned) {
    //  gray1 = (short *)malloc( sizeX * sizeY * sizeof(short) );
    //  int offset = (1 << precision ) >> 1;
    //  for (int i = 0; i < sizeX * sizeY ;i++)
	//gray1[i] = gray[i] - offset;
    //  bufptr = gray1;
    //} else
      bufptr = gray;

    while (compressor.push_stripe( bufptr, stripe_heights, NULL, NULL, NULL, precisions, signs )) {
      compressor.get_recommended_stripe_heights( 8, 1024, stripe_heights, NULL );
      bufptr += stripe_heights[0] * sizeX;
    }

    // Clean up.
    compressor.finish();
    codestream.destroy();
    output->close();
    if (gray1)
      free( gray1 );
    if (threadEnv.exists())
      threadEnv.destroy();
    if (jp2_ultimate_tgt.exists())
      jp2_ultimate_tgt.close();

    return 1;
  } else
    return 0;

} /* end KDUWriteJP2Mono16NoGeo */



/***************************************************************************
 * function GetUTMCode
 *
 * Compose a proper UTM zone string from the supplied TIFF zone number.
 ***************************************************************************/

static void GetUTMCode( int code, long northing, char zone[ZONE_LEN] )
{
  int  idx;
  int  zNum;
  char northCodes[] = "NPQRSTUVWXX";
  char southCodes[] = "CDEFGHJKLM";


  if (code >= UTM_NORTH_START && code < (UTM_NORTH_START + 60)) {
    // Northern easting zone
    zNum = code - UTM_NORTH_START + 1;
    // Northern northing index
    idx = northing / 1000000;
    sprintf( zone, "%d%c", zNum, northCodes[idx] );
  } else if (code >= UTM_SOUTH_START && code < (UTM_SOUTH_START + 60)) {
    // Southern easting zone
    zNum = code - UTM_SOUTH_START + 1;
    // Southern northing index
    idx = northing / 1000000;
    sprintf( zone, "%d%c", zNum, southCodes[idx] );
  }
  PutLog( "GetUTMCode - code: %d, northing: %ld, zone: %s\n", code, northing, zone );

} /* end GetUTMCode */



/***************************************************************************
 * function UTCTime
 *
 * Take a TIFF-legal time string and create a UTC time nugget.
 * If a blank string is input, or on error, 0 is returned in time64.
 ***************************************************************************/

static void UTCTime( char date[DATESIZE], __int64 &time64 )
{
  time_t  now;
  __int64 nugget;
  struct tm *nowS, thenS;


  if (strlen(date) == 0) {
    time64 = 0;
    return;
  }
  now  = time( NULL );
  nowS = localtime( &now );
  sscanf( date, "%d:%d:%d %d:%d:%d", &thenS.tm_year,
	  &thenS.tm_mon, &thenS.tm_mday, &thenS.tm_hour,
	  &thenS.tm_min, &thenS.tm_sec );
  thenS.tm_isdst = nowS->tm_isdst;
  thenS.tm_year -= 1900;
  thenS.tm_mon  -= 1;
  thenS.tm_isdst = nowS->tm_isdst;
  nugget = mktime( &thenS );
  if (nugget == -1)
    time64 = 0;
  else
    time64 = nugget * 1000000;

} /* end UTCTime */



/******************************************************************************
 * CLASS                     hsdMemoryTarget
 *
 * This is a derived class from kdu_compressed_target, so that we can use all
 * the usual KDU stuff on a memory buffer.
 ******************************************************************************/

class hsdMemoryTarget : public kdu_compressed_target {
public:
  hsdMemoryTarget( unsigned char *data, unsigned int size ) {
    bufPtr = data; bufSize = size; offset = 0; dataSize = 0; rewrite_held_value = -1; }
  ~hsdMemoryTarget() { bufPtr = NULL; bufSize = 0; offset = 0; }
  int get_capabilities() { return KDU_SOURCE_CAP_SEQUENTIAL | KDU_SOURCE_CAP_SEEKABLE; }
  bool write( const kdu_byte *buf, int numBytes ) {
    if (offset + numBytes > bufSize) numBytes = (int)(bufSize - offset);
    if (numBytes <= 0) return false;
    memcpy( bufPtr + offset, buf, numBytes );
    offset += numBytes;
    if (offset > dataSize) dataSize = offset;
    return true;
  }
  bool seek( kdu_long offst ) { offset = offst; return true; }
  int  getSize() { return (int)dataSize; }
  kdu_long get_pos() { return offset; }
  bool start_rewrite( kdu_long backtrack) {
    rewrite_held_value = offset;
    offset -= backtrack;
    return true;
  }
  bool end_rewrite( void ) {
    if(rewrite_held_value >= 0) {
      offset = rewrite_held_value;
      rewrite_held_value = -1;
      return true;
    } else {
      return false;
    }
  }
private:
  unsigned char *bufPtr;
  kdu_long bufSize;
  kdu_long dataSize;
  kdu_long offset;
  kdu_long rewrite_held_value;
};


class hsdMemorySource : public kdu_compressed_source {
public:
  hsdMemorySource( const unsigned char *data, unsigned int size ) {
    bufPtr = data; bufSize = size; offset = 0; dataSize = 0; }
  ~hsdMemorySource() { bufPtr = NULL; bufSize = 0; offset = 0; }
  int get_capabilities() { return KDU_SOURCE_CAP_SEQUENTIAL | KDU_SOURCE_CAP_SEEKABLE; }
  int read( kdu_byte *buf, int numBytes ) {
    if (offset + numBytes > bufSize) numBytes = (int)(bufSize - offset);
    if (numBytes <= 0) return 0;
    memcpy( buf, bufPtr + offset, numBytes );
    offset += numBytes;
    if (offset > dataSize) dataSize = offset;
    return numBytes;
  }
  bool seek( kdu_long offst ) { offset = offst; return true; }
  int  getSize() { return (int)dataSize; }
  kdu_long get_pos() { return offset; }

private:
  const unsigned char *bufPtr;
  kdu_long bufSize;
  kdu_long dataSize;
  kdu_long offset;
};


/* Here's a source for reading from a regular file, where the
   j2k stream does not start at the beginning. */
class kdu_embedded_file_source : public kdu_compressed_source {
public:
	kdu_embedded_file_source(FILE *fid, kdu_long codestream_offset)
    : m_fid(fid), m_codestream_offset(codestream_offset), m_total_bytes_read(0)
	{ 
        seek(0);
    }

    ~kdu_embedded_file_source(void)
    {
        //printf("total read: %i\n", m_total_bytes_read);
    }

    virtual int get_capabilities() { return KDU_SOURCE_CAP_SEQUENTIAL | KDU_SOURCE_CAP_SEEKABLE; }

	virtual int read(kdu_byte *buf_d, int num_bytes)
	{
        m_total_bytes_read += num_bytes;
        return(fread(buf_d, 1, num_bytes, m_fid));
    }

    virtual bool seek(kdu_long offset)
    { 
        kdu_long temp = offset + m_codestream_offset;

#ifdef _MSC_VER
        int ret = fsetpos(m_fid, &temp);
#else
        fseeko64(m_fid, (int64_t)temp, SEEK_SET);
#endif
        return true; 
    }

    virtual kdu_long get_pos() 
    { 
        return (kdu_long) ftell(m_fid) - m_codestream_offset;
    }

private:
    FILE *m_fid;
    kdu_long m_codestream_offset;
    int m_total_bytes_read;
};

/***************************************************************************
 * function KDUCompressJP2RGB16
 *
 * Compress data into a regular JPEG2000 file with no geospacial information.
 * The input image must be an 10 to 16-bit image.  'sizeX' and 'sizeY'
 * are the size of the image in pixels, and there must be 2 bytes for each
 * pixel.  'targetSizeBytes' requests the target size of the data body.
 * If 0 was removed
 *
 * Returns a pointer to the compressed block on success, and NULL on failure.
 * Also on success, actualSizeBytes will be set to the block size, 0 on failure.
 ***************************************************************************/

unsigned char *KDUCompressJP2RGB16( short *rgb, unsigned char *output, int sizeX, int sizeY,
                                     long targetSizeBytes, int *actualSizeBytes,
                                     bool isSigned, int precision, int stride )
{
  int numChannels=3;
  if (stride==-1) {
    stride = sizeX*numChannels;
  }
  int dataSize = sizeof(short) * sizeX * sizeY * numChannels;
  unsigned char *tgt;
  hsdMemoryTarget *memTgt;
  jp2_target     jp2_out;
  jp2_family_tgt jp2_ultimate_tgt;
  siz_params  siz;
  kdu_params *siz_ref = &siz;
  kdu_codestream codestream;
  kdu_stripe_compressor compressor;
  kdu_thread_env threadEnv;
  kdu_thread_env *threadEnvRef = NULL;


  // Check inputs
  if (precision < 10 || precision > 16)
    return 0;

  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Set up multi-threaded environment.
  if (numCPUs > 1) {
      int nt = numCPUs;
      threadEnv.create();
      for (int ct = 1; ct < nt; ct++)
          if (!threadEnv.add_thread()) {
              nt = ct; // Unable to create all the threads requested
              PutLog( "KDUWriteJP2Mono8 - Tried to create %d threads, but created %d instead\n",
                  numCPUs, nt );
          }
          threadEnvRef = &threadEnv;
  }

  // Allocate
  if(output)
	  tgt = output;
  else{
	  tgt = (unsigned char *)malloc( dataSize );
	  if (tgt == NULL) {
	    *actualSizeBytes = 0;
	  return NULL;
	  }
  }

  /* Setup buffer target
   */
  memTgt = new hsdMemoryTarget( tgt, dataSize );
  jp2_ultimate_tgt.open( memTgt );
  jp2_out.open( &jp2_ultimate_tgt );

  /* Construct codestream object
   */
  siz.set( Scomponents, 0, 0, numChannels ); // Color
  siz.set( Sdims, 0, 0, sizeY );
  siz.set( Sdims, 0, 1, sizeX );
  siz.set( Sprecision, 0, 0, precision );
  siz.set( Ssigned, 0, 0, isSigned );
  siz.set(Stiles,0,0,512);
  siz.set(Stiles,0,1,512);
  siz_ref->finalize();

  codestream.create( &siz, memTgt );
  codestream.access_siz()->parse_string( "Clayers=1" );
  codestream.access_siz()->parse_string( "Creversible=no" );
  //if (targetSizeBytes != 0)
  //  codestream.set_max_bytes( targetSizeBytes );

  kdu_params *org = codestream.access_siz()->access_cluster(ORG_params);
  org->set(ORGgen_tlm, 0, 0, 1);
  org->set(ORGgen_plt, 0, 0, 1);

  kdu_params *cod = codestream.access_siz()->access_cluster(COD_params);
  cod->set(Corder, 0, 0, Corder_PCRL);

  cod->parse_string("Cprecincts={512,512},{256,256}");

  //kdu_params *poc = codestream.access_siz()->access_cluster(POC_params);
  //poc->parse_string("Porder={0,0,1,2,1,LRCP}");
  //poc->parse_string("Porder={2,0,1,6,1,LRCP}");

  kdu_params *qcd = codestream.access_siz()->access_cluster(QCD_params);
  qcd->set(Qstep, 0, 0, 1.0/8192.0);			

  codestream.access_siz()->finalize_all();

  /* Write the JP2 header
   */
  if (jp2_ultimate_tgt.exists()) {
    int precisions[3];
	bool is_signed[3];
    short *stripe_bufs[3];
    jp2_dimensions dimensions;
    dimensions = jp2_out.access_dimensions();
    dimensions.init( codestream.access_siz() );
    jp2_colour colour = jp2_out.access_colour();
    colour.init( JP2_sRGB_SPACE );
    jp2_out.write_header();

    /* Compress the image in memory in one shot.
     */
    jp2_out.open_codestream( true );
    int height[3] = { sizeY, sizeY, sizeY };
	kdu_long layer_sizes[1];
    layer_sizes[0] = (kdu_long)targetSizeBytes;		
    //compressor.start( codestream, 1, layer_sizes );

    compressor.start( codestream, 1, layer_sizes, NULL, 0, false,
		      false, true, 0.0, 0, false, threadEnvRef,
		      NULL, doubleBufferSize );

    //compressor.start( codestream );
    // In theory, the image should go out in one hit, but loop anyway.
    /*
     * JVB - SECRET SAUCE: Must use *stripe_bufs[], must use precisions[],
     *       and data coming in MUST be signed.
     */
	unsigned short *redchannel=(unsigned short *)calloc(sizeX*sizeY,sizeof(short));
	unsigned short *bluechannel=(unsigned short *)calloc(sizeX*sizeY,sizeof(short));
	unsigned short *greenchannel=(unsigned short *)calloc(sizeX*sizeY,sizeof(short));
	for(int y=0; y!=sizeY; y++)
		for(int x=0; x!=sizeX; x++){
			*(redchannel+y*sizeX+x)=*(rgb+y*stride+(x*3+0));
			*(greenchannel+y*sizeX+x)=*(rgb+y*stride+(x*3+1));
			*(bluechannel+y*sizeX+x)=*(rgb+y*stride+(x*3+2));
		}

    stripe_bufs[0] = (short *)redchannel; stripe_bufs[1]=(short *)greenchannel; stripe_bufs[2]=(short *)bluechannel;
    precisions[0] = precision; precisions[1] = precision; precisions[2] = precision;
	is_signed[0] = isSigned; 	is_signed[1] = isSigned; 	is_signed[2] = isSigned;
	int row_gaps[3] = {sizeX, sizeX, sizeX};

    while (compressor.push_stripe( stripe_bufs, height, NULL, row_gaps, precisions, is_signed ));

    // Clean up.
    compressor.finish();
    codestream.destroy();
    if (jp2_ultimate_tgt.exists())
      jp2_ultimate_tgt.close();
    if (threadEnv.exists())
      threadEnv.destroy();

    *actualSizeBytes = memTgt->getSize();
    delete memTgt;
	delete redchannel;
	delete bluechannel;
	delete greenchannel;

    return tgt;
  } else
    return NULL;

} /* end KDUCompressJP2RGB16 */

/***************************************************************************
 * function KDUCompressJP2Mono8
 *
 * Compress data into a JPEG2000-format memory buffer with GeoTIFF-style
 * geospacial information.  The input image in 'inBuf' must be an 8-bit
 * image.  If outBuf is non-NULL, it is assumed to be a buffer large enough
 * to hold the uncompressed image, otherwise a buffer is allocated that must
 * be deallocated by the calling application.  'sizeX' and 'sizeY' are the
 * size of the image in pixels.  'targetSizeBytes' requests the target size
 * of the data body - if 0, no compression is used and the image is lossless.
 * 'utm', 'zone', 'res', and 'time64' are used to write the geospacial information.
 *
 * Returns a pointer to the compressed block on success, and NULL on failure.
 * Also on success, actualSizeBytes will be set to the block size, 0 on failure.
 ***************************************************************************/

unsigned char *KDUCompressJP2Mono8( unsigned char *inBuf, unsigned char *outBuf, int sizeX, int sizeY,
				       RECT utm, char zone[ZONE_LEN], double res, __int64 time64,
				       long targetSizeBytes, int *actualSizeBytes )
{
  int numComponents = 1;
  int dataSize = sizeof(char) * sizeX * sizeY;
  unsigned char *tgt, *tmp;
  unsigned short dummyData[4] = { 0xADDE, 0xEFBE, 0xADDE, 0xEFBE };
  hsdMemoryTarget *memTgt;
  jp2_target     jp2_out;
  jp2_family_tgt jp2_ultimate_tgt;
  siz_params  siz;
  kdu_params *siz_ref = &siz;
  kdu_codestream codestream;
  kdu_stripe_compressor  compressor;
  kdu_compressed_target *output = NULL;


  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Allocate
  if (outBuf != NULL)
    tgt = outBuf;
  else {
    tgt = (unsigned char *)malloc( dataSize );
    if (tgt == NULL) {
      *actualSizeBytes = 0;
      return NULL;
    }
  }

  /* Setup buffer target
   */
  memTgt = new hsdMemoryTarget( tgt, dataSize );
  output = &jp2_out;
  jp2_ultimate_tgt.open( memTgt );
  jp2_out.open( &jp2_ultimate_tgt );

  /* Construct codestream object
   */
  siz.set( Scomponents, 0, 0, numComponents ); // Greyscale
  siz.set( Sdims, 0, 0, sizeY );
  siz.set( Sdims, 0, 1, sizeX );
  siz.set( Sprecision, 0, 0, 8 );
  siz.set( Ssigned, 0, 0, false );
  siz.set( Stiles, 0, 0, 512 );
  siz.set( Stiles, 0, 1, 512 );
  siz_ref->finalize();

  codestream.create( &siz, output );
  codestream.access_siz()->parse_string( "Clayers=1" );
  codestream.access_siz()->parse_string( "Creversible=yes" );

  kdu_params *cod = codestream.access_siz()->access_cluster(COD_params);
  cod->set(Corder, 0, 0, Corder_PCRL);
  cod->parse_string( "Cprecincts={512,512},{256,256}" );

  kdu_params *org = codestream.access_siz()->access_cluster(ORG_params);
  org->set(ORGgen_tlm, 0, 0, 1);
  codestream.access_siz()->finalize_all();

  /* Write the JP2 header
   */
  if (jp2_ultimate_tgt.exists()) {

    // Write the standard header before we dive into the UUID box.
    jp2_dimensions dimensions;
    dimensions = jp2_out.access_dimensions();
    dimensions.init( codestream.access_siz() );
    jp2_colour colour = jp2_out.access_colour();
    colour.init( JP2_sLUM_SPACE );
    jp2_out.write_header();

    /* This is where the "smart" data box goes.
     * Create a GeoTIFF data blob and stuff it in the box.
     */
    kdu_uint32 boxType = jp2_uuid_4cc;
    jp2_output_box smartBox;
    smartBox.open( &jp2_ultimate_tgt, boxType );
    smartBox.write( geoTiffBox, TIFFBOXSIZE );
    smartBox.write( headerChars, HEADERCHARSIZE );
    smartBox.write( (unsigned char *)tags, sizeof(TIFFTagA) * NUMTIFFTAGS );
    geoKeyData[15] = GetUTMIndex( zone );
    smartBox.write( (unsigned char *)geoKeyData, sizeof(unsigned short) * GEOKEYSIZE);
    geoDoubles[0] = res;
    geoDoubles[1] = res;
    geoDoubles[6] = utm.top;
    geoDoubles[7] = utm.left;
    smartBox.write( (unsigned char *)geoDoubles, sizeof(double) * GEODOUBLESIZE );
    FormatTime( time64, date );
    smartBox.write( (unsigned char *)date, sizeof(unsigned char) * DATESIZE);
    smartBox.write( (unsigned char *)dummyData, sizeof(unsigned short) * 4);
    smartBox.close();

    /* Compress the image using KDU-recommended parameters in a
     * single-threaded environment.
     */
    int precisions[1] = { 8 };
    int stripe_heights[1];
    int max_stripe_heights[1];
    int preferred_min_stripe_height = 8;
    int absolute_max_stripe_height = 1024;
    kdu_long layer_sizes[1] = { (kdu_long)targetSizeBytes };

    jp2_out.open_codestream( true );

    compressor.start( codestream, 1, layer_sizes );
    compressor.get_recommended_stripe_heights( preferred_min_stripe_height,
					       absolute_max_stripe_height,
					       stripe_heights, max_stripe_heights );
    tmp = inBuf;
    while (compressor.push_stripe( tmp, stripe_heights, NULL, NULL, precisions )) {
      tmp += stripe_heights[0] * sizeX;
      compressor.get_recommended_stripe_heights( preferred_min_stripe_height,
                                                 absolute_max_stripe_height,
                                                 stripe_heights, NULL );
    } 

    // Clean up.
    compressor.finish();
    codestream.destroy();
    if (jp2_ultimate_tgt.exists())
      jp2_ultimate_tgt.close();
    output->close();

    *actualSizeBytes = memTgt->getSize();
    delete memTgt;

    return tgt;
  }

  // Failure
  codestream.destroy();
  if (jp2_ultimate_tgt.exists())
    jp2_ultimate_tgt.close();
  output->close();
  delete memTgt;

  *actualSizeBytes = 0;
  return NULL;

} /* end KDUCompressJP2Mono8 */



/***************************************************************************
 * function KDUCompressJP2Mono16
 *
 * Compress data into a regular JPEG2000 file with no geospacial information.
 * The input image must be an 10 to 16-bit image.  'sizeX' and 'sizeY'
 * are the size of the image in pixels, and there must be 2 bytes for each
 * pixel.  'targetSizeBytes' requests the target size of the data body.
 * If 0, no compression is used and the image is lossless.
 *
 * Returns a pointer to the compressed block on success, and NULL on failure.
 * Also on success, actualSizeBytes will be set to the block size, 0 on failure.
 ***************************************************************************/

unsigned char *KDUCompressJP2Mono16( short *gray, unsigned char *output, int sizeX, int sizeY,
                                     long targetSizeBytes, int *actualSizeBytes,
                                     bool isSigned, int precision, int stride )
{
#if 0
  int numComponents = 1;  // Grayscale
  unsigned char *tgt;
  hsdMemoryTarget *memTgt;
  jp2_target     jp2_out;
  jp2_family_tgt jp2_ultimate_tgt;
  siz_params  siz;
  kdu_params *siz_ref = &siz;
  kdu_codestream codestream;
  kdu_stripe_compressor compressor;
  kdu_thread_env threadEnv;
  kdu_thread_env *threadEnvRef = NULL;
  int dataSize = sizeof(short) * sizeX * sizeY;


  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Allocate
  if(output)
      tgt = output;
  else{
      tgt = (unsigned char *)malloc( dataSize );
      if (tgt == NULL) {
        *actualSizeBytes = 0;
        return NULL;
      }
  }

  // Set up multi-threaded environment.
  if (numCPUs > 1) {
    int nt = numCPUs;
    threadEnv.create();
    for (int ct = 1; ct < nt; ct++)
      if (!threadEnv.add_thread()) {
	nt = ct; // Unable to create all the threads requested
	PutLog( "KDUCompressJP2Mono16 - Tried to create %d threads, but created %d instead\n",
		numCPUs, nt );
      }
    threadEnvRef = &threadEnv;
  }

  /* Setup buffer target
   */
  memTgt = new hsdMemoryTarget( tgt, dataSize );
  jp2_ultimate_tgt.open( memTgt );
  jp2_out.open( &jp2_ultimate_tgt );

  /* Construct codestream object
   */
  siz.set( Scomponents, 0, 0, numComponents );
  siz.set( Sdims, 0, 0, sizeY );
  siz.set( Sdims, 0, 1, sizeX );
  siz.set( Sprecision, 0, 0, precision );
  siz.set( Ssigned, 0, 0, isSigned );
  siz_ref->finalize();

  codestream.create( &siz, memTgt );
  codestream.access_siz()->parse_string( "Clayers=1" );
  codestream.access_siz()->parse_string( "Creversible=yes" );
  codestream.access_siz()->finalize_all();

  /* Write the JP2 header
   */
  if (jp2_ultimate_tgt.exists()) {
    jp2_dimensions dimensions;
    dimensions = jp2_out.access_dimensions();
    dimensions.init( codestream.access_siz() );
    jp2_colour colour = jp2_out.access_colour();
    colour.init( JP2_sLUM_SPACE );
    jp2_out.write_header();

    /* Compress the image in memory using the stripe compressor.
     */
    int precisions[1] = {precision};
    int stripe_heights[1];
    int max_stripe_heights[1];
    bool signs[1] = { isSigned };
    short *gray1 = NULL;
    short *bufptr;
    jp2_out.open_codestream( true );
    kdu_long layer_sizes[1] = { (kdu_long)targetSizeBytes };

    compressor.start( codestream, 1, layer_sizes, NULL, 0, false,
		      false, true, 0.0, numComponents, false, threadEnvRef,
		      NULL, doubleBufferSize );
    compressor.get_recommended_stripe_heights( 8, 1024, stripe_heights, max_stripe_heights );

    // Convert data into signed representation.
    // The internal data representation *must* be signed, even if the
    // data values themselves are unsigned, or Kakadu will do the wrong
    // thing and wash out the data.
    if (!isSigned) {
      gray1 = (short *)malloc( sizeX * sizeY * sizeof(short) );
      int offset = (1 << precision ) >> 1;
      for (int i = 0; i < sizeX * sizeY ;i++)
	gray1[i] = gray[i] - offset;
      bufptr = gray1;
    } else
      bufptr = gray;

    while (compressor.push_stripe( bufptr, stripe_heights, NULL, NULL, NULL, precisions, signs )) {
      compressor.get_recommended_stripe_heights( 8, 1024, stripe_heights, NULL );
      bufptr += stripe_heights[0] * sizeX;
    }

    // Clean up.
    compressor.finish();
    codestream.destroy();
    if (gray1)
      free( gray1 );
    if (threadEnv.exists())
      threadEnv.destroy();
    if (jp2_ultimate_tgt.exists())
      jp2_ultimate_tgt.close();

    *actualSizeBytes = memTgt->getSize();
    delete memTgt;

    return tgt;
  } else
    return NULL;
#endif

  if (stride==-1) {
    stride = sizeX;
  }
  int dataSize = sizeof(short) * sizeX * sizeY;
  unsigned char *tgt;
  hsdMemoryTarget *memTgt;
  jp2_target     jp2_out;
  jp2_family_tgt jp2_ultimate_tgt;
  siz_params  siz;
  kdu_params *siz_ref = &siz;
  kdu_codestream codestream;
  kdu_stripe_compressor compressor;
  kdu_thread_env threadEnv;
  kdu_thread_env *threadEnvRef = NULL;


  // Check inputs
  if (precision < 10 || precision > 16)
    return 0;

  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Set up multi-threaded environment.
  if (numCPUs > 1) {
      int nt = numCPUs;
      threadEnv.create();
      for (int ct = 1; ct < nt; ct++)
          if (!threadEnv.add_thread()) {
              nt = ct; // Unable to create all the threads requested
              PutLog( "KDUWriteJP2Mono8 - Tried to create %d threads, but created %d instead\n",
                  numCPUs, nt );
          }
          threadEnvRef = &threadEnv;
  }

  // Allocate
  if(output)
	  tgt = output;
  else{
	  tgt = (unsigned char *)malloc( dataSize );
	  if (tgt == NULL) {
	    *actualSizeBytes = 0;
	  return NULL;
	  }
  }

  /* Setup buffer target
   */
  memTgt = new hsdMemoryTarget( tgt, dataSize );
  jp2_ultimate_tgt.open( memTgt );
  jp2_out.open( &jp2_ultimate_tgt );

  /* Construct codestream object
   */
  siz.set( Scomponents, 0, 0, 1 ); // Greyscale
  siz.set( Sdims, 0, 0, sizeY );
  siz.set( Sdims, 0, 1, sizeX );
  siz.set( Sprecision, 0, 0, precision );
  siz.set( Ssigned, 0, 0, isSigned );
  siz.set(Stiles,0,0,512);
  siz.set(Stiles,0,1,512);
  siz_ref->finalize();

  codestream.create( &siz, memTgt );
  codestream.access_siz()->parse_string( "Clayers=1" );
  codestream.access_siz()->parse_string( "Creversible=no" );
  //if (targetSizeBytes != 0)
  //  codestream.set_max_bytes( targetSizeBytes );

  kdu_params *cod = codestream.access_siz()->access_cluster(COD_params);
  //cod->set(Corder, 0, 0, Corder_LRCP);
  cod->set(Corder, 0, 0, Corder_PCRL);

  cod->parse_string("Cprecincts={512,512},{256,256}");

  //kdu_params *poc = codestream.access_siz()->access_cluster(POC_params);
  //poc->parse_string("Porder={0,0,1,2,1,LRCP},{2,0,1,6,1,LRCP}");
  //poc->parse_string("Porder={2,0,1,6,1,LRCP}");
  
  kdu_params *qcd = codestream.access_siz()->access_cluster(QCD_params);
  qcd->set(Qstep, 0, 0, 1.0/8192.0);

  kdu_params *org = codestream.access_siz()->access_cluster(ORG_params);
  org->set(ORGgen_tlm, 0, 0, 1);
  
  codestream.access_siz()->finalize_all();

  /* Write the JP2 header
   */
  if (jp2_ultimate_tgt.exists()) {
    int precisions[1];
	bool is_signed[1];
    kdu_int16 *stripe_bufs[1];
    jp2_dimensions dimensions;
    dimensions = jp2_out.access_dimensions();
    dimensions.init( codestream.access_siz() );
    jp2_colour colour = jp2_out.access_colour();
    colour.init( JP2_sLUM_SPACE );
    jp2_out.write_header();

    /* Compress the image in memory in one shot.
     */
    jp2_out.open_codestream( true );
    int height[3] = { sizeY, sizeY, sizeY };
	kdu_long layer_sizes[1];
    layer_sizes[0] = (kdu_long)targetSizeBytes;
    //compressor.start( codestream, 1, layer_sizes );

    compressor.start( codestream, 1, layer_sizes, NULL, 0, false,
		      false, true, 0.0, 1, false, threadEnvRef,
		      NULL, doubleBufferSize );

    //compressor.start( codestream );
    // In theory, the image should go out in one hit, but loop anyway.
    /*
     * JVB - SECRET SAUCE: Must use *stripe_bufs[], must use precisions[],
     *       and data coming in MUST be signed.
     */
    stripe_bufs[0] = gray;
    precisions[0] = precision;
	is_signed[0] = isSigned;
    int row_gaps[1] = {stride};
    while (compressor.push_stripe( stripe_bufs, height, NULL, row_gaps, precisions, is_signed ));

    // Clean up.
    compressor.finish();
    codestream.destroy();
    if (jp2_ultimate_tgt.exists())
      jp2_ultimate_tgt.close();
    if (threadEnv.exists())
      threadEnv.destroy();

    *actualSizeBytes = memTgt->getSize();
    delete memTgt;

    return tgt;
  } else
    return NULL;

} /* end KDUCompressJP2Mono16 */


bool KDUGetJP2SizeBuffer( const unsigned char *buffer, int numBytes, int &sizeX, int &sizeY)
{
    jp2_source jp2_in;
    kdu_codestream codestream;

    // Redirect the kdu error routine so that kdu errors don't terminate the app.
    kdu_customize_errors( &err_formatter );
    
    /* Open up through KDU and get codestream/size info.
     */
    try {
      // Create a KDU source buffer in memory to read the stripe.
      kdu_simple_buffer_source input( buffer, numBytes );
      codestream.create( &input, NULL );
      
      siz_params *siz = codestream.access_siz();
      siz->get( Ssize, 0, 0, sizeY );
      siz->get( Ssize, 0, 1, sizeX );
      
      codestream.destroy();

    } catch (...) {
        // File open failed.
        sizeX = 0;
        sizeY = 0;
        return false;
    }

    return true;
}


bool KDUGetJP2Size( const char filename[], int &sizeX, int &sizeY)
{
    jp2_family_src jp2_ultimate_src;
    jp2_source jp2_in;
    kdu_codestream codestream;

    // Redirect the kdu error routine so that kdu errors don't terminate the app.
    kdu_customize_errors( &err_formatter );

    /* Open up through KDU and get codestream/size info.
     */
    try {
        jp2_ultimate_src.open( filename, true );
        if (jp2_in.open( &jp2_ultimate_src ) == false)
            throw 0;
        else {
            if (!jp2_in.read_header()) {
                jp2_in.close();
                return false;
            }
        }
    } catch (...) {
        // File open failed.
        sizeX = 0;
        sizeY = 0;
        return false;
    }

    codestream.create( &jp2_in, NULL );
    
    siz_params *siz = codestream.access_siz();
    siz->get( Ssize, 0, 0, sizeY );
    siz->get( Ssize, 0, 1, sizeX );

    codestream.destroy();
    jp2_in.close();

    return true;
}


/***************************************************************************
 * function KDUReadJP2Mono8
 *
 * Read a "Smart" JPEG2000 file that has been written with KDUWriteJP2Mono8.
 * We only properly read the GeoJP2 data if it was written by our routine.
 *
 * If 'outBuf' is supplied, that is used to return the result, otherwise
 * a buffer is allocated  and the calling routine must deallocate the image
 * when done with it. 
 *
 * The output image returned is an 8-bit grayscale image, On error, the
 * returned pointer is NULL, even if a buffer was supplied.
 *
 * Inputs:
 * 'filename' is the input file name.
 * 'ROI' is the region of interest.  If all values are 0, the whole image.
 *
 * When ROI is all zeroes, the following outputs are set, otherwise, they
 * are meaningless:
 * 'sizeX' and 'sizeY' are the size of the image in pixels.
 * 'utm' is a RECT containing the upper-left and lower-right corners of the
 *      image to the nearest meter.  If no geo data is present, all zeros.
 * 'zone' is the output UTM zone (1 or 2-digit ASCII number and one letter).
 * 'time64' is the output UTC time nugget which indicates the time of the
 *         image, which could be 0 if no time was encoded.
 ***************************************************************************/

unsigned char *KDUReadJP2Mono8( char filename[], unsigned char *outBuf,
                                kdu_dims ROI, int &sizeX, int &sizeY, double &res,
                                RECT &utm, char zone[], __int64 &time64, int res_level )
{
  int blockSize;
  int numComponents = 1;
  unsigned char *buf;
  jp2_family_src jp2_ultimate_src;
  jp2_source jp2_in;
  kdu_codestream codestream;
  siz_params *siz;
  kdu_thread_env threadEnv;
  kdu_thread_env *threadEnvRef = NULL;


  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Set up multi-threaded environment.
  if (numCPUs > 1) {
    int nt = numCPUs;
    threadEnv.create();
    for (int ct = 1; ct < nt; ct++)
      if (!threadEnv.add_thread()) {
	nt = ct; // Unable to create all the threads requested
	PutLog( "KDUReadJP2Mono8 - Tried to create %d threads, but created %d instead\n",
		numCPUs, nt );
      }
    threadEnvRef = &threadEnv;
  }

  /* Open up through KDU and get codestream/size info.
   */
  try {
    jp2_ultimate_src.open( filename, true );
    if (jp2_in.open( &jp2_ultimate_src ) == false)
      throw 0;
    else {
      if (!jp2_in.read_header()) {
	jp2_in.close();
	return NULL;
      }
    }
  }
  catch (...) {
    // File open failed.
    sizeX = 0;
    sizeY = 0;
    return NULL;
  }

  codestream.create( &jp2_in, threadEnvRef );
  kdu_dims *reg_ptr = NULL;

  /* If all elements of the ROI are 0, get the whole image,
   * otherwise, pull out the requested region.
   */
  if (ROI.pos.x == 0  && ROI.pos.y == 0 &&
      ROI.size.x == 0 && ROI.size.y == 0) {
    siz = codestream.access_siz();
    siz->get( Ssize, 0, 0, ROI.size.y );
    siz->get( Ssize, 0, 1, ROI.size.x );
    reg_ptr = NULL;
  } else
    reg_ptr = &ROI;
  
  codestream.apply_input_restrictions( 0, numComponents, res_level, 0, reg_ptr );
  
  kdu_dims output_dims;
  codestream.get_dims( 0, output_dims );
  sizeX = output_dims.size.x;
  sizeY = output_dims.size.y;

  blockSize = sizeX * sizeY;
  if (outBuf == NULL) {
    buf = (unsigned char *)malloc( blockSize );
    if (buf == NULL) {
      codestream.destroy();
      jp2_in.close();
      return NULL;
    }
  } else
    buf = outBuf;

  /* Try to read out the "smart" GeoJP2 information.
   */
  bool found = false;
  jp2_input_box smartBox;
  char datetime[DATESIZE];
  double geodoubles[GEODOUBLESIZE];
  unsigned char geoBoxAndHeader[TIFFBOXSIZE + HEADERCHARSIZE];
  unsigned short geokeys[GEOKEYSIZE];
  TIFFTagA tifftags[NUMTIFFTAGS];
  if (smartBox.open( &jp2_ultimate_src )) {
    //PutLog( "KDUReadJP2Mono8 - opened first box\n" );
    if (smartBox.get_box_type() == jp2_uuid_4cc) {
      //PutLog( "KDUReadJP2Mono8 - first box is UUID\n" );
      found = true;
    } else {
      bool openok;
      do {
	smartBox.close();
	openok = smartBox.open_next();
	if (openok) {
	  //PutLog( "KDUReadJP2Mono8 - opened another box\n" );
	  if (smartBox.get_box_type() == jp2_uuid_4cc) {
	    PutLog( "KDUReadJP2Mono8 - found UUID box\n" );
	    found = true;
	  }
	}
      } while (openok == true && found == false);
    }
    if (found == true) {
      double res;
      smartBox.read( geoBoxAndHeader, TIFFBOXSIZE + HEADERCHARSIZE );
      smartBox.read( (unsigned char *)tifftags, sizeof(TIFFTagA) * NUMTIFFTAGS );
      smartBox.read( (unsigned char *)geokeys, sizeof(unsigned short) * GEOKEYSIZE);
      smartBox.read( (unsigned char *)geodoubles, sizeof(double) * GEODOUBLESIZE );
      smartBox.read( (unsigned char *)datetime, sizeof(unsigned char) * DATESIZE);
      smartBox.close();
      utm.left = (long)geodoubles[6];
      utm.top  = (long)geodoubles[7];
      res = geodoubles[0];
      utm.right  = (long)(utm.left + (res * sizeX));
      utm.bottom = (long)(utm.top - (res * sizeY));
      UTCTime( datetime, time64 );
      GetUTMCode( geokeys[15], utm.top, zone );
    } else {
      PutLog( "KDUReadJP2Mono8 - never found UUID box\n" );
      utm.left   = 0;
      utm.top    = 0;
      utm.right  = 0;
      utm.bottom = 0;
    }
  }

  int stripe_heights[1];
  int max_stripe_heights[1];
  int absolute_max_stripe_height = 1024;
  int preferred_min_stripe_height = 8;
  kdu_stripe_decompressor decompressor;

  // Now decompress the image using `kdu_stripe_decompressor'
  decompressor.start( codestream, false, false, threadEnvRef, NULL, doubleBufferSize );
  decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
					       absolute_max_stripe_height,
					       stripe_heights, max_stripe_heights );
  bool continues = true;
  unsigned char *bufptr = buf;
  try {
    while (continues) {
      decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
						   absolute_max_stripe_height,
						   stripe_heights, NULL );
      continues = decompressor.pull_stripe( bufptr, stripe_heights );
      bufptr += stripe_heights[0] * sizeX;
    }
  }
  catch (...) {
    // Input data is corrupt.  Cannot continue.
    if (outBuf != buf)
      free( buf );
    buf = NULL;
  }

  // Cleanup.
  decompressor.finish();
  codestream.destroy();
  jp2_in.close();
  if (threadEnv.exists())
    threadEnv.destroy();
  return buf;

} /* end KDUReadJP2Mono8 */



/***************************************************************************
 * function KDUReadJP2Mono8NoGeo
 *
 * If 'outBuf' is supplied, that is used to return the result, otherwise
 * a buffer is allocated  and the calling routine must deallocate the image
 * when done with it. 
 *
 * The output image returned is an 8-bit grayscale image, On error, the
 * returned pointer is NULL, even if a buffer was supplied.
 *
 * Inputs:
 * 'filename' is the input file name.
 * 'ROI' is the region of interest.  If all values are 0, the whole image.
 ***************************************************************************/

unsigned char *KDUReadJP2Mono8NoGeo( char filename[], unsigned char *outBuf,
					kdu_dims ROI, int &sizeX, int &sizeY )
{
  int blockSize;
  int numComponents = 1;
  unsigned char *buf;
  jp2_family_src jp2_ultimate_src;
  jp2_source jp2_in;
  kdu_codestream codestream;
  siz_params *siz;
  kdu_thread_env threadEnv;
  kdu_thread_env *threadEnvRef = NULL;


  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Set up multi-threaded environment.
  if (numCPUs > 1) {
    int nt = numCPUs;
    threadEnv.create();
    for (int ct = 1; ct < nt; ct++)
      if (!threadEnv.add_thread()) {
	nt = ct; // Unable to create all the threads requested
	PutLog( "KDUReadJP2Mono8 - Tried to create %d threads, but created %d instead\n",
		numCPUs, nt );
      }
    threadEnvRef = &threadEnv;
  }

  /* Open up through KDU and get codestream/size info.
   */
  try {
    jp2_ultimate_src.open( filename, true );
    if (jp2_in.open( &jp2_ultimate_src ) == false)
      throw 0;
    else {
      if (!jp2_in.read_header()) {
	jp2_in.close();
	return NULL;
      }
    }
  }
  catch (...) {
    // File open failed.
    sizeX = 0;
    sizeY = 0;
    return NULL;
  }

  codestream.create( &jp2_in, threadEnvRef );
  kdu_dims *reg_ptr = NULL;

  /* If all elements of the ROI are 0, get the whole image,
   * otherwise, pull out the requested region.
   */
  if (ROI.pos.x == 0  && ROI.pos.y == 0 &&
      ROI.size.x == 0 && ROI.size.y == 0) {
    siz = codestream.access_siz();
    siz->get( Ssize, 0, 0, ROI.size.y );
    siz->get( Ssize, 0, 1, ROI.size.x );
    reg_ptr = NULL;
  } else
    reg_ptr = &ROI;
  sizeX = ROI.size.x;
  sizeY = ROI.size.y;
  codestream.apply_input_restrictions( 0, numComponents, 0, 0, reg_ptr );

  blockSize = sizeX * sizeY;
  if (outBuf == NULL) {
    buf = (unsigned char *)malloc( blockSize );
    if (buf == NULL) {
      codestream.destroy();
      jp2_in.close();
      return NULL;
    }
  } else
    buf = outBuf;

  int stripe_heights[1];
  int max_stripe_heights[1];
  int absolute_max_stripe_height = 1024;
  int preferred_min_stripe_height = 8;
  kdu_stripe_decompressor decompressor;

  // Now decompress the image using `kdu_stripe_decompressor'
  decompressor.start( codestream, false, false, threadEnvRef, NULL, doubleBufferSize );
  decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
					       absolute_max_stripe_height,
					       stripe_heights, max_stripe_heights );
  bool continues = true;
  unsigned char *bufptr = buf;
  try {
    while (continues) {
      decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
						   absolute_max_stripe_height,
						   stripe_heights, NULL );
      continues = decompressor.pull_stripe( bufptr, stripe_heights );
      bufptr += stripe_heights[0] * sizeX;
    }
  }
  catch (...) {
    // Input data is corrupt.  Cannot continue.
    if (outBuf != buf)
      free( buf );
    buf = NULL;
  }

  // Cleanup.
  decompressor.finish();
  codestream.destroy();
  jp2_in.close();
  if (threadEnv.exists())
    threadEnv.destroy();
  return buf;

} /* end KDUReadJP2Mono8NoGeo */



/***************************************************************************
 * function KDUReadJP2Mono16
 *
 * Read a "Smart" JPEG2000 file that has been written with KDUWriteJP2Mono16.
 * We only properly read the GeoJP2 data if it was written by our routine.
 *
 * The output image returned is a signed 16-bit buffer.  If an output buffer
 * is passed in through 'outBuf', this is returned on success.  If outBuf is
 * NULL, the returned buffer is allocated in this routine and the calling
 * routine must deallocate the image when done with it.  On error, the
 * returned pointer is always NULL.
 *
 * Inputs:
 * 'filename' is the input file name.
 * 'ROI' is the region of interest.  If all values are 0, the whole image.
 *
 * When ROI is all zeroes, the following outputs are set, otherwise, they
 * are meaningless:
 * 'sizeX' and 'sizeY' are the size of the image in pixels.
 * 'utm' is a RECT containing the upper-left and lower-right corners of the
 *      image to the nearest meter. 
 * 'zone' is the output UTM zone (1 or 2-digit ASCII number and one letter).
 * 'time64' is the output UTC time nugget which indicates the time of the
 *         image, which could be 0 if no time was encoded.
 ***************************************************************************/

short *KDUReadJP2Mono16( char filename[], kdu_dims ROI,
			    int &sizeX, int &sizeY, double &res, 
			    RECT &utm, char zone[], __int64 &time64,
			    bool isSigned, int precision, short *outBuf )
{
  int blockSize;
  int numComponents = 1;
  short *buf;
  jp2_family_src jp2_ultimate_src;
  jp2_source jp2_in;
  kdu_codestream codestream;
  siz_params *siz;
  kdu_thread_env threadEnv;
  kdu_thread_env *threadEnvRef = NULL;


  // Check inputs
  if (precision < 10 || precision > 16)
    return NULL;

  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Set up multi-threaded environment.
  if (numCPUs > 1) {
    int nt = numCPUs;
    threadEnv.create();
    for (int ct = 1; ct < nt; ct++)
      if (!threadEnv.add_thread()) {
	nt = ct; // Unable to create all the threads requested
	PutLog( "KDUReadJP2Mono16 - Tried to create %d threads, but created %d instead\n",
		numCPUs, nt );
      }
    threadEnvRef = &threadEnv;
  }

  /* Open up through KDU and get codestream/size info.
   */
  try {
    jp2_ultimate_src.open( filename, true );
    if (jp2_in.open( &jp2_ultimate_src ) == false)
      throw 0;
    else {
      if (!jp2_in.read_header()) {
	jp2_in.close();
	return NULL;
      }
    }
  }
  catch (...) {
    // File open failed.
    sizeX = 0;
    sizeY = 0;
    return NULL;
  }

  codestream.create( &jp2_in, threadEnvRef );

  kdu_dims* reg_ptr = NULL;
  /* If all elements of the ROI are 0, get the whole image,
   * otherwise, pull out the requested region.
   */
//  if (ROI.pos.x == 0  && ROI.pos.y == 0 &&
//      ROI.size.x == 0 && ROI.size.y == 0) 
  {
    siz = codestream.access_siz();
    siz->get( Ssize, 0, 0, ROI.size.y );
    siz->get( Ssize, 0, 1, ROI.size.x );
  sizeX = ROI.size.x;
  sizeY = ROI.size.y;

    reg_ptr = NULL;
  } 
/*
  else
  {
     reg_ptr = &ROI;
  }
*/

  sizeX = ROI.size.x;
  sizeY = ROI.size.y;
  codestream.apply_input_restrictions( 0, numComponents, 0, 0, reg_ptr );
  //codestream.apply_input_restrictions( 0, numComponents, 2, 0, reg_ptr );

  blockSize = sizeX * sizeY;
  if (outBuf == NULL) {
    buf = (short *)malloc( blockSize*sizeof(short) );
    if (buf == NULL) {
      codestream.destroy();
      jp2_in.close();
      return NULL;
    }
  } else
    buf = outBuf;

  /* Try to read out the "smart" GeoJP2 information.
   */
  bool found = false;
  jp2_input_box smartBox;
  char datetime[DATESIZE];
  double geodoubles[GEODOUBLESIZE];
  unsigned char geoBoxAndHeader[TIFFBOXSIZE + HEADERCHARSIZE];
  unsigned short geokeys[GEOKEYSIZE];
  TIFFTagA tifftags[NUMTIFFTAGS];

  if (smartBox.open( &jp2_ultimate_src )) {
    //PutLog( "KDUReadJP2Mono16 - opened first box\n" );
    if (smartBox.get_box_type() == jp2_uuid_4cc) {
      //PutLog( "KDUReadJP2Mono16 - first box is UUID\n" );
      found = true;
    } else {
      bool openok;
      do {
	smartBox.close();
	openok = smartBox.open_next();
	if (openok) {
	  //PutLog( "KDUReadJP2Mono16 - opened another box\n" );
	  if (smartBox.get_box_type() == jp2_uuid_4cc) {
	    PutLog( "KDUReadJP2Mono16 - found UUID box\n" );
	    found = true;
	  }
	}
      } while (openok == true && found == false);
    }
    if (found == true) {
      double res;
      smartBox.read( geoBoxAndHeader, TIFFBOXSIZE + HEADERCHARSIZE );
      smartBox.read( (unsigned char *)tifftags, sizeof(TIFFTagA) * NUMTIFFTAGS );
      smartBox.read( (unsigned char *)geokeys, sizeof(unsigned short) * GEOKEYSIZE);
      smartBox.read( (unsigned char *)geodoubles, sizeof(double) * GEODOUBLESIZE );
      smartBox.read( (unsigned char *)datetime, sizeof(unsigned char) * DATESIZE);
      smartBox.close();
      utm.left = (long)geodoubles[6];
      utm.top  = (long)geodoubles[7];
      res = geodoubles[0];
      utm.right  = (long)(utm.left + (res * sizeX));
      utm.bottom = (long)(utm.top - (res * sizeY));
      UTCTime( datetime, time64 );
      GetUTMCode( geokeys[15], utm.top, zone );
    } else {
      PutLog( "KDUReadJP2Mono16 - never found UUID box\n" );
      res    = 0;
      time64 = 0;
      utm.left   = 0;
      utm.right  = 0;
      utm.top    = 0;
      utm.bottom = 0;
      if (zone != NULL)
	zone[0] = '\0';
    }
  }
    
  // Extract region directly from the image using kdu_stripe_decompressor
  int preferred_min_stripe_height = 8;
  int absolute_max_stripe_height = 1024;
  int precisions[1] = { precision };
  int stripe_heights[1];
  int max_stripe_heights[1];
  bool signs[1] = { isSigned };
  kdu_stripe_decompressor decompressor;

  decompressor.start( codestream, false, false, threadEnvRef, NULL, doubleBufferSize );
  decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
					       absolute_max_stripe_height,
					       stripe_heights, max_stripe_heights );
  bool continues = true;
  short *bufptr = buf;
  while (continues) {
    decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
						 absolute_max_stripe_height,
						 stripe_heights, NULL );
    continues = decompressor.pull_stripe( bufptr, stripe_heights, NULL, NULL,
					  NULL, precisions, signs );
    bufptr += stripe_heights[0] * sizeX;
  }
  decompressor.finish();
  codestream.destroy();
  jp2_in.close();
  if (jp2_ultimate_src.exists())
    jp2_ultimate_src.close();
  if (threadEnv.exists())
    threadEnv.destroy();

  // Convert to unsigned representation
  if (!isSigned) {
    int offset = (1 << precision) >> 1;
    for (int i = 0 ; i < sizeX * sizeY; i++)
      buf[i] = buf[i] + offset;
  }
  return buf;

} /* end KDUReadJP2Mono16 */


/***************************************************************************
 * function KDUReadJP2Mono8
 *
 * Read a "Smart" JPEG2000 file that has been written with KDUWriteJP2Mono8.
 * We only properly read the GeoJP2 data if it was written by our routine.
 *
 * The output image returned is an 8-bit grayscale image, and the calling
 * routine must deallocate the image when done with it.  On error, the
 * returned pointer is NULL.
 *
 * Inputs:
 * 'buffer' is the input compressed buffer.
 * 'ROI' is the region of interest.  If all values are 0, the whole image.
 *
 * When ROI is all zeroes, the following outputs are set, otherwise, they
 * are meaningless:
 * 'sizeX' and 'sizeY' are the size of the image in pixels.
 * 'utm' is a RECT containing the upper-left and lower-right corners of the
 *      image to the nearest meter. 
 * 'zone' is the output UTM zone (1 or 2-digit ASCII number and one letter).
 * 'time64' is the output UTC time nugget which indicates the time of the
 *         image, which could be 0 if no time was encoded.
 ***************************************************************************/

unsigned char *KDUReadJP2Mono8Buf( unsigned char *buffer, unsigned char *outBuf, int numBytes, int numCPUs, kdu_dims ROI,
				   int &sizeX, int &sizeY, double &res,
				   RECT &utm, char zone[], __int64 &time64 )
{
  hsdMemorySource *memTgt;
  int blockSize;
  int numComponents = 1;
  unsigned char *buf;
  jp2_family_src jp2_ultimate_src;
  jp2_source jp2_in;
  kdu_codestream codestream;
  siz_params *siz;
  kdu_thread_env threadEnv;
  kdu_thread_env *threadEnvRef = NULL;


  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Set up multi-threaded environment.
  if (numCPUs > 1) {
    int nt = numCPUs;
    threadEnv.create();
    for (int ct = 1; ct < nt; ct++)
      if (!threadEnv.add_thread()) {
	nt = ct; // Unable to create all the threads requested
	PutLog( "KDUReadJP2Mono8 - Tried to create %d threads, but created %d instead\n",
		numCPUs, nt );
      }
    threadEnvRef = &threadEnv;
  }

  /* Open up through KDU and get codestream/size info.
   */
  try {
    //jp2_ultimate_src.open( filename, true );
    memTgt = new hsdMemorySource( buffer, numBytes );
    jp2_ultimate_src.open( memTgt );
    if (jp2_in.open( &jp2_ultimate_src ) == false)
      throw 0;
    else {
      if (!jp2_in.read_header()) {
	jp2_in.close();
	return NULL;
      }
    }
  }
  catch (...) {
    // File open failed.
    sizeX = 0;
    sizeY = 0;
    return NULL;
  }

  codestream.create( &jp2_in, threadEnvRef );
  kdu_dims *reg_ptr = NULL;

  /* If all elements of the ROI are 0, get the whole image,
   * otherwise, pull out the requested region.
   */
  if (ROI.pos.x == 0  && ROI.pos.y == 0 &&
      ROI.size.x == 0 && ROI.size.y == 0) {
    siz = codestream.access_siz();
    siz->get( Ssize, 0, 0, ROI.size.y );
    siz->get( Ssize, 0, 1, ROI.size.x );
    reg_ptr = NULL;
  } else
    reg_ptr = &ROI;
  sizeX = ROI.size.x;
  sizeY = ROI.size.y;
  codestream.apply_input_restrictions( 0, numComponents, 0, 0, reg_ptr );

  blockSize = sizeX * sizeY;
  if (outBuf == NULL) {
    buf = (unsigned char *)malloc( blockSize );
    if (buf == NULL) {
      codestream.destroy();
      jp2_in.close();
      return NULL;
    }
  } else
    buf = outBuf;

  /* Try to read out the "smart" GeoJP2 information.
   */
  bool found = false;
  jp2_input_box smartBox;
  char datetime[DATESIZE];
  double geodoubles[GEODOUBLESIZE];
  unsigned char geoBoxAndHeader[TIFFBOXSIZE + HEADERCHARSIZE];
  unsigned short geokeys[GEOKEYSIZE];
  TIFFTagA tifftags[NUMTIFFTAGS];
  if (smartBox.open( &jp2_ultimate_src )) {
    //PutLog( "KDUReadJP2Mono8 - opened first box\n" );
    if (smartBox.get_box_type() == jp2_uuid_4cc) {
      //PutLog( "KDUReadJP2Mono8 - first box is UUID\n" );
      found = true;
    } else {
      bool openok;
      do {
	smartBox.close();
	openok = smartBox.open_next();
	if (openok) {
	  //PutLog( "KDUReadJP2Mono8 - opened another box\n" );
	  if (smartBox.get_box_type() == jp2_uuid_4cc) {
	    PutLog( "KDUReadJP2Mono8 - found UUID box\n" );
	    found = true;
	  }
	}
      } while (openok == true && found == false);
    }
    if (found == true) {
      double res;
      smartBox.read( geoBoxAndHeader, TIFFBOXSIZE + HEADERCHARSIZE );
      smartBox.read( (unsigned char *)tifftags, sizeof(TIFFTagA) * NUMTIFFTAGS );
      smartBox.read( (unsigned char *)geokeys, sizeof(unsigned short) * GEOKEYSIZE);
      smartBox.read( (unsigned char *)geodoubles, sizeof(double) * GEODOUBLESIZE );
      smartBox.read( (unsigned char *)datetime, sizeof(unsigned char) * DATESIZE);
      smartBox.close();
      utm.left = (long)geodoubles[6];
      utm.top  = (long)geodoubles[7];
      res = geodoubles[0];
      utm.right  = (long)(utm.left + (res * sizeX));
      utm.bottom = (long)(utm.top - (res * sizeY));
      UTCTime( datetime, time64 );
      GetUTMCode( geokeys[15], utm.top, zone );
    } else {
      PutLog( "KDUReadJP2Mono8 - never found UUID box\n" );
      utm.left   = 0;
      utm.top    = 0;
      utm.right  = 0;
      utm.bottom = 0;
    }
  }

  int stripe_heights[1];
  int max_stripe_heights[1];
  int absolute_max_stripe_height = 1024;
  int preferred_min_stripe_height = 8;
  kdu_stripe_decompressor decompressor;

  // Now decompress the image using `kdu_stripe_decompressor'
  decompressor.start( codestream, false, false, threadEnvRef, NULL, doubleBufferSize );
  decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
					       absolute_max_stripe_height,
					       stripe_heights, max_stripe_heights );
  bool continues = true;
  unsigned char *bufptr = buf;
  try {
    while (continues) {
      decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
						   absolute_max_stripe_height,
						   stripe_heights, NULL );
      continues = decompressor.pull_stripe( bufptr, stripe_heights );
      bufptr += stripe_heights[0] * sizeX;
    }
  }
  catch (...) {
    // Input data is corrupt.  Cannot continue.
    if (outBuf != buf)
      free( buf );
    buf = NULL;
  }

  // Cleanup.
  decompressor.finish();
  codestream.destroy();
  jp2_in.close();
  if (threadEnv.exists())
    threadEnv.destroy();
  return buf;
} /* end KDUReadJP2Mono8 */


/***************************************************************************
 * function KDUReadJP2Mono16
 *
 * Read a "Smart" JPEG2000 file that has been written with KDUWriteJP2Mono16.
 * We only properly read the GeoJP2 data if it was written by our routine.
 *
 * The output image returned is a signed 16-bit buffer, and the calling
 * routine must deallocate the image when done with it.  On error, the
 * returned pointer is NULL.
 *
 * Inputs:
 * 'filename' is the input file name.
 * 'ROI' is the region of interest.  If all values are 0, the whole image.
 *
 * When ROI is all zeroes, the following outputs are set, otherwise, they
 * are meaningless:
 * 'sizeX' and 'sizeY' are the size of the image in pixels.
 * 'utm' is a RECT containing the upper-left and lower-right corners of the
 *      image to the nearest meter. 
 * 'zone' is the output UTM zone (1 or 2-digit ASCII number and one letter).
 * 'time64' is the output UTC time nugget which indicates the time of the
 *         image, which could be 0 if no time was encoded.
 ***************************************************************************/

short *KDUReadJP2Mono16Buf( unsigned char *buffer, short *outBuf, int numBytes, kdu_dims ROI,
								int &sizeX, int &sizeY, double &res, 
								RECT &utm, char zone[], __int64 &time64,
                            bool isSigned, int precision, int stride, int res_level )
{
    std::auto_ptr<hsdMemorySource> memTgt;
  int blockSize;
  int numComponents = 1;
  short *buf;
  jp2_family_src jp2_ultimate_src;
  jp2_source jp2_in;
  kdu_codestream codestream;
  siz_params *siz;
  kdu_thread_env threadEnv;
  kdu_thread_env *threadEnvRef = NULL;


  // Check inputs
  if (precision < 10 || precision > 16)
    return NULL;

  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Set up multi-threaded environment.
  if (numCPUs > 1) {
    int nt = numCPUs;
    threadEnv.create();
    for (int ct = 1; ct < nt; ct++)
      if (!threadEnv.add_thread()) {
	nt = ct; // Unable to create all the threads requested
	PutLog( "KDUReadJP2Mono16 - Tried to create %d threads, but created %d instead\n",
		numCPUs, nt );
      }
    threadEnvRef = &threadEnv;
  }

  /* Open up through KDU and get codestream/size info.
   */
  int jp2Open = 1;
  kdu_simple_buffer_source input( buffer, numBytes );
  if((buffer[0] == 0xff) && (buffer[1] == 0x4f)) { // raw codestream
    jp2Open = 0;
  } else {
  try {
      memTgt.reset(new hsdMemorySource( buffer, numBytes ));
      jp2_ultimate_src.open( memTgt.get() );

      //jp2_ultimate_src.open( filename, true );
      if (jp2_in.open( &jp2_ultimate_src ) == false){
          throw 0;
      } else {
          if (!jp2_in.read_header()) {
              jp2_in.close();
              return NULL;
          }else{
              codestream.create( &jp2_in, threadEnvRef );
          }
      }
  }
  catch (...) {
    // File open failed.  Maybe it's a tile.
      jp2Open = 0;
    //sizeX = 0;
    //sizeY = 0;
    //return NULL;
  }
  }

  try {
      if(!jp2Open)
        codestream.create( &input, threadEnvRef );
  } catch (...) {
        return NULL;
  }

  //codestream.create( &jp2_in, threadEnvRef );
  kdu_dims *reg_ptr = NULL;

  /* If all elements of the ROI are 0, get the whole image,
   * otherwise, pull out the requested region.
   */
  if (ROI.pos.x == 0  && ROI.pos.y == 0 &&
      ROI.size.x == 0 && ROI.size.y == 0) {
    siz = codestream.access_siz();
    siz->get( Ssize, 0, 0, ROI.size.y );
    siz->get( Ssize, 0, 1, ROI.size.x );
    reg_ptr = NULL;
  } else
    reg_ptr = &ROI;
  sizeX = ROI.size.x;
  sizeY = ROI.size.y;
  //codestream.apply_input_restrictions( 0, numComponents, 0, 0, reg_ptr );
  codestream.apply_input_restrictions( 0, numComponents, res_level, 0, reg_ptr );

  //codestream.set_block_truncation(2048);  this looks like crap but makes it go fast

  blockSize = sizeX * sizeY;
  if (outBuf == NULL) {
    buf = (short *)malloc( blockSize*sizeof(short) );
    if (buf == NULL) {
      codestream.destroy();
	  if(jp2Open)
	      jp2_in.close();
      return NULL;
    }
  } else
    buf = outBuf;

  /* Try to read out the "smart" GeoJP2 information.
   */
  bool found = false;
  jp2_input_box smartBox;
  char datetime[DATESIZE];
  double geodoubles[GEODOUBLESIZE];
  unsigned char geoBoxAndHeader[TIFFBOXSIZE + HEADERCHARSIZE];
  unsigned short geokeys[GEOKEYSIZE];
  TIFFTagA tifftags[NUMTIFFTAGS];

  if (jp2Open && smartBox.open( &jp2_ultimate_src )) {
    //PutLog( "KDUReadJP2Mono16 - opened first box\n" );
    if (smartBox.get_box_type() == jp2_uuid_4cc) {
      //PutLog( "KDUReadJP2Mono16 - first box is UUID\n" );
      found = true;
    } else {
      bool openok;
      do {
	smartBox.close();
	openok = smartBox.open_next();
	if (openok) {
	  //PutLog( "KDUReadJP2Mono16 - opened another box\n" );
	  if (smartBox.get_box_type() == jp2_uuid_4cc) {
	    PutLog( "KDUReadJP2Mono16 - found UUID box\n" );
	    found = true;
	  }
	}
      } while (openok == true && found == false);
    }
    if (found == true) {
      double res;
      smartBox.read( geoBoxAndHeader, TIFFBOXSIZE + HEADERCHARSIZE );
      smartBox.read( (unsigned char *)tifftags, sizeof(TIFFTagA) * NUMTIFFTAGS );
      smartBox.read( (unsigned char *)geokeys, sizeof(unsigned short) * GEOKEYSIZE);
      smartBox.read( (unsigned char *)geodoubles, sizeof(double) * GEODOUBLESIZE );
      smartBox.read( (unsigned char *)datetime, sizeof(unsigned char) * DATESIZE);
      smartBox.close();
      utm.left = (long)geodoubles[6];
      utm.top  = (long)geodoubles[7];
      res = geodoubles[0];
      utm.right  = (long)(utm.left + (res * sizeX));
      utm.bottom = (long)(utm.top - (res * sizeY));
      UTCTime( datetime, time64 );
      GetUTMCode( geokeys[15], utm.top, zone );
    } else {
      PutLog( "KDUReadJP2Mono16 - never found UUID box\n" );
      res    = 0;
      time64 = 0;
      utm.left   = 0;
      utm.right  = 0;
      utm.top    = 0;
      utm.bottom = 0;
      if (zone != NULL)
	zone[0] = '\0';
    }
  }

  if (stride==-1) {
    stride = sizeX;
  }
    
  // Extract region directly from the image using kdu_stripe_decompressor
  int preferred_min_stripe_height = 8;
  int absolute_max_stripe_height = 1024;
  int precisions[1] = { precision };
  int stripe_heights[1];
  int max_stripe_heights[1];
  bool signs[1] = { isSigned };
  int row_gaps[1] = {stride};
  kdu_stripe_decompressor decompressor;

  decompressor.start( codestream, false, false, threadEnvRef, NULL, doubleBufferSize );
  decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
					       absolute_max_stripe_height,
					       stripe_heights, max_stripe_heights );
  bool continues = true;
  short *bufptr = buf;
  while (continues) {
    decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
						 absolute_max_stripe_height,
						 stripe_heights, NULL );
    continues = decompressor.pull_stripe( bufptr, stripe_heights, NULL, NULL,
					  row_gaps, precisions, signs );
    bufptr += stripe_heights[0] * stride;
  }
  decompressor.finish();
  codestream.destroy();
  if(jp2Open)
	  jp2_in.close();
  if (jp2Open && jp2_ultimate_src.exists())
    jp2_ultimate_src.close();
  if (threadEnv.exists())
    threadEnv.destroy();

  // Convert to unsigned representation
#if 0
  if (!isSigned) {
    int offset = (1 << precision) >> 1;
    for (int i = 0 ; i < sizeX * sizeY; i++)
      buf[i] = buf[i] + offset;
  }
#endif
  return buf;

} /* end KDUReadJP2Mono16 */


/***************************************************************************
 * function KDUReadJP2RGB16
 *
 * Read a "Smart" JPEG2000 file that has been written with KDUWriteJP2Mono16.
 * We only properly read the GeoJP2 data if it was written by our routine.
 *
 * The output image returned is a signed 16-bit buffer, and the calling
 * routine must deallocate the image when done with it.  On error, the
 * returned pointer is NULL.
 *
 * Inputs:
 * 'filename' is the input file name.
 * 'ROI' is the region of interest.  If all values are 0, the whole image.
 *
 * When ROI is all zeroes, the following outputs are set, otherwise, they
 * are meaningless:
 * 'sizeX' and 'sizeY' are the size of the image in pixels.
 * 'utm' is a RECT containing the upper-left and lower-right corners of the
 *      image to the nearest meter. 
 * 'zone' is the output UTM zone (1 or 2-digit ASCII number and one letter).
 * 'time64' is the output UTC time nugget which indicates the time of the
 *         image, which could be 0 if no time was encoded.
 ***************************************************************************/

short *KDUReadJP2RGB16Buf( unsigned char *buffer, short *outBuf, int numBytes, kdu_dims ROI,
								int &sizeX, int &sizeY, double &res, 
								RECT &utm, char zone[], __int64 &time64,
								bool isSigned, int precision, int stride )
{
  hsdMemorySource *memTgt;
  int blockSize;
  int numComponents = 3;
  short *buf;
  jp2_family_src jp2_ultimate_src;
  jp2_source jp2_in;
  kdu_codestream codestream;
  siz_params *siz;
  kdu_thread_env threadEnv;
  kdu_thread_env *threadEnvRef = NULL;


  // Check inputs
  if (precision < 10 || precision > 16)
    return NULL;

  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Set up multi-threaded environment.
  if (numCPUs > 1) {
    int nt = numCPUs;
    threadEnv.create();
    for (int ct = 1; ct < nt; ct++)
      if (!threadEnv.add_thread()) {
	nt = ct; // Unable to create all the threads requested
	PutLog( "KDUReadJP2RGB16 - Tried to create %d threads, but created %d instead\n",
		numCPUs, nt );
      }
    threadEnvRef = &threadEnv;
  }

  /* Open up through KDU and get codestream/size info.
   */
  int jp2Open = 1;
  kdu_simple_buffer_source input( buffer, numBytes );
  if((buffer[0] == 0xff) && (buffer[1] == 0x4f)) { // raw codestream
    jp2Open = 0;
  } else {
  try {
      memTgt = new hsdMemorySource( buffer, numBytes );
      jp2_ultimate_src.open( memTgt );

      //jp2_ultimate_src.open( filename, true );
      if (jp2_in.open( &jp2_ultimate_src ) == false){
          throw 0;
      } else {
          if (!jp2_in.read_header()) {
              jp2_in.close();
              return NULL;
          }else{
              codestream.create( &jp2_in, threadEnvRef );
          }
      }
  }
  catch (...) {
    // File open failed.  Maybe it's a tile.
      jp2Open = 0;
    //sizeX = 0;
    //sizeY = 0;
    //return NULL;
  }
  }

  try {
      if(!jp2Open)
        codestream.create( &input, threadEnvRef );
  } catch (...) {
        return NULL;
  }

  //codestream.create( &jp2_in, threadEnvRef );
  kdu_dims *reg_ptr = NULL;

  /* If all elements of the ROI are 0, get the whole image,
   * otherwise, pull out the requested region.
   */
  if (ROI.pos.x == 0  && ROI.pos.y == 0 &&
      ROI.size.x == 0 && ROI.size.y == 0) {
    siz = codestream.access_siz();
    siz->get( Ssize, 0, 0, ROI.size.y );
    siz->get( Ssize, 0, 1, ROI.size.x );
    reg_ptr = NULL;
  } else
    reg_ptr = &ROI;
  sizeX = ROI.size.x;
  sizeY = ROI.size.y;
  codestream.apply_input_restrictions( 0, numComponents, 0, 0, reg_ptr );

  blockSize = sizeX * sizeY;
  if (outBuf == NULL) {
    buf = (short *)malloc( blockSize*sizeof(short)*numComponents );
    if (buf == NULL) {
      codestream.destroy();
	  if(jp2Open)
	      jp2_in.close();
      return NULL;
    }
  } else
    buf = outBuf;

  /* Try to read out the "smart" GeoJP2 information.
   */
  bool found = false;
  jp2_input_box smartBox;
  char datetime[DATESIZE];
  double geodoubles[GEODOUBLESIZE];
  unsigned char geoBoxAndHeader[TIFFBOXSIZE + HEADERCHARSIZE];
  unsigned short geokeys[GEOKEYSIZE];
  TIFFTagA tifftags[NUMTIFFTAGS];

  if (jp2Open && smartBox.open( &jp2_ultimate_src )) {
    //PutLog( "KDUReadJP2Mono16 - opened first box\n" );
    if (smartBox.get_box_type() == jp2_uuid_4cc) {
      //PutLog( "KDUReadJP2Mono16 - first box is UUID\n" );
      found = true;
    } else {
      bool openok;
      do {
	smartBox.close();
	openok = smartBox.open_next();
	if (openok) {
	  //PutLog( "KDUReadJP2RGB16 - opened another box\n" );
	  if (smartBox.get_box_type() == jp2_uuid_4cc) {
	    PutLog( "KDUReadJP2RGB16 - found UUID box\n" );
	    found = true;
	  }
	}
      } while (openok == true && found == false);
    }
    if (found == true) {
      double res;
      smartBox.read( geoBoxAndHeader, TIFFBOXSIZE + HEADERCHARSIZE );
      smartBox.read( (unsigned char *)tifftags, sizeof(TIFFTagA) * NUMTIFFTAGS );
      smartBox.read( (unsigned char *)geokeys, sizeof(unsigned short) * GEOKEYSIZE);
      smartBox.read( (unsigned char *)geodoubles, sizeof(double) * GEODOUBLESIZE );
      smartBox.read( (unsigned char *)datetime, sizeof(unsigned char) * DATESIZE);
      smartBox.close();
      utm.left = (long)geodoubles[6];
      utm.top  = (long)geodoubles[7];
      res = geodoubles[0];
      utm.right  = (long)(utm.left + (res * sizeX));
      utm.bottom = (long)(utm.top - (res * sizeY));
      UTCTime( datetime, time64 );
      GetUTMCode( geokeys[15], utm.top, zone );
    } else {
      PutLog( "KDUReadJP2RGB16 - never found UUID box\n" );
      res    = 0;
      time64 = 0;
      utm.left   = 0;
      utm.right  = 0;
      utm.top    = 0;
      utm.bottom = 0;
      if (zone != NULL)
	zone[0] = '\0';
    }
  }
    
  // Extract region directly from the image using kdu_stripe_decompressor
  int preferred_min_stripe_height = 8;
  int absolute_max_stripe_height = 1024;
  int precisions[3] = { precision, precision, precision };
  int stripe_heights[3];
  int max_stripe_heights[3];
  bool signs[3] = { isSigned , isSigned, isSigned };
  int row_gaps[3] = {sizeX, sizeX, sizeX};
  kdu_stripe_decompressor decompressor;

  decompressor.start( codestream, false, false, threadEnvRef, NULL, doubleBufferSize );
  decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
					       absolute_max_stripe_height,
					       stripe_heights, max_stripe_heights );
  bool continues = true;
  short *bufptr = (short *)calloc(3*sizeX*sizeY,sizeof(unsigned short));
  printf("bufptr: %d", bufptr);
  kdu_int16 *stripebufs[3]={(kdu_int16*)bufptr, (kdu_int16*)bufptr+sizeX*sizeY, (kdu_int16*)bufptr+2*sizeX*sizeY};
  stripe_heights[0]=sizeY; stripe_heights[1]=sizeY; stripe_heights[2]=sizeY;
  while (continues) {
    decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
						 absolute_max_stripe_height,
						 stripe_heights, NULL );
    continues = decompressor.pull_stripe( stripebufs, stripe_heights, NULL, 
					  row_gaps, precisions, signs );
	  stripebufs[0] += stripe_heights[0] * sizeX;
	  stripebufs[1] += stripe_heights[1] * sizeX;
	  stripebufs[2] += stripe_heights[2] * sizeX;
  }
  decompressor.finish();
  codestream.destroy();
  if(jp2Open)
	  jp2_in.close();
  if (jp2Open && jp2_ultimate_src.exists())
    jp2_ultimate_src.close();
  if (threadEnv.exists())
    threadEnv.destroy();

  //rewrap the 3 color channels in the right format
  for(int y=0; y!=sizeY; y++)
	  for(int x=0; x!=sizeX; x++){
			*(buf+y*stride+x*3)=*(bufptr+y*sizeX+x);
			*(buf+y*stride+x*3+1)=*((bufptr+sizeX*sizeY)+y*sizeX+x);
			*(buf+y*stride+x*3+2)=*((bufptr+sizeX*sizeY*2)+y*sizeX+x);
	  }
  // Convert to unsigned representation
#if 0
  if (!isSigned) {
    int offset = (1 << precision) >> 1;
    for (int i = 0 ; i < sizeX * sizeY; i++)
      buf[i] = buf[i] + offset;
  }
#endif
  outBuf=buf;
  delete(bufptr);
  return buf;

} /* end KDUReadJP2RGB16buf */

/***************************************************************************
 * function KDUReadJP2Mono16
 *
 * Read a "Smart" JPEG2000 file that has been written with KDUWriteJP2Mono16.
 * We only properly read the GeoJP2 data if it was written by our routine.
 *
 * The output image returned is a signed 16-bit buffer, and the calling
 * routine must deallocate the image when done with it.  On error, the
 * returned pointer is NULL.
 *
 * Inputs:
 * 'filename' is the input file name.
 * 'ROI' is the region of interest.  If all values are 0, the whole image.
 *
 * When ROI is all zeroes, the following outputs are set, otherwise, they
 * are meaningless:
 * 'sizeX' and 'sizeY' are the size of the image in pixels.
 * 'utm' is a RECT containing the upper-left and lower-right corners of the
 *      image to the nearest meter. 
 * 'zone' is the output UTM zone (1 or 2-digit ASCII number and one letter).
 * 'time64' is the output UTC time nugget which indicates the time of the
 *         image, which could be 0 if no time was encoded.
 ***************************************************************************/

short *KDUReadJP2Mono16Buf( FILE *fid, short *outBuf, kdu_long fileOffset, kdu_dims ROI,
								int &sizeX, int &sizeY, double &res, 
								RECT &utm, char zone[], __int64 &time64,
								bool isSigned, int precision, int stride, int resolution )
{
  kdu_embedded_file_source *fileTgt;
  int blockSize;
  int numComponents = 1;
  short *buf;
  jp2_family_src jp2_ultimate_src;
  jp2_source jp2_in;
  kdu_codestream codestream;
  siz_params *siz;
  kdu_thread_env threadEnv;
  kdu_thread_env *threadEnvRef = NULL;


  // Check inputs
  if (precision < 10 || precision > 16)
    return NULL;

  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Set up multi-threaded environment.
  if (numCPUs > 1) {
    int nt = numCPUs;
    threadEnv.create();
    for (int ct = 1; ct < nt; ct++)
      if (!threadEnv.add_thread()) {
	nt = ct; // Unable to create all the threads requested
	PutLog( "KDUReadJP2Mono16 - Tried to create %d threads, but created %d instead\n",
		numCPUs, nt );
      }
    threadEnvRef = &threadEnv;
  }

  /* Open up through KDU and get codestream/size info.
   */
  try {
    fileTgt = new kdu_embedded_file_source( fid, fileOffset );
    jp2_ultimate_src.open( fileTgt );

    //jp2_ultimate_src.open( filename, true );
    if (jp2_in.open( &jp2_ultimate_src ) == false)
      throw 0;
    else {
      if (!jp2_in.read_header()) {
	jp2_in.close();
	return NULL;
      }
    }
  }
  catch (...) {
    // File open failed.
    sizeX = 0;
    sizeY = 0;
    return NULL;
  }

  codestream.create( &jp2_in, threadEnvRef );
  kdu_dims *reg_ptr = NULL;

  /* If all elements of the ROI are 0, get the whole image,
   * otherwise, pull out the requested region.
   */
  if (ROI.pos.x == 0  && ROI.pos.y == 0 &&
      ROI.size.x == 0 && ROI.size.y == 0) {
    siz = codestream.access_siz();
    siz->get( Ssize, 0, 0, ROI.size.y );
    siz->get( Ssize, 0, 1, ROI.size.x );
    reg_ptr = NULL;
  } else
    reg_ptr = &ROI;
  sizeX = ROI.size.x;
  sizeY = ROI.size.y;
  codestream.apply_input_restrictions( 0, numComponents, resolution, 0, reg_ptr );

  blockSize = sizeX * sizeY;
  if (outBuf == NULL) {
    buf = (short *)malloc( blockSize*sizeof(short) );
    if (buf == NULL) {
      codestream.destroy();
      jp2_in.close();
      return NULL;
    }
  } else
    buf = outBuf;

  /* Try to read out the "smart" GeoJP2 information.
   */
  bool found = false;
  jp2_input_box smartBox;
  char datetime[DATESIZE];
  double geodoubles[GEODOUBLESIZE];
  unsigned char geoBoxAndHeader[TIFFBOXSIZE + HEADERCHARSIZE];
  unsigned short geokeys[GEOKEYSIZE];
  TIFFTagA tifftags[NUMTIFFTAGS];

  if (smartBox.open( &jp2_ultimate_src )) {
    //PutLog( "KDUReadJP2Mono16 - opened first box\n" );
    if (smartBox.get_box_type() == jp2_uuid_4cc) {
      //PutLog( "KDUReadJP2Mono16 - first box is UUID\n" );
      found = true;
    } else {
      bool openok;
      do {
	smartBox.close();
	openok = smartBox.open_next();
	if (openok) {
	  //PutLog( "KDUReadJP2Mono16 - opened another box\n" );
	  if (smartBox.get_box_type() == jp2_uuid_4cc) {
	    PutLog( "KDUReadJP2Mono16 - found UUID box\n" );
	    found = true;
	  }
	}
      } while (openok == true && found == false);
    }
    if (found == true) {
      double res;
      smartBox.read( geoBoxAndHeader, TIFFBOXSIZE + HEADERCHARSIZE );
      smartBox.read( (unsigned char *)tifftags, sizeof(TIFFTagA) * NUMTIFFTAGS );
      smartBox.read( (unsigned char *)geokeys, sizeof(unsigned short) * GEOKEYSIZE);
      smartBox.read( (unsigned char *)geodoubles, sizeof(double) * GEODOUBLESIZE );
      smartBox.read( (unsigned char *)datetime, sizeof(unsigned char) * DATESIZE);
      smartBox.close();
      utm.left = (long)geodoubles[6];
      utm.top  = (long)geodoubles[7];
      res = geodoubles[0];
      utm.right  = (long)(utm.left + (res * sizeX));
      utm.bottom = (long)(utm.top - (res * sizeY));
      UTCTime( datetime, time64 );
      GetUTMCode( geokeys[15], utm.top, zone );
    } else {
      PutLog( "KDUReadJP2Mono16 - never found UUID box\n" );
      res    = 0;
      time64 = 0;
      utm.left   = 0;
      utm.right  = 0;
      utm.top    = 0;
      utm.bottom = 0;
      if (zone != NULL)
	zone[0] = '\0';
    }
  }
    
  if (stride==-1) {
    stride = sizeX;
  }

  // Extract region directly from the image using kdu_stripe_decompressor
  int preferred_min_stripe_height = 8;
  int absolute_max_stripe_height = 1024;
  int precisions[1] = { precision };
  int stripe_heights[1];
  int max_stripe_heights[1];
  bool signs[1] = { isSigned };
  int row_gaps[1] = {stride};
  kdu_stripe_decompressor decompressor;

  decompressor.start( codestream, false, false, threadEnvRef, NULL, doubleBufferSize );
  decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
					       absolute_max_stripe_height,
					       stripe_heights, max_stripe_heights );
  bool continues = true;
  short *bufptr = buf;
  while (continues) {
    decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
						 absolute_max_stripe_height,
						 stripe_heights, NULL );
    continues = decompressor.pull_stripe( bufptr, stripe_heights, NULL, NULL,
					  row_gaps, precisions, signs );
    bufptr += stripe_heights[0] * stride;
  }
  decompressor.finish();
  codestream.destroy();
  jp2_in.close();
  if (jp2_ultimate_src.exists())
    jp2_ultimate_src.close();
  if (threadEnv.exists())
    threadEnv.destroy();

  // Convert to unsigned representation
  //if (!isSigned) {
  //  int offset = (1 << precision) >> 1;
  //  for (int i = 0 ; i < sizeX * sizeY; i++)
  //    buf[i] = buf[i] + offset;
  //}
  return buf;

} /* end KDUReadJP2Mono16 */


/***************************************************************************
 * function KDUGetGeoJP2NoData
 *
 * Read the size, time, and georeference information out of a "Smart"
 * JPEG2000 file that has been written with one of the KDUWriteJP2
 * functions above.  We only properly read the GeoJP2 data if it was
 * written by our routine.
 *
 * No image data is returned, just the information about it.
 *
 * Inputs:
 * 'filename' is the input file name.
 *
 * Outputs:
 * 'sizeX' and 'sizeY' are the size of the image in pixels.
 * 'utm' is a RECT containing the upper-left and lower-right corners of the
 *      image to the nearest meter. 
 * 'zone' is the output UTM zone (1 or 2-digit ASCII number and one letter).
 * 'time64' is the output UTC time nugget which indicates the time of the
 *         image, which could be 0 if no time was encoded.
 *
 * Returns 1 on success, or 0 if there is no GeoJP2 information.
 ***************************************************************************/

int KDUGetGeoJP2NoData( char filename[], int &sizeX, int &sizeY,
			   RECT &utm, char zone[], __int64 &time64 )
{
  FILE *fp;
  jp2_family_src jp2_ultimate_src;
  jp2_source jp2_in;
  kdu_codestream codestream;
  siz_params *siz;


  /* Make sure the file can be opened. 
   * KDU is not graceful about non-existent files.
   */
  fp = fopen( filename, "rb" );
  if (fp == NULL)
    return 0;
  fclose( fp );
  jp2_ultimate_src.open( filename, true );

  /* Open up through KDU and get codestream/size info.
   */
  jp2_in.open( &jp2_ultimate_src );
  if (!jp2_in.read_header()) {
    jp2_in.close();
    return 0;
  }
  codestream.create( &jp2_in );
  siz = codestream.access_siz();
  siz->get( Ssize, 0, 0, sizeX );
  siz->get( Ssize, 0, 1, sizeY );

  /* Try to read out the "smart" GeoJP2 information.
   */
  bool found = false;
  jp2_input_box smartBox;
  char datetime[DATESIZE];
  double geodoubles[GEODOUBLESIZE];
  unsigned char geoBoxAndHeader[TIFFBOXSIZE + HEADERCHARSIZE];
  unsigned short geokeys[GEOKEYSIZE];
  TIFFTagA tifftags[NUMTIFFTAGS];
  if (smartBox.open( &jp2_ultimate_src )) {
    //PutLog( "KDUGetGeoJP2NoData - opened first box\n" );
    if (smartBox.get_box_type() == jp2_uuid_4cc) {
      //PutLog( "KDUGetGeoJP2NoData - first box is UUID\n" );
      found = true;
    } else {
      bool openok;
      do {
	smartBox.close();
	openok = smartBox.open_next();
	if (openok) {
	  //PutLog( "KDUGetGeoJP2NoData - opened another box\n" );
	  if (smartBox.get_box_type() == jp2_uuid_4cc) {
	    PutLog( "KDUGetGeoJP2NoData - found UUID box\n" );
	    found = true;
	  }
	}
      } while (openok == true && found == false);
    }
    if (found == true) {
      double res;
      smartBox.read( geoBoxAndHeader, TIFFBOXSIZE + HEADERCHARSIZE );
      smartBox.read( (unsigned char *)tifftags, sizeof(TIFFTagA) * NUMTIFFTAGS );
      smartBox.read( (unsigned char *)geokeys, sizeof(unsigned short) * GEOKEYSIZE);
      smartBox.read( (unsigned char *)geodoubles, sizeof(double) * GEODOUBLESIZE );
      smartBox.read( (unsigned char *)datetime, sizeof(unsigned char) * DATESIZE);
      smartBox.close();
      utm.left = (long)geodoubles[6];
      utm.top  = (long)geodoubles[7];
      res = geodoubles[0];
      utm.right  = (long)(utm.left + (res * sizeX));
      utm.bottom = (long)(utm.top - (res * sizeY));
      UTCTime( datetime, time64 );
      GetUTMCode( geokeys[15], utm.top, zone );
    } else {
      PutLog( "KDUGetGeoJP2NoData - never found UUID box\n" );
      codestream.destroy();
      jp2_in.close();
      return 0;
    }
  }
  codestream.destroy();
  jp2_in.close();
  return 1;

} /* end KDUGetGeoJP2NoData */


				   
/***************************************************************************
 * function KDUDecompressROI
 *
 * JP2 stripe decompressor with ROI.  This is the workhorse routine for
 * CH Phase3 data that has been tiled, so extra work is done here for
 * efficiency.
 ***************************************************************************/
				   
				   
void KDUDecompressROI( kdu_byte *buffer_out, kdu_byte *buffer_com,
			  int bytes_compressed, kdu_dims ROI, int &sizeX, int &sizeY, int res_level )
{
  //int sizeX;
  int stripe_heights[1];
  int max_stripe_heights[1];
  int absolute_max_stripe_height = 1024;
  int preferred_min_stripe_height = 8;
  kdu_dims dims;
  kdu_codestream codestream;
  kdu_stripe_decompressor decompressor;
  kdu_thread_env threadEnv;
  kdu_thread_env *threadEnvRef = NULL;


  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Set up multi-threaded environment.
  if (numCPUs > 1) {
    int nt = numCPUs;
    threadEnv.create();
    for (int ct = 1; ct < nt; ct++)
      if (!threadEnv.add_thread()) {
	nt = ct; // Unable to create all the threads requested
	PutLog( "KDUDecompressROI - Tried to create %d threads, but created %d instead\n",
		numCPUs, nt );
      }
    threadEnvRef = &threadEnv;
  }

  // Create a KDU source buffer in memory to read the stripe.
  kdu_simple_buffer_source input( buffer_com, bytes_compressed );
  try {
    codestream.create( &input, threadEnvRef );
  }
  catch (...) {
    codestream.destroy();
    PutLog( "KDUDecompressROI - Failed to create codestream from input buffer.\n" );
  }
					   
  // Determine number of components to decompress
  codestream.get_dims( 0, dims );
  int num_components = codestream.get_num_components();
  codestream.apply_input_restrictions( 0, num_components, res_level, 0, &ROI );
					   
  kdu_dims output_dims;
  codestream.get_dims( 0, output_dims );
  sizeX = output_dims.size.x;
  sizeY = output_dims.size.y;
    					   
  // Now decompress the image using `kdu_stripe_decompressor'
  decompressor.start( codestream, false, false, threadEnvRef, NULL, doubleBufferSize );
  decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
					       absolute_max_stripe_height,
					       stripe_heights, max_stripe_heights );
  //sizeX = ROI.size.x;
  bool continues = true;
  unsigned char *bufptr = buffer_out;
  try {
    while (continues) {
      decompressor.get_recommended_stripe_heights( preferred_min_stripe_height,
						   absolute_max_stripe_height,
						   stripe_heights, NULL );
      continues = decompressor.pull_stripe( bufptr, stripe_heights );
      bufptr += stripe_heights[0] * sizeX;
    }
  }
  catch (...) {
    PutLog( "KDUDecompressROI - Error during decompression.  Cleaning up and returning whatever we have.\n" );
  }
  decompressor.finish();
  codestream.destroy();
  if (threadEnv.exists())
    threadEnv.destroy();

} /* end KDUDecompressROI */


				   
/***************************************************************************
 * function KDUSimpleDecompress
 *
 * Basic JP2 stripe decompressor.
 ***************************************************************************/
				   
void KDUSimpleDecompress( kdu_byte *buffer_out, kdu_byte *buffer_com,
			     int bytes_compressed )
{
  kdu_dims dims;
  kdu_codestream codestreamD;
  kdu_stripe_decompressor decompressor;


  // Create a KDU source buffer in memory to read the stripe.
  kdu_simple_buffer_source input( buffer_com, bytes_compressed );
  codestreamD.create( &input );
					   
  // Determine number of components to decompress -- simple app only writes PNM
  codestreamD.get_dims( 0, dims );
  int num_components = codestreamD.get_num_components();
  codestreamD.apply_input_restrictions( 0, num_components, 0, 0, NULL );
					   
  // Now decompress the image in one hit, using `kdu_stripe_decompressor'
  decompressor.start( codestreamD );
  int stripe_heights[3]={ dims.size.y, dims.size.y, dims.size.y };
  decompressor.pull_stripe( buffer_out,stripe_heights );
					   
  //printf( "bytes read = %d\n", input.bytes_read );
  decompressor.finish();
  codestreamD.destroy();

} /* end KDUSimpleDecompress */


				   
/***************************************************************************
 * function KDUDecompress
 *
 * Stripe decompressor.  If the return buffer_out is NULL, allocates
 * a buffer large enough to hold the expected decompressed image.  Uses
 * multi-threading if available.  Fills in width, height, precision, and
 * number of channels about input image.
 *
 * Returns NULL on failure, or a buffer pointer on success.
 ***************************************************************************/

kdu_byte *KDUDecompress( kdu_byte *buffer_out, kdu_byte *buffer_com, int bytes_compressed,
			    int &width, int &height, int &prec, int &channels )
{
  int x, y, bpp, com, nbytes;
  bool isCdst = false;
  kdu_byte *buf = NULL;
  kdu_dims  dims;
  kdu_codestream codestream;
  kdu_compressed_source *input = NULL;
  kdu_stripe_decompressor decompressor;
  kdu_thread_env threadEnv;
  kdu_thread_env *threadEnvRef = NULL;
  jp2_family_src jp2_ultimate_src;
  jpx_source jpx_in;
  jpx_layer_source jpx_layer;
  jpx_codestream_source jpx_stream;


  // Redirect the kdu error routine so that kdu errors don't terminate the app.
  kdu_customize_errors( &err_formatter );

  // Set up multi-threaded environment.
  if (numCPUs > 1) {
    int nt = numCPUs;
    threadEnv.create();
    for (int ct = 1; ct < nt; ct++)
      if (!threadEnv.add_thread()) {
	nt = ct; // Unable to create all the threads requested
	PutLog( "KDUDecompress - Tried to create %d threads, but created %d instead\n",
		numCPUs, nt );
      }
    threadEnvRef = &threadEnv;
  }

  // Create a KDU source buffer in memory to read the stripe.
  // The data buffer might just be a codestream, so try to process it like one.
  // This is also the simpler case to recover from, in case it's a file buffer.
  kdu_simple_buffer_source compBuf( (kdu_byte *)buffer_com, bytes_compressed );
  try {
    codestream.create( &compBuf, threadEnvRef );
    isCdst = true;
  }
  catch (...) {
    codestream.destroy();
  }
  kdu_simple_buffer_source compBuf2( (kdu_byte *)buffer_com, bytes_compressed );
  if (isCdst == false) {
    if (threadEnv.exists())
      threadEnv.destroy();
    if (numCPUs > 1) {
      threadEnv.create();
      for (int ct = 1; ct < numCPUs; ct++)
	threadEnv.add_thread();
    }
    try {
      jp2_ultimate_src.open( &compBuf2 );
      if (jpx_in.open( &jp2_ultimate_src, true ) < 0)
	throw 0;
      else {
	jpx_layer = jpx_in.access_layer( 0 );
	if (!jpx_layer) {
	  PutLog( "KDUDecompress - Couldn't find first JPX layer in buffer.\n" );
	  throw 0;
	} else {
	  int cmp, plt, stream_id;
	  jp2_channels channels = jpx_layer.access_channels();
	  channels.get_colour_mapping( 0, cmp, plt, stream_id );
	  jpx_stream = jpx_in.access_codestream( stream_id );
	  input = jpx_stream.open_stream();
	  codestream.create( input, threadEnvRef );
	}
      }
    }
    catch (...) {
      // Both methods failed, bail.
      PutLog( "KDUDecompress - Couldn't open JPEG2000 buffer as a codestream or a file buffer\n" );
      if (threadEnv.exists())
	threadEnv.destroy();
      return NULL;
    }
  }

  // Allocate buffer, if necessary.
  siz_params *siz = codestream.access_siz();
    siz->get( Ssize, 0, 0, y );
    siz->get( Ssize, 0, 1, x );
    siz->get( Sprecision, 0, 0, bpp );
    siz->get( Scomponents, 0, 0, com );
    if (bpp <= 8)
      nbytes = 1;
    else nbytes = 2;
    if (com == 3)
      nbytes *= 3;
  if (buffer_out == NULL) {
    buf = (kdu_byte *)malloc( x * y * nbytes );
    if (buf == NULL) {
      codestream.destroy();
      return NULL;
    }
  } else
    buf = buffer_out;

  width = x;
  height = y;
  prec = bpp;
  channels = com;

  // Now decompress the image in one hit, using `kdu_stripe_decompressor'
  codestream.get_dims( 0, dims );
  decompressor.start( codestream, false, false, threadEnvRef, NULL, doubleBufferSize );
  int stripe_heights[3]={ dims.size.y, dims.size.y, dims.size.y };
  decompressor.pull_stripe( buf, stripe_heights );
					   
  decompressor.finish();
  codestream.destroy();
  if (threadEnv.exists())
    threadEnv.destroy();

  return buf;

} /* end KDUDecompress */



/***************************************************************************
 * function KDUSimpleCompress
 *
 * Basic JP2 stripe compressor.
 *
 * Returns the actual byte size of the compressed codestream, which may be
 * different from the target bytes_compressed that was supplied.
 ***************************************************************************/

kdu_long KDUSimpleCompress( kdu_byte *buffer_in, int width, int height, 
			       kdu_byte *buffer_com, int bytes_compressed )
{
  int num_components = 1;
  siz_params siz;
  kdu_params *siz_ref = &siz;
  kdu_codestream codestream;
  kdu_stripe_compressor compressor;


  // Construct code-stream object
  siz.set( Scomponents, 0, 0, num_components );
  siz.set( Sdims, 0, 0, height );  // Height of first image component
  siz.set( Sdims, 0, 1, width );   // Width of first image component
  siz.set( Sprecision, 0, 0, 8 );  // Image samples have original bit-depth of 8
  siz.set( Ssigned, 0, 0, false ); // Image samples are originally unsigned
  siz_ref->finalize();
  hsdMemoryTarget output = hsdMemoryTarget( buffer_com, bytes_compressed );
  codestream.create( &siz, &output );
	
  // Set up any specific coding parameters and finalize them.
  codestream.access_siz()->parse_string( "Clayers=12" );
  codestream.access_siz()->parse_string( "Creversible=yes" );
  codestream.access_siz()->finalize_all(); // Set up coding defaults
	
  // Now compress the image in one hit, using `kdu_stripe_compressor'
  compressor.start( codestream );
  int stripe_heights[3]={ height, height, height };
  compressor.push_stripe( buffer_in, stripe_heights );
  compressor.finish();
  codestream.destroy();

  kdu_long actualSize = output.getSize();
  return actualSize;

} /* end KDUSimpleCompress */



/*****************************************************************************
 * STATIC copy_block
 *
 * Helper function for copy_tile
 *****************************************************************************/

static void copy_block(kdu_block *in, kdu_block *out)
{
  if (in->K_max_prime != out->K_max_prime)
    { kdu_error e; e << "Cannot copy blocks belonging to subbands with "
      "different quantization parameters."; }
  assert(!(out->transpose || out->vflip || out->hflip));
  kdu_coords size = in->size;
  if (in->transpose) size.transpose();
  if ((size.x != out->size.x) || (size.y != out->size.y))  
    { kdu_error e; e << "Cannot copy code-blocks with different dimensions."; }
  out->missing_msbs = in->missing_msbs;
  if (out->max_passes < (in->num_passes+2))      // Gives us enough to round up
    out->set_max_passes(in->num_passes+2,false); // to the next whole bit-plane
  out->num_passes = in->num_passes;
  int num_bytes = 0;
  for (int z=0; z < in->num_passes; z++) {
    num_bytes += (out->pass_lengths[z] = in->pass_lengths[z]);
    out->pass_slopes[z] = in->pass_slopes[z];
  }

  // Just copy compressed code-bytes.  No need for block transcoding.
  if (out->max_bytes < num_bytes)
      out->set_max_bytes(num_bytes,false);
  memcpy(out->byte_buffer,in->byte_buffer,(size_t) num_bytes);

} // end copy_block



/*****************************************************************************
 * STATIC copy_tile
 *
 * Helper function for KDUGetRegionJPGServer
 *****************************************************************************/

static void
  copy_tile(kdu_tile tile_in, kdu_tile tile_out, int tnum_in, int tnum_out,
            kdu_params *siz_in, kdu_params *siz_out, int skip_components,
            int &num_blocks, int discard_levels)
  /* Although there could be more efficient ways of doing this (in terms of
     saving memory), we currently just walk through all code-blocks in the
     most obvious order, copying them from the input to the output tile.
     Note that the main tile-header coding parameters should have been
     copied already, but this function will copy POC parameters for
     non-initial tile-parts, wherever the information has not already
     been substituted for the purpose of rearranging the packet sequence
     during transcoding. */
{
  int num_components = tile_out.get_num_components();
  int new_tpart=0, next_tpart = 1;

  for (int c=0; c < num_components; c++) {
    kdu_tile_comp comp_in, comp_out;
    comp_in = tile_in.access_component(c+skip_components);
    comp_out = tile_out.access_component(c);
    int num_resolutions = comp_out.get_num_resolutions();
    for (int r=0; r < num_resolutions - discard_levels; r++) {
      kdu_resolution res_in;  res_in = comp_in.access_resolution(r);
      kdu_resolution res_out; res_out = comp_out.access_resolution(r);
      int min_band = (r==0)?0:1;
      int max_band = (r==0)?0:3;
      for (int b=min_band; b <= max_band; b++) {
	kdu_subband band_in;  band_in = res_in.access_subband(b);
	kdu_subband band_out; band_out = res_out.access_subband(b);
	kdu_dims blocks_in;  band_in.get_valid_blocks(blocks_in);
	kdu_dims blocks_out; band_out.get_valid_blocks(blocks_out);
	if ((blocks_in.size.x != blocks_out.size.x) ||
	    (blocks_in.size.y != blocks_out.size.y))
	  { kdu_error e; e << "Transcoding operation cannot proceed: "
			   "Code-block partitions for the input and output "
			   "code-streams do not agree."; }
	kdu_coords idx;
	for (idx.y=0; idx.y < blocks_out.size.y; idx.y++)
	  for (idx.x=0; idx.x < blocks_out.size.x; idx.x++) {
	    kdu_block *in =
	      band_in.open_block(idx+blocks_in.pos,&new_tpart);
	    for (; next_tpart <= new_tpart; next_tpart++)
	      siz_out->copy_from(siz_in,tnum_in,tnum_out,next_tpart,
				 skip_components);
	    kdu_block *out = band_out.open_block(idx+blocks_out.pos);

	    //                    printf("[%i %i]   [%i %i]\n", in->size.x, in->size.y, out->size.x, out->size.y);
	    copy_block(in,out);
	    band_in.close_block(in);
	    band_out.close_block(out);
	    num_blocks++;
	  }
      }
    }
  }
} // end copy_tile



/***************************************************************************
 * function KDUGetRegionJPGServer
 *
 * This is a specialized routine to support the JPG Server.
 * When the JPG server goes to read a file, it has been requested to either
 * return a compressed, uncompressed, or "best" tile. 
 *
 * Compressed tiles are requested when the link between the viewer and
 * server is slow - in this case, the CPU time required to recompress
 * the tile is smaller than the time to transmit the larger tile.
 * The 'compress' parameter will come in with a value of 5 when this
 * is the case.  After decompressing the ROI, we re-stripe compress it.
 *
 * Uncompressed tiles are requested when the link between the viewer and
 * server is fast - in this case, the 'compress' parameter will come in as
 * 0 for no compression.  We send back an uncompressed tile.
 *
 * When the "best" method is selected, we determine if the JP2 file being
 * read is tiled, if so, then it is more efficient to transcode the requested
 * ROI, so we will do that, and change the compress flag to 1 to indicate
 * that we are returning a raw codestream so the viewer knows how to
 * decompress it.  Otherwise, we will return an uncompressed tile.
 ***************************************************************************/

unsigned char *KDUGetRegionJPGServer( char *fullPath, kdu_dims ROI,
									 int &bufSize, short &compress )
{
	int tilesize_x, tilesize_y;
	unsigned char *buf;
	unsigned char *comp = NULL;
	siz_params *siz_in;
	jp2_source  jp2_in;
	jp2_family_src jp2_ultimate_src;
	kdu_codestream codestream;
	kdu_thread_env threadEnv;
	kdu_thread_env *threadEnvRef = NULL;


	// Redirect the kdu error routine so that kdu errors don't terminate the app.
	kdu_customize_errors( &err_formatter );

	// Open the file and read the header to determine if this JP2 is tiled or not.
	try {
		jp2_ultimate_src.open( fullPath, true );
		if (!jp2_in.open( &jp2_ultimate_src )) {
			jp2_ultimate_src.close();
			throw 0;
		} else {
			if (!jp2_in.read_header()) {
				jp2_in.close();
				jp2_ultimate_src.close();
				throw 0;
			}
			codestream.create( &jp2_in, threadEnvRef );
			int numComponents = codestream.get_num_components();
			siz_in = codestream.access_siz();
			siz_in->get( Stiles, 0, 1, tilesize_x );
			siz_in->get( Stiles, 0, 0, tilesize_y );

			// If the tile sizes are larger than reasonable, then there is
			// no tiling advantage.  Do whatever the compression flag asks for.
			if (compress != -1 || (tilesize_x > 1024 && tilesize_y > 1024)) {

				buf = (unsigned char *)malloc( ROI.size.x * ROI.size.y );
				if (buf == NULL) {
					jp2_in.close();
					return NULL;
				}

				if (compress == -1)
					compress = 0;
				if (compress == 0) {
					// No compression
					bufSize = ROI.size.x * ROI.size.y;
				} else {
					// Re-compressing tile after extracting ROI.
					bufSize = (ROI.size.x * ROI.size.y) / compress;
					comp = (unsigned char *)malloc( bufSize );
					if (comp == NULL) {
						free( buf );
						jp2_in.close();
						return NULL;
					}
				}

				// Set up multi-threaded environment.
				if (numCPUs > 1) {
					int nt = numCPUs;
					threadEnv.create();
					for (int ct = 1; ct < nt; ct++)
						if (!threadEnv.add_thread()) {
							nt = ct; // Unable to create all the threads requested
							PutLog( "KDUGetRegionJPGServer - Tried to create %d threads, but created %d instead\n",
								numCPUs, nt );
						}
						threadEnvRef = &threadEnv;
				}

				// Now decompress the image using `kdu_stripe_decompressor'
				codestream.apply_input_restrictions( 0, numComponents, 0, 0, &ROI );
				int stripe_heights[3]={ ROI.size.y, ROI.size.y, ROI.size.y };
				kdu_stripe_decompressor decompressor;
				decompressor.start( codestream, false, false, threadEnvRef, NULL, doubleBufferSize );
				decompressor.pull_stripe( buf, stripe_heights );

				decompressor.finish();
				codestream.destroy();
				if (threadEnv.exists())
					threadEnv.destroy();
				jp2_in.close();

				if (compress == 0) {
					// For no re-compression, we are done.
					return buf;
				} else {
					// For re-compressed image, do that here.
					KDUSimpleCompress( buf, ROI.size.x, ROI.size.y, comp, bufSize );
					free( buf );
					return comp;
				}

			} else {

				int dataSize;
				int discard_levels = 0;
				int skip_components = 0;
				int min_tile_x, min_tile_y, max_tile_x, max_tile_y;
				kdu_dims tile_indices_in;

				// Create a codestream and transcode only the tiles that comprise the
				// ROI into that codestream.
				codestream.get_valid_tiles( tile_indices_in );
				min_tile_x = ROI.pos.x / tilesize_x;
				min_tile_y = ROI.pos.y / tilesize_y;

				max_tile_x = (ROI.pos.x + ROI.size.x) / tilesize_x;
				max_tile_y = (ROI.pos.y + ROI.size.y) / tilesize_y;

				if (max_tile_y > tile_indices_in.size.y - 1)
					max_tile_y = tile_indices_in.size.y - 1;
				if (max_tile_x > tile_indices_in.size.x - 1)
					max_tile_x = tile_indices_in.size.x - 1;
				if (min_tile_x < 0)
					min_tile_x = 0;
				if (min_tile_y < 0)
					min_tile_y = 0;

				// Use 1 byte per pixel for maximum output buffer size
				dataSize = tilesize_x * tilesize_y *
					(max_tile_x - min_tile_x + 1) * (max_tile_y - min_tile_y + 1);

				// Create the output codestream object.
				buf = (unsigned char *)malloc( dataSize );
				if (buf == NULL) {
					jp2_in.close();
					return NULL;
				}
				hsdMemoryTarget output( buf, dataSize );

				siz_params siz;
				siz.copy_from( siz_in, -1, -1, -1, skip_components, discard_levels, 0, 0, 0 );
				siz.set( Scomponents, 0, 0, codestream.get_num_components() );

				kdu_codestream codestream_out;
				codestream_out.create( &siz, &output );
				codestream_out.share_buffering( codestream );
				siz_params *siz_out = codestream_out.access_siz();
				siz_out->copy_from( siz_in, -1, -1, -1, skip_components, discard_levels, 0, 0, 0 );

				codestream_out.access_siz()->finalize_all( -1 );

				// Set up rate control variables
				kdu_long max_bytes = KDU_LONG_MAX;

				kdu_params *cod = siz_out->access_cluster( COD_params );
				int total_layers;
				cod->get( Clayers, 0, 0, total_layers );
				kdu_long *layer_bytes = new kdu_long[total_layers];
				int nel, non_empty_layers = 0;

				// Now ready to perform the transfer of compressed data between streams
				kdu_dims tile_indices_out;
				codestream_out.get_valid_tiles( tile_indices_out );
				assert( (tile_indices_in.size.x == tile_indices_out.size.x) &&
					(tile_indices_in.size.y == tile_indices_out.size.y) );
				int num_blocks = 0;

				kdu_coords idx;
				for (idx.y = min_tile_y; idx.y <= max_tile_y; idx.y++){
					for (idx.x = min_tile_x; idx.x <= max_tile_x; idx.x++) {
						kdu_tile tile_in = codestream.open_tile( idx + tile_indices_in.pos );
						int tnum_in = tile_in.get_tnum();
						int tnum_out = (idx.x) + (idx.y) * tile_indices_out.size.x;
						siz_out->copy_from( siz_in, tnum_in, tnum_out, 0, skip_components,
							discard_levels, 0, 0, 0 );
						siz_out->finalize_all( tnum_out );

						/* Note carefully: we must not open the output tile without
						first copying any tile-specific code-stream parameters, as
						above.  It is tempting to do this. */
						kdu_tile tile_out = codestream_out.open_tile( idx + tile_indices_out.pos );
						assert( tnum_out == tile_out.get_tnum() );

						copy_tile( tile_in, tile_out, tnum_in, tnum_out, siz_in, siz_out,
                                   skip_components, num_blocks, 0 );
						tile_in.close();
						tile_out.close();
					}
				}

				// Generate the output code-stream
				if (codestream_out.ready_for_flush()) {
					nel = codestream_out.trans_out( max_bytes, layer_bytes, total_layers );
					non_empty_layers = (nel > non_empty_layers) ? nel : non_empty_layers;
				}
				if (non_empty_layers > total_layers)
					non_empty_layers = total_layers; // Can happen if a tile has more layers

				bufSize  = (int)codestream_out.get_total_bytes();
				compress = 1; // Raw codestream

				// Cleanup
				codestream_out.destroy();
				codestream.destroy();
				jp2_in.close();
				delete[] layer_bytes;

				return buf;

			} // end tiled processing
		}
	}
	catch (...) {
		PutLog( "KDUGetRegionJPGServer - Couldn't open JPEG2000 file: %s\n", fullPath );
		return NULL;
	}

	return NULL;

} /* end KDUGetRegionJPGServer */



/***************************************************************************
 * function KDUGetRegionCondorServer
 *
 * This is a specialized routine to support the JPG Server.
 * When the JPG server goes to read a file, it has been requested to either
 * return a compressed, uncompressed, or "best" tile. 
 *
 * Compressed tiles are requested when the link between the viewer and
 * server is slow - in this case, the CPU time required to recompress
 * the tile is smaller than the time to transmit the larger tile.
 * The 'compress' parameter will come in with a value of 5 when this
 * is the case.  After decompressing the ROI, we re-stripe compress it.
 *
 * Uncompressed tiles are requested when the link between the viewer and
 * server is fast - in this case, the 'compress' parameter will come in as
 * 0 for no compression.  We send back an uncompressed tile.
 *
 * When the "best" method is selected, we determine if the JP2 file being
 * read is tiled, if so, then it is more efficient to transcode the requested
 * ROI, so we will do that, and change the compress flag to 1 to indicate
 * that we are returning a raw codestream so the viewer knows how to
 * decompress it.  Otherwise, we will return an uncompressed tile.
 ***************************************************************************/

unsigned char *KDUGetRegionCondorServer( FILE *fid, kdu_long fileOffset, kdu_dims ROI, std::vector<unsigned char> &outbuf, int res_level )
{
	
	int tilesize_x, tilesize_y;
	unsigned char *buf;
	unsigned char *comp = NULL;
	siz_params *siz_in;
	jp2_source  jp2_in;
	jp2_family_src jp2_ultimate_src;
	kdu_codestream codestream;
	kdu_thread_env threadEnv;
	kdu_thread_env *threadEnvRef = NULL;


	// Redirect the kdu error routine so that kdu errors don't terminate the app.
	kdu_customize_errors( &err_formatter );

	// Open the file and read the header to determine if this JP2 is tiled or not.
	try {
        std::auto_ptr < kdu_embedded_file_source> fileTgt(new kdu_embedded_file_source( fid, fileOffset ) );

		jp2_ultimate_src.open( fileTgt.get() );
		if (!jp2_in.open( &jp2_ultimate_src )) {
			jp2_ultimate_src.close();
			throw 0;
		} else {
			if (!jp2_in.read_header()) {
				jp2_in.close();
				jp2_ultimate_src.close();
				throw 0;
			}
			codestream.create( &jp2_in, threadEnvRef );
			int numComponents = codestream.get_num_components();
			siz_in = codestream.access_siz();
			siz_in->get( Stiles, 0, 1, tilesize_x );
			siz_in->get( Stiles, 0, 0, tilesize_y );

			int dataSize;
			int discard_levels = 0;

			int skip_components = 0;
			int min_tile_x, min_tile_y, max_tile_x, max_tile_y;
			kdu_dims tile_indices_in;

			// Create a codestream and transcode only the tiles that comprise the
			// ROI into that codestream.
			codestream.get_valid_tiles( tile_indices_in );
			min_tile_x = ROI.pos.x / tilesize_x;
			min_tile_y = ROI.pos.y / tilesize_y;

			max_tile_x = (ROI.pos.x + ROI.size.x) / tilesize_x;
			max_tile_y = (ROI.pos.y + ROI.size.y) / tilesize_y;

			if (max_tile_y > tile_indices_in.size.y - 1)
				max_tile_y = tile_indices_in.size.y - 1;
			if (max_tile_x > tile_indices_in.size.x - 1)
				max_tile_x = tile_indices_in.size.x - 1;
			if (min_tile_x < 0)
				min_tile_x = 0;
			if (min_tile_y < 0)
				min_tile_y = 0;

			// Use 1 byte per pixel for maximum output buffer size
			dataSize = tilesize_x * tilesize_y *
				(max_tile_x - min_tile_x + 1) * (max_tile_y - min_tile_y + 1);

			// Create the output codestream object.
			outbuf.resize(dataSize);
			buf = &(outbuf[0]);
			hsdMemoryTarget output( buf, dataSize );

			siz_params siz;
			siz.copy_from( siz_in, -1, -1, -1, skip_components, discard_levels, 0, 0, 0 );
			siz.set( Scomponents, 0, 0, codestream.get_num_components() );

			kdu_codestream codestream_out;
			codestream_out.create( &siz, &output );
			codestream_out.share_buffering( codestream );
			siz_params *siz_out = codestream_out.access_siz();
			siz_out->copy_from( siz_in, -1, -1, -1, skip_components, discard_levels, 0, 0, 0 );

			codestream_out.access_siz()->finalize_all( -1 );

			// Set up rate control variables
			kdu_long max_bytes = KDU_LONG_MAX;

			kdu_params *cod = siz_out->access_cluster( COD_params );
			int total_layers;
			cod->get( Clayers, 0, 0, total_layers );
			kdu_long *layer_bytes = new kdu_long[total_layers];
			int nel, non_empty_layers = 0;

			// Now ready to perform the transfer of compressed data between streams
			kdu_dims tile_indices_out;
			codestream_out.get_valid_tiles( tile_indices_out );
			assert( (tile_indices_in.size.x == tile_indices_out.size.x) &&
				(tile_indices_in.size.y == tile_indices_out.size.y) );
			int num_blocks = 0;

			kdu_coords idx;
			for (idx.y = min_tile_y; idx.y <= max_tile_y; idx.y++){
				for (idx.x = min_tile_x; idx.x <= max_tile_x; idx.x++) {
					kdu_tile tile_in = codestream.open_tile( idx + tile_indices_in.pos );
					int tnum_in = tile_in.get_tnum();
					int tnum_out = (idx.x) + (idx.y) * tile_indices_out.size.x;
					siz_out->copy_from( siz_in, tnum_in, tnum_out, 0, skip_components,
						discard_levels, 0, 0, 0 );
					siz_out->finalize_all( tnum_out );

					/* Note carefully: we must not open the output tile without
					first copying any tile-specific code-stream parameters, as
					above.  It is tempting to do this. */
					kdu_tile tile_out = codestream_out.open_tile( idx + tile_indices_out.pos );
					assert( tnum_out == tile_out.get_tnum() );

					copy_tile( tile_in, tile_out, tnum_in, tnum_out, siz_in, siz_out,
                               skip_components, num_blocks, res_level );
					tile_in.close();
					tile_out.close();
				}
			}

			// Generate the output code-stream
			if (codestream_out.ready_for_flush()) {
				nel = codestream_out.trans_out( max_bytes, layer_bytes, total_layers );
				non_empty_layers = (nel > non_empty_layers) ? nel : non_empty_layers;
			}
			if (non_empty_layers > total_layers)
				non_empty_layers = total_layers; // Can happen if a tile has more layers

			outbuf.resize((int)codestream_out.get_total_bytes());

			// Cleanup
			codestream_out.destroy();
			codestream.destroy();
			jp2_in.close();
			delete[] layer_bytes;

			return buf;
		}
	}
	catch (...) {
		PutLog( "KDUGetRegionJPGServer - Couldn't read previously opened JPEG2000 file\n" );
		return NULL;
	}

	return NULL;

} /* end KDUGetRegionJPGServer */
