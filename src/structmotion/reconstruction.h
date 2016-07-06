// =========================================================================
// Header file for Reconstruction class which attempts to recover 3D
// structure and motion from a temporal sequence of 2D images.
// =========================================================================
// Last modified on 2/20/06; 8/5/06; 3/26/12; 4/5/14
// =========================================================================

#ifndef RECONSTRUCTION_H
#define RECONSTRUCTION_H

#include <vector>
#include "math/fourvector.h"
#include "geometry/linesegment.h"
#include "math/threevector.h"
#include "math/twovector.h"

class genmatrix;
class newton;

class Reconstruction
{

  public:

   Reconstruction();
   ~Reconstruction();

// Set and get methods:

   void set_D_ptr(genmatrix* Dptr);
   void set_missing_data_ptr(genmatrix* m_ptr);
   genmatrix* get_D_ptr();
   genmatrix* get_missing_data_ptr();
   genmatrix* get_A_ptr();
   genmatrix* get_B_ptr();
   genmatrix get_AB();

// Tiepoint COM methods:

   twovector compute_2D_COM(const std::vector<twovector>& XY);
   void recenter_tiepoints(std::vector<twovector>& XY);
   void recenter_4D_COM(
      std::vector<twovector>& XY,std::vector<twovector>& UV);

// Missing measurement recovery methods:

   void parse_multiimage_tiepoints(
      unsigned int mdim,unsigned int ndim,
      const std::vector<std::vector<twovector> >& XY);
   void instantiate_RC_matrices();
   void randomly_initialize_missing_data();
   void randomly_initialize_missing_data(
      double missing_magnitude,const genmatrix& Wbest);
   void initialize_column_factor_matrix();
   void estimate_RC_matrices(
      unsigned int n_iters,double max_reasonable_determinant);
   void refine_measurement_matrix(
      unsigned int n_iters,genmatrix& W,const genmatrix& Wreduced);

// Affine reconstruction methods:

   void recenter_measurement_matrix(unsigned int mdim,unsigned int ndim);
   void reconstruct_affine_structure(
      unsigned int mdim,unsigned int ndim,genmatrix* A0_ptr,genmatrix* P0_ptr);
   genmatrix* recompute_measurement_matrix(
      unsigned int mdim,unsigned int ndim,genmatrix* A0_ptr,genmatrix* P0_ptr);

// Affine to Euclidean reconstruction upgrade methods:

   void affine_to_euclidean_transformation(
      int n_images,int n_perpendicular_constraints,
      int n_equal_length_constraints,const genmatrix* A0_ptr,
      const std::vector<linesegment>* edge_ptr,
      const genmatrix* perpendicular_edges_ptr,
      const genmatrix* equal_length_edges_ptr,genmatrix& A);
   void generate_camera_newton_inputs(
      unsigned int n_images,const genmatrix* A0_ptr,newton& Newton_camera);
   void generate_target_newton_inputs(
      const std::vector<linesegment>* edge_ptr,
      const genmatrix* perpendicular_edges_ptr,
      const genmatrix* equal_length_edges_ptr,newton& Newton_target);
   void compute_A_from_camera_constraints(
      unsigned int max_iters,newton& Newton_camera);
   void compute_S_from_camera_and_target_constraints(
      int max_iters,newton& Newton_camera,newton& Newton_target);

// Euclidean information extraction methods:

   void reconstruct_Euclidean_worldpoints(
      const genmatrix& A,genmatrix* P_ptr);
   void extract_imageplanes_and_uscales(
      const genmatrix* A0_ptr,const genmatrix& A,
      std::vector<threevector>& camera_axes,std::vector<double>& u_scale);
   void compute_camera_orientation_and_uscale(
      int i,const genmatrix* A0_ptr,const genmatrix& A,
      std::vector<threevector>& camera_axes,std::vector<double>& u_scale);
   void compute_orthographic_coords(
      const genmatrix* P0_ptr,const std::vector<threevector>& camera_axes,
      const std::vector<double>& u_scale,std::vector<threevector>& UVW);
   void compute_orthographic_coords(
      const genmatrix* P0_ptr,const threevector& u_hat,
      const threevector& v_hat,const threevector& w_hat,
      std::vector<threevector>& UVW);
   
  private:

   static const unsigned int rank; // rank=3 for affine imagery reconstruction

// Fourvector to hold COM defined by (X,Y) and (U,V) tiepoints:

   fourvector COM;

// 2m dimensional genvector *trans_ptr holds image plane translations:

   genvector* trans_ptr;

// 2m x n multiimage measurement genmatrix *D_ptr:

   genmatrix* D_ptr;
   genmatrix* Drecenter_ptr;

// 2m x n missing data binary genmatrix *missing_data_ptr:

   genmatrix* missing_data_ptr;

// 2x4 projection matrix *M_ptr:

   genmatrix* M_ptr;

// 2m x 3 column factor matrix *A_ptr and 3 x n row factor matrix
// *B_ptr are used in the Guerreiro & Aguiar algorithm to recover
// missing data in the rank 3 measurement matrix *D_ptr:

   genmatrix* A_ptr;
   genmatrix* B_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void Reconstruction::set_D_ptr(genmatrix* Dptr)
{
   D_ptr=Dptr;
}

inline void Reconstruction::set_missing_data_ptr(genmatrix* m_ptr)
{
   missing_data_ptr=m_ptr;
}

inline genmatrix* Reconstruction::get_D_ptr()
{
   return D_ptr;
}

inline genmatrix* Reconstruction::get_missing_data_ptr()
{
   return missing_data_ptr;
}

inline genmatrix* Reconstruction::get_A_ptr()
{
   return A_ptr;
}

inline genmatrix* Reconstruction::get_B_ptr()
{
   return B_ptr;
}

inline genmatrix Reconstruction::get_AB()
{
   if (A_ptr != NULL && B_ptr != NULL)
   {
      return (*A_ptr) * (*B_ptr);
   }
   else
   {
      std::cout << "Error in Reconstruction::get_AB()" << std::endl;
      std::cout << "A_ptr = " << A_ptr << " B_ptr = " << B_ptr << std::endl;
   }
}

#endif // Reconstruction.h




