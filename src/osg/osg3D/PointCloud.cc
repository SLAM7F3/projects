// Note added on 10/25/07: read_extrainfo_from_TDP_file() method needs
// to be simplified (and perhaps even eliminated)!

// ==========================================================================
// POINTCLOUD class member function definitions
// ==========================================================================
// Last modified on 11/19/11; 11/27/11; 12/18/11; 1/6/12; 4/6/14
// ==========================================================================

#include <fstream>
#include <iostream>
#include <libtdp/tdp.h>
#include <libtdp/point_data.conf.h>
#include <vector>
#include <osg/Array>

#include <osg/Geometry>
#include <osg/Notify>
#include <osg/observer_ptr>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "math/adv_mathfuncs.h"
#include "math/basic_math.h"
#include "color/colorfuncs.h"
#include "osg/osgSceneGraph/ColormapPtrs.h"
#include "io/DataSetFile.h"
#include "threeDgraphics/draw3Dfuncs.h"
#include "math/fourvector.h"
#include "math/genmatrix.h"
#include "astro_geo/geocalibfuncs.h"
#include "ladar/ladarfuncs.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "math/mathfuncs.h"
#include "osg/osgfuncs.h"
#include "general/outputfuncs.h"
#include "geometry/plane.h"
#include "geometry/polygon.h"
#include "osg/osg3D/PointCloud.h"
#include "math/prob_distribution.h"
#include "osg/osgSceneGraph/scenegraphfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "osg/osg3D/tdpfuncs.h"
#include "math/threevector.h"
#include "osg/osgGeometry/TrianglesGroup.h"
#include "threeDgraphics/xyzpfuncs.h"

using namespace io;

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PointCloud::allocate_member_objects()
{
//   cout << "inside PointCloud::allocate_member_objects()" << endl;
   indices_refptr=new osg::UIntArray;
   ColormapPtrs_ptr=new ColormapPtrs(); 

// Recall ColormapPtrs is an osg::Referenced object.  Since it
// automatically goes out of scope, we should NOT explicitly delete it
// within PointCloud's destructor...

}		       

void PointCloud::initialize_member_objects()
{
//   cout << "PointCloud::initialize_member_objects()" << endl;
   
   Graphical_name="PointCloud";
   use_maps_to_color_flag=true;

   UTMzone="";
   UTMoffset=threevector(0,0,0);

   min_p=POSITIVEINFINITY;
   max_p=NEGATIVEINFINITY;
   metadata=NULL;

   shadows_ptr=NULL;
   normals_ptr=NULL;
   ladarimage_ptr=NULL;
   TrianglesGroup_ptr=NULL;
   ColorGeodeVisitor_ptr=NULL;
   SetupGeomVisitor_ptr=NULL;
   HiresDataVisitor_ptr=NULL;
   Grid_ptr=NULL;

   z_ColorMap_ptr->set_mapnumber(pass_ptr->get_PassInfo_ptr()->
                                  get_height_colormap_number());

// Note added on 5/12/09: If prob color map = grey, need to set some
// sort of grid_coloring flag s.t. we'll know to switch grid color
// from grey to purple whenever probs are displayed...

   p_ColorMap_ptr->set_mapnumber(pass_ptr->get_PassInfo_ptr()->
                                  get_prob_colormap_number());
   z_ColorMap_ptr->set_cyclic_frac_offset(
      pass_ptr->get_PassInfo_ptr()->
      get_height_colormap_cyclic_fraction_offset());
   p_ColorMap_ptr->set_cyclic_frac_offset(
      pass_ptr->get_PassInfo_ptr()->
      get_prob_colormap_cyclic_fraction_offset());

   ColormapPtrs_ptr->set_height_colormap_ptr(z_ColorMap_ptr);
   ColormapPtrs_ptr->set_prob_colormap_ptr(p_ColorMap_ptr);
}		       

PointCloud::PointCloud(
   Pass* currpass_ptr,
   LeafNodeVisitor* LNV_ptr,TreeVisitor* TV_ptr,ColorGeodeVisitor* CGV_ptr,
   SetupGeomVisitor* SGV_ptr,HiresDataVisitor* HRV_ptr,
   int ID,TrianglesGroup* TG_ptr): 
   DataGraph(3,ID,LNV_ptr,TV_ptr)
{	
//   cout << "inside PointCloud constructor" << endl;
   pass_ptr=currpass_ptr;

   allocate_member_objects();
   initialize_member_objects();
   
   TrianglesGroup_ptr=TG_ptr;
   ColorGeodeVisitor_ptr=CGV_ptr;
   SetupGeomVisitor_ptr=SGV_ptr;
   HiresDataVisitor_ptr=HRV_ptr;

   ColorGeodeVisitor_ptr->set_ColormapPtrs_ptr(ColormapPtrs_ptr);
   HiresDataVisitor_ptr->set_ColormapPtrs_ptr(ColormapPtrs_ptr);

// Save name of first input file within member variable data_filename.
// This information is useful provided PointCloud is built from just a
// single input file:

   data_filename=pass_ptr->get_first_filename();
}		       

PointCloud::~PointCloud()
{	
   delete shadows_ptr;
   delete normals_ptr;
   delete ladarimage_ptr;
}

// ==========================================================================
// Input member functions:   
// ==========================================================================

void PointCloud::parse_input_data()
{
//   cout << "inside PointCloud::parse_input_data()" << endl;

   if (pass_ptr->get_input_filetype()==Pass::xyz)
   {
      read_points_from_XYZ_file();
   }
   else if (pass_ptr->get_input_filetype()==Pass::xyzrgba)
   {
      use_maps_to_color_flag=false;
      read_points_from_XYZRGBA_file();
   }
   else
   {
      read_input_file();
      if (pass_ptr->get_input_filetype()==Pass::tdp)
      {
         read_extrainfo_from_TDP_file();
         if (use_maps_to_color_flag) 
            insert_fake_xyzp_points_for_dataviewer_coloring();
      }
   }
}

// ---------------------------------------------------------------------
// Member function read_input_file starts to use Ross Anderson's
// classes to parse input XYZP and TDP files.  As of 5/10/06, we
// extract 3-vertices, dependent "metadata" and XYZ bbox info from
// Ross' methods...

bool PointCloud::read_input_file()
{
//   cout << "inside PointCloud::read_input_file()" << endl;

   string banner="Reading data from input file:";
   outputfunc::write_banner(banner);

// Read input file's contents:

   cout << "data_filename = " << data_filename << endl;

   DataSetFile* input_file = DataSetFile::newDataSetFromFile(data_filename);
   if ( !input_file ) return false;
		
   n_points=input_file->getCount();
   cout << "n_points = " << get_npoints() << endl;

   vertices=input_file->getVertices();
   metadata = input_file->getMetadata();
//   cout << "metadata->dims() = " << metadata->dims() << endl;

   xyz_bbox = input_file->getBounds();
   cout << "xmin = " << xyz_bbox.xMin() 
        << " xmax = " << xyz_bbox.xMax() << endl;
   cout << "ymin = " << xyz_bbox.yMin() 
        << " ymax = " << xyz_bbox.yMax() << endl;
   cout << "zmin = " << xyz_bbox.zMin() 
        << " zmax = " << xyz_bbox.zMax() << endl;

// Read in colors from input file if they exist.  If not, instantiate
// new colors array and fill it with dummy black entries:

   colors=input_file->getColors();
//   cout <<  "colors.valid() = " << colors.valid() << endl;

   if (colors != NULL)
   {
      use_maps_to_color_flag=false;
   }
   else
   {
      cout << "colors = NULL; instantiating new colors array" << endl;

      colors = new osg::Vec4ubArray;
      colors->clear();
      colors->reserve(get_npoints());
      for (unsigned int n=0; n<get_npoints(); n++)
      {
         colors->push_back(osg::Vec4ub(0,0,0,0));
      }
      use_maps_to_color_flag=true;
   }
   
   for (unsigned int n=0; n<get_npoints(); n++)
   {

// Make sure probability values are real numbers rather than NaNs.  If
// so, change their value to -1:

      double currp=-1;

      if (metadata.get() != NULL && !mathfunc::my_isnan(metadata->get(n,0)))
      {
         currp=metadata->get(n,0);
//         cout << "curr_p = " << currp << endl;
//         p_values.push_back(currp);
      }

      max_p=basic_math::max(max_p,currp);
      min_p=basic_math::min(min_p,currp);
   } // loop over index n labeling points
   cout << "p_min = " << min_p << " p_max = " << max_p << endl;

// On December 27, 2007, we added the next few lines of code which
// renormalize p values so that they range from 0 to 1 for satellite
// optical imagery draping purposes:

   for (unsigned int n=0; n<get_npoints(); n++)
   {
      if (metadata.get() != NULL && !mathfunc::my_isnan(metadata->get(n,0)))
      {
         double curr_p=metadata->get(n,0);
         double renorm_p=(curr_p-min_p)/(max_p-min_p);
         metadata->set(n,0,renorm_p);
      }
   } // loop over index n labeling points

//   osg::Matrixd localToWorld = input_file->getLocalToWorld();
//   cout << "LocalToWorld = " << endl;

   delete input_file;
   input_file = NULL;
		
   if ( !vertices.valid() || (metadata.get() != NULL && !metadata.valid()) 
        || !xyz_bbox.valid() )
      return false;
		
// Check whether file is empty:
   
   if ( vertices->getNumElements() == 0 ) return false;

   return true;
}

// ---------------------------------------------------------------------
void PointCloud::read_extrainfo_from_TDP_file()
{ 
   cout << "inside PointCloud::read_extrainfo_from_TDP_file()" << endl;

   Tdp_file tdp_file;

   if (!tdp_file.file_open(data_filename))
   {
      cout << "Unable to open TDP file " << data_filename << endl;
      cout << "Exiting in PointCloud::read_extrainfo_from_TDP_file()"
           << endl;
      exit(1);
   }

   tdpfunc::parse_UTM_info(tdp_file,UTMzone,UTMoffset);
   cout << "UTMzone = " << UTMzone << endl;
   cout << "UTMoffset = " << UTMoffset << endl;

   osg::Vec3d offset(UTMoffset.get(0),UTMoffset.get(1),UTMoffset.get(2));
   for (unsigned int i=0; i<vertices->size(); i++)
   {
      vertices->at(i) += offset;
   }
   xyz_bbox.set(xyz_bbox.xMin()+offset.x(),xyz_bbox.yMin()+offset.y(),
                xyz_bbox.zMin()+offset.z(),xyz_bbox.xMax()+offset.x(),
                xyz_bbox.yMax()+offset.y(),xyz_bbox.zMax()+offset.z());
   tdp_file.file_close();
}
// ---------------------------------------------------------------------
// As of 4/2/07, Ross Anderson's classes do not support parsing of XYZ
// files.  So we have to continue to use our old (and ugly!) methods
// to read in .xyz files.

void PointCloud::read_points_from_XYZ_file()
{ 
//   cout << "inside PointCloud::read_points_from_XYZ_file()" << endl;

   ifstream infile;
   infile.open(data_filename.c_str(),std::ios::in | std::ios::binary );
   if (!infile)
   {
      cout << "File not found: " << data_filename << endl;
      exit(1);
   }

   n_points=xyzpfunc::npoints_inside_xyz_file(data_filename);

   vertices=new osg::Vec3Array;
   vertices->clear();
   vertices->reserve(get_npoints());

   float xyz[3];
   infile.read( reinterpret_cast<char *>(xyz), sizeof(float)*3 );
   while (infile)
   {
      vertices->push_back(osg::Vec3(xyz[0],xyz[1],xyz[2]));
      xyz_bbox.expandBy(xyz[0],xyz[1],xyz[2]);
      infile.read( reinterpret_cast<char *>(xyz), sizeof(float)*3 );
//      cout << "x = " << xyz[0] << " y = " << xyz[1] << " z = " << xyz[2]
//           << endl;
   }
   infile.close();

   cout << "n_points = " << get_npoints() << endl;
   cout << "bbox xmin = " << xyz_bbox.xMin() 
        << " bbox xmax = " << xyz_bbox.xMax() << endl;
   cout << "bbox ymin = " << xyz_bbox.yMin() 
        << " bbox ymax = " << xyz_bbox.yMax() << endl;
   cout << "bbox zmin = " << xyz_bbox.zMin() 
        << " bbox zmax = " << xyz_bbox.zMax() << endl;
}

// ---------------------------------------------------------------------
// As of 5/11/06, Ross Anderson's classes do not support parsing of
// XYZRGBA files.  So we have to continue to use our old (and ugly!)
// methods to read in .xyzrgba files.

void PointCloud::read_points_from_XYZRGBA_file()
{ 
//   cout << "inside PC::read_points_from_XYZRGBA" << endl;
   ifstream infile;
   infile.open(data_filename.c_str(),std::ios::in | std::ios::binary );
   if (!infile)
   {
      cout << "File not found: " << data_filename << endl;
      exit(1);
   }

   n_points=xyzpfunc::npoints_inside_xyzp_file(data_filename);
//   cout << "n_points = " << n_points << endl;

   vertices=new osg::Vec3Array;
   vertices->clear();
   vertices->reserve(get_npoints());

   metadata=new model::Metadata;

   colors = new osg::Vec4ubArray;
   colors->clear();
   colors->reserve(get_npoints());

/*
   typedef union RGBAStruct
   {
     float p;
     unsigned char rgba[4];
   };
   union RGBAStruct RGBA;
*/

   union RGBAStruct
   {
     float p;
     unsigned char rgba[4];
   };
   union RGBAStruct RGBA;

   float xyzp[4];
   infile.read( reinterpret_cast<char *>(xyzp), sizeof(float)*4 );

   while (infile)
   {
      vertices->push_back(osg::Vec3(xyzp[0],xyzp[1],xyzp[2]));
      xyz_bbox.expandBy(xyzp[0],xyzp[1],xyzp[2]);

      RGBA.p=xyzp[3];

//         cout << "p = " << xyzp[3] << endl;
//         cout << "r = " 
//              << stringfunc::unsigned_char_to_ascii_integer(RGBA.rgba[0])
//              << " g = " 
//              << stringfunc::unsigned_char_to_ascii_integer(RGBA.rgba[1])
//              << " b = " 
//              << stringfunc::unsigned_char_to_ascii_integer(RGBA.rgba[2])
//              << " a = " 
//              << stringfunc::unsigned_char_to_ascii_integer(RGBA.rgba[3])
//              << endl;

      colors->push_back(osg::Vec4ub(
         RGBA.rgba[0],RGBA.rgba[1],RGBA.rgba[2],RGBA.rgba[3]));
      infile.read( reinterpret_cast<char *>(xyzp), sizeof(float)*4 );
         
//      double r=colors->back().r();
//      double g=colors->back().g();
//      double b=colors->back().b();
//      double a=colors->back().a();
//      cout << "r = " << r << " g = " << g << " b = " << b 
//           << " a = " << a << endl;

   }
   infile.close();

//   check_colors_array();

   cout << "n_points = " << get_npoints() << endl;
   cout << "bbox xmin = " << xyz_bbox.xMin() 
        << " bbox xmax = " << xyz_bbox.xMax() << endl;
   cout << "bbox ymin = " << xyz_bbox.yMin() 
        << " bbox ymax = " << xyz_bbox.yMax() << endl;
   cout << "bbox zmin = " << xyz_bbox.zMin() 
        << " bbox zmax = " << xyz_bbox.zMax() << endl;
}

// ---------------------------------------------------------------------
// Method insert_fake_xyzp_points_for_dataviewer_coloring adds a
// spread of p values ranging from 0 to 1 in order to fix the
// dataviewer's probability colormap:

// p value      JET color	JET+white	Hue+value

//   0.0        indigo		indigo		dark red
//   0.1       	dark blue	dark blue	orange
//   0.2       	dark blue	dark blue	dark yellow
//   0.3        medium blue	medium blue	olive
//   0.4	blue-green	blue-green	bright green
//   0.5       	green		green		dark green
//   0.6       	yellow		yellow		dark cyan
//   0.7       	orange		orange		blue
//   0.8       	red-orange	red		indigo
//   0.9       	red		brick red	purple
//   1.0       	brick red	white		white

// p value      JET+white	Hue+value

//   0.15       royal blue	light orange
//   0.45       green		green
//   0.55	yellow		dark olive
//   0.65 	light orange	bright blue
//   0.9  	brick red       purple
//   0.95  	pink		bright violet
//   1		white		white
   
void PointCloud::insert_fake_xyzp_points_for_dataviewer_coloring()
{
   if (metadata.get() != NULL)
   {
      for (unsigned int i=0; i<=20; i++)
      {
         vertices->push_back(vertices->at(0));
         double p_value=i*0.05;  

         vector<float> row;
         row.push_back(p_value);

// QUESTION !?!  Shouldn't the following appendRow statement appear
// outside the for loop?

         metadata->appendRow(row);

         max_p=basic_math::max(max_p,p_value);
         min_p=basic_math::min(min_p,p_value);
         colors->push_back(osg::Vec4ub(0,0,0,0));

         n_points++;
      }
   }
}

// ---------------------------------------------------------------------
void PointCloud::read_normals_from_xyz_file()
{ 
   string normals_filename;
   cout << "Enter name of XYZ file containing normals information:" << endl;
   cin >> normals_filename;
   normals_ptr=xyzpfunc::read_xyz_float_data(normals_filename);
}

void PointCloud::read_normals_from_xyzp_file()
{ 
   string normals_filename;
   cout << "Enter name of XYZP file containing normals information:" << endl;
   cin >> normals_filename;

   vector<fourvector>* normals_xyzp_ptr=
      xyzpfunc::read_xyzp_float_data(normals_filename);

   normals_ptr=new vector<threevector>;
   for (unsigned int i=0; i<normals_xyzp_ptr->size(); i++)
   {
      fourvector norm4( (*normals_xyzp_ptr)[i] );
      normals_ptr->push_back(threevector(
         norm4.get(0),norm4.get(1),norm4.get(2)));
   }
   delete normals_xyzp_ptr;
}

// ==========================================================================
// Scene graph generation member functions
// ==========================================================================

// Member function GenerateCloudGraph first tries to read in an
// existing datagraph from file.  If it is not successful, it
// generates a datagraph using Ross Anderson's summer 2005 approach to
// building a tree containing approximate geodes and LODs.  

osg::Node* PointCloud::GenerateCloudGraph(bool index_tree_flag)
{
//   cout << "inside PointCloud::GenerateCloudGraph()" << endl;

   if (pass_ptr != NULL) ReadGraph();

   if (get_DataNode_ptr()==NULL)
   {
      parse_input_data();

      indices_refptr->clear();
      indices_refptr->reserve(n_points);
      
      for (unsigned int n=0; n<n_points; n++) indices_refptr->push_back(n);

      Generate_Ross_Tree(
         vertices.get(),metadata.get(),colors.get(),indices_refptr.get(),
         xyz_bbox,index_tree_flag);
   }

   InitializeCloudGraph();
   return get_DataNode_ptr();
};

// ---------------------------------------------------------------------
// Member function InitializeCloudGraph runs SetupGeomVisitor and
// LeafNodeVisitor starting at the top node of the datagraph.  It also
// computes the datagraph's XYZ bounding box and initializes point
// colors.

void PointCloud::InitializeCloudGraph()
{
//   cout << "inside PointCloud::InitializeCloudGraph()" << endl;

// Set label of very top node in pointcloud DataGraph so that it can
// subsequently be distinguished from other non-pointcloud DataGraphs:

   get_DataNode_ptr()->setName("PointCloud");

   get_DataNode_ptr()->accept(*SetupGeomVisitor_ptr);

// Traverse through instantiated DataGraph and save geode addresses within 
// STL vector members of *LeafNodeVisitor_ptr:

   refresh_leafnodevisitor_geodes();

// Initialize protected member variable curr_leaf_geometry_ptr to
// point to zeroth leaf geometry:

   curr_leaf_geometry_refptr=get_leaf_geometry(0).first;

   compute_xyz_and_hyper_bboxes();

   if (use_maps_to_color_flag)
   {
      compute_points_thresholds();
   }
};

// ---------------------------------------------------------------------
// Member function Generate_Ross_Tree implements Ross Anderson's
// clever (circa summer 2005) approach to storing point information
// within a tree datastructure.  Ross does not assume that the XY
// distribution of points is basically square.  Instead, he bisects in
// whichever direction is largest.  Moreover, his approach implements
// level-of-detail ideas to avoid having to render far-away regions of
// the scene in unnecessary detail.

void PointCloud::Generate_Ross_Tree(
   const osg::Vec3Array* vertices_ptr, const model::Metadata* metadata_ptr,
   const osg::Vec4ubArray* colors_ptr, const osg::UIntArray* indices_ptr, 
   const osg::BoundingBox& box, bool index_tree_flag)
{ 
//   cout << "inside PointCloud::Generate_Ross_Tree()" << endl;
   indices_stored_flag=index_tree_flag;
   
   string banner="Generating Ross' ancient scene graph:";
   outputfunc::write_banner(banner);

   int level=0;
   int parent_ID=-1;
   set_DataNode_ptr(build_datagraph_tree(
      level,parent_ID,vertices_ptr,metadata_ptr,colors_ptr,indices_ptr,box));
}

// ---------------------------------------------------------------------
// Ross Anderson's recursive method to build tree data structure for
// data objects within overall scene graph

osg::Node* PointCloud::build_datagraph_tree( 
   int level,int parent_ID,
   const osg::Vec3Array* vertices_ptr,const model::Metadata* metadata_ptr, 
   const osg::Vec4ubArray* colors_ptr,const osg::UIntArray* indices_ptr,
   const osg::BoundingBox& box)
{
//   cout << "inside PointCloud::build_datagraph_tree()" << endl;
//   cout << "level = " << level << endl;
//   cout << "metadata_ptr = " << metadata_ptr << endl;
//   cout << "colors_ptr = " << colors_ptr << endl;
//   cout << "indices_ptr = " << indices_ptr << endl;

   const long max_bin_size = 10000;	
   const long max_leaf_size = 2*max_bin_size;
   const long natural_px_size = 200;

   if (level == int(level_ID.size()))
   {
      level_ID.push_back(0);
   }
   else
   {
      level_ID[level]=level_ID[level]+1;
   }

// Are there any points?

   if ( indices_ptr->size() == 0 ) 
   {
      indices_ptr->unref();
      return NULL;
   }
		 
// Do we need to split up input points further?

   if ( int(indices_ptr->size()) > max_leaf_size ) 
   {

// Instantiate a LOD at current level in tree:

      osg::LOD*	lodGroup = new osg::LOD;

      string LOD_name="LOD level="+stringfunc::number_to_string(level)
         +" level_ID="+stringfunc::number_to_string(level_ID[level])
         +" parent_ID="+stringfunc::number_to_string(parent_ID);
      lodGroup->setName(LOD_name);
//      cout << LOD_name << endl;

      lodGroup->setRangeMode( osg::LOD::PIXEL_SIZE_ON_SCREEN );

      lodGroup->setCenter( box.center() );
      lodGroup->setRadius( box.radius() );
      lodGroup->setInitialBound(lodGroup->computeBound());
		
//      osg::BoundingSphere sphere=lodGroup->getBound();
//      cout << "LOD sphere radius = " << sphere._radius << endl;
//      cout << "LOD sphere center = " << sphere._center.x() << ","
//           << sphere._center.y() << "," << sphere._center.z() << endl;

// Split current bounding box along its longest axis:

      float middle;
      float width = box.xMax() - box.xMin();
      float height = box.yMax() - box.yMin();
      osg::BoundingBox lower_bbox = box, upper_bbox = box;
      if ( width > height ) 
      {
         middle = box.center().x();
         lower_bbox.xMax() = middle;
         upper_bbox.xMin() = middle;
      } 
      else 
      {
         middle = box.center().y();
         lower_bbox.yMax() = middle;
         upper_bbox.yMin() = middle;
      }
		
// Bin indices into two halves:

      unsigned int index;
      osg::UIntArray* indicesLower = new osg::UIntArray;
      osg::UIntArray* indicesUpper = new osg::UIntArray;
      for( unsigned int i=0; i < indices_ptr->size(); ++i ) 
      {
         index = indices_ptr->at(i);
         if ( lower_bbox.contains( vertices_ptr->at( index ) ) )
            indicesLower->push_back( index );
         else
            indicesUpper->push_back( index );
      }
				
// Build low resolution approximation geode:

      osg::Geometry* geometry = new osg::Geometry;
      if (indices_stored_flag)
      {
         osg::ref_ptr<osg::UIntArray> reducedIndices = new osg::UIntArray;
         reducedIndices->reserve( max_bin_size );
         for( int i=0; i < max_bin_size; ++i )
            reducedIndices->push_back( 
               indices_ptr->at( random() % indices_ptr->size() ) );	
         // !!! Might take 1 index more than once

         geometry->setVertexArray(const_cast<osg::Vec3Array*>(vertices_ptr));
         osg::DrawElementsUInt* ReducedIndices_ptr = 
            new osg::DrawElementsUInt( 
               GL_POINTS, max_bin_size, (GLuint*)reducedIndices->
               getDataPointer() );
         geometry->addPrimitiveSet(ReducedIndices_ptr);
         geometry->setColorArray(const_cast<osg::Vec4ubArray*>(colors_ptr));
         geometry->setUserData(const_cast<model::Metadata*>(metadata_ptr));
      }
      else
      {
         osg::Vec3Array* reduced_vertices_ptr=new osg::Vec3Array;
         reduced_vertices_ptr->reserve(max_bin_size);

         bool fill_color_array=false;
         osg::Vec4ubArray* reduced_colors_ptr=NULL;
         if (colors_ptr != NULL)
         {
            fill_color_array=(colors_ptr->size()==0);
            reduced_colors_ptr=scenegraphfunc::instantiate_color_array(
               max_bin_size,fill_color_array,geometry,
               scenegraphfunc::get_mutable_colors_label());
         }

         model::Metadata* reduced_metadata_ptr=NULL;
         if (metadata_ptr != NULL)
         {
            reduced_metadata_ptr=new model::Metadata(metadata_ptr->dims());
            reduced_metadata_ptr->reserve(max_bin_size);
         }

         for (unsigned int i=0; i < max_bin_size; ++i)
         {
            unsigned int j=indices_ptr->at( random() % indices_ptr->size() );
            reduced_vertices_ptr->push_back(vertices_ptr->at(j));
            if (metadata_ptr != NULL)
            {
               reduced_metadata_ptr->appendRow( *metadata_ptr, j );
            }
            if (colors_ptr != NULL && colors_ptr->size() > 0)
               reduced_colors_ptr->push_back(colors_ptr->at(j));
          
         // !!! Might take 1 index more than once
         } // loop over index i

         geometry->setVertexArray(reduced_vertices_ptr);
         osg::DrawArrays* ReducedSet_ptr=new osg::DrawArrays(
            GL_POINTS, 0, reduced_vertices_ptr->getNumElements());
         geometry->addPrimitiveSet(ReducedSet_ptr);
         geometry->setColorArray(reduced_colors_ptr);
         geometry->setUserData(reduced_metadata_ptr);
      } // indices_stored_flag conditional
   
      indices_ptr->unref();		

      geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
      
      osg::Geode* approx_geode = new osg::Geode;
      approx_geode->addDrawable( geometry );
      approx_geode->setInitialBound(lodGroup->getBound());
      lodGroup->addChild( approx_geode, 0, natural_px_size );

      int childindex=lodGroup->getChildIndex(approx_geode);

      string approx_geode_name="APPROX_GEODE level="
         +stringfunc::number_to_string(level+1)
         +" LODlevelID="+stringfunc::number_to_string(level_ID[level])
         +" child #"+stringfunc::number_to_string(childindex);
      approx_geode->setName(approx_geode_name);
//      cout << approx_geode_name << endl;

//      osg::BoundingBox bbox(geometry->getInitialBound());
//      osg::Vec3 corner0=bbox.corner(0);
//      osg::Vec3 corner7=bbox.corner(7);
//      cout << "approx geom corner0 = " << corner0.x() << ","
//           << corner0.y() << "," << corner0.z() << endl;
//      cout << "approx geom corner7 = " << corner7.x() << ","
//           << corner7.y() << "," << corner7.z() << endl;

// Bin the two halves (show when 0 < px size < 1 ):

      osg::Node* lowerNode = build_datagraph_tree( 
         level+1, level_ID[level], vertices_ptr, metadata_ptr, colors_ptr, 
         indicesLower, lower_bbox );
      osg::Node* upperNode = build_datagraph_tree( 
         level+1, level_ID[level], vertices_ptr, metadata_ptr, colors_ptr, 
         indicesUpper, upper_bbox );
    
// Show the child node if the on-screen size is larger than the
// natural size:

      if (lowerNode != NULL)
      {
         lodGroup->addChild( lowerNode,natural_px_size, FLT_MAX );
         childindex=lodGroup->getChildIndex(lowerNode);
         lowerNode->setName(
            lowerNode->getName()+" child #"+stringfunc::number_to_string(
               childindex));
      }
      if (upperNode != NULL)
      {
         lodGroup->addChild( upperNode,natural_px_size, FLT_MAX );
         childindex=lodGroup->getChildIndex(upperNode);
         upperNode->setName(
            upperNode->getName()+" child #"+stringfunc::number_to_string(
               childindex));
      }
//      cout << endl;

      return lodGroup;
   } // indices_ptr->size() > max_bin_size conditional

// Leaf geode. Store all data points:

   osg::Geometry* geometry = new osg::Geometry;
   if (indices_stored_flag)
   {
      geometry->setVertexArray(const_cast<osg::Vec3Array*>(vertices_ptr));
      osg::DrawElementsUInt* LeafIndices_ptr = new osg::DrawElementsUInt( 
         GL_POINTS, indices_ptr->getNumElements(), 
         (GLuint*)indices_ptr->getDataPointer() );
      geometry->addPrimitiveSet(LeafIndices_ptr);
      geometry->setColorArray(const_cast<osg::Vec4ubArray*>(colors_ptr));
      geometry->setUserData(const_cast<model::Metadata*>(metadata_ptr));
   }
   else
   {
      unsigned int leaf_size=indices_ptr->size();
      osg::Vec3Array* leaf_vertices_ptr=new osg::Vec3Array;
      leaf_vertices_ptr->reserve(leaf_size);

      bool fill_color_array=false;
      osg::Vec4ubArray* leaf_colors_ptr=NULL;

      if (colors_ptr != NULL)
      {
         fill_color_array=(colors_ptr->size()==0);
         leaf_colors_ptr=scenegraphfunc::instantiate_color_array(
            leaf_size,fill_color_array,geometry,
            scenegraphfunc::get_mutable_colors_label());
      }
      
      model::Metadata* leaf_metadata_ptr=NULL;
      if (metadata_ptr != NULL)
      {
         leaf_metadata_ptr=new model::Metadata(metadata_ptr->dims());
         leaf_metadata_ptr->reserve(leaf_size);
      }
      for (unsigned int i=0; i < leaf_size; i++)
      {
         unsigned int j=indices_ptr->at(i);
         leaf_vertices_ptr->push_back(vertices_ptr->at(j));
         if (metadata_ptr != NULL)
         {
            leaf_metadata_ptr->appendRow( *metadata_ptr, j );
         }
         if (colors_ptr != NULL && colors_ptr->size() > 0)
         {
            leaf_colors_ptr->push_back(colors_ptr->at(j));
         }
      } // loop over index i labeling leaf vertices
      
      geometry->setVertexArray(leaf_vertices_ptr);
      osg::DrawArrays* LeafSet_ptr=new osg::DrawArrays(
         GL_POINTS, 0, leaf_vertices_ptr->getNumElements());
      geometry->addPrimitiveSet(LeafSet_ptr);
      geometry->setColorArray(leaf_colors_ptr);
      geometry->setUserData(leaf_metadata_ptr);
   } // indices_stored conditional
   
   indices_ptr->unref();	

   geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

   osg::Geode* leaf_geode = new osg::Geode;
   string leaf_name="LEAF_GEODE level="+stringfunc::number_to_string(level)
      +" levelID="+stringfunc::number_to_string(level_ID[level])
      +" parent_ID="+stringfunc::number_to_string(parent_ID);
//   cout << leaf_name << endl;
   leaf_geode->setName(leaf_name);
   leaf_geode->addDrawable(geometry);

//   osg::BoundingBox bbox(geometry->getInitialBound());
//   osg::Vec3 corner0=bbox.corner(0);
//   osg::Vec3 corner7=bbox.corner(7);
//   cout << "leaf geom corner0 = " << corner0.x() << ","
//        << corner0.y() << "," << corner0.z() << endl;
//   cout << "leaf geom corner7 = " << corner7.x() << ","
//        << corner7.y() << "," << corner7.z() << endl;

   leaf_geode->setInitialBound(
      osg::BoundingSphere(box.center(),box.radius()));
//   osg::BoundingSphere sphere=leaf_geode->getBound();
//   cout << "leaf sphere radius = " << sphere._radius << endl;
//   cout << "leaf sphere center = " << sphere._center.x() << ","
//        << sphere._center.y() << "," << sphere._center.z() << endl;
//   cout << endl;

   return leaf_geode;
}

// ---------------------------------------------------------------------
// Member function refresh_leafnodevisitor_geodes refreshes the list
// of geodes within *LeafNodeVisitor_refptr which may have been
// updated via Database paging since the last time this method was
// called.

void PointCloud::refresh_leafnodevisitor_geodes()
{
   LeafNodeVisitor_refptr->get_Geodes_ptr()->clear();
   LeafNodeVisitor_refptr->get_LeafGeodes_ptr()->clear();
   get_DataNode_ptr()->accept(*(LeafNodeVisitor_refptr.get()));
}

// ---------------------------------------------------------------------
// Member function get_zeroth_vertex returns a threevector containing
// the very first XYZ location in world space coordinates.

threevector PointCloud::get_zeroth_vertex()
{
   refresh_leafnodevisitor_geodes();

   pair<osg::observer_ptr<osg::Geode>,osg::Matrix> p=LeafNodeVisitor_refptr->
      get_LeafGeodes_ptr()->at(0);
   osg::Geometry* curr_Geometry_ptr=scenegraphfunc::get_geometry(
      (p.first).get());

   osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
      curr_Geometry_ptr->getVertexArray());
   osg::Vec3 zeroth_vertex=curr_vertices_ptr->at(0);
   
   osg::Matrix LocalToWorld(p.second);

   float curr_x=zeroth_vertex.x()*LocalToWorld(0,0)
      +zeroth_vertex.y()*LocalToWorld(1,0)
      +zeroth_vertex.z()*LocalToWorld(2,0)+LocalToWorld(3,0);
   float curr_y=zeroth_vertex.x()*LocalToWorld(0,1)
      +zeroth_vertex.y()*LocalToWorld(1,1)
      +zeroth_vertex.z()*LocalToWorld(2,1)+LocalToWorld(3,1);
   float curr_z=zeroth_vertex.x()*LocalToWorld(0,2)
      +zeroth_vertex.y()*LocalToWorld(1,2)
      +zeroth_vertex.z()*LocalToWorld(2,2)+LocalToWorld(3,2);
   return threevector(curr_x,curr_y,curr_z);
}

// ==========================================================================
// Color manipulation member functions
// ==========================================================================

// Member function compute_points_thresholds loops over all vertices
// currently stored within the DataGraph and forms their X, Y and Z
// distributions.  It then sets the ColorMaps extremal thresholds for
// these spatial directions equal to the independent values
// corresponding to the min_threshold_frac and max_threshold_frac
// points in these distributions.

void PointCloud::compute_points_thresholds()
{ 
//   cout << "inside PointCloud::compute_points_thresholds()" << endl;
   string banner="Computing points' limits for colormapping purposes:";
   outputfunc::write_banner(banner);

   min_threshold_frac=pass_ptr->get_PassInfo_ptr()->
      get_min_threshold_fraction();
   max_threshold_frac=pass_ptr->get_PassInfo_ptr()->
      get_max_threshold_fraction();

   if (min_threshold_frac < 0) min_threshold_frac=0;
   if (max_threshold_frac < 0) max_threshold_frac=1;

   cout << "min_threshold_frac = " << min_threshold_frac
        << " max_threshold_frac = " << max_threshold_frac << endl;

   compute_prob_dist();

// Check whether X,Y,Z,P thresholds were specified within input .pkg
// files.  If so, reset min and max thresholds:

   double specified_min_threshold,specified_max_threshold;
   for (unsigned int i=0; i<4; i++)
   {
//      cout << "i = " << i << endl;
      specified_min_threshold=
         pass_ptr->get_PassInfo_ptr()->get_min_threshold(i);
      specified_max_threshold=
         pass_ptr->get_PassInfo_ptr()->get_max_threshold(i);

      if (specified_min_threshold < 0.5*POSITIVEINFINITY)
      {
         min_threshold[i]=specified_min_threshold;
      }
      if (specified_max_threshold > 0.5*NEGATIVEINFINITY)
      {
         max_threshold[i]=specified_max_threshold;
      }
   } // loop over index i

   reset_ColorMap_thresholds();
}

// ------------------------------------------------------------------------
// Member function compute_prob_dist loops over every leaf geometry
// within the DataGraph and extracts its vertices.  It then computes
// probability distributions for the cumulative X, Y and Z values.
// The min_threshold_frac and max_threshold_frac percentiles for the
// X, Y and Z data are returned within output STL vectors min_thresh
// and max_thresh.  Colormap thresholds can be calculated even if the
// point cloud is read in from .ive rather than .xyzp or .tdp files.

void PointCloud::compute_prob_dist()
{ 
//   cout << "inside PointCloud::compute_prob_dist()" << endl;
   
   osg::Vec3Array* vertices_ptr=NULL;
   osg::ref_ptr<osg::Vec3Array> vertices_refptr=new osg::Vec3Array;

   if (indices_stored_flag)
   {
      osg::Geometry* curr_Geometry_ptr=get_leaf_geometry(0).first;
//    curr_MatrixTransform=get_leaf_geometry(0).second;
      vertices_ptr=dynamic_cast<osg::Vec3Array*>(
         curr_Geometry_ptr->getVertexArray());
      for (unsigned int i=0; i<vertices_ptr->size(); i++)
      {
         vertices_ptr->at(i)=vertices_ptr->at(i)*get_leaf_geometry(0).second;
      }
   }
   else
   {
      if (vertices.valid())
      {
         vertices_ptr=vertices.get();
      }
      else
      {
         vertices_ptr=vertices_refptr.get();
         vertices_ptr->reserve(get_ntotal_leaf_vertices());

         for (unsigned int g=0; g<LeafNodeVisitor_refptr->get_n_leaf_geodes();
              g++)
         {
            osg::Geometry* curr_Geometry_ptr=get_leaf_geometry(g).first;
            if (curr_Geometry_ptr==NULL) continue;

//         curr_MatrixTransform=get_leaf_geometry(g).second;
            osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
               curr_Geometry_ptr->getVertexArray());

            for (unsigned int i=0; i<curr_vertices_ptr->size(); i++)
            {
               vertices_ptr->push_back( curr_vertices_ptr->at(i) * 
                                        get_leaf_geometry(g).second ) ;
            }
         } // loop over index g labeling leaf geodes
      } // vertices.valid() flag
   } // indices_stored_flag conditional

// Recall OSG geometries do not necessarily contain vertices.  So do
// not attempt to compute prob_distribution unless n_vertices is
// larger than some reasonable value:

   if (vertices_ptr->size() > 10)
   {
      int n_prob_bins=10000;
      for (unsigned int j=0; j<3; j++)
      {
         prob_distribution prob(j,vertices_ptr,n_prob_bins);
         min_threshold.push_back(
            prob.find_x_corresponding_to_pcum(min_threshold_frac));
         max_threshold.push_back(
            prob.find_x_corresponding_to_pcum(max_threshold_frac));
      }
   }
   else
   {
      min_threshold.push_back(get_xyz_bbox().xMin());
      min_threshold.push_back(get_xyz_bbox().yMin());
      min_threshold.push_back(get_xyz_bbox().zMin());

      max_threshold.push_back(get_xyz_bbox().xMax());
      max_threshold.push_back(get_xyz_bbox().yMax());
      max_threshold.push_back(get_xyz_bbox().zMax());
   }
   
//   cout << "min thresholds = " << endl;
//   templatefunc::printVector(min_threshold);
//   cout << "max thresholds = " << endl;
//   templatefunc::printVector(max_threshold);

//   cout << "min z threshold = " << min_threshold[2]
//        << " max z threshold = " << max_threshold[2] << endl;

}

// ------------------------------------------------------------------------
void PointCloud::reset_ColorMap_thresholds()
{ 
//   cout << "inside PointCloud::reset_ColorMap_thresholds()" << endl;
   for (unsigned int j=0; j<3; j++)
   {
      z_ColorMap_ptr->set_min_value(j,min_threshold[j]);
      z_ColorMap_ptr->set_max_value(j,max_threshold[j]);
      z_ColorMap_ptr->set_min_threshold(j,min_threshold[j]);
      z_ColorMap_ptr->set_max_threshold(j,max_threshold[j]);
   }
   p_ColorMap_ptr->set_min_value(3,0);
   p_ColorMap_ptr->set_max_value(3,1);
   p_ColorMap_ptr->set_min_threshold(3,0);
   p_ColorMap_ptr->set_max_threshold(3,1);

   for (unsigned int j=0; j<3; j++)
   {
      cout << "j = " << j 
           << " min_threshold = " << z_ColorMap_ptr->get_min_threshold(j)
           << " max_threshold = " << z_ColorMap_ptr->get_max_threshold(j)
           << endl;
   }
   int j=3;
   cout << "j = 3 min_threshold = " << p_ColorMap_ptr->get_min_threshold(j)
        << " max_threshold = " << p_ColorMap_ptr->get_max_threshold(j) 
        << endl;
//   outputfunc::enter_continue_char();
}

// ------------------------------------------------------------------------
// Member function reload_colormap_array loops over every point within
// the cloud.  It computes each point's fractional value on the
// current dependent variable scale.  This method then assigns an
// integer index ranging from 0 to max_colors-1 which is proportional
// to that fractional value.  The RGB values corresponding to that
// integer index are retrieved from the current colormap and converted
// into byte form (unsigned chars).  Member array colors are packed
// within the RGB bytes plus an additional nominal alpha channel byte.

// Every XYZ point has a 4-byte RGBA color associated with it in
// member array colors.  

// Note added on 6/6/06: If indices have been stored within the data
// scene graph, then we need to assign colors to the global color
// array contained within the PointCloud class.  On the other hand, if
// buckets of vertices, colors and metadata are stored in the data
// graph, then we need to explicitly iterate over every geometry
// within the data graph and make sure that every vertex is assigned a
// new color...

void PointCloud::reload_colormap_array()
{
//   cout << "inside PointCloud::reload_colormap_array()" << endl;
//   cout << "use_maps_to_color_flag = " << use_maps_to_color_flag << endl;
//   cout << "get_store_indices_flag() = " << get_store_indices_flag() << endl;

   if (use_maps_to_color_flag)
   {
      if (get_store_indices_flag())
      {
         int n_depend_var=z_ColorMap_ptr->get_dependent_var();
         double curr_value=0;

         ColorMap* curr_ColorMap_ptr=z_ColorMap_ptr;
         for (unsigned int i=0; i<n_points; i++)
         {
            switch(n_depend_var)
            {
               case 0: 
                  curr_value=vertices->at(i).x();
                  break;
               case 1: 
                  curr_value=vertices->at(i).y();
                  break;
               case 2: 
                  curr_value=vertices->at(i).z();
                  break;
               case 3:
                  if (metadata != NULL)
                  {
                     curr_value=metadata->get(i,0);
                     curr_ColorMap_ptr=p_ColorMap_ptr;
                  }
                  else
                  {
                     curr_value=vertices->at(i).z();
                  }
                  
                  if (shadows_ptr != NULL)
                  {
                     if ( shadows_ptr->at(i) ) curr_value=1.0;
                  }
                  break;
            } // switch on n_depend_var
            colors->at(i)=curr_ColorMap_ptr->retrieve_curr_color(curr_value);
         } // loop over index i labeling points in cloud
      } 
      else
      {
         p_ColorMap_ptr->IncrementUpdateIndex();
         z_ColorMap_ptr->IncrementUpdateIndex();

         ColorGeodeVisitor_ptr->initialize_LocalToWorld();
         get_DataNode_ptr()->accept( *ColorGeodeVisitor_ptr );
         
      } // datagraph stores indices conditional
   } // use_maps_to_color_flag boolean conditional
}

// ---------------------------------------------------------------------
// Member pure_hues converts computes the hue, saturation and
// intensity for every colored point within the cloud.  It then sets
// the saturation and intensity values equal to unity.  This method
// converts the modified pure hue back to RGB and resets the colored
// points' entries within the RGBA member array colors.

void PointCloud::pure_hues()
{
   double s_final=1;
//   cout << "Enter final saturation value:" << endl;
//   cin >> s_final;
   double v_final=1;
   cout << "Enter final intensity value:" << endl;
   cin >> v_final;

   const unsigned char alpha_byte=
      static_cast<unsigned char>(stringfunc::ascii_integer_to_char(255));

   for (osg::Geometry* curr_Geometry_ptr=
           get_curr_geometry(); curr_Geometry_ptr != NULL; 
        curr_Geometry_ptr=get_next_geometry())
   {
      osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
         curr_Geometry_ptr->getVertexArray());
      osg::Vec4ubArray* curr_colors_ptr=dynamic_cast<osg::Vec4ubArray*>(
         curr_Geometry_ptr->getColorArray());

      for (unsigned int i=0; i<curr_vertices_ptr->size(); i++)
      {
         double r=static_cast<double>(curr_colors_ptr->at(i).r())/255.0;
         double g=static_cast<double>(curr_colors_ptr->at(i).g())/255.0;
         double b=static_cast<double>(curr_colors_ptr->at(i).b())/255.0;
         double h,s,v;
         colorfunc::RGB_to_hsv(r,g,b,h,s,v);
   
         double new_r=r;
         double new_g=g;
         double new_b=b;
         if (h > 0.5*NEGATIVEINFINITY)
         {
            colorfunc::hsv_to_RGB(h,s_final,v_final,new_r,new_g,new_b);
         }

         int R,G,B;
         colorfunc::rgb_to_RGB(new_r,new_g,new_b,R,G,B);
         curr_colors_ptr->at(i)=osg::Vec4ub(
            static_cast<unsigned char>(static_cast<unsigned int>(R)),
            static_cast<unsigned char>(static_cast<unsigned int>(G)),
            static_cast<unsigned char>(static_cast<unsigned int>(B)),
            alpha_byte);
      } // loop over index i labeling points within geode
   } // loop over index g labeling DataGraph geodes
   cout << endl;
}

// ---------------------------------------------------------------------
// Member pure_intensities converts computes the hue, saturation and
// intensity for every colored point within the cloud.  It then sets
// the saturation values equal to zero.  This method converts the
// resulting pure grey values back to RGB and resets the colored
// points' entries within the RGBA member array colors.  It also
// resets detection probabilities within the metadata array equal to
// the greyscale values.

void PointCloud::pure_intensities()
{
   const unsigned char alpha_byte=
      static_cast<unsigned char>(stringfunc::ascii_integer_to_char(255));

   for (osg::Geometry* curr_Geometry_ptr=
           get_curr_geometry(); curr_Geometry_ptr != NULL; 
        curr_Geometry_ptr=get_next_geometry())
   {
      osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
         curr_Geometry_ptr->getVertexArray());
      osg::Vec4ubArray* curr_colors_ptr=dynamic_cast<osg::Vec4ubArray*>(
         curr_Geometry_ptr->getColorArray());
      model::Metadata* curr_metadata_ptr=model::getMetadataForGeometry(
         *curr_Geometry_ptr);

      for (unsigned int i=0; i<curr_vertices_ptr->size(); i++)
      {

// Null p values are usually negative.  So do not recolor any voxel if
// its p value is <= 0:

//         const double TINY_NEGATIVE=-1E-6;
//         if (curr_metadata_ptr->get(i,0) > TINY_NEGATIVE )

// Added these next lines for satellite shadowing compensation
// purposes on December 18, 2007:

         double curr_p=curr_metadata_ptr->get(i,0);
         if (nearly_equal(curr_p,0))
         {
            const double missing_data_value=-0.1;
            curr_metadata_ptr->set(i,0,missing_data_value);
         }
         else
         {
            double r=static_cast<double>(curr_colors_ptr->at(i).r())/255.0;
            double g=static_cast<double>(curr_colors_ptr->at(i).g())/255.0;
            double b=static_cast<double>(curr_colors_ptr->at(i).b())/255.0;
            double h,s,v;
            colorfunc::RGB_to_hsv(r,g,b,h,s,v);

            double new_r=r;
            double new_g=g;
            double new_b=b;
            if (h > 0.5*NEGATIVEINFINITY)
            {
               colorfunc::hsv_to_RGB(h,0,v,new_r,new_g,new_b);
            }

            int R,G,B;
            colorfunc::rgb_to_RGB(new_r,new_g,new_b,R,G,B);
            curr_colors_ptr->at(i)=osg::Vec4ub(
               static_cast<unsigned char>(static_cast<unsigned int>(R)),
               static_cast<unsigned char>(static_cast<unsigned int>(G)),
               static_cast<unsigned char>(static_cast<unsigned int>(B)),
               alpha_byte);
            curr_metadata_ptr->set(i,0,v);
         } // curr_metadata_ptr->get(0,i) >= 0 conditional
      } // loop over index i labeling points within geode
   } // loop over index g labeling DataGraph geodes
   cout << endl;
}

// ---------------------------------------------------------------------
// Member function modify_hues works with a point cloud which we
// assume contains only pure hue coloring information generated by
// PointCloud::pure_hues().  This method remaps green SPASE hues to
// either the red-yellow sector or to the cyan-blue sector.

void PointCloud::modify_hues()
{
   const unsigned char alpha_byte=
      static_cast<unsigned char>(stringfunc::ascii_integer_to_char(255));

   for (unsigned int i=0; i<get_npoints(); i++)
   {
      if (i%100000==0) cout << i/100000 << " " << flush;

      double r=static_cast<double>(colors->at(i).r())/255.0;
      double g=static_cast<double>(colors->at(i).g())/255.0;
      double b=static_cast<double>(colors->at(i).b())/255.0;
      double h,s,v;
      colorfunc::RGB_to_hsv(r,g,b,h,s,v);

      double new_r=r;
      double new_g=g;
      double new_b=b;
      if (h > 0.5*NEGATIVEINFINITY)
      {
         double new_h;
         const double h_threshold=120;
         if (h < h_threshold)
         {
            new_h=90.0*h/120.0;
         }
         else
         {
            new_h=(275.0-175.0)/(360.0-h_threshold)*(h-h_threshold)+175.0;
         }
         colorfunc::hsv_to_RGB(new_h,s,v,new_r,new_g,new_b);
      }

      int R,G,B;
      colorfunc::rgb_to_RGB(new_r,new_g,new_b,R,G,B);
      colors->at(i)=osg::Vec4ub(
         static_cast<unsigned char>(static_cast<unsigned int>(R)),
         static_cast<unsigned char>(static_cast<unsigned int>(G)),
         static_cast<unsigned char>(static_cast<unsigned int>(B)),alpha_byte);
   } // loop over index i labeling points within cloud
   outputfunc::newline();
}

// ---------------------------------------------------------------------
// Member function modify_intensities can be used to renormalize point
// cloud intensities from the interval [0,1] to [min_v,1].  We wrote
// this little method in order to brighten LL optical photos of SPASE.
// But in Feb 2006, we ended up using histogram specification to
// brighten up the raw PNG photos rather than this approach when
// fusing optical and ISAR images into a single 3D point cloud.

void PointCloud::modify_intensities()
{
   const unsigned char alpha_byte=
      static_cast<unsigned char>(stringfunc::ascii_integer_to_char(255));

   for (unsigned int i=0; i<get_npoints(); i++)
   {
      if (i%100000==0) cout << i/100000 << " " << flush;

      double r=static_cast<double>(colors->at(i).r())/255.0;
      double g=static_cast<double>(colors->at(i).g())/255.0;
      double b=static_cast<double>(colors->at(i).b())/255.0;
      double h,s,v;
      colorfunc::RGB_to_hsv(r,g,b,h,s,v);

      const double min_v=0.5;
      double new_v=(1-min_v)*v+min_v;

      double new_r=r;
      double new_g=g;
      double new_b=b;
      colorfunc::hsv_to_RGB(h,s,new_v,new_r,new_g,new_b);

      int R,G,B;
      colorfunc::rgb_to_RGB(new_r,new_g,new_b,R,G,B);
      colors->at(i)=osg::Vec4ub(
         static_cast<unsigned char>(static_cast<unsigned int>(R)),
         static_cast<unsigned char>(static_cast<unsigned int>(G)),
         static_cast<unsigned char>(static_cast<unsigned int>(B)),alpha_byte);
   } // loop over index i labeling points within cloud
}

// ----------------------------------------------------------------
// Member function equalize_intensity_histogram implements "intensity
// histogram specification" for point cloud p values.  This
// renormalization increases intensity contrast.  This method sets the
// final intensity distribution to a gaussian centered about a mean
// 50% intensity level.

void PointCloud::equalize_intensity_histogram()
{
   outputfunc::write_banner("Equalizing intensity histogram:");
   cout << "n_points = " << get_npoints() << endl;

// Generate final, desired gaussian intensity distribution:

   const int nbins=200;
   double mu=0.5;
   double sigma=0.2;
   prob_distribution p_gaussian=advmath::generate_gaussian_density(
      nbins,mu,sigma);

// Load probabilities greater than input threshold into STL vector,
// and then compute their cumulative distribution:
   
   double intensity_threshold=-1;
//   cout << "Enter threshold below which intensities will not be equalized:"
//        << endl;
//   cin >> intensity_threshold;
   
   vector<double> intensities;
   intensities.reserve(get_npoints());
   for (unsigned int i=0; i<get_npoints(); i++)
   {
      double p=metadata->get(i,0);
      if (p > intensity_threshold) intensities.push_back(p);
   }
   prob_distribution prob(intensities,nbins);

// To perform "intensity histogram equalization", we reset each
// normalized intensity value 0 <= x <= 1 to Pcum(x).  To perform
// "intensity histogram specification" onto the desired gaussian
// distribution, we perform an inverse histogram equalization and map
// Pcum(x) onto the y value for which Pcum(x) = Pgaussian(y):

   for (unsigned int i=0; i<get_npoints(); i++)
   {
      double old_p=metadata->get(i,0);
      double new_p=old_p;
      if (old_p > intensity_threshold)
      {
         int n=prob.get_bin_number(old_p);
         double pcum=prob.get_pcum(n);
         new_p=pcum;		// Histogram equalization
//         new_p=p_gaussian.find_x_corresponding_to_pcum(pcum);
      }
      cout << "i = " << i 
           << " old_p = " << old_p
           << " new_p = " << new_p << endl;

      metadata->set(i,0,new_p);
   } // loop over index i labeling points in cloud

   reload_colormap_array();
}

// ---------------------------------------------------------------------
// Member function remap_intensities performs a piecewise linear
// interpolation of p-values in order to stretch extremely bright
// regions in black & white images over a larger grey scale.  We wrote
// this method on December 27, 2007 in order to optimize our composite
// 3D SJ7 image.

void PointCloud::remap_intensities()
{
   outputfunc::write_banner("Remapping intensities:");

   for (osg::Geometry* curr_Geometry_ptr=
           get_curr_geometry(); curr_Geometry_ptr != NULL; 
        curr_Geometry_ptr=get_next_geometry())
   {
      osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
         curr_Geometry_ptr->getVertexArray());
      model::Metadata* curr_metadata_ptr=model::getMetadataForGeometry(
         *curr_Geometry_ptr);

      for (unsigned int i=0; i<curr_vertices_ptr->size(); i++)
      {
         double curr_p=curr_metadata_ptr->get(i,0);
         if (curr_p <0 || nearly_equal(curr_p,0))
         {
            const double missing_data_value=-0.1;
            curr_metadata_ptr->set(i,0,missing_data_value);
         }
         else
         {
            const double p_lo=0.35;
            const double p_hi=0.85;

            const double p0=0.0;
            const double p1=0.15;
            const double p2=0.60;
            const double p3=1.0;
            
            double new_p=0;
            if (curr_p >= 0.0 && curr_p < p_lo)
            {
               new_p=p0+(curr_p-0.0)/(p_lo-0.0)*(p1-p0);
            }
            else if (curr_p >= p_lo && curr_p < p_hi)
            {
               new_p=p1+(curr_p-p_lo)/(p_hi-p_lo)*(p2-p1);
            }
            else if (curr_p >= p_hi && curr_p <= 1.0)
            {
               new_p=p2+(curr_p-p_hi)/(1-p_hi)*(p3-p2);
            }
            curr_metadata_ptr->set(i,0,new_p);
         }
      } // loop over index i labeling curr vertices
   } // loop over geometries
}

// ---------------------------------------------------------------------
// Member function check_colors_array

void PointCloud::check_colors_array()
{
   cout << "inside PointCloud::check_colors_array():" << endl;
   for (unsigned int i=0; i<20; i++)
   {
      int r=static_cast<int>(colors->at(i).r());
      int g=static_cast<int>(colors->at(i).g());
      int b=static_cast<int>(colors->at(i).b());
      int a=static_cast<int>(colors->at(i).a());
      cout << "i = " << i << " r = " << r << " g = " << g << " b = " << b
           << " a = " << a << endl;
   }
   outputfunc::newline();
   
   for (unsigned int i=get_npoints()-20; i<get_npoints(); i++)
   {
      int r=static_cast<int>(colors->at(i).r());
      int g=static_cast<int>(colors->at(i).g());
      int b=static_cast<int>(colors->at(i).b());
      int a=static_cast<int>(colors->at(i).a());
      cout << "i = " << i << " r = " << r << " g = " << g << " b = " << b
           << " a = " << a << endl;
   }
}

// ==========================================================================
// P-value retrieval member functions:
// ==========================================================================

// Member function get_curr_p uses private member variables
// vertex_counter and leaf_counter to retrieve the next probability
// value from the Data Scene Graph.  It updates both counters so that
// they are ready to be used in a subsequent call to this method.

int PointCloud::get_curr_p(float& curr_p)
{ 
   model::Metadata* metadata = model::getMetadataForGeometry( 
      *(curr_leaf_geometry_refptr.get()));

   if (indices_stored_flag)
   {
      int j=retrieve_curr_vertex_index();
      curr_p=metadata->get(j,0);
      return j;
   }
   else
   {
      curr_p=metadata->get(curr_geometry_vertex_counter,0);
      return curr_geometry_vertex_counter;
   }
}

float PointCloud::get_curr_p()
{ 
   model::Metadata* metadata = model::getMetadataForGeometry( 
      *(curr_leaf_geometry_refptr.get()));
   if (indices_stored_flag)
   {
      int j=retrieve_curr_vertex_index();
      float curr_p=metadata->get(j,0);
      if (mathfunc::my_isnan(curr_p)) 
         cout << "inside DG::get_curr_p(), j = " << j
              << " curr_p = " << curr_p << endl;
      return curr_p;
//      return metadata->get(j,0);
   }
   else
   {
      float curr_p=metadata->get(curr_geometry_vertex_counter,0);
      if (mathfunc::my_isnan(curr_p)) 
         cout << "inside DG::get_curr_p(), curr_geom_vertex_counter = " 
              << curr_geometry_vertex_counter << " curr_p = " << curr_p
              << endl;
      return curr_p;
//      return metadata->get(curr_geometry_vertex_counter,0);
   }
}

// ------------------------------------------------------------------------
// Member function set_curr_p resets the p value for the current
// element within the data scene graph to the value passed into this
// method.

void PointCloud::set_curr_p(float curr_p)
{ 
   model::Metadata* metadata = model::getMetadataForGeometry( 
      *(curr_leaf_geometry_refptr.get()));
   if (indices_stored_flag)
   {
      int j=retrieve_curr_vertex_index();
      metadata->set(j,0,curr_p);
   }
   else
   {
      metadata->set(curr_geometry_vertex_counter,0,curr_p);
   }
}

// ==========================================================================
// Point manipulation member functions
// ==========================================================================

// Member function transform_vertices is a specialized method which we
// wrote to correct the 2005 Boston ALIRT data set based upon
// calibration information derived from Google Earth tiepoints.  After
// correcting every input point's XYZ location, this method writes out
// a new TDP file containing UTMzone and offset information within the
// TDP header and displacements relative to the zeroth point in the
// TDP body.

void PointCloud::transform_vertices()
{ 
   string banner="Transforming vertices in "+data_filename;
   outputfunc::write_big_banner(banner);

//   geocalibfunc::initialize_Boston_fit_tensor_params();
//   geocalibfunc::initialize_NYC_RTV_fit_tensor_params();
//   geocalibfunc::initialize_NYC_ALIRT_fit_tensor_params();
//   geocalibfunc::initialize_Baghdad_fit_tensor_params();
   geocalibfunc::initialize_Lowell_RTV_fit_tensor_params();

   double z_new;
   twovector X,UTM_coords;
   const int ten_percent_size=vertices->size()/10;

   for (unsigned int i=0; i<vertices->size(); i++)
   {
      if (i%ten_percent_size==0) cout << i/ten_percent_size*10 << "% " 
                                      << flush;
      X.put(0,vertices->at(i).x());
      X.put(1,vertices->at(i).y());
//      geocalibfunc::compute_Boston_UTM(
//         X,vertices->at(i).z(),UTM_coords,z_new);
      geocalibfunc::compute_transformed_UTM(
         X,vertices->at(i).z(),UTM_coords,z_new);
//      cout << "i = " << i << " X = " << X.get(0) << " Y = " << X.get(1) 
//           << " E = " << UTM_coords.get(0) << " N = " << UTM_coords.get(1)
//           << " z_new = " << z_new 
//           << endl;

// As of 4/16/07, we believe that any georegistered point within the
// RTV NYC map which has z > 400 meters corresponds to noise.  So
// reset such points' heights to a sentinel value:

//      if (z_new > 400) z_new=NEGATIVEINFINITY;

      vertices->at(i).set(UTM_coords.get(0),UTM_coords.get(1),z_new);
   }
   cout << endl << endl;

   string prefix(stringfunc::prefix(data_filename));
   string tdp_filename=prefix+"_qfit.tdp";
//   string tdp_filename=prefix+"_lfit.tdp";

   cout << "Writing transformed results to " << tdp_filename << endl << endl;
   
   tdpfunc::write_relative_xyz_data(tdp_filename,UTMzone,get_vertices_ptr());
//   tdpfunc::write_relative_xyzp_data(
//      tdp_filename,UTMzone,get_vertices_ptr(),get_metadata_ptr());
//   tdpfunc::write_relative_xyzrgba_data(
//      tdp_filename,UTMzone,get_vertices_ptr(),get_colors_ptr());
}

// ---------------------------------------------------------------------
// Member function mark_aboveTIN_snowflake_points loops over every
// point within *vertices.  For each point, it computes an approximate
// height threshold based upon a manually constructed Delaunay
// Triangulated Irregular Network.  Any points (hopefully
// corresponding just to snowflakes) lying above the surface defined
// by the TIN are marked for threshold removal.

void PointCloud::mark_aboveTIN_snowflake_points(string triangles_subdir)
{ 
   string prefix(stringfunc::prefix(data_filename));
   string triangles_filename=triangles_subdir+"/"+"triangles_"+prefix+".txt";

   cout << "triangles_filename = " << triangles_filename << endl;
   TrianglesGroup_ptr->reconstruct_triangles_from_file_info(
      triangles_filename);

   TrianglesGroup_ptr->update_display();
   TrianglesGroup_ptr->sample_zcoords_on_XYgrid();
   for (unsigned int i=0; i<vertices->size(); i++)
   {
      double zthresh=TrianglesGroup_ptr->approx_zcoord(
         vertices->at(i).x(),vertices->at(i).y());
      if (vertices->at(i).z() > zthresh)
      {
         vertices->at(i).set(
            vertices->at(i).x(),vertices->at(i).y(),xyzpfunc::null_value);
      }
   }
}

// ---------------------------------------------------------------------
// Member function remove_snowflakes statistically identifies below
// ground noise points based upon their location within the total
// height distribution.  It also identifies all points lying above a
// manually constructed triangle network.  After removing these noise
// points, this method writes out a new TDP file with a _thresh
// suffix.

void PointCloud::remove_snowflakes(
   string triangles_subdir,double ceiling_min_z)
{ 
   string banner="Removing snowflakes from "+data_filename;
   outputfunc::write_big_banner(banner);

   const double min_zdensity=1E-4;
   const double delta_z=0.30;	// meter
   ladarfunc::mark_belowground_snowflake_points(
      min_zdensity,get_min_value(2),get_max_value(2),delta_z,vertices.get(),
      ceiling_min_z);

   mark_aboveTIN_snowflake_points(triangles_subdir);

   osg::Vec3Array* new_vertices_ptr=new osg::Vec3Array;
   osg::FloatArray* new_parray_ptr=new osg::FloatArray;
   new_vertices_ptr->reserve(vertices->size());
   new_parray_ptr->reserve(vertices->size());
   
   for (unsigned int i=0; i<vertices->size(); i++)
   {
      if (vertices->at(i).z() > xyzpfunc::null_value+1)
      {
         new_vertices_ptr->push_back(vertices->at(i));
         new_parray_ptr->push_back(get_metadata_ptr()->get(i,0));
      }
   }

   model::Metadata* new_metadata_ptr=
      new model::Metadata(new_parray_ptr);

   cout << "Number of original points = " << vertices->size() << endl;
   cout << "Number of cleaned points = " << new_vertices_ptr->size() << endl;
   cout << "Number thresholded points = " 
        << vertices->size()-new_vertices_ptr->size() << endl;

   string prefix(stringfunc::prefix(data_filename));
   string tdp_filename=prefix+"_thresh.tdp";
   cout << "Writing cleaned results to " << tdp_filename << endl << endl;
   tdpfunc::write_relative_xyzp_data(
      tdp_filename,UTMzone,new_vertices_ptr,new_metadata_ptr);
}

// ==========================================================================
// Symmetry direction determination member functions
// ==========================================================================

// Member function center_of_mass loops over all vertices and returns
// their average 3-space location:

threevector PointCloud::center_of_mass()
{
   threevector COM(0,0,0);
   for (unsigned int i=0; i<get_ntotal_leaf_vertices(); i++)
   {
      COM += get_next_leaf_vertex();
   }
   COM /= get_ntotal_leaf_vertices();
//   cout << "COM = " << COM << endl;
   return COM;
}

// ---------------------------------------------------------------------
// Member function XYplane_sym_rot_angle subsamples the point cloud's
// X and Y values.  It sets up and diagonalizes a moment-of-inertia
// matrix in order to determine the symmetry axes of the cloud's
// footprint in the XY plane.  This method returns angle theta in
// radians by which the cloud needs to be rotated about the origin
// (0,0) in order to align it with the XY axes.  See our "Image
// moments of inertia" notes dated 8/29/00 under the math section of
// our STO notebook.

double PointCloud::XYplane_sym_rot_angle()
{
   const unsigned int nreduced_points=20000;
   int skip=1;
   if (get_npoints() > nreduced_points)
   {
      skip=basic_math::mytruncate(get_npoints()/nreduced_points);
   }
//   cout << "skip = " << skip << endl;
   
   int counter=0;
   double xmean,ymean,xsqmean,ysqmean,xymean;
   xmean=ymean=xsqmean=ysqmean=xymean=0;

   for (unsigned int i=0; i<get_npoints(); i += skip)
   {
      double x_i=vertices->at(i).x();
      double y_i=vertices->at(i).y();
      xmean += x_i;
      ymean += y_i;
      counter++;
   }
   xmean /= counter;
   ymean /= counter;

   for (unsigned int i=0; i<get_npoints(); i += skip)
   {
//      double x_i=vertices->at(i).x()-xmean;
//      double y_i=vertices->at(i).y()-ymean;
      double x_i=vertices->at(i).x();
      double y_i=vertices->at(i).y();
      xsqmean += sqr(x_i);
      xymean += x_i*y_i;
      ysqmean += sqr(y_i);
   } // loop over index i labeling subsampled XYZ points
   xsqmean /= counter;
   xymean /= counter;
   ysqmean /= counter;

//   cout << "counter = " << counter << endl;
//   cout << "xmean = " << xmean << " ymean = " << ymean << endl;
//   cout << "xsqmean = " << xsqmean << " ysqmean = " << ysqmean << endl;
//   cout << "xymean = " << xymean << endl;
//   cout << "xmean * ymean = " << xmean*ymean << endl;

   double Ixx=ysqmean;
   double Iyy=xsqmean;
   double Ixy=-xymean;
//   cout << "Ixx = " << Ixx << " Ixy = " << Ixy << " Iyy = " << Iyy << endl;
   
   double sqr_root=sqrt(sqr(Ixx-Iyy)+4*sqr(Ixy));
   double lambda_1=0.5*(Ixx+Iyy+sqr_root);
   double lambda_2=0.5*(Ixx+Iyy-sqr_root);
//   cout << "lambda_1 = " << lambda_1 << " lambda_2 = " << lambda_2 << endl;

//   double trace=lambda_1+lambda_2;
//   double det=lambda_1*lambda_2;
//   cout << "trace = " << trace << " det = " << det << endl;
   
   double theta1a=-atan2(Ixx-lambda_1,Ixy);
   double theta1b=-atan2(Ixy,Iyy-lambda_1);
   double theta2a=-atan2(Ixx-lambda_2,Ixy);
   double theta2b=-atan2(Ixy,Iyy-lambda_2);

   theta1a=basic_math::phase_to_canonical_interval(theta1a,0,PI);
   theta1b=basic_math::phase_to_canonical_interval(theta1b,0,PI);
   theta2a=basic_math::phase_to_canonical_interval(theta2a,0,PI);
   theta2b=basic_math::phase_to_canonical_interval(theta2b,0,PI);
   theta_xy_uv=basic_math::min(theta1a,theta1b,theta2a,theta2b);
   
   cout << "theta_xy_uv = " << theta_xy_uv*180/PI << endl;
   return theta_xy_uv;
}

// ---------------------------------------------------------------------
// Method find_extremal_uv_values rotates every XYZ point in the cloud
// through member angle theta_xy_uv about the Z axis to generate a new
// UVZ point.  It stores the minimum and maximum U and V values within
// member variables u_max, u_min, v_max and v_min.

void PointCloud::find_extremal_uv_values()
{
   u_max=v_max=NEGATIVEINFINITY;
   u_min=v_min=POSITIVEINFINITY;

   double cos_theta=cos(theta_xy_uv);
   double sin_theta=sin(theta_xy_uv);
   for (unsigned int i=0; i<get_npoints(); i++)
   {
      double x=vertices->at(i).x();
      double y=vertices->at(i).y();
      double u=cos_theta*x+sin_theta*y;
      double v=-sin_theta*x+cos_theta*y;
      u_max=basic_math::max(u_max,u);
      u_min=basic_math::min(u_min,u);
      v_max=basic_math::max(v_max,v);
      v_min=basic_math::min(v_min,v);
   }
   cout << "u_max = " << u_max << " u_min = " << u_min << endl;
   cout << "v_max = " << v_max << " v_min = " << v_min << endl;
}

// ==========================================================================
// Point searching member functions
// ==========================================================================

// Member function find_Z_given_XY first checks whether the input
// (x,y) pair lies inside the PointCloud's bounding box.  If not, this
// boolean method returns false.  Otherwise, it uses the PointCloud's
// LeafNodeVisitor to scan over all LeafGeode points for the one whose
// lateral coordinates agree to within close_enough_d2 with the input
// (x,y) pair.  It then returns that point's z value as the best
// estimate for the input point's height.

bool PointCloud::find_Z_given_XY(double x,double y,double& z)
{
//   cout << "inside PointCloud::find_Z_given_XY()" << endl;
   
// First check whether input (x,y) pair lies inside cloud's bbox:

   if (x >= get_xyz_bbox().xMin() && x <= get_xyz_bbox().xMax() &&
       y >= get_xyz_bbox().yMin() && y <= get_xyz_bbox().yMax())
   {
      refresh_leafnodevisitor_geodes();

      LeafNodeVisitor_refptr->set_XY(x,y);

// For searching speed purposes, we are willing to accept any point
// lying within a squared radius equal to close_enough_d2 of (x,y) as
// having a height value which can be used as a good estimate for the
// input location:

      const double close_enough_d2=sqr(1.0);	// meters**2

      for (unsigned int g=0; g<get_n_leaf_geodes(); g++)
      {
         pair<osg::observer_ptr<osg::Geode>,osg::Matrix> p=
            LeafNodeVisitor_refptr->get_LeafGeodes_ptr()->at(g);
         osg::Geode* curr_leafGeode_ptr=(p.first).get();

         osg::Geometry* geometry_ptr=scenegraphfunc::get_geometry(
            curr_leafGeode_ptr);

         if ( geometry_ptr->getVertexArray()->getType() == 
              osg::Array::Vec3ArrayType )
         {
            osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
               geometry_ptr->getVertexArray());

            LeafNodeVisitor_refptr->set_LocalToWorld(p.second);
            LeafNodeVisitor_refptr->set_WorldToLocal(
               p.second.inverse(p.second));
            if (LeafNodeVisitor_refptr->find_Zs_given_XYs(
               *curr_leafGeode_ptr,curr_vertices_ptr,close_enough_d2)) break;

         } // geometry type == Vec3 array conditional
      } // loop over index g labeling leaf geodes

//   osg::Vec3 curr_XYZ(LeafNodeVisitor_refptr->get_XYZ(0));
      z=LeafNodeVisitor_refptr->get_XYZ(0).z();
      return true;
   }
   else
   {
      z=NEGATIVEINFINITY;
      return false;
   }
}

// ==========================================================================
// Orthorectification member functions
// ==========================================================================

// Member function orthorectify resets the z values for all points
// within the vertices vector equal to zmin.  The resulting squished
// point cloud then becomes an orthorectified 2D map.

void PointCloud::orthorectify()
{
   for (osg::Geometry* curr_Geometry_ptr=
           get_curr_geometry(); curr_Geometry_ptr != NULL; 
        curr_Geometry_ptr=get_next_geometry())
   {
      osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
         curr_Geometry_ptr->getVertexArray());
      osg::Vec4ubArray* curr_colors_ptr=dynamic_cast<osg::Vec4ubArray*>(
         curr_Geometry_ptr->getColorArray());
      for (unsigned int i=0; i<curr_vertices_ptr->size(); i++)
      {
         double z=curr_vertices_ptr->at(i).z();
         curr_vertices_ptr->at(i).z()=xyz_bbox.zMin();

         const double zoffset=0;
         if (z < xyz_bbox.zMin()+zoffset)
         {
            curr_colors_ptr->at(i).set(0,0,0,0);
         }
            
      } // loop over index i labeling vertices in *curr_Geometry_ptr
   } // loop over index g labeling geodes in *DataGraph_ptr
}

// ==========================================================================
// Ladar image member functions
// ==========================================================================

// Member function find_extremal_z_values() uses the HiresDataVisitor
// to gather and fill its STL member vector XYZ with information from
// the highest level-of-detail geodes in the scene graph.  It then
// scans through all XYZ points for those lying within the 2D bounding
// box specified by the input X & Y limits.  This method returns the
// minimal and maximal z values corresponding to the points within the
// 2D bounding box.

void PointCloud::find_extremal_z_values(
   double xlo,double xhi,double ylo,double yhi,
   double& zmin,double &zmax)
{
   cout << "inside PointCloud::find_extremal_z_values()" << endl;

   unsigned int n_XYZs=HiresDataVisitor_ptr->get_n_XYZs();
   cout << "n_XYZs = " << n_XYZs << endl;

   if (n_XYZs==0)
   {
      HiresDataVisitor_ptr->
         set_application_type(HiresDataVisitor::retrieve_XYZs);
      get_DataNode_ptr()->accept(*HiresDataVisitor_ptr);
   }

   zmin=POSITIVEINFINITY;
   zmax=NEGATIVEINFINITY;
   for (unsigned int i=0; i<n_XYZs; i++)
   {
      osg::Vec3 curr_xyz=HiresDataVisitor_ptr->get_XYZ(i);
      double curr_x=curr_xyz.x();
      if (curr_x < xlo || curr_x > xhi) continue;
      double curr_y=curr_xyz.y();
      if (curr_y < ylo || curr_y > yhi) continue;
      double curr_z=curr_xyz.z();
      
      zmin=basic_math::min(zmin,curr_z);
      zmax=basic_math::min(zmax,curr_z);
   }

   cout << "zmin = " << zmin << " zmax = " << zmax << endl;
}

// ---------------------------------------------------------------------
// Member function retrieve_hires_XYZPs uses the HiresDataVisitor to
// gather and fill member arrays vertices and metadata with XYZ and P
// information from the highest level-of-detail geodes in the scene
// graph.

void PointCloud::retrieve_hires_XYZPs()
{
   if (vertices.valid()) 
   {
      cout << "inside PointCloud::retrieve_hires_XYZPs()" << endl;
      cout << "vertices.valid() = " << vertices.valid() << endl;
      cout << "Won't retrieve hires XYZPs !" << endl;
   }
   
   HiresDataVisitor_ptr->
      set_application_type(HiresDataVisitor::retrieve_XYZPs);
   get_DataNode_ptr()->accept(*HiresDataVisitor_ptr);

   vector<osg::Vec3> hires_XYZ=HiresDataVisitor_ptr->get_XYZ();

   unsigned int n_points=hires_XYZ.size();
   vertices=new osg::Vec3Array;
   vertices->reserve(n_points);

   metadata=new model::Metadata(HiresDataVisitor_ptr->get_probs_ptr());

   xyz_bbox.init();
   for (unsigned int n=0; n<n_points; n++)
   {
      osg::Vec3 xyz(hires_XYZ[n]);
      vertices->push_back(hires_XYZ[n]);
      xyz_bbox.expandBy(xyz[0],xyz[1],xyz[2]);
   }
}

// ---------------------------------------------------------------------
// Member function generate_ladarimage traverses over the entire cloud
// and effectively generates a projection into a z=constant
// orthorectified map plane.  The deltax and deltay spacing of the
// ladarimage's twoDarrays need not be as fine as that of the cloud
// itself.  TwoDarray connectivity information allows for maximally
// fast computation of draped video imagery intensity gradients in the
// map plane.

void PointCloud::generate_ladarimage(double delta_x,double delta_y)
{
   outputfunc::write_banner("Generating ladar image:");
   ladarimage_ptr=new ladarimage;

// If vertices member is empty, fill its contents using
// HiresDataVisitor:

   if (!vertices.valid())
   {
      retrieve_hires_XYZPs();
   }
   
   ladarimage_ptr->set_npoints(vertices->size());

   if (metadata.valid())
   {
      ladarimage_ptr->store_input_data(
         xyz_bbox.xMin(),xyz_bbox.yMin(),xyz_bbox.xMax(),xyz_bbox.yMax(),
         delta_x,delta_y,vertices.get(),metadata->getDimensionColumn(0));
   }
   else
   {
      ladarimage_ptr->store_input_data(
         xyz_bbox.xMin(),xyz_bbox.yMin(),xyz_bbox.xMax(),xyz_bbox.yMax(),
         delta_x,delta_y,vertices.get());
   }
}

// ---------------------------------------------------------------------
void PointCloud::generate_ladarimage(
   double xmin,double xmax,double ymin,double ymax,
   double delta_x,double delta_y)
{
   outputfunc::write_banner("Generating ladar image:");
   ladarimage_ptr=new ladarimage;

// If vertices member is empty, fill its contents using
// HiresDataVisitor:

   if (!vertices.valid())
   {
      retrieve_hires_XYZs_in_bbox(xmin,xmax,ymin,ymax);
   }
   
   ladarimage_ptr->set_npoints(vertices->size());

   ladarimage_ptr->store_input_data(
      xyz_bbox.xMin(),xyz_bbox.yMin(),xyz_bbox.xMax(),xyz_bbox.yMax(),
      delta_x,delta_y,vertices.get());
}

// ==========================================================================
// File output member functions
// ==========================================================================

void PointCloud::write_output_file(string output_filename,
                                   bool set_origin_to_zeroth_xyz)
{
   cout << "Enter 'XYZ', 'XYZP', 'XYZRGBA', 'TDP' or 'IVE' to specify output file type:" << endl;
   string output_filetype;
   cin >> output_filetype;
   if (output_filetype=="XYZ" || output_filetype=="xyz")
   {
      write_XYZ_file(output_filename,"./XYZ/",set_origin_to_zeroth_xyz);
   }
   else if (output_filetype=="XYZP" || output_filetype=="xyzp")
   {
      write_XYZP_file(output_filename,"./XYZP/");
   }
   else if (output_filetype=="XYZRGBA" || output_filetype=="xyzrgba")
   {
      write_XYZRGBA_file(output_filename,"./XYZRGBA/");
   }
   else if (output_filetype=="TDP" || output_filetype=="tdp")
   {
      write_TDP_file(output_filename,"./TDP/",set_origin_to_zeroth_xyz);
   }
   else if (output_filetype=="IVE" || output_filetype=="ive")
   {
      write_IVE_file(output_filename,"./IVE/");
   }
   else
   {
      cout << "Output file type not recognized" << endl;
   }
}

// ---------------------------------------------------------------------
void PointCloud::write_XYZP_file(string output_filename,string subdir)
{
   outputfunc::write_banner("Writing out XYZP file:");

   ofstream binary_outstream;
   filefunc::dircreate(subdir);
   output_filename=subdir+output_filename+".xyzp";
   filefunc::deletefile(output_filename);
   binary_outstream.open(output_filename.c_str(),ios::app|ios::binary);

   refresh_leafnodevisitor_geodes();

   for (unsigned int g=0; g<get_n_leaf_geodes(); g++)
   {
      pair<osg::observer_ptr<osg::Geode>,osg::Matrix> p=
         LeafNodeVisitor_refptr->get_LeafGeodes_ptr()->at(g);
      osg::Geometry* curr_leafgeom_ptr=scenegraphfunc::get_geometry(
         (p.first).get());
      scenegraphfunc::write_geometry_xyzp(
         binary_outstream,curr_leafgeom_ptr,p.second);
   } // loop over index g labeling leaf geodes

   cout << endl;
   cout << "Wrote " << output_filename << " to disk" << endl;
   binary_outstream.close();
}

// ---------------------------------------------------------------------
void PointCloud::write_XYZRGBA_file(string output_filename,string subdir)
{
   outputfunc::write_banner("Writing out XYZRGBA file:");

   ofstream binary_outstream;
   filefunc::dircreate(subdir);
   output_filename=subdir+output_filename+".xyzrgba";
   filefunc::deletefile(output_filename);
   binary_outstream.open(output_filename.c_str(),ios::app|ios::binary);

   refresh_leafnodevisitor_geodes();

   for (unsigned int g=0; g<get_n_leaf_geodes(); g++)
   {
      pair<osg::observer_ptr<osg::Geode>,osg::Matrix> p=
         LeafNodeVisitor_refptr->get_LeafGeodes_ptr()->at(g);
      osg::Geometry* curr_leafgeom_ptr=scenegraphfunc::get_geometry(
         (p.first).get());
      scenegraphfunc::write_geometry_xyzrgba(
         binary_outstream,curr_leafgeom_ptr,p.second);
   } // loop over index g labeling leaf geodes

   cout << endl;
   cout << "Wrote " << output_filename << " to disk" << endl;
   binary_outstream.close();
}

// ---------------------------------------------------------------------
void PointCloud::write_TDP_file(
   string output_filename,string subdir,bool set_origin_to_zeroth_xyz)
{
//   cout << "inside PointCloud::write_TDP_file()" << endl;
   
   outputfunc::write_banner("Writing out TDP file:");

   filefunc::dircreate(subdir);
   output_filename=subdir+output_filename+".tdp";
   filefunc::deletefile(output_filename);

   Tdp_file tdp_file;
   tdp_file.file_open( output_filename, "w+b" );

   HiresDataVisitor_ptr->reset_n_hires_vertices();
   HiresDataVisitor_ptr->set_application_type(
      HiresDataVisitor::vertex_count);
   get_DataNode_ptr()->accept(*HiresDataVisitor_ptr);

   int n_hires_vertices=HiresDataVisitor_ptr->get_n_hires_vertices();
   cout << "n_hires_vertices = " << n_hires_vertices << endl;

   tdp_file.klv_create(TdpKeyXYZ_POINT_DATA,3*4*n_hires_vertices);
   tdp_file.klv_create(TdpKeyRGBA_COLOR_8,4*n_hires_vertices);
   
   threevector zeroth_xyz(0,0,0);
   if (set_origin_to_zeroth_xyz)
   {

// Fetch very first vertex within PointCloud and use it as a relative
// reference for all other vertices:

// Conversion factors derived on 1/30/09 needed to transform z-values
// in 2007 Jaudit MIT point cloud to match those in 2005 Alirt Boston
// cloud:

//      const double z_scale=1.018;
//      const double z_offset=28.658;	

      zeroth_xyz=get_zeroth_vertex();
//      double z_new=z_scale*zeroth_xyz.get(2)+z_offset;
//      zeroth_xyz.put(2,z_new);

      tdpfunc::write_UTM_zone_and_offset(tdp_file,UTMzone,zeroth_xyz);
   }

   HiresDataVisitor_ptr->set_zeroth_xyz_ptr(&zeroth_xyz);

   int xyz_byte_counter=0;
   int color_byte_counter=0;
   HiresDataVisitor_ptr->set_xyz_byte_counter_ptr(&xyz_byte_counter);
   HiresDataVisitor_ptr->set_color_byte_counter_ptr(&color_byte_counter);
   HiresDataVisitor_ptr->set_tdp_file_ptr(&tdp_file);

   HiresDataVisitor_ptr->set_application_type(
      HiresDataVisitor::tdp_export);
   get_DataNode_ptr()->accept(*HiresDataVisitor_ptr);

   cout << endl;
   cout << "Wrote " << output_filename << " to disk" << endl;
}

// ---------------------------------------------------------------------
// Member function write_XYZ_file uses the HiresDataVisitor to iterate
// over every point (both in memory and on disk) within the scene
// graph.  

void PointCloud::write_XYZ_file(
   string output_filename,string subdir,bool set_origin_to_zeroth_xyz)
{
   cout << "inside PointCloud::write_XYZ_file()" << endl;
   outputfunc::write_banner("Writing out XYZ file:");

   filefunc::dircreate(subdir);
   output_filename=subdir+output_filename+".xyz";
   filefunc::deletefile(output_filename);

   HiresDataVisitor_ptr->reset_n_hires_vertices();
   HiresDataVisitor_ptr->set_application_type(
      HiresDataVisitor::vertex_count);
   get_DataNode_ptr()->accept(*HiresDataVisitor_ptr);

   unsigned int n_hires_vertices=HiresDataVisitor_ptr->get_n_hires_vertices();
   cout << "n_hires_vertices = " << n_hires_vertices << endl;

   threevector zeroth_xyz(0,0,0);
/*
   if (set_origin_to_zeroth_xyz)
   {

// Fetch very first vertex within PointCloud and use it as a relative
// reference for all other vertices:

      zeroth_xyz=get_zeroth_vertex();
   }
*/
   HiresDataVisitor_ptr->set_zeroth_xyz_ptr(&zeroth_xyz);

   int xyz_byte_counter=0;
   HiresDataVisitor_ptr->set_xyz_byte_counter_ptr(&xyz_byte_counter);

   cout << "Retrieving XYZ points from scenegraph:" << endl;
   HiresDataVisitor_ptr->set_application_type(
      HiresDataVisitor::retrieve_XYZs);
   get_DataNode_ptr()->accept(*HiresDataVisitor_ptr);
   cout << "Number of points stored in XYZ vector = "
        << HiresDataVisitor_ptr->get_XYZ().size() << endl;

   ofstream binary_outstream;
   binary_outstream.open(output_filename.c_str(),ios::app|ios::binary);

//   float p=0;
   for (unsigned int i=0; i<n_hires_vertices; i++)
   {
      osg::Vec3 curr_xyz=HiresDataVisitor_ptr->get_XYZ(i);
//      cout << "i = " << i
//           << " x = " << curr_xyz.x()
//           << " y = " << curr_xyz.y()
//           << " z = " << curr_xyz.z() << endl;

      xyzpfunc::write_single_xyz_point(
         binary_outstream,curr_xyz.x(),curr_xyz.y(),curr_xyz.z());
//      xyzpfunc::write_single_xyzp_point(
//         binary_outstream,curr_xyz.x(),curr_xyz.y(),curr_xyz.z(),p);

   }
   binary_outstream.close();

   cout << "Wrote " << output_filename << " to disk" << endl;
}

// ==========================================================================
// Shadowing member functions
// ==========================================================================

// Member function interpoint_RMS_distance forms a KDTree from all of
// the XYZ threevectors.  It then loops over each XYZ point and finds
// its nearest neighbor.  Neighbors which are located too close are
// ignored.  Otherwise, this method returns the RMS nearest neighbor
// distance.

double PointCloud::interpoint_RMS_distance(double minimal_separation)
{
   vector<fourvector>* XYZP_ptr=new vector<fourvector>;
   for (unsigned int i=0; i<get_npoints(); i++)
   {
      XYZP_ptr->push_back(fourvector(
         vertices->at(i).x(),vertices->at(i).y(),vertices->at(i).z(),i));
   }
   double RMS_dist=xyzpfunc::interpoint_RMS_distance(
      minimal_separation,XYZP_ptr);
   delete XYZP_ptr;
   return RMS_dist;
}

// ----------------------------------------------------------------
// Member function generate_shadow_map instantiates a depth buffer in
// the plane normal to input illumination direction vector r_hat.  The
// pixelization of this depth buffer plane should be commensurate with
// nearest neighbor spacing of the point cloud itself.  This method
// loops over every point in the cloud and determines into which depth
// buffer pixel it falls (assuming orthographic projection as of
// 3/6/06).  It keeps track of the points' depths relative to the
// illumination plane within STL vectors for each pixel.  After all
// XYZ points have been mapped into planar coordinates and their
// depths have been sorted, those world points which lie more than
// some tolerance away from the points furthest upstream are declared
// to be in shadow.  Their boolean flags within member *shadows_ptr
// are set to true.

void PointCloud::generate_shadow_map(const threevector& r_hat)
{
   outputfunc::write_banner("Generating shadow map:");

   if (shadows_ptr==NULL) shadows_ptr=new vector<bool>;
   shadows_ptr->clear();
   shadows_ptr->reserve(get_npoints());
   for (unsigned int i=0; i<get_npoints(); i++)
   {
      shadows_ptr->push_back(false);
   }
   
//   const double min_separation=1E-4;
//   double duv=interpoint_RMS_distance(min_separation);
//   double duv=0.0048;	// OK for SPASE file rotated_isar0.xyzp 
//   double duv=0.0024;	// OK for SPASE file rotated_isar1.xyzp 
   double duv=0.0012;	// OK for SPASE file rotated_isar2.xyzp 
//   double duv=0.035;	// OK for original new (i.e. Ken's best) AK model 
//   double duv=0.017;	// OK for subdivided new (i.e. Ken's best) AK model 

// Note added on 3/9/06: Ken's AK model is NOT perfectly symmetric wrt
// the y axis.  Instead, it is shifted by 0.092 meters wrt y=0.  We
// therefore need to add Delta y = 0.092 meter to all AK model 3D
// tiepoints...

/*
   double rx,ry,rz;
   cout << "Enter x component of illumination direction vector:" << endl;
   cin >> rx;
   cout << "Enter y component of illumination direction vector:" << endl;
   cin >> ry;
   cout << "Enter z component of illumination direction vector:" << endl;
   cin >> rz;
   threevector r_hat(rx,ry,rz);
   r_hat=r_hat.unitvector();
*/

   threevector COM(0,0,0);
   plane illumination_plane(r_hat,COM);
   cout << "a_hat = " << illumination_plane.get_ahat() << endl;
   cout << "b_hat = " << illumination_plane.get_bhat() << endl;   
   cout << "n_hat = " << illumination_plane.get_nhat() << endl;

   vector<threevector>* planar_coords_ptr=new vector<threevector>;
   for (unsigned int i=0; i<get_ntotal_leaf_vertices(); i++)
   {
      threevector curr_XYZ(get_next_leaf_vertex());
      planar_coords_ptr->push_back(illumination_plane.coords_wrt_plane(
         curr_XYZ));
   }

   double a_max=NEGATIVEINFINITY;
   double b_max=NEGATIVEINFINITY;   
   double n_max=NEGATIVEINFINITY;
   double a_min=POSITIVEINFINITY;
   double b_min=POSITIVEINFINITY;
   double n_min=POSITIVEINFINITY;
   for (unsigned int i=0; i<planar_coords_ptr->size(); i++)
   {
      threevector curr_planar_coords( (*planar_coords_ptr)[i] );
      a_max=basic_math::max(a_max,curr_planar_coords.get(0));
      a_min=basic_math::min(a_min,curr_planar_coords.get(0));
      b_max=basic_math::max(b_max,curr_planar_coords.get(1));
      b_min=basic_math::min(b_min,curr_planar_coords.get(1));
      n_max=basic_math::max(n_max,curr_planar_coords.get(2));
      n_min=basic_math::min(n_min,curr_planar_coords.get(2));

//      cout << "i = " << i << " a = " << curr_planar_coords.get(0)
//           << " b = " << curr_planar_coords.get(1)
//           << " n = " << curr_planar_coords.get(2) << endl;
   }
   cout << "a_max = " << a_max << " a_min = " << a_min << endl;
   cout << "b_max = " << b_max << " b_min = " << a_min << endl;
   cout << "n_max = " << n_max << " n_min = " << a_min << endl;

   double da=duv;
   double db=duv;
   int width=basic_math::round((a_max-a_min)/da)+1;
   int height=basic_math::round((b_max-b_min)/db)+1;
   cout << "Shadow map width = " << width 
        << " shadow map height = " << height << endl;
    
//   double max_factor=1;
   double max_factor=22;  // needed for draping image 3 onto SPASE
//   double max_factor=8; // OK for subdivided new (i.e. Ken's best) AK model
   cout << "Enter max surface depth factor:" << endl;
   cin >> max_factor;
   double dn=max_factor*basic_math::max(da,db);
   cout << "max surface depth = " << dn << endl;

   TwoDarray<vector<pair<double,int> >* >* depth_buffer_ptr=
      new TwoDarray<vector<pair<double,int> >* >(width,height);

   int overlap=1; // OK for subdivided new (i.e. Ken's best) AK model 
		  //   and for SPASE model
//   cout << "Enter overlap:" << endl;
//   cin >> overlap;

   for (unsigned int i=0; i<planar_coords_ptr->size(); i++)
   {
      if (i%100000==0) cout << i/100000 << " " << flush;

      threevector curr_planar_coords( (*planar_coords_ptr)[i] );
      int pa=basic_math::round((curr_planar_coords.get(0)-a_min)/da);
      int pb=basic_math::round((curr_planar_coords.get(1)-b_min)/db);

      for (int d_pa=-overlap; d_pa<=overlap; d_pa++)
      {
         int curr_pa=basic_math::max(0,pa+d_pa);
         curr_pa=basic_math::min(curr_pa,width-1);
         for (int d_pb=-overlap; d_pb<=overlap; d_pb++)
         {
            int curr_pb=basic_math::max(0,pb+d_pb);
            curr_pb=basic_math::min(curr_pb,height-1);
            vector<pair<double,int> >* curr_vector_ptr=
               depth_buffer_ptr->get(curr_pa,curr_pb);
            if (curr_vector_ptr==NULL)
            {
               curr_vector_ptr=new vector<pair<double,int> >;
               depth_buffer_ptr->put(curr_pa,curr_pb,curr_vector_ptr);
            }
            curr_vector_ptr->push_back(pair<double,int>(
               curr_planar_coords.get(2),i));
         } // loop over d_pb index
      } // loop over d_pa index
   } // loop over index i labeling points in cloud
   
   for (int pa=0; pa<width; pa++)
   {
      for (int pb=0; pb<height; pb++)
      {
         vector<pair<double,int> >* curr_vector_ptr=
            depth_buffer_ptr->get(pa,pb);
         if (curr_vector_ptr != NULL)
         {
            std::sort(curr_vector_ptr->begin(),curr_vector_ptr->end());
         
            double nearest_depth=(*curr_vector_ptr)[0].first;
            for (unsigned int j=1; j<curr_vector_ptr->size(); j++)
            {
               double curr_depth=(*curr_vector_ptr)[j].first;
               int i=(*curr_vector_ptr)[j].second;

               if (curr_depth-nearest_depth > dn)
               {
                  (*shadows_ptr)[i]=true;
               }
// Need to comment out next lines for subdivided version of Ken's new
// AK model:

//               {
//                  (*shadows_ptr)[i]=false;
//               }
            } // loop over index j labeling entries in (pa,pb) depth vector
            delete curr_vector_ptr;
         } // curr_vector_ptr != NULL conditional
      } // loop over pb index
   } // loop over pa index
}

// ----------------------------------------------------------------
// Member function determine_shaded_points loops over all points in
// the cloud.  It sets their boolean shadow flags to true if their
// associated normal dot product with the input illumination direction
// vector > 0 (i.e. point is shaded if its normal is NOT anti-parallel
// to r_hat).

void PointCloud::determine_shaded_points(const threevector& r_hat)
{
   outputfunc::write_banner("Determining shaded points:");

/*
   if (shadows_ptr==NULL) shadows_ptr=new vector<bool>;
   shadows_ptr->clear();
   shadows_ptr->reserve(get_npoints());
   for (unsigned int i=0; i<get_npoints(); i++)
   {
      shadows_ptr->push_back(false);
   }
*/

   if (normals_ptr==NULL)
   {
      cout << "Cannot determine shaded points since normals_ptr==NULL"
           << endl;
   }
   else
   {
      if (shadows_ptr != NULL)
      {
         for (unsigned int i=0; i<get_npoints(); i++)
         {
            double dotproduct=r_hat.dot( (*normals_ptr)[i] );
            if (dotproduct > 0) (*shadows_ptr)[i]=true;            
         } // loop over index i labeling points in cloud
      } // shadows_ptr != NULL && normals_ptr != NULL conditional
   }
}

// ----------------------------------------------------------------
// Member function retrieve_hires_XYZs_in_bbox() uses the
// HiresDataVisitor to gather and fill member arrays vertices and
// metadata with XYZ information from the highest level-of-detail
// geodes in the scene graph.

void PointCloud::retrieve_hires_XYZs_in_bbox()
{
   double xmin,xmax,ymin,ymax;
/*
   cout << "Enter xmin:" << endl;
   cin >> xmin;
   cout << "Enter ymin:" << endl;
   cin >> ymin;
   cout << "Enter xmax:" << endl;
   cin >> xmax;
   cout << "Enter ymax:" << endl;
   cin >> ymax;
*/

   xmin=320944;
   ymin=3456309;
   xmax=339852;
   ymax=3475268;
   retrieve_hires_XYZs_in_bbox(xmin,xmax,ymin,ymax);
}

void PointCloud::retrieve_hires_XYZs_in_bbox(
   double xmin,double xmax,double ymin,double ymax)
{
   cout << "inside PointCloud::retrieve_hires_XYZs_in_bbox()" << endl;

   if (vertices.valid()) 
   {
      cout << "vertices.valid() = " << vertices.valid() << endl;
      cout << "Won't retrieve hires XYZs !" << endl;
      return;
   }
   
   bounding_box* bbox_ptr=HiresDataVisitor_ptr->get_bbox_ptr();
   bbox_ptr->set_xy_bounds(xmin,xmax,ymin,ymax);
   HiresDataVisitor_ptr->
      set_application_type(HiresDataVisitor::retrieve_XYZs_inside_bbox);
   get_DataNode_ptr()->accept(*HiresDataVisitor_ptr);
   vector<osg::Vec3> hires_XYZ=HiresDataVisitor_ptr->get_XYZ();

   unsigned int n_points=hires_XYZ.size();
   cout << "n_points = " << n_points << endl;
   vertices=new osg::Vec3Array;
   vertices->reserve(n_points);


   xyz_bbox.init();
   for (unsigned int n=0; n<n_points; n++)
   {
      osg::Vec3 xyz(hires_XYZ[n]);
      vertices->push_back(hires_XYZ[n]);
      xyz_bbox.expandBy(xyz[0],xyz[1],xyz[2]);
   }
}
