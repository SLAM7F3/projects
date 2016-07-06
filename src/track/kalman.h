// ==========================================================================
// Header file for kalman class
// ==========================================================================
// Last modified on 3/27/07
// ==========================================================================

#ifndef KALMAN_H
#define KALMAN_H

#include <iostream>
#include "math/genmatrix.h"
#include "math/genvector.h"

class kalman
{

  public:

// Initialization, constructor and destructor functions:

   kalman(int n,int m);
   kalman(const kalman& k);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~kalman();
   kalman& operator= (const kalman& k);
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const kalman& k);

// Set and get member functions:

   int get_ndim() const;
   int get_mdim() const;

   genvector* get_X_ptr();
   genvector* get_Z_ptr();
   genvector* get_Zsigma_ptr();
   genvector* get_V_ptr();
   genmatrix* get_Phi_ptr();
   genmatrix* get_H_ptr();
   genmatrix* get_Q_ptr();
   genmatrix* get_R_ptr();
   genmatrix* get_P_ptr();
   genmatrix* get_K_ptr();

// Kalman filter algorithm steps

   void compute_Kalman_gain();
   void update_estimate();

  private: 

   int ndim;	// Dimension of state vector
   int mdim;	// Dimension of measurement vector

   genvector *X_ptr,*Z_ptr,*Zsigma_ptr,*V_ptr;
   genmatrix *Phi_ptr,*H_ptr,*Q_ptr,*R_ptr,*P_ptr,*K_ptr;
   genmatrix *tmp_ptr,*inverse_tmp_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const kalman& p);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int kalman::get_ndim() const
{
   return ndim;
}

inline int kalman::get_mdim() const
{
   return mdim;
}

inline genvector* kalman::get_X_ptr()
{
   return X_ptr;
}

inline genvector* kalman::get_Z_ptr()
{
   return Z_ptr;
}

inline genvector* kalman::get_Zsigma_ptr()
{
   return Zsigma_ptr;
}

inline genvector* kalman::get_V_ptr()
{
   return V_ptr;
}

inline genmatrix* kalman::get_Phi_ptr()
{
   return Phi_ptr;
}

inline genmatrix* kalman::get_H_ptr()
{
   return H_ptr;
}

inline genmatrix* kalman::get_Q_ptr()
{
   return Q_ptr;
}

inline genmatrix* kalman::get_R_ptr()
{
   return R_ptr;
}

inline genmatrix* kalman::get_P_ptr()
{
   return P_ptr;
}

inline genmatrix* kalman::get_K_ptr()
{
   return K_ptr;
}

#endif  // kalman.h



