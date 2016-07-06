// ==========================================================================
// Stand-alone optimizer_func methods
// ==========================================================================
// Last updated on 4/6/09; 5/28/09; 12/4/10; 11/25/13; 4/6/14
// ==========================================================================

#include <iostream>
#include <levmar.h>
#include <vector>
#include "video/camera.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "math/Genarray.h"
#include "geometry/homography.h"
#include "templates/mytemplates.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "optimum/optimizer.h"
#include "optimum/optimizer_funcs.h"
#include "video/photogroup.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;

namespace optimizer_func
{
   bool photo_pair_bundle_adjust_flag=false;
   unsigned int n_features,score_evaluation_counter=0;
   unsigned int n_params,n_params_per_photo,n_camera_trans_params;
   int i_photo,j_photo,n_photos=-1;
   vector<int> photo_indices;
   int n_prev_composited_photos;
   double* camera_params_ptr=NULL;
   double* homography_params_ptr=new double[9];
   double opts[LM_OPTS_SZ];
   double ba_info[LM_INFO_SZ];
   vector<double> fu,u0,v0,aspect_ratio,skew,az,el,roll,tx,ty,tz;
   genmatrix* u_meas_ptr=NULL;
   genmatrix* v_meas_ptr=NULL;
   vector<threevector> n_hats;
   optimizer* optimizer_ptr=NULL;
   photogroup* photogroup_ptr=NULL;
   photogroup* bundler_photogroup_ptr=NULL;
   FeaturesGroup* FeaturesGroup_ptr=NULL;

   rotation* Rp_ptr=new rotation();
   rotation* Rp_inv_ptr=new rotation();
   genmatrix* Kp_ptr=new genmatrix(3,3);
   genmatrix* Kp_inv_ptr=new genmatrix(3,3);

   rotation* Rq_ptr=new rotation();
   rotation* Rq_inv_ptr=new rotation();
   genmatrix* Kq_ptr=new genmatrix(3,3);
   genmatrix* Kq_inv_ptr=new genmatrix(3,3);

   threevector camera_world_posn;

   rotation* R0_ptr=NULL;
   genmatrix* G_ren_ptr=new genmatrix(3,3);

   genmatrix* output_matrix1_ptr=new genmatrix(3,3);
   genmatrix* output_matrix2_ptr=new genmatrix(3,3);
   genmatrix* output_matrix3_ptr=new genmatrix(3,3);
   genmatrix* output_matrix4_ptr=new genmatrix(3,3);
   genmatrix* output_matrix5_ptr=new genmatrix(3,3);
   genmatrix* output_matrix6_ptr=new genmatrix(3,3);
   genmatrix* derivative_term1_ptr=new genmatrix(3,3);
   genmatrix* derivative_term2_ptr=new genmatrix(3,3);
   genmatrix* derivative_term3_ptr=new genmatrix(3,3);
   genmatrix* derivative_term4_ptr=new genmatrix(3,3);
   genmatrix* derivative_term5_ptr=new genmatrix(3,3);
   genmatrix* derivative_term6_ptr=new genmatrix(3,3);

   threevector V;
   double ROSD=105;

// ==========================================================================
// Set & get methods
// ==========================================================================

   void set_photo_pair_bundle_adjust_flag(bool flag)
   {
      photo_pair_bundle_adjust_flag=flag;
   }
   
   void set_V(const threevector& V_input)
   {
      V=V_input;
      ROSD=V[0];
   }

   double* get_homography_params_array()
   {
      return homography_params_ptr;
   }

   homography* get_homography_ptr(int p,int q)
   {
      if (p > q)
      {
         cout << "Error in optimizer_func::get_homography_ptr()" << endl;
         cout << "p = " << p << " should not be greater than q = " << q
              << endl;
         return NULL;
      }
      else
      {
         return optimizer_ptr->get_homography_ptr(p,q);
      }
   }

// ==========================================================================
// Minimization methods
// ==========================================================================

   void rosenbrock(double* p, double* x, int m, int n, void* data)
   {
//         cout << "inside optimizer_func::rosenbrock()" << endl;

      x[0]=((1.0-p[0])*(1.0-p[0]) + 
            ROSD*(p[1]-p[0]*p[0])*(p[1]-p[0]*p[0]));

      for (int i=1; i<n; i++)
      {
         x[i]=x[0];
      } 
   }
   
   void rosenbrock_jacobian(
      double* p, double* jac, int m, int n, void* data)
   {
      jac[0]=(-2 + 2*p[0]-4*ROSD*(p[1]-p[0]*p[0])*p[0]);
      jac[1]=(2*ROSD*(p[1]-p[0]*p[0]));

      for (int i=1; i<n; i++)
      {
         jac[2*i]=jac[0];
         jac[2*i+1]=jac[1];
      }
   }

/*
// --------------------------------------------------------------------------
// Deprecated score function for 3D panorama problem employing
// projections of averaged 3D rays onto image planes.  This method
// does not use homography computation results in any way.

void score(double* input_param, double* x, int m, int n, void* data)
{
//         cout << "inside optimizer_func::score()" << endl;
         
double score=0;
         
for (unsigned int p=0; p<n_photos; p++)
{
double curr_az=input_param[n_params_per_photo*p+0];
double curr_el=input_param[n_params_per_photo*p+1];
double curr_roll=input_param[n_params_per_photo*p+2];
double curr_fu=input_param[n_params_per_photo*p+3];

double curr_u0=0.6666;
double curr_v0=0.5;
            
camera curr_camera(curr_fu,curr_fu,curr_u0,curr_v0);
curr_camera.set_Rcamera(curr_az,curr_el,curr_roll);
//            cout << "curr_camera = " << curr_camera << endl;

rotation* Rcamera_ptr=curr_camera.get_Rcamera_ptr();
genmatrix* K_ptr=curr_camera.get_K_ptr();

for (unsigned int f=0; f<n_features; f++)
{
double u_measured=u_meas_ptr->get(f,p);
double v_measured=v_meas_ptr->get(f,p);
               
const double TINY=0.001;
if (u_measured > -TINY && v_measured > -TINY)
{
threevector curr_q=(*K_ptr) * (*Rcamera_ptr) * n_hats[f];
double u_proj=curr_q.get(0)/curr_q.get(2);
double v_proj=curr_q.get(1)/curr_q.get(2);
score += sqr(u_proj-u_measured)+sqr(v_proj-v_measured);
} // u_measured & v_measured greater than sentinel value

} // loop over index f labeling 3D features

} // loop over index p labeling photos

x[0]=score;
for (unsigned int i=1; i<n; i++)
{
x[i]=x[0];
} 

} 
*/
 
// ==========================================================================
// Rotation homography bundle adjustment methods
// ==========================================================================

// Method initialize_bundle_adjustment_params

    void initialize_bundle_adjustment()
    {
       opts[0]=LM_INIT_MU; 		// mu
       opts[1]=1E-15; 		// eps1
       opts[2]=1E-15; 		// eps2
       opts[3]=1E-20;			// eps3
       opts[4]=LM_DIFF_DELTA; 	// delta
       // relevant only if the finite difference Jacobian version is used
    }

// --------------------------------------------------------------------------
// Method rotation_homography_bundle_adjustment first initializes some
// parameters for nonlinear Levenberg-Marquadt optimization.  It next
// computes the number of parameters to be varied which depends upon
// the number of photos which were previously composited.  This method
// then performs bundle adjustment for the photos' focal and rotation
// parameters using the symemtric score function of Brown and Lowe
// which treats every photo input within the composite symmetrically.
// As of Jan 2009, we let the Levenberg-Marquadt optimization routines
// compute derivatives numerically rather than employ closed-form
// Jacobian derivative expressions.

    void rotation_homography_bundle_adjustment(
       int n_previously_composited_photos,
       const vector<int>& indices_of_photos_to_be_composited,optimizer* o_ptr)
    {
       cout << "inside optimizer_func::rotation_homography_bundle_adjustment()" << endl;
       string banner="Performing rotation homography bundle adjustment:";
       outputfunc::write_banner(banner);

       initialize_bundle_adjustment();
       optimizer_ptr=o_ptr;
       photogroup_ptr=optimizer_ptr->get_photogroup_ptr();
       FeaturesGroup_ptr=optimizer_ptr->get_FeaturesGroup_ptr();

       n_prev_composited_photos=n_previously_composited_photos;
       n_photos=indices_of_photos_to_be_composited.size();
       n_params_per_photo=optimizer_ptr->get_n_params_per_photo();
       n_camera_trans_params=0;
//         n_camera_trans_params=3;
       n_params=n_photos*n_params_per_photo+n_camera_trans_params;

       cout << "n_photos = " << n_photos << endl;
       cout << "n_params_per_photo = " << n_params_per_photo << endl;
       cout << "n_camera_trans_params = " << n_camera_trans_params << endl;
       cout << "n_params = " << n_params << endl;

       delete camera_params_ptr;
       camera_params_ptr=new double[n_params];

// Copy n_params parameters from photogroup cameras into namespace
// double array *camera_params_ptr:

       photo_indices.clear();
       for (int p=0; p<n_photos; p++)
       {
          photo_indices.push_back(indices_of_photos_to_be_composited[p]);
//            cout << "p = " << p << " photo_indices[p] = " << photo_indices[p]
//                 << endl;
       }
       copy_params_in_to_buffer(photo_indices);

       double x[n_params];
       for(unsigned int i=0; i<n_params; i++) x[i]=0.0;

       int m=n_params;
       int n=m;

//         int itmax=100;				
//         int itmax=1000;
//         int itmax=2001;				
       int itmax=4001;			       
//         int itmax=10000;				
//         int itmax=50000;				

// Perform bundle adjustment using numerical differentiation:

       int ret;
//         cout << "photo_pair_bundle_adjust_flag = "
//              << photo_pair_bundle_adjust_flag << endl;

       if (photo_pair_bundle_adjust_flag)
       {
          ret=dlevmar_dif(
             &(optimizer_func::single_pair_homography_decomposition_score),
             camera_params_ptr, x, m, n, itmax, opts, ba_info, 
             NULL, NULL, NULL);  
       }
       else
       {
          ret=dlevmar_dif(
             &(optimizer_func::symmetric_homography_decomposition_score),
             camera_params_ptr, x, m, n, itmax, opts, ba_info, 
             NULL, NULL, NULL);  
       }

// Perform bundle adjustment using closed-form jacobian information:

//   int ret=dlevmar_der(
//      &(optimizer_func::homography_decomposition_score),
//      &(optimizer_func::homography_decomposition_score_jacobian),
//      p, x, m, n, itmax, opts, ba_info, NULL, NULL, NULL); 

       finalize_bundle_adjustment(ret);

// Copy n_params parameters from of namespace double array
// *camera_params_ptr back onto photogroup cameras:

       copy_params_out_of_buffer(photo_indices);
    }

// --------------------------------------------------------------------------
// Method finalize_bundle_adjustment

    void finalize_bundle_adjustment(int ret)
    {
       cout << "========================================================="
            << endl;
       cout << "Results for score function minimization" << endl;
       cout << "Levenberg-Marquadt returned " << ret
            << " in " << ba_info[5] << " iter, reason = " 
            << ba_info[6] << endl;

       cout << "Minimization information:" << endl;

       cout << "||e||_2 at initial p = " << ba_info[0] << endl;
       cout << "||e||_2 at estimated p = " << ba_info[1] << endl;
       cout << "||J^T e||_inf at estimated p = " << ba_info[2] << endl;
       cout << "||Dp||_2 at estimated p = " << ba_info[3] << endl;
       cout << "/mu/max[J^T J]_ii at estimated p = " << ba_info[4] << endl;
       cout << "Number of iterations = " << ba_info[5] << endl;
       cout << "Number of function evaluations = " << ba_info[7] << endl;
       cout << "Number of Jacobian evaluations = " << ba_info[8] << endl;

       cout << "Termination reason:" << endl;
       int termination(ba_info[6]);
       if (termination==1)
       {
          cout << "Stopped by small gradient J^T e" << endl;
       }
       else if (termination==2)
       {
          cout << "Stopped by small Dp" << endl;
       }
       else if (termination==3)
       {
          cout << "Stopped by itmax" << endl;
       }
       else if (termination==4)
       {
          cout << "Singular matrix.  Restart from current p with increased mu" 
               << endl;
       }
       else if (termination==5)
       {
          cout << "No further error reduction is possible.  Restart with increased mu" << endl;
       }
       else if (termination==6)
       {
          cout << "Stopped by smal ||e||_2" << endl;
       }
       else if (termination==7)
       {
          cout << "Stopped by invalid (e.g. NaN or Inf) func values" 
               << endl;
       }

// ba_info[0] = ||e||_2 at initial p.
// ba_info[1-4]=[ ||e||_2, ||J^T e||_inf,  ||Dp||_2, \mu/max[J^T J]_ii ], all computed at estimated p.

// ba_info[5]= # iterations,
// ba_info[6]=reason for terminating:
//   1 - stopped by small gradient J^T e
//   2 - stopped by small Dp
//   3 - stopped by itmax
//   4 - singular matrix. Restart from current p with increased \mu 
//   5 - no further error reduction is possible. Restart with increased mu
//   6 - stopped by small ||e||_2

//   7 - stopped by invalid (i.e. NaN or Inf) "func" values. This is a
//       user error

// info[7]= # function evaluations
// info[8]= # Jacobian evaluations

       cout << endl;
    }
   
// --------------------------------------------------------------------------
// Method copy_params_in_to_buffer copies parameters for
// photo_indices.size() photos from photogroup's cameras onto
// optimizer_funcs namespace double array *camera_params_ptr.

    void copy_params_in_to_buffer(const vector<int>& photo_indices)
    {
//         cout << "inside optimizer_func::copy_params_in_to_buffer()" << endl;
       n_params_per_photo=optimizer_ptr->get_n_params_per_photo();
       unsigned int curr_n_photos=photo_indices.size();
//         int curr_n_params=curr_n_photos*n_params_per_photo;

//         cout << "n_params_per_photo = " << n_params_per_photo << endl;
//         cout << "curr_n_photos = " << curr_n_photos << endl;
//         cout << "curr_n_params = " << curr_n_params << endl;

       for (unsigned int p=0; p<curr_n_photos; p++)
       {
          photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(
             photo_indices[p]);
          camera* camera_ptr=photograph_ptr->get_camera_ptr();
            
          camera_params_ptr[p*n_params_per_photo+0]=
             camera_ptr->get_rel_az();
          camera_params_ptr[p*n_params_per_photo+1]=
             camera_ptr->get_rel_el();
          camera_params_ptr[p*n_params_per_photo+2]=
             camera_ptr->get_rel_roll();
          camera_params_ptr[p*n_params_per_photo+3]=
             camera_ptr->get_fu();

//            int curr_index=p*n_params_per_photo;
//            cout << "curr_index = " << curr_index+0
//                 << " rel_az = " << camera_params_ptr[curr_index+0]*180/PI
//                 << endl;
//            cout << "curr_index = " << curr_index+1
//                 << " rel_el = " << camera_params_ptr[curr_index+1]*180/PI
//                 << endl;
       } // loop over index p labeling current photos

// Recall last 3 parameters within *camera_params_ptr hold common
// camera world position:
 
//         cout << "n_camera_trans_params = " << n_camera_trans_params << endl;
       if (n_camera_trans_params==3)
       {
          photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(0);
          camera* camera_ptr=photograph_ptr->get_camera_ptr();

          threevector camera_posn=camera_ptr->get_world_posn();
          camera_params_ptr[n_params-3]=camera_posn.get(0);
          camera_params_ptr[n_params-2]=camera_posn.get(1);
          camera_params_ptr[n_params-1]=camera_posn.get(2);
       } // n_camera_trans_params==3 conditional
    }

// --------------------------------------------------------------------------
// Method copy_params_out_of_buffer copies parameters for
// photo_indices.size() photos from namespace double array
// *camera_params_ptr back into photogroup's cameras.

    void copy_params_out_of_buffer(const vector<int>& photo_indices)
    {
       n_params_per_photo=optimizer_ptr->get_n_params_per_photo();
       unsigned int curr_n_photos=photo_indices.size();

       for (unsigned int p=0; p<curr_n_photos; p++)
       {
          photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(
             photo_indices[p]);
          camera* camera_ptr=photograph_ptr->get_camera_ptr();
          camera_ptr->set_rel_az(
             camera_params_ptr[p*n_params_per_photo+0]);
          camera_ptr->set_rel_el(
             camera_params_ptr[p*n_params_per_photo+1]);
          camera_ptr->set_rel_roll(
             camera_params_ptr[p*n_params_per_photo+2]);
          camera_ptr->set_fu(
             camera_params_ptr[p*n_params_per_photo+3]);
       } // loop over index p

// Recall last 3 parameters within *camera_params_ptr hold common
// camera world position:

       if (n_camera_trans_params==3)
       {
          threevector camera_posn;
          camera_posn.put(0,camera_params_ptr[n_params-3]);
          camera_posn.put(1,camera_params_ptr[n_params-2]);
          camera_posn.put(2,camera_params_ptr[n_params-1]);

          for (int p=0; p<n_photos; p++)
          {
             photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(
                photo_indices[p]);
             camera* camera_ptr=photograph_ptr->get_camera_ptr();
             camera_ptr->set_world_posn(camera_posn);
          } // loop over index p
       } // n_camera_trans_params==3 conditional
    }
   
// ==========================================================================
// Rotation homography decomposition methods
// ==========================================================================

// Method symmetric_homography_decomposition_score() implements the
// score function described by Brown and Lowe in their auto-panorama
// paper which treats every photo input into the composite
// symmetrically.

    void symmetric_homography_decomposition_score(
       double* input_param, double* x, int m, int n, void* data)
    {
//         cout << "inside optimizer_func::symmetric_homography_decomposition_score()" 
//              << endl;

       score_evaluation_counter++;

       double score=0;
       double external_score=0;
//         double external_score=1;
         
       genmatrix H(3,3);
       for (unsigned int i=0; i<photo_indices.size(); i++)
       {
          int p=photo_indices[i];
          set_camera_params(
             p,input_param,Rp_ptr,Rp_inv_ptr,Kp_ptr,Kp_inv_ptr);

          int n_qvalues=1;
          int q_start=0;
          int q_stop=1;
          if (n_prev_composited_photos >= 2)
          {
             q_stop=n_prev_composited_photos;
             const int max_n_qvalues=4;
             n_qvalues=basic_math::min(
                n_prev_composited_photos,max_n_qvalues);
          }

//            cout << "p = " << p 
//                 << " start_photo_index = " << starting_photo_index
//                 << " stop_photo_index = " << stopping_photo_index << endl;
//            cout << "n_prev_composited_photos = " 
//                 << n_prev_composited_photos
//                 << " n_qvalues = " << n_qvalues 
//                 << endl;
            
          vector<int> q_values=FeaturesGroup_ptr->photo_feature_overlap(
             p,q_start,q_stop,n_qvalues);

          for (unsigned int j=0; j<q_values.size(); j++)
          {
             int q=q_values[j];
//               cout << "p = " << p << " j = " << j << " q = " << q << endl;

             set_camera_params(
                q,input_param,Rq_ptr,Rq_inv_ptr,Kq_ptr,Kq_inv_ptr);

             double prefactor=get_prefactor(p,q);
             H=prefactor * (*Kq_ptr) * (*Rq_ptr) * (*Rp_inv_ptr) 
                * (*Kp_inv_ptr);

             score += optimizer_ptr->homography_error(p,q,H);

//               cout << "prefactor = " << prefactor << endl;
//               cout << "H = " << H << endl;
//               cout << "score = " << score << endl;

          } // loop over index j labeling matching photo q indices

// Compute discrepancy between projections of manually extracted 3D
// XYZ points onto 2D image planes and manually extracted 2D UV
// counterparts:

          if (optimizer_ptr->get_fit_external_params_flag())
          {
             external_score += optimizer_ptr->projection_error(p);
          }

       } // loop over index i labeling photo p indices

/*
  double beta=0;
  if (optimizer_ptr->get_fit_external_params_flag())
  {
//            score *= external_score;
//            score += external_score;
//            score += 100*external_score;
//            score += 1000*external_score;
//            score += 10000*external_score;
//            beta=0.1;
beta=1.0;
}
*/
       double total_score=score + 
          optimizer_ptr->get_external_params_weight()*external_score;
//         double total_score=score*external_score;

       if (score_evaluation_counter%100==0)
       {
          cout << "score = " << score 
               << " ext score = " << external_score 
               << " tot score = " << total_score
               << endl;
       }

       x[0]=total_score;

       for (int i=1; i<n; i++)
       {
          x[i]=x[0];
       } 

    } 

// --------------------------------------------------------------------------
    void single_pair_homography_decomposition_score(
       double* input_param, double* x, int m, int n, void* data)
    {
//         cout << "inside optimizer_func::single_pair_homography_decomposition_score()" 
//              << endl;

       if (photo_indices.size() != 2)
       {
          cout << "Error in optimizer_func:single_pair_homography_decomposition_score()" << endl;
       }

       score_evaluation_counter++;

       double score=0;
       genmatrix H(3,3);

       for (unsigned int i=0; i<photo_indices.size(); i++)
       {
          int p=photo_indices[0];
          int q=photo_indices[1];

          if (i==1)
          {
             p=photo_indices[1];
             q=photo_indices[0];
          }

//            cout << "p = " << p << " q = " << q << endl;

          set_camera_params(
             p,input_param,Rp_ptr,Rp_inv_ptr,Kp_ptr,Kp_inv_ptr);
          set_camera_params(
             q,input_param,Rq_ptr,Rq_inv_ptr,Kq_ptr,Kq_inv_ptr);

          double prefactor=get_prefactor(p,q);
          H=prefactor * (*Kq_ptr) * (*Rq_ptr) * (*Rp_inv_ptr) 
             * (*Kp_inv_ptr);

          score += optimizer_ptr->homography_error(p,q,H);
               
//            cout << "prefactor = " << prefactor << endl;
//            cout << "H = " << H << endl;
//            cout << "score = " << score << endl;

       } // loop over index i labeling photo_indices

// Multiply score by prefactor which quantifies discrepancy between
// projections of manually extracted 3D XYZ points onto 2D image
// planes and manually extracted 2D UV counterparts:

//         double beta=0;
       double external_score=0;
//         double external_score=1;
       if (optimizer_ptr->get_fit_external_params_flag())
       {
          external_score += optimizer_ptr->projection_error(
             photo_indices[0]);
          external_score += optimizer_ptr->projection_error(
             photo_indices[1]);

//            score *= external_score;
//            score += external_score;
//            score += 100*external_score;
//            score += 1000*external_score;
//            score += 10000*external_score;
//            beta=0.1;
//            beta=1.0;
       }

       double total_score=score + 
          optimizer_ptr->get_external_params_weight()*external_score;
//         double total_score=score + beta*external_score;
//         double total_score=score*external_score;

       if (score_evaluation_counter%100==0)
       {
          cout << "score = " << score 
               << " ext score = " << external_score 
               << " tot score = " << total_score
               << endl;
       }

       x[0]=total_score;
       for (int i=1; i<n; i++)
       {
          x[i]=x[0];
       } 
    } 

// --------------------------------------------------------------------------
// Method set_camera_params takes in integer p which labels some
// photograph along with double array input_param[] which holds all
// independent parameters (i.e. fu,u0,v0,az,el,roll for each image).
// This method constructs a camera from the pth image's parameters,
// forms its extrinsic K and intrinsic R matrices, and returns
// R, Rinv, K and Kinv.

    void set_camera_params(
       int p,double* input_param,rotation* R_ptr,rotation* Rinv_ptr,
       genmatrix* K_ptr,genmatrix* Kinv_ptr)
    {
//         cout << "inside optimizer_func::set_camera_params()" << endl;

       photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(p);
       camera* camera_ptr=photograph_ptr->get_camera_ptr();

       bool p_inside_photo_indices_flag=false;
       int p_shifted=-1;
       for (unsigned int i=0; i<photo_indices.size(); i++)
       {
          if (photo_indices[i]==p)
          {
             p_inside_photo_indices_flag=true;
             p_shifted=i;
             break;
          }
       } // loop over index i labeling photo_indices[]

       double az_p,el_p,roll_p,fu_p;
       if (p_inside_photo_indices_flag)
       {
          az_p=input_param[n_params_per_photo*p_shifted+0];
          el_p=input_param[n_params_per_photo*p_shifted+1];
          roll_p=input_param[n_params_per_photo*p_shifted+2];
          fu_p=input_param[n_params_per_photo*p_shifted+3];

//            cout << "az_p = " << az_p << " el_p = " << el_p
//                 << " roll_p = " << roll_p << endl;

// Reset camera params corresponding to photograph indexed by integer
// p within *optimizer_ptr based upon current values in variable
// parameter array input_param:

          camera_ptr->set_fu(fu_p);
          camera_ptr->set_rel_az(az_p);
          camera_ptr->set_rel_el(el_p);
          camera_ptr->set_rel_roll(roll_p);
       }
       else
       {
          fu_p=camera_ptr->get_fu();
          az_p=camera_ptr->get_rel_az();
          el_p=camera_ptr->get_rel_el();
          roll_p=camera_ptr->get_rel_roll();
       } // photo index p lies in interval 
       //    [starting_photo_index,stopping_photo_index] conditional
         
       if (fu_p > 0)
       {
          cout << "***************  TROUBLE   ****************" << endl;
       }

// Reset focal length and rotation parameters for pth photograph's
// camera:

       camera_ptr->set_f(fu_p);
       camera_ptr->construct_internal_parameter_K_matrix();

       *K_ptr=*(camera_ptr->get_K_ptr());
       K_ptr->inverse(*Kinv_ptr);

       camera_ptr->set_Rcamera(az_p,el_p,roll_p);
       *R_ptr=*(camera_ptr->get_Rcamera_ptr());
       *Rinv_ptr=R_ptr->transpose();
//         cout << "*R_ptr = " << *R_ptr << endl;

// Recall last 3 parameters within input_param[] hold common camera
// world position.  Reset position for pth photograph's camera:

       if (n_camera_trans_params==3)
       {
          threevector camera_posn(
             input_param[n_params-3],
             input_param[n_params-2],
             input_param[n_params-1]);
          camera_ptr->set_world_posn(camera_posn);

          bool recompute_internal_params_flag=false;
          camera_ptr->construct_projection_matrix(
             recompute_internal_params_flag);
       }

    }

// --------------------------------------------------------------------------
// Method get_prefactor returns (fu_p/fu_q)^(2/3).

    double get_prefactor(int p,int q)
    {
//         cout << "inside optimizer_func::get_prefactor()" << endl;
//         cout << "p = " << p << " q = " << q << endl;

       double fu_p=photogroup_ptr->get_photograph_ptr(p)->get_camera_ptr()->
          get_fu();
       double fu_q=photogroup_ptr->get_photograph_ptr(q)->get_camera_ptr()->
          get_fu();
//         cout << "fu_p = " << fu_p << " fu_q = " << fu_q << endl;
       double prefactor_pq=pow(fu_p/fu_q,0.666666);
       return prefactor_pq;
    }

// --------------------------------------------------------------------------
// Method homography_decomposition_score_jacobian computes
// dhomography_decomposition_score/dx where x ranges over all
// independent input parameters.  Recall input parameter m = number of
// independent variable parameters.  Input parameter n should equal
// number of output score functions.  But in order for Lourakis'
// Levenberg-Marquadt routines to work, they require that n be at
// least as large as m.  So n = m in practice.

    void homography_decomposition_score_jacobian(
       double* input_param, double* jacobian, int m, int n, void* data)
    {
//         cout << "inside homography_decomposition_score_jacobian()" << endl;
//         cout << "m = " << m << " n_params = " << n_params << endl;

       genmatrix Delta(3,3);

       for (unsigned int indep_param_index=0; indep_param_index < n_params;
            indep_param_index++)
       {
          double curr_jacobian=0;

          for (int p=0; p<n_photos; p++)
          {
             for (int q=p+1; q<n_photos; q++)
             {
//                  int r=indep_param_index/6;	// labels current photo
//                  int s=indep_param_index%6;	// labels curr camera param

                bool print_flag=false;
                compute_Gren(p,q,input_param,print_flag);
                     
                homography* H_ren_ptr=get_homography_ptr(p,q);

//               cout << "G_ren = " << G_ren << endl;
//               cout << "H_ren = " << *(H_ren_ptr->get_H_ptr()) << endl;
//               cout << "------------------------------------------" << endl;
               
                Delta=*G_ren_ptr-( *(H_ren_ptr->get_H_ptr()) );
                genmatrix* dGren_dparam_ptr= 
                   Gren_derivative(p,q,indep_param_index,input_param);
                if (dGren_dparam_ptr==NULL) continue;

/*
// Numerically evaluate dGren(p,q)/d_param:

genmatrix G_ren(3,3),Gdelta_ren(3,3),numer_deriv(3,3);
G_ren=*G_ren_ptr;

const double delta=0.0001;
double curr_param=input_param[indep_param_index];
input_param[indep_param_index]=curr_param+delta;
                  
compute_Gren(p,q,input_param,print_flag);
Gdelta_ren=*G_ren_ptr;
numer_deriv=(Gdelta_ren-G_ren)/delta;

cout << "p = " << p << " q = " << q 
<< " indep_param_index = " << indep_param_index 
<< " r = curr photo = " << r
<< " s = cam param = " << s
<< endl;

cout << "numer_deriv = " << numer_deriv << endl;
cout << "*dGren_dparam_ptr = " << *dGren_dparam_ptr << endl;
cout << "=================================================="
<< endl;

input_param[indep_param_index]=curr_param;
*/

                curr_jacobian += 2 *
                   ( Delta.transpose() * (*dGren_dparam_ptr) ).trace();

             } // loop over index q labeling photos
          } // loop over index p labeling photos

          jacobian[indep_param_index]=curr_jacobian;
//            cout << "indep_param_index = " << indep_param_index
//                 << " jacobian = " << jacobian[indep_param_index]
//                 << endl;
         
       } // loop over index_param_index 
         
       for (int i=1; i<n; i++)
       {
          for (unsigned int indep_param_index=0; indep_param_index<n_params; 
               indep_param_index++)
          {
             jacobian[i*n_params+indep_param_index]=
                jacobian[indep_param_index];
          }
       }

    }
 
// ==========================================================================
// Rotation homography decomposition derivative methods
// ==========================================================================

// Method compute_Gren

    void compute_Gren(int p,int q,double* input_param,bool print_flag)
    {
       set_camera_params(
          p,input_param,Rp_ptr,Rp_inv_ptr,Kp_ptr,Kp_inv_ptr);

       set_camera_params(
          q,input_param,Rq_ptr,Rq_inv_ptr,Kq_ptr,Kq_inv_ptr);

       double prefactor=get_prefactor(p,q);
       *G_ren_ptr=prefactor * (*Kq_ptr) * (*Rq_ptr) * 
          (*Rp_inv_ptr) * (*Kp_inv_ptr);

       if (print_flag)
       {
//            cout << "inside compute_Gren(), p = " << p 
//                 << " q = " << q << " prefactor = " << prefactor << endl;
//            cout << "*Kq_ptr = " << *Kq_ptr << endl;
//            cout << " *Rq_ptr = " << *Rq_ptr << endl;
//            cout << "*Rp_inv_ptr = " << *Rp_inv_ptr << endl;
//            cout << " *Kp_inv_ptr = " << *Kp_inv_ptr << endl;
//            cout << "*Gren_ptr = " << *G_ren_ptr << endl;
       }
         
    }

// --------------------------------------------------------------------------
// On 11/6/08, we remembered the hard and painful way that that the
// camera.set_Rcamera() member function introduces a preliminary R0
// rotation prior to calling rotation_from_az_el_roll().  So when we
// compute camera rotation derivatives wrt azimuth, elevation and
// roll, we must include R0 in the final result!

    void initialize_R0()
    {
       R0_ptr=new rotation;
       R0_ptr->clear_values();
       R0_ptr->put(0,2,-1);
       R0_ptr->put(1,0,-1);
       R0_ptr->put(2,1,1);
    }

// --------------------------------------------------------------------------
// r = curr_param_index/6 = current photo index

// Index 0 <= i <= n_params labels all independent parameter variables
// for all photos.  

// Index r = i/6 labels current photo

// Index s = i%6 labels current parameter

    genmatrix* Gren_derivative(int p,int q,int indep_param_index,
                               double* input_param)
    {
//         cout << "inside Gren_derivative(), p = " << p << " q = " << q
//              << " indep_param_index = " << indep_param_index << endl;
         
       int r=indep_param_index/6;	// labels current photo
       if ((r != p) && (r != q)) return NULL;
         
       int s=indep_param_index%6;	// labels current camera parameter
//         cout << "r = " << r << " s = " << s << endl;

       double prefactor_pq=get_prefactor(p,q);

       double az_p=input_param[n_params_per_photo*p+0];
       double el_p=input_param[n_params_per_photo*p+1];
       double roll_p=input_param[n_params_per_photo*p+2];
       double fu_p=input_param[n_params_per_photo*p+3];
         
       double az_q=input_param[n_params_per_photo*q+0];
       double el_q=input_param[n_params_per_photo*q+1];
       double roll_q=input_param[n_params_per_photo*q+2];
       double fu_q=input_param[n_params_per_photo*q+3];

       double u0_p=0.666;
       double v0_p=0.5;
//         double u0_q=0.6666;
//         double v0_q=0.5;

       if (n_params_per_photo==6)
       {
          u0_p=input_param[n_params_per_photo*p+4];
          v0_p=input_param[n_params_per_photo*p+5];
//            u0_q=input_param[n_params_per_photo*q+4];
//            v0_q=input_param[n_params_per_photo*q+5];
       }
	
       if (s==0)
       {
          return dGren_df(p,q,r,prefactor_pq,fu_p,fu_q,u0_p,v0_p);
       }
       else if (s==1)
       {
          return dGren_du0(p,q,r,prefactor_pq,fu_p);
       }
       else if (s==2)
       {
          return dGren_dv0(p,q,r,prefactor_pq,fu_p);
       }
       else if (s==3)
       {
          return dGren_daz(p,q,r,prefactor_pq,
                           az_p,el_p,roll_p,az_q,el_q,roll_q);
       }
       else if (s==4)
       {
          return dGren_del(p,q,r,prefactor_pq,
                           az_p,el_p,roll_p,az_q,el_q,roll_q);
       }
       else if (s==5)
       {
          return dGren_droll(p,q,r,prefactor_pq,
                             az_p,el_p,roll_p,az_q,el_q,roll_q);
       }
       else
       {
          return NULL;
       }
    }
  
// --------------------------------------------------------------------------

    genmatrix* dGren_df(int p,int q,int r,
                        double prefactor_pq,double fu_p,double fu_q,
                        double u0_p,double v0_p)
    {
//         cout << "inside dGren_df, p = " << p << " q = " << q
//              << " r = " << r << endl;
//         cout << "prefactor_pq = " << prefactor_pq << endl;
//         cout << "*Kq_ptr = " << *Kq_ptr << endl;
//         cout << "*Rq_ptr = " << *Rq_ptr << endl;
//         cout << "*Rp_inv_ptr = " << *Rp_inv_ptr << endl;
//         cout << "*Kp_inv_ptr = " << *Kp_inv_ptr << endl;
       derivative_term1_ptr->clear_values();
       *derivative_term1_ptr = dprefactor_df(p,q,r,fu_p,fu_q) * 
          (*Kq_ptr)*(*Rq_ptr)*(*Rp_inv_ptr)*(*Kp_inv_ptr);
       *derivative_term1_ptr += prefactor_pq * (*dK_df(q,r)) * (*Rq_ptr) *
          (*Rp_inv_ptr) * (*Kp_inv_ptr);
       *derivative_term1_ptr += prefactor_pq * (*Kq_ptr) * (*Rq_ptr) *
          (*Rp_inv_ptr) * (*dKinv_df(p,r,fu_p,u0_p,v0_p));
       return derivative_term1_ptr;
    }

    genmatrix* dGren_du0(int p,int q,int r,double prefactor_pq,double fu_p)
    {
       derivative_term2_ptr->clear_values();
       *derivative_term2_ptr = prefactor_pq * (*dK_du0(q,r)) * (*Rq_ptr) *
          (*Rp_inv_ptr) * (*Kp_inv_ptr);
       *derivative_term2_ptr += prefactor_pq * (*Kq_ptr) * (*Rq_ptr) *
          (*Rp_inv_ptr) * (*dKinv_du0(p,r,fu_p));
       return derivative_term2_ptr;
    }

    genmatrix* dGren_dv0(int p,int q,int r,double prefactor_pq,double fu_p)
    {
       derivative_term3_ptr->clear_values();
       *derivative_term3_ptr = prefactor_pq * (*dK_dv0(q,r)) * (*Rq_ptr) *
          (*Rp_inv_ptr) * (*Kp_inv_ptr);
       *derivative_term3_ptr += prefactor_pq * (*Kq_ptr) * (*Rq_ptr) *
          (*Rp_inv_ptr) * (*dKinv_dv0(p,r,fu_p));
       return derivative_term3_ptr;
    }

    genmatrix* dGren_daz(int p,int q,int r,double prefactor_pq,
                         double az_p,double el_p,double roll_p,
                         double az_q,double el_q,double roll_q)
    {
//         cout << "inside optimizer_func::dGren_daz()" << endl;
//         cout << "az_p = " << az_p << " az_q = " << az_q << endl;

       derivative_term4_ptr->clear_values();
       if (q==r)
       {
          *derivative_term4_ptr = prefactor_pq * (*Kq_ptr) * 
             R0_ptr->transpose() * 
             (Rq_ptr->dRdaz(az_q,el_q,roll_q)).transpose() *
             (*Rp_inv_ptr) * (*Kp_inv_ptr);
       }
       else if (p==r)
       {
          *derivative_term4_ptr = prefactor_pq * (*Kq_ptr) * 
             (*Rq_ptr) *
             Rp_ptr->dRdaz(az_p,el_p,roll_p) * (*R0_ptr) * 
             (*Kp_inv_ptr);
       }
         
       return derivative_term4_ptr;
    }

    genmatrix* dGren_del(int p,int q,int r,double prefactor_pq,
                         double az_p,double el_p,double roll_p,
                         double az_q,double el_q,double roll_q)
    {
       derivative_term5_ptr->clear_values();
       if (q==r)
       {
          *derivative_term5_ptr = prefactor_pq * (*Kq_ptr) * 
             R0_ptr->transpose() * 
             (Rq_ptr->dRdel(az_q,el_q,roll_q)).transpose() *
             (*Rp_inv_ptr) * (*Kp_inv_ptr);
       }
       else if (p==r)
       {
          *derivative_term5_ptr = prefactor_pq * (*Kq_ptr) * (*Rq_ptr) *
             (Rp_ptr->dRdel(az_p,el_p,roll_p)) * (*R0_ptr) * 
             (*Kp_inv_ptr);
       }
         
       return derivative_term5_ptr;
    }

    genmatrix* dGren_droll(int p,int q,int r,double prefactor_pq,
                           double az_p,double el_p,double roll_p,
                           double az_q,double el_q,double roll_q)
    {
//         cout << "inside optimizer_func::dGren_droll" << endl;
//         cout << "p = " << p << " q = " << q << " r = " << r << endl;
//         cout << "*Rp_ptr = " << *Rp_ptr << endl;
//         cout << "Rp_ptr->dRdroll(az_p,el_p,roll_p) = "
//              << Rp_ptr->dRdroll(az_p,el_p,roll_p) << endl;

//         cout << "Rq_ptr->dRdroll(az_q,el_q,roll_q) = "
//              << Rq_ptr->dRdroll(az_q,el_q,roll_q) << endl;
//         cout << "Rp_ptr->dRdroll(az_p,el_p,roll_p).transpose() = "
//              << (Rp_ptr->dRdroll(az_p,el_p,roll_p)).transpose() << endl;

       derivative_term6_ptr->clear_values();
       if (q==r)
       {
          *derivative_term6_ptr = prefactor_pq * (*Kq_ptr) * 
             R0_ptr->transpose() * 
             (Rq_ptr->dRdroll(az_q,el_q,roll_q)).transpose() *
             (*Rp_inv_ptr) * (*Kp_inv_ptr);
       }
       else if (p==r)
       {
          *derivative_term6_ptr = prefactor_pq * (*Kq_ptr) * (*Rq_ptr) *
             (Rp_ptr->dRdroll(az_p,el_p,roll_p)) * (*R0_ptr) * 
             (*Kp_inv_ptr);
       }
         
//         cout << "*derivative_term6_ptr = " << *derivative_term6_ptr << endl;
       return derivative_term6_ptr;
    }

// --------------------------------------------------------------------------
// Method dprefactor_df computes dprefactor/df_r where
// prefactor=(f_p/f_q)^0.66666

    double dprefactor_df(int p,int q,int r,double fu_p,double fu_q)
    {
//         cout << "inside dprefactor_df(p,q,r)" << endl;
//         cout << "fu_p = " << fu_p << endl;
//         cout << "fu_q = " << fu_q << endl;

// Recall prefactor = (fp/fq)^(2/3) and that both fp and fq are
// negative.  So both dprefactor/dfu_p and dprefactor/dfu_q are
// negative.  But pow(x,y) function must take in a positive x value.
// So we factor out an overall negative sign and store it within the
// following local sgn variable:

       int sgn=-1;	
       double derivative=0;
       if (r==p)
       {
          derivative=0.66666*pow(fabs(fu_p)*sqr(fu_q),-0.33333);
       }
       else if (r==q)
       {
          derivative=-0.66666*pow(fabs(fu_p),0.66666)*
             pow(fabs(fu_q),-1.666666);
       }
//         cout << "derivative = " << derivative << endl;
       return sgn*derivative;
    }
   
// --------------------------------------------------------------------------

    genmatrix* dK_df(int q,int r)
    {
       output_matrix1_ptr->clear_values();
       if (q==r)
       {
          output_matrix1_ptr->put(0,0,1);
          output_matrix1_ptr->put(1,1,1);
       }
       return output_matrix1_ptr;
    }

    genmatrix* dK_du0(int q,int r)
    {
       output_matrix2_ptr->clear_values();
       if (q==r)
       {
          output_matrix2_ptr->put(0,2,1);
       }
       return output_matrix2_ptr;
    }

    genmatrix* dK_dv0(int q,int r)
    {
       output_matrix3_ptr->clear_values();
       if (q==r)
       {
          output_matrix3_ptr->put(1,2,1);
       }
       return output_matrix3_ptr;
    }

// --------------------------------------------------------------------------

    genmatrix* dKinv_df(int p,int r,double fu_p,double u0_p,double v0_p)
    {
       output_matrix4_ptr->clear_values();
       if (p==r)
       {
          output_matrix4_ptr->put(0,0,-1/sqr(fu_p));
          output_matrix4_ptr->put(0,2,u0_p/sqr(fu_p));
          output_matrix4_ptr->put(1,1,-1/sqr(fu_p));
          output_matrix4_ptr->put(1,2,v0_p/sqr(fu_p));
       }
       return output_matrix4_ptr;
    }
   
    genmatrix* dKinv_du0(int p,int r,double fu_p)
    {
       output_matrix5_ptr->clear_values();
       if (p==r)
       {
          output_matrix5_ptr->put(0,2,-1/fu_p);
       }
       return output_matrix5_ptr;
    }
   
    genmatrix* dKinv_dv0(int p,int r,double fu_p)
    {
       output_matrix6_ptr->clear_values();
       if (p==r)
       {
          output_matrix6_ptr->put(1,2,-1/fu_p);
       }
       return output_matrix6_ptr;
    }

// ==========================================================================
// Homography fitting methods
// ==========================================================================

    void compute_symmetric_renormalized_homographies()
    {
       string banner="Computing symmetric renormalized homographies:";
       outputfunc::write_banner(banner);

       for (i_photo=0; i_photo<n_photos; i_photo++)
       {
          for (j_photo=i_photo+1; j_photo<n_photos; j_photo++)
          {
             homography* curr_H_ptr=get_homography_ptr(i_photo,j_photo);
             generate_homography_params_array(curr_H_ptr);
             photo_pair_homography_bundle_adjustment();
             reset_homography_params_array(curr_H_ptr);
               
             curr_H_ptr->enforce_unit_determinant();
             curr_H_ptr->compute_homography_inverse();
            
             cout << "i_photo = " << i_photo 
                  << " j_photo = " << j_photo
                  << " H = " << (*curr_H_ptr) << endl;
//            outputfunc::enter_continue_char();

          } // loop over index j_photo
       } // loop over index i_photo
    }

// --------------------------------------------------------------------------
    int generate_homography_params_array(homography* curr_H_ptr)
    {
//         cout << "inside optimizer_func::generate_homography_params_array()" << endl;

       int counter=0;
       homography_params_ptr[counter++]=curr_H_ptr->get_H_ptr()->get(0,0);
       homography_params_ptr[counter++]=curr_H_ptr->get_H_ptr()->get(0,1);
       homography_params_ptr[counter++]=curr_H_ptr->get_H_ptr()->get(0,2);

       homography_params_ptr[counter++]=curr_H_ptr->get_H_ptr()->get(1,0);
       homography_params_ptr[counter++]=curr_H_ptr->get_H_ptr()->get(1,1);
       homography_params_ptr[counter++]=curr_H_ptr->get_H_ptr()->get(1,2);
         
       homography_params_ptr[counter++]=curr_H_ptr->get_H_ptr()->get(2,0);
       homography_params_ptr[counter++]=curr_H_ptr->get_H_ptr()->get(2,1);
       homography_params_ptr[counter++]=curr_H_ptr->get_H_ptr()->get(2,2);
         
       n_params=9;
//         cout << "n_params = " << n_params << endl;
       return n_params;
    }

// --------------------------------------------------------------------------
// Method photo_pair_homography_bundle_adjustment

    void photo_pair_homography_bundle_adjustment()
    {
       string banner="Performing photo pair homography bundle adjustment:";
       outputfunc::write_banner(banner);

       initialize_bundle_adjustment();

       double* p=get_homography_params_array();

       double x[n_params];
       for(unsigned int i=0; i<n_params; i++) x[i]=0.0;

       int m=n_params;
       int n=m;

       cout << "n_params = " << n_params << endl;

       int itmax=1000;				
//   int itmax=2001;				
//         int itmax=4001;				
//   int itmax=10000;				
//   int itmax=50000;				

// Perform bundle adjustment using numerical differentiation:

       int ret=dlevmar_dif(
          &(optimizer_func::homography_fit_score),
          p, x, m, n, itmax, opts, ba_info, NULL, NULL, NULL);  

// Perform bundle adjustment using closed-form jacobian information:

//   int ret=dlevmar_der(
//      &(optimizer_func::homography_decomposition_score),
//      &(optimizer_func::homography_decomposition_score_jacobian),
//      p, x, m, n, itmax, opts, ba_info, NULL, NULL, NULL); 

       finalize_bundle_adjustment(ret);
    }

// --------------------------------------------------------------------------
    void homography_fit_score(
       double* input_param, double* x, int m, int n, void* data)
    {
//         cout << "inside optimizer_func::homography_fit_score()" << endl;
//         cout << "m = " << m << " n = " << n << endl;

       homography* H_ptr=get_homography_ptr(i_photo,j_photo);
       set_H_and_Hinv(input_param,H_ptr);

       double score=0;
       for (unsigned int f=0; f<n_features; f++)
       {
          twovector XY(u_meas_ptr->get(f,i_photo),
                       v_meas_ptr->get(f,i_photo));
          twovector UV(u_meas_ptr->get(f,j_photo),
                       v_meas_ptr->get(f,j_photo));
          twovector UV_proj(H_ptr->project_world_plane_to_image_plane(XY));
          twovector XY_proj(H_ptr->project_image_plane_to_world_plane(UV));
          score += (UV_proj-UV).sqrd_magnitude()+
             (XY_proj-XY).sqrd_magnitude();
       } // loop over index f labeling inlier features

       score_evaluation_counter++;
       if (score_evaluation_counter%1000==0)
       {
          cout << "score = " << score << endl;
       }
         
       x[0]=score;
       for (int i=1; i<n; i++)
       {
          x[i]=x[0];
       } 
    } 

// --------------------------------------------------------------------------
    void set_H_and_Hinv(double* input_param,homography* H_ptr)
    {
//         cout << "inside optimizer_func::set_H_and_Hinv()" << endl;
       H_ptr->set_H_ptr_element(0,0,input_param[0]);
       H_ptr->set_H_ptr_element(0,1,input_param[1]);
       H_ptr->set_H_ptr_element(0,2,input_param[2]);

       H_ptr->set_H_ptr_element(1,0,input_param[3]);
       H_ptr->set_H_ptr_element(1,1,input_param[4]);
       H_ptr->set_H_ptr_element(1,2,input_param[5]);
         
       H_ptr->set_H_ptr_element(2,0,input_param[6]);
       H_ptr->set_H_ptr_element(2,1,input_param[7]);
       H_ptr->set_H_ptr_element(2,2,input_param[8]);
       H_ptr->compute_homography_inverse();
    }

// --------------------------------------------------------------------------
    void reset_homography_params_array(homography* curr_H_ptr)
    {
//         cout << "inside optimizer_func::generate_homography_params_array()" << endl;

       int counter=0;
       curr_H_ptr->set_H_ptr_element(0,0,homography_params_ptr[counter++]);
       curr_H_ptr->set_H_ptr_element(0,1,homography_params_ptr[counter++]);
       curr_H_ptr->set_H_ptr_element(0,2,homography_params_ptr[counter++]);

       curr_H_ptr->set_H_ptr_element(1,0,homography_params_ptr[counter++]);
       curr_H_ptr->set_H_ptr_element(1,1,homography_params_ptr[counter++]);
       curr_H_ptr->set_H_ptr_element(1,2,homography_params_ptr[counter++]);

       curr_H_ptr->set_H_ptr_element(2,0,homography_params_ptr[counter++]);
       curr_H_ptr->set_H_ptr_element(2,1,homography_params_ptr[counter++]);
       curr_H_ptr->set_H_ptr_element(2,2,homography_params_ptr[counter++]);
    }

// ==========================================================================
// Global photosynth parameter estimation methods
// ==========================================================================

// Method global_photosynth_bundle_adjustment()

    void global_photosynth_bundle_adjustment(
       optimizer* o_ptr)
    {
       string banner="Performing global photosynth bundle adjustment:";
       outputfunc::write_banner(banner);

       initialize_bundle_adjustment();
       optimizer_ptr=o_ptr;
       photogroup_ptr=optimizer_ptr->get_photogroup_ptr();
       bundler_photogroup_ptr=optimizer_ptr->get_bundler_photogroup_ptr();
       FeaturesGroup_ptr=optimizer_ptr->get_FeaturesGroup_ptr();

       n_params_per_photo=0;
       int n_global_trans_params=3;
       int n_global_rot_params=3;
       int n_global_scale_params=1;
       n_params=n_global_trans_params+n_global_rot_params+
          n_global_scale_params;
       cout << "n_params = " << n_params << endl;

       delete camera_params_ptr;
       camera_params_ptr=new double[n_params];
       copy_global_params_in_to_buffer();

       double x[n_params];
       for (unsigned int i=0; i<n_params; i++) x[i]=0.0;

       int m=n_params;
       int n=m;

//         int itmax=100;				
//         int itmax=1000;
//         int itmax=2001;				
       int itmax=4001;			       
//         int itmax=10000;				
//         int itmax=50000;				

// Perform bundle adjustment using numerical differentiation:

       int ret=dlevmar_dif(
          &(optimizer_func::tiepoint_reprojection_score),
          camera_params_ptr, x, m, n, itmax, opts, ba_info, 
          NULL, NULL, NULL);  

       finalize_bundle_adjustment(ret);

// Copy n_params parameters from of namespace double array
// *camera_params_ptr back onto photogroup cameras:

       copy_global_params_out_of_buffer();
    }

// --------------------------------------------------------------------------
// Method copy_global_params_in_to_buffer() copies global translation,
// rotation and scale parameters from photogroup member variables onto
// optimizer_funcs namespace double array *camera_params_ptr.

    void copy_global_params_in_to_buffer()
    {
//        cout << "inside optimizer_func::copy_params_in_to_buffer()" << endl;

// When optimizing global parameters needed to map Bundler to world
// coordinates, recall first 3 parameters in *camera_params_ptr
// correspond to global translation, next 3 parameters correspond to
// global az, el and roll and final parameter corresponds to global
// scale:

       threevector global_trans(
          photogroup_ptr->get_bundler_to_world_translation());
       camera_params_ptr[0]=global_trans.get(0);
       camera_params_ptr[1]=global_trans.get(1);
       camera_params_ptr[2]=global_trans.get(2);

       camera_params_ptr[3]=photogroup_ptr->get_bundler_to_world_az();
       camera_params_ptr[4]=photogroup_ptr->get_bundler_to_world_el();
       camera_params_ptr[5]=photogroup_ptr->get_bundler_to_world_roll();

       camera_params_ptr[6]=photogroup_ptr->
          get_bundler_to_world_scalefactor();
    }

// --------------------------------------------------------------------------
// Method copy_global_params_out_of_buffer() copies global
// translation, rotation and scale parameters from namespace double
// array *camera_params_ptr back into photogroup member variables

    void copy_global_params_out_of_buffer()
    {
       threevector global_trans(
          camera_params_ptr[0],camera_params_ptr[1],
          camera_params_ptr[2]);
       photogroup_ptr->set_bundler_to_world_translation(global_trans);
       photogroup_ptr->set_bundler_to_world_az(camera_params_ptr[3]);
       photogroup_ptr->set_bundler_to_world_el(camera_params_ptr[4]);
       photogroup_ptr->set_bundler_to_world_roll(camera_params_ptr[5]);
       photogroup_ptr->set_bundler_to_world_scalefactor(
          camera_params_ptr[6]);
    }

// --------------------------------------------------------------------------
// Method tiepoint_reprojection_score() 

    void tiepoint_reprojection_score(
       double* input_param, double* x, int m, int n, void* data)
    {
//         cout << "inside optimizer_func::tiepoint_reprojection_score()" 
//              << endl;

       score_evaluation_counter++;
       double score=0;

       for (unsigned int k=0; k<photogroup_ptr->get_n_photos(); k++)
       {
//            cout << "k = " << k << endl;
          photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(k);
          camera* camera_ptr=photograph_ptr->get_camera_ptr();

          photograph* bundler_photograph_ptr=bundler_photogroup_ptr->
             get_photograph_ptr(k);
          *camera_ptr=*(bundler_photograph_ptr->get_camera_ptr());

// FAKE FAKE:  thurs, May 28, 2009 at 6:20 pm
// Hardcode bundler rotation origin for NYC example.
// need to pass this threevector as variable parameter in future...

          threevector bundler_rotation_origin(
             583299.023754546 , 4506310.19790909 , 101.864543137272);
          // NYC feature COM


          camera_ptr->convert_bundler_to_world_coords(
             input_param[0],input_param[1],input_param[2],
             bundler_rotation_origin,
             input_param[3],input_param[4],input_param[5],
             input_param[6]);

          camera_ptr->construct_projection_matrix_for_fixed_K();
//            cout << " *P_ptr = " 
//                 << *(camera_ptr->get_P_ptr()) << endl;
                           
          double avg_residual=optimizer_ptr->projection_error(n);
//            cout << "avg_residual = " << avg_residual << endl;
          if (finite(avg_residual) != 0)
          {
             score += 1000*avg_residual;
          }
          else
          {
             score += POSITIVEINFINITY;
             break;
          }
       } // loop over index k labeling input photos

       if (score_evaluation_counter%100==0)
       {
          cout << "score = " << score << endl;
       }

       x[0]=score;
       for (int i=1; i<n; i++)
       {
          x[i]=x[0];
       } 

    } 


} // optimizer_func namespace





