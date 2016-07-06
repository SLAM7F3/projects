/*
 *  tile.cpp
 *  data_viewer
 *
 *  Created by Ross Anderson on 1/31/06.
 *  Copyright 2006 MIT Lincoln Laboratory. All rights reserved.
 *
 */

#include "astro_geo/latlong2utmfuncs.h"

#include <config.h>

#include <stdio.h>
#include <limits>
#include <iostream>
#include <string>

// !!! use config files...
#include <libtdp/tdp.h>
#include <libtdp/alirt.conf.h>
#include <libtdp/alirt2.conf.h>
#include <libtdp/point_data.conf.h>

#include <osg/Notify>
#include <osg/io_utils>
#include <osg/Math>
#include <osg/BoundingBox>
#include <osg/Array>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>

#include <model/CoordinateSpaceConverter.h>
#include <model/HyperBoundingBox.h>
#include <model/BoundingBoxUtils.h>

#include <util/utm.h>
#include <util/tdp_utils.h>

#define DIRECTORY_SEPARATOR '/'

using namespace std;

typedef enum {
   TypeUint16,
   TypeUint32,
   TypeReal32,
   TypeUndefined
} TypeEnumeration;

class TdpDataType {
  public:
		
   TdpDataType() :
      valid(false),
      key(0),
      name("void"),
      type(TypeUndefined)
      {}
		
   TdpDataType( unsigned int k, std::string n, TypeEnumeration t ) :
      valid(true),
      key(k),
      name(n),
      type(t)
      {}
		
   TdpDataType( const TdpDataType& mdt ) :
      valid(mdt.valid),
      key(mdt.key),
      name(mdt.name),
      type(mdt.type)
      {}
		
   std::string typeDescription() const {
      switch( type ) {
         case TypeUint16: return "uint16";
         case TypeUint32: return "uint32";
         case TypeReal32: return "real32";
         default: break;
      }
      return "undefined";
   }
		
   unsigned int elementSizeBytes() const {
      switch( type ) {
         case TypeUint16: return 2;
         case TypeUint32: return 4;
         case TypeReal32: return 4;
         default: break;
      }
      return 1;
   }
		
   ~TdpDataType() {}
		
   bool					valid;
   unsigned int			key;
   std::string				name;
   TypeEnumeration			type;
};

typedef std::map< unsigned int, TdpDataType >	TdpDataTypeMap;


class FileInfo {
  public:
		
   std::string				path;
   osg::BoundingBox		unified_extent;
   osg::Matrixd			local_to_unified;
};

/** Abstract base clas for tiles of different types. */
class Tile {
  public:
		
   Tile() :
      has_been_flushed_to_disk(false)
      {}
		
   virtual ~Tile() {}
		
   /** Tests if this tile's n-gon intersects the 3D point in X and Y only. */
   virtual bool contains( const osg::Vec3f& point ) = 0;

   typedef std::map< unsigned int, osg::ref_ptr<osg::Array> >	ArrayMap;
	
   bool									has_been_flushed_to_disk;
   unsigned int							xi;
   unsigned int							yi;
   std::string								filename;
   osg::BoundingBox						bounds;
   osg::BoundingBox						extent;
   osg::ref_ptr<osg::Vec3Array>			vertices;
   ArrayMap								metadata;
};

class TileBox : public Tile
{
  public:
		
   TileBox( const osg::BoundingBox& bb ) :
      Tile()
      {
         bounds = bb;
      }
		
   virtual ~TileBox() {}
				
   /** Tests if this tile's n-gon intersects the 3D point in X and Y only. */
   bool contains( const osg::Vec3f& point ) {
      return bounds.contains( point );
   }
	
};

class Tile2DPoly : public Tile
{
  public:
	
   Tile2DPoly() :
      Tile()
      {
         bounds.init();
         bounds.zMin() = -FLT_MAX;
         bounds.zMax() = FLT_MAX;
      }
		
   virtual ~Tile2DPoly() {}
		
   /** Append the 2D point to the Ngon. Don't forget to close the loop. */
   void add( const osg::Vec2f& point ) {
      poly_points.push_back( point );
      bounds.expandBy( point.x(), point.y(), 0.0f );
   }
		
   /** Tests if this tile's n-gon intersects the 3D point in X and Y only. */
   bool contains( const osg::Vec3f& point ) {
      /* Adapted from: http://astronomy.swin.edu.au/~pbourke/geometry/insidepoly/ */
      std::vector< osg::Vec2f >	poly = poly_points;
      if ( poly_points.size() ) poly.push_back( poly_points.front() );	// close the polygon
			
      bool isInside = false;
      unsigned int i, j;
      unsigned int n = poly.size();
			
      for( i = 0, j = n-1; i < n; j = i++ ) {
         if ( ( ( (poly[i].y() <= point.y()) && (point.y() < poly[j].y()) ) ||
                ( (poly[j].y() <= point.y()) && (point.y() < poly[i].y()) ) ) &&
              ( point.x() < (poly[j].x() - poly[i].x()) * (point.y() - poly[i].y()) / (poly[j].y() - poly[i].y()) + poly[i].x()) )
         {
            isInside = !isInside;
         }
      }
			
      return isInside;
   }
	
  protected:
			
   std::vector<osg::Vec2f>					poly_points;
};

class TileCircle : public Tile
{
  public:
		
   TileCircle( const osg::Vec2d& center, const double radius ) :
      Tile(),
      _center( center ),
      _radius2( radius*radius )
      {
         osg::Vec2d  delta( radius, radius );
         bounds.set( osg::Vec3d( center-delta, -FLT_MAX ), osg::Vec3d( center+delta, FLT_MAX ) );
      }
		
   virtual ~TileCircle() {}
				
   /** Tests if this tile's circle contains the 3D point in X and Y only. */
   bool contains( const osg::Vec3f& point ) {
      return ( osg::Vec2d( point.x(), point.y() ) - _center ).length2() < _radius2;
   }
	
  protected:
        
   osg::Vec2d  _center;
   double      _radius2;
};

void alignBoundingBox( osg::BoundingBox& bb, const float align )
{
   bb.xMin() = floor( bb.xMin() / align ) * align;
   bb.yMin() = floor( bb.yMin() / align ) * align;
    
   bb.xMax() = ceil( bb.xMax() / align ) * align;
   bb.yMax() = ceil( bb.yMax() / align ) * align;
}


int main (int argc, char * argv[])
{
   // use an ArgumentParser object to manage the program arguments.
   osg::ArgumentParser arguments(&argc, argv);
    
    // set up the usage document, in case we need to print out how to use this program.
   arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
   arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" sorts TDP point cloud swaths into x-y plane tiles");
   arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] input_file [input_file] ...");
	
   arguments.getApplicationUsage()->addCommandLineOption( "-v", "Overlap distance between tiles, in meters" );
   arguments.getApplicationUsage()->addCommandLineOption( "-n", "X by Y tile count (i.e. -n 4 4)" );
   arguments.getApplicationUsage()->addCommandLineOption( "-s", "X by Y tile size, in meters (i.e. -s 5 5)" );
   arguments.getApplicationUsage()->addCommandLineOption( "--crop", "Crop by bounding box (i.e. --crop xmin ymin xmax ymax)" );
   arguments.getApplicationUsage()->addCommandLineOption( "--crop-z", "Crop by a Z range (i.e. --crop-z zmin zmin)" );
   arguments.getApplicationUsage()->addCommandLineOption( "--crop-meta", "Crop by a metadata value (i.e. --crop-meta tdp_key_num min min)" );
   arguments.getApplicationUsage()->addCommandLineOption( "--circle", "Crop by a circle (i.e. --circle x y radius)" );
   arguments.getApplicationUsage()->addCommandLineOption( "--poly", "Crop by polygon from file (i.e. --poly shape.txt)" );
   arguments.getApplicationUsage()->addCommandLineOption( "--ll", "Specify coordinates in lat/long (vs. cartesian)" );
   arguments.getApplicationUsage()->addCommandLineOption( "-b", "Base file name (i.e. BASE.x0.y0.tdp)" );
   arguments.getApplicationUsage()->addCommandLineOption( "-o", "Directory path for output files" );
   arguments.getApplicationUsage()->addCommandLineOption( "-t", "Directory path for temporary files" );
   arguments.getApplicationUsage()->addCommandLineOption( "-a", "Align tiles to the given increment" );
   arguments.getApplicationUsage()->addCommandLineOption( "--no-rotation", "Force the tiles to have no rotation" );
   arguments.getApplicationUsage()->addCommandLineOption( "--heading-space", "Always produce tiles where the X axis is the plane's heading (not georeferenced)" );
   arguments.getApplicationUsage()->addCommandLineOption( "-h or --help", "Print this help" );

   // if user request help write it out to cout.
   if (arguments.read("-h") || arguments.read("--help") || arguments.argc()<=1)
   {
      arguments.getApplicationUsage()->write(std::cout);
		
      std::cout << std::endl;
      std::cout << "Sample: tile -o tile_output_dir -b boston -n 8 4 input_dir/*.tdp" << std::endl;
      std::cout << std::endl;
      std::cout << "When using the crop or poly mode, you must specify coordinates relative to " << std::endl;
      std::cout << "the local UTM zone, not in any offset space. Or, specify in global lat/long and " << std::endl;
      std::cout << "use the --ll flag. Polygons are described by a whitespace delimited text file, in " << std::endl;
      std::cout << "East/North/Alt triplets in UTM or Long/Lat/Alt triplets with --ll. These files " << std::endl;
      std::cout << "can also be used with the marker tool. The altitudes given in this file are ignored." << std::endl;
      std::cout << std::endl;
      std::cout << "The cropping modes --crop-z and --crop-meta are independant of the output tiles. Points" << std::endl;
      std::cout << "falling outside the given crop ranges are filtered out before being written to a tile." << std::endl;
      std::cout << std::endl;
      std::cout << "Version 1.0, written by Ross Anderson" << std::endl;
      std::cout << "Copyright 2006 MIT Lincoln Laboratory" << std::endl;
      std::cout << std::endl;
      return 1;
   }
	
   // check for argument consistency
   osg::Vec2f			tile_size;
   unsigned int		x_tiles, y_tiles;
   osg::Vec3d			crop_box_range[2];
   osg::Vec3d          circle_center;
   double              circle_radius;
   std::string			crop_poly_filename;
   float               alignment = 0.0f;
   float               crop_z_min, crop_z_max;
   unsigned int        crop_meta_key;
   float               crop_meta_min, crop_meta_max;
    
    // initialize
   crop_box_range[0].set( -FLT_MAX, -FLT_MAX, -FLT_MAX );
   crop_box_range[1].set( FLT_MAX, FLT_MAX, FLT_MAX );
	
   bool	convert_to_utm = arguments.read( "--ll" );
   bool	calc_tile_size = arguments.read( "-n", x_tiles, y_tiles );
   bool	calc_tile_num = arguments.read( "-s", tile_size.x(), tile_size.y() );
   bool	crop_by_box = arguments.read( "--crop", crop_box_range[0].x(), crop_box_range[0].y(), crop_box_range[1].x(), crop_box_range[1].y() );
   bool	crop_by_z = arguments.read( "--crop-z", crop_z_min, crop_z_max );
   bool	crop_by_meta = arguments.read( "--crop-meta", crop_meta_key, crop_meta_min, crop_meta_max );
   bool	crop_by_circle = arguments.read( "--circle", circle_center.x(), circle_center.y(), circle_radius );
   bool	crop_by_poly = arguments.read( "--poly", crop_poly_filename );
   bool	no_rotation = arguments.read( "--no-rotation" );
   bool	heading_align_tiles = arguments.read( "--heading-space" );
    
   arguments.read( "-a", alignment );

   if ( ( (int)calc_tile_size + calc_tile_num + crop_by_box + crop_by_poly + crop_by_circle ) != 1 ) {
      arguments.getApplicationUsage()->write(std::cout);
		
      std::cout << "You must specify exactly one argument from the group -s, -n, --crop, --circle, or --poly. They are mutually exclusive." << std::endl;
      std::cout << std::endl;
      return 1;
   }

   // quiet the TDP library, because we will run into warnings under correct operation
   Tdp_debug::show_warnings( false );
   Tdp_debug::show_errors( false );

	// command line options
   float			overlap = 0.0f;
   std::string		base_name = "tile";
	
   arguments.read("-v", overlap);
   arguments.read("-b", base_name);
	
   overlap += 0.0001f;	// add a small delta value to avoid edge cases of the bounding box
    
   // Setup paths
   std::string output_path;
   std::string	temp_path;

    // read command line args
   if ( arguments.read("-o", output_path) ) {
      if ( output_path[output_path.length()-1] != DIRECTORY_SEPARATOR )
         output_path += DIRECTORY_SEPARATOR;
			
      // make directory if it doesn't exist
      osgDB::makeDirectory( output_path );
   }
	
   if ( arguments.read("-t", temp_path) ) {
      if ( temp_path[temp_path.length()-1] != DIRECTORY_SEPARATOR )
         temp_path += DIRECTORY_SEPARATOR;
			
      // make directory if it doesn't exist
      osgDB::makeDirectory( temp_path );
   } else {
      temp_path = output_path;
   }
	
   // read file list
   std::vector<FileInfo>		file_info_list;
	
   for( int argumentsPos=1; argumentsPos<arguments.argc(); ++argumentsPos )
   {
      if ( ! arguments.isOption(argumentsPos) ) {
         FileInfo	info;
         info.path = arguments[argumentsPos];
         file_info_list.push_back( info );
      }
   }
	
   model::CoordinateSpaceConverter	unified;
   osg::BoundingBox				bounds;		// in unified space
	
   /**
	 * Step 1: Find overall min/max and unified coordinate space
	 */

   for (unsigned int file_index=0; file_index<file_info_list.size(); file_index++)
   {
      FileInfo&	file_info = file_info_list[file_index];
      std::string file_name = file_info.path;
		
      Tdp_file	input_file;
      input_file.file_open( file_name, "rb" );
		
      if ( input_file.file_is_open() == false ) {
         std::cerr << "Could not open file \"" << file_name << "\". Skipping...\n";
			
         // remove from list, continue
         file_info_list.erase( file_info_list.begin()+file_index );
         file_index--;
         continue;
      }
		
      std::map< uint64_t, uint64_t >	tdp_contents = input_file.klv_index();
		
      if ( tdp_contents[ TdpKeyXYZ_POINT_DATA ] < 1 && tdp_contents[ TdpKeyALIRT_XYZP ] < 1 ) {
         std::cerr << "The input file \"" << file_name << "\" does not contain any point cloud data. Skipping...\n";
			
         // remove from list, continue
         file_info_list.erase( file_info_list.begin()+file_index );
         file_index--;
         continue;
      }
		
      char				utm_zone[4] = "\0\0\0";
      osg::Matrixd		localToWorld;

      // read UTM zone
      if ( tdp_contents[ TdpKeyUTMZone ] ) {
         input_file.klv_read( TdpKeyUTMZone, 0, (char *)utm_zone, 3 * sizeof(char) );
         utm_zone[3] = 0;
			
         osg::notify(osg::NOTICE) << "\tUTM Zone: " << utm_zone << std::endl;
      }
		
      // read theta key - skip if the output should be aligned along the heading direction
      if ( tdp_contents[ TdpKeyALIRT_THETA ] && !heading_align_tiles ) {
         real64_t	theta;
         input_file.klv_read( TdpKeyALIRT_THETA, 0, &theta, sizeof(real64_t) );
			
         localToWorld *= osg::Matrixd::rotate( theta, osg::Z_AXIS );
         osg::notify(osg::NOTICE) << "\tTheta Rotation: " << theta << " radians" << std::endl;
      }
		
      // read UTM offset
      if ( tdp_contents[ TdpKeyUTMOffset ] ) {
         osg::Vec3d	offset;
         input_file.klv_read( TdpKeyUTMOffset, 0, offset.ptr(), 3 * sizeof(real64_t) );
			
         localToWorld *= osg::Matrixd::translate( offset );
         osg::notify(osg::NOTICE) << "\tUTM Zone Offset: " << offset << " meters" << std::endl;
      }
		
      if ( no_rotation ) {
         // produce axis-aligned tiles by faking a conversion from the translation matrix only first
         unified.convert( utm_zone, osg::Matrixd::translate( localToWorld.getTrans() ), file_info.local_to_unified );
      }
		
      // find transform into unified space
      unified.convert( utm_zone, localToWorld, file_info.local_to_unified );
		
      osg::BoundingBox	local_bounds;
		
      // does this file have min and max keys?
      if ( tdp_contents[ TdpKeyALIRT_MIN ] && tdp_contents[ TdpKeyALIRT_MAX ] ) {		
         float		min[3];
         float		max[3];
			
         input_file.klv_read( TdpKeyALIRT_MIN, 0, min, 3 * sizeof(float) );
         input_file.klv_read( TdpKeyALIRT_MAX, 0, max, 3 * sizeof(float) );
			
         // if one of these values is very nearly zero, warn the user
         static bool s_ZeroWarning = false;
         if ( !s_ZeroWarning && (
            osg::equivalent( min[0], 0.0f ) || osg::equivalent( min[1], 0.0f ) || osg::equivalent( min[2], 0.0f ) ||
            osg::equivalent( max[0], 0.0f ) || osg::equivalent( max[1], 0.0f ) || osg::equivalent( max[2], 0.0f ) ) )
         {
            std::cout << "*** Warning: TdpKeyALIRT_MIN or TdpKeyALIRT_MAX contains a nearly zero value. This could indicate that it was incorrectly calculated." << std::endl;
            s_ZeroWarning = true;
         }
			
         local_bounds.set( min[0], min[1], min[2], max[0], max[1], max[2] );
			
         if ( ! local_bounds.valid() )
            std::cerr << "Found illegal bounds in \"" << file_name << "\". I will ignoring this.\n";
      }
		
      // calculate min/max from data
      if ( ! local_bounds.valid() ) {
         // load xyz point data
         osg::Vec3Array *	points = new osg::Vec3Array;
         unsigned int		count = 0;
			
         if ( tdp_contents[ TdpKeyXYZ_POINT_DATA ] ) {
            count = input_file.klv_length( TdpKeyXYZ_POINT_DATA, 0 ) / sizeof(float) / 3;
            points->resize( count );
				
            input_file.klv_read( TdpKeyXYZ_POINT_DATA, 0, (float *)points->getDataPointer(), count * 3 * sizeof(float) );
         }
			
         if ( tdp_contents[ TdpKeyALIRT_XYZP ] ) {
            count = input_file.klv_length( TdpKeyALIRT_XYZP, 0 ) / sizeof(float) / 4;
            points->resize( count );
				
            float *	buffer = new float[count * 4];
            input_file.klv_read( TdpKeyALIRT_XYZP, 0, buffer, count * 4 * sizeof(float) );
				
				// copy xyz components, drop p
            for( unsigned long i = 0; i < count; i++ ) {
               (*points)[i].set( buffer[4*i + 0], buffer[4*i + 1], buffer[4*i + 2] );
            }
				
            delete [] buffer;
         }

         for ( unsigned int i = 0; i < count; i++ )
            local_bounds.expandBy( (*points)[i] );
			
         points->unref();
      }
		
      if ( ! local_bounds.valid() ) {
         std::cerr << "The input file \"" << file_name << "\" does not contain a valid set of points. Skipping...\n";
			
         // remove from list, continue
         file_info_list.erase( file_info_list.begin()+file_index );
         file_index--;
         continue;
      }
		
      // transform the bounding box into unified space
      file_info.unified_extent = model::transform( local_bounds, file_info.local_to_unified );
      bounds.expandBy( file_info.unified_extent );
   }
	
   if ( ! bounds.valid() ) {
      std::cerr << "There does not appear to be any valid data here.\n";
      return 1;
   }

   std::cout << "Bounds of input set: ";
   std::cout << "[" << bounds.xMin() << " " << bounds.xMax() << "] ";
   std::cout << "[" << bounds.yMin() << " " << bounds.yMax() << "] ";
   std::cout << "[" << bounds.zMin() << " " << bounds.zMax() << "]\n";

   osg::Vec3d	unifiedOffset;
   double		unifiedAngle;
   {
      osg::Matrixd		matrix = unified.getLocalOffset();
      osg::Quat			quat;
      osg::Vec3d			axis;
		
      unifiedOffset = matrix.getTrans();
      quat.set( matrix );
      quat.getRotate( unifiedAngle, axis );

      // flip sign if axis is -Z
      if ( (axis + osg::Z_AXIS).length() < 0.001 ) {
         axis = osg::Z_AXIS;
         unifiedAngle *= -1;
      }
		
      std::cout << "Unified Offset: " << unifiedOffset << "\n";
      std::cout << "Unified Angle: " << unifiedAngle << "\n";
   }

   /**
	 * Step 2: Build array of tile objects 
	 */
	
   typedef std::vector<Tile*>				TileList;
   TileList								tiles;
   TileList::iterator						tileIter;
   osg::Matrixd							world_to_unified = osg::Matrixd::inverse( unified.getLocalOffset() );
	
   if ( crop_by_box ) {
      // convert ll to utm
      if ( convert_to_utm ) {
         char UTMZone[4];
			
         // Ref Ellipsoid 23 = WGS-84. See list with file "LatLong- UTM conversion.cpp" for id numbers

         bool northern_hemisphere_flag;
         int UTM_zonenumber;
         double UTMNorthing,UTMEasting;
         latlongfunc::LLtoUTM(
            crop_box_range[0].y(),crop_box_range[0].x(),
            UTM_zonenumber,northern_hemisphere_flag,UTMNorthing,UTMEasting);
         crop_box_range[0].x()=UTMEasting;
         crop_box_range[0].y()=UTMNorthing;

         latlongfunc::LLtoUTM(
            crop_box_range[1].y(),crop_box_range[1].x(),
            UTM_zonenumber,northern_hemisphere_flag,UTMNorthing,UTMEasting);
         crop_box_range[1].x()=UTMEasting;
         crop_box_range[1].y()=UTMNorthing;

/*
         ll_to_utm( 23, crop_box_range[0].y(), crop_box_range[0].x(), crop_box_range[0].y(), crop_box_range[0].x(), UTMZone);
         ll_to_utm( 23, crop_box_range[1].y(), crop_box_range[1].x(), crop_box_range[1].y(), crop_box_range[1].x(), UTMZone);
*/


      }
		
      // apply the local offset to the UTM coords
      osg::BoundingBox	crop_box( crop_box_range[0], crop_box_range[1] );
      crop_box = model::transform( crop_box, world_to_unified );
				
      // never crop in z axis
      crop_box.zMin() = -FLT_MAX;
      crop_box.zMax() = FLT_MAX;
        
      if ( alignment > 0 ) alignBoundingBox( crop_box, alignment );

      // display crop box in local coords
      std::cout << "Crop box converted into offset UTM:" << std::endl;
      std::cout << "Xmin: " << crop_box.xMin() << " Ymin: " << crop_box.yMin() 
                << " Xmax: " << crop_box.xMax() << " Ymax: " << crop_box.yMax() << std::endl;
      std::cout << std::endl;
		
      TileBox *	croptile = new TileBox( crop_box );
		
      croptile->filename = "crop_buffer.tdp";
      croptile->xi = 0;
      croptile->yi = 0;
		
      tiles.push_back( croptile );
   }
   else if ( crop_by_poly ) {
      // Read points from file into polygon
      Tile2DPoly *	polytile = new Tile2DPoly;

      std::ifstream	input_file( crop_poly_filename.c_str() );
		
      if ( ! input_file ) {
         std::cerr << "Can't open poly file \"" << crop_poly_filename << "\"\n";
         return 1;
      }
		
      while ( input_file.good() ) {
         // check for comment - skip line
         if ( input_file.peek() == '#' ) {
            std::string	comment;
            getline( input_file, comment );
            continue;
         }

         osg::Vec3d	vertex;

         if ( input_file.eof() ) break;
         input_file >> vertex.x();

         if ( input_file.eof() ) break;
         input_file >> vertex.y();

         if ( input_file.eof() ) break;
         input_file >> vertex.z();

         if ( convert_to_utm ) {
            char UTMZone[4];

            // Ref Ellipsoid 23 = WGS-84. See list with file "LatLong- UTM conversion.cpp" for id numbers

            bool northern_hemisphere_flag;
            int UTM_zonenumber;
            double UTMNorthing,UTMEasting;
            latlongfunc::LLtoUTM(
               vertex.y(),vertex.x(),UTM_zonenumber,northern_hemisphere_flag,
               UTMNorthing,UTMEasting);
            vertex.x()=UTMEasting;
            vertex.y()=UTMNorthing;

//            ll_to_utm( 23, vertex.y(), vertex.x(), vertex.y(), vertex.x(), UTMZone);
         }

         printf( "Polygon Vertex (UTM): %f %f %f\n", vertex.x(), vertex.y(), vertex.z() );

            // apply the local offset to the UTM coord
         vertex = world_to_unified.preMult( vertex );

         polytile->add( osg::Vec2f( vertex.x(), vertex.y() ) );
      };
		
      polytile->filename = "crop_buffer.tdp";
      polytile->xi = 0;
      polytile->yi = 0;
		
      tiles.push_back( polytile );
   }
   else if ( crop_by_circle ) {
      // convert ll to utm
      if ( convert_to_utm ) {
         char UTMZone[4];
			
         // Ref Ellipsoid 23 = WGS-84. See list with file "LatLong- UTM conversion.cpp" for id numbers

         bool northern_hemisphere_flag;
         int UTM_zonenumber;
         double UTMNorthing,UTMEasting;
         latlongfunc::LLtoUTM(
            circle_center.y(),circle_center.x(),
            UTM_zonenumber,northern_hemisphere_flag,UTMNorthing,UTMEasting);
         circle_center.x()=UTMEasting;
         circle_center.y()=UTMNorthing;
            
//         ll_to_utm( 23, circle_center.y(), circle_center.x(), circle_center.y(), circle_center.x(), UTMZone);
//         ll_to_utm( 23, circle_center.y(), circle_center.x(), circle_center.y(), circle_center.x(), UTMZone);
      }
		
      // apply the local offset to the UTM coords
      circle_center = circle_center * world_to_unified;
        
      // display crop box in local coords
      std::cout << "Crop circle converted into offset UTM:" << std::endl;
      std::cout << "X: " << circle_center.x() << " Y: " << circle_center.y() 
                << " Radius: " << circle_radius << std::endl;
      std::cout << std::endl;
		
      TileCircle *	cropcircle = new TileCircle( osg::Vec2d( circle_center.x(), circle_center.y() ), circle_radius );
		
      cropcircle->filename = "crop_buffer.tdp";
      cropcircle->xi = 0;
      cropcircle->yi = 0;
		
      tiles.push_back( cropcircle );
   }
   else
   {
      // split whole data bounds into tiles
      float	x_length = bounds.xMax() - bounds.xMin();
      float	y_length = bounds.yMax() - bounds.yMin();

      if ( calc_tile_size ) {
         // calculate size of tiles from count
         tile_size.x() = x_length / x_tiles;
         tile_size.y() = y_length / y_tiles;
      } else if ( calc_tile_num ) {
         // calculate number of tiles from size
         x_tiles = (unsigned int)ceil( x_length / tile_size.x() );
         y_tiles = (unsigned int)ceil( y_length / tile_size.y() );
      } else {
         // use default - one large tile
         tile_size.set( bounds.xMax() - bounds.xMin(), bounds.yMax() - bounds.yMin() );
         x_tiles = 1;
         y_tiles = 1;
      }

      std::cout << "Sorting " << file_info_list.size() << " input files into " << x_tiles << " x " << y_tiles << " tiles." << std::endl;
      std::cout << "Tile size: " << tile_size.x() << " x " << tile_size.y() << " m" << std::endl;	
		
      for( unsigned int xti = 0; xti < x_tiles; xti++ ) {
         for( unsigned int yti = 0; yti < y_tiles; yti++ ) {
            std::ostringstream	ostream;
            ostream << temp_path << base_name << "_buffer";
            ostream << ".x" << xti << ".y" << yti << ".tdp";
				
            osg::BoundingBox	box;
				
            box.xMin() = bounds.xMin() + xti * tile_size.x();
            box.xMax() = box.xMin() + tile_size.x() + overlap;
				
            box.yMin() = bounds.yMin() + yti * tile_size.y();
            box.yMax() = box.yMin() + tile_size.y() + overlap;

            box.zMin() = -std::numeric_limits<float>::max();
            box.zMax() = std::numeric_limits<float>::max();
				
            if ( alignment > 0 ) alignBoundingBox( box, alignment );
                
            TileBox *	tile = new TileBox( box );
            tile->filename = ostream.str();
            tile->xi = xti;
            tile->yi = yti;
				
            tiles.push_back( tile );
         }
      }
   }
	
   // perform other setup per-tile
   for( tileIter = tiles.begin(); tileIter != tiles.end(); ++tileIter ) {
      // delete old buffer file if it exists
      if( remove( (*tileIter)->filename.c_str() ) == 0 ) {
         std::cout << "Deleted \"" << (*tileIter)->filename << "\".\n";
      }
   }
	
   /**
	 * Step 3: For each swath, place points and corresponding metadata into tile files 
	 */
	
	// setup metadata type map
   TdpDataTypeMap						metadataTypes;
   TdpDataTypeMap::const_iterator		mdtIter;

   metadataTypes[20300] = TdpDataType( 20300, "ALIRT_XYZP", TypeReal32 );
   metadataTypes[31100] = TdpDataType( 31100, "METADATA_NUMBER_OF_NEIGHBORS", TypeUint16 );
   metadataTypes[31101] = TdpDataType( 31101, "METADATA_COVERAGE_DENSITY", TypeUint16 );
   metadataTypes[31102] = TdpDataType( 31102, "METADATA_PROBABILITY_OF_DETECTION", TypeReal32 );
   metadataTypes[31103] = TdpDataType( 31103, "METADATA_RELATIVE_REFLECTIVITY", TypeReal32 );
   metadataTypes[31104] = TdpDataType( 31104, "METADATA_RELATIVE_REFLECTIVITY_CONFIDENCE", TypeUint16 );
   metadataTypes[20258] = TdpDataType( 20258, "ALIRT2_INTEGRATED_ZCP_COUNT", TypeUint32 );
   metadataTypes[31105] = TdpDataType( 31105, "METADATA_NUMBER_OF_HITS", TypeUint16 );
   metadataTypes[31106] = TdpDataType( 31106, "METADATA_SIGMA_Z", TypeUint16 );
   metadataTypes[31107] = TdpDataType( 31107, "METADATA_FRAME_NUMBER", TypeUint32 );
   metadataTypes[31108] = TdpDataType( 31108, "METADATA_PIXEL_NUMBER", TypeUint16 );
   metadataTypes[31110] = TdpDataType( 31110, "METADATA_NORMALIZED_PEAK_POWER", TypeReal32 );
   metadataTypes[31114] = TdpDataType( 31114, "METADATA_HEIGHT_ABOVE_GROUND", TypeReal32 );
   metadataTypes[31115] = TdpDataType( 31115, "METADATA_PERCENT_GROUND_POINTS", TypeReal32 );
		
   for (unsigned int file_index=0; file_index<file_info_list.size(); file_index++)
   {
      FileInfo&	file_info = file_info_list[file_index];
      std::string file_name = file_info.path;
		
      Tdp_file	input_file;
      input_file.file_open( file_name, "rb" );
		
      if ( input_file.file_is_open() == false ) {
         std::cerr << "Could not open file \"" << file_name << "\". Skipping...\n";
			
         // remove from list, continue
         file_info_list.erase( file_info_list.begin()+file_index );
         file_index--;
         continue;
      }
        
      std::cout << "--- Sorting file \"" << file_name << "\". (" << file_index+1 << " of " << file_info_list.size() << ")\n";
		
      std::map< uint64_t, uint64_t >	tdp_contents = input_file.klv_index();
		
      // load xyz point data
      osg::ref_ptr<osg::Vec3Array>	points = new osg::Vec3Array;
      Tile::ArrayMap					metadata;
      unsigned int					count = 0;
		
      if ( tdp_contents[ TdpKeyXYZ_POINT_DATA ] ) {
         count = input_file.klv_length( TdpKeyXYZ_POINT_DATA, 0 ) / sizeof(float) / 3;
         points->resize( count );
			
         input_file.klv_read( TdpKeyXYZ_POINT_DATA, 0, (float *)points->getDataPointer(), count * 3 * sizeof(float) );
      }

      if ( tdp_contents[ TdpKeyALIRT_XYZP ] ) {
         count = input_file.klv_length( TdpKeyALIRT_XYZP, 0 ) / sizeof(float) / 4;
         points->resize( count );
			
         // save XYZP stream into a float buffer
         float *	buffer = new float[count * 4];
         input_file.klv_read( TdpKeyALIRT_XYZP, 0, buffer, count * 4 * sizeof(float) );
			
         // store p's in a separate metadata array
         osg::FloatArray *	pArray = new osg::FloatArray;
         pArray->resize( count );

         // copy xyz components into one array, p into another
         for( unsigned long i = 0; i < count; i++ ) {
            (*points)[i].set( buffer[4*i + 0], buffer[4*i + 1], buffer[4*i + 2] );
            (*pArray)[i] = buffer[4*i + 3];
         }
			
         // save p's as metadata
         metadata[TdpKeyALIRT_XYZP] = pArray;
			
         delete [] buffer;
      }
		
      if ( count < 1 ) {
         std::cerr << "The input file \"" << file_name << "\" does not contain any point cloud data. Skipping...\n";
			
         // remove from list, continue
         file_info_list.erase( file_info_list.begin()+file_index );
         file_index--;
         continue;
      }
		
      // transform data into unified space
      for( unsigned long i = 0; i < count; i++ )
         (*points)[i] = file_info.local_to_unified.preMult( (*points)[i] );
		
      // load in metadata
      for( mdtIter = metadataTypes.begin(); mdtIter != metadataTypes.end(); mdtIter++ )
      {
         unsigned int key = mdtIter->first;
			
         // treat ALIRT_XYZP as a special case. don't load it twice.
         if ( key == TdpKeyALIRT_XYZP ) continue;
			
         // does this input file contain this metadata type?
         if ( tdp_contents[ key ] ) {
				// expected size?
            unsigned int	local_count = input_file.klv_length( key, 0 ) / mdtIter->second.elementSizeBytes();
				
            if ( local_count != count ) {
               std::cerr << "In file \"" << file_name << "\", the metadata type " << mdtIter->second.name
                         << " does not have the correct length. Skipping...\n";
               continue;
            }
				
				// load data
            osg::Array *	data = NULL;

            switch( mdtIter->second.type ) {
               case TypeUint16:
               {	
                  osg::UShortArray *	typeArray = new osg::UShortArray;
                  typeArray->resize( count );
                  input_file.klv_read( key, 0, (uint16_t *)typeArray->getDataPointer(), count * sizeof(uint16_t) );
                  data = typeArray;
                  break;
               }
               case TypeUint32:
               {	
                  osg::UIntArray *	typeArray = new osg::UIntArray;
                  typeArray->resize( count );
                  input_file.klv_read( key, 0, (uint32_t *)typeArray->getDataPointer(), count * sizeof(uint32_t) );
                  data = typeArray;
                  break;
               }
               case TypeReal32:
               {	
                  osg::FloatArray *	typeArray = new osg::FloatArray;
                  typeArray->resize( count );
                  input_file.klv_read( key, 0, (float *)typeArray->getDataPointer(), count * sizeof(float) );
                  data = typeArray;
                  break;
               }
               case TypeUndefined: break;
            }
								
				// did we load anything?
            if ( data ) {
               metadata[key] = data;
            }
         }
      }
		
      input_file.file_close();

      // search tiles for a matching bounding box
      for( tileIter = tiles.begin(); tileIter != tiles.end(); tileIter++ )
      {
         Tile *	tile = *tileIter;
			
         // are we even close?
         if ( ! file_info.unified_extent.intersects( tile->bounds ) )
            continue;
			
         for ( unsigned int i = 0; i < count; i++ ) {
            if ( crop_by_z ) {
               float& z = (*points)[i].z();
               if ( z < crop_z_min || z > crop_z_max )
                  continue;   // skip point
            }
                
            if ( crop_by_meta ) {
               osg::Array *    mda = metadata[crop_meta_key].get();
               if ( mda ) {
                  switch( mda->getType() ) {
                     case osg::Array::ShortArrayType:
                        if ( osg::UShortArray* mda_ushort = dynamic_cast<osg::UShortArray*>( mda ) )
                           if ( (*mda_ushort)[i] < crop_meta_min || (*mda_ushort)[i] > crop_meta_max )
                              continue; // skip point
                        break;
                     case osg::Array::IntArrayType:
                        if ( osg::UIntArray* mda_uint = dynamic_cast<osg::UIntArray*>( mda ) )
                           if ( (*mda_uint)[i] < crop_meta_min || (*mda_uint)[i] > crop_meta_max )
                              continue; // skip point
                        break;
                     case osg::Array::FloatArrayType:
                        if ( osg::FloatArray* mda_float = dynamic_cast<osg::FloatArray*>( mda ) )
                           if ( (*mda_float)[i] < crop_meta_min || (*mda_float)[i] > crop_meta_max )
                              continue; // skip point
                        break;
                     default:
                        break; // can't handle this type
                  }
               }
            }
                
            if ( tile->contains( (*points)[i] ) )
            {
               // copy point into buffer
               if ( ! tile->vertices.valid() )
                  tile->vertices = new osg::Vec3Array;
										
               tile->vertices->push_back( (*points)[i] );
					
               // expand bounds of this tile
               tile->extent.expandBy( (*points)[i] );
					
               // copy metadata into buffer
               Tile::ArrayMap::const_iterator	mdIter;
               for( mdIter = metadata.begin(); mdIter != metadata.end(); mdIter++ ) {
                  unsigned int key = mdIter->first;
                        
                  // make sure source metadata array is valid
                  if ( mdIter->second.valid() == false ) continue;
						
                  // make sure metadata buffer is valid
                  if ( ! tile->metadata[key].valid() ) {
                     switch( metadataTypes[key].type ) {
                        case TypeUint16: tile->metadata[key] = new osg::UShortArray; break;
                        case TypeUint32: tile->metadata[key] = new osg::UIntArray; break;
                        case TypeReal32: tile->metadata[key] = new osg::FloatArray; break;
                        case TypeUndefined: break;
                     }
							
                     if ( ! mdIter->second.valid() ) break;
                  }
						
                  switch( metadataTypes[key].type ) {
                     case TypeUint16:
                     {	
                        const osg::UShortArray * source = static_cast<const osg::UShortArray *>( mdIter->second.get() );
                        osg::UShortArray * destination = static_cast<osg::UShortArray *>( tile->metadata[key].get() );
                        destination->push_back( (*source)[i] );
                        break;
                     }
                     case TypeUint32:
                     {	
                        const osg::UIntArray * source = static_cast<const osg::UIntArray *>( mdIter->second.get() );
                        osg::UIntArray * destination = static_cast<osg::UIntArray *>( tile->metadata[key].get() );
                        destination->push_back( (*source)[i] );
                        break;
                     }
                     case TypeReal32:
                     {	
                        const osg::FloatArray * source = static_cast<const osg::FloatArray *>( mdIter->second.get() );
                        osg::FloatArray * destination = static_cast<osg::FloatArray *>( tile->metadata[key].get() );
                        destination->push_back( (*source)[i] );
                        break;
                     }
                     case TypeUndefined: break;
                  }
               }
            }
         }
      }
		
      points = NULL;
      metadata.clear();
		
      // flush buffers
      for( tileIter = tiles.begin(); tileIter != tiles.end(); tileIter++ )
      {
         Tile *	tile = *tileIter;
			
         if ( tile->vertices.valid() ) {
            Tdp_file	output_file;
				
				// open file if it already exist
            if ( output_file.file_open( tile->filename, "r+b" ) == false )
               // create file
               output_file.file_open( tile->filename, "w+b" );
				
            if ( ! output_file.file_is_open() ) {
               std::cerr << "Could not open output file \"" << tile->filename << "\". Buffer will not be written to disk.\n";
            } else {
               unsigned int flush_count = tile->vertices->getNumElements();
					
               // if a XYZP metadata key is present, write out an XYZP key instead of XYZ
               if ( tile->metadata.find( TdpKeyALIRT_XYZP ) != tile->metadata.end() ) {
                  const osg::FloatArray * pArray = static_cast<const osg::FloatArray *>( tile->metadata[TdpKeyALIRT_XYZP].get() );
						
                  // save XYZP stream into a float buffer
                  float *	buffer = new float[flush_count * 4];
						
                  for( unsigned long i = 0; i < flush_count; i++ ) {
                     buffer[4*i + 0] = (*tile->vertices)[i].x();
                     buffer[4*i + 1] = (*tile->vertices)[i].y();
                     buffer[4*i + 2] = (*tile->vertices)[i].z();
                     buffer[4*i + 3] = (*pArray)[i];
                  }
						
                  // write xyzp data
                  output_file.klv_create( TdpKeyALIRT_XYZP, flush_count * 4 * sizeof(float) );
						
                  unsigned int klv_point_index = output_file.klv_index()[ TdpKeyALIRT_XYZP ] - 1;
						
                  output_file.klv_write( TdpKeyALIRT_XYZP, klv_point_index, 
                                         (float *)buffer, flush_count * 4 * sizeof(float) );
						
                  // do not write out with other metadata
                  tile->metadata.erase( TdpKeyALIRT_XYZP );

                  delete [] buffer;
               } else {					
                  // write point data
                  output_file.klv_create( TdpKeyXYZ_POINT_DATA, flush_count * 3 * sizeof(float) );
						
                  unsigned int klv_point_index = output_file.klv_index()[ TdpKeyXYZ_POINT_DATA ] - 1;
						
                  output_file.klv_write( TdpKeyXYZ_POINT_DATA, klv_point_index, 
                                         (float *)tile->vertices->getDataPointer(), flush_count * 3 * sizeof(float) );
               }
					
               // write metadata
               Tile::ArrayMap::const_iterator	mdIter;
               for( mdIter = tile->metadata.begin(); mdIter != tile->metadata.end(); mdIter++ ) {
                  if ( mdIter->second.valid() == false ) continue;
						
                  unsigned int key = mdIter->first;
						
                  output_file.klv_create( key, flush_count * metadataTypes[ key ].elementSizeBytes() );
                  unsigned int klv_metadata_index = output_file.klv_index()[ key ] - 1;
						
                  switch( metadataTypes[key].type ) {
                     case TypeUint16:
                     {	
                        output_file.klv_write( key, klv_metadata_index, 
                                               (uint16_t *)tile->metadata[key]->getDataPointer(), flush_count * sizeof(uint16_t) );
                        break;
                     }
                     case TypeUint32:
                     {	
                        output_file.klv_write( key, klv_metadata_index, 
                                               (uint32_t *)tile->metadata[key]->getDataPointer(), flush_count * sizeof(uint32_t) );
                        break;
                     }
                     case TypeReal32:
                     {	
                        output_file.klv_write( key, klv_metadata_index, 
                                               (float *)tile->metadata[key]->getDataPointer(), flush_count * sizeof(float) );
                        break;
                     }
                     case TypeUndefined: break;
                  }
               }
							
               std::cout << "Wrote " << flush_count << " points to \"" << tile->filename << "\".\n";
            }
				
            tile->has_been_flushed_to_disk = true;
         }
			
         tile->vertices = NULL;
         tile->metadata.clear();
      }
   }
	
   file_info_list.clear();
	
	/**
	 * Step 4: For each tile, output one KLV per data type, overwrite in place 
	 */
	
	// compile a list of keys to combine
	/*std::vector<unsigned int>	keylist;
	keylist.push_back( TdpKeyXYZ_POINT_DATA );
	for( mdtIter = metadataTypes.begin(); mdtIter != metadataTypes.end(); mdtIter++ )
		keylist.push_back( mdtIter->first );*/
	
   for( tileIter = tiles.begin(); tileIter != tiles.end(); tileIter++ )
   {
      Tile *	tile = *tileIter;
		
      if ( tile->has_been_flushed_to_disk ) {			
         std::ostringstream	ostream;
         ostream << output_path << base_name;
         ostream << ".x" << tile->xi << ".y" << tile->yi << ".tdp";
			
         std::string	filename = ostream.str();
			
         // open buffer tdp file
         Tdp_file	buffer_file;
         buffer_file.file_open( tile->filename, "rb" );
			
         if ( ! buffer_file.file_is_open() ) {
            std::cerr << "Could not open tile buffer file \"" << tile->filename << "\". Could not condense its contents.\n";
            continue;
         }

         std::map<uint64_t, uint64_t>	buffer_index = buffer_file.klv_index();

         // create file, overwriting in place
         Tdp_file	output_file;
         output_file.file_open( filename, "w+b" );
         output_file.stream_set_endian( buffer_file.stream_get_endian() );	// endian must be equal for byte copy to work
						
         if ( ! output_file.file_is_open() ) {
            std::cerr << "Could not create output file \"" << filename << "\".\n";
            continue;
         } else {
				// write UTM zone
            if ( unified.isDefined() ) {
               std::string	utmZone = unified.getUTMString();
					
               output_file.klv_create( TdpKeyUTMZone, 3 * sizeof(char) );
               output_file.klv_write( TdpKeyUTMZone, 0, (char *)utmZone.c_str(), 3 * sizeof(char) );
            }
				
				// write the unified theta and offset into the file
            if ( fabs(unifiedAngle) > 0.001 ) {
               real64_t	theta = unifiedAngle;
               output_file.klv_create( TdpKeyALIRT_THETA, sizeof(real64_t) );
               output_file.klv_write( TdpKeyALIRT_THETA, 0, &theta, sizeof(real64_t) );
            }

            if ( unifiedOffset.length() > 0.001 ) {
               output_file.klv_create( TdpKeyUTMOffset, 3 * sizeof(real64_t) );
               output_file.klv_write( TdpKeyUTMOffset, 0, unifiedOffset.ptr(), 3 * sizeof(real64_t) );
            }
				
				// write the min and max keys
            float	min[3];
            float	max[3];
            min[0] = tile->extent.xMin();	min[1] = tile->extent.yMin();	min[2] = tile->extent.zMin();
            max[0] = tile->extent.xMax();	max[1] = tile->extent.yMax();	max[2] = tile->extent.zMax();
				
            output_file.klv_create( TdpKeyALIRT_MIN, 3 * sizeof(real32_t) );
            output_file.klv_write( TdpKeyALIRT_MIN, 0, min, 3 * sizeof(real32_t) );

            output_file.klv_create( TdpKeyALIRT_MAX, 3 * sizeof(real32_t) );
            output_file.klv_write( TdpKeyALIRT_MAX, 0, max, 3 * sizeof(real32_t) );
				
				// consolidate all buffered data
            util::consolidate_klv_types( buffer_file, output_file );

            output_file.file_close();
         }

         buffer_file.file_close();

         // delete temp file if it exists
         if( remove( tile->filename.c_str() ) == 0 ) {
            std::cout << "Deleted \"" << tile->filename << "\".\n";
         }
      }
   }
	
   // free tiles
   while( ! tiles.empty() )
   {
      delete tiles.back();
      tiles.pop_back();
   }
	
   return 0;
}
