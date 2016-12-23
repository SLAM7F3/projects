// ==========================================================================
// Header file for genfloatmatrix class 
// ==========================================================================
// Last modified on 10/20/16; 11/28/16; 12/4/16; 12/23/16
// ==========================================================================

#ifndef GENFLOATMATRIX_H
#define GENFLOATMATRIX_H

#include "gmm/gmm.h"
#include "gmm/gmm_matrix.h"
#include "math/Genarray.h"

class genvector;
class rotation;
class threevector;

class genfloatmatrix:public genfloatarray
{

  public:

// Initialization, allocation and construction functions

   genfloatmatrix(int m);
   genfloatmatrix(int m,int n);
   genfloatmatrix(const rotation& R);
   genfloatmatrix(const tensor& T);
   genfloatmatrix(const genfloatmatrix& m);
   genfloatmatrix(const genvector& V);
   ~genfloatmatrix();

   genfloatmatrix& operator= (const genfloatmatrix& m);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const genfloatmatrix& A); 

// Set and get member functions:

   genvector get_row(int m) const;
   void get_row(int m, genvector& row) const;
   void put_row(int m, const genvector& row);
   void copy_row(int m, const genfloatmatrix& M);
   void renormalize_row(int m);

   genvector get_column(int n) const;
   void get_column(int n,genvector& column) const;
   void put_column(int n,const genvector& column);
   void renormalize_column(int n);

   void put_smaller_column(int n,const genvector& column);
   void put_smaller_column(int m_start,int n,const genvector& column);
   void put_smaller_matrix(const genfloatmatrix& M);
   void put_smaller_matrix(int m_start,int n_start,const genfloatmatrix& M);
   void get_smaller_matrix(genfloatmatrix& M);
   
   float rows_dotproduct(int r1, int r2);
   float columns_dotproduct(int c1, int c2);

// Basic matrix manipulation member functions:

   void clear_matrix_values();
   genfloatmatrix transpose() const;
   genfloatmatrix absolute_value() const;
   void identity();
   genfloatmatrix commutator(const genfloatmatrix& A) const;
   genfloatmatrix anticommutator(const genfloatmatrix& A) const;
   float trace() const;
   genfloatmatrix power(unsigned int n);
   genfloatmatrix elementwise_product(const genfloatmatrix& M);
   void elementwise_power(const genfloatmatrix& M, float alpha);
   void hadamard_sqrt(const genfloatmatrix& M);
   genfloatmatrix hadamard_power(float alpha);
   void hadamard_sum(float alpha);
   void hadamard_division(const genfloatmatrix& D);
   float sqrd_Frobenius_norm();

// Extremal element member functions:

   float max_element_value();
   float max_abs_element_value();
   float max_abs_element_value(int& m_max,int& n_max);
   float max_abs_element_difference();

// Inversion member functions:

   float compute_geometric_mean_scale() const;
   float determinant() const;
   float two_determinant() const;
   float three_determinant() const;
   bool determinant(float& d) const;
   bool determinant(float& d,float& scaleless_det) const;
   bool inverse(genfloatmatrix& Ainv) const;
   bool two_inverse(genfloatmatrix& Ainv) const;

// Decomposition member functions:
   
   bool sorted_singular_value_decomposition(
      genfloatmatrix& U,genfloatmatrix& W,genfloatmatrix& V) const;
   bool sorted_singular_value_decomposition_w_mdim_less_than_ndim(
      genfloatmatrix& Usorted,genfloatmatrix& Wsorted,genfloatmatrix& Vsorted) const;
   bool pseudo_inverse(
      float min_abs_singular_value,genfloatmatrix& Apseudo_inverse) const;
   genfloatmatrix pseudo_inverse();

   bool cholesky_decomposition(genfloatmatrix& L) const;
   bool sym_eigen_decomposition(genfloatmatrix& D,genfloatmatrix& U) const;
   bool square_root(genfloatmatrix& Sqrt);

   bool decompose_sym(
      threevector& lambda,float& theta,float& phi,float& chi,float& detU) 
      const;
   void reconstruct_sym(const threevector& lambda,float theta,float phi,
                        float chi,float detU);
   void reconstruct_inverse_sym(
      const threevector& lambda,float theta,float phi,
      float chi,float detU);
   rotation projection_rotation();

// Triangular decomposition member functions:

   void flipud(genfloatmatrix& A_flipud) const;
   void reverselr(genfloatmatrix& A_reverselr) const;
   void QR_decomposition(genfloatmatrix& Q,genfloatmatrix& R) const;
   void RQ_decomposition(genfloatmatrix& R,genfloatmatrix& Q) const;

// Linear system solution member functions:

   bool homogeneous_soln(genvector& X) const;
   bool inhomogeneous_soln(const genvector& B,genvector& X) const;
   bool solve_linear_system(const genvector& B,genvector& X) const;

// Rank deficient matrix approximation member functions:

   unsigned int rank() const;
   bool rank_approximation(
      unsigned int r,
      genfloatmatrix& Ur,genfloatmatrix& Wr,genfloatmatrix& Vr);
   bool rank_approximation(unsigned int r,genfloatmatrix& reduced_rank_matrix);
   void generate_mask_matrix(float null_value,genfloatmatrix& M);

// Matrix export member functions:

   int number_nonzero_entries(float TINY=1E-5);
   void export_to_dense_text_format(std::string output_filename);
   void export_to_dense_binary_format(std::string output_filename);
   void export_to_sparse_text_format(std::string output_filename);
   void export_to_sparse_binary_format(std::string output_filename);
   void sparse_SVD_approximation(int k_dims);

// Friend methods:

   friend genfloatmatrix operator+ (
      const genfloatmatrix& A,const genfloatmatrix& B);
   friend genfloatmatrix operator- (
      const genfloatmatrix& A,const genfloatmatrix& B);
   friend genfloatmatrix operator- (const genfloatmatrix& A);
   friend genfloatmatrix operator* (float a,const genfloatmatrix& A);
   friend genfloatmatrix operator* (const genfloatmatrix& A,float a);
   friend genfloatmatrix operator/ (const genfloatmatrix& A,float a);
   friend genfloatmatrix operator* (
      const genfloatmatrix& A, const genfloatmatrix& B);

   void matrix_sum(const genfloatmatrix& A, const genfloatmatrix& B);
   void matrix_increment(float alpha, const genfloatmatrix& B);
   void matrix_transpose(const genfloatmatrix& A);
   void matrix_mult(const genfloatmatrix& A, const genfloatmatrix& B);
   void matrix_column_mult(
      const genfloatmatrix& A, const genfloatmatrix& B, int bcol);
   void matrix_column_mult_sum(
      const genfloatmatrix& A, const genfloatmatrix& B, 
      const genvector& V, int bcol);
   void accumulate_outerprod(const genvector& A, const genvector& B);

  protected:

   void docopy(const genfloatmatrix& m);

  private: 

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif  // math/genfloatmatrix.h




