// ==========================================================================
// Class euler iteratively solves the system of quadratic equations

// 	       	F_9x1 = M_9x10 Esq_10x1 - N_9x1 = 0

// for Euler (quaternion) parameters (e0,e1,e2,e3) which specify an
// arbitrary rotation in 3-space.  Elements of rotation matrix R are
// placed within the 9-dimensional vector N, while matrix M involves
// integer constants.

// Note: This class is irrelevant as of 2/8/06.  Use mathfunc methods
// decompose_orthogonal_matrix and construct_orthogonal_matrix instead
// of this class !

// ==========================================================================
// Last modified on 2/7/06; 10/27/08; 1/29/12
// ==========================================================================

#include <iostream>
#include <string>
#include "math/basic_math.h"
#include "numerical/euler.h"
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "math/rotation.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void euler::allocate_member_objects()
{
   M_ptr=new genmatrix(9,10);
   N_ptr=new genvector(9);
   E_ptr=new genvector(4);
   best_Eptr=new genvector(4);
   F_ptr=new genvector(9);
   J_ptr=new genmatrix(9,4);
}		       

void euler::initialize_member_objects()
{
}

euler::euler()
{
   initialize_member_objects();
   allocate_member_objects();
}

euler::~euler()
{
   delete M_ptr;
   delete N_ptr;
   delete E_ptr;
   delete best_Eptr;
   delete F_ptr;
   delete J_ptr;
}

// ==========================================================================
// Initialization member functions
// ==========================================================================

// Member function fill_data_matrix builds the symmetric matrix M
// described in our "Euclidean upgrade from multiple affine views"
// notes dated 1/30/06 from Delta and Deltap information.  It also
// fills genvector *N_ptr with any nontrivial values previously
// entered into member STL vector N.

void euler::fill_data_matrix()
{
   M_ptr->put(0,0,1);
   M_ptr->put(0,1,1);
   M_ptr->put(0,2,-1);
   M_ptr->put(0,3,-1);

   M_ptr->put(1,6,2);
   M_ptr->put(1,7,2);

   M_ptr->put(2,5,-2);
   M_ptr->put(2,8,2);

   M_ptr->put(3,6,-2);
   M_ptr->put(3,7,2);

   M_ptr->put(4,0,1);
   M_ptr->put(4,1,-1);
   M_ptr->put(4,2,1);
   M_ptr->put(4,3,-1);
   
   M_ptr->put(5,4,2);
   M_ptr->put(5,9,2);

   M_ptr->put(6,5,2);
   M_ptr->put(6,8,2);

   M_ptr->put(7,4,-2);
   M_ptr->put(7,9,2);
   
   M_ptr->put(8,0,1);
   M_ptr->put(8,1,-1);
   M_ptr->put(8,2,-1);
   M_ptr->put(8,3,1);

//   cout << "*M_ptr = " << *M_ptr << endl;
}

// ---------------------------------------------------------------------
void euler::fill_N_vector(const rotation& R)
{
   int counter=0;
   for (int i=0; i<3; i++)
   {
      for (int j=0; j<3; j++)
      {
         N_ptr->put(counter,R.get(i,j));
         counter++;
      }
   }
//   cout << "*N_ptr = " << *N_ptr << endl;
}

// ---------------------------------------------------------------------   
void euler::initialize_E()
{
   E_ptr->put(0,1);
   E_ptr->put(1,0);
   E_ptr->put(2,0);
   E_ptr->put(3,0);

//   E_ptr->put(0,0);
//   E_ptr->put(1,1);
//   E_ptr->put(2,0);
//   E_ptr->put(3,0);

/*
   E_ptr->put(0,0.5);
   E_ptr->put(1,0.5);
   E_ptr->put(2,0.5);
   E_ptr->put(3,0.5);
*/

}

// ---------------------------------------------------------------------
// Member function compute_Jacobian fills up the 9 x 4 matrix *J_ptr:

void euler::compute_Jacobian()
{
   double e0=E_ptr->get(0);
   double e1=E_ptr->get(1);
   double e2=E_ptr->get(2);
   double e3=E_ptr->get(3);
   
   J_ptr->put(0,0,e0);
   J_ptr->put(0,1,e1);
   J_ptr->put(0,2,-e2);
   J_ptr->put(0,3,-e3);

   J_ptr->put(1,0,e3);
   J_ptr->put(1,1,e2);
   J_ptr->put(1,2,e1);
   J_ptr->put(1,3,e0);

   J_ptr->put(2,0,-e2);
   J_ptr->put(2,1,e3);
   J_ptr->put(2,2,-e0);
   J_ptr->put(2,3,e1);

   J_ptr->put(3,0,-e3);
   J_ptr->put(3,1,e2);
   J_ptr->put(3,2,e1);
   J_ptr->put(3,3,-e0);

   J_ptr->put(4,0,e0);
   J_ptr->put(4,1,-e1);
   J_ptr->put(4,2,e2);
   J_ptr->put(4,3,-e3);

   J_ptr->put(5,0,e1);
   J_ptr->put(5,1,e0);
   J_ptr->put(5,2,e3);
   J_ptr->put(5,3,e2);

   J_ptr->put(6,0,e2);
   J_ptr->put(6,1,e3);
   J_ptr->put(6,2,e0);
   J_ptr->put(6,3,e1);

   J_ptr->put(7,0,-e1);
   J_ptr->put(7,1,-e0);
   J_ptr->put(7,2,e3);
   J_ptr->put(7,3,e2);

   J_ptr->put(8,0,e0);
   J_ptr->put(8,1,-e1);
   J_ptr->put(8,2,-e2);
   J_ptr->put(8,3,e3);

//   cout << "*J_ptr = " << *J_ptr << endl;
}

// ==========================================================================
// Iterative refinement member functions
// ==========================================================================

// Member function compute_F evaluates the vector valued score
// function F=M*Bsq-N which should ideally equal zero.  This method
// returns the squared magnitude of vector F which is a scalar measure
// of how close the current B matrix is to a location in the dim x dim
// space where the score function vanishes.

double euler::compute_F()
{
   genvector Esq(10);
   double e0=E_ptr->get(0);
   double e1=E_ptr->get(1);
   double e2=E_ptr->get(2);
   double e3=E_ptr->get(3);

   Esq.put(0,sqr(e0));
   Esq.put(1,sqr(e1));
   Esq.put(2,sqr(e2));
   Esq.put(3,sqr(e3));

   Esq.put(4,e0*e1);
   Esq.put(5,e0*e2);
   Esq.put(6,e0*e3);
   Esq.put(7,e1*e2);
   Esq.put(8,e1*e3);
   Esq.put(9,e2*e3);

   *F_ptr=(*M_ptr)*Esq-(*N_ptr);
//   cout << "M*Esq = " << (*M_ptr)*Esq << endl;
//   cout << "*N_ptr = " << *N_ptr << endl;
//   cout << "*E_ptr = " << *E_ptr << endl;
//   cout << "Esq = " << Esq << endl;
//   cout << "*F_ptr = " << *F_ptr << endl;
   
   double score=F_ptr->sqrd_magnitude();
   return score;
}

// ---------------------------------------------------------------------
// Member function refine_E resets genvector *E_ptr to *E_ptr+delta
// where delta=-Jinv*F.  This member function implements one iteration
// of Euler's method.

void euler::refine_E()
{
   genmatrix Jinv(4,9);
   const double SMALL=1E-4;
   J_ptr->pseudo_inverse(SMALL,Jinv);
//   cout << "Jinv = " << Jinv << endl << endl;
//   cout << "Jinv*J = " << Jinv * (*J_ptr) << endl << endl;
//   cout << "J*Jinv = " << (*J_ptr) * Jinv << endl << endl;

   genvector delta(4);
   delta=-Jinv*(*F_ptr);
//   cout << "delta = " << delta << endl;
   *E_ptr += delta;
//   cout << "*E_ptr = " << *E_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function solve_for_E takes in a 3x3 rotation matrix R and
// iteratively solves for Euler (quaternion) parameters (e0,e1,e2,e3)
// corresponding to R.  

bool euler::solve_for_E(const rotation& R)
{
   bool valid_rotation_flag=R.rotation_sanity_check();
   if (valid_rotation_flag)
   {
      fill_data_matrix();
      fill_N_vector(R);

      initialize_E();
      int max_iters=10000;
      double minimum_score=POSITIVEINFINITY;
      for (int i=0; i<max_iters; i++)
      {
         double curr_score=compute_F();
         compute_Jacobian();
         refine_E();
         if (curr_score < minimum_score)
         {
            minimum_score=curr_score;
            *(get_best_Eptr())=*(get_Eptr());
         }
//         cout << "i = " << i << " curr_score = " << curr_score
//              << " min score = " << minimum_score << endl;
      }

      cout << "min score = " << minimum_score << endl;
      *(get_Eptr())=*(get_best_Eptr());

      cout << "E = " << *(get_Eptr()) << endl;
      cout << "|E| = " << get_Eptr()->magnitude() << endl;
   } // rotation_sanity_check conditional
   return valid_rotation_flag;
}

// ---------------------------------------------------------------------
// Member function rotation_axis_and_angle computes the rotation axis
// n_hat and rotation angle phi corresponding to Euler (quaternion)
// parameters (e0,e1,e2,e3).  

void euler::rotation_axis_and_angle(threevector& n_hat,double& phi)
{
   genvector E(4);
   E=*(get_Eptr());

   n_hat=threevector(E.get(1),E.get(2),E.get(3));
   n_hat=n_hat.unitvector();
   cout << "Rotation axis n_hat = " << n_hat << endl;

   int counter=0;
   double sin_halfphi_sum=0;
   for (int i=0; i<3; i++)
   {
      if (!nearly_equal(n_hat.get(i),0))
      {
         sin_halfphi_sum += E.get(i+1)/n_hat.get(i);
         counter++;
      }
   }
   double sin_halfphi=sin_halfphi_sum/counter;
   phi=-2*atan2(sin_halfphi,E.get(0));

   cout << "Rotation angle phi = " << phi*180/PI << endl;
}

