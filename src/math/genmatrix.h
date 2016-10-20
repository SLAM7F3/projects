// ==========================================================================
// Header file for genmatrix class 
// ==========================================================================
// Last modified on 10/12/16; 10/17/16; 10/19/16; 10/20/16
// ==========================================================================

#ifndef GENMATRIX_H
#define GENMATRIX_H

#include "gmm/gmm.h"
#include "gmm/gmm_matrix.h"
#include "math/Genarray.h"

class genvector;
class rotation;
class threevector;

class genmatrix:public genarray
{

  public:

// Initialization, allocation and construction functions

   genmatrix(int m);
   genmatrix(int m,int n);
   genmatrix(const rotation& R);
   genmatrix(const tensor& T);
   genmatrix(const genmatrix& m);
   genmatrix(const genvector& V);
   ~genmatrix();

   genmatrix& operator= (const genmatrix& m);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const genmatrix& A); 

// Set and get member functions:

   genvector get_row(int m) const;
   void get_row(int m,genvector& row) const;
   void put_row(int m,const genvector& row);
   void renormalize_row(int m);

   genvector get_column(int n) const;
   void get_column(int n,genvector& column) const;
   void put_column(int n,const genvector& column);
   void renormalize_column(int n);

   void put_smaller_column(int n,const genvector& column);
   void put_smaller_column(int m_start,int n,const genvector& column);
   void put_smaller_matrix(const genmatrix& M);
   void put_smaller_matrix(int m_start,int n_start,const genmatrix& M);
   void get_smaller_matrix(genmatrix& M);
   
   double rows_dotproduct(int r1, int r2);
   double columns_dotproduct(int c1, int c2);

// Basic matrix manipulation member functions:

   void clear_matrix_values();
   genmatrix transpose() const;
   genmatrix absolute_value() const;
   void identity();
   genmatrix commutator(const genmatrix& A) const;
   genmatrix anticommutator(const genmatrix& A) const;
   double trace() const;
   genmatrix power(unsigned int n);
   genmatrix elementwise_product(const genmatrix& M);
   void elementwise_power(const genmatrix& M, double alpha);
   void hadamard_sqrt(const genmatrix& M);
   genmatrix hadamard_power(double alpha);
   void hadamard_sum(double alpha);
   void hadamard_division(const genmatrix& D);
   double sqrd_Frobenius_norm();

// Extremal element member functions:

   double max_element_value();
   double max_abs_element_value();
   double max_abs_element_value(int& m_max,int& n_max);
   double max_abs_element_difference();

// Inversion member functions:

   double compute_geometric_mean_scale() const;
   double determinant() const;
   double two_determinant() const;
   double three_determinant() const;
   bool determinant(double& d) const;
   bool determinant(double& d,double& scaleless_det) const;
   bool inverse(genmatrix& Ainv) const;
   bool two_inverse(genmatrix& Ainv) const;

// Decomposition member functions:
   
   bool sorted_singular_value_decomposition(
      genmatrix& U,genmatrix& W,genmatrix& V) const;
   bool sorted_singular_value_decomposition_w_mdim_less_than_ndim(
      genmatrix& Usorted,genmatrix& Wsorted,genmatrix& Vsorted) const;
   bool pseudo_inverse(
      double min_abs_singular_value,genmatrix& Apseudo_inverse) const;
   genmatrix pseudo_inverse();

   bool cholesky_decomposition(genmatrix& L) const;
   bool sym_eigen_decomposition(genmatrix& D,genmatrix& U) const;
   bool square_root(genmatrix& Sqrt);

   bool decompose_sym(
      threevector& lambda,double& theta,double& phi,double& chi,double& detU) 
      const;
   void reconstruct_sym(const threevector& lambda,double theta,double phi,
                        double chi,double detU);
   void reconstruct_inverse_sym(
      const threevector& lambda,double theta,double phi,
      double chi,double detU);
   rotation projection_rotation();

// Triangular decomposition member functions:

   void flipud(genmatrix& A_flipud) const;
   void reverselr(genmatrix& A_reverselr) const;
   void QR_decomposition(genmatrix& Q,genmatrix& R) const;
   void RQ_decomposition(genmatrix& R,genmatrix& Q) const;

// Linear system solution member functions:

   bool homogeneous_soln(genvector& X) const;
   bool inhomogeneous_soln(const genvector& B,genvector& X) const;
   bool solve_linear_system(const genvector& B,genvector& X) const;

// Rank deficient matrix approximation member functions:

   unsigned int rank() const;
   bool rank_approximation(unsigned int r,
                           genmatrix& Ur,genmatrix& Wr,genmatrix& Vr);
   bool rank_approximation(unsigned int r,genmatrix& reduced_rank_matrix);
   void generate_mask_matrix(double null_value,genmatrix& M);

// Matrix export member functions:

   int number_nonzero_entries(double TINY=1E-5);
   void export_to_dense_text_format(std::string output_filename);
   void export_to_dense_binary_format(std::string output_filename);
   void export_to_sparse_text_format(std::string output_filename);
   void export_to_sparse_binary_format(std::string output_filename);
   void sparse_SVD_approximation(int k_dims);

// Friend methods:

   friend genmatrix operator+ (const genmatrix& A,const genmatrix& B);
   friend genmatrix operator- (const genmatrix& A,const genmatrix& B);
   friend genmatrix operator- (const genmatrix& A);
   friend genmatrix operator* (double a,const genmatrix& A);
   friend genmatrix operator* (const genmatrix& A,double a);
   friend genmatrix operator/ (const genmatrix& A,double a);
   friend genmatrix operator* (const genmatrix& A, const genmatrix& B);

   void matrix_sum(const genmatrix& A, const genmatrix& B);
   void matrix_increment(double alpha, const genmatrix& B);
   void matrix_mult(const genmatrix& A, const genmatrix& B);
   void matrix_column_mult(const genmatrix& A, const genmatrix& B, int bcol);
   void accumulate_outerprod(const genvector& A, const genvector& B);

  protected:

   void docopy(const genmatrix& m);

  private: 

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

#endif  // math/genmatrix.h




