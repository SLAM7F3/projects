// ==========================================================================
// SATELLITEIMAGE derived class member function definitions
// ==========================================================================
// Last modified on 4/27/06; 7/20/06; 7/26/06; 10/8/11
// ==========================================================================

#include <iostream>
#include "space/satellite.h"
#include "space/satelliteimage.h"
#include "space/satellitepass.h"
#include "astro_geo/ground_radar.h"
#include "image/TwoDarray.h"

using std::cout;
using std::endl;

// ==========================================================================
// Initialization, constructor and destructor methods:
// ==========================================================================

void satelliteimage::allocate_member_objects()
{
   curr_target_ptr=new satellite();
   curr_ground_radar_ptr=new ground_radar();
}		       

void satelliteimage::initialize_member_objects()
{
}

satelliteimage::satelliteimage(satellitepass* satpass_ptr)
{
   pass_ptr=satpass_ptr;
   allocate_member_objects();
   initialize_member_objects();

}

// On 11/13/01, we learned from Tara Dennis that the following syntax
// explicitly calls the myimage constructor before the remainder of
// the satellite image constructor is executed:

satelliteimage::satelliteimage(
   int Nxbins,int Nybins,satellitepass* satpass_ptr):
   myimage(Nxbins,Nybins)
{
   pass_ptr=satpass_ptr;
   allocate_member_objects();
   initialize_member_objects();
}

satelliteimage::satelliteimage(const twoDarray& T,satellitepass* satpass_ptr):
   myimage(T)
{		  
   initialize_member_objects();
   allocate_member_objects();
   pass_ptr=satpass_ptr;
}

// Copy constructor:

satelliteimage::satelliteimage(const satelliteimage& m):
   myimage(m)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(m);
}

satelliteimage::~satelliteimage()
{
   delete curr_target_ptr;
   delete curr_ground_radar_ptr;
}

// ---------------------------------------------------------------------
void satelliteimage::docopy(const satelliteimage & m)
{
}

// Overload = operator:

satelliteimage& satelliteimage::operator= (const satelliteimage& m)
{
   if (this==&m) return *this;
   myimage::operator=(m);
   docopy(m);
   return *this;
}

// ==========================================================================
// Image property member functions 
// ==========================================================================

// On 1/22/02, we realized (to our horror) that the alignment of our
// raw RH images did not agree with that of XELIAS.  We learned from
// Forrest Hunsberger that a center location is placed by ARIES into
// imagecdf files which indicates which pixel within the image array
// corresponds to the image center.  This method computes the values
// of cross range and range member variables x_center and y_center
// corresponding to this center pixel.  

void satelliteimage::compute_ARIES_image_center(twoDarray* ztwoDarray_ptr)
{
// We do not call member function myimage::pixel_to_point(), for pixel
// locations px_center and py_center generally contain some nonzero
// fractional part...

   double xoffset=px_center*ztwoDarray_ptr->get_deltax();
   double yoffset=py_center*ztwoDarray_ptr->get_deltay();
   x_center=ztwoDarray_ptr->get_xlo()+xoffset;
   y_center=ztwoDarray_ptr->get_yhi()-yoffset;
//   cout << "xcenter = " << x_center << " ycenter = " << y_center << endl;
}

