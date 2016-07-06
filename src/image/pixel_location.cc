// =========================================================================
// Pixel_Location class member function definitions
// =========================================================================
// Last modified on 7/16/12; 7/17/12
// =========================================================================

#include <iostream>
#include "image/graphicsfuncs.h"
#include "image/pixel_location.h"
#include "general/outputfuncs.h"

using std::cout;
using std::endl;
using std::flush;
using std::ostream;
using std::pair;
using std::vector;


// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void pixel_location::allocate_member_objects()
{
}

void pixel_location::initialize_member_objects()
{
   px=py=-1;
   image_width=image_height=ID=-1;
   intensity=-1;
}		 

// ---------------------------------------------------------------------
pixel_location::pixel_location()
{
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
pixel_location::pixel_location(int px,int py,int image_width,int image_height)
{
   allocate_member_objects();
   initialize_member_objects();
   this->px=px;
   this->py=py;
   this->image_width=image_width;
   this->image_height=image_height;
   ID=graphicsfunc::get_pixel_ID(px,py,image_width);
}

// ---------------------------------------------------------------------
// Copy constructor:

pixel_location::pixel_location(const pixel_location& pl)
{
   docopy(pl);
}

pixel_location::~pixel_location()
{
//   cout << "inside pixel_location destructor" << endl;
}

// ---------------------------------------------------------------------
void pixel_location::docopy(const pixel_location& pl)
{
}

// Overload = operator:

pixel_location& pixel_location::operator= (const pixel_location& pl)
{
   if (this==&pl) return *this;
   docopy(pl);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const pixel_location&pl)
{
   outstream << "px = " << pl.get_px() 
             << " py = " << pl.get_py() 
             << " ID = " << pl.get_ID() 
             << " intensity = " << pl.get_intensity() << endl;
   return outstream;
}

// =========================================================================
// =========================================================================

vector<int> pixel_location::get_four_neighbor_IDs()
{
   vector<int> four_neighbor_pixel_IDs;
   if (py >= 1)
   {
      four_neighbor_pixel_IDs.push_back(
         graphicsfunc::get_pixel_ID(px,py-1,image_width));
   }
   if (py <= image_height-2)
   {
      four_neighbor_pixel_IDs.push_back(
         graphicsfunc::get_pixel_ID(px,py+1,image_width));
   }
   if (px >= 1)
   {
      four_neighbor_pixel_IDs.push_back(
         graphicsfunc::get_pixel_ID(px-1,py,image_width));
   }
   if (px <= image_width-2)
   {
      four_neighbor_pixel_IDs.push_back(
         graphicsfunc::get_pixel_ID(px+1,py,image_width));
   }
   return four_neighbor_pixel_IDs;
}
