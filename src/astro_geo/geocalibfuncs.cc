// ==========================================================================
// GEOCALIB methods
// ==========================================================================
// Last modified on 4/19/07; 5/7/07; 6/8/07
// ==========================================================================

#include <iostream>
#include "math/genmatrix.h"
#include "math/Tensor.h"
#include "math/threevector.h"
#include "math/twovector.h"

using std::cout;
using std::endl;

namespace geocalibfunc
{

// Use "private" global tensors A, B and C to hold ALIRT/Google
// calibration information for Boston 2005 map:
   
   tensor* A_ptr=NULL;
   genmatrix* B_ptr=NULL;
   twovector* C_ptr=NULL;

   genmatrix* M_ptr=NULL;
   double height_magnification_factor=1.0;
   double avg_sealevel_Z=0;

// On 7/6/06, we used program TRANSFORM_COORDS to determine the best
// quadratic transformation of ALIRT's X and Y 2005 Boston map
// coordinates of the form

// 	            U^i = A^i_jk x^j x^k + B^i_j x^j + C^i 

// which depends only linearly upon the a priori unknown 12 parameters
// in tensors A, B and C.  Using 23 tiepoints selected from the 3D map
// and Google Earth which were well-distributed around the Boston, we
// found a 1.73 meter RMS residual discrepancy between the fitted and
// measured tie point features.

   void initialize_Boston_fit_tensor_params()
      {
         A_ptr=new tensor(2,2,2);
         B_ptr=new genmatrix(2,2);
         C_ptr=new twovector();

         A_ptr->put(0,0,0,     -1.9456196377e-07);
         A_ptr->put(0,0,1,	2.433285382e-07);
         A_ptr->put(0,1,0,	2.433285382e-07);
         A_ptr->put(0,1,1,	3.33586352323e-06);
         A_ptr->put(1,0,0,	3.71971128377e-07);
         A_ptr->put(1,0,1,	3.439194535e-07);
         A_ptr->put(1,1,0,	3.439194535e-07);
         A_ptr->put(1,1,1,	1.32162326156e-06);

         B_ptr->put(0,0,	0.994908931661);
         B_ptr->put(0,1,	0.129530978989);
         B_ptr->put(1,0,	0.0309029278203);
         B_ptr->put(1,1,	1.07000674583);

         C_ptr->put(0,	360074.894366);
         C_ptr->put(1,	4703847.16337);

//         cout << "*A_ptr = " << *A_ptr << endl;
//         cout << "*B_ptr = " << *B_ptr << endl;
//         cout << "*C_ptr = " << *C_ptr << endl;
      }

// --------------------------------------------------------------------------
// Perform linear transformation on Baghdad ladar easting & northing
// coordinates in order to align them with Baghdad satellite EO
// easting & northing coordinates.  We perform a linear rather than
// quadratic transformation, for both yield nearly the same RMS
// residual.  And the former should extrapolate better than the latter
// into regions were the satellite EO imagery does not overlap the
// ladar data...

   void initialize_Baghdad_fit_tensor_params()
      {
         A_ptr=new tensor(2,2,2);
         B_ptr=new genmatrix(2,2);
         C_ptr=new twovector();

         A_ptr->put(0,0,0,0);
         A_ptr->put(0,0,1,0);
         A_ptr->put(0,1,0,0);
         A_ptr->put(0,1,1,0);
         A_ptr->put(1,0,0,0);
         A_ptr->put(1,0,1,0);
         A_ptr->put(1,1,0,0);
         A_ptr->put(1,1,1,0);

         B_ptr->put(0,0,  1.000029259);
         B_ptr->put(0,1, -6.149983819e-05);
         B_ptr->put(1,0,  9.759377917e-05);
         B_ptr->put(1,1,  0.9999130063);

         C_ptr->put(0,	198.8826148);
         C_ptr->put(1,	272.0876808);

         avg_sealevel_Z=-(36.6-25);	// meters

//         cout << "*A_ptr = " << *A_ptr << endl;
//         cout << "*B_ptr = " << *B_ptr << endl;
//         cout << "*C_ptr = " << *C_ptr << endl;
      }

// --------------------------------------------------------------------------
   void initialize_NYC_RTV_fit_tensor_params()
      {
         B_ptr=new genmatrix(2,2);
         C_ptr=new twovector();

         B_ptr->put(0,0,	1.000413721);
         B_ptr->put(0,1,	-0.0001261733494);
         B_ptr->put(1,0,	-0.00016618094);
         B_ptr->put(1,1,	1.000109997);

         C_ptr->put(0,	359004.9);
         C_ptr->put(1,	4702897.0);

         avg_sealevel_Z=-81.47;	// meters

//         cout << "*A_ptr = " << *A_ptr << endl;
//         cout << "*B_ptr = " << *B_ptr << endl;
//         cout << "*C_ptr = " << *C_ptr << endl;
      }

// --------------------------------------------------------------------------
   void initialize_NYC_ALIRT_fit_tensor_params()
      {
         A_ptr=new tensor(2,2,2);
         B_ptr=new genmatrix(2,2);
         C_ptr=new twovector();

// Quadratic fit parameters:

         A_ptr->put(0,0,0,    	1.201308284e-06);
         A_ptr->put(0,0,1,     	-7.316196219e-07);
         A_ptr->put(0,1,0,	-7.316196219e-07);
         A_ptr->put(0,1,1,    	4.265402477e-07);
         A_ptr->put(1,0,0,    	-4.484390531e-08);
         A_ptr->put(1,0,1,    	2.504076252e-08);
         A_ptr->put(1,1,0,     	2.504076252e-08);
         A_ptr->put(1,1,1,   	-3.440400287e-09);

         B_ptr->put(0,0,	0.1823427633);
         B_ptr->put(0,1,	0.4910107057);
         B_ptr->put(1,0,	0.02835282152);
         B_ptr->put(1,1,	0.9879859788);

         C_ptr->put(0,	497619.2774);
         C_ptr->put(1,	4698713.712);

/*
// Linear fit parameters:

         B_ptr=new genmatrix(2,2);
         C_ptr=new twovector();

         B_ptr->put(0,0,	1.004099647);
         B_ptr->put(0,1,	-0.002338730435);
         B_ptr->put(1,0,	-0.001447176841);
         B_ptr->put(1,1,	1.000604888);

         C_ptr->put(0,	357755.36);
         C_ptr->put(1,	4703281.08);
*/

         avg_sealevel_Z=-78.89;	// meters

//         cout << "*A_ptr = " << *A_ptr << endl;
//         cout << "*B_ptr = " << *B_ptr << endl;
//         cout << "*C_ptr = " << *C_ptr << endl;
      }

// --------------------------------------------------------------------------
   void initialize_Lowell_RTV_fit_tensor_params()
      {
         A_ptr=new tensor(2,2,2);
         B_ptr=new genmatrix(2,2);
         C_ptr=new twovector();

// Quadratic fit parameters:

         A_ptr->put(0,0,0,    	1.024072146e-07);
         A_ptr->put(0,0,1,     	2.098814173e-08);
         A_ptr->put(0,1,0,	2.098814173e-08);
         A_ptr->put(0,1,1,    	3.430952686e-08);
         A_ptr->put(1,0,0,    	7.29716371e-08);
         A_ptr->put(1,0,1,    	-1.453118926e-08);
         A_ptr->put(1,1,0,      -1.453118926e-08);
         A_ptr->put(1,1,1,   	-1.836119189e-08);

         B_ptr->put(0,0,	1.00891854);
         B_ptr->put(0,1,	0.0007401367759);
         B_ptr->put(1,0,	0.00800971493);
         B_ptr->put(1,1,	0.9993305035);
         
         C_ptr->put(0,	359326.1796);
         C_ptr->put(1,	4703047.569);

// According to Google Earth, elevation of Lowell river nearby large
// rotary on northern shore is approximately 17 meters.  According to
// raw RTV map, raw elevation = -63.2 meters.  So we need to set
// avg_sealevel_z = - (17 +63.2) = -80.2 meters:

         avg_sealevel_Z=-80.2;	// meters 

//         cout << "*A_ptr = " << *A_ptr << endl;
//         cout << "*B_ptr = " << *B_ptr << endl;
//         cout << "*C_ptr = " << *C_ptr << endl;
      }

// --------------------------------------------------------------------------
// As of 6/23/06, member function compute_lat_long_alt performs a
// linear transformation on raw XYZ values coming from the original
// Boston data set.  Using latitude and longitude values extracted
// from Google Earth for markers located close to the surface (in
// order to minimize parallax distortions), we derived a 2x2 matrix
// plus a translation 2-vector which converts the XY values into UTM
// easting and northing values for zone 19T. 

// As of 7/7/06, member function compute_lat_long_alt performs a
// quadratic transformation on raw XYZ values coming from the original
// Boston data set.  Using latitude and longitude values manually
// extracted from Google Earth for markers located close to the
// surface (in order to minimize parallax distortions), we performed a
// least-squares fit for the 12 parameters within tensors A(2,2,2),
// B(2,2) and C(2,2) entering into the transformation 

// 	            U^i = A^i_jk x^j x^k + B^i_j x^j + C^i 

// which converts XY values into UTM easting and northing values for
// zone 19T.  


// We also used the following skyscraper height information which we
// found on the web:

// Skyscraper			Height (meters)

// Hancock			241
// Prudential			229
// Federal Reserve Bank 	187     

// plus the fact that the ocean basically lies at 0 elevation
// (relative to the ocean) in order to deduce Z_true = 1.05 (Z_alirt+
// 70.34).

// This method returns UTM_coords = (UTMEasting,UTMNorthing,altitude).

/*
   void compute_Boston_UTM(const threevector& p,threevector& UTM_coords)
      {
         genmatrix M(3,3);
         M.put(0,0,1.00099834235);
         M.put(0,1,0.0412387651285);
         M.put(1,0,0.00114270461243);
         M.put(1,1,1.02024243252);
         M.put(2,2,1.05);

         const threevector trans(359676.455566,4703133.08791,
                                 M.get(2,2)*70.3994);
         UTM_coords=M*p+trans;
      }
*/

   void compute_Boston_UTM(const threevector& p,threevector& UTM_coords)
      {
         if (A_ptr==NULL || B_ptr==NULL || C_ptr==NULL)
         {
            initialize_Boston_fit_tensor_params();
         }

         twovector X(p.get(0),p.get(1));
         genmatrix XX(X.outerproduct(X));
         tensor AXX( (A_ptr->outerproduct(XX)).
                     contract_adjacent_pairs(1) );
         UTM_coords=threevector(twovector( AXX ) + *B_ptr * X + *C_ptr,
                                1.05*(p.get(2)+70.3994 ));

//         cout << "UTM_coords = " << UTM_coords << endl;
      }

   void compute_Boston_UTM(
      const twovector& X,double z,twovector& UTM_coords,double& z_new)
      {
         double x=X.get(0);
         double y=X.get(1);
         UTM_coords=twovector(
            A_ptr->get(0,0,0)*sqr(x)+2*A_ptr->get(0,0,1)*x*y+
            A_ptr->get(0,1,1)*sqr(y)+B_ptr->get(0,0)*x+B_ptr->get(0,1)*y+
            C_ptr->get(0),
            A_ptr->get(1,0,0)*sqr(x)+2*A_ptr->get(1,0,1)*x*y+
            A_ptr->get(1,1,1)*sqr(y)+B_ptr->get(1,0)*x+B_ptr->get(1,1)*y+
            C_ptr->get(1));

         z_new=1.05*(z+70.3994);
      }

   void compute_transformed_UTM(
      const twovector& X,double z,twovector& UTM_coords,double& z_new)
      {
//         cout << "inside geocalibfunc::compute_NYC_UTM()" << endl;
//         cout << "X = " << X << endl;
//         cout << "*A_ptr = " << *A_ptr << endl;
//         cout << "*B_ptr = " << *B_ptr << endl;
//         cout << "*C_ptr = " << *C_ptr << endl;

         double x=X.get(0);
         double y=X.get(1);
         UTM_coords=twovector(
            A_ptr->get(0,0,0)*sqr(x)+2*A_ptr->get(0,0,1)*x*y+
            A_ptr->get(0,1,1)*sqr(y)+B_ptr->get(0,0)*x+B_ptr->get(0,1)*y+
            C_ptr->get(0),
            A_ptr->get(1,0,0)*sqr(x)+2*A_ptr->get(1,0,1)*x*y+
            A_ptr->get(1,1,1)*sqr(y)+B_ptr->get(1,0)*x+B_ptr->get(1,1)*y+
            C_ptr->get(1));

//            B_ptr->get(0,0)*x+B_ptr->get(0,1)*y+C_ptr->get(0),
//            B_ptr->get(1,0)*x+B_ptr->get(1,1)*y+C_ptr->get(1));

//         const double avg_sealevel_Z=-81.47;	// RTV NYC data (meters)
//         const double avg_sealevel_Z=-78.89;	// ALIRT-A NYC data (meters)
         z_new=height_magnification_factor*(z-avg_sealevel_Z);
      }

} // GEOCALIB namespace
