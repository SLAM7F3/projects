// ==========================================================================
// SCENEGRAPHFUNCS stand-alone methods
// ==========================================================================
// Last modified on 1/30/09; 11/12/10; 11/13/10
// ==========================================================================

#include <iostream>
#include <string>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/StateSet>
#include <libtdp/tdp_file.h>
#include <libtdp/alirt.conf.h>
#include <libtdp/alirt2.conf.h>
#include <libtdp/point_data.conf.h>
#include "color/colorfuncs.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "osg/osgSceneGraph/ColormapPtrs.h"
#include "general/filefuncs.h"
#include "model/Metadata.h"
#include "osg/osgSceneGraph/scenegraphfuncs.h"
#include "general/stringfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

namespace scenegraphfunc
{

// Method get_geometry takes in a Geode which is assumed to contain a
// geometry with a point vertex array.  If such a geometry exists,
// this method returns a pointer to it.  Otherwise, it returns null.

   osg::Geometry* get_geometry(osg::Geode* Geode_ptr)
      {
    
// Iterate through all drawables for current Geode to find its
// geometries:

         osg::Geode::DrawableList::const_iterator drawableIter = 
            Geode_ptr->getDrawableList().begin();

         while( drawableIter != Geode_ptr->getDrawableList().end() ) 
         {
            if ( osg::Geometry* geometry = const_cast<osg::Geometry*>( 
               drawableIter->get()->asGeometry() ) ) 
            {
               if ( geometry->getVertexArray()->getType() == 
                    osg::Array::Vec3ArrayType ) 
               {

// Find any metadata buried within scene graph:

                  osg::ref_ptr<osg::Array> metadataArray = NULL;
				
// The preferred way to store metadata is as a fog coordinate array,
// which is not normally used:

                  if ( geometry->getFogCoordArray() ) 
                  {
//                     cout << "inside scenegraphfunc::get_geometry()" << endl;
//                     cout << "Setting metadatarray to FogCoordArray" << endl;
                     metadataArray = geometry->getFogCoordArray();
                     geometry->setFogCoordBinding(osg::Geometry::BIND_OFF);
                     geometry->setFogCoordArray( NULL );
                  } 

/*
                  else if ( geometry->getSecondaryColorArray() ) 
                  {

// We will never use a secondary color array for its usual purpose, so
// use this space to pack in an array contains extra metadata.  

// *** This may cause a crash if OpenGL attempts to use a FloatArray
// as a secondary color array!  ***

                     cout << "inside scenegraphfunc::get_geometry()"
                          << endl;
                     cout << "Setting metadatarray to 2ndColorArray" << endl;

                     metadataArray = geometry->getSecondaryColorArray();
                     geometry->setSecondaryColorBinding(
                        osg::Geometry::BIND_OFF);
                     geometry->setSecondaryColorArray( NULL );
                  }
*/
			
// Convert the osg::Array object into Ross Anderson's model::Metadata
// object:

                  if ( metadataArray.valid())
                  {
                     geometry->setUserData( new model::Metadata( 
                        metadataArray.get() ) );
                  }

                  return geometry;
               } // geometry->vertex array == vec3array conditional
            } // drawable == geometry conditional
            ++drawableIter;
         } // while loop over drawables
         return NULL;
      }

// ------------------------------------------------------------------------
   vector<osg::Geometry*> get_geometries(osg::Geode* Geode_ptr)
      {
         vector<osg::Geometry*> geometries;
 
// Iterate through all drawables for current Geode to find its
// geometries:

         osg::Geode::DrawableList::const_iterator drawableIter = 
            Geode_ptr->getDrawableList().begin();

         while( drawableIter != Geode_ptr->getDrawableList().end() ) 
         {
            if ( osg::Geometry* geometry = const_cast<osg::Geometry*>( 
               drawableIter->get()->asGeometry() ) ) 
            {
               if ( geometry->getVertexArray()->getType() == 
                    osg::Array::Vec3ArrayType ) 
               {
                  geometries.push_back(geometry);
               } // geometry->vertex array == vec3array conditional
            } // drawable == geometry conditional
            ++drawableIter;
         } // while loop over drawables
         return geometries;
      }

// ------------------------------------------------------------------------
// Method function get_n_geometry_vertices returns the number of XYZ
// vertices residing within the input geometry that is assumed to come
// from some approximation or leaf Geode in the Data Scene Graph.

   int get_n_geometry_vertices(
      osg::Geometry* Geometry_ptr,bool indices_stored_flag)
      { 
         if (Geometry_ptr==NULL) 
         {
            return 0;
         }

         osg::PrimitiveSet* primitiveSet = NULL;
         if ( Geometry_ptr->getNumPrimitiveSets() == 1 ) 
         {
            primitiveSet = Geometry_ptr->getPrimitiveSet(0);
         } 
         else 
         {
            return 0; // Can't filter a set with <> 1 PrimitiveSet
         }

         if (indices_stored_flag)
         {
            osg::DrawElementsUInt* primitive_ptr=dynamic_cast<
               osg::DrawElementsUInt*>(primitiveSet);
            return primitive_ptr->getNumIndices();
         }
         else
         {
            osg::DrawArrays* primitive_ptr=dynamic_cast<osg::DrawArrays*>(
               primitiveSet);
            if (primitive_ptr != NULL)
            {
               return primitive_ptr->getNumIndices();
            }
            else
            {
               return 0;
            }
         } // indices_stored_flag conditional
      }

// ==========================================================================
// Coloring methods
// ==========================================================================

// Method instantiate_color_array dynamically allocates an
// osg::Vec4ubArray of size set by input parameter nbins.  It also
// fills the array with black color entries.

   osg::Vec4ubArray* instantiate_color_array(
      unsigned int nbins,bool fill_color_array,osg::Geometry* geometry_ptr,
      string geometry_name)
      { 
         osg::Vec4ubArray* new_colors_ptr=new osg::Vec4ubArray;
         new_colors_ptr->reserve(nbins);

         if (fill_color_array)
         {
            const osg::Vec4ub null_color(osg::Vec4ub(0,0,0,0));
            for (unsigned int i=0; i < nbins; i++)
            {
               new_colors_ptr->push_back(null_color);
            }
         }

         geometry_ptr->setName(geometry_name);
         return new_colors_ptr;
      }

// ------------------------------------------------------------------------
   string get_mutable_colors_label() 
      { 
         const string label("Mutable colors");
         return label;
      }

   string get_fixed_colors_label() 
      { 
         const string label("Fixed colors");
         return label;
      }

// ------------------------------------------------------------------------
// Method saved_fixed_colors instantiates a new SecondaryColorArray
// for input *curr_Geometry_ptr if the Geometry does not already have
// one.  It then copies the contents of the Geometry's ColorArray into
// its new SecondaryColorArray.  This method enables fixed RGBA color
// information read in from .osga files to be permanently stored and
// retrieved on demand even if the contents of a Geometry's ColorArray
// have been overwritten by calls to
// ColorGeodeVisitor::color_geometry_vertices().

   void save_fixed_colors(osg::Geometry* curr_Geometry_ptr)
      { 
//         cout << "inside scenegraphfunc::save_fixed_colors()" << endl;
         osg::Vec4ubArray* colors_ptr=dynamic_cast<osg::Vec4ubArray*>(
            curr_Geometry_ptr->getColorArray());
         osg::Vec4ubArray* secondary_colors_ptr=
            dynamic_cast<osg::Vec4ubArray*>(
               curr_Geometry_ptr->getSecondaryColorArray());
         
         if (colors_ptr != NULL && secondary_colors_ptr==NULL)
         {
            secondary_colors_ptr=new osg::Vec4ubArray(*colors_ptr);
            curr_Geometry_ptr->setSecondaryColorArray(secondary_colors_ptr);
            curr_Geometry_ptr->setSecondaryColorBinding(
               osg::Geometry::BIND_PER_VERTEX);
         }
      }

// ==========================================================================
// Geometry output methods
// ==========================================================================

// Method write_geometry_xyzp takes in a Geometry and loops over all
// its vertices.  It writes out x,y,z,p values to the specified
// ofstream.

   void write_geometry_xyzp(
      ofstream& binary_outstream,osg::Geometry* curr_Geometry_ptr,
      const osg::Matrix& LocalToWorld)
      { 
         osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
            curr_Geometry_ptr->getVertexArray());

         for (unsigned int i=0; i<curr_vertices_ptr->size(); i++)
         {
            float curr_x=curr_vertices_ptr->at(i).x()*LocalToWorld(0,0)
               +curr_vertices_ptr->at(i).y()*LocalToWorld(1,0)
               +curr_vertices_ptr->at(i).z()*LocalToWorld(2,0)
               +LocalToWorld(3,0);
            float curr_y=curr_vertices_ptr->at(i).x()*LocalToWorld(0,1)
               +curr_vertices_ptr->at(i).y()*LocalToWorld(1,1)
               +curr_vertices_ptr->at(i).z()*LocalToWorld(2,1)
               +LocalToWorld(3,1);
            float curr_z=curr_vertices_ptr->at(i).x()*LocalToWorld(0,2)
               +curr_vertices_ptr->at(i).y()*LocalToWorld(1,2)
               +curr_vertices_ptr->at(i).z()*LocalToWorld(2,2)
               +LocalToWorld(3,2);
            float curr_p=-1.0;
            model::Metadata* curr_metadata_ptr=
               model::getMetadataForGeometry(*curr_Geometry_ptr);
            if (curr_metadata_ptr != NULL)
            {
               curr_p=curr_metadata_ptr->get(i,0);
            }

            filefunc::writeobject(binary_outstream,curr_x);
            filefunc::writeobject(binary_outstream,curr_y);
            filefunc::writeobject(binary_outstream,curr_z);
            filefunc::writeobject(binary_outstream,curr_p);
         } // loop over index i labeling vertices in *curr_Geometry_ptr
      }

// ------------------------------------------------------------------------
// Method write_geometry_xyzrgba takes in a Geometry and loops over
// all its vertices.  It writes out x,y,z,R,G,B,A values to the
// specified ofstream.

   void write_geometry_xyzrgba(
      ofstream& binary_outstream,osg::Geometry* curr_Geometry_ptr,
      const osg::Matrix& LocalToWorld)
      { 
         osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
            curr_Geometry_ptr->getVertexArray());
         osg::Vec4ubArray* curr_colors_ptr=dynamic_cast<osg::Vec4ubArray*>(
            curr_Geometry_ptr->getColorArray());
         const unsigned char alpha_byte=static_cast<unsigned char>(255);

         for (unsigned int i=0; i<curr_vertices_ptr->size(); i++)
         {
            float curr_x=curr_vertices_ptr->at(i).x()*LocalToWorld(0,0)
               +curr_vertices_ptr->at(i).y()*LocalToWorld(1,0)
               +curr_vertices_ptr->at(i).z()*LocalToWorld(2,0)
               +LocalToWorld(3,0);
            float curr_y=curr_vertices_ptr->at(i).x()*LocalToWorld(0,1)
               +curr_vertices_ptr->at(i).y()*LocalToWorld(1,1)
               +curr_vertices_ptr->at(i).z()*LocalToWorld(2,1)
               +LocalToWorld(3,1);
            float curr_z=curr_vertices_ptr->at(i).x()*LocalToWorld(0,2)
               +curr_vertices_ptr->at(i).y()*LocalToWorld(1,2)
               +curr_vertices_ptr->at(i).z()*LocalToWorld(2,2)
               +LocalToWorld(3,2);

            filefunc::writeobject(binary_outstream,curr_x);
            filefunc::writeobject(binary_outstream,curr_y);
            filefunc::writeobject(binary_outstream,curr_z);

            osg::Vec4ub curr_color=curr_colors_ptr->at(i);
            filefunc::writeobject(binary_outstream,curr_color.r());
            filefunc::writeobject(binary_outstream,curr_color.g());
            filefunc::writeobject(binary_outstream,curr_color.b());
            filefunc::writeobject(binary_outstream,alpha_byte);

         } // loop over index i labeling vertices in *curr_Geometry_ptr
      }

/*
// ------------------------------------------------------------------------
// Method write_geometry_relative_xyzrgba takes in a Geometry and
// loops over all its vertices.  It writes out x,y,z,R,G,B,A values to
// an output TDP file.

   void write_geometry_relative_xyzrgba(
      const threevector& zeroth_xyz,Tdp_file& tdp_file,
      int& xyz_byte_counter,int& color_byte_counter,
      osg::Geometry* curr_Geometry_ptr,const osg::Matrix& LocalToWorld)
      { 
         osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
            curr_Geometry_ptr->getVertexArray());

// If curr_Geometry_ptr does not already contain a ColorArray, we
// instantiate one and initially fill it up with black entries:

         osg::Vec4ubArray* curr_colors_ptr=dynamic_cast<osg::Vec4ubArray*>(
            curr_Geometry_ptr->getColorArray());

         if (curr_colors_ptr==NULL)
         {
            curr_colors_ptr=instantiate_color_array(
               curr_vertices_ptr->size(),true,curr_Geometry_ptr,
               get_fixed_colors_label());
            curr_Geometry_ptr->setColorArray(curr_colors_ptr);
            curr_Geometry_ptr->setColorBinding(
               osg::Geometry::BIND_PER_VERTEX);
         }

         int npoints=curr_vertices_ptr->size();
         const int nfloats_per_point=3;
         const int nchars_per_point=4;
         int n_xyz_bytes=npoints*nfloats_per_point*sizeof(real32_t);
         int n_color_bytes=npoints*nchars_per_point*sizeof(char8_t);
         real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];
         char8_t* rgba_data=new char8_t[n_color_bytes];

         for (int i=0; i<npoints; i++)
         {
            float curr_x=curr_vertices_ptr->at(i).x()*LocalToWorld(0,0)
               +curr_vertices_ptr->at(i).y()*LocalToWorld(1,0)
               +curr_vertices_ptr->at(i).z()*LocalToWorld(2,0)
               +LocalToWorld(3,0);
            float curr_y=curr_vertices_ptr->at(i).x()*LocalToWorld(0,1)
               +curr_vertices_ptr->at(i).y()*LocalToWorld(1,1)
               +curr_vertices_ptr->at(i).z()*LocalToWorld(2,1)
               +LocalToWorld(3,1);
            float curr_z=curr_vertices_ptr->at(i).x()*LocalToWorld(0,2)
               +curr_vertices_ptr->at(i).y()*LocalToWorld(1,2)
               +curr_vertices_ptr->at(i).z()*LocalToWorld(2,2)
               +LocalToWorld(3,2);

// Conversion factors derived on 1/30/09 needed to transform z-values
// in 2007 Jaudit MIT point cloud to match those in 2005 Alirt Boston
// cloud:

//            const double z_scale=1.018;
//            const double z_offset=28.658;	
//            curr_z = z_scale*curr_z+z_offset;

            rel_xyz_data[3*i+0]=curr_x-zeroth_xyz.get(0);
            rel_xyz_data[3*i+1]=curr_y-zeroth_xyz.get(1);
            rel_xyz_data[3*i+2]=curr_z-zeroth_xyz.get(2);

            osg::Vec4ub curr_RGBA=curr_colors_ptr->at(i);
            rgba_data[4*i+0]=static_cast<char8_t>(curr_RGBA.r());
            rgba_data[4*i+1]=static_cast<char8_t>(curr_RGBA.g());
            rgba_data[4*i+2]=static_cast<char8_t>(curr_RGBA.b());
            rgba_data[4*i+3]=static_cast<char8_t>(curr_RGBA.a());
         } // loop over index i labeling vertices in *curr_Geometry_ptr

         tdp_file.klv_write( 
            TdpKeyXYZ_POINT_DATA,0,rel_xyz_data,n_xyz_bytes,tdp_data,
            xyz_byte_counter);
         delete [] rel_xyz_data;

         tdp_file.klv_write( 
            TdpKeyRGBA_COLOR_8,0,rgba_data,n_color_bytes,tdp_data,
            color_byte_counter);
         delete [] rgba_data;

         xyz_byte_counter += n_xyz_bytes;
         color_byte_counter += n_color_bytes;
      }
*/

// ------------------------------------------------------------------------
// Method write_geometry_relative_xyzrgba takes in a Geometry and
// loops over all its vertices.  It writes out x,y,z,R,G,B,A values to
// an output TDP file.

   void write_geometry_relative_xyzrgba(
      const threevector& zeroth_xyz,Tdp_file& tdp_file,
      int& xyz_byte_counter,int& color_byte_counter,
      ColormapPtrs* ColormapPtrs_ptr,osg::Geometry* curr_Geometry_ptr,
      const osg::Matrix& LocalToWorld)
      { 
//         cout << "ColormapPtrs_ptr = " << ColormapPtrs_ptr << endl;
         if (ColormapPtrs_ptr == NULL)
         {
            cout << "Trouble in scenegraphfunc::write_geometry_relative_xyzrgba()" 
                 << endl;
            cout << "ColormapPtrs_ptr = NULL" << endl;
            exit(-1);
         }
   
// First retrieve ColorMaps from ColormapPtrs object:

         ColorMap* height_ColorMap_ptr=
            ColormapPtrs_ptr->get_height_colormap_ptr();
//   cout << "height_ColorMap_ptr = " << height_ColorMap_ptr << endl;
//   cout << "*height_ColorMap_ptr = " << *height_ColorMap_ptr << endl;
//         ColorMap* prob_ColorMap_ptr=
//            ColormapPtrs_ptr->get_prob_colormap_ptr();

         osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
            curr_Geometry_ptr->getVertexArray());

         model::Metadata* curr_metadata_ptr=
            model::getMetadataForGeometry(*curr_Geometry_ptr);

// If curr_Geometry_ptr does not already contain a ColorArray, we
// instantiate one and initially fill it up with black entries:

         osg::Vec4ubArray* curr_colors_ptr=dynamic_cast<osg::Vec4ubArray*>(
            curr_Geometry_ptr->getColorArray());

         if (curr_colors_ptr==NULL)
         {
            curr_colors_ptr=instantiate_color_array(
               curr_vertices_ptr->size(),true,curr_Geometry_ptr,
               get_fixed_colors_label());
            curr_Geometry_ptr->setColorArray(curr_colors_ptr);
            curr_Geometry_ptr->setColorBinding(
               osg::Geometry::BIND_PER_VERTEX);

// On 10/28/10, Ross Anderson told us that the geometry's stateset has
// its blend state attribute enabled before alpha-blending for
// points can work:

            osg::StateSet* stateset_ptr=curr_Geometry_ptr->
               getOrCreateStateSet();
            stateset_ptr->setMode(GL_BLEND,osg::StateAttribute::ON);
         }

         int npoints=curr_vertices_ptr->size();
         const int nfloats_per_point=3;
         const int nchars_per_point=4;
         int n_xyz_bytes=npoints*nfloats_per_point*sizeof(real32_t);
         int n_color_bytes=npoints*nchars_per_point*sizeof(char8_t);
         real32_t* rel_xyz_data=new real32_t[n_xyz_bytes];
         char8_t* rgba_data=new char8_t[n_color_bytes];
         const unsigned char alpha_byte=
            stringfunc::ascii_integer_to_unsigned_char(255);

         for (int i=0; i<npoints; i++)
         {
            float curr_x=curr_vertices_ptr->at(i).x()*LocalToWorld(0,0)
               +curr_vertices_ptr->at(i).y()*LocalToWorld(1,0)
               +curr_vertices_ptr->at(i).z()*LocalToWorld(2,0)
               +LocalToWorld(3,0);
            float curr_y=curr_vertices_ptr->at(i).x()*LocalToWorld(0,1)
               +curr_vertices_ptr->at(i).y()*LocalToWorld(1,1)
               +curr_vertices_ptr->at(i).z()*LocalToWorld(2,1)
               +LocalToWorld(3,1);
            float curr_z=curr_vertices_ptr->at(i).x()*LocalToWorld(0,2)
               +curr_vertices_ptr->at(i).y()*LocalToWorld(1,2)
               +curr_vertices_ptr->at(i).z()*LocalToWorld(2,2)
               +LocalToWorld(3,2);

            rel_xyz_data[3*i+0]=curr_x-zeroth_xyz.get(0);
            rel_xyz_data[3*i+1]=curr_y-zeroth_xyz.get(1);
            rel_xyz_data[3*i+2]=curr_z-zeroth_xyz.get(2);

// Color according to fused combination of z and p.  Correlate height with
// hue and intensity.  Correlate intensity with saturation.

            int dependent_var=2;
            osg::Vec4ub curr_rgba_bytes=height_ColorMap_ptr->
               retrieve_curr_color(curr_z,dependent_var);
            colorfunc::RGBA curr_RGBA=colorfunc::bytes_to_RGBA(
               curr_rgba_bytes);
         
            double hue,s,value;
            colorfunc::RGB_to_hsv(
               curr_RGBA.first,curr_RGBA.second,curr_RGBA.third,hue,s,value);
            
// For ALIRT theater imagery, intensity p values tend to be very close
// to zero.  So we raise them to a small fractional power in order to
// distribute saturations more evenly across [0,1]:

            double p=curr_metadata_ptr->get(i,0);
//            double saturation=1-pow(p,0.28);
            double saturation=1-pow(p,0.45);

/*
         cout << " R=" << curr_RGBA.first
              << " G=" << curr_RGBA.second
              << " B=" << curr_RGBA.third
              << " H=" << h
              << " S=" << s
              << " V=" << v << endl;
*/
     
            colorfunc::RGB_bytes rgb_bytes=colorfunc::RGB_to_bytes(
               colorfunc::hsv_to_RGB(colorfunc::HSV(hue,saturation,value)));

            rgba_data[4*i+0]=static_cast<char8_t>(rgb_bytes.first);
            rgba_data[4*i+1]=static_cast<char8_t>(rgb_bytes.second);
            rgba_data[4*i+2]=static_cast<char8_t>(rgb_bytes.third);
            rgba_data[4*i+3]=static_cast<char8_t>(alpha_byte);
         } // loop over index i labeling vertices in *curr_Geometry_ptr

         tdp_file.klv_write( 
            TdpKeyXYZ_POINT_DATA,0,rel_xyz_data,n_xyz_bytes,tdp_data,
            xyz_byte_counter);
         delete [] rel_xyz_data;

         tdp_file.klv_write( 
            TdpKeyRGBA_COLOR_8,0,rgba_data,n_color_bytes,tdp_data,
            color_byte_counter);
         delete [] rgba_data;

         xyz_byte_counter += n_xyz_bytes;
         color_byte_counter += n_color_bytes;
      }

} // scenegraphfunc namespace




