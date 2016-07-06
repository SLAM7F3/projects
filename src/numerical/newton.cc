// ==========================================================================
// Newton class may be used to iteratively solve the symmetric
// quadratic form

// 			Delta'^T A^T A Delta = N

// where vectors Delta, Delta' and N are specified for matrix A.  If A
// is a 2x2 [ 3x3 ] matrix, then 4 [9] sets of (Delta,Delta',N) triples
// must be supplied.  
// ==========================================================================
// Last modified on 2/9/06; 7/29/06; 4/5/14
// ==========================================================================

#include <iostream>
#include "math/basic_math.h"
#include "math/constant_vectors.h"
#include "numerical/newton.h"
#include "numrec/nrfuncs.h"

using std::cout;
using std::endl;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void newton::allocate_member_objects()
{
   Delta.reserve(mdim);
   Deltap.reserve(mdim);
   Eps.reserve(mdim);
   Epsp.reserve(mdim);
   N.reserve(mdim);

   M_ptr=new genmatrix(mdim,dim*(dim+1)/2);
   N_ptr=new genvector(mdim);
   A_ptr=new genmatrix(dim,dim);
   Ainv_ptr=new genmatrix(dim,dim);
   Ainvtrans_ptr=new genmatrix(dim,dim);
   best_Aptr=new genmatrix(dim,dim);
   S_ptr=new genmatrix(dim,dim);
   Sinv_ptr=new genmatrix(dim,dim);
   Bvec_ptr=new genvector(dim*dim); 
   F_ptr=new genvector(mdim);
   J_ptr=new genmatrix(mdim,dim*dim);
}		       

void newton::initialize_member_objects()
{
   B_ptr=NULL;
   T_ptr=NULL;

   for (unsigned int m=0; m<mdim; m++)
   {
      Eps.push_back(threevector(Zero_vector));
      Epsp.push_back(threevector(Zero_vector));
   }
}

newton::newton(int d,int m)
{
   dim=d;	// dim = 2 or 3 spatial dimensions
   mdim=m;	// mdim = size of Delta & Deltap vectors
   initialize_member_objects();
   allocate_member_objects();
}

newton::~newton()
{
   delete M_ptr;
   delete N_ptr;
   delete A_ptr;
   delete Ainv_ptr;
   delete Ainvtrans_ptr;
   delete best_Aptr;
   delete S_ptr;
   delete Sinv_ptr;
   delete Bvec_ptr;
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

void newton::fill_data_matrix()
{
   for (unsigned int m=0; m<mdim; m++)
   {
      threevector curr_Deltap=Deltap[m];
      threevector curr_Delta=Delta[m];

//      double dotproduct=curr_Delta.dot(curr_Deltap);
//      cout << "m = " << m << " curr_Delta = " << curr_Delta
//           << " curr_Deltap = " << curr_Deltap << endl;
//      cout << "dotproduct = " << dotproduct << endl;

      unsigned int counter=0;
      for (unsigned int i=0; i<dim; i++)
      {
         for (unsigned int j=i; j<dim; j++)
         {
            double matrix_element;
            if (i==j)
            {
               matrix_element=curr_Delta.get(j)*curr_Deltap.get(i);
            }
            else
            {
               matrix_element=curr_Delta.get(j)*curr_Deltap.get(i)+
                  curr_Delta.get(i)*curr_Deltap.get(j);
            }
            
            M_ptr->put(m,counter,matrix_element);
            counter++;
         } // loop over index j
      } // loop over index i

// Fill genvector *N_ptr with any values in STL vector N which may
// have been previously entered:

      if (m < N.size())
      {
         N_ptr->put(m,N[m]);
      }
      
   } // loop over index m labeling constraint rows

//   cout << "*M_ptr = " << *M_ptr << endl;
//   cout << "*N_ptr = " << *N_ptr << endl;
}

// ---------------------------------------------------------------------   
void newton::randomly_initialize_A()
{
   for (unsigned int i=0; i<dim; i++)
   {
      for (unsigned int j=0; j<dim; j++)
      {
         int sign=sgn(nrfunc::ran1()-0.5);
         A_ptr->put(i,j,sign*nrfunc::ran1());
      }
   }
}

// ---------------------------------------------------------------------   
void newton::transfer_B_to_Bvec()
{
   int counter=0;
   for (unsigned int i=0; i<dim; i++)
   {
      for (unsigned int j=0; j<dim; j++)
      {
         Bvec_ptr->put(counter,B_ptr->get(i,j));
         counter++;
      } // loop over index j
   } // loop over index i
//   cout << "*B_ptr = " << *B_ptr << endl;
//   cout << "*Bvec_ptr = " << *Bvec_ptr << endl;
}

void newton::transfer_Bvec_to_B()
{
   int counter=0;
   for (unsigned int i=0; i<dim; i++)
   {
      for (unsigned int j=0; j<dim; j++)
      {
         B_ptr->put(i,j,Bvec_ptr->get(counter));
         counter++;
      } // loop over index j
   } // loop over index i
}

// ---------------------------------------------------------------------   
void newton::compute_Ainvtrans_from_A()
{
   A_ptr->inverse(*Ainv_ptr);
   *Ainvtrans_ptr=Ainv_ptr->transpose();
}

void newton::compute_A_from_Ainvtrans()
{
   *Ainv_ptr=Ainvtrans_ptr->transpose();
   Ainv_ptr->inverse(*A_ptr);
}

// ==========================================================================
// Iterative refinement member functions
// ==========================================================================

// Member function compute_F evaluates the vector valued score
// function F=M*Bsq-N which should ideally equal zero.  This method
// returns the squared magnitude of vector F which is a scalar measure
// of how close the current B matrix is to a location in the dim x dim
// space where the score function vanishes.

double newton::compute_F()
{
   genvector Bsq(dim*(dim+1)/2);

   int counter=0;
   transfer_B_to_Bvec();
   for (unsigned int i=0; i<dim; i++)
   {
      for (unsigned int j=i; j<dim; j++)
      {
         double sum=0;
         for (unsigned int k=0; k<dim; k++)
         {
            sum += B_ptr->get(k,i)*B_ptr->get(k,j);
         }
         Bsq.put(counter,sum);
         counter++;
      }
   }

   *F_ptr=(*M_ptr)*Bsq-(*N_ptr);
//   cout << "M*Bsq = " << (*M_ptr)*Bsq << endl;
//   cout << "*N_ptr = " << *N_ptr << endl;
//   cout << "*F_ptr = " << *F_ptr << endl;
//   cout << "Bsq = " << Bsq << endl;
//   check_A(A_ptr);

   double score=F_ptr->sqrd_magnitude();
   return score;
}

// ---------------------------------------------------------------------
// Member function compute_F_from_T explicitly evaluates the mdim x 1
// vector F = Delta' T Delta - N where T = S or Sinv.

void newton::compute_F_from_T()
{
//   if (T_ptr==NULL)
//   {
//      cout << "Error in newton::compute_F_from_T()" << endl;
//      cout << "T_ptr = NULL !" << endl;
//   }
   
   for (unsigned int m=0; m<mdim; m++)
   {
      F_ptr->put(m,Deltap[m].dot((*T_ptr)*Delta[m])
                 -Epsp[m].dot((*T_ptr)*Eps[m])-N_ptr->get(m));
   }
   
//   cout << "M*Bsq = " << (*M_ptr)*Bsq << endl;
//   cout << "*N_ptr = " << *N_ptr << endl;
//   cout << "*F_ptr = " << *F_ptr << endl;
//   cout << "Bsq = " << Bsq << endl;
//   check_A(A_ptr);
}

// ---------------------------------------------------------------------
// Member function median_abs_Fentry returns the median of the
// absolute values of all entries within genvector *F_ptr.  This
// median can be used as a score function to minimize.

double newton::median_abs_Fentry()
{
   vector<double> abs_diff;
   for (unsigned int i=0; i<mdim; i++)
   {
      double curr_abs_diff=fabs(F_ptr->get(i));
      abs_diff.push_back(curr_abs_diff);
//      cout << "i = " << i << "| F[i] | = " << curr_abs_diff << endl;
   }
//   cout << endl;
   double median_abs_diff=mathfunc::median_value(abs_diff);
   return median_abs_diff;
}

// ---------------------------------------------------------------------   
// Member function dFdApq computes the partial derivative of score
// function F with respect to A^p_q:

double newton::dFdApq(int I,int p,int q)
{
   threevector curr_Delta=Delta[I];
   threevector curr_Deltap=Deltap[I];

   double sum=0;
   for (unsigned int j=0; j<dim; j++)
   {
      double prefactor=(curr_Delta.get(q)*curr_Deltap.get(j)+
                        curr_Delta.get(j)*curr_Deltap.get(q));
      sum += prefactor*B_ptr->get(p,j);
   }
   return sum;
}

// ---------------------------------------------------------------------
// Member function compute_Jacobian fills up the mdim x (dim)**2
// matrix *J_ptr with dF/dA^p_q partial derivatives:

void newton::compute_Jacobian()
{
   for (unsigned int I=0; I<mdim; I++)
   {
      unsigned int counter=0;
      for (unsigned int p=0; p<dim; p++)
      {
         for (unsigned int q=0; q<dim; q++)
         {
            J_ptr->put(I,counter,dFdApq(I,p,q));
            counter++;
         } // loop over index q
      } // loop over index p
   }
//   cout << "*J_ptr = " << *J_ptr << endl;
}

// ---------------------------------------------------------------------
// Member function refine_B resets genvector Bvec to Bvec+delta where
// delta=-Jinv*F.  It then transfers the contents of Bvec into the dim
// x dim matrix B.  This member function implements one iteration of
// Newton's method.

void newton::refine_B()
{
   genmatrix Jinv(dim*dim,mdim);
   const double SMALL=1E-4;
   J_ptr->pseudo_inverse(SMALL,Jinv);
//   cout << "Jinv = " << Jinv << endl << endl;
//   cout << "Jinv*J = " << Jinv * (*J_ptr) << endl << endl;
//   cout << "J*Jinv = " << (*J_ptr) * Jinv << endl << endl;

   genvector delta(dim*dim);
   delta=-Jinv*(*F_ptr);
//   cout << "delta = " << delta << endl;
   *Bvec_ptr += delta;
//   cout << "New Bvec = " << Bvec << endl;

   transfer_Bvec_to_B();
}

// ---------------------------------------------------------------------
// Diagnostic member function check_A evaluates Delta'^T A^T A Delta
// for all (Delta',Delta) pairs and for input A=*curr_Aptr.

void newton::check_A(const genmatrix* curr_Aptr)
{
   genmatrix S(dim,dim);
   S=curr_Aptr->transpose() * (*curr_Aptr);
//   cout << "S = A^T A = " << S << endl;

   for (unsigned int m=0; m<mdim; m++)
   {
      threevector curr_Delta=Delta[m];
      threevector curr_Deltap=Deltap[m];
      double quadratic_form=curr_Deltap.dot(S*curr_Delta);
      cout << "m = " << m 
//           << " Deltap[m] = " << Deltap[m] 
//           << " Delta[m] = " << Delta[m]
           << " Delta' S Delta = " << quadratic_form
           << endl;
   } // loop over index m labeling Delta,Delta' constraints
}

// ---------------------------------------------------------------------
// Member function compute_S returns A^T A within member genmatrix
// *S_ptr.

genmatrix* newton::compute_S_from_A()
{
   *S_ptr=A_ptr->transpose() * (*A_ptr);
   return S_ptr;
}

genmatrix* newton::compute_Sinv_from_Ainvtrans()
{
   *Sinv_ptr=Ainvtrans_ptr->transpose() * (*Ainvtrans_ptr);
   return Sinv_ptr;
}
// ---------------------------------------------------------------------
// Member function compute_A_from_S performs a Cholesky decomposition
// on member *S_ptr = Atrans * A.  If the decomposition is successful,
// this method sets member *A_ptr equal to A and returns A_ptr.
// Otherwise, this method returns NULL.

genmatrix* newton::compute_A_from_S()
{
   genmatrix Atrans(dim,dim);
   if (S_ptr->cholesky_decomposition(Atrans))
   {
      *A_ptr=Atrans.transpose();
      return A_ptr;
   }
   else
   {
      cout << "Cholesky decomposition of S into Atrans*A failed " << endl;
      cout << " in newton::compute_A_from_S()" << endl;
      return NULL;
   }

}
