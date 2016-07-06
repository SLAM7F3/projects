// ========================================================================
// Ross Anderson's LODTREE program from his trunk eyeglass SVN
// repository modified to incorporate a CoordinateSystemNode at the
// very top of the .osga datagraph tree.  Metadata is also written to
// geometries' FogCoordArrays rather than to their SecondaryColorArrays. 

// Note: UTM zones and northern_hemisphere boolean flags for
// particular TDP files are hardwired into the code below.

// Example invocation statement:

// 				lodtree mit.tdp

// ========================================================================
// Last updated on by PC on 11/29/10; 2/22/13; 4/5/13
// ========================================================================

/*
 *  lodtree.cpp
 *  Nonpoint
 *
 *  Created by Ross Anderson on 7/22/05.
 *  Copyright 2005 MIT Lincoln Laboratory. All rights reserved.
 *
 */

#include <config.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <list>

#if defined( _WIN32 )
#include <direct.h>
#define DIRECTORY_SEPARATOR '\\'
#define MAXPATHLEN 255
#else
#include <sys/param.h>
#define DIRECTORY_SEPARATOR '/'
#endif

#include <osg/Notify>
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Vec3>
#include <osg/Group>
#include <osg/PagedLOD>
#include <osg/MatrixTransform>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/Archive>
#include <osg/io_utils>

#include <lodtree/DataSet_OutOfCore.h>
#include <lodtree/PageAssembler.h>
#include <model/Metadata.h>
#include <model/CoordinateSpaceConverter.h>

#include "astro_geo/geofuncs.h"
#include "general/inputfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using namespace lodtree;
using namespace std;


class PackMetadataIntoArrayVisitor : public osg::NodeVisitor
{
  public:
	
   PackMetadataIntoArrayVisitor() :
      NodeVisitor( TRAVERSE_ALL_CHILDREN )
      {}
		
   void apply(osg::Geode& geode)
      {
         osg::Geode::DrawableList::const_iterator	drawableIter;
         for( drawableIter = geode.getDrawableList().begin(); 
              drawableIter != geode.getDrawableList().end(); 
              ++drawableIter )
         {
            if ( osg::Geometry* geometry = const_cast<osg::Geometry*>( 
               drawableIter->get()->asGeometry() ) ) {
               model::Metadata*	metadata = model::getMetadataForGeometry( 
                  *geometry );
					
               // does this Geometry contain Metadata?
               if ( metadata ) {
                  osg::Array*	metadata_array = 
                     metadata->asStandardArrayType();
						
                  // Bury the metadata as a fog coordinate array which
                  // is not normally used:

                  geometry->setFogCoordBinding( osg::Geometry::BIND_OFF );
                  geometry->setFogCoordArray( metadata_array );

                  // Bury the metadata as a secondary color array
//                  geometry->setSecondaryColorBinding( 
//                     osg::Geometry::BIND_OFF );
//                  geometry->setSecondaryColorArray( metadata_array );
						
                  // remove the Metadata object
                  geometry->setUserData( NULL );
               }
            }
         }
			
         NodeVisitor::apply((osg::Node&)geode);
      }
};


int main (int argc, char **argv)
{
   // use an ArgumentParser object to manage the program arguments.
   osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
   arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
   arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" will generate an optimized kd-tree based on the given XYZP input files.");
   arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] input_file [input_file] ...");
   arguments.getApplicationUsage()->addCommandLineOption("-o",					"Set the output path");
   arguments.getApplicationUsage()->addCommandLineOption("--name",				"Set the base name use for the master file, db path, and archive file");
   arguments.getApplicationUsage()->addCommandLineOption("--no-pages",			"Do not break the model into pages on disk. Use for small data sets only.");
   arguments.getApplicationUsage()->addCommandLineOption("--no-archive",		"No not package output into an OSGA archive file, write IVEs instead.");
   arguments.getApplicationUsage()->addCommandLineOption("--offset",			"Explicity add an X Y Z offset of the dataset");
   arguments.getApplicationUsage()->addCommandLineOption("--max_levels",		"Limit the number of levels possible in the tree. Detail will be thrown out");
   arguments.getApplicationUsage()->addCommandLineOption("--max_page_levels",	"Limit the number of levels in each page allowed before LODs are split out to new pages");
   arguments.getApplicationUsage()->addCommandLineOption("--max_leaf_size",		"Set the maximum number of vertices allowed in a leaf node");
   arguments.getApplicationUsage()->addCommandLineOption("--max_bin_size",		"Set the maximum number of vertices allowed in a approximation node");
   arguments.getApplicationUsage()->addCommandLineOption("--separately",		"Perform a separate lodtree operation on each input file");
   arguments.getApplicationUsage()->addCommandLineOption("--no-metadata",		"Place only geometric data in the output");
   arguments.getApplicationUsage()->addCommandLineOption("-h or --help",		"Display this information");
		
   // if user request help write it out to cout
   if (arguments.read("-h") || arguments.read("--help") || arguments.argc() < 2)
   {
      arguments.getApplicationUsage()->write(cout);

      cout << endl;
      cout << "Sample: lodtree -o output_dir/boston.ive boston_parts/*.tdp" << endl;
      cout << endl;
		
      cout << "Version 1.0, written by Ross Anderson" << endl;
      cout << "Copyright 2006 MIT Lincoln Laboratory" << endl;
      cout << endl;
      return 1;
   }
	
   // Treat --separately as a completely separate mode
   if ( arguments.read("--separately") ) {
      string			temp;
      osg::Vec3d			temp_offset;
      ostringstream	argstream;
		
      // setup child process path
      argstream << argv[0];
		
      // setup child process arguments
      if ( arguments.read("-o", temp) ) argstream << " -o " << temp;
      if ( arguments.read("--no-pages") ) argstream << " --no-pages";
      if ( arguments.read("--no-archive") ) argstream << " --no-archive";
      if ( arguments.read("--offset", temp_offset.x(), temp_offset.y(), temp_offset.z() ) )
         argstream << " --offset " << temp_offset.x() << " " << temp_offset.y() << " " << temp_offset.z();
      if ( arguments.read("--max_levels", temp) ) argstream << " --max_levels " << temp;
      if ( arguments.read("--max_page_levels", temp) ) argstream << " --max_page_levels " << temp;
      if ( arguments.read("--max_leaf_size", temp) ) argstream << " --max_leaf_size " << temp;
      if ( arguments.read("--max_bin_size", temp) ) argstream << " --max_bin_size " << temp;
      if ( arguments.read("--no-metadata") ) argstream << " --no-metadata";
		
      argstream << " ";
		
      // read input file paths
      for(int argumentsPos=1;argumentsPos<arguments.argc();++argumentsPos)
      {
         string	command = argstream.str() + arguments[argumentsPos];
			
         cout << "Executing: " << command << endl;
         if ( system( command.c_str() ) != 0 )
            exit(1);
         cout << endl << endl;
      }
		
      return 0;
   }

// =======================================================================

//   geofunc::print_recommended_UTM_zonenumbers();

/*
   string label_command="Enter UTM zone number for all input TDP files";
   int UTM_zone=inputfunc::enter_nonnegative_integer(label_command);

   label_command="Enter 's'/'n' for southern/northern hemisphere";
   string hemisphere_char=inputfunc::enter_string(label_command);
   bool northern_hemisphere_flag=true;
   if (hemisphere_char=="s") northern_hemisphere_flag=false;
   
   string banner="UTM zone entered into LODTREE program = "
      +stringfunc::number_to_string(UTM_zone);
   outputfunc::write_big_banner(banner);
   string banner2="Northern hemisphere flag = "
      +stringfunc::boolean_to_string(northern_hemisphere_flag);
   outputfunc::write_big_banner(banner2);
   outputfunc::enter_continue_char();
*/

// Afghanistan:

/*
   string region_name="Afghanistan";
   int UTM_zone=42;
   bool northern_hemisphere_flag=true;
*/


/*
// Arizona:

   string region_name="Arizona";
   int UTM_zone=12;
   bool northern_hemisphere_flag=true;
*/


/*
// Baghdad

   string region_name="Baghdad";
   int UTM_zone=38;
   bool northern_hemisphere_flag=true;
*/

// Boston and Lowell:

   string region_name="Boston/Lowell";
   int UTM_zone=19;
   bool northern_hemisphere_flag=true;

/*
// Florida panhandle:

   string region_name="FloridaPanhandle";
   int UTM_zone=16;
   bool northern_hemisphere_flag=true;
*/

/*
// Haiti:

   string region_name="Haiti";
   int UTM_zone=18;
   bool northern_hemisphere_flag=true;
*/

/*
// Hawaii

   string region_name="Hawaii";
   int UTM_zone=5;
   bool northern_hemisphere_flag=true;
*/

// Horn-of-Africa:

   //string region_name="Iraq/Yemen/Somalia";
  //int UTM_zone=38;
   // bool northern_hemisphere_flag=true;

// Iran/Iraq:

//   string region_name="Iran/Iraq";
//   int UTM_zone=38;
//   bool northern_hemisphere_flag=true;

// Korea:

//   string region_name="Korea";
//   int UTM_zone=52;
//   bool northern_hemisphere_flag=true;

// Lubbock, TX:  

//   string region_name="Lubbock";
//   int UTM_zone=14;
//   bool northern_hemisphere_flag=true;

/*
// Milwaukee:

   string region_name="Milwaukee";
   int UTM_zone=16;
   bool northern_hemisphere_flag=true;
*/

// New York City:

//   string region_name="New York City";
//   int UTM_zone=18;
//   bool northern_hemisphere_flag=true;

// Puerto Rico:

//   string region_name="Puerto Rico";
//   int UTM_zone=20;
//   bool northern_hemisphere_flag=true;

// Southwest US:


//   string region_name="Southwest US";
//   int UTM_zone=13;
//   bool northern_hemisphere_flag=true;

   //string region_name="California Arizona";
   //int UTM_zone=12;
   //bool northern_hemisphere_flag=true;

   string region_message="Region = "+region_name;
   string UTM_zone_message="UTM zone = "+stringfunc::number_to_string(
      UTM_zone);

   outputfunc::write_big_banner(region_message);
   outputfunc::write_big_banner(UTM_zone_message);
   string unixcommandstr="sleep 3";
   sysfunc::unix_command(unixcommandstr);

// =======================================================================

   // read arguments
   Configuration	config;
   string		outputPath;
   string		baseName;
   osg::Vec3d		explicit_offset;
   int             max_page_levels = 0;
   bool			no_metadata = false;
   bool            use_pages = true;
	
   config.page_extension = ".ive";
   if ( arguments.read("--no-pages") ) use_pages = false;
   no_metadata = arguments.read("--no-metadata");
   arguments.read("-o", outputPath);
   arguments.read("--name", baseName);
   arguments.read("--offset", explicit_offset.x(), explicit_offset.y(), explicit_offset.z());
   arguments.read("--max_levels", config.max_levels);
   arguments.read("--max_leaf_size", config.decimateConfig.max_leaf_size);
   arguments.read("--max_bin_size", config.decimateConfig.max_bin_size);
   arguments.read("--max_page_levels", max_page_levels);
    
   // Setup paths
   string					databaseRelativePath;
   vector<string>	inputFilePaths;

	// read input file paths
   for(int argumentsPos=1;argumentsPos<arguments.argc();++argumentsPos)
   {
      if (!arguments.isOption(argumentsPos))
         inputFilePaths.push_back( arguments[argumentsPos] );
   }
	
   // must have at least one input file
   if ( inputFilePaths.empty() ) {
      cerr << "No input files." << endl;
      return 1;
   }

   // ensure output dir has trailing slash
   if ( outputPath.length() > 0 ) {
      if ( outputPath[outputPath.length()-1] != DIRECTORY_SEPARATOR )
         outputPath += DIRECTORY_SEPARATOR;
			
      // make directory if it doesn't exist
      osgDB::makeDirectory( outputPath );
   }
		
   // were we not explicitly given a base name?
   if ( baseName.length() == 0 ) {
      // if we were only given one input file, we can base the output filename on that
      if ( inputFilePaths.size() == 1 )
         baseName = osgDB::getStrippedName( inputFilePaths[0] );
      else
         baseName = "master"; // default output path
   }
    
   // build archive if desired
   osg::ref_ptr<osgDB::Archive>    archive;
    
   if ( ! arguments.read("--no-archive") ) {
      unsigned int indexBlockSizeHint=4096;
      string archivePath = outputPath + baseName + ".osga";
      archive = osgDB::openArchive( archivePath, osgDB::Archive::CREATE, indexBlockSizeHint);
      if ( archive.valid() )
         osg::notify(osg::NOTICE) << "Writing archive to \"" << archivePath << "\"." << endl;
      else
         osg::notify(osg::WARN) << "Archive could not be written to \"" << archivePath << "\". Is the path legal?" << endl;
   }

   if ( use_pages ) {
      if ( archive.valid() == false )
         databaseRelativePath = baseName + "_db" + DIRECTORY_SEPARATOR;
   } else {
      // disable pagination if there is no database path
      config.page_data_threshold_bytes = numeric_limits<unsigned long>::max();
      max_page_levels = -1;
   }
	
   config.decimateConfig.temporary_file_path = outputPath + databaseRelativePath;
	
	// build a master DataSet from the input files
   DataSet_OutOfCore *		inputDataSet = new DataSet_OutOfCore;
   inputDataSet->setLocation("r");
	
   vector<string>::const_iterator	inputFilePathsIter;
   for(inputFilePathsIter=inputFilePaths.begin(); inputFilePathsIter!=inputFilePaths.end(); inputFilePathsIter++)
   {
      string		filename = *inputFilePathsIter;
		
      if ( inputDataSet->add( filename, false ) )
      {
//         cout << "Added file \"" << filename << "\"...\n";
      }

   }

   // is there no data to process?
   if ( inputDataSet->getCount() == 0 ) {
      cerr << endl << "There is no data." << endl;
      return 1;
   }
	
   osg::BoundingBox	data_bounds = inputDataSet->getBounds();

   cout << "Bounds of Input Set: ";
   cout << "[" << data_bounds.xMin() << " - " << data_bounds.xMax() << "]";
   cout << "[" << data_bounds.yMin() << " - " << data_bounds.yMax() << "]";
   cout << "[" << data_bounds.zMin() << " - " << data_bounds.zMax() << "]";
   cout << "\n";
	
   // split into tree
   list< osg::ref_ptr<PageAssembler> >	assemblerQueue;
	
   // pack Metadata into oag::Arrays to save to disk
   PackMetadataIntoArrayVisitor	packMetadataIntoArrayVisitor;
	
   // use a scope for the master page, so memory is dealloc'ed afterward
   {
      osg::Matrixd	localToWorld = inputDataSet->getLocalToWorld();
		
      // apply explicit offset
      if ( explicit_offset.length2() > 0.001 )
         localToWorld *= osg::Matrixd::translate(explicit_offset);
		
      osg::ref_ptr<PageAssembler>	master_page = new PageAssembler( config );
      master_page->setLocation( "r" );
      master_page->setDatabasePath( databaseRelativePath );
		
//      cout << "Assembling master page...\n";
      master_page->add( inputDataSet );
      if ( !master_page->assemble( max_page_levels ) ) {
         osg::notify(osg::FATAL) << "Assemble failed for master file. Perhaps the data is too big to fit in memory?\n";
         return 1;
      }
		
      osg::Node*	root = master_page->getTopLevelNode();
		
      // pack metadata into OSG array format
      if ( ! no_metadata ) root->accept( packMetadataIntoArrayVisitor );

      // disable lighting
      osg::StateSet*	stateset = root->getOrCreateStateSet();
      stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE );

      osg::ref_ptr<osg::MatrixTransform> transform_node = 
         new osg::MatrixTransform;
      transform_node->setMatrix( localToWorld );
      transform_node->addChild( root );

      osg::CoordinateSystemNode* coord_sysnode_ptr=
         geofunc::generate_Coordinate_System_Node(
            UTM_zone,northern_hemisphere_flag);
      coord_sysnode_ptr->addChild(transform_node.get());

      if ( archive.valid() ) {
         string filepath = "master" + config.page_extension;
//         archive->writeNode( *transform_node, filepath );
         archive->writeNode( *coord_sysnode_ptr, filepath );
            
//         cout << "Master: " << archive->getMasterFileName() << "\n";
      } 
      else 
      {
         string filepath = outputPath + baseName + config.page_extension;
         if ( osgDB::writeNodeFile( *coord_sysnode_ptr,filepath ) )
//         if ( osgDB::writeNodeFile( *transform_node,filepath ) )
            cout << "Wrote master file: " << filepath << "\n";
         else
            cout << "Could not write file.\n";
      }
        
      const PageAssembler::PageAssemblerList&	masterChildren = master_page->getChildPages();
				
      for( unsigned int child_i=0; child_i < masterChildren.size(); child_i++ ) {
         assemblerQueue.push_back( masterChildren[child_i] );
      }
   }
	
   // process the remaining pages
//   cout << assemblerQueue.size() << " pages connected to master file.\n";
	
   // make directory if it doesn't exist
   if ( use_pages && archive.valid() == false ) {
      osgDB::makeDirectory( outputPath + databaseRelativePath );
   }
	
   while( !assemblerQueue.empty() )
   {
      osg::ref_ptr<PageAssembler>	page = assemblerQueue.front();
      assemblerQueue.pop_front();
		
      if ( !page.valid() ) continue;

      string     pageFilename = page->getLocation() + config.page_extension;
		
//      cout << "Assembling page " << pageFilename << "...\n";
      page->assemble( max_page_levels );
		
      osg::Node *		page_node = page->getTopLevelNode();
		
      if ( ! no_metadata )
         page_node->accept( packMetadataIntoArrayVisitor );
        
      if ( archive.valid() ) {
         archive->writeNode( *page_node, pageFilename );
      } else {
         string filepath = outputPath + databaseRelativePath + pageFilename;
         if ( osgDB::writeNodeFile( *page_node, filepath ) )
            osg::notify(osg::NOTICE) << "Wrote file: " << filepath << "\n";
         else
            osg::notify(osg::WARN) << "Could not write file.\n";
      }
        
      const PageAssembler::PageAssemblerList&	children = page->getChildPages();
		
      for( unsigned int child_i=0; child_i < children.size(); child_i++ ) {
         // we should add children in order, but at the front of the list
         // this way, in-core sets don't stick around for too long
         assemblerQueue.push_front( children[children.size()-1-child_i] );
      }
   }
		
   return 0;
}
