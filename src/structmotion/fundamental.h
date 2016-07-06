// =========================================================================
// Header file for fundamental matrix class
// =========================================================================
// Last modified on 5/23/13; 6/16/13; 7/2/13; 4/5/14
// =========================================================================

#ifndef FUNDAMENTAL_H
#define FUNDAMENTAL_H

#include <iostream>
#include <vector>

#include "math/fourvector.h"
#include "math/threevector.h"
#include "math/twovector.h"

class genmatrix;
class genvector;
class homography;
class rotation;

class fundamental
{

  public:

   fundamental();
   ~fundamental();
   friend std::ostream& operator<< 
      (std::ostream& outstream,fundamental& f);

// Set & get member functions:

   genmatrix* get_A_ptr();
   const genmatrix* get_A_ptr() const;
   void set_F_values(genmatrix& Finput);
   genmatrix* get_F_ptr();
   const genmatrix* get_F_ptr() const;
   genmatrix* get_P_ptr();
   const genmatrix* get_P_ptr() const;
   genmatrix* get_Pprime_ptr();
   const genmatrix* get_Pprime_ptr() const;

   void set_Fbest(genmatrix* F_ptr);
   genmatrix* get_Fbest_ptr();

   std::vector<genmatrix*> get_F_ptrs();
   void set_E_ptr(genmatrix* E_ptr);
   genmatrix* get_E_ptr();
   const genmatrix* get_E_ptr() const;
   std::vector<twovector>& get_XY_sorted();
   std::vector<twovector>& get_UV_sorted();

// Least-squares determination of fundamental matrix member functions:

   void parse_fundamental_inputs(
      const std::vector<threevector>& XY,const std::vector<threevector>& UV);
   void parse_fundamental_inputs(
      const std::vector<threevector>& XY,const std::vector<threevector>& UV,
      unsigned int n_inputs);
   void parse_fundamental_inputs(
      const std::vector<twovector>& XY,const std::vector<twovector>& UV);
   void parse_fundamental_inputs(
      const std::vector<twovector>& XY,const std::vector<twovector>& UV,
      unsigned int n_inputs);
   bool parse_fundamental_inputs(
      const std::vector<double>& X_init,const std::vector<double>& Y_init,
      const std::vector<double>& U_init,const std::vector<double>& V_init,
      unsigned int n_inputs);

   bool Hartley_normalize_homogenous_vectors(
      const std::vector<double>& X,const std::vector<double>& Y,
      unsigned int n_inputs,
      std::vector<double>& X_ren,std::vector<double>& Y_ren,genmatrix* T_ptr);
   std::vector<twovector> Hartley_normalize_homogenous_vectors(
      const std::vector<twovector>& XY,unsigned int n_inputs,genmatrix* T_ptr);

   bool compute_fundamental_matrix(bool print_flag=false);
   bool check_fundamental_matrix(
      const std::vector<twovector>& XY,const std::vector<twovector>& UV,
      bool print_flag=false);
   bool check_fundamental_matrix(
      const std::vector<twovector>& XY,const std::vector<twovector>& UV,
      unsigned int n_inputs,bool print_flag=false);

// 7-point algorithm member functions:

   int seven_point_algorithm(bool ignore_triple_roots_flag=false);
   void initialize_Ninverse_matrix();

// Fundamental matrix linear algebra member functions:

   threevector get_null_vector() const;
   threevector get_null_vector(genmatrix* f_ptr) const;
   double scalar_product(const twovector& xy,const twovector& uv);

// Iterative determination of fundamental matrix member functions:

   genvector* solve_for_fundamental(const threevector& epipole);
   genmatrix* generate_bigE_matrix(const threevector& e_hat) const;

// Epipole manipulation member functions:

   threevector get_epipole_XY() const;
   threevector get_epipole_UV() const;
   homography* map_epipole_to_infinity(double Umax,const threevector& e);
   homography* compute_matching_UV_homography(homography* H_XY_ptr);
   
   double projection_matrix_rows_determinant(
      int p,int q,int r,int s,const genmatrix& P_UV,const genmatrix& P_XY);
   void compute_from_projection_matrices(
      const genmatrix& P_UV,const genmatrix& P_XY);
   void renormalize_F_entries();
   void renormalize_Fbest_entries();

// Projection matrix member functions:

   genmatrix* compute_trivial_projection_matrix();
   genmatrix* compute_nontrivial_projection_matrix();

// Essential matrix member functions:

   genmatrix* generate_essential_matrix(
      const genmatrix* K_xy_ptr,const genmatrix* K_uv_ptr);
   bool correct_essential_matrix_singular_values();
   bool compute_four_relative_projection_matrix_candidates(
      std::vector<genmatrix*> projection_matrix_ptrs);
   void Horns_decomposition(
      double sgn_E,double sgn_b,rotation& R,threevector& b);

// Triangulation member functions:

   void correct_tiepoint_coordinates(
      const twovector& curr_XY,const twovector& curr_UV,
      twovector& corrected_XY,twovector& corrected_UV);
   double g(
      double t,double a,double b,double c,double d,double f,double fprime) 
      const;
   std::vector<double> find_real_g_poly_roots(
      double a,double b,double c,double d,double f,double fprime) const;
   double minimize_s_cost_function(
      double a,double b,double c,double d,double f,double fprime,
      std::vector<double>& g_roots) const;
   double s_cost_function(
      double t,double a,double b,double c,double d,double f,double fprime) 
      const;
   void find_corrected_correspondences(
      double t_min,double a,double b,double c,double d,double f,double fprime,
      threevector& xhat,threevector& xprime_hat) const;

   threevector triangulate(
      const twovector& corrected_UV,const twovector& corrected_XY);
   threevector triangulate_noisy_tiepoints(
      const twovector& curr_XY,const twovector& curr_UV);

   double reprojection_error(
      const twovector& curr_XY,const twovector& curr_UV,
      const twovector& corrected_XY,const twovector& corrected_UV);
   double reprojection_error(
      const twovector& curr_XY,const twovector& curr_UV);
   double sampson_error(
      const twovector& curr_XY,const twovector& curr_UV);

// Import/export member functions

   void export_best_matrix(int i,int j,std::string output_filename);
   void import_matrix(std::string input_filename);

  private:

// nx9 matrix *A_ptr holds (X,Y) and (U,V) tiepoint information:
// nx9 matrix *B_ptr holds (U,V) and (X,Y) tiepoint information:

   genmatrix *A_ptr,*B_ptr;

// 3x3 matrices T_XY_ptr and T_UV_ptr hold Hartley normalization
// translations & scalings:

   genmatrix *T_XY_ptr,*T_UV_ptr;

// 3x3 matrix *F_ptr holds the fundamental matrix

   genmatrix* F_ptr;
   genmatrix* Fbest_ptr;
   std::vector<genmatrix*> F_ptrs;

// 3x4 canonical projection matrices corresponding to *F_ptr:

   genmatrix* P_ptr;
   genmatrix* Pprime_ptr;

// 3x3 matrix *E_ptr holds the essential matrix

   genmatrix* E_ptr;

// 4x4 matrix *Ninverse_ptr holds matrix needed to recover cubic
// polynomial coeffs for 7-point algorithm:

   genmatrix* Ninverse_ptr;

// STL vectors to hold input XY and output UV twovectors sorted
// according to homography projection error:

   std::vector<twovector> XY_sorted,UV_sorted;

   void allocate_member_objects();
   void initialize_member_objects();

   genmatrix* decompose_fundamental_as_M_times_eUV();
   genmatrix* decompose_fundamental_as_eXY_times_M();
   double compute_linear_combo_det(
      double alpha,const genmatrix& F1_mat,const genmatrix& F2_mat) const;
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline genmatrix* fundamental::get_A_ptr()
{
   return A_ptr;
}

inline const genmatrix* fundamental::get_A_ptr() const
{
   return A_ptr;
}


inline genmatrix* fundamental::get_F_ptr()
{
   return F_ptr;
}

inline const genmatrix* fundamental::get_F_ptr() const
{
   return F_ptr;
}

inline std::vector<genmatrix*> fundamental::get_F_ptrs()
{
   return F_ptrs;
}

inline genmatrix* fundamental::get_E_ptr()
{
   return E_ptr;
}

inline const genmatrix* fundamental::get_E_ptr() const
{
   return E_ptr;
}

inline std::vector<twovector>& fundamental::get_XY_sorted()
{
   return XY_sorted;
}

inline std::vector<twovector>& fundamental::get_UV_sorted()
{
   return UV_sorted;
}

inline void fundamental::set_Fbest(genmatrix* F_ptr)
{
   *Fbest_ptr=*F_ptr;
}

inline genmatrix* fundamental::get_Fbest_ptr()
{
   return Fbest_ptr;
}

inline genmatrix* fundamental::get_P_ptr()
{
   return P_ptr;
}

inline const genmatrix* fundamental::get_P_ptr() const
{
   return P_ptr;
}

inline genmatrix* fundamental::get_Pprime_ptr()
{
   return Pprime_ptr;
}

inline const genmatrix* fundamental::get_Pprime_ptr() const
{
   return Pprime_ptr;
}

// ---------------------------------------------------------------------
// Specialized utility method compute_linear_combo_det() takes in 3x3
// matrices F1_mat and F2_mat along with scalar alpha.  It returns the
// determinant of the linear combination of F1_mat and F2_mat
// specified by alpha.

inline double fundamental::compute_linear_combo_det(
   double alpha,const genmatrix& F1_mat,const genmatrix& F2_mat) const
{
   return (alpha*F1_mat + (1-alpha)*F2_mat).determinant();
}


#endif // fundamental.h




