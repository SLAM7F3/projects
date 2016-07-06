// ==========================================================================
// COLORMAPPTRS class member function definitions
// ==========================================================================
// Last modified on 4/2/07; 4/15/07; 4/22/07; 6/27/07
// ==========================================================================

#include <iostream>
#include "osg/osgSceneGraph/ColormapPtrs.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ColormapPtrs::initialize_member_objects()
{
   height_colormap_ptr=NULL;
   prob_colormap_ptr=NULL;
}		       

void ColormapPtrs::allocate_member_objects()
{
}		       

ColormapPtrs::ColormapPtrs()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

ColormapPtrs::~ColormapPtrs()
{	
}

void ColormapPtrs::set_height_colormap_ptr(ColorMap* CM_ptr)
{
//   std::cout << "inside ColormapPtrs::set_height_colormap_ptr()"
//             << std::endl;
//   cout << "this = " << this << endl;
//   cout << "CM_ptr = " << CM_ptr << endl;
   height_colormap_ptr=CM_ptr;
}

ColorMap* ColormapPtrs::get_height_colormap_ptr()
{
//   cout << "inside ColormapPtrs::get_height_colormap_ptr()" << endl;
//   cout << "this = " << this << endl;
//   cout << "height_colormap_ptr = " << height_colormap_ptr << endl;
   return height_colormap_ptr;
}

const ColorMap* ColormapPtrs::get_height_colormap_ptr() const
{
//   cout << "inside ColormapPtrs::get_height_colormap_ptr() const" << endl;
//   cout << "this = " << this << endl;
//   cout << "height_colormap_ptr = " << height_colormap_ptr << endl;
   return height_colormap_ptr;
}
