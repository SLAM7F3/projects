// ==========================================================================
// Projective class member functions
// ==========================================================================
// Last modified on 2/20/06; 12/15/06; 3/23/12; 4/4/14
// ==========================================================================

#include "math/genmatrix.h"
#include "math/genvector.h"
#include "geometry/projective.h"

using std::cout;
using std::cin;
using std::endl;
using std::ostream;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void projective::allocate_member_objects()
{
   M_ptr=new genmatrix(4,4);
   P_ptr=new genmatrix(npoints,4);
   Q_ptr=new genmatrix(npoints,4);
}

void projective::initialize_member_objects()
{
}		       

projective::projective(int n_pnts)
{
   npoints=n_pnts;
   allocate_member_objects();
   initialize_member_objects();
}

projective::~projective()
{
   delete M_ptr;
   delete P_ptr;
   delete Q_ptr;
   M_ptr=NULL;
   P_ptr=NULL;
   Q_ptr=NULL;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const projective& proj)
{
   outstream << endl;
   outstream << "Projective transformation M = " << *(proj.M_ptr) << endl;
   return outstream;
}

// ==========================================================================
// 4x4 projective matrix determination:
// ==========================================================================

void projective::parse_projective_inputs(
   const vector<threevector>& XYZ,const vector<threevector>& UVW)
{
   if (XYZ.size() != npoints || UVW.size() != npoints)
   {
      cout << "Error in projective::parse_projective_inputs()"
           << endl;
      cout << "npoints = " << npoints << endl;
      cout << "XYZ.size() = " << XYZ.size() 
           << " UVW.size() = " << UVW.size() << endl;
      exit(-1);
   }

   for (unsigned int n=0; n<npoints; n++)
   {
      P_ptr->put(n,0,XYZ[n].get(0));
      P_ptr->put(n,1,XYZ[n].get(1));
      P_ptr->put(n,2,XYZ[n].get(2));
      P_ptr->put(n,3,1);
      
      Q_ptr->put(n,0,UVW[n].get(0));
      Q_ptr->put(n,1,UVW[n].get(1));
      Q_ptr->put(n,2,UVW[n].get(2));
      Q_ptr->put(n,3,1);
   } // loop over index n labeling input points
}

// ---------------------------------------------------------------------
// Method compute_projective_matrix 
   
void projective::compute_projective_matrix()
{
   genmatrix U(npoints,4),W(4,4),V(4,4);
   P_ptr->sorted_singular_value_decomposition(U,W,V);

   genmatrix Ppseudo_inv(4,npoints);

   const double SMALL=1E-5;
   P_ptr->pseudo_inverse(SMALL,Ppseudo_inv);
//      cout << "Ppseudo_inv = " << Ppseudo_inv << endl;
//      cout << "Ppseudo_inv*P = " << Ppseudo_inv * (*P_ptr) << endl;

   if (npoints==4)
   {
      double det;
      if (P_ptr->determinant(det))
      {
         genmatrix Pinv(4,4);
         P_ptr->inverse(Pinv);
         cout << "det = " << det << endl;
         cout << "Pinv = " << Pinv << endl;
         cout << "P*Pinv = " << (*P_ptr) * Pinv << endl;
         cout << "Pinv*P = " << Pinv * (*P_ptr) << endl;
      }
      else
      {
         cout << "Cannot invert P" << endl;
      }
   }
      
   genmatrix Mtrans(4,4);
   Mtrans=Ppseudo_inv*(*Q_ptr);
   *M_ptr=Mtrans.transpose();
//      cout << "Projective transformation *M_ptr = " << *M_ptr << endl;
}

// ---------------------------------------------------------------------
// Method check_projective_matrix reads in the original XYZ-UVW
// information.  It transforms the XYZ points into UVW points using
// the 4x4 projective matrix *M_ptr.  This method's direct comparison
// of the projected UVW values with the measured ones provides some
// indication of the projective transformation's quality.

threevector projective::transform_XYZ_to_UVW(const threevector& XYZ)
{
   fourvector p(XYZ,1);
   fourvector q=(*M_ptr) * p;

   double denom=q.get(3);
   if (nearly_equal(denom,0))
   {
      cout << "Error in projective::transform_XYZ_to_UVW()" << endl;
      cout << "p = " << p << endl;
      cout << "q = " << q << endl;
      exit(-1);
   }
   return threevector(q.get(0)/denom,q.get(1)/denom,q.get(2)/denom);
}

void projective::transform_XYZs_to_UVWs(
   const vector<threevector>& XYZ,vector<threevector>& UVW)
{
   for (unsigned int n=0; n<XYZ.size(); n++)
   {
      fourvector p(XYZ[n],1);
      fourvector q=(*M_ptr) * p;
      double denom=q.get(3);
      if (nearly_equal(denom,0))
      {
         cout << "Error in projective::transform_XYZ_to_UVW()" << endl;
         cout << "p = " << p << endl;
         cout << "q = " << q << endl;
         exit(-1);
      }
      else
      {
         UVW.push_back(threevector(q.get(0),q.get(1),q.get(2)));
      }
   } // loop over index n labeling 3D points
}

void projective::transform_XYZPs_to_UVWPs(
   const vector<fourvector>& XYZP,vector<fourvector>& UVWP,
   double null_value)
{
   vector<threevector> XYZ,UVW;
   for (unsigned int n=0; n<XYZP.size(); n++)
   {
      fourvector curr_XYZP=XYZP[n];
      XYZ.push_back(threevector(curr_XYZP.get(0),curr_XYZP.get(1),
                                curr_XYZP.get(2)));
   }
   transform_XYZs_to_UVWs(XYZ,UVW);
   for (unsigned int n=0; n<XYZP.size(); n++)
   {
      fourvector curr_XYZP=XYZP[n];
      double curr_p=curr_XYZP.get(3);
      if (!nearly_equal(curr_p,null_value,1))
      {
         UVWP.push_back(fourvector(UVW[n],curr_XYZP.get(3)));
//         cout << "n = "  << n << " p = " << UVWP.back().get(3) << endl;
      }
   }
}

// ---------------------------------------------------------------------
// Method check_projective_matrix reads in the original XYZ-UVW
// information.  It transforms the XYZ points into UVW points using
// the 4x4 projective matrix *M_ptr.  This method's direct comparison
// of the projected UVW values with the measured ones provides some
// indication of the projective transformation's quality.

double projective::check_projective_matrix(
   const vector<threevector>& XYZ,const vector<threevector>& UVW)
{
   double chisq=0;
   for (unsigned int n=0; n<npoints; n++)
   {
      threevector q=transform_XYZ_to_UVW(XYZ[n]);
      double d_chisq = (q-UVW[n]).sqrd_magnitude();
      chisq += d_chisq;

      threevector curr_XYZ(XYZ[n]);
      threevector curr_UVW(UVW[n]);
      cout << "n = " << n 
           << " X = " << curr_XYZ.get(0) 
           << " Y = " << curr_XYZ.get(1)
           << " Z = " << curr_XYZ.get(2) << endl;
      cout << "Input U = " << curr_UVW.get(0) 
           << " projected U = " << q.get(0)
           << " input V = " << curr_UVW.get(1) 
           << " projected V = " << q.get(1) 
           << " input W = " << curr_UVW.get(2) 
           << " projected W = " << q.get(2) 
           << " sqrt(d_chisq) = " << sqrt(d_chisq) 
           << endl << endl;
   } // loop over index n labeling tiepoints
   double RMS_residual=sqrt(chisq/npoints);

   cout << endl;
//   cout << "===============================================" << endl;
   cout << "RMS residual between measured and transformed XYZ points = " 
        << RMS_residual << endl;
//   cout << "===============================================" << endl;
   cout << endl;
   return RMS_residual;
}
