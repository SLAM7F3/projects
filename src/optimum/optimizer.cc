// ==========================================================================
// Optimizer class member function definitions
// ==========================================================================
// Last modified on 1/17/11; 2/26/11; 6/4/11; 4/25/13
// ==========================================================================

#include <iostream>
#include "math/basic_math.h"
#include "osg/osgFeatures/FeaturesGroup.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "geometry/homography.h"
#include "numrec/nrfuncs.h"
#include "optimum/optimizer.h"
#include "optimum/optimizer_funcs.h"
#include "general/outputfuncs.h"
#include "math/rotation.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void optimizer::allocate_member_objects()
{
   R0_ptr=new rotation;
}		       

void optimizer::initialize_member_objects()
{
   fit_external_params_flag=false;
   
   n_params_per_photo=4;	// 3 angles, 1 focal param
//   n_params_per_photo=7;	// 3 angles, 1 focal param, 3 trans
   n_photos=-1;
//    camera_params_filename="camera_params.txt";

   sigma=1000;
   sqr_sigma=sqr(sigma);

   u_meas_ptr=NULL;
   v_meas_ptr=NULL;
   u_manual_ptr=NULL;
   v_manual_ptr=NULL;
   homography_ptrs_ptr=NULL;
   FeaturesGroup_ptr=NULL;

   R0_ptr->clear_values();
   R0_ptr->put(0,2,-1);
   R0_ptr->put(1,0,-1);
   R0_ptr->put(2,1,1);

   bundler_photogroup_ptr=NULL;
}

optimizer::optimizer(void)
{
   allocate_member_objects();
   initialize_member_objects();
}

optimizer::optimizer(photogroup* photogroup_ptr)
{
   allocate_member_objects();
   initialize_member_objects();

   this->photogroup_ptr=photogroup_ptr;
   n_photos=photogroup_ptr->get_n_photos();
   n_params=n_params_per_photo*n_photos;
}

// Copy constructor:

optimizer::optimizer(const optimizer& o)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(o);
}

optimizer::~optimizer()
{
//   cout << "inside optimizer destructor" << endl;
   delete R0_ptr;
   delete u_meas_ptr;
   delete v_meas_ptr;
   delete u_manual_ptr;
   delete v_manual_ptr;

   if (homography_ptrs_ptr != NULL)
   {
      for (unsigned int i=0; i<homography_ptrs_ptr->get_mdim(); i++)
      {
         for (unsigned int j=0; j<homography_ptrs_ptr->get_ndim(); j++)
         {
            delete homography_ptrs_ptr->get(i,j);
         }
      }
      delete homography_ptrs_ptr;
   }
}

// ---------------------------------------------------------------------
void optimizer::docopy(const optimizer& o)
{
}

// Overload = operator:

optimizer& optimizer::operator= (const optimizer& o)
{
   if (this==&o) return *this;
   docopy(o);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const optimizer& o)
{
   outstream << endl;
   return outstream;
}


// =====================================================================
// Set & get member functions
// =====================================================================

// Member function get_homography_ptr takes in integers p and q which
// correspond to integer orderings for mosaicing of multiple photos.
// p and q do NOT correspond to individual photo IDs.

homography* optimizer::get_homography_ptr(int p,int q)
{
   if (p > q)
   {
      cout << "Error in optimizer::get_homography_ptr()" << endl;
      cout << "p = " << p << " should not be greater than q = " << q << endl;
      return NULL;
   }
   else
   {
      return homography_ptrs_ptr->get(p,q);
   }
}

homography* optimizer::get_homography_ptr_given_photo_indices(int i,int j)
{
   int p=photogroup_ptr->get_photo_order_given_index(i);
   int q=photogroup_ptr->get_photo_order_given_index(j);
   cout << "Photo indices: i = " << i << " j = " << j << endl;
   cout << "Photo orders: p = " << p << " q = " << q << endl;
   return homography_ptrs_ptr->get(basic_math::min(p,q),basic_math::max(p,q));
}

// =====================================================================
// Camera parameter manipulation member functions
// =====================================================================


// Member function print_camera_parameters

void optimizer::print_camera_parameters(int curr_n_photos) const
{
//   cout << "inside optimizer::print_camera_parameters()" << endl;
//         cout << "n_photos = " << n_photos << endl;

   cout.precision(10);
   for (int i=0; i<curr_n_photos; i++)
   {
      camera* camera_ptr=photogroup_ptr->get_photograph_ptr(i)->
         get_camera_ptr();
      
      cout << "Parameters for camera " << i << endl << endl;
      cout << "--Uaxis_focal_length " << camera_ptr->get_fu() << endl;
      cout << "--Vaxis_focal_length " << camera_ptr->get_fv() << endl;
      cout << "--U0 " << camera_ptr->get_u0() << endl;
      cout << "--V0 " << camera_ptr->get_v0() << endl;
      cout << "--relative_az " << camera_ptr->get_rel_az()*180/PI << endl;
      cout << "--relative_el " << camera_ptr->get_rel_el()*180/PI << endl;
      cout << "--relative_roll " << camera_ptr->get_rel_roll()*180/PI << endl;
      threevector camera_posn=camera_ptr->get_world_posn();
      cout << "--camera_x_posn " << camera_posn.get(0) << endl;
      cout << "--camera_y_posn " << camera_posn.get(1) << endl;
      cout << "--camera_z_posn " << camera_posn.get(2) << endl;
      cout << endl;
   }
   cout << endl;

//   outputfunc::enter_continue_char();
}

// =====================================================================
// Homography computation member functions
// =====================================================================

// Member function extract_photo_feature_info() loops over all
// features within input *FeaturesGroup_ptr.  It extract UV
// coordinates for each photo within member *photogroup_ptr and stores
// them within member genmatrices *u_meas_ptr and *v_meas_ptr.

void optimizer::extract_photo_feature_info(FeaturesGroup* FeaturesGroup_ptr)
{
//   cout << "inside optimizer::extract_photo_feature_info()" << endl;
   string banner="Extracting photo feature information:";
   outputfunc::write_banner(banner);

   this->FeaturesGroup_ptr=FeaturesGroup_ptr;
   unsigned int n_features=FeaturesGroup_ptr->get_n_Graphicals();

   if (n_photos != photogroup_ptr->get_photo_order().size())
   {
      cout << "Error in optimizer::extract_photo_feature_info()!" << endl;
      cout << "n_photos = " << n_photos << endl;
      cout << "photogroup_ptr->get_photo_order.size() = " 
           << photogroup_ptr->get_photo_order().size() << endl;
      exit(-1);
   }
   
   cout << "n_features = " << n_features
        << " n_photos = " << n_photos << endl;

   delete u_meas_ptr;
   delete v_meas_ptr;
   u_meas_ptr=new genmatrix(n_features,n_photos);
   v_meas_ptr=new genmatrix(n_features,n_photos);

// Fill *u_meas_ptr and *v_meas_ptr with sentinel -1 values indicating
// missing data:

   u_meas_ptr->initialize_values(-1);
   v_meas_ptr->initialize_values(-1);

   for (unsigned int f=0; f<n_features; f++)
   {
      Feature* feature_ptr=FeaturesGroup_ptr->get_Feature_ptr(f);
      instantaneous_obs* obs_ptr=
         feature_ptr->get_all_particular_time_observations(
            FeaturesGroup_ptr->get_curr_t());

// Extract UV coordinate pairs for *feature_ptr:

      for (unsigned int p=0; p<n_photos; p++)
      {
         int curr_passnumber=photogroup_ptr->get_photo_order().at(p);
         if (obs_ptr->check_for_pass_entry(curr_passnumber))
         {
            threevector curr_UVW=obs_ptr->retrieve_UVW_coords(
               curr_passnumber);
            u_meas_ptr->put(f,p,curr_UVW.get(0));
            v_meas_ptr->put(f,p,curr_UVW.get(1));
         }
      } // loop over index p labeling photos in *photogroup_ptr
   } // loop over index f labeling features

//   cout << "u_meas = " << *u_meas_ptr << endl;
//   cout << "v_meas = " << *v_meas_ptr << endl;
}

// --------------------------------------------------------------------------
// Member function extract_manual_feature_info() loops over all
// features within input *manual_FeaturesGroup_ptr.  It extracts XYZ
// and UV coordinates for each manually established tiepoint pair and
// stores them within member STL vector XYZ_manual and genmatrices
// *u_manual_ptr and *v_manual_ptr.

void optimizer::extract_manual_feature_info(FeaturesGroup* FeaturesGroup_ptr)
{
//   cout << "inside optimizer::extract_manual_feature_info()" << endl;
   string banner="Extracting manual feature information:";
   outputfunc::write_banner(banner);

   unsigned int n_features=FeaturesGroup_ptr->get_n_Graphicals();
//   cout << "n_features = " << n_features << " n_photos = " << n_photos << endl;

   delete u_manual_ptr;
   delete v_manual_ptr;
   u_manual_ptr=new genmatrix(n_features,n_photos);
   v_manual_ptr=new genmatrix(n_features,n_photos);

// Fill *u_manual_ptr and *v_manual_ptr with sentinel -1 values
// indicating missing data:

   u_manual_ptr->initialize_values(-1);
   v_manual_ptr->initialize_values(-1);

   for (unsigned int f=0; f<n_features; f++)
   {
      Feature* feature_ptr=FeaturesGroup_ptr->get_Feature_ptr(f);
//      cout << "f = " << f 
//           << " feature_ptr = " << feature_ptr 
//           << " feature ID = " << feature_ptr->get_ID() 
//           << endl;
      instantaneous_obs* obs_ptr=
         feature_ptr->get_all_particular_time_observations(
            FeaturesGroup_ptr->get_curr_t());

// Extract UV coordinate pairs for *feature_ptr from passes 0 through
// n_photos-1:

      int photo_order_size=photogroup_ptr->get_photo_order().size();
//      cout << "photo_order.size() = " << photo_order_size << endl;
//      cout << "photo order = " << endl;
//      templatefunc::printVector(photogroup_ptr->get_photo_order());

      for (unsigned int p=0; p<n_photos; p++)
      {
         int curr_passnumber=p;
         if (photo_order_size > 0)
         {
            curr_passnumber=photogroup_ptr->
               get_photo_index_given_order(p);
         }

         if (obs_ptr->check_for_pass_entry(curr_passnumber))
         {
//            cout << "p = " << p << " passnumber = " << curr_passnumber
//                 << endl;
            threevector curr_UVW=obs_ptr->retrieve_UVW_coords(
               curr_passnumber);
//            cout << "curr_UVW = " << curr_UVW << endl;

            u_manual_ptr->put(f,p,curr_UVW.get(0));
            v_manual_ptr->put(f,p,curr_UVW.get(1));
         }
      } // loop over index p labeling photos in *photogroup_ptr

// Extract XYZ coordinates for *feature_ptr whose pass number is
// assumed to be greater than or equal to n_photos:

      obs_ptr->get_pass_numbers();

      int old_XYZ_passnumber=
         obs_ptr->find_first_passnumber_greater_than_input(n_photos-1);
      int new_XYZ_passnumber=n_photos;
      cout << "old_XYZ_passnumber = " << old_XYZ_passnumber
           << " new_XYZ_passnumber = " << new_XYZ_passnumber << endl;
      obs_ptr->change_passnumber(old_XYZ_passnumber,new_XYZ_passnumber);

      if (obs_ptr->check_for_pass_entry(new_XYZ_passnumber))
      {
         threevector curr_XYZ=obs_ptr->retrieve_UVW_coords(
            new_XYZ_passnumber);
//         cout << "curr_XYZ = " << curr_XYZ << endl;
         XYZ_manual.push_back(curr_XYZ);
      }
      else
      {
         cout << "ERROR in optimizer::extract_manual_feature_info()" << endl;
         cout << "No XYZ coords found for feature f = " << f << " ID = "
              << feature_ptr->get_ID() << endl;
      }

   } // loop over index f labeling features

   cout << "XYZ_manual = " << endl;
//   templatefunc::printVector(XYZ_manual);

   for (unsigned int i=-0; i<XYZ_manual.size(); i++)
   {
      threevector curr_XYZ=XYZ_manual.at(i);
      cout << curr_XYZ.get(0) << "  "
           << curr_XYZ.get(1) << "  "
           << curr_XYZ.get(2) << "  " << endl;
   }

   cout << "u_manual = " << *u_manual_ptr << endl;
   cout << "v_manual = " << *v_manual_ptr << endl;

   cout << "XYZ_manual.size() = " << XYZ_manual.size() << endl;
   cout << "n_features = " << n_features << endl;

//   outputfunc::enter_continue_char();
}

// --------------------------------------------------------------------------
// Member function compute_renormalized_homographies performs a linear
// least squares fit to overlapping feature information for each image
// pair labeled by indices i and j.  This method stores homographies
// whose determinants equal unity within member Genarray
// *homography_ptrs_ptr.

void optimizer::compute_renormalized_homographies()
{
//   cout << "inside optimizer::compute_renormalized_homgraphies()" 
//        << endl;

   string banner="Calculating renormalized homographies for each image pair:";
   outputfunc::write_banner(banner);

   homography_ptrs_ptr=new Genarray<homography*>(n_photos,n_photos);
   for (unsigned int p=0; p<n_photos; p++)
   {
      for (unsigned int q=0; q<p; q++)
      {
         homography_ptrs_ptr->put(p,q,NULL);
      }
      for (unsigned int q=p; q<n_photos; q++)
      {
         homography* curr_H_ptr=new homography();

//         int i=photogroup_ptr->get_photo_index_given_order(p);
//         int j=photogroup_ptr->get_photo_index_given_order(q);
//         photograph* photo_i_ptr=photogroup_ptr->get_photograph_ptr(i);
//          photograph* photo_j_ptr=photogroup_ptr->get_photograph_ptr(j);

//         cout << "Photo index: i = " << i << " j = " << j << endl;
         cout << "Photo order:  p = " << p << " q = " << q << endl;
//         cout << "photo_i: ID = " << photo_i_ptr->get_ID() 
//              << " filename = " << photo_i_ptr->get_filename() << endl;
//         cout << "photo_j: ID = " << photo_j_ptr->get_ID() 
//              << " filename = " << photo_j_ptr->get_filename() << endl;

         if (q > p)
         {
            vector<twovector> XY,UV;
            bool homography_calculated_flag=
               curr_H_ptr->parse_homography_inputs(
                  FeaturesGroup_ptr->get_n_Graphicals(),
                  p,q,u_meas_ptr,v_meas_ptr,XY,UV);
//            cout << "XY.size() = " << XY.size() << endl;

            if (!homography_calculated_flag)
            {
               delete curr_H_ptr;
               curr_H_ptr=NULL;
            }
            else
            {
               curr_H_ptr->compute_homography_matrix();
               curr_H_ptr->check_homography_matrix(XY,UV,false);

// Recompute homography after rejecting worst feature pairs:

               const double worst_frac_to_reject=0.15;
               int reduced_n_features=(1-worst_frac_to_reject)*XY.size();

               curr_H_ptr->parse_homography_inputs(
                  curr_H_ptr->get_XY_sorted(),curr_H_ptr->get_UV_sorted(),
                  reduced_n_features);
               curr_H_ptr->compute_homography_matrix();
               curr_H_ptr->enforce_unit_determinant();
               curr_H_ptr->compute_homography_inverse();
            } // homography_calculated_flag conditional
         } // j > i conditional

         homography_ptrs_ptr->put(p,q,curr_H_ptr);
         homography_ptrs_ptr->put(q,p,curr_H_ptr);

         if (curr_H_ptr==NULL)
         {
            cout << "No homography exists" << endl << endl;
         }
         else
         {
            cout << " H = " << (*curr_H_ptr) << endl;
         }
      } // loop over photo order q
   } // loop over photo order p


//   genmatrix* H01_ptr=homography_ptrs_ptr->get(0,1)->get_H_ptr();
//   genmatrix* H12_ptr=homography_ptrs_ptr->get(1,2)->get_H_ptr();
//   genmatrix* H02_ptr=homography_ptrs_ptr->get(0,2)->get_H_ptr();

//   cout << "H_01 * H_12 = " 
//        << (*H01_ptr) * (*H12_ptr) << endl;
//   cout << "H_02 = " << *H02_ptr << endl;
}
   
// --------------------------------------------------------------------------
// Method readin_renormalized_homographies is a utility function for
// raeding the homography output of program TIEPOINTS into program
// PANORAMA.

void optimizer::readin_renormalized_homographies(string homography_filename)
{
//   cout << "inside optimizer::readin_renormalized_homographies()" 
//        << endl;
   filefunc::ReadInfile(homography_filename);
//      cout << "text_line.size() = " << filefunc::text_line.size() << endl;

   unsigned int n_homographies=filefunc::text_line.size()/3;

   vector<homography*> homography_ptrs;
   for (unsigned int h=0; h<n_homographies; h++)
   {
      homography* H_ptr=new homography();
      homography_ptrs.push_back(H_ptr);
      for (unsigned int l=0; l<3; l++)
      {
         int i=h*3+l;
         vector<double> row_values=stringfunc::string_to_numbers(
            filefunc::text_line[i]);
         H_ptr->set_H_ptr_element(l,0,row_values[0]);
         H_ptr->set_H_ptr_element(l,1,row_values[1]);
         H_ptr->set_H_ptr_element(l,2,row_values[2]);
      } // loop over index l labeling text file row
   } // loop over index h labeling homographies

   int homography_counter=0;
   homography_ptrs_ptr=new Genarray<homography*>(n_photos,n_photos);
   for (unsigned int i=0; i<n_photos; i++)
   {
      for (unsigned int j=0; j<i+1; j++)
      {
         homography_ptrs_ptr->put(i,j,NULL);
      }
      for (unsigned int j=i+1; j<n_photos; j++)
      {
         homography* curr_H_ptr=homography_ptrs[homography_counter++];
         homography_ptrs_ptr->put(i,j,curr_H_ptr);

         cout << "i = " << i << " j = " << j
              << " H = " << (*curr_H_ptr) << endl;
//            outputfunc::enter_continue_char();

      } // loop over index j
   } // loop over index i
}

// =====================================================================
// Camera parameter estimation member functions
// =====================================================================

// Member function estimate_internal_and_relative_rotation_params
// works with previously computed values for camera internal and
// external parameters if they exist.  If input parameter
// curr_n_photos > 2, this method searches for the previously
// composited photograph which has the greatest number of feature
// overlaps with the curr_n_photos'th photograph.  It then computes the
// homography relating these two latter photos.  After decomposing the
// homography into its rotational part, this method assigns relative
// az, el and roll values to the curr_n_photos'th photo.

void optimizer::estimate_internal_and_relative_rotation_params(
   int curr_n_photos)
{
   string banner="Estimating internal & relative rotation parameters:";
   cout << "curr_n_photos = " << curr_n_photos << endl;
   outputfunc::write_banner(banner);
//   outputfunc::enter_continue_char();

   int qstart=0;
   if (curr_n_photos==2)
   {
      for (int q=qstart; q<curr_n_photos; q++)
      {
//         cout << "Photo order: qstart = " << qstart << " q = " << q << endl;
//         cout << "photo_index(qstart) = " 
//              << photogroup_ptr->get_photo_index_given_order(qstart) 
//              << " photo_index(q) = "
//              << photogroup_ptr->get_photo_index_given_order(q) << endl;
         estimate_relative_rotation(qstart,q);
      } // loop over index q labeling photographs
   }
   else
   {

// Find photo among already reconstructed set of curr_n_photos-1
// pictures which has greatest number of feature overlap with input
// photo labeled by curr_n_photos:

      genmatrix* ntiepoints_matrix_ptr=
         FeaturesGroup_ptr->get_ntiepoints_matrix_ptr();
      
      qstart=-1;
      int max_n_feature_matches=0;
      unsigned int qstop=curr_n_photos-1;
      int j=photogroup_ptr->get_photo_index_given_order(qstop);
      for (unsigned int q=0; q<qstop; q++)
      {
         int i=photogroup_ptr->get_photo_index_given_order(q);
         int n_feature_matches=ntiepoints_matrix_ptr->get(i,j);
//         cout << "q = " << q << " qstop = " << qstop
//              << " i = " << i << " j = " << j << endl;
//         cout << "nfeature_matches(i,j) = "
//              << n_feature_matches << endl;
         
         if (n_feature_matches > max_n_feature_matches)
         {
            qstart=q;
            max_n_feature_matches=n_feature_matches;
         }
      } // loop over order q labeling photos already reconstructed
      
      int i=photogroup_ptr->get_photo_index_given_order(qstart);

      cout << "Photo w/ greatest # of prev matches: qstart = " << qstart
           << " photo index i = " << i << endl;
      cout << "qstop = curr_n_photos-1 = " << qstop
           << " photo index j = " << j << endl;
      cout << "max_n_feature_matches = " << max_n_feature_matches << endl;
      cout << "*ntiepoints_matrix_ptr = " << *ntiepoints_matrix_ptr << endl;
//      outputfunc::enter_continue_char();

      estimate_relative_rotation(qstart,qstop);

   } // curr_n_photos==2 conditional

   print_camera_parameters(curr_n_photos);
}

// -------------------------------------------------------------------------
// Member function estimate_relative_rotation first implements the ray
// approach described in section 8 of "Minimal solutions for panoramic
// stitching" by Brown, Hartley and Nister to roughly estimate the
// relative rotation between photos labeled by input ordering indices
// p and q.  On 12/10/08, we followed Noah Snavely's suggestion to use
// this initial approach which does not depend upon potentially noisy
// homography matrices.

void optimizer::estimate_relative_rotation(int p, int q)
{
//   cout << endl;
   cout << "inside optimizer::estimate_relative_rotation()" << endl;
   cout << "p = " << p << " q = " << q << endl;

   if (p==q) return;
   
   camera* camera_p_ptr=photogroup_ptr->get_photograph_ptr(p)->
      get_camera_ptr();
   camera* camera_q_ptr=photogroup_ptr->get_photograph_ptr(q)->
      get_camera_ptr();

   double f_p=camera_p_ptr->get_fu();
   double f_q=camera_q_ptr->get_fu();
   cout << "f_p = " << f_p << " f_q = " << f_q << endl;

   genmatrix* Kpinv_ptr=camera_p_ptr->get_Kinv_ptr();
   genmatrix* Kqinv_ptr=camera_q_ptr->get_Kinv_ptr();

// Loop over all features and extract those which have valid UV
// coordinates for the two photos labeled by ordering indices p and q:

   vector<threevector> ray_pf,ray_qf;
   for (unsigned int f=0; f<u_meas_ptr->get_mdim(); f++)
   {
      double u_p=u_meas_ptr->get(f,p);
      if (u_p < 0) continue;

      double u_q=u_meas_ptr->get(f,q);
      if (u_q < 0) continue;

      double v_p=v_meas_ptr->get(f,p);
//      if (v_p < 0) continue;

      double v_q=v_meas_ptr->get(f,q);
//      if (v_q < 0) continue;

      ray_pf.push_back(((*Kpinv_ptr) * threevector(u_p,v_p,1)).unitvector());
      ray_qf.push_back(((*Kqinv_ptr) * threevector(u_q,v_q,1)).unitvector());
   } // loop over index f labeling features

   rotation R;
   R.rotation_between_ray_bundles(ray_pf,ray_qf);

// delta_az = az_q - az_p

   double delta_az,delta_el,delta_roll;
   R.az_el_roll_from_rotation(delta_az,delta_el,delta_roll);

   cout << "delta_az = " << delta_az*180/PI 
        << " delta_el = " << delta_el*180/PI 
        << " delta_roll = " << delta_roll*180/PI << endl;

   double az_p=camera_p_ptr->get_rel_az();
   double el_p=camera_p_ptr->get_rel_el();
   double roll_p=camera_p_ptr->get_rel_roll();
//   cout << "az_p = " << az_p*180/PI
//        << " el_p = " << el_p*180/PI
//        << " roll_p = " << roll_p*180/PI << endl;

   camera_q_ptr->set_rel_az(az_p+delta_az);
   camera_q_ptr->set_rel_el(el_p+delta_el);
   camera_q_ptr->set_rel_roll(roll_p+delta_roll);

//   cout << "----------------------------------------------" << endl;
//   cout << "Based upon ray estimates:" << endl;
//   cout << "az_q = " << camera_q_ptr->get_rel_az()*180/PI 
//        << " el_q = " << camera_q_ptr->get_rel_el()*180/PI 
//        << " roll_q = " << camera_q_ptr->get_rel_roll()*180/PI << endl;
   
// After roughly estimating the relative rotation angles, perform
// bundle adjustment for just photos p and q.  Reset angles for photo
// q based upon the bundle adjustment output.  In Dec 2008, we found
// that these "mini" bundle adjustment values are generally much more
// accurate than those based upon the ray technique employed above:

   int n_prev_composited_photos=0;
   vector<int> indices_of_photos_to_be_composited;
   indices_of_photos_to_be_composited.push_back(p);
   indices_of_photos_to_be_composited.push_back(q);

   optimizer_func::set_photo_pair_bundle_adjust_flag(true);
   optimizer_func::rotation_homography_bundle_adjustment(
      n_prev_composited_photos,
      indices_of_photos_to_be_composited,this);
   optimizer_func::set_photo_pair_bundle_adjust_flag(false);

   delta_az=camera_q_ptr->get_rel_az()-camera_p_ptr->get_rel_az();
   delta_el=camera_q_ptr->get_rel_el()-camera_p_ptr->get_rel_el();
   delta_roll=camera_q_ptr->get_rel_roll()-camera_p_ptr->get_rel_roll();

// Recall that we hold all parameters for photograph p fixed.  So
// reset the focal length and rotation angles to their original values
// which they had before this method was called:

   camera_p_ptr->set_f(f_p);
   camera_p_ptr->set_rel_az(az_p);
   camera_p_ptr->set_rel_el(el_p);
   camera_p_ptr->set_rel_roll(roll_p);

// On the other hand, assign new, improved values to all of the
// parameters for photograph q:

//      camera_q_ptr->set_f(f_q);
   camera_q_ptr->set_rel_az(az_p+delta_az);
   camera_q_ptr->set_rel_el(el_p+delta_el);
   camera_q_ptr->set_rel_roll(roll_p+delta_roll);

   cout << "----------------------------------------------" << endl;
   cout << "Based upon bundle adjustment:" << endl;
   cout << "az_q = " << camera_q_ptr->get_rel_az()*180/PI 
        << " el_q = " << camera_q_ptr->get_rel_el()*180/PI 
        << " roll_q = " << camera_q_ptr->get_rel_roll()*180/PI << endl;

//   outputfunc::enter_continue_char();
}

/*
// --------------------------------------------------------------------------

// Note added on 12/11/08: This next member function is deprecated.
// To our great surprise, we found in Dec 2008 the homography
// decomposition performed in this method often yields rotation
// matrices which are very far away from true SO(3) elements.  After
// projecting a noisy rotation down to a true SO(3) member, the
// extracted azimuth and elevation angles can be off from their true
// values by many dozens of degrees.  So we now use the approach
// advocated by Noah Snavely and estimate the rotation matrices by
// matching of ray bundles which does not involve noisy homographies.



// Member function estimate_relative_rotation_params() first recovers
// the internal fu, u0 and v0 parameters for cameras p and q from
// their input photographs.  It next solves H_pq = prefactor_pq * Kq *
// RqRpinv * Kpinv for rotation matrix RqRpinv.  After projecting out
// the genuine rotation part from RqRpinv via a singular value
// decomposition, this method subsequently extracts relative azimuth,
// elevation and roll angles for photograph q relative to photograph
// p.  It then updates the angles for camera_q (but leaves those for
// camera_p unchanged).


   
void optimizer::estimate_relative_rotation_params(
   int p,int q,homography* Hpq_ptr)
{
   cout << "inside optimizer:estimate_relative_rotation_params()" << endl;
   cout << "p = " << p << " q = " << q << endl;

   if (Hpq_ptr==NULL) return;

   camera* camera_p_ptr=photogroup_ptr->get_photograph_ptr(p)->
      get_camera_ptr();
   camera* camera_q_ptr=photogroup_ptr->get_photograph_ptr(q)->
      get_camera_ptr();

   double f_p=camera_p_ptr->get_fu();
   double f_q=camera_q_ptr->get_fu();
   cout << "f_p = " << f_p << " f_q = " << f_q << endl;

   genmatrix* Kp_ptr=camera_p_ptr->get_K_ptr();
   genmatrix* Kqinv_ptr=camera_q_ptr->get_Kinv_ptr();

//   cout << "*Kp_ptr = " << *Kp_ptr << endl;
//   cout << "*Kqinv_ptr = " << *Kqinv_ptr << endl;

   double prefactor_pq=pow(f_p/f_q,0.666666);
  
   genmatrix* H_ptr=Hpq_ptr->get_H_ptr();
//   cout << "*H_ptr = " << *H_ptr << endl;
//   cout << "H_ptr->determinant() = "
//        << H_ptr->determinant() << endl;

   rotation RqRpinv_approx=1.0/prefactor_pq * (*Kqinv_ptr) * 
      (*(Hpq_ptr->get_H_ptr())) * (*Kp_ptr);
//   cout << "RqRpinv_approx = " << RqRpinv_approx << endl;
//   cout << "RqRpinv_approx.determinant() = "
//        << RqRpinv_approx.determinant() << endl;
//   cout << "RqRpinv_approx * RqRpinv_approx.trans = "
//        << RqRpinv_approx*RqRpinv_approx.transpose() << endl;
//   rotation RqRpinv=RqRpinv_approx.projectio_rotation(&RqRpinv_approx);
   rotation RqRpinv=RqRpinv_approx.projection_rotation();
//   cout << "RqRpinv = " << RqRpinv << endl;

   double rel_az,rel_el,rel_roll;
   RqRpinv.az_el_roll_from_rotation(rel_az,rel_el,rel_roll);
   cout << "rel_az = " << rel_az*180/PI
        << " rel_el = " << rel_el*180/PI
        << " rel_roll = " << rel_roll*180/PI << endl;

//   cout << "Rot from az, el, roll = "
//        << RqRpinv.rotation_from_az_el_roll(rel_az,rel_el,rel_roll) << endl;

   double az_p=camera_p_ptr->get_rel_az();
   double el_p=camera_p_ptr->get_rel_el();
   double roll_p=camera_p_ptr->get_rel_roll();
   cout << "az_p = " << az_p*180/PI
        << " el_p = " << el_p*180/PI
        << " roll_p = " << roll_p*180/PI << endl;
   camera_q_ptr->set_rel_az(az_p-rel_az);
   camera_q_ptr->set_rel_el(el_p-rel_el);
   camera_q_ptr->set_rel_roll(roll_p-rel_roll);

   cout << "az_q = " << camera_q_ptr->get_rel_az()*180/PI 
        << " el_q = " << camera_q_ptr->get_rel_el()*180/PI 
        << " roll_q = " << camera_q_ptr->get_rel_roll()*180/PI << endl;

   estimate_relative_rotation(p,q);

//   outputfunc::enter_continue_char();
}
*/

// -------------------------------------------------------------------------
// Member function homography_error takes in photo indices p and q
// along with matrices *u_meas_ptr and *v_meas_ptr which hold feature
// coordinates for all input photos.  Following "Automatic Panoramic
// Image Stitching using Invariant Features" by Brown and Lowe, we
// project (u_p,v_p) into the q image plane via input homography
// matrix *H_ptr = K_q R_q R_p_inv K_p_inv.  This method returns the
// summed squared residual between the projected feature and the input
// (u_q,v_q) coordinates.

double optimizer::homography_error(int p, int q, const genmatrix& H)
{
//   cout << "inside optimizer::homography_error()" << endl;

   double u_p,u_q,v_p,v_q;
   double U_proj,V_proj,W_proj;

   double h00=H.get(0,0);
   double h01=H.get(0,1);
   double h02=H.get(0,2);

   double h10=H.get(1,0);
   double h11=H.get(1,1);
   double h12=H.get(1,2);

   double h20=H.get(2,0);
   double h21=H.get(2,1);
   double h22=H.get(2,2);

   double residual=0;
   unsigned int n_features=u_meas_ptr->get_mdim();
//   cout << "n_features = " << n_features << endl;
   for (unsigned int f=0; f<n_features; f++)
   {
      u_p=u_meas_ptr->get(f,p);
      if (u_p < 0) continue;

      u_q=u_meas_ptr->get(f,q);
      if (u_q < 0) continue;

      v_p=v_meas_ptr->get(f,p);
//      if (v_p < 0) continue;

      v_q=v_meas_ptr->get(f,q);
//      if (v_q < 0) continue;

      U_proj = h00*u_p + h01*v_p + h02;
      V_proj = h10*u_p + h11*v_p + h12;
      W_proj = h20*u_p + h21*v_p + h22;
      
      double sqrd_delta=sqr(U_proj/W_proj-u_q)+sqr(V_proj/W_proj-v_q);

      if (sqrd_delta < sqr_sigma)
      {
         residual += sqrd_delta;
      }
      else
      {
         residual += 2*sigma*sqrt(sqrd_delta)-sqr_sigma;
      }
      
//      cout << "f = " << f << " residual = " << residual <<  endl;
   } // loop over index f labeling features

   double avg_residual=residual/n_features;
//   return residual;
   return avg_residual;
}

// -------------------------------------------------------------------------
// Member function projection_error() takes in photo index q and
// recovers the corresponding calibrated camera's 3x4 projection
// matrix *P_ptr.  *P_ptr is supposed to project manually selected 3D
// feature coordinates stored in member STL vector XYZ_manual onto
// photo q.  This method returns the averaged squared residual between
// the projected features and the manually extracted photo coordinates
// stored within members *u_manual_ptr and *v_manual_ptr.

double optimizer::projection_error(int q)
{
//   cout << "inside optimizer::projection_error()" << endl;

   photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(q);
   camera* camera_ptr=photograph_ptr->get_camera_ptr();
   const genmatrix* P_ptr=camera_ptr->get_P_ptr();

   double P00=P_ptr->get(0,0);
   double P01=P_ptr->get(0,1);
   double P02=P_ptr->get(0,2);
   double P03=P_ptr->get(0,3);

   double P10=P_ptr->get(1,0);
   double P11=P_ptr->get(1,1);
   double P12=P_ptr->get(1,2);
   double P13=P_ptr->get(1,3);

   double P20=P_ptr->get(2,0);
   double P21=P_ptr->get(2,1);
   double P22=P_ptr->get(2,2);
   double P23=P_ptr->get(2,3);

   double residual=0;
   unsigned int n_features=XYZ_manual.size();

   if (n_features==0)
   {
      cout << "Error in optimizer::projection_error()" << endl;
      cout << "q = " << q << " n_features = " << n_features << endl;
   }
   
   for (unsigned int f=0; f<n_features; f++)
   {

// Ignore any feature whose UV coordinates are not valid:

      double u_q=u_manual_ptr->get(f,q);
      if (u_q < 0) continue;
      double v_q=v_manual_ptr->get(f,q);
      if (v_q < 0) continue;

      double X=XYZ_manual[f].get(0);
      double Y=XYZ_manual[f].get(1);
      double Z=XYZ_manual[f].get(2);

      double U_proj = P00*X + P01*Y + P02*Z + P03;
      double V_proj = P10*X + P11*Y + P12*Z + P13;
      double W_proj = P20*X + P21*Y + P22*Z + P23;

      double u_proj=U_proj/W_proj;
      double v_proj=V_proj/W_proj;

//      cout << "q = " << q 
//           << " photo name = " << photograph_ptr->get_filename() << endl;
//      cout << " f = " << f 
//           << " u_q = " << u_q << " u_proj = " << u_proj
//           << " v_q = " << v_q << " v_proj = " << v_proj << endl;
//      cout << "X = " << X << " Y = " << Y << " Z = " << Z << endl;
//      cout << "U_proj = " << U_proj << " V_proj = " << V_proj
//           << " W_proj = " << W_proj << endl;
//      cout << "*P_ptr = " << *P_ptr << endl;

      double sqrd_delta=sqr(u_proj-u_q)+sqr(v_proj-v_q);
      residual += sqrd_delta;
      
   } // loop over index f labeling manually selected XYZ features
   double avg_residual=residual/n_features;

//   return residual;
   return avg_residual;
}

// --------------------------------------------------------------------------
// Member function bundle_adjust_for_rotating_camera sequentially
// forms 2, 3, 4, ... , n_photo mosaics.  Hold fitted camera
// parameters for previous n-1 photos fixed when iteratively solving
// for internal and external parameters for nth photo.  After this
// loop is completed, perform one final bundle adjustment where all
// parameters for all cameras are allowed to vary.

void optimizer::bundle_adjust_for_rotating_camera(int n_photos)
{
   cout << "inside optimizer:bundle_adjust_for_rotating_camera()" << endl;
   cout << "n_photos = " << n_photos << endl;

   for (int curr_n_photos=2; curr_n_photos <= n_photos; curr_n_photos++)
   {
//      cout << "curr_n_photos = " << curr_n_photos << endl;
      estimate_internal_and_relative_rotation_params(curr_n_photos);

      cout << "Initial camera parameters:" << endl;
      print_camera_parameters(curr_n_photos);
//      outputfunc::enter_continue_char();

// Perform iterative non-linear Levenberg-Marquadt least squares
// minimization to improve initial estimates for camera parameters:

      external_params_weight=0;
      if (fit_external_params_flag)
      {
         external_params_weight=1.0;
      }
      iteratively_bundle_adjust_for_rotating_camera(curr_n_photos);

// Perform global bundle adjustment after every iteration to improve
// 3D external parameter fitting in program LADARPAN:

      if (fit_external_params_flag)
      {
         external_params_weight=1.0;
         group_bundle_adjust_for_rotating_camera(curr_n_photos);
         external_params_weight=0;
         group_bundle_adjust_for_rotating_camera(curr_n_photos);
      }
   } // loop over curr_n_photos 

// Perform global bundle adjustment with all photo parameters allowed
// to vary:
   
   external_params_weight=0.0;
   group_bundle_adjust_for_rotating_camera(n_photos);

// Perform one more round of global bundle adjustment with and without
// manually extracted features for 3D external parameter fitting:

   if (fit_external_params_flag)
   {
      external_params_weight=1.0;
      group_bundle_adjust_for_rotating_camera(n_photos);
      external_params_weight=0;
      group_bundle_adjust_for_rotating_camera(n_photos);
   }

}

// --------------------------------------------------------------------------
// Member function iteratively_bundle_adjust_for_rotating_camera
// performs iterative non-linear Levenberg-Marquadt least squares
// minimization to improve initial estimates for camera parameters.

void optimizer::iteratively_bundle_adjust_for_rotating_camera(
   int curr_n_photos)
{
   cout << "inside optimizer:iteratively_bundle_adjust_for_rotating_camera()" 
        << endl;

   unsigned int n_iters=3;
   for (unsigned int iter=0; iter<n_iters; iter++)
   {
      if (iter==0)
      {
         set_sigma(100);
      }
      else if (iter==1)
      {
         set_sigma(1);
      }
      else
      {
         set_sigma(0.01);
      } // iter conditional

      int n_prev_composited_photos=0;
      int starting_photo_number,stopping_photo_number;
      if (curr_n_photos==2)
      {
         starting_photo_number=0;
         stopping_photo_number=1;
      }
      else
      {
         n_prev_composited_photos=curr_n_photos-1;
         starting_photo_number=curr_n_photos-1;
         stopping_photo_number=curr_n_photos-1;
      } // curr_n_photos conditional

      vector<int> indices_of_photos_to_be_composited;
      for (int i=starting_photo_number; i<=stopping_photo_number; i++)
      {
         indices_of_photos_to_be_composited.push_back(i);
      }
      
      optimizer_func::rotation_homography_bundle_adjustment(
         n_prev_composited_photos,
         indices_of_photos_to_be_composited,this);
      cout << "iter = " << iter+1 << " of " << n_iters << endl;
//         print_camera_parameters(curr_n_photos);
   } // loop over iter index 
//      outputfunc::enter_continue_char();
}

// --------------------------------------------------------------------------
// Member function group_bundle_adjust_for_rotating_camera allows all
// parameters for photos labeled by indices 0 through input
// curr_n_photos to vary.

void optimizer::group_bundle_adjust_for_rotating_camera(int curr_n_photos)
{
   cout << "inside optimizer:group_bundle_adjust_for_rotating_camera()" << endl;

   string banner="Performing group bundle adjustment for "
      +stringfunc::number_to_string(curr_n_photos)+" photos";
   outputfunc::write_big_banner(banner);

   int n_prev_composited_photos=curr_n_photos-1;
   int starting_photo_number=0;
   int stopping_photo_number=curr_n_photos-1;

   vector<int> indices_of_photos_to_be_composited;
   for (int i=starting_photo_number; i<=stopping_photo_number; i++)
   {
      indices_of_photos_to_be_composited.push_back(i);
   }
   
   optimizer_func::rotation_homography_bundle_adjustment(
      n_prev_composited_photos,
      indices_of_photos_to_be_composited,this);
   print_camera_parameters(curr_n_photos);
}

// --------------------------------------------------------------------------
// Member function bundle_adjust_for_global_photosynth_params()
// searches for the absolute translation, rotation and scale
// parameters which minimizes the reprojection error of a relatively
// small number of manually selected 3D/2D tiepoint pairs.

void optimizer::bundle_adjust_for_global_photosynth_params(
   FeaturesGroup* manual_FeaturesGroup_ptr)
{
   cout << "inside optimizer:bundle_adjust_for_global_photosynth_params()" 
        << endl;
   
   FeaturesGroup_ptr=manual_FeaturesGroup_ptr;

   optimizer_func::global_photosynth_bundle_adjustment(this);
   photogroup_ptr->print_bundler_to_world_params();
}

// =====================================================================
// Camera rotation from image and world space ray matching member functions
// =====================================================================

// Member function compute_world_and_imagespace_feature_rays()
// converts the feature world-space coordinates in member STL vector
// XYZ_manual into world-space rays relative to the panorama camera's
// position.  It also computes corresponding average rays in
// imagespace coordinates for each feature.  The results are stored in
// member STL vectors worldspace_ray and imagespace_ray.

void optimizer::compute_world_and_imagespace_feature_rays()
{
   threevector camera_posn=photogroup_ptr->get_photograph_ptr(0)->
      get_camera_ptr()->get_world_posn();
   compute_world_and_imagespace_feature_rays(camera_posn);
}

void optimizer::compute_world_and_imagespace_feature_rays(
   const threevector& camera_posn)
{
//   cout << "inside optimizer::compute_world_and_imagespace_feature_rays()" 
//        << endl;
//   cout << "camera_posn = " << camera_posn << endl;
   string banner="Computing world and image space feature rays:";
//   outputfunc::write_banner(banner);

   worldspace_ray.clear();
   imagespace_ray.clear();

   unsigned int n_features=u_manual_ptr->get_mdim();
//   cout << "n_features = " << n_features << endl;
   for (unsigned int f=0; f<n_features; f++)
   {
//      cout << "Feature index f = " << f << endl;

// Compute average of rays corresponding to UV coordinates for each
// feature:

      int n_rays=0;
      threevector avg_ray;
      for (unsigned int p=0; p<u_manual_ptr->get_ndim(); p++)
      {
         double curr_u=u_manual_ptr->get(f,p);
         double curr_v=v_manual_ptr->get(f,p);
         if (curr_u < 0 || curr_v < 0) continue;

//         cout << "photo_order.size() = "
//              << photogroup_ptr->get_photo_order().size() 
//              << endl;

         photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(p);
         if (photogroup_ptr->get_photo_order().size() > 0)
         {
            photograph_ptr=photogroup_ptr->
               get_ordered_photograph_ptr(p);
         }
//         cout << "*photograph_ptr = " << *photograph_ptr << endl;
         camera* camera_ptr=photograph_ptr->get_camera_ptr();
//         cout << "*camera_ptr = " << *camera_ptr << endl;
         threevector curr_ray = 
            camera_ptr->pixel_ray_direction(curr_u,curr_v);
//         cout << "p = " << p
//              << " filename = " << photograph_ptr->get_filename()
//              << " curr_ray = " << curr_ray << endl;
         avg_ray += curr_ray;
         n_rays++;
      } // loop over index p labeling photos in *photogroup_ptr

      if (n_rays==0) continue;

      avg_ray /= double(n_rays);
      imagespace_ray.push_back(avg_ray.unitvector());

//      cout << "XYZ_manual[f] = " << XYZ_manual[f] << endl;
      worldspace_ray.push_back( (XYZ_manual[f]-camera_posn).unitvector() );

//      cout << "worldspace_ray = " << worldspace_ray.back() << endl;
//      cout << "imagespace_ray = " << imagespace_ray.back() << endl;
      
   } // loop over index f labeling features
}

// --------------------------------------------------------------------------
// Member function
// compute_scalefactor_between_world_and_imagespace_rays() calculates
// (n_features choose 2) angles between pairs of worldspace rays and
// their imagespace ray counterparts.  This method subsequently
// reports the mean and standard deviation of the ratios of worldspace
// ray pair angles to corresponding imagespace ray pair angles.

double optimizer::compute_scalefactor_between_world_and_imagespace_rays()
{
   cout << "inside optimizer::compute_scalefactor_between_world_and_imagespace_rays()" << endl;
   string banner="Computing scalefactor between world and imagespace rays:";
   outputfunc::write_banner(banner);

   unsigned int n_features=basic_math::min(
      u_manual_ptr->get_mdim(),(unsigned int) worldspace_ray.size());
//   cout << "n_features = " << n_features << endl;
//   cout << "worldspace_ray.size() = " << worldspace_ray.size() << endl;
//   cout << "imagespace_ray.size() = " << imagespace_ray.size() << endl;

   vector<double> world_to_image_space_angle_ratios;
   for (unsigned int f=0; f<n_features; f++)
   {
      threevector curr_worldspace_ray=worldspace_ray[f];
      threevector curr_imagespace_ray=imagespace_ray[f];
      for (unsigned int g=f+1; g<n_features; g++)
      {
         threevector next_worldspace_ray=worldspace_ray[g];
         threevector next_imagespace_ray=imagespace_ray[g];
         
         double worldspace_dotproduct=curr_worldspace_ray.dot(
            next_worldspace_ray);
         double imagespace_dotproduct=curr_imagespace_ray.dot(
            next_imagespace_ray);
         double worldspace_angle=acos(worldspace_dotproduct);
         double imagespace_angle=acos(imagespace_dotproduct);
         double ratio=worldspace_angle/imagespace_angle;
         world_to_image_space_angle_ratios.push_back(ratio);
//         cout << "f = " << f << " g = " << g
//              << " w_angle = " << worldspace_angle*180/PI
//              << " i_angle = " << imagespace_angle*180/PI
//              << " r = " << ratio 
//              << endl;
      } // loop over index g
   } // loop over index f
   
   double mu=mathfunc::mean(world_to_image_space_angle_ratios);
   double sigma=mathfunc::std_dev(world_to_image_space_angle_ratios);

   cout << "Angular scale stretch factor = " << endl;
   cout << "  ratio of worldspace/imagespace ray pair angles = "
        << mu << " +/- " << sigma << endl;

/*
// Recall nonlinear relationship between photograph's field-of-view
// and its focal parameter f (see "Relationships between focal
// parameter and FOV" notes dated 2/9/09)

// 	FOV_U = 2 atan[ (Umax-U0)/f_u ]   
// 	FOV_V = 2 atan[ (Vmax-V0)/f_v ]

// By linearizing this relationship around an initial focal parameter
// value f0, we can estimate a new f value which to good approximation
// rescales the imagespace rays so that their angular extents match
// their worldspace counterparts:

   double f0=2.64;	// f value for Jan 6, 2009 Lobby7 panorama
   double alpha=mu-1;
   double delta=0.5 * (0.5 + 0.66666); // avg of (Umax-U0) & (Vmax-V0)

   double fnew=f0-alpha*(sqr(f0)+sqr(delta))/delta*atan(delta/f0);

   cout << "f0 = " << f0 << endl;
   cout << "fnew = " << fnew << endl;
   cout << "Focal parameter scale factor = fnew/f0 = " << fnew/f0 << endl;
*/

//   outputfunc::enter_continue_char();

   return mu;
}

// --------------------------------------------------------------------------
// Member function
// compute_rotation_between_imagespace_rays_and_world_rays()
// calculates the best-fit rotation R_global which maps the bundle of
// rays within member STL vector imagespace_ray onto their
// counterparts in STL vector worldspace_ray.  Once a panorama has
// been properly rescaled, the global rotation subsequently aligns it
// with a 3D point cloud.  This method applies R_global to each camera
// for each photo within *photogroup_ptr.

rotation optimizer::compute_rotation_between_imagespace_rays_and_world_rays()
{
   double delta_thetas_mu,delta_thetas_sigma;
   return compute_rotation_between_imagespace_rays_and_world_rays(
      imagespace_ray,worldspace_ray,delta_thetas_mu,delta_thetas_sigma);
}

rotation optimizer::compute_rotation_between_imagespace_rays_and_world_rays(
   const vector<threevector>& curr_imagespace_ray,
   const vector<threevector>& curr_worldspace_ray)
{
   double delta_thetas_mu,delta_thetas_sigma;
   return compute_rotation_between_imagespace_rays_and_world_rays(
      curr_imagespace_ray,curr_worldspace_ray,
      delta_thetas_mu,delta_thetas_sigma);
}

rotation optimizer::compute_rotation_between_imagespace_rays_and_world_rays(
   double& delta_thetas_mu,double& delta_thetas_sigma)
{
   return compute_rotation_between_imagespace_rays_and_world_rays(
      imagespace_ray,worldspace_ray,delta_thetas_mu,delta_thetas_sigma);
}

rotation optimizer::compute_rotation_between_imagespace_rays_and_world_rays(
   const vector<threevector>& curr_imagespace_ray,
   const vector<threevector>& curr_worldspace_ray,
   double& delta_thetas_mu,double& delta_thetas_sigma)
{
//   cout << "inside optimizer::compute_rotation_between_imagespace_rays_and_world_rays()" << endl;

   rotation R_global;
   R_global.rotation_between_ray_bundles(
      curr_worldspace_ray,curr_imagespace_ray);
//   cout << "R_global = " << R_global << endl;

   double az_global,el_global,roll_global;
   R_global.az_el_roll_from_rotation(az_global,el_global,roll_global);
//   cout << "az_global = " << az_global*180/PI << endl;
//   cout << "el_global = " << el_global*180/PI << endl;
//   cout << "roll_global = " << roll_global*180/PI << endl;

   unsigned int n_features=basic_math::min(
      u_manual_ptr->get_mdim(),(unsigned int) curr_worldspace_ray.size());
//   cout << "n_features = " << n_features << endl;

// Store discrepancies between rotated imagespace rays and their
// worldspace counterparts within STL vector delta_thetas:

   vector<double> delta_thetas;
   for (unsigned int f=0; f<n_features; f++)
   {
      threevector rot_imagespace_ray=R_global*curr_imagespace_ray[f];
//      cout << "f = " << f
//           << " world ray = " << worldspace_ray[f]
//           << " R_global* image ray = " << rot_imagespace_ray << endl;
      double dotproduct=curr_worldspace_ray[f].dot(rot_imagespace_ray);
      delta_thetas.push_back(acos(dotproduct));
   } // loop over index f
   
   delta_thetas_mu=mathfunc::mean(delta_thetas)*180/PI;
   delta_thetas_sigma=mathfunc::std_dev(delta_thetas)*180/PI;

//   cout << endl;
//   cout << "=========================================================" << endl;
//   cout << "Absolute value of discrepancy between worldspace and rotated"
//        << endl;
//   cout << "image space rays = " << delta_thetas_mu << " +/- " 
//        << delta_thetas_sigma << " degrees" << endl;
//   cout << "=========================================================" << endl;

//   outputfunc::enter_continue_char();

   return R_global;
}


// --------------------------------------------------------------------------
// Member function
// randomly_compute_rotation_between_imagespace_rays_and_world_rays()
// randomly
// chooses one manual feature from each of the input photos
// (e.g. OBSFRUSTA within panoramic video camera).  It returns the
// global rotation which maps imagespace rays onto their world space
// counterparts based upon just this small number (n_photos) worth of
// input feature tiepoints.

rotation optimizer::randomly_compute_rotation_between_imagespace_rays_and_world_rays(
   const threevector& camera_posn)
{
//   cout << "inside optimizer::randomly_compute_rotation_between_imagespace_rays_and_world_rays()" << endl;

   vector<threevector> tmp_worldspace_ray,tmp_imagespace_ray;

   unsigned int n_features=XYZ_manual.size();
   for (unsigned int p=0; p<n_photos; p++)
   {

// For pth photo, first count number of non-negative U and V feature coords:

      double curr_u,curr_v;
      int n_features_for_photo=0;
      for (unsigned int m=0; m<u_manual_ptr->get_mdim(); m++)
      {
         curr_u=u_manual_ptr->get(m,p);
         if (curr_u >= 0) n_features_for_photo++;
      }
      if (n_features_for_photo==0) continue;

      bool feature_picked_flag=false;
      int f;
      while (!feature_picked_flag)
      {
         f=n_features*nrfunc::ran1();
         curr_u=u_manual_ptr->get(f,p);
         if (curr_u >= 0) feature_picked_flag=true;
      }
      curr_v=v_manual_ptr->get(f,p);

      photograph* photograph_ptr=photogroup_ptr->get_photograph_ptr(p);
      if (photogroup_ptr->get_photo_order().size() > 0)
      {
         photograph_ptr=photogroup_ptr->get_ordered_photograph_ptr(p);
      }
      camera* camera_ptr=photograph_ptr->get_camera_ptr();
      tmp_imagespace_ray.push_back(
         camera_ptr->pixel_ray_direction(curr_u,curr_v));
      tmp_worldspace_ray.push_back( (XYZ_manual[f]-camera_posn).unitvector() );
   } // loop over index p labeling photos
   
   rotation R_global;
   R_global.rotation_between_ray_bundles(
      tmp_worldspace_ray,tmp_imagespace_ray);
//   cout << "R_global = " << R_global << endl;

   double az_global,el_global,roll_global;
   R_global.az_el_roll_from_rotation(az_global,el_global,roll_global);
//   cout << "az_global = " << az_global*180/PI << endl;
//   cout << "el_global = " << el_global*180/PI << endl;
//   cout << "roll_global = " << roll_global*180/PI << endl;

   return R_global;
}

// --------------------------------------------------------------------------
// Member function
// compute_inlier_rotation_between_imagespace_rays_and_world_rays()
// takes in a candidate R_global calculated from just n_photos worth
// of manually identified imagespace and worldspace features.  Looping
// over all features, it identifies inliers whose rotated imagespace
// rays lie close to their worldspace counterparts.  This method then
// recomputes and returns the global rotation corresponding to just
// inlier imagespace and worldspace features.

bool 
optimizer::compute_inlier_rotation_between_imagespace_rays_and_world_rays(
   double min_dotproduct,const rotation& R_global,
   unsigned int& max_n_inliers,rotation& inlier_R_global,
   double& delta_thetas_mu,double& delta_thetas_sigma)
{
//   cout << "inside optimizer::compute_inlier_rotation_between_imagespace_rays_and_world_rays()" << endl;

   bool curr_best_inlier_rotation_flag=false;
   unsigned int n_features=XYZ_manual.size();

//   const double min_dotproduct=cos(0.75*PI/180);
//   const double min_dotproduct=cos(0.8*PI/180);
//   const double min_dotproduct=cos(0.85*PI/180);
//   const double min_dotproduct=cos(0.85*PI/180);
//   const double min_dotproduct=cos(0.9*PI/180);
//   const double min_dotproduct=cos(0.95*PI/180);
//   const double min_dotproduct=cos(1.0*PI/180);
   vector<threevector> inlier_imagespace_ray,inlier_worldspace_ray;
   for (unsigned int f=0; f<n_features; f++)
   {
      threevector rot_imagespace_ray=R_global*imagespace_ray[f];
//      cout << "f = " << f
//           << " world ray = " << worldspace_ray[f]
//           << " R_global* image ray = " << rot_imagespace_ray << endl;
      double dotproduct=worldspace_ray[f].dot(rot_imagespace_ray);
      if (dotproduct < min_dotproduct) continue;
      inlier_imagespace_ray.push_back(imagespace_ray[f]);
      inlier_worldspace_ray.push_back(worldspace_ray[f]);
   } // loop over index f

   unsigned int n_inliers=inlier_imagespace_ray.size();
//   cout << "n_inliers = " << n_inliers << endl;
//   cout << "max_n_inliers = " << max_n_inliers << endl;
   if (n_inliers < max_n_inliers)
   {
      return curr_best_inlier_rotation_flag;
   }
   else
   {
      max_n_inliers=n_inliers;
   }

// Recompute Rglobal using only inlier image and world space rays:

   inlier_R_global=
      compute_rotation_between_imagespace_rays_and_world_rays(
         inlier_imagespace_ray,inlier_worldspace_ray);
//   double az_global,el_global,roll_global;
//   inlier_R_global.az_el_roll_from_rotation(az_global,el_global,roll_global);

// Store discrepancies between rotated inlier imagespace rays and their
// worldspace counterparts within STL vector delta_thetas:

   vector<double> delta_thetas;
   for (unsigned int f=0; f<inlier_imagespace_ray.size(); f++)
   {
      threevector rot_imagespace_ray=inlier_R_global*inlier_imagespace_ray[f];
      double dotproduct=inlier_worldspace_ray[f].dot(rot_imagespace_ray);
      delta_thetas.push_back(acos(dotproduct));
   } // loop over index f
   
   delta_thetas_mu=mathfunc::mean(delta_thetas)*180/PI;
   delta_thetas_sigma=mathfunc::std_dev(delta_thetas)*180/PI;

   curr_best_inlier_rotation_flag=true;
   return curr_best_inlier_rotation_flag;
}

// --------------------------------------------------------------------------
// Member function
// compute_RANSAC_rotation_between_imagespace_rays_and_world_rays() 
// first computes rotations which map one feature from each photo onto
// its manually selected worldspace counterpart.  This initial
// rotation is used to define inlier and outlier features.  An
// improved rotation estimate is recomputed based upon inlier
// features.  This RANSAC method returns the inlier rotation with the
// maximum number of inlier features.

rotation 
optimizer::compute_RANSAC_rotation_between_imagespace_rays_and_world_rays(
   unsigned int iter,const threevector& camera_posn,double& delta_thetas_mu,
   double& delta_thetas_sigma)
{
   cout << "in optimizer::compute_RANSAC_rotation_between_imagespace_rays_and_world_rays()" << endl;
   
   unsigned int max_n_inliers=0;
   rotation random_R_global,inlier_R_global;

   double max_angle_discrepancy=0.95*PI/180;
//   double max_angle_discrepancy=1.0*PI/180;
   for (unsigned int j=0; j<iter; j++)
   {
      max_angle_discrepancy *= 0.975;
//      max_angle_discrepancy *= 0.95;
   }
//   cout << "iter = " << iter 
//        << " Max angle discrepancy = " << max_angle_discrepancy*180/PI 
//        << " degs" << endl;
   double min_dotproduct=cos(max_angle_discrepancy);

   const unsigned int n_RANSAC_iters=1000;
   for (unsigned int i=0; i<n_RANSAC_iters; i++)
   {
      random_R_global=
         randomly_compute_rotation_between_imagespace_rays_and_world_rays(
            camera_posn);
      if (compute_inlier_rotation_between_imagespace_rays_and_world_rays(
         min_dotproduct,random_R_global,max_n_inliers,inlier_R_global,
         delta_thetas_mu,delta_thetas_sigma))
      {
      }
   } // loop over index i labeling RANSAC iterations
   cout << "max_n_inliers = " << max_n_inliers << endl;
   return inlier_R_global;
}
