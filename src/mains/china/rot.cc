// ========================================================================
// Program ROT is a laboratory for computing relative rotations
// between photos.  It generates a set of direction vectors
// corresponding to features selected from a photo and assembles them
// into matrix P_3xn.  The direction vectors are then rotated through
// some azimuth, elevation and roll angles by matrix R_3x3 to form
// matrix Q_3xn.  After errors are added into the unrotated direction
// vectors, the rotation matrix is recovered by inverting

// 			   R_3x3 P_3xn= Q_3xn

// The azimuth, elevation and roll angles are reconstructed from the
// recovered R matrix.

// ========================================================================
// Last updated on 5/30/07; 10/27/08; 10/29/08
// ========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "math/genmatrix.h"
#include "numrec/nrfuncs.h"
#include "math/rotation.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;

// ========================================================================
int main (int argc, char** argv)
{
   int n_vectors=10;
   genmatrix P(3,n_vectors);

   double theta=5*PI/180;
   double phi=20*PI/180;
   for (int n=0; n<n_vectors; n++)
   {
      double x=sin(theta)*cos(phi);
      double y=sin(theta)*sin(phi);
      double z=cos(theta);
   
      P.put(0,n,x);
      P.put(1,n,y);
      P.put(2,n,z);

      threevector n_hat;
      P.get_column(n,n_hat);
//      cout << "n_hat.n_hat = " << n_hat.dot(n_hat) << endl;

      theta += 15*PI/180;
      phi += 20*PI/180;
   }

   cout << "P = " << P << endl;

   double az,el,roll;
   cout << "Enter azimuth angle:" << endl;
   cin >> az;
   cout << "Enter elevation angle:" << endl;
   cin >> el;
   cout << "Enter roll angle:" << endl;
   cin >> roll;

   az *= PI/180;
   el *= PI/180;
   roll *= PI/180;

   rotation R;
   R=R.rotation_from_az_el_roll(az,el,roll);
   
//   double alpha=10*PI/180;
//   double beta=40*PI/180;
//   double gamma=70*PI/180;
//   genmatrix R(3,3);
//   R=rotation(alpha,beta,gamma);
   
   genmatrix Q(3,n_vectors);
   Q=R*P;
   
   cout << "R = " << R << endl;
   cout << "R*Rtrans = " << R*R.transpose() << endl;
   cout << "Q = " << Q << endl;

// Introduce rotational errors into P vectors:

   double error_amp=3.0;	// degs
   nrfunc::init_default_seed(-1000);
   for (int n=0; n<n_vectors; n++)
   {
      double alpha=error_amp*PI/180*(nrfunc::ran1()-0.5);
      double beta=error_amp*PI/180*(nrfunc::ran1()-0.5);
      double gamma=error_amp*PI/180*(nrfunc::ran1()-0.5);
      rotation Rerror(alpha,beta,gamma);
      threevector curr_p;
      P.get_column(n,curr_p);      
      P.put_column(n,Rerror*curr_p);
   }

   cout << "P with errors = " << P << endl;

   genmatrix U(n_vectors,3),W(3,3),V(3,3);
   genmatrix Ptransinv(3,n_vectors),Pinv(n_vectors,3);

   const double min_abs_singular_value=1E-6;
   P.transpose().pseudo_inverse(min_abs_singular_value,Ptransinv);
   Pinv=Ptransinv.transpose();

   cout << "P*Pinv = " << P*Pinv << endl;

//   genmatrix Rnew(3,3);
   rotation Rnew;
   Rnew=Q*Pinv;
   cout << "Rnew = " << Rnew << endl;
   cout << "Rnew-R = "  << Rnew-R << endl;

   double az_new,el_new,roll_new;
   Rnew.az_el_roll_from_rotation(az_new,el_new,roll_new);
   cout << "Recovered az = " << az_new*180/PI << endl;
   cout << "Recovered el = " << el_new*180/PI << endl;
   cout << "Recovered roll = " << roll_new*180/PI << endl;


}
