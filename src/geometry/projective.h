// =========================================================================
// Header file for projective class
// =========================================================================
// Last modified on 2/20/06; 3/23/12
// =========================================================================

#ifndef PROJECTIVE_H
#define PROJECTIVE_H

#include <iostream>
#include <vector>
#include "math/fourvector.h"
#include "math/threevector.h"

class genmatrix;

class projective
{

  public:

   projective(int n_pnts);
   ~projective();
   friend std::ostream& operator<< 
      (std::ostream& outstream,const projective& M);

// 4x4 projective matrix determination:

   void parse_projective_inputs(
      const std::vector<threevector>& XYZ,
      const std::vector<threevector>& UVW);
   void compute_projective_matrix();
   threevector transform_XYZ_to_UVW(const threevector& XYZ);
   void transform_XYZs_to_UVWs(
      const std::vector<threevector>& XYZ,std::vector<threevector>& UVW);
   void transform_XYZPs_to_UVWPs(
      const std::vector<fourvector>& XYZP,
      std::vector<fourvector>& UVWP,double null_value);
   double check_projective_matrix(
      const std::vector<threevector>& XYZ,
      const std::vector<threevector>& UVW);

  private:

   unsigned int npoints;

// 4x4 matrix *M_ptr holds the projective transformation:

   genmatrix* M_ptr;

// npoints x 4 matrices *P_ptr and *Q_ptr hold 3D vertices measured
// relative to two different Euclidean coordinate systems:

   genmatrix *P_ptr, *Q_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

#endif // projective.h




