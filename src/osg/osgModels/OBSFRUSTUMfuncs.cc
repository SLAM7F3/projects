// ==========================================================================
// OBSFRUSTUMfuncs namespace method definitions
// ==========================================================================
// Last modified on 11/5/10; 11/7/10; 12/7/10
// ==========================================================================

#include <iostream>
#include "math/basic_math.h"
#include "math/constants.h"
#include "math/constant_vectors.h"
#include "osg/osgModels/OBSFRUSTUMfuncs.h"
#include "math/rotation.h"


using std::cout;
using std::endl;
using std::vector;

namespace OBSFRUSTUMfunc
{

// Method convert_FOVs_to_alpha_beta_angles() takes in "horizontal"
// and "vertical" field-of-view angles for an OBSFRUSTUM measured in
// radians.  The horizontal FOV lies along the v_hat direction, while
// the vertical FOV lies along the w_hat = v_hat x z_hat direction.
// This method computes opening OBSFRUSTUM angles alpha and beta in
// terms of the input field-of-view angles using Newton's method to
// solve a nonlinear expression.

// See notes entitled "Relationship between OBSFRUSTA FOV angles &
// opening angles alpha & beta" dated 5/18/09.

// OBSFRUSTUM_alpha = opening angle between velocity direction vector
// (assumed to lie in the XY plane) and OBSFRUSTUM's side projected
// into the plane defined by the observation direction vector n_hat
// (which as of 7/20/08 is assumed to lie in the -Z direction).

// OBSFRUSTUM_beta = angle between OBSFRUSTUM's 3D side(s) and n_hat.

   double prev_horiz_FOV=NEGATIVEINFINITY;
   double prev_vert_FOV=NEGATIVEINFINITY;
   double prev_alpha=NEGATIVEINFINITY;
   double prev_beta=NEGATIVEINFINITY;

   void convert_FOVs_to_alpha_beta_angles(
      double horiz_FOV,double vert_FOV,double& alpha,double& beta)
      {
//         cout << "inside OBSFRUSTUMfuncs::convert_FOVs_to_alpha_beta_angles()"
//              << endl;
//         cout << "horiz_FOV = " << horiz_FOV*180/PI
//              << " vert_FOV = " << vert_FOV*180/PI << endl;

         if (nearly_equal(horiz_FOV,prev_horiz_FOV) &&
             nearly_equal(vert_FOV,prev_vert_FOV))
         {
            alpha=prev_alpha;
            beta=prev_beta;
            return;
         }

         double cos_horiz_FOV=cos(horiz_FOV);
         double cos_vert_FOV=cos(vert_FOV);
         
// Perform brute force evaluation of F=F(A).  Initialize Newton's
// method with A value which minimizes |F(A)|:

         int n_Abins=100;
         double Astart=0;
         double Astop=1.0;
         double dA=(Astop-Astart)/n_Abins;

         double Astart_best=1.0;
         double Fmin=POSITIVEINFINITY;
         for (int n=0; n<=n_Abins; n++)
         {
            double A=Astart+n*dA;
            double F=Ffunc(A,cos_horiz_FOV,cos_vert_FOV);
//            cout << "A = " << A
//                 << " F = " << F << endl;
            if (fabs(F) < Fmin)
            {
               Fmin=fabs(F);
               Astart_best=A;
            }
         }
//         cout << "Astart = " << Astart_best << endl;

// Newton's iterative loop starts here:

         int iter=0;
         double A=Astart_best;
         double B;
         double F=1;
         while (fabs(F) > 1E-6)
         {
            B=Bfunc(A,cos_horiz_FOV);

            double numer=A*(1+cos_horiz_FOV);
            double denom=1+A+(A-1)*cos_horiz_FOV;
            double numer_prime=1+cos_horiz_FOV;
            double denom_prime=1+cos_horiz_FOV;
            double Bprime=(denom*numer_prime-numer*denom_prime)/sqr(denom);
         
            numer=B*(2-A)+A-1;
            denom=B*A-A+1;
            F=Ffunc(A,cos_horiz_FOV,cos_vert_FOV);

            numer_prime=-B+(2-A)*Bprime+1;
            denom_prime=B+A*Bprime-1;
            double Fprime=(denom*numer_prime-numer*denom_prime)/sqr(denom);
            double dA=-F/Fprime;
            A += dA;
            iter++;
//            cout << "iter = " << iter 
//                 << " A = " << A
//                 << " B = " << B
//                 << " F = " << F << endl;
         }

         if (A < 0 || A > 1)
         {
            cout << "Error in OBSFRUSTUMfunc::convert_FOVs_to_alpha_beta_angles()" << endl;
            cout << "A = " << A << endl;
            cout << "A should lie within interval [0,1]" << endl;
            exit(-1);
         }

         if (B < 0 || B > 1)
         {
            cout << "Error in OBSFRUSTUMfunc::convert_FOVs_to_alpha_beta_angles()" << endl;
            cout << "B = " << B << endl;
            cout << "B should lie within interval [0,1]" << endl;
            exit(-1);
         }

         double cos_alpha=sqrt(A);
         double cos_beta=sqrt(B);
//         cout << "cos_alpha = " << cos_alpha << endl;
//         cout << "cos_beta = " << cos_beta << endl;
         alpha=acos(cos_alpha);
         beta=acos(cos_beta);
//         cout << "alpha = " << alpha*180/PI 
//              << " beta = " << beta*180/PI << endl;

         prev_horiz_FOV=horiz_FOV;
         prev_vert_FOV=vert_FOV;
         prev_alpha=alpha;
         prev_beta=beta;
      }

// ---------------------------------------------------------------------
   double Bfunc(double A,double cos_horiz_FOV)
   {
      double numer=A*(1+cos_horiz_FOV);
      double denom=1+A+(A-1)*cos_horiz_FOV;
      double B=numer/denom;
      return B;
   }

   double Ffunc(double A,double cos_horiz_FOV,double cos_vert_FOV)
   {
      double B=Bfunc(A,cos_horiz_FOV);
      double numer=B*(2-A)+A-1;
      double denom=B*A-A+1;
      double F=numer/denom-cos_vert_FOV;
      return F;
   }
   
// ---------------------------------------------------------------------
   void convert_alpha_beta_angles_to_FOVs(
      double alpha,double beta,double& horiz_FOV,double& vert_FOV) 
     {
//         cout << "inside OBSFRUSTUMfuncs::convert_alpha_beta_angles_to_FOVs()"
//              << endl;
//         cout <<  "alpha = " << alpha*180/PI << " beta = " << beta*180/PI
//              << endl;

         double cos_alpha=cos(alpha);
         double cos_beta=cos(beta);
         double c2a=sqr(cos_alpha);
         double c2b=sqr(cos_beta);
         
         double numer=c2b*(1+c2a)-c2a;
         double denom=c2b*(1-c2a)+c2a;
         double cos_horiz_FOV=numer/denom;
         
         numer=c2b*(2-c2a)+c2a-1;
         denom=c2b*c2a-c2a+1;
         double cos_vert_FOV=numer/denom;
   
         horiz_FOV=acos(cos_horiz_FOV);
         vert_FOV=acos(cos_vert_FOV);

//         cout << "horiz_FOV = " << horiz_FOV*180/PI
//              << " vert_FOV = " << vert_FOV*180/PI << endl;
      }

// ---------------------------------------------------------------------
// Method compute_corner_rays() takes in OBSFRUSTUM extent angles
// alpha and beta (which are functions of az_extent and el_extent) as
// well as orientation angles roll (measured from nadir) and pitch
// along with velocity direction vector v_hat.  It computes the
// direction vectors of the OBSFRUSTUM's four corners and checks
// whether they all have nonzero projection along -z_hat.  If so, this
// boolean method returns true.

   bool compute_corner_rays(
      const threevector& v_hat,
      double horiz_FOV,double vert_FOV,double roll,double pitch)
      {
         double alpha,beta;
         vector<threevector> ray_corner;
         convert_FOVs_to_alpha_beta_angles(horiz_FOV,vert_FOV,alpha,beta);
         return compute_corner_rays(alpha,beta,roll,pitch,v_hat,ray_corner);
      }

   bool compute_corner_rays(
      double alpha,double beta,double roll,double pitch,
      const threevector& v_hat,vector<threevector>& ray_corner)
      {
//         cout << "inside OBSFRUSTUMfunc::compute_ray_corners(alpha,beta,roll,pitch)" 
//              << endl;
//         cout << "alpha = " << alpha*180/PI 
//              << " beta = " << beta*180/PI << endl;
//         cout << "roll = " << roll*180/PI << " pitch = " << pitch*180/PI
//              << endl;

         threevector w_hat(v_hat.cross(z_hat));
//         cout << "v_hat = " << v_hat << " w_hat = " << w_hat << endl;

         ray_corner.clear();
         ray_corner.push_back(
            sin(beta)*cos(alpha)*v_hat
            -sin(beta)*sin(alpha)*w_hat
            -cos(beta)*z_hat);

         ray_corner.push_back(
            sin(beta)*cos(alpha)*v_hat
            +sin(beta)*sin(alpha)*w_hat
            -cos(beta)*z_hat);

         ray_corner.push_back(
            -sin(beta)*cos(alpha)*v_hat
            +sin(beta)*sin(alpha)*w_hat
            -cos(beta)*z_hat);

         ray_corner.push_back(
            -sin(beta)*cos(alpha)*v_hat
            -sin(beta)*sin(alpha)*w_hat
            -cos(beta)*z_hat);

// Roll corner rays about v_hat away from nadir direction.  Then pitch
// rays about w_hat:

         rotation Rroll,Rpitch;
         Rroll.rotation_taking_pqr_to_uvw(
            v_hat,z_hat,w_hat,
            v_hat,cos(roll)*z_hat+sin(roll)*w_hat,
            -sin(roll)*z_hat+cos(roll)*w_hat);
         Rpitch.rotation_taking_pqr_to_uvw(
            v_hat,z_hat,w_hat,
            cos(pitch)*v_hat+sin(pitch)*z_hat,
            -sin(pitch)*v_hat+cos(pitch)*z_hat,
            w_hat);

//         cout << "Rroll = " << Rroll << endl;
//         cout << "Rpitch = " << Rpitch << endl;

         for (int c=0; c<int(ray_corner.size()); c++)
         {
            ray_corner[c]=Rpitch*Rroll*ray_corner[c];
         }

// Check whether all corner rays point downwards (e.g. aerial sensor
// looking at ground):

         bool all_downward_rays_flag=true;
         const double SMALL_NEG=-0.0000001;
         for (int c=0; c<4; c++)
         {
            if (ray_corner[c].get(2) > SMALL_NEG) 
               all_downward_rays_flag=false;
//            cout << "c = " << c << " ray_corner[c] = " << ray_corner[c]
//                 << endl;
         }
//         cout << "all_downward_rays_flag = " << all_downward_rays_flag 
//              << endl;
         return all_downward_rays_flag;
      }

} // OBSFRUSTUMfunc namespace

