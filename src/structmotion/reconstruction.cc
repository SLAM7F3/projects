// ==========================================================================
// Reconstruction matrix class member functions
// ==========================================================================
// Last modified on 7/31/06; 3/23/12; 3/26/12; 4/5/14
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "templates/mytemplates.h"
#include "numerical/newton.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "numerical/param_range.h"
#include "structmotion/reconstruction.h"
#include "general/stringfuncs.h"

using std::cout;
using std::cin;
using std::endl;
using std::pair;
using std::string;
using std::vector;

const unsigned int Reconstruction::rank=3;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Reconstruction::initialize_member_objects()
{
   D_ptr=NULL;
   A_ptr=NULL;
   B_ptr=NULL;
   Drecenter_ptr=NULL;
   missing_data_ptr=NULL;
   trans_ptr=NULL;
}		       

void Reconstruction::allocate_member_objects()
{
   M_ptr=new genmatrix(2,4);
}

Reconstruction::Reconstruction()
{
   allocate_member_objects();
   initialize_member_objects();
}

Reconstruction::~Reconstruction()
{
   delete D_ptr;
   delete A_ptr;
   delete B_ptr;
   delete Drecenter_ptr;
   delete missing_data_ptr;
   delete trans_ptr;
   delete M_ptr;
   D_ptr=NULL;
   Drecenter_ptr=NULL;
   missing_data_ptr=NULL;
   trans_ptr=NULL;
   M_ptr=NULL;
}

// ==========================================================================
// Tiepoint COM methods
// ==========================================================================

// Method compute_2D_COM

twovector Reconstruction::compute_2D_COM(const vector<twovector>& XY)
{
   twovector XY_avg(0,0);
   for (unsigned int n=0; n<XY.size(); n++)
   {
      XY_avg += XY[n];
   }
   XY_avg /= double(XY.size());
   return XY_avg;
}

void Reconstruction::recenter_tiepoints(vector<twovector>& XY)
{
   twovector XY_avg(compute_2D_COM(XY));
   for (unsigned int n=0; n<XY.size(); n++)
   {
      XY[n] -= XY_avg;
   }
}

void Reconstruction::recenter_4D_COM(
   vector<twovector>& XY,vector<twovector>& UV)
{
   twovector XY_avg(compute_2D_COM(XY));
   twovector UV_avg(compute_2D_COM(UV));

   recenter_tiepoints(XY);
   recenter_tiepoints(UV);
   COM=fourvector(UV_avg.get(0),UV_avg.get(1),XY_avg.get(0),XY_avg.get(1));
}

// ==========================================================================
// Missing measurement recovery methods:
// ==========================================================================

// Member function parse_multiimage_tiepoints reads in an STL vector
// of vectors containing multi-image tiepoint information.  It stores
// all the tiepoint data within member matrix *D_ptr.

void Reconstruction::parse_multiimage_tiepoints(
   unsigned int mdim,unsigned int ndim,const vector<vector<twovector> >& XY)
{
   delete D_ptr;
   D_ptr=new genmatrix(mdim,ndim);

   for (unsigned int i=0; i<mdim/2; i++)
   {
      vector<twovector> curr_tiepoints=XY[i];
      for (unsigned int j=0; j<ndim; j++)
      {
         twovector curr_tiepoint=curr_tiepoints[j];
         double x=curr_tiepoints[j].get(0);
         double y=curr_tiepoints[j].get(1);
         D_ptr->put(2*i,j,x);
         D_ptr->put(2*i+1,j,y);
      } // loop over index j labeling tiepoints
   } // loop over index i labeling image number

   cout << "*D_ptr = " << *D_ptr << endl;
//   cout << "D_ptr->mdim = " << D_ptr->get_mdim() 
//        << " D_ptr->ndim = " << D_ptr->get_ndim() << endl;
}

// ---------------------------------------------------------------------
// Member function instantiate_RC_matrices dynamically generates the
// column and row factor matrices used in the Guerreiro & Aguiar
// algorithm to recover missing data in the rank 3 measurement matrix
// *D_ptr:

void Reconstruction::instantiate_RC_matrices()
{
   A_ptr=new genmatrix(D_ptr->get_mdim(),3);
   B_ptr=new genmatrix(3,D_ptr->get_ndim());
}

// ---------------------------------------------------------------------
// Member function randomly_initialize_missing_data fills missing data
// slots within measurement matrix W with random entries drawn from a
// gaussian distribution.  Binary mask genmatrix *missing_data_ptr
// contains 1 [0] at locations where *D_ptr does [not] have data.

void Reconstruction::randomly_initialize_missing_data()
{
   for (unsigned int m=0; m<D_ptr->get_mdim(); m++)
   {
      for (unsigned int n=0; n<D_ptr->get_ndim(); n++)
      {
         if (nearly_equal(missing_data_ptr->get(m,n),0))
         {
            D_ptr->put(m,n,0.5*nrfunc::gasdev());
         }
      }
   }
}

void Reconstruction::randomly_initialize_missing_data(
   double missing_magnitude,const genmatrix& Wbest_estimate)
{
   for (unsigned int m=0; m<D_ptr->get_mdim(); m++)
   {
      for (unsigned int n=0; n<D_ptr->get_ndim(); n++)
      {
         if (nearly_equal(missing_data_ptr->get(m,n),0))
         {
            D_ptr->put(m,n,Wbest_estimate.get(m,n)+
                       missing_magnitude*nrfunc::gasdev());
         }
      }
   }
}

// ---------------------------------------------------------------------
// Member function initialize_column_factor_matrix sets the initial
// guess for column factor matrix *A_ptr used in the Guerreiro &
// Aguiar algorithm equal to unitary matrix U derived in the singular
// value decomposition of measurement matrix *D_ptr.

void Reconstruction::initialize_column_factor_matrix()
{
   unsigned int mdim=D_ptr->get_mdim();
   unsigned int ndim=D_ptr->get_ndim();
   
   genmatrix U(mdim,rank),W(rank,rank),V(ndim,rank);
   D_ptr->rank_approximation(rank,U,W,V);
   if (A_ptr==NULL)
   {
      cout << "Error in Reconstruction::initialize_column_factor_matrix()!"
           << endl;
      cout << "A_ptr = NULL" << endl;
      cout << "Need to have called Reconstruction::initialize_RC_matrices()"
           << " before this method" << endl;
      exit(-1);
   }
   else
   {
      genmatrix sqrtW(rank,rank);
      for (unsigned int r=0; r<rank; r++)
      {
         sqrtW.put(r,r,sqrt(W.get(r,r)));
      }
      *A_ptr=U*sqrtW;
//      *A_ptr=U;
   }
}

// ---------------------------------------------------------------------
// Member function estimate_RC_matrices implements the 2 step "RC"
// algorithm of Guerreiro and Aguiar outlined in their article
// "Estimate of rank deficient matrices from partial observations"
// (2003).  

void Reconstruction::estimate_RC_matrices(
   unsigned int n_iters,double max_reasonable_determinant)
{
   unsigned int mdim=D_ptr->get_mdim();
   unsigned int ndim=D_ptr->get_ndim();
   genvector ones(rank);
   ones.initialize_values(1);

   genmatrix AMn(mdim,rank);
   genmatrix prefactor(rank,rank),prefactor_inv(rank,rank);
   genvector wn(mdim);
   genvector mn(mdim);
   genvector wnmn(mdim);
   genvector bn(rank);

   genmatrix BMm(rank,ndim);
   genmatrix postfactor(rank,rank),postfactor_inv(rank,rank);
   genvector wm(ndim);
   genvector mm(ndim);
   genvector wmmm(ndim);

   genvector am(rank);

//   const double max_reasonable_determinant=1E10;
//   const double max_reasonable_determinant=1E8;
   
   for (unsigned int iter=0; iter<n_iters; iter++)
   {
      for (unsigned int n=0; n<ndim; n++)
      {
         D_ptr->get_column(n,wn);
         missing_data_ptr->get_column(n,mn);
         wnmn=wn.elementwise_product(mn);

         genmatrix Mn(mn.outerproduct(ones));
         AMn=A_ptr->elementwise_product(Mn);
         prefactor=A_ptr->transpose() * AMn;
         prefactor.inverse(prefactor_inv);

         double pre_det;
         prefactor.determinant(pre_det);
         if (fabs(pre_det) > max_reasonable_determinant)
         {
//         cout << "Warning prefactor determinant = " << pre_det << endl;
            int m_max,r_max;
            A_ptr->max_abs_element_value(m_max,r_max);
            A_ptr->put(m_max,r_max,2*(nrfunc::ran1()-0.5));
         } // pre_det > max_reasonable_determinant conditional
         else
         {
            bn=prefactor_inv*A_ptr->transpose()*wnmn;
            B_ptr->put_column(n,bn);
         }
//         cout << "n = " << n << " bn = " << bn << endl;
      } // loop over index n labeling columns

//      cout << "After perturbing B:" << endl;
//      cout << "A = " << A << endl;
//      cout << "B = " << B << endl;
//      cout << "A*B = " << A*B << endl;

      for (unsigned int m=0; m<mdim; m++)
      {
         D_ptr->get_row(m,wm);
         missing_data_ptr->get_row(m,mm);
         wmmm=wm.elementwise_product(mm);         

         genmatrix Mm(ones.outerproduct(mm));
         BMm=B_ptr->elementwise_product(Mm);
         
         postfactor=BMm * B_ptr->transpose();
         postfactor.inverse(postfactor_inv);

         double post_det;
         postfactor.determinant(post_det);
         if (fabs(post_det) > max_reasonable_determinant)
         {
//         cout << "Warning postfactor determinant = " << post_det << endl;
            int r_max,n_max;
            B_ptr->max_abs_element_value(r_max,n_max);
            B_ptr->put(r_max,n_max,2*(nrfunc::ran1()-0.5));
         } // post_det > max_reasonable_determinant conditional
         else
         {
            am=wmmm*B_ptr->transpose()*postfactor_inv;
            A_ptr->put_row(m,am);
         }
      
//         cout << "m = " << m << " A = " << A << endl;
//         cout << "am = " << am << endl;
//         cout << "wmmm = " << wmmm << " postface_inv = " << postfactor_inv
//              << endl;
      } // loop over index m labeling rows
   
   } // loop over iter index
}

// ---------------------------------------------------------------------
// Member function refine_measurement_matrix implements a naive
// Newton-Raphson procedure which we had hoped might yield a
// significant improvement in missing data reconstruction results.
// Given some initial estimate for the measurement matrix within input
// genmatrix W, this method first resets the known elements to their
// "perfect" values.  It next performs an SVD reduction of the
// adjusted measurement matrix to rank 3.  After iterating n_iters, we
// hoped this trivial approach would yield a measurement matrix closer
// to the true measurement matrix passed as Wreduced into this method
// for comparison purposes.  As of 2/20/06, we no longer believe that
// this naive approach works...

void Reconstruction::refine_measurement_matrix(
   unsigned int n_iters,genmatrix& W,const genmatrix& Wreduced)
{
   genmatrix Wnew(W);
   for (unsigned int iter=0; iter<n_iters; iter++)
   {

      Wnew=W;

// First force all known entries in measurement matrix to agree with
// input data:

      for (unsigned int m=0; m<D_ptr->get_mdim(); m++)
      {
         for (unsigned int n=0; n<D_ptr->get_ndim(); n++)
         {
            if (!nearly_equal(missing_data_ptr->get(m,n),0))
            {
               Wnew.put(m,n,D_ptr->get(m,n));
            }
         } // loop over index n labeling columns in *D_ptr
      } // loop over index m labeling rows in *D_ptr

// Next perform SVD reduction to force measurement matrix to have rank
// 3:

      Wnew.rank_approximation(3,W);
      
//      double score=(Wreduced-W).sqrd_Frobenius_norm();
//      if (iter%100==0)
//      {
//         cout << "iter = " << iter 
//              << " score = " << score 
//              << " Max abs diff value = " 
//              << (Wreduced-W).max_abs_element_value() << endl;
//      }
      
   } // loop over iter index
   
}

// ==========================================================================
// Affine reconstruction methods
// ==========================================================================

// Member function recenter_measurement_matrix computes the average of
// each row within measurement matrix *D_ptr.  It then substracts this
// average from all of the elements within the row, enters the
// recentered values into the rows of *Drecenter_ptr and saves the
// image plane translation information within genvector *trans_ptr.

void Reconstruction::recenter_measurement_matrix(
   unsigned int mdim,unsigned int ndim)
{
   delete Drecenter_ptr;
   Drecenter_ptr=new genmatrix(mdim,ndim);
   delete trans_ptr;
   trans_ptr=new genvector(mdim);

   for (unsigned int m=0; m<mdim; m++)
   {
      int counter=0;
      trans_ptr->put(m,0);
      for (unsigned int n=0; n<ndim; n++)
      {
         double curr_value=D_ptr->get(m,n);
         if (curr_value > 0.5*NEGATIVEINFINITY)
         {
            trans_ptr->put(m,curr_value+trans_ptr->get(m));
            counter++;
         }
      } // loop over index n
      trans_ptr->put(m,trans_ptr->get(m)/counter);

      for (unsigned int n=0; n<ndim; n++)
      {
         Drecenter_ptr->put(m,n,D_ptr->get(m,n)-trans_ptr->get(m));
      }
   } // loop over index m
}

// ---------------------------------------------------------------------
// Member function reconstruct_affine_structure implements the
// Tomasi-Kanade factorization algorithm for affine shape from motion.
// In this method, we follow both the original paper "Factoring image
// sequences into shape and motion" by Tomasi and Kanade as well as
// section 12.3 in "Computer Vision" by Forsyth and Ponce.

// This method returns an affine estimate of the camera motion in
// output mdim x 3 genmatrix *A0_ptr as well as 3D affine points in
// output 3 x ndim genmatrix *P0_ptr.
   
void Reconstruction::reconstruct_affine_structure(
   unsigned int mdim,unsigned int ndim,genmatrix* A0_ptr,genmatrix* P0_ptr)
{
   if (mdim < ndim)
   {
      genmatrix U(ndim,mdim),W(mdim,mdim),V(mdim,mdim);
      genmatrix Dtrans(ndim,mdim);
      Dtrans=Drecenter_ptr->transpose();
      
      Dtrans.sorted_singular_value_decomposition(U,W,V);

//         cout << "U = " << U << endl;
//         cout << "W = " << W << endl;
//         cout << "V = " << V << endl;

      genmatrix U3(ndim,3);
      genmatrix W3(3,3);
      genmatrix V3(mdim,3);

      threevector column;
      for (unsigned int c=0; c<3; c++)
      {
         U3.genarray::put_column(c,U.genarray::get_column(c));
         W3.put(c,c,W.get(c,c));
         V3.genarray::put_column(c,V.genarray::get_column(c));
      }

//         cout << "U3 = " << U3 << endl;
//         cout << "W3 = " << W3 << endl;
//         cout << "V3 = " << V3 << endl;
//         cout << "U3*W3*V3.trans = " << U3*W3*V3.transpose() << endl;

      *A0_ptr=V3;
      *P0_ptr=W3*U3.transpose();
   }
   else
   {
      genmatrix U(mdim,ndim),W(ndim,ndim),V(ndim,ndim);
      Drecenter_ptr->sorted_singular_value_decomposition(U,W,V);

      genmatrix U3(mdim,3);
      genmatrix W3(3,3);
      genmatrix V3(ndim,3);

      for (unsigned int c=0; c<3; c++)
      {
         U3.genarray::put_column(c,U.genarray::get_column(c));
         W3.put(c,c,W.get(c,c));
         V3.genarray::put_column(c,V.genarray::get_column(c));
      }

//         cout << "U3 = " << U3 << endl;
//         cout << "W3 = " << W3 << endl;
//         cout << "V3 = " << V3 << endl;
//         cout << "U3*W3*V3.trans = " << U3*W3*V3.transpose() << endl;

      *A0_ptr=U3;
      *P0_ptr=W3*V3.transpose();
   } // mdim < ndim conditional

//   cout << "A0 = " << *A0_ptr << endl;
//   cout << "A0trans = " << A0_ptr->transpose() << endl;
//   cout << "A0*A0trans = " << (*A0_ptr) * (A0_ptr->transpose()) << endl;
//   cout << "A0trans*A0 = " << (A0_ptr->transpose()) * (*A0_ptr) << endl;
}

// ---------------------------------------------------------------------
// Member function recompute_measurement_matrix reforms and returns
// the uncentered tiepoint measurement matrix from its factorized
// components *A0_ptr and *P0_ptr.

genmatrix* Reconstruction::recompute_measurement_matrix(
   unsigned int mdim,unsigned int ndim,genmatrix* A0_ptr,genmatrix* P0_ptr)
{
   genvector ones(ndim);
   for (unsigned int n=0; n<ndim; n++)
   {
      ones.put(n,1);
   }
   
   genmatrix* Dnew_ptr=new genmatrix(mdim,ndim);
   *Dnew_ptr=( *A0_ptr ) * ( *P0_ptr) + 
      genmatrix(trans_ptr->outerproduct(ones));
   return Dnew_ptr;
}

// ==========================================================================
// Affine to Euclidean reconstruction upgrade methods:
// ==========================================================================

// Member function affine_to_euclidean_transformation takes in affine
// transformation information within input genmatrix *A0_ptr.  It
// instantiates a quadratic newton form using row information stored
// within affine camera matrix *A0_ptr.  It then iteratively searches
// for the 3x3 invertible matrix A which converts these rows into an
// orthonormal basis.

// See PC's notes "Euclidean upgrade from multiple affine views" dated
// 1/30/06.

void Reconstruction::affine_to_euclidean_transformation(
   int n_images,int n_perpendicular_constraints,
   int n_equal_length_constraints,const genmatrix* A0_ptr,
   const vector<linesegment>* edge_ptr,
   const genmatrix* perpendicular_edges_ptr,
   const genmatrix* equal_length_edges_ptr,genmatrix& A)
{
   const int dim=3;

   const unsigned int mdim=2*n_images;
   newton Newton_camera(dim,mdim);
   Newton_camera.set_Bptr_to_Aptr();
   Newton_camera.set_Tptr_to_Sptr();
   generate_camera_newton_inputs(n_images,A0_ptr,Newton_camera);

   int n_target_constraints=n_perpendicular_constraints+
      n_equal_length_constraints;
   newton Newton_target(dim,n_target_constraints);
   Newton_target.set_Bptr_to_Ainvtransptr();
   Newton_target.set_Tptr_to_Sinvptr();
   generate_target_newton_inputs(
      edge_ptr,perpendicular_edges_ptr,equal_length_edges_ptr,Newton_target);

// Initially use just camera constraints to generate reasonable first
// estimate for invertible 3x3 matrix A:
   
   int max_iters=50;
//   for (unsigned int I=0; I<4; I++)
   {
      Newton_camera.randomly_initialize_A();
      compute_A_from_camera_constraints(max_iters,Newton_camera);
   }

   Newton_camera.compute_Ainvtrans_from_A();
   *(Newton_target.get_Ainvtransptr())=*(Newton_camera.get_Ainvtransptr());

// Next use both camera and target constraints to jointly refine
// estimate for matrix A:

//   Newton_camera.compute_S_from_A();

//   compute_S_from_camera_and_target_constraints(
//      max_iters,Newton_camera,Newton_target);
//   Newton_camera.compute_A_from_S();

   A=*(Newton_camera.get_Aptr());
}

// ---------------------------------------------------------------------
// Member function generate_camera_newton_inputs encodes a2.a2=1 and
// a2.a1=0 where a1 and a2 are row vectors within the affine
// projection matrix.  (See section 12.4.2 in Computer Vision: A
// Modern Approach by Forsyth and Ponce.)  For application to
// nonnominal satellite motion determination, we do NOT impose the
// constraint a1.a1=1, for the cross range scale is generally unknown.

void Reconstruction::generate_camera_newton_inputs(
   unsigned int n_images,const genmatrix* A0_ptr,newton& Newton_camera)
{
   for (unsigned int i=0; i<n_images; i++)
   {
      threevector p1(A0_ptr->get(2*i+0,0),A0_ptr->get(2*i+0,1),
                     A0_ptr->get(2*i+0,2));
      threevector p2(A0_ptr->get(2*i+1,0),A0_ptr->get(2*i+1,1),
                     A0_ptr->get(2*i+1,2));

// To simulate a priori unknown cross-range scaling, we do NOT impose
// a normalization constraint on horizontal direction vector p1:

//      Newton_camera.get_Delta().push_back(p1);
//      Newton_camera.get_Deltap().push_back(p1);
//      Newton_camera.get_N().push_back(1);

      Newton_camera.get_Delta().push_back(p2);
      Newton_camera.get_Deltap().push_back(p2);
      Newton_camera.get_N().push_back(1);
      
      Newton_camera.get_Delta().push_back(p1);
      Newton_camera.get_Deltap().push_back(p2);
      Newton_camera.get_N().push_back(0);
   }
   
   Newton_camera.fill_data_matrix();
}

// ---------------------------------------------------------------------
// Member function generate_target_newton_inputs encodes parallel and
// orthogonal Euclidean target edge constraints within the Deltap,
// Delta and N members of input Newton_target.

void Reconstruction::generate_target_newton_inputs(
   const vector<linesegment>* edge_ptr,
   const genmatrix* perpendicular_edges_ptr,
   const genmatrix* equal_length_edges_ptr,newton& Newton_target)
{
   Newton_target.get_Epsp().clear();
   Newton_target.get_Eps().clear();

// First take care of target right angle constraints:
   
   for (unsigned int i=0; i<12; i++)
   {
      for (unsigned int j=i+1; j<12; j++)
      {
         double curr_dotproduct=perpendicular_edges_ptr->get(i,j);
         if (nearly_equal(curr_dotproduct,0))
         {
            threevector p1( edge_ptr->at(i).get_ehat() );
            threevector p2( edge_ptr->at(j).get_ehat() );
            Newton_target.get_Deltap().push_back(p1);            
            Newton_target.get_Delta().push_back(p2);            
            Newton_target.get_Epsp().push_back(Zero_vector);
            Newton_target.get_Eps().push_back(Zero_vector);
            Newton_target.get_N().push_back(0);

            cout << "Deltap.Delta = " << p1.dot(p2)
                 << " N = " << curr_dotproduct << endl;
         }
      } // loop over index j labeling columns in *perpendicular_edges_ptr
   } // loop over index i labeling rows in sym *perpendicular_edges_ptr matrix
   
// Next take care of target equal side length constraints:

   for (unsigned int i=0; i<12; i++)
   {
      for (unsigned int j=i+1; j<12; j++)
      {
         double curr_dotproduct=equal_length_edges_ptr->get(i,j);
         if (nearly_equal(curr_dotproduct,1))
         {
            threevector p1( edge_ptr->at(i).get_v2()-
                            edge_ptr->at(i).get_v1() );
            threevector p2( edge_ptr->at(j).get_v2()-
                            edge_ptr->at(j).get_v1() );

            Newton_target.get_Deltap().push_back(p1);            
            Newton_target.get_Delta().push_back(p1);            
            Newton_target.get_Epsp().push_back(p2);
            Newton_target.get_Eps().push_back(p2);
            Newton_target.get_N().push_back(0);

            cout << "Deltap.Delta = " << p1.dot(p1)
                 << " Epsp.Eps = " << p2.dot(p2) << endl;
         }
      } // loop over index j labeling columns in *equal_length_edges_ptr
   } // loop over index i labeling rows in sym *equal_length_edges_ptr matrix

   outputfunc::enter_continue_char();
//   Newton_target.fill_data_matrix();
}

// ---------------------------------------------------------------------
// Member function compute_A_from_camera_constraints 

void Reconstruction::compute_A_from_camera_constraints(
   unsigned int max_iters,newton& Newton_camera)
{
   const double TINY_score=1E-7;
   double minimum_score=POSITIVEINFINITY;

   Newton_camera.randomly_initialize_A();

   for (unsigned int i=0; i<max_iters; i++)
   {
      double curr_score=Newton_camera.compute_F();
      Newton_camera.compute_Jacobian();
      Newton_camera.refine_B();
//      cout << "i = " << i 
//           << " camera_score = " << curr_score 
//           << " min score = " << minimum_score << endl;

      if (curr_score < minimum_score)
      {
         minimum_score=curr_score;
         *(Newton_camera.get_best_Aptr())=*(Newton_camera.get_Aptr());
//         cout << "best A = " << *best_Aptr << endl;

// Break out of Newton-Raphson iteration loop if minimum_score is
// sufficiently close to zero:

         if (minimum_score < TINY_score) break;
      }
   } // loop over index i labeling Newton-Raphson iteration

   *(Newton_camera.get_Aptr())=*(Newton_camera.get_best_Aptr());

   cout << "Camera A = " << *(Newton_camera.get_Aptr()) << endl;
   cout << "min score = " << minimum_score << endl;
//   cout << endl;
}

// ---------------------------------------------------------------------
// Member function compute_S_from_camera_and_target_constraints first
// generates the 3x3 symmetric matrix S = Atrans*A from the initial
// estimate for the 3x3 invertible matrix A which converts the affine
// to a Euclidean reconstruction.  It next decomposes S into 6
// continuous plus 1 discrete parameter ( = det U).  This method then
// iteratively performs a brute force search over the 6-dimensional
// parameter space within the vicinity of the point corresponding to
// the initial estimate.  For each S, it evaluates all components of
// the camera difference vector F = Deltap^T S Delta - N which should
// ideally equal 0.  It similarly evaluates all components of the
// target difference vector G = Deltap^T Sinv Delta - Epsp^T Sinv Eps
// - N which should also ideally equal 0.  It takes the camera motion
// [target structure] score function to equal the median valued F [G]
// entry.  The total score is set equal to the camera score times some
// power of the target score.  The machine minimizes this score as a
// function of the 6 parameters which define matrix S.

// This member function generates the optimal value of S from which A
// may be recovered via Cholesky decomposition.  (The search over S is
// performed so as to ensure that S always has positive definite
// eigenvalues.  So taking A = sqrt(S) is guaranteed to be possible.)

void Reconstruction::compute_S_from_camera_and_target_constraints(
   int max_iters,newton& Newton_camera,newton& Newton_target)
{
   genmatrix* S_ptr=Newton_camera.compute_S_from_A();
   genmatrix* Sinv_ptr=Newton_target.compute_Sinv_from_Ainvtrans();

   cout << "S = " << *S_ptr << endl;
   cout << "Sinv = " << *Sinv_ptr << endl;
   cout << "S*Sinv = " << (*S_ptr) * (*Sinv_ptr) << endl;

   double theta,phi,chi,detU;
   threevector lambda;
   if (S_ptr->decompose_sym(lambda,theta,phi,chi,detU))
   {
      cout << "lambda = " << lambda << endl;
      cout << "theta = " << theta*180/PI << " phi = " << phi*180/PI
           << " chi = " << chi*180/PI << endl << endl;
      cout << "detU = " << detU << endl;

//      S_ptr->reconstruct_sym(lambda,theta,phi,chi,detU);
//      Sinv_ptr->reconstruct_inverse_sym(lambda,theta,phi,chi,detU);
//      cout << "Reconstructed S = " << *S_ptr << endl;
//      cout << "Reconstructed Sinv = " << *Sinv_ptr << endl;
   }

// Set up parameter search intervals:

   threevector log_lambda(log(lambda.get(0)),log(lambda.get(1)),
                          log(lambda.get(2)));
//   threevector dlog_lambda(0,0,0);
   threevector dlog_lambda(1,1,1);
   threevector log_lambda_lo=log_lambda-dlog_lambda;
   threevector log_lambda_hi=log_lambda+dlog_lambda;

   threevector angle(theta,phi,chi);
//   threevector dangle(0,0,0);
   threevector dangle(10*PI/180,10*PI/180,10*PI/180);
   threevector angle_lo=angle-dangle;
   threevector angle_hi=angle+dangle;

//   int n_lambda_bins=1;
//   int n_angle_bins=1;
   int n_lambda_bins=9;
   int n_angle_bins=9;

   param_range log_lambda0(
      log_lambda_lo.get(0),log_lambda_hi.get(0),n_lambda_bins);
   param_range log_lambda1(
      log_lambda_lo.get(1),log_lambda_hi.get(1),n_lambda_bins);
   param_range log_lambda2(
      log_lambda_lo.get(2),log_lambda_hi.get(2),n_lambda_bins);
   
   param_range angle0(angle_lo.get(0),angle_hi.get(0),n_angle_bins);
   param_range angle1(angle_lo.get(1),angle_hi.get(1),n_angle_bins);
   param_range angle2(angle_lo.get(2),angle_hi.get(2),n_angle_bins);

   cout << "n_lambda0 bins = " << log_lambda0.get_nbins() << endl;

//   const int n_iters=1;
   const unsigned int n_iters=5;
//   const int n_iters=6;
   double best_score=POSITIVEINFINITY;
   double best_camera_score,best_target_score;
   for (unsigned int iter=0; iter<n_iters; iter++)
   {
      outputfunc::newline();
      string iter_banner="Iteration = "+stringfunc::number_to_string(iter+1)
         +" of "+stringfunc::number_to_string(n_iters);
      outputfunc::write_big_banner(iter_banner);
      
      while (log_lambda0.prepare_next_value())
      {
         double lambda0=exp(log_lambda0.get_value());
         lambda.put(0,lambda0);
         string banner="lambda0 counter = "+
            stringfunc::number_to_string(log_lambda0.get_counter())
            +" of "+stringfunc::number_to_string(log_lambda0.get_nbins());
         outputfunc::write_banner(banner);

         while (log_lambda1.prepare_next_value())
         {
            double lambda1=exp(log_lambda1.get_value());
            lambda.put(1,lambda1);

            while (log_lambda2.prepare_next_value())
            {
               double lambda2=exp(log_lambda2.get_value());
               lambda.put(2,lambda2);

               while (angle0.prepare_next_value())
               {
                  theta=angle0.get_value();
                  while (angle1.prepare_next_value())
                  {
                     phi=angle1.get_value();
                     while (angle2.prepare_next_value())
                     {
                        chi=angle2.get_value();
                        S_ptr->reconstruct_sym(
                           lambda,theta,phi,chi,detU);
                        Sinv_ptr->reconstruct_inverse_sym(
                           lambda,theta,phi,chi,detU);
      
                        Newton_camera.compute_F_from_T();
                        Newton_target.compute_F_from_T();
                        double camera_score=Newton_camera.median_abs_Fentry();
                        double target_score=Newton_target.median_abs_Fentry();

// In order to strongly weight 3D target constraints, we raise the
// power with which target_score enters into the total score compared
// to that of camera_score:

//                        double total_score=camera_score*target_score;
                        double total_score=camera_score*sqr(target_score);
//                        double total_score=camera_score*
//                           sqr(sqr(target_score));
   
//                        cout << "lambda0 = " << lambda0 
//                             << " lambda1 = " << lambda1 
//                             << " lambda2 = " << lambda2 << endl;
//                        cout << "theta = " << theta*180/PI 
//                             << " phi = " << phi*180/PI
//                             << " chi = " << chi*180/PI << endl;
//                        cout << "cam score=" << camera_score 
//                             << " targ score=" << target_score
//                             << " tot score=" << total_score 
//                             << " best score=" << best_score << endl;

                        if (total_score < best_score)
                        {
                           best_score=total_score;
                           best_camera_score=camera_score;
                           best_target_score=target_score;
                           log_lambda0.set_best_value();
                           log_lambda1.set_best_value();
                           log_lambda2.set_best_value();
                           angle0.set_best_value();
                           angle1.set_best_value();
                           angle2.set_best_value();

                           cout << "lambda0 = " << lambda0 
                                << " lambda1 = " << lambda1 
                                << " lambda2 = " << lambda2 << endl;
                           cout << "theta = " << theta*180/PI 
                                << " phi = " << phi*180/PI
                                << " chi = " << chi*180/PI << endl;
                           cout << "cam score=" << camera_score 
                                << " targ score=" << target_score
                                << " best score=" << best_score << endl;

                        } // total_score < best_score conditional
                     } // angle2 while loop
                  } // angle1 while loop
               } // angle0 while loop
            } // log_lambda2 while loop
         } // log_lambda1 while loop
      } // log_lambda0 while loop
         
      outputfunc::newline();
      lambda=threevector(
         exp(log_lambda0.get_best_value()),exp(log_lambda1.get_best_value()),
         exp(log_lambda2.get_best_value()));
      theta=angle0.get_best_value();
      phi=angle1.get_best_value();
      chi=angle2.get_best_value();

      cout << "iter = " << iter+1 << " best_score = " << best_score << endl;
      cout << "best camera score = " << best_camera_score
           << " best target score = " << best_target_score << endl;
      cout << "best lambda0 = " << lambda.get(0) << endl;
      cout << "best lambda1 = " << lambda.get(1) << endl;
      cout << "best lambda2 = " << lambda.get(2) << endl;
      cout << "best theta = " << theta*180/PI << endl;
      cout << "best phi = " << phi*180/PI << endl;
      cout << "best chi = " << chi*180/PI << endl;

      S_ptr->reconstruct_sym(lambda,theta,phi,chi,detU);
      Sinv_ptr->reconstruct_inverse_sym(lambda,theta,phi,chi,detU);

//      cout << "Best S = " << *S_ptr << endl;
//      cout << "Best Sinv = " << *Sinv_ptr << endl;
//      cout << "S*Sinv = " << (*S_ptr) * (*Sinv_ptr) << endl;

//      double frac=0.5;
      double frac=0.6;
      log_lambda0.shrink_search_interval(log_lambda0.get_best_value(),frac);
      log_lambda1.shrink_search_interval(log_lambda1.get_best_value(),frac);
      log_lambda2.shrink_search_interval(log_lambda2.get_best_value(),frac);
      angle0.shrink_search_interval(angle0.get_best_value(),frac);
      angle1.shrink_search_interval(angle1.get_best_value(),frac);
      angle2.shrink_search_interval(angle2.get_best_value(),frac);

   } // loop over iter index labeling param lattice refinement level
}

// ==========================================================================
// Euclidean information extraction methods:
// ==========================================================================

// Member function reconstruct_Euclidean_worldpoints applies linear
// transformation Atransinv to all affine Reconstructioned points to
// convert them to a Euclidean Reconstructionion.

void Reconstruction::reconstruct_Euclidean_worldpoints(
   const genmatrix& A,genmatrix* P_ptr)
{
   genmatrix Ainv(3,3),Atrans(3,3),Ainvtrans(3,3);

   Atrans=A.transpose();
   A.inverse(Ainv);
   Ainvtrans=Ainv.transpose();

//   cout << "A = " << A << endl;
//   cout << "Atrans = " << Atrans << endl;
//   cout << "Ainv = " << Ainv << endl;
//   cout << "Ainvtrans = " << Ainvtrans << endl;

   for (unsigned int j=0; j<P_ptr->get_ndim(); j++)
   {
      threevector V(P_ptr->get(0,j),P_ptr->get(1,j),P_ptr->get(2,j));
      threevector Vnew=Ainvtrans*V;
      P_ptr->put(0,j,Vnew.get(0));
      P_ptr->put(1,j,Vnew.get(1));
      P_ptr->put(2,j,Vnew.get(2));
      cout << "Reconstructed Euclidean point " << j << " = ( "
           << P_ptr->get(0,j) << " , " << P_ptr->get(1,j)
           << " , " << P_ptr->get(2,j) << " ) " << endl;
   } // loop over index j labeling 3D world space point
}

// ---------------------------------------------------------------------
// Member function extract_imageplanes_and_uscales recovers the 3D
// worldspace locations of each affine image plane as well as cross
// range scale values.

void Reconstruction::extract_imageplanes_and_uscales(
   const genmatrix* A0_ptr,const genmatrix& A,
   vector<threevector>& camera_axes,vector<double>& u_scale)
{
   unsigned int n_images=A0_ptr->get_mdim()/2;
   for (unsigned int i=0; i<n_images; i++)
   {
//      string banner="Image "+stringfunc::number_to_string(i);
//      outputfunc::write_banner(banner);
      
      compute_camera_orientation_and_uscale(i,A0_ptr,A,camera_axes,u_scale);
      threevector u_hat(camera_axes[camera_axes.size()-3]);
      threevector v_hat(camera_axes[camera_axes.size()-2]);
      threevector w_hat(camera_axes[camera_axes.size()-1]);
   }
}

// ---------------------------------------------------------------------
// Member function compute_camera_orientation_and_uscale takes in
// integer i labeling an affine image as well as genmatrices *A0_ptr
// and A.  It reconstructs from the genmatrices the u_hat and v_hat
// axes in 3D worldspace for the ith affine image.  This method
// returns u_hat, v_hat and w_hat=u_hat x v_hat within an STL vector.

void Reconstruction::compute_camera_orientation_and_uscale(
   int i,const genmatrix* A0_ptr,const genmatrix& A,
   vector<threevector>& camera_axes,vector<double>& u_scale)
{
   threevector p1(A0_ptr->get(2*i+0,0),A0_ptr->get(2*i+0,1),
                  A0_ptr->get(2*i+0,2));
   threevector p2(A0_ptr->get(2*i+1,0),A0_ptr->get(2*i+1,1),
                  A0_ptr->get(2*i+1,2));

   threevector u_hat=A*p1;
   threevector v_hat=A*p2;
   threevector w_hat=u_hat.cross(v_hat);

//   cout << "i = " << i << " p1.mag = " << p1.magnitude()
//        << " p2.mag = " << p2.magnitude() << endl;
//   cout << "p1.p2 = " << p1.dot(p2) << endl;
//   double theta=acos(p1.unitvector().dot(p2.unitvector()));
//   cout << "theta = " << theta*180/PI << endl;

// U axis is not guaranteed to be normalized.  For satellite ISAR
// imagery cross range scaling purposes, we save its magnitude within
// output STL vector u_scale before forcing u_hat to be a unit vector:

   u_scale.push_back(u_hat.magnitude());

   u_hat=u_hat.unitvector();
   v_hat=v_hat.unitvector();
   w_hat=w_hat.unitvector();

//   cout << "u_hat = " << u_hat << endl;
//   cout << "v_hat = " << v_hat << endl;
//   cout << "w_hat = " << w_hat << endl;

// Doublecheck that reconstructed camera axes form an orthonormal
// basis:

//   cout << "u_hat.uhat = " << u_hat.dot(u_hat) << endl;
//   cout << "v_hat.vhat = " << v_hat.dot(v_hat) << endl;
//   cout << "w_hat.what = " << w_hat.dot(w_hat) << endl;
//   cout << "u_hat.vhat = " << u_hat.dot(v_hat) << endl;
//   double new_theta=
//      acos(u_hat.unitvector().dot(v_hat.unitvector()));
//   cout << "new theta = " << new_theta*180/PI << endl;

   camera_axes.push_back(u_hat);
   camera_axes.push_back(v_hat);
   camera_axes.push_back(w_hat);
}

// ---------------------------------------------------------------------
// Member function compute_orthographic_coordinates simply projects
// all 3D world points in input genmatrix *P_ptr (which are assumed to
// have already undergone a Euclidean reconstruction) onto input
// camera axes u_hat, v_hat and w_hat.

void Reconstruction::compute_orthographic_coords(
   const genmatrix* P_ptr,const vector<threevector>& camera_axes,
   const vector<double>& u_scale,vector<threevector>& UVW)
{
   UVW.clear();

   unsigned int n_images=camera_axes.size()/3;
   for (unsigned int i=0; i<n_images; i++)
   {
      cout << "Affine image i = " << i << endl;
      threevector u_vec=u_scale[i]*camera_axes[i*3+0];
      compute_orthographic_coords(
         P_ptr,u_vec,camera_axes[i*3+1],camera_axes[i*3+2],UVW);
   }
}

void Reconstruction::compute_orthographic_coords(
   const genmatrix* P_ptr,const threevector& u_hat,const threevector& v_hat,
   const threevector& w_hat,vector<threevector>& UVW)
{
   for (unsigned int j=0; j<P_ptr->get_ndim(); j++)
   {
      threevector vertex(
         P_ptr->get(0,j),P_ptr->get(1,j),P_ptr->get(2,j));
      double U=vertex.dot(u_hat);
      double V=vertex.dot(v_hat);
      double W=vertex.dot(w_hat);
//      cout << "Point j = " << j << " U = " << U << " V = " << V
//           << " W = " << W << endl;
      UVW.push_back(threevector(U,V,W));
   } // loop over index j labeling Euclidean world points
}
    
