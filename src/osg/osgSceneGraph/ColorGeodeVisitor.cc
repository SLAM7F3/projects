// ========================================================================
// ColorGeodeVisitor class member function definitions
// ========================================================================
// Last updated on 2/5/11; 11/27/11; 12/2/11
// ========================================================================

#include <osg/Geometry>
#include "osg/osgSceneGraph/ColorGeodeVisitor.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "astro_geo/latlong2utmfuncs.h"
#include "model/Metadata.h"
#include "osg/osgfuncs.h"
#include "general/outputfuncs.h"
#include "osg/osgSceneGraph/scenegraphfuncs.h"
#include "general/stringfuncs.h"
#include "osg/osgSceneGraph/UpdateColormapCallback.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ColorGeodeVisitor::allocate_member_objects()
{
   long_lat_ID_map_ptr=new LONGLATID_MAP;
}

void ColorGeodeVisitor::initialize_member_objects()
{
   Visitor_name="ColorGeodeVisitor";
   fixed_to_mutable_colors_flag=false;
   mutable_to_fixed_colors_flag=!fixed_to_mutable_colors_flag;

   p_ID_start=p_ID_stop=0;
   p_counter=0;
   p_magnification_factor=1;
}

ColorGeodeVisitor::ColorGeodeVisitor():
   MyNodeVisitor()
{ 
//   cout << "inside ColorGeodeVisitor constructor #1, this = " << this << endl;
   allocate_member_objects();
   initialize_member_objects();
} 

ColorGeodeVisitor::ColorGeodeVisitor(
   ColorMap* height_CM_ptr,ColorMap* prob_CM_ptr):
   MyNodeVisitor()
{ 
//   cout << "inside ColorGeodeVisitor constructor #2, this = " << this << endl;
   allocate_member_objects();
   initialize_member_objects();
   set_height_ColorMap_ptr(height_CM_ptr);

   if (prob_CM_ptr != NULL)
   {
      set_prob_ColorMap_ptr(prob_CM_ptr);
   }
   else
   {
      prob_ColorMap_ptr=height_ColorMap_ptr;
   }
   outputfunc::enter_continue_char();
} 

ColorGeodeVisitor::~ColorGeodeVisitor()
{
   delete long_lat_ID_map_ptr;
}

// ========================================================================
// Set & get member functions
// ========================================================================

void ColorGeodeVisitor::set_height_ColorMap_ptr(ColorMap* height_CM_ptr)
{
//   cout << "inside ColorGeodeVisitor::set_height_ColorMap_ptr()" << endl;
//   cout << "*height_CM_ptr = " << *height_CM_ptr << endl;

   height_ColorMap_ptr=height_CM_ptr;

   UpdateColormapCallback* UpdateCallback_ptr=
      dynamic_cast<UpdateColormapCallback*>(
         height_CM_ptr->get_UpdateCallback_ptr());
   UpdateCallback_ptr->set_ColorGeodeVisitor_ptr(this);
   addCullCallback(UpdateCallback_ptr);
}

ColorMap* ColorGeodeVisitor::get_height_ColorMap_ptr()
{
   return height_ColorMap_ptr;
}

void ColorGeodeVisitor::set_prob_ColorMap_ptr(ColorMap* prob_CM_ptr)
{
//   cout << "inside ColorGeodeVisitor::set_prob_ColorMap_ptr()" << endl;
//   cout << "*prob_CM_ptr = " << *prob_CM_ptr << endl;

   prob_ColorMap_ptr=prob_CM_ptr;
//   cout << "prob_ColorMap_ptr = " << prob_ColorMap_ptr << endl;

   if (prob_ColorMap_ptr != height_ColorMap_ptr)
   {
      UpdateColormapCallback* UpdateCallback_ptr=
         dynamic_cast<UpdateColormapCallback*>(
            prob_CM_ptr->get_UpdateCallback_ptr());
      UpdateCallback_ptr->set_ColorGeodeVisitor_ptr(this);
      addCullCallback(UpdateCallback_ptr);
   }
}

ColorMap* ColorGeodeVisitor::get_prob_ColorMap_ptr()
{
   return prob_ColorMap_ptr;
}

// ========================================================================
void ColorGeodeVisitor::set_ptwoDarray_ptr(twoDarray* input_ptwoDarray_ptr)
{
//   cout << "inside ColorGeodeVisitor::set_ptwoDarray_ptr()" << endl;
   clear_ptwoDarray_ptrs();
   push_back_ptwoDarray_ptr(input_ptwoDarray_ptr);
}

unsigned int ColorGeodeVisitor::get_n_ptwoDarray_ptrs() const
{
   return ptwoDarray_ptrs.size();
}

void ColorGeodeVisitor::push_back_ptwoDarray_ptr(
   twoDarray* input_ptwoDarray_ptr)
{
//   cout << "inside ColorGeodeVisitor::push_back_ptwoDarray_ptr()" << endl;
   ptwoDarray_ptrs.push_back(input_ptwoDarray_ptr);
//   cout << "ptwoDarray_ptrs.size() = " << ptwoDarray_ptrs.size() << endl;
//   outputfunc::enter_continue_char();
}

void ColorGeodeVisitor::push_back_ptwoDarray_ptrs(
   const std::vector<twoDarray*> input_ptwoDarray_ptrs)
{
//   cout << "inside ColorGeodeVisitor::push_back_ptwoDarray_ptrs()" << endl;
   for (unsigned int i=0; i< input_ptwoDarray_ptrs.size(); i++)
   {
      push_back_ptwoDarray_ptr(input_ptwoDarray_ptrs[i]);
   }
}

void ColorGeodeVisitor::clear_ptwoDarray_ptrs()
{
//   cout << "inside ColorGeodeVisitor::clear_ptwoDarray_ptrs()" << endl;
   ptwoDarray_ptrs.clear();
}

// ------------------------------------------------------------------------
// This apply method takes in a Geode which may either be a static
// resident in memory or may have been recently paged into memory.  It
// first attempts to retrieve a Geometry containing vertices from the
// Geode.  It next instantiates a corresponding color array if one
// does not already exist within the Geometry.  This method then fills
// the array with colors based upon the current independent variable.

// Following Ross Anderson's recommendation, we wrote this method (and
// indeed this entire class) to add colors to paged data after they
// are read into memory from disk.

void ColorGeodeVisitor::apply(osg::Geode& currGeode) 
{ 
//   cout << "Inside ColorGeodeVisitor::apply(Geode)" << endl;
//   cout << "Geode = " << &currGeode << endl;
//   cout << "classname = " << currGeode.className() << endl;
//   cout << "fixed_to_mutable_colors = " << fixed_to_mutable_colors_flag
//        << " mutable_to_fixed_colors = " << mutable_to_fixed_colors_flag
//        << endl;

   MyNodeVisitor::apply(currGeode);
}

// ------------------------------------------------------------------------
// Member function color_geometry_vertices() takes in a Geometry and
// determines whether it contains a ColorArray.  If not, it
// instantiates one and initially fills it up with blank entries.
// This method then loops over every vertex within the Geometry and
// assigns a corresponding color based upon the current colormap and
// its dependent Z or P variable or a fused combination of the two.

void ColorGeodeVisitor::color_geometry_vertices(
   osg::Geometry* curr_Geometry_ptr)
{ 
//   cout << "inside ColorGeodevisitor::color_geometry_vertices()" << endl;

//   cout << "initial height_ColorMap_ptr = " << height_ColorMap_ptr << endl;
//   cout << "LocalToWorld = " << endl;
//   osgfunc::print_matrix(LocalToWorld);

//   osg::Matrixd localToWorld = osg::computeLocalToWorld( getNodePath() );
//   cout << "Ross' localToWorld = " << endl;
//   osgfunc::print_matrix(localToWorld);

//   cout << "ColormapPtrs_ptr = " << ColormapPtrs_ptr << endl;
   if (ColormapPtrs_ptr == NULL)
   {
      cout << "Trouble in ColorGeodeVisitor::color_geom_vertices()" << endl;
      cout << "ColormapPtrs_ptr = NULL" << endl;
      cout << "this = " << this << endl;
      exit(-1);
   }
   
// First retrieve ColorMaps from ColormapPtrs object:

   height_ColorMap_ptr=ColormapPtrs_ptr->get_height_colormap_ptr();
//   cout << "height_ColorMap_ptr = " << height_ColorMap_ptr << endl;
//   cout << "*height_ColorMap_ptr = " << *height_ColorMap_ptr << endl;
//   cout << "height_ColorMap_ptr->get_dependent_var() = "
//        << height_ColorMap_ptr->get_dependent_var() << endl;

   prob_ColorMap_ptr=ColormapPtrs_ptr->get_prob_colormap_ptr();

   double zmin_threshold=height_ColorMap_ptr->get_min_threshold(2);
   double zmax_threshold=height_ColorMap_ptr->get_max_threshold(2);
//   cout << "zmin_threshold = " << zmin_threshold << endl;
//   cout << "zmax_threshold = " << zmax_threshold << endl;

   double pmin_threshold=prob_ColorMap_ptr->get_min_threshold(3);
   double pmax_threshold=prob_ColorMap_ptr->get_max_threshold(3);
//   cout << "pmin_threshold = " << pmin_threshold << endl;
//   cout << "pmax_threshold = " << pmax_threshold << endl;
   
   osg::Vec3Array* curr_vertices_ptr=dynamic_cast<osg::Vec3Array*>(
      curr_Geometry_ptr->getVertexArray());

// If curr_Geometry_ptr does not already contain a ColorArray, we
// instantiate one and initially fill it up with black entries:

   osg::Vec4ubArray* curr_colors_ptr=dynamic_cast<osg::Vec4ubArray*>(
      curr_Geometry_ptr->getColorArray());
   
   if (curr_colors_ptr==NULL)
   {
      curr_colors_ptr=scenegraphfunc::instantiate_color_array(
         curr_vertices_ptr->size(),true,curr_Geometry_ptr,
         scenegraphfunc::get_mutable_colors_label());
      curr_Geometry_ptr->setColorArray(curr_colors_ptr);
      curr_Geometry_ptr->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

// On 10/28/10, Ross Anderson told us that the geometry's stateset has
// its blend state attribute enabled before alpha-blending for
// points can work:

      osg::StateSet* stateset_ptr=curr_Geometry_ptr->getOrCreateStateSet();
      stateset_ptr->setMode(GL_BLEND,osg::StateAttribute::ON);
   }

   model::Metadata* curr_metadata_ptr=
      model::getMetadataForGeometry(*curr_Geometry_ptr);
//   cout << "curr_metadata_ptr = " << curr_metadata_ptr << endl;

//   cout << "LocalToWorld = " << endl;
//   osgfunc::print_matrix(LocalToWorld);
   
//   double saturation=0.1;
//   double saturation=0.9;
   double saturation=1.0;
   const unsigned char alpha_byte=
      stringfunc::ascii_integer_to_unsigned_char(255);
   colorfunc::RGB_bytes rgb_bytes;

//   cout << "height_ColorMap_ptr->get_dependent_var() = "
//        << height_ColorMap_ptr->get_dependent_var() << endl;

// Color according to z:

   if (height_ColorMap_ptr->get_dependent_var()==2 ||
       (height_ColorMap_ptr->get_dependent_var()==3 &&
        curr_metadata_ptr==NULL))
   {
      for (int i=0; i<int(curr_vertices_ptr->size()); i++)
      {
         double curr_z=curr_vertices_ptr->at(i).x()*LocalToWorld(0,2)
            +curr_vertices_ptr->at(i).y()*LocalToWorld(1,2)
            +curr_vertices_ptr->at(i).z()*LocalToWorld(2,2)
            +LocalToWorld(3,2);
//         cout << "i = " << i << " curr_z = " << curr_z << endl;
         curr_colors_ptr->at(i)=height_ColorMap_ptr->retrieve_curr_color(
            curr_z);
      } // loop over index i labeling vertices
   }

// Color according to p if necessary metadata exists:

   else if (height_ColorMap_ptr->get_dependent_var()==3)
   {
//      cout << "Color geoms according to p" << endl;
//      cout << "curr_Geom_ptr = " << curr_Geometry_ptr 
//           << " curr_vertices_ptr->size() = " << curr_vertices_ptr->size()
//           << endl;
      unsigned int px,py;
      double longitude,latitude;
      twovector longlat;
      for (unsigned int i=0; i<curr_vertices_ptr->size(); i++)
      {
         double curr_x=curr_vertices_ptr->at(i).x()*LocalToWorld(0,0)
            +curr_vertices_ptr->at(i).y()*LocalToWorld(1,0)
            +curr_vertices_ptr->at(i).z()*LocalToWorld(2,0)
            +LocalToWorld(3,0);
         double curr_y=curr_vertices_ptr->at(i).x()*LocalToWorld(0,1)
            +curr_vertices_ptr->at(i).y()*LocalToWorld(1,1)
            +curr_vertices_ptr->at(i).z()*LocalToWorld(2,1)
            +LocalToWorld(3,1);

// As of Mar 2009, we check contents of ptwoDarray_ptrs for
// Afghanistan LOS ray tracing results.  Unit [zero] probability value
// within this twoDarray indicates clear [blocked] line-of-sight.  Set
// hue of ground voxels for the former [latter] case to green [red].
// Take ground voxels' intensity value to equal metadata coming from
// CIB imagery.

         bool color_according_to_metadata_flag=true;

// If just a single ptwoDarray has been loaded into the
// ColorGeodeVisitor's ptwoDarray_ptrs vector, then its ID must be 0.
// If several ptwoDarrays have been loaded (e.g. corresponding to
// multiple averaged LOS ptwoDarray tiles), we can convert the current
// vertex's easting and northing coordinates into quantized longitude
// and latitude coordinates.  The ID corresponding to the ptwoDarray
// holding LOS raytracing info is then recoverable from member STL map
// *long_lat_ID_map_ptr.  

// But performing the UTM to long,lat coordinate conversion is
// expensive.  Since vertices within a geometry are strongly
// correlated, we can get away with performing this computation once
// in a while and reusing the ptwoDarray ID multiple times.  Only
// perform UTM to long,lat conversion when p_counter reaches some
// counter threshold value.

         int counter_threshold=10;
//         cout << "ptwoDarray_ptrs.size() = " 
//              << ptwoDarray_ptrs.size() << endl;
         
         if (ptwoDarray_ptrs.size()==0)
         {
            p_ID_start=p_ID_stop=-1;
         }
         else if (ptwoDarray_ptrs.size()==1)
         {
            p_ID_start=p_ID_stop=0;
         }
         else if (ptwoDarray_ptrs.size()==2)
         {
            p_ID_start=0;
            p_ID_stop=1;
         }
         else
         {
//            cout << "curr_x = " << curr_x << " curr_y = " << curr_y << endl;
            if (p_counter==0)
            {
               latlongfunc::UTMtoLL(
                  get_UTM_zone(),get_northern_hemisphere_flag(),
                  curr_y,curr_x,latitude,longitude);
               longlat=twovector(basic_math::mytruncate(longitude),
                                 basic_math::mytruncate(latitude));
//            cout << "longitude = " << longitude 
//                 << " latitude = " << latitude << endl;
//            cout << "longlat = " << longlat
//                 << " ptwoDarray_ID = "
//                 << get_ptwoDarray_ID(longlat) << endl;
               p_ID_start=p_ID_stop=get_ptwoDarray_ID(longlat);
            }
            p_counter++;
            if (p_counter==counter_threshold) p_counter=0;
//            cout << "p_ID_start = " << p_ID_start 
//   		   << " p_ID_stop = " << p_ID_stop
//                 << " p_counter = " << p_counter << endl;
         } // ptwoDarray_ptrs.size() > 1 conditional

//         cout << "p_ID_start = " << p_ID_start 
//              << " p_ID_stop = " << p_ID_stop << endl;

         for (int p_ID=p_ID_start; p_ID <= p_ID_stop && p_ID >= 0 ; p_ID++)
         {
            if (ptwoDarray_ptrs[p_ID]->point_to_pixel(curr_x,curr_y,px,py))
            {
               double curr_p=ptwoDarray_ptrs[p_ID]->get(px,py);
               if (curr_p >= -0.01 && curr_p <= 1.01)
//               if (curr_p >= 0 && curr_p <= 1)
               {
                  double hue=120*curr_p;
                  double value=curr_metadata_ptr->get(i,0);
                  if (p_magnification_factor > 1) magnify_p(value);

                  rgb_bytes=colorfunc::RGB_to_bytes(
                     colorfunc::hsv_to_RGB(
                        colorfunc::HSV(hue,saturation,value)));

                  curr_colors_ptr->at(i)=osg::Vec4ub(
                     rgb_bytes.first , rgb_bytes.second , rgb_bytes.third , 
                     alpha_byte);
                  color_according_to_metadata_flag=false;
               }

            } // point_to_pixel conditional
         } // loop over p_ID

         if (color_according_to_metadata_flag)
         {
            double curr_p=curr_metadata_ptr->get(i,0);
            if (p_magnification_factor > 1) magnify_p(curr_p);

// On 11/17/2011, we decided to qualitatively change the way
// thresholding works for p-data.  The colors corresponding to certain
// p=0, p=0.1, p=0.2 etc are now held fixed.  But if the current
// point's p value lies outside the interval
// [pmin_threshold,pmax_threshold], we now effectively set its color
// to black:

            double frac=curr_p;
            if (curr_p < pmin_threshold || curr_p > pmax_threshold)
            {
               frac=-1;
//               cout << "pmin_thresh = " << pmin_threshold
//                    << " curr_p = " << curr_p
//                    << " pmax_thresh = " << pmax_threshold << endl;
            }
            
//            double frac=(curr_p-pmin_threshold)/
//               (pmax_threshold-pmin_threshold);
//            frac=basic_math::max(0.0,frac);
//            frac=basic_math::min(1.0,frac);
            curr_colors_ptr->at(i)=prob_ColorMap_ptr->retrieve_frac_color(
               frac);

//            curr_colors_ptr->at(i)=prob_ColorMap_ptr->retrieve_curr_color(
//               curr_p);
         }
      } // loop over index i labeling vertices
   }

// Color according to fused combination of z and p.  Correlate height
// with hue and intensity with value.  This coloring scheme works well
// for LOST:

   else if ((height_ColorMap_ptr->get_dependent_var()==0) &&
             curr_metadata_ptr != NULL)
   {
      const double v_lo=0.0;		// OK for LOST
      const double v_hi=1.0;		

//      const double hue_lo=0;		// red
      const double hue_lo=30;		// orange	 - OK for LOST
      const double hue_hi=240;		// blue		 - OK for LOST
//      const double hue_hi=300;	// purple

      double hue,value;
      colorfunc::RGB_bytes curr_RGB_bytes;
      for (int i=0; i<int(curr_vertices_ptr->size()); i++)
      {
         double z=curr_vertices_ptr->at(i).x()*LocalToWorld(0,2)
            +curr_vertices_ptr->at(i).y()*LocalToWorld(1,2)
            +curr_vertices_ptr->at(i).z()*LocalToWorld(2,2)
            +LocalToWorld(3,2);
         double p=curr_metadata_ptr->get(i,0);
         if (p_magnification_factor > 1) magnify_p(p);

         xyzpfunc::convert_zp_to_hue_and_intensity(
            zmax_threshold,zmin_threshold,z,p,v_hi,v_lo,hue,value,
            hue_hi,hue_lo);

         rgb_bytes=colorfunc::RGB_to_bytes(
            colorfunc::hsv_to_RGB(
               colorfunc::HSV(hue,saturation,value)));
         curr_colors_ptr->at(i)=osg::Vec4ub(
            rgb_bytes.first , rgb_bytes.second , rgb_bytes.third , 
            alpha_byte);
      } // loop over index i labeling vertices
   } 

// Color according to fused combination of z and p.  Correlate height with
// hue and intensity.  Correlate intensity with saturation.  Yields
// reasonable coloring for ALIRT 2010 imagery.

   else if ((height_ColorMap_ptr->get_dependent_var()==1) &&
            curr_metadata_ptr != NULL)
   {
//      const double v_lo=0.4;
//      const double v_hi=1.0;		

//      const double hue_lo=-60;  // purple
//      const double hue_hi=240;	// blue

      colorfunc::RGB_bytes curr_RGB_bytes;

      for (int i=0; i<int(curr_vertices_ptr->size()); i++)
      {
         double z=curr_vertices_ptr->at(i).x()*LocalToWorld(0,2)
            +curr_vertices_ptr->at(i).y()*LocalToWorld(1,2)
            +curr_vertices_ptr->at(i).z()*LocalToWorld(2,2)
            +LocalToWorld(3,2);
         double p=curr_metadata_ptr->get(i,0);
         if (p_magnification_factor > 1) magnify_p(p);

/*
         cout << "i=" << i
              << " z=" << z
              << " p=" << p << endl;
*/

         int dependent_var=2;
         osg::Vec4ub curr_rgba_bytes=height_ColorMap_ptr->
            retrieve_curr_color(z,dependent_var);
         colorfunc::RGBA curr_RGBA=colorfunc::bytes_to_RGBA(
            curr_rgba_bytes);
         
         double hue,s,value;
         colorfunc::RGB_to_hsv(
            curr_RGBA.first,curr_RGBA.second,curr_RGBA.third,hue,s,value);

// For ALIRT theater imagery, intensity p values tend to be very close
// to zero.  So we raise them to a small fractional power in order to
// distribute saturations more evenly across [0,1]:

         saturation=1-pow(p,0.28);

/*
         cout << " R=" << curr_RGBA.first
              << " G=" << curr_RGBA.second
              << " B=" << curr_RGBA.third
              << " H=" << h
              << " S=" << s
              << " V=" << v << endl;
*/
     
         rgb_bytes=colorfunc::RGB_to_bytes(
            colorfunc::hsv_to_RGB(colorfunc::HSV(hue,saturation,value)));
         curr_colors_ptr->at(i)=osg::Vec4ub(
            rgb_bytes.first , rgb_bytes.second , rgb_bytes.third , 
            alpha_byte);
      } // loop over index i labeling vertices

   } // conditional on height_ColorMap_ptr->get_dependent_var()

//   cout << "at end of ColorGeodevisitor::color_geometry_vertices()" << endl;
}

// ========================================================================
// LONG_LAT_ID map member functions
// ========================================================================

// Member function get_ptwoDarray_ID() takes in twovector longlat and
// searches member STL map *long_lat_ID_map_ptr for a corresponding
// ptwoDarray ID.  If none is found, this method returns -1.

int ColorGeodeVisitor::get_ptwoDarray_ID(const twovector& longlat)
{
//   cout << "inside ColorGeodeVisitor::get_ptwoDarray_ID()" << endl;
   LONGLATID_MAP::iterator iter=long_lat_ID_map_ptr->find(longlat);
   if (iter==long_lat_ID_map_ptr->end()) return -1;
   
   int ptwoDarray_ID=iter->second;
   return ptwoDarray_ID;
}

// ---------------------------------------------------------------------
void ColorGeodeVisitor::insert_long_lat_ID_map_entry(
   const twovector& longlat,int ptwoDarray_ID)
{
//   cout << "inside ColorGeodeVisitor::insert_long_lat_ID_map_entry()" << endl;

   int ID=get_ptwoDarray_ID(longlat);
   if (ID==-1)
   {
      (*long_lat_ID_map_ptr)[longlat]=ptwoDarray_ID;
   }
//   cout << "long_lat_ID_map_ptr->size() = " << long_lat_ID_map_ptr->size() 
//        << endl;
}

// ---------------------------------------------------------------------
void ColorGeodeVisitor::clear_long_lat_ID_map()
{
   long_lat_ID_map_ptr->clear();
}

// ---------------------------------------------------------------------
int ColorGeodeVisitor::get_long_lat_ID_map_size() const
{
   return long_lat_ID_map_ptr->size();
}

// ---------------------------------------------------------------------
// Member function print_long_lat_ID_map_contents()

void ColorGeodeVisitor::print_long_lat_ID_map_contents() const
{
   cout << "inside ColorGeodeVisitor::print_long_lat_ID_map_contents()" 
        << endl;
   cout << "long_lat_ID_map_ptr->size() = " << long_lat_ID_map_ptr->size() 
        << endl;

   int map_counter=0;
   for (LONGLATID_MAP::iterator itr=long_lat_ID_map_ptr->begin();
        itr != long_lat_ID_map_ptr->end(); ++itr)
   {
      cout << "map counter = " << map_counter++ << endl;
      cout << "longlat = " << itr->first
           << " ID = " << itr->second << endl;
   }
}
