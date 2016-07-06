// ==========================================================================
// Kalman class member function definitions
// ==========================================================================
// Last modified on 3/27/07
// ==========================================================================

#include "track/kalman.h"

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void kalman::allocate_member_objects()
{
   X_ptr=new genvector(ndim);
   Z_ptr=new genvector(mdim);
   Zsigma_ptr=new genvector(mdim);
   V_ptr=new genvector(mdim);
   Phi_ptr=new genmatrix(ndim,ndim);
   H_ptr=new genmatrix(mdim,ndim);
   Q_ptr=new genmatrix(ndim,ndim);
   R_ptr=new genmatrix(mdim,mdim);
   P_ptr=new genmatrix(ndim,ndim);
   K_ptr=new genmatrix(ndim,mdim);

   tmp_ptr=new genmatrix(mdim,mdim);
   inverse_tmp_ptr=new genmatrix(mdim,mdim);
}		       

void kalman::initialize_member_objects()
{
}

kalman::kalman(int n,int m)
{
   ndim=n;
   mdim=m;

   allocate_member_objects();
   initialize_member_objects();
}

// Copy constructor:

kalman::kalman(const kalman& k)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(k);
}

kalman::~kalman()
{
   delete X_ptr;
   delete Z_ptr;
   delete Zsigma_ptr;
   delete V_ptr;
   delete Phi_ptr;
   delete H_ptr;
   delete Q_ptr;
   delete R_ptr;
   delete P_ptr;
   delete K_ptr;

   delete tmp_ptr;
   delete inverse_tmp_ptr;
}

// ---------------------------------------------------------------------
void kalman::docopy(const kalman& k)
{
}

// Overload = operator:

kalman& kalman::operator= (const kalman& k)
{
   if (this==&k) return *this;
   docopy(k);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const kalman& k)
{
   outstream << endl;

   return(outstream);
}

// =====================================================================
// Kalman filter algorithm steps
// =====================================================================

// Member function compute_Kalman_gain performs the first step within
// the update cycle

void kalman::compute_Kalman_gain()
{
   *tmp_ptr=(*H_ptr) * (*P_ptr) * H_ptr->transpose() + (*R_ptr);
   tmp_ptr->inverse(*inverse_tmp_ptr);
   (*K_ptr)=(*P_ptr) * H_ptr->transpose() * (*inverse_tmp_ptr);
}

// Member function update_estimate 

void kalman::update_estimate()
{
//   Z_ptr->put(0,x_true.back());
//   Z_ptr->put(1,v_true.back());
//   V_ptr->put(0,Zsigma_ptr->get(0)*nrfunc::gasdev());
//   V_ptr->put(1,Zsigma_ptr->get(1)*nrfunc::gasdev());
//   (*Z_ptr) = (*Z_ptr) + (*V_ptr);

   (*X_ptr) += (*K_ptr) * ( (*Z_ptr) - (*H_ptr) * (*X_ptr) );
}
