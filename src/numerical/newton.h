// ==========================================================================
// Header file for newton class 
// ==========================================================================
// Last modified on 2/9/06; 4/5/14
// ==========================================================================

#ifndef NEWTON_H
#define NEWTON_H

#include <vector>
#include "math/threevector.h"

class newton
{

  public:

// Initialization, constructor and destructor functions:

   newton(int d,int m);
   ~newton();

//   newton& operator= (const newton& p);
//   friend std::ostream& operator<< 
//      (std::ostream& outstream,const newton& p);

// Set & get member functions:

   void set_Bptr_to_Aptr();
   void set_Bptr_to_Ainvtransptr();
   void set_Tptr_to_Sptr();
   void set_Tptr_to_Sinvptr();
   
   int get_dim() const;
   std::vector<threevector>& get_Delta();
   std::vector<threevector>& get_Deltap();
   std::vector<threevector>& get_Eps();
   std::vector<threevector>& get_Epsp();
   std::vector<double>& get_N();
   genmatrix* get_Aptr();
   genmatrix* get_Ainvtransptr();
   genmatrix* get_best_Aptr();
   genmatrix* get_Sptr();
   genmatrix* get_Sinvptr();

// Initialization member functions:

   void fill_data_matrix();
   void randomly_initialize_A();
   void transfer_B_to_Bvec();
   void transfer_Bvec_to_B();
   void compute_Ainvtrans_from_A();
   void compute_A_from_Ainvtrans();

// Iterative refinement member functions:

   double compute_F();
   void compute_F_from_T();
   double median_abs_Fentry();
   double dFdApq(int I,int p,int q);
   void compute_Jacobian();
   void refine_B();
   void check_A(const genmatrix* curr_Aptr);
   genmatrix* compute_S_from_A();
   genmatrix* compute_Sinv_from_Ainvtrans();
   genmatrix* compute_A_from_S();

  private: 

   unsigned int dim,mdim;

   std::vector<threevector> Delta,Deltap,Eps,Epsp;
   std::vector<double> N;
   genmatrix* M_ptr;
   genvector* N_ptr;
   genvector* F_ptr;
   genmatrix* J_ptr;   
   genmatrix *A_ptr,*Ainv_ptr,*Ainvtrans_ptr,*best_Aptr;
   genmatrix *S_ptr,*Sinv_ptr;

// In order to easily perform dual camera motion and target
// reconstruction computations, we introduce pointers B_ptr and T_ptr
// which respectively point to A_ptr [Ainvtrans_ptr] and S_ptr
// [Sinv_ptr] for camera [target] computations:

   genmatrix* B_ptr; // B = A [Ainvtrans] for camera [target]
   genmatrix* T_ptr; // T = S [Sinvtrans] for camera [target] 
   genvector* Bvec_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline std::vector<threevector>& newton::get_Delta()
{
   return Delta;
}

inline std::vector<threevector>& newton::get_Deltap()
{
   return Deltap;
}

inline std::vector<threevector>& newton::get_Eps()
{
   return Eps;
}

inline std::vector<threevector>& newton::get_Epsp()
{
   return Epsp;
}

inline int newton::get_dim() const
{
   return dim;
}

inline std::vector<double>& newton::get_N()
{
   return N;
}

inline genmatrix* newton::get_Aptr()
{
   return A_ptr;
}

inline genmatrix* newton::get_Ainvtransptr()
{
   return Ainvtrans_ptr;
}

inline genmatrix* newton::get_best_Aptr()
{
   return best_Aptr;
}

inline void newton::set_Bptr_to_Aptr()
{
   B_ptr=A_ptr;
}

inline void newton::set_Bptr_to_Ainvtransptr()
{
   B_ptr=Ainvtrans_ptr;
}

inline void newton::set_Tptr_to_Sptr()
{
   T_ptr=S_ptr;
}

inline void newton::set_Tptr_to_Sinvptr()
{
   T_ptr=Sinv_ptr;
}

inline genmatrix* newton::get_Sptr()
{
   return S_ptr;
}

inline genmatrix* newton::get_Sinvptr()
{
   return Sinv_ptr;
}


#endif  // numerical/newton.h





