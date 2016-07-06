// Note added on 7/26/06: User should be able to request colormap
// based upon its string name rather than its index value !!!

// ==========================================================================
// COLORMAP class member function definitions
// ==========================================================================
// Last modified on 11/19/11; 12/25/11; 2/22/13
// ==========================================================================

#include <algorithm>
#include <fstream>
#include <iostream>
#include <osg/StateSet>
#include "math/adv_mathfuncs.h"
#include "math/basic_math.h"
#include "osg/osgSceneGraph/ColorMap.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "osg/osgSceneGraph/UpdateColormapCallback.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ColorMap::initialize_member_objects()
{
   colormap_name.push_back("jet");				// 0
   colormap_name.push_back("jet_white");			// 1
   colormap_name.push_back("small_hue_value");			// 2
   colormap_name.push_back("large_hue_value");			// 3
   colormap_name.push_back("large_hue_value_sans_white");	// 4
   colormap_name.push_back("RGB");				// 5
   colormap_name.push_back("grey");				// 6
   colormap_name.push_back("pure_hue");				// 7
   colormap_name.push_back("wrap1");				// 8
   colormap_name.push_back("wrap2");				// 9
   colormap_name.push_back("wrap3");				// 10
   colormap_name.push_back("wrap4");				// 11
   colormap_name.push_back("wrap8");				// 12
   colormap_name.push_back("wrap16");				// 13
   colormap_name.push_back("reverse_large_hue_value_sans_white"); // 14
//   colormap_name.push_back("reverse_hue_value");
   colormap_name.push_back("land_sea");				// 15
   N_COLORMAPS=colormap_name.size();

   map=0;		// Default colormap = jet
   dependent_var=2;	// Default dependent variable = Z
   _currentUpdateIndex=1;
   cyclic_frac_offset=0;

   for (int i=0; i<3; i++)
   {
      min_threshold.put(i,POSITIVEINFINITY);
      max_threshold.put(i,NEGATIVEINFINITY);
   }
   max_threshold.put(3,1.0);
   min_threshold.put(3,0.0);

   null_color=osg::Vec4ub(0,0,0,0);
}		       

void ColorMap::allocate_member_objects()
{
   updateCallback_refptr = new UpdateColormapCallback(this);
   for (int n=0; n<N_COLORMAPS; n++)
   {
      colorarray_ptrs.push_back(new osg::Vec4ubArray);
   }
}		       

ColorMap::ColorMap()
{	
   initialize_member_objects();
   allocate_member_objects();
   directory_name=getenv("OSG_FILE_PATH");
   directory_name += "/3D_colormaps/";
//   cout << "Colormap directory = " << directory_name << endl;
   map=9;		// wrap3 colormap
   dependent_var=2;	// color based on Z values
   load_all_maps();
}		       

ColorMap::ColorMap(string dir_name,int p_map,int p_dependent_var)
{	
//   cout << "inside ColorMap constructor" << endl;
   initialize_member_objects();
   allocate_member_objects();
   directory_name=dir_name;
//   cout << "Colormap directory = " << directory_name << endl;
   map=p_map;
   dependent_var=p_dependent_var;
   load_all_maps();
}		       

ColorMap::~ColorMap()
{	
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const ColorMap& C)
{
   cout << "inside operator<< for ColorMap" << endl;
   cout << "&C = " << &C << endl;
   outstream << endl;
   outstream << "directory_name = " << C.directory_name << endl;
   outstream << "N_COLORMAPS = " << C.N_COLORMAPS << endl;
   outstream << "map = " << C.map << endl;
   outstream << "dependent_var = " << C.dependent_var << endl;
   outstream << "cyclic_frac_offset = " << C.cyclic_frac_offset << endl;
   
   return outstream;
}

// ==========================================================================
// Color loading & retrieval member functions
// ==========================================================================

// Member function load_all_maps loops over all colormaps and preloads
// them into STL member colorarray_ptrs.  This method should be called
// only once.

void ColorMap::load_all_maps()
{
   for (int m=0; m<N_COLORMAPS; m++) 
   {
      load(m,colorarray_ptrs[m]);
   }
}

// ---------------------------------------------------------------------
// Member function load takes in an ascii file of RGB values, converts
// them to osg::Vec4ubs and loads them into *curr_colorarray_ptr.

void ColorMap::load(int m,osg::Vec4ubArray* curr_colorarray_ptr)
{
//   cout << "inside ColorMap::load()" << endl;
   
   const unsigned char alpha_byte=
      static_cast<unsigned char>(stringfunc::ascii_integer_to_char(255));

   ifstream infile;
   float r,g,b;
   string full_pathname=directory_name+colormap_name[m]+".txt";
//   cout << "full_pathname = " << full_pathname << endl;
   infile.open(full_pathname.c_str(), std::ios::in);
   if (!infile)
   {
      cout << "Colormap text file " << colormap_name[m] << " not found!" 
           << endl;
   }
   
   infile >> r;
   infile >> g;
   infile >> b;
   while (infile)
   {
//      cout << "colormap r = " << r << " g = " << g << " b = " << b << endl;
      colorfunc::RGB_bytes curr_RGB_bytes=
         colorfunc::RGB_to_bytes(r,g,b,true);
      curr_colorarray_ptr->push_back(osg::Vec4ub(
         curr_RGB_bytes.first,curr_RGB_bytes.second,curr_RGB_bytes.third,
         alpha_byte));

      infile >> r;
      infile >> g;
      infile >> b;
   }
   infile.close();

// For thresholding purposes, we push back a final black color onto
// the colors Vec4ubArray:

//   curr_colorarray_ptr->push_back(osg::Vec4ub(0,0,0,0));
};

// ---------------------------------------------------------------------
// Member function load reads in an ascii file of RGB values ranging
// from 0 - 1, multiplies them by 255, and loads them into input STL
// vector colors.

void ColorMap::load(int m,vector<osg::Vec4>& colors)
{
   const float alpha=1.0;

   ifstream infile;
   float r,g,b;
   string full_pathname=directory_name+colormap_name[m]+".txt";
   infile.open(full_pathname.c_str(), std::ios::in);
   if (!infile)
   {
      cout << "Colormap text file " << colormap_name[m] << " not found!" 
           << endl;
   }
   
   infile >> r;
   infile >> g;
   infile >> b;
   while (infile)
   {
      colors.push_back(osg::Vec4(255*r,255*g,255*b,255*alpha));
      infile >> r;
      infile >> g;
      infile >> b;

   }
   infile.close();
};

// ---------------------------------------------------------------------
// Member function change_dependent_coloring_var()

void ColorMap::change_dependent_coloring_var(int var_increment)
{
//   cout << "inside ColorMap::change_dependent_coloring_var()" << endl;
//   cout << "var_increment = " << var_increment << endl;

   dependent_var += var_increment;
   dependent_var=modulo(dependent_var,get_n_dependent_vars());

   switch(dependent_var)
   {
      case 0: 
         cout << "Dependent coloring variable = X" << endl;
         break;
      case 1: 
         cout << "Dependent coloring variable = Y" << endl;
         break;
      case 2: 
         cout << "Dependent coloring variable = Z" << endl;
         break;
      case 3: 
         cout << "Dependent coloring variable = P" << endl;
         break;
   }
}
 
// ---------------------------------------------------------------------
// Member function turns input curr_value into a fraction ranging
// between 0 and 1.  It then converts the fraction into a colormap
// index and returns the osg::Vec4ub value corresponding to this
// index.  If the fraction lies outside [0,1], the topmost color in
// the map (which presumably is black ) is returned.

const osg::Vec4ub& ColorMap::retrieve_frac_color(double curr_frac) const
{
//   cout << "inside ColorMap::retrieve_frac_color, cyclic_frac_offset = "
//        << cyclic_frac_offset << " curr_frac = " << curr_frac << endl;
   
   int curr_colorindex= get_n_map_colors()-1;
   if ((curr_frac >= 0) && (curr_frac <= 1))
   {

// Allow user to cyclically permute output colors:

      double f=curr_frac+cyclic_frac_offset;
      if (f > 1) f -= 1.0;
      
      curr_colorindex=basic_math::round(f*(curr_colorindex-1));
//      cout << "curr_colorindex = " << curr_colorindex 
//           << " curr_colorarray.size() = "
//           << get_curr_colorarray_ptr()->size() << endl;

      return get_curr_colorarray_ptr()->at(curr_colorindex);
   }
   else
   {
//      cout << "curr_frac = " << curr_frac << " returning null color" << endl;
      return null_color;
   }
}

// ---------------------------------------------------------------------
const osg::Vec4ub& ColorMap::retrieve_curr_color(double curr_value) const
{
//   cout << "inside ColorMap::retrieve_curr_color() #1" << endl;
   return retrieve_curr_color(curr_value,get_dependent_var());
}

// On 11/19/2011, we've decided to qualitatively change the way
// thresholding works for z and p-data.  The colors corresponding to Z
// and P values are now held fixed.  But if the input curr_value lies
// outside the threshold interval, we now effectively set its color to
// black:

const osg::Vec4ub& ColorMap::retrieve_curr_color(
   double curr_value,int depend_var) const
{
//   cout << "inside ColorMap::retrieve_curr_color() #2" << endl;
//   cout << "Depend var = " << depend_var << endl;

   double max_value=get_max_value(depend_var);
   double min_value=get_min_value(depend_var);
   double curr_frac=(curr_value-min_value)/(max_value-min_value);
//   cout << "min = " << min_value << " curr = " << curr_value
//        << " max = " << max_value << " frac = " << curr_frac << endl;
   
   if (curr_value < get_min_threshold(depend_var) ||
       curr_value > get_max_threshold(depend_var))
   {
      curr_frac=-1;
//      cout << "curr_value = " << curr_value
//           << " min_threshold = " << get_min_threshold(depend_var)
//           << " max_threshold = " << get_max_threshold(depend_var)
//           << endl;
   }

//   cout << "min_thresh = " << get_min_threshold(depend_var)
//        << " max_thresh = " << get_max_threshold(depend_var) << endl;

   return retrieve_frac_color(curr_frac);
}

const colorfunc::RGBA ColorMap::retrieve_curr_RGBA(double curr_value) const
{
   return retrieve_curr_RGBA(curr_value,get_dependent_var());
}

const colorfunc::RGBA ColorMap::retrieve_curr_RGBA(
   double curr_value,int depend_var) const
{
   return colorfunc::bytes_to_RGBA(retrieve_curr_color(curr_value,depend_var));
}

// ==========================================================================
// Member functions copied from Ross' Colormap class
// ==========================================================================

// Apply the colormap to the given node, or disable the colormap for
// this node.

void ColorMap::setEnabled( osg::Node& n, bool enable )
{
//   cout << "inside ColorMap::setEnabled" << endl;
//   osg::StateSet* stateset = n.getOrCreateStateSet();

   if ( enable ) 
   {
/*
      if ( _texture.valid() ) 
      {
         // Apply texture to the root node
         stateset->setTextureAttributeAndModes( 
            _textureUnit, _texture.get(), StateAttribute::ON );
            
         osg::TexEnv* env = new osg::TexEnv( osg::TexEnv::MODULATE );	
         stateset->setTextureAttributeAndModes( 
            _textureUnit, env, StateAttribute::ON );
      }
*/
      
      // Apply current vertex program state

/*
      if ( _program.valid() ) 
      {
         osg::StateAttribute::Values	value = _use_vertex_program 
            ? osg::StateAttribute::ON : osg::StateAttribute::OFF;
            
         stateset->setAttributeAndModes( _program.get(), value );
      }
*/
      
      // add to list of enabled root nodes
      if ( std::find( _roots.begin(), _roots.end(), &n ) == _roots.end() )
         _roots.push_back( &n );
   } 
   else 
   {
      // clear state
/*
      stateset->setTextureAttributeAndModes( 
         _textureUnit, _texture.get(), StateAttribute::OFF );
      stateset->setAttributeAndModes( 
         _program.get(), osg::StateAttribute::OFF );
*/
      // remove from list of enabled root nodes

      NodeObserverList::iterator pos = std::find( 
         _roots.begin(), _roots.end(), osg::observer_ptr<osg::Node>(&n) );
      if ( pos != _roots.end() )
         _roots.erase( pos );
   }
}

// ---------------------------------------------------------------------
// Member function isEnabled() returns true if the input node is
// currently colormapped.

bool ColorMap::isEnabled( osg::Node& n )
{
   return std::find( _roots.begin(), _roots.end(), &n ) != _roots.end();
}

// ---------------------------------------------------------------------
// Member function get_UpdateCallback_ptr() returns the cull callback
// that must be called to update the colormap for each Geode.  You
// must attach this callback to every geode for the colormap to be
// updated correctly.

osg::NodeCallback* ColorMap::get_UpdateCallback_ptr() 
{ 
//    std::cout << "inside ColorMap::get_UpdateCallback_ptr()" << std::endl;
   return updateCallback_refptr.get(); 
}











osg::Vec4ubArray* ColorMap::get_curr_colorarray_ptr()
{
//   cout << "inside ColorMap::get_curr_colorarray_ptr(), map = " << map
//        << endl;
   return colorarray_ptrs.at(map);
}

const osg::Vec4ubArray* ColorMap::get_curr_colorarray_ptr() const
{
//   cout << "inside ColorMap::get_curr_colorarray_ptr(), map = " << map
//        << endl;
   return colorarray_ptrs.at(map);
}
