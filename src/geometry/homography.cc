// ==========================================================================
// Homography class member functions
// ==========================================================================
// Last modified on 8/8/13; 10/1/13; 12/5/13; 4/4/14
// ==========================================================================

#include "general/filefuncs.h"
#include "geometry/homography.h"
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "templates/mytemplates.h"
#include "geometry/plane.h"
#include "math/prob_distribution.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"

using std::cout;
using std::cin;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void homography::allocate_member_objects()
{
   const int ndim=3;
   H_ptr=new genmatrix(ndim,ndim);
   Hinv_ptr=new genmatrix(ndim,ndim);
   P_ptr=new genvector(ndim);
   p_ptr=new genvector(ndim);
}

void homography::initialize_member_objects()
{
   A_ptr=NULL;
   plane1_ptr=NULL;
   plane2_ptr=NULL;

   H_ptr->identity();
   Hinv_ptr->identity();
}		       

homography::homography()
{
   allocate_member_objects();
   initialize_member_objects();
}

homography::homography(const genmatrix* Hcopy_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   load(Hcopy_ptr);
}

// ---------------------------------------------------------------------
// Member function load copies the entries of an external 3x3
// genmatrix *Hcopy_ptr into member *H_ptr:

void homography::load(const genmatrix* Hcopy_ptr)
{
   if (Hcopy_ptr->get_mdim()==3 && Hcopy_ptr->get_ndim()==3)
   {
      for (unsigned int m=0; m<3; m++)
      {
         for (unsigned int n=0; n<3; n++)
         {
            H_ptr->put(m,n,Hcopy_ptr->get(m,n));
         } // loop over index n
      } // loop over index m
   }
}

homography::~homography()
{
   delete A_ptr;
   delete H_ptr;
   delete Hinv_ptr;
   delete P_ptr;
   delete p_ptr;
   A_ptr=NULL;
   H_ptr=NULL;
   Hinv_ptr=NULL;
   P_ptr=NULL;
   p_ptr=NULL;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const homography& h)
{
   outstream << endl;
   outstream << "H = " << *(h.H_ptr) << endl;
   return outstream;
}

// ==========================================================================
// 3x3 homography matrix determination:
// ==========================================================================

bool homography::parse_homography_inputs(
   const vector<threevector>& XY,const vector<threevector>& UV)
{
   return parse_homography_inputs(XY,UV,XY.size());
}

bool homography::parse_homography_inputs(
   const vector<threevector>& XY,const vector<threevector>& UV,
   unsigned int n_inputs)
{
   vector<twovector> XY2,UV2;
   for (unsigned int n=0; n<n_inputs; n++)
   {
      XY2.push_back(twovector(XY[n]));
      UV2.push_back(twovector(UV[n]));
   }
   return parse_homography_inputs(XY2,UV2,n_inputs);
}
   
bool homography::parse_homography_inputs(
   const vector<twovector>& XY,const vector<twovector>& UV)
{
   return parse_homography_inputs(XY,UV,XY.size());
}

// ---------------------------------------------------------------------
// This next overloaded version of parse_homography_inputs takes in
// (U,V) features coordinates within input genmatrices *u_meas_ptr and
// *v_meas_ptr.  Rows within these genmatrices correspond to different
// features, while columns correspond to different photos.  Missing
// data are indicated by -1 sentinel values.  This member function
// computes the homography that maps features for image p1 to their
// counterparts in image p2.
   
bool homography::parse_homography_inputs(
   unsigned int n_features,int p1,int p2,
   const genmatrix* u_meas_ptr,const genmatrix* v_meas_ptr,
   vector<twovector>& XY,vector<twovector>& UV)
{
//   cout << "inside homography::parse_homography_inputs()" << endl;
//   cout << "*u_meas_ptr = " << *u_meas_ptr << endl;
//   cout << "*v_meas_ptr = " << *v_meas_ptr << endl;
//   cout << "n_features = " << n_features << endl;
//   cout << "p1 = " << p1 << " p2 = " << p2 << endl;

   int n_inputs=0;

   const double SMALL=-0.001;
   for (unsigned int f=0; f<n_features; f++)
   {
      twovector curr_xy(u_meas_ptr->get(f,p1),v_meas_ptr->get(f,p1));
      twovector curr_uv(u_meas_ptr->get(f,p2),v_meas_ptr->get(f,p2));
      if (curr_xy.get(0) > SMALL && curr_xy.get(1) > SMALL &&
          curr_uv.get(0) > SMALL && curr_uv.get(1) > SMALL)
      {
         XY.push_back(curr_xy);
         UV.push_back(curr_uv);
         n_inputs++;
      }
   } // loop over index f labeling rows in input genmatrices
   
//   cout << "XY = " << endl;
//   templatefunc::printVector(XY);
//   cout << "UV = " << endl;
//   templatefunc::printVector(UV);
//   cout << "n_inputs = " << n_inputs << endl;

   return parse_homography_inputs(XY,UV,n_inputs);
}

// ---------------------------------------------------------------------
// This overloaded version of boolean member function
// parse_homography_inputs returns false if there are insufficient
// tiepoint pair inputs to actually compute a homography.

bool homography::parse_homography_inputs(
   const vector<twovector>& XY,const vector<twovector>& UV,
   unsigned int n_inputs)
{
//   cout << "inside homography::parse_homography_inputs()" << endl;
//   cout << "n_inputs = " << n_inputs << endl;
   
   if (XY.size() != UV.size())
   {
      cout << "Error in homography::parse_homography_inputs()"
           << endl;
      cout << "XY.size() = " << XY.size() 
           << " UV.size() = " << UV.size() << endl;
      exit(-1);
   }

// Recall that at least 4 sets of tiepoints are needed in order to
// compute a homography:

   if (n_inputs < 4)
   {
      return false;
   }

   int nrows=2*n_inputs;
   int ncolumns=9;
   delete A_ptr;
   A_ptr=new genmatrix(nrows,ncolumns);

   for (unsigned int i=0; i<n_inputs; i++)
   {
      double x=XY[i].get(0);
      double y=XY[i].get(1);
      double u=UV[i].get(0);
      double v=UV[i].get(1);

//      cout << "i = " << i
//           << " X = " << x << " Y = " << y 
//           << " U = " << u << " V = " << v << endl;

      int row=i*2+0;
      A_ptr->put(row,0,x);
      A_ptr->put(row,1,y);
      A_ptr->put(row,2,1);

      A_ptr->put(row,3,0);
      A_ptr->put(row,4,0);
      A_ptr->put(row,5,0);

      A_ptr->put(row,6,-u*x);
      A_ptr->put(row,7,-u*y);
      A_ptr->put(row,8,-u);

      row++;

      A_ptr->put(row,0,0);
      A_ptr->put(row,1,0);
      A_ptr->put(row,2,0);

      A_ptr->put(row,3,x);
      A_ptr->put(row,4,y);
      A_ptr->put(row,5,1);

      A_ptr->put(row,6,-v*x);
      A_ptr->put(row,7,-v*y);
      A_ptr->put(row,8,-v);

   } // loop over index i
   
//   cout << "A = " << *A_ptr << endl;
//   cout << "A.mdim = " << A_ptr->get_mdim() << " A.ndim = " 
//        << A_ptr->get_ndim() << endl;

   return true;
}

// ---------------------------------------------------------------------
bool homography::parse_tieline_inputs(
   const vector<threevector>& abc,const vector<threevector>& def)
{
//   cout << "inside homography::parse_tieline_inputs()" << endl;
   
   if (abc.size() != def.size())
   {
      cout << "Error in homography::parse_tieline_inputs()"
           << endl;
      cout << "abc.size() = " << abc.size() 
           << " def.size() = " << def.size() << endl;
      exit(-1);
   }

// Recall that at least 4 sets of tielines are needed in order to
// compute a homography:

   unsigned int n_inputs=abc.size();
   if (n_inputs < 4) return false;

   int nrows=2*n_inputs;
   int ncolumns=9;
   delete A_ptr;
   A_ptr=new genmatrix(nrows,ncolumns);

   for (unsigned int i=0; i<n_inputs; i++)
   {
      double a=abc[i].get(0);
      double b=abc[i].get(1);
      double c=abc[i].get(2);

      double d=def[i].get(0);
      double e=def[i].get(1);
      double f=def[i].get(2);

      int row=i*2+0;
      A_ptr->put(row,0,0);
      A_ptr->put(row,1,0);
      A_ptr->put(row,2,0);

      A_ptr->put(row,3,-c*d);
      A_ptr->put(row,4,-c*e);
      A_ptr->put(row,5,-c*f);
      
      A_ptr->put(row,6,b*d);
      A_ptr->put(row,7,b*e);
      A_ptr->put(row,8,b*f);

      row++;

      A_ptr->put(row,0,c*d);
      A_ptr->put(row,1,c*e);
      A_ptr->put(row,2,c*f);

      A_ptr->put(row,3,0);
      A_ptr->put(row,4,0);
      A_ptr->put(row,5,0);

      A_ptr->put(row,6,-a*d);
      A_ptr->put(row,7,-a*e);
      A_ptr->put(row,8,-a*f);

   } // loop over index i
   
//   cout << "A = " << *A_ptr << endl;
//   cout << "A.mdim = " << A_ptr->get_mdim() << " A.ndim = " 
//        << A_ptr->get_ndim() << endl;

   return true;
}

// ---------------------------------------------------------------------
// Method compute_homography_matrix() works with the 2r x 9 genmatrix
// *A_ptr generated by method parse_homography_inputs.  It first
// performs a singular value decomposition of *A_ptr = U diag(w) V^T.
// It then identifies the smallest singular value (which should be
// close to zero) as well as the corresponding column eigenvector v
// within matrix V.  This column vector should approximately satisfy
// the homogenous system of equantions *A_ptr v = 0.  Finally, this
// method rearranges the 9 entries within v into the 3x3 projection
// genmatrix *H_ptr which maps homogeneous (x,y,1) coordinates from a
// world plane onto their corresponding homogenous (u,v,1) coordinates
// within image space.  
   
void homography::compute_homography_matrix(bool tielines_flag)
{
//   cout << "inside homography::compute_homography_matrix()" << endl;

   genvector X(9);
   
// Recall that our SVD routines needed to compute the solution to the
// homogeneous matrix equation A X = 0 require that mdim >= ndim.  So
// for the case where r=4, we need to explicitly augment input matrix
// A by adding a 9th row filled with zeros:

   unsigned int mdim=A_ptr->get_mdim();
   unsigned int ndim=A_ptr->get_ndim();
   if (mdim==8)
   {
      genmatrix A(9,9);
      A.clear_values();

      for (unsigned int m=0; m<mdim; m++)
      {
         for (unsigned int n=0; n<ndim; n++)
         {
            A.put(m,n,A_ptr->get(m,n));
         }
      }

      A.homogeneous_soln(X);
   }
   else
   {
      A_ptr->homogeneous_soln(X);
   }

   rearrange_homogeneous_soln_into_H(tielines_flag,X);
   compute_homography_inverse();

//   cout << "Homography matrix H = " << *H_ptr << endl;
//   cout << "Homography inverse matrix Hinv = " << *Hinv_ptr << endl;
//   cout << "H*Hinv = " << *H_ptr * (*Hinv_ptr) << endl;
}

// ---------------------------------------------------------------------
// Rearrange X vector's contents within 3x3 homography matrix *H_ptr.
// If input boolean tielines_flag==true, X contents go into columns of
// *H_ptr.  Otherwise, X contents go into rows of *H_ptr for tiepoints
// matching.  

void homography::rearrange_homogeneous_soln_into_H(
   bool tielines_flag,const genvector& X)
{
   if (tielines_flag)
   {
      H_ptr->put(0,0,X.get(0));
      H_ptr->put(1,0,X.get(1));
      H_ptr->put(2,0,X.get(2));

      H_ptr->put(0,1,X.get(3));
      H_ptr->put(1,1,X.get(4));
      H_ptr->put(2,1,X.get(5));
   
      H_ptr->put(0,2,X.get(6));
      H_ptr->put(1,2,X.get(7));
      H_ptr->put(2,2,X.get(8));
   }
   else
   {
      H_ptr->put(0,0,X.get(0));
      H_ptr->put(0,1,X.get(1));
      H_ptr->put(0,2,X.get(2));

      H_ptr->put(1,0,X.get(3));
      H_ptr->put(1,1,X.get(4));
      H_ptr->put(1,2,X.get(5));
   
      H_ptr->put(2,0,X.get(6));
      H_ptr->put(2,1,X.get(7));
      H_ptr->put(2,2,X.get(8));
   }
   
//      cout << "Homography matrix H = " << *H_ptr << endl;
}

// ---------------------------------------------------------------------
// Method project_world_plane_to_image_plane takes in the world
// coordinates for some planar point within twovector r.  It projects
// this world point down onto the image plane via 3x3 homography
// matrix *H_ptr.  This method returns the (u,v) image plane
// coordinates of the projected point.

void homography::project_world_plane_to_image_plane(
   double x,double y,double& u,double& v)
{
//   cout << "inside homography::project_world_plane_to_image_plane()" << endl;
   P_ptr->put(0,x);
   P_ptr->put(1,y);
   P_ptr->put(2,1);
//         cout << "Input vector = " << *P_ptr << endl;
   *p_ptr=(*H_ptr)*(*P_ptr);
//   cout << "Output vector = " << *p_ptr << endl;
   u=p_ptr->get(0)/p_ptr->get(2);
   v=p_ptr->get(1)/p_ptr->get(2);
//         cout << "u = " << u << " v = " << v << endl;
}

twovector homography::project_world_plane_to_image_plane(const twovector& r)
{
   P_ptr->put(0,r.get(0));
   P_ptr->put(1,r.get(1));
   P_ptr->put(2,1);
//         cout << "Input vector = " << *P_ptr << endl;
   *p_ptr=(*H_ptr)*(*P_ptr);
//         cout << "Output vector = " << *p_ptr << endl;
   double u=p_ptr->get(0)/p_ptr->get(2);
   double v=p_ptr->get(1)/p_ptr->get(2);
//         cout << "u = " << u << " v = " << v << endl;
   return twovector(u,v);
}

threevector homography::project_world_plane_to_image_plane(
   const threevector& r)
{
   return threevector(project_world_plane_to_image_plane(twovector(r)));
}

// ---------------------------------------------------------------------
// Member function project_ray_to_image_plane takes in 3D direction
// vector r_hat and multiplies it by *H_ptr.  This method returns the
// U & V coordinates corresponding to r_hat.  We wrote this method in
// Jan 2009 for video frame - panorama still matching purposes.

twovector homography::project_ray_to_image_plane(const threevector& r_hat)
{
//   cout << "Input vector = " << r_hat << endl;
   *p_ptr=(*H_ptr)*r_hat;
//         cout << "Output vector = " << *p_ptr << endl;
   double u=p_ptr->get(0)/p_ptr->get(2);
   double v=p_ptr->get(1)/p_ptr->get(2);
   return twovector(u,v);
}

// ---------------------------------------------------------------------
void homography::project_image_plane_to_world_plane(
   double u,double v,double& x,double& y)
{
   p_ptr->put(0,u);
   p_ptr->put(1,v);
   p_ptr->put(2,1);
//   cout << "Input uv vector = " << *p_ptr << endl;
   *P_ptr=(*Hinv_ptr)*(*p_ptr);
//   cout << "Output XY vector = " << *P_ptr << endl;
   x=P_ptr->get(0)/P_ptr->get(2);
   y=P_ptr->get(1)/P_ptr->get(2);
//   cout << "x = " << x << " y = " << y << endl;
}

twovector homography::project_image_plane_to_world_plane(const twovector& uv)
{
   p_ptr->put(0,uv.get(0));
   p_ptr->put(1,uv.get(1));
   p_ptr->put(2,1);
//         cout << "Input uv vector = " << *p_ptr << endl;
   *P_ptr=(*Hinv_ptr)*(*p_ptr);
//         cout << "Output XY vector = " << *P_ptr << endl;
   double x=P_ptr->get(0)/P_ptr->get(2);
   double y=P_ptr->get(1)/P_ptr->get(2);
//         cout << "x = " << x << " y = " << y << endl;
   return twovector(x,y);
}

// ---------------------------------------------------------------------
// Method check_homography_matrix reads in the original XY-UV
// information.  It projects the XY points down into the UV image
// plane using the 3x3 matrix *H_ptr.  This method's direct comparison
// of the projected UV values with the measured ones provides some
// indication of the projection fit's quality.

double homography::check_homography_matrix(
   vector<twovector>& XY,vector<twovector>& UV,bool print_flag)
{
   return check_homography_matrix(XY,UV,int(XY.size()),print_flag);
}

// ---------------------------------------------------------------------
double homography::check_homography_matrix(
   const vector<threevector>& XY,const vector<threevector>& UV,
   bool print_flag)
{
   vector<twovector> XY2,UV2;
   for (unsigned int n=0; n<XY.size(); n++)
   {
      XY2.push_back(twovector(XY[n]));
      UV2.push_back(twovector(UV[n]));
   }
   return check_homography_matrix(XY2,UV2,print_flag);
}

// ---------------------------------------------------------------------
double homography::check_homography_matrix(
   vector<twovector>& XY,vector<twovector>& UV,int n_inputs,bool print_flag)
{
//   cout << "inside homography::check_homography_matrix() #1" << endl;
//   cout << "XY.size() = " << XY.size()
//        << " UV.size() = " << UV.size() << endl;

   vector<int> index;
   for (unsigned int i=0; i<XY.size(); i++)
   {
      index.push_back(i);
   }
   return check_homography_matrix(XY,UV,index,n_inputs,print_flag);
}

// ---------------------------------------------------------------------
double homography::check_homography_matrix(
   vector<twovector>& XY,vector<twovector>& UV,vector<int>& index,
   unsigned int n_inputs,bool print_flag)
{
//   cout << "inside homography::check_homography_matrix() #2" << endl;
//   cout << "XY.size() = " << XY.size()
//        << " UV.size() = " << UV.size() << endl;
//   cout << "index.size() = " << index.size() << endl;

   double chisq=0;
   vector<double> d_chisq;

   XY_sorted.clear();
   UV_sorted.clear();

   for (unsigned int i=0; i<XY.size(); i++)
   {
      XY_sorted.push_back(XY[i]);
      UV_sorted.push_back(UV[i]);
      twovector q=project_world_plane_to_image_plane(XY[i]);

      d_chisq.push_back(sqr(UV[i].get(0)-q.get(0))+
                        sqr(UV[i].get(1)-q.get(1)));
      chisq += d_chisq.back();
   }
   
   templatefunc::Quicksort(d_chisq,index,XY_sorted,UV_sorted);
   for (unsigned int i=0; i<n_inputs; i++)
   {
      twovector q=project_world_plane_to_image_plane(XY_sorted[i]);

      if (print_flag)
      {
         cout << "i = " << i << " index = " << index[i] 
              << " X = " << XY_sorted[i].get(0) 
              << " Y = " << XY_sorted[i].get(1) << endl;
         cout << "Input U = " << UV_sorted[i].get(0) 
              << " projected U = " << q.get(0)
              << " input V = " << UV_sorted[i].get(1) 
              << " projected V = " << q.get(1) 
              << " sqrt(d_chisq) = " << sqrt(d_chisq[i]) 
              << endl << endl;
      }
      
   } // loop over index i 
   double RMS_residual=sqrt(chisq/n_inputs);

   cout << endl;
   cout << "===============================================" << endl;
   cout << "RMS residual between measured and calculated UV points = " 
        << RMS_residual << endl;
   cout << "===============================================" << endl;
   cout << endl;

// Store sqrt(d_chisq) within STL vector member d_chi:

   d_chi.clear();
   for (unsigned int i=0; i<d_chisq.size(); i++)
   {
      d_chi.push_back(sqrt(d_chisq[i]));
   }
//   prob_distribution prob(d_chi,100,0);
//   prob.writeprobdists(false);

   return RMS_residual;
}

// ---------------------------------------------------------------------
double homography::check_inverse_homography_matrix(
   vector<twovector>& UV,vector<twovector>& XY,unsigned int n_inputs,
   bool print_flag)
{
   cout << "inside homography::check_inverse_homography_matrix()" << endl;
//   cout << "UV.size() = " << UV.size()
//        << " XY.size() = " << XY.size() << endl;

   double chisq=0;
   vector<int> index;
   vector<double> d_chisq;

   UV_sorted.clear();
   XY_sorted.clear();

   for (unsigned int i=0; i<UV.size(); i++)
   {
      index.push_back(i);
      UV_sorted.push_back(UV[i]);
      XY_sorted.push_back(XY[i]);

      twovector q=project_image_plane_to_world_plane(UV[i]);
      d_chisq.push_back(sqr(XY[i].get(0)-q.get(0))+
                        sqr(XY[i].get(1)-q.get(1)));
      chisq += d_chisq.back();
   }
   
   templatefunc::Quicksort(d_chisq,index,UV_sorted,XY_sorted);
   for (unsigned int i=0; i<n_inputs; i++)
   {
      twovector q=project_image_plane_to_world_plane(UV_sorted[i]);

      if (print_flag)
      {
         cout << "i = " << i << " original index = " << index[i] 
              << " U = " << UV_sorted[i].get(0) 
              << " V = " << UV_sorted[i].get(1) << endl;
         cout << "Input X = " << XY_sorted[i].get(0) 
              << " projected X = " << q.get(0)
              << " input Y = " << XY_sorted[i].get(1) 
              << " projected Y = " << q.get(1) 
              << " sqrt(d_chisq) = " << sqrt(d_chisq[i]) 
              << endl << endl;
      }
      
   } // loop over index i 
   double RMS_residual=sqrt(chisq/n_inputs);

   cout << endl;
   cout << "===============================================" << endl;
   cout << "RMS residual between measured and calculated XY points = " 
        << RMS_residual << endl;
   cout << "===============================================" << endl;
   cout << endl;

   return RMS_residual;
}

// ==========================================================================
// Homography manipulation methods
// ==========================================================================

// Method compute_homography_inverse

bool homography::compute_homography_inverse()
{
   return H_ptr->inverse(*Hinv_ptr);
}

// ---------------------------------------------------------------------
// Member fucntion enforce_unit_determinant divides the 3x3 homography
// matrix H by the cube-root of its determinant.  The renormalized
// homography then has unit determinant. This boolean method returns
// false if the determinant computation fails.

bool homography::enforce_unit_determinant()
{
//   cout << "inside homography::enforce_unit_determinant" << endl;
   double det_H;
   if (!H_ptr->determinant(det_H))
   {
      return false;
   }
   else
   {
      double det_H_cuberoot=sgn(det_H)*pow(fabs(det_H),0.3333333333333);
//      cout << "detH = " << det_H << endl;
//      cout << "cuberoot = " << det_H_cuberoot << endl;
      
      *H_ptr /= det_H_cuberoot;

      H_ptr->determinant(det_H);
//      cout << "Renormalized H = " << *H_ptr << endl;
//      cout << "New det_H = " << det_H << endl;
      return true;
   }
}

/*
// ==========================================================================
// 2D UV to 2D XY planar homography methods:
// ==========================================================================

// Method compute_homography implements an SVD procedure for
// determining the invertible 3x3 matrix *H_ptr which maps planar XY
// world points to image plane UV points.  See section 3.1 in
// "Multiple View Geometry in Computer Vision" by R. Hartley and
// A. Zisserman as well as section 1.5 in "The Geometry of Multiple
// Images" by O. Faugeras and Q-T Luong for the mathematical
// derivation of this transformation.  

void homography::compute_homography(
   const vector<double>& X,const vector<double>& Y,
   const vector<double>& u,const vector<double>& v,genmatrix* H_ptr)
{
   int mdim=2*X.size();
   int ndim=9;
   genmatrix M(mdim,ndim);
   M.clear_values();

   for (unsigned int j=0; j<mdim/2; j++)
   {
      M.put(2*j,0,X[j]);
      M.put(2*j,1,Y[j]);
      M.put(2*j,2,1);
      M.put(2*j,6,-u[j]*X[j]);
      M.put(2*j,7,-u[j]*Y[j]);
      M.put(2*j,8,-u[j]);

      M.put(2*j+1,3,X[j]);
      M.put(2*j+1,4,Y[j]);
      M.put(2*j+1,5,1);
      M.put(2*j+1,6,-v[j]*X[j]);
      M.put(2*j+1,7,-v[j]*Y[j]);
      M.put(2*j+1,8,-v[j]);
   } // loop over index j labeling input XY-uv pairs
//          cout << "M = " << M << endl;

   genmatrix U(mdim,ndim);
   genmatrix W(ndim,ndim);
   genmatrix V(ndim,ndim);
   M.singular_value_decomposition(U,W,V);
//          cout << "U = " << U << " W = "  << W << " V = " << V << endl;

   vector<int> label;
   vector<double> w;
   for (unsigned int n=0; n<ndim; n++)
   {
      label.push_back(n);
      w.push_back(W.get(n,n));
   }
   templatefunc::Quicksort(w,label);
   
// Extract V-vector corresponding to smallest singular value:

   double column[ndim];
   V.get_column(label[0],column);

   cout << "Homography matrix entries:" << endl;
   for (unsigned int n=0; n<ndim; n++)
   {
      cout << column[n] << endl;
   }
   cout << endl;
          
   H_ptr->put(0,0,column[0]);
   H_ptr->put(0,1,column[1]);
   H_ptr->put(0,2,column[2]);
   H_ptr->put(1,0,column[3]);
   H_ptr->put(1,1,column[4]);
   H_ptr->put(1,2,column[5]);
   H_ptr->put(2,0,column[6]);
   H_ptr->put(2,1,column[7]);
   H_ptr->put(2,2,column[8]);
}
*/
    
// ---------------------------------------------------------------------
// Method generate_world_to_world_homography

void homography::generate_world_to_world_homography(
   const vector<threevector>& XYZ,const vector<threevector>& UVW)
{
   delete plane1_ptr;
   delete plane2_ptr;
   plane1_ptr=new plane(XYZ[0],XYZ[1],XYZ[2]);
   plane2_ptr=new plane(UVW[0],UVW[1],UVW[2]);
   vector<twovector> AB=plane1_ptr->planar_coords(XYZ);
   vector<twovector> CD=plane2_ptr->planar_coords(UVW);

   parse_homography_inputs(AB,CD);
   compute_homography_matrix();
   check_homography_matrix(AB,CD);
}
    
// ---------------------------------------------------------------------
// Method world_to_world_map

threevector homography::world_to_world_map(const threevector& XYZ)
{
   twovector AB;
   if (plane1_ptr->planar_coords(XYZ,AB))
   {
      twovector CD=project_world_plane_to_image_plane(AB);
      return plane2_ptr->world_coords(CD);
   }
   return threevector(NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
}

// ==========================================================================
// Panorama/video matching project specific member functions
// ==========================================================================

// Special purpose member function parse_homography_results reads in
// the homography results generated by Soonmin Bae from a text file.
// Soonmin's file contains the name of an individual video frame
// followed by the 9 elements which make up the homography that
// matches the frame to a background panorama image.

vector<homography*> homography::parse_homography_results_file(
   std::string input_filename)
{
   vector<homography*> homography_ptrs;
   if (!filefunc::ReadInfile(input_filename))
   {
      cout << "Error in homography::parse_homography_results_file()!" 
           << endl;
      cout << "Could not open input file " << input_filename 
           << " containing homography results" << endl;
      return homography_ptrs;
   }
   
   for (unsigned int l=0; l<filefunc::text_line.size(); l++)
   {
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[l]);
      homography* homography_ptr=new homography();
      int substr_counter=1;
      for (unsigned int i=0; i<3; i++)
      {
         for (unsigned int j=0; j<3; j++)
         {
            homography_ptr->set_H_ptr_element(
               i,j,stringfunc::string_to_number(substrings[substr_counter]));
            substr_counter++;
         }
      }

      homography_ptr->compute_homography_inverse();
//      cout << "l = " << l 
//           << " H = " << *(homography_ptr->get_H_ptr()) << endl;

      homography_ptrs.push_back(homography_ptr);
   } // loop over index l labeling lines in input file
   
   return homography_ptrs;
}
    
// ---------------------------------------------------------------------
// Member function convert_video_to_panorama_coords takes in some
// foreground video frame's (U,V) coordinates for some feature.  This
// method return the feature's corresponding (U,V) coordinates within
// the background panorama.

void homography::convert_video_to_panorama_coords(
   int video_width,int video_height,int panorama_width,int panorama_height,
   double video_u,double video_v,double& panorama_u,double& panorama_v)
{
   double video_pu = video_u * video_height;
   double video_pv = (1-video_v) * video_height;
   
//   cout << "video_pu = " << video_pu 
//        << " video_pv = " << video_pv << endl;
   
   double panorama_pu,panorama_pv;
   project_world_plane_to_image_plane(
      video_pu,video_pv,panorama_pu,panorama_pv);

//   cout << "panorama_pu = " << panorama_pu
//        << " panorama_pv = " << panorama_pv << endl;

   panorama_u = double(panorama_pu)/panorama_height;
   panorama_v = double(panorama_height-panorama_pv)/panorama_height;
      
//   cout << "panorama_u = " << panorama_u
//        << " panorama_v = " << panorama_v << endl << endl;
}

// ==========================================================================
// 3D ray projection onto 2D features (video imagery projected onto
// panoramas):
// ==========================================================================

// Member function parse_projection_inputs() takes in STL vector nhat
// containing 3D panoramic rays and STL vector UV containing
// corresponding 2D feature coordinates.  This method fills member
// genmatrix *A_ptr with combinations of these inputs.  After singular
// value decomposition of *A_ptr, we recover the homogenous 3x3 matrix
// H which maps the 3D rays onto the 2D feature coordinates.

bool homography::parse_projection_inputs(
   const vector<threevector>& nhat,const vector<twovector>& UV,
   double input_frac_to_use)
{
//   cout << "inside homography::parse_projection_inputs()" << endl;
//   cout << "input_frac_to_use = " << input_frac_to_use << endl;
   
   if (nhat.size() != UV.size())
   {
      cout << "Error in homography::parse_projection_inputs()"
           << endl;
      cout << "nhat.size() = " << nhat.size() 
           << " UV.size() = " << UV.size() << endl;
      exit(-1);
   }

// Recall that at least 4 sets of tiepoints are needed in order to
// compute a 3x3 homogenous projection:

   unsigned int n_inputs=input_frac_to_use*nhat.size();
//   cout << "n_inputs = " << n_inputs << endl;
   if (n_inputs < 4)
   {
      return false;
   }

   int nrows=2*n_inputs;
   int ncolumns=9;
   delete A_ptr;
   A_ptr=new genmatrix(nrows,ncolumns);

   for (unsigned int i=0; i<n_inputs; i++)
   {
      double nx=nhat[i].get(0);
      double ny=nhat[i].get(1);
      double nz=nhat[i].get(2);
      double u=UV[i].get(0);
      double v=UV[i].get(1);

//      cout << "i = " << i
//           << " nx = " << nx << " ny = " << ny << " nz = " << nz  
//           << " U = " << u << " V = " << v << endl;

      int row=i*2+0;
      A_ptr->put(row,0,nx);
      A_ptr->put(row,1,ny);
      A_ptr->put(row,2,nz);

      A_ptr->put(row,3,0);
      A_ptr->put(row,4,0);
      A_ptr->put(row,5,0);

      A_ptr->put(row,6,-u*nx);
      A_ptr->put(row,7,-u*ny);
      A_ptr->put(row,8,-u*nz);

      row++;

      A_ptr->put(row,0,0);
      A_ptr->put(row,1,0);
      A_ptr->put(row,2,0);

      A_ptr->put(row,3,nx);
      A_ptr->put(row,4,ny);
      A_ptr->put(row,5,nz);

      A_ptr->put(row,6,-v*nx);
      A_ptr->put(row,7,-v*ny);
      A_ptr->put(row,8,-v*nz);

   } // loop over index i
   
//   cout << "A = " << *A_ptr << endl;
//   cout << "A.mdim = " << A_ptr->get_mdim() << " A.ndim = " 
//        << A_ptr->get_ndim() << endl;

   return true;
}

// ---------------------------------------------------------------------
// Member function check_ray_projection takes in STL vector nhat
// containing 3D panoramic rays and STL vector UV containing 2D
// feature coordinates.  This method projects the former via the 3x3
// homogeneous matrix H and compares the results with the latter.  The
// input STL vectors are sorted according to the residual differences
// between the input and projected UV coordinates and returned within
// member STL vectors nhat_sorted and UV_sorted.

double homography::check_ray_projection(
   const vector<threevector>& nhat,const vector<twovector>& UV, 
   double input_frac_to_use,bool print_flag)
{
   double chisq=0;
   vector<int> label;
   vector<double> d_chisq;

   nhat_sorted.clear();
   UV_sorted.clear();
   for (unsigned int i=0; i<nhat.size(); i++)
   {
      label.push_back(i);
      nhat_sorted.push_back(nhat[i]);
      UV_sorted.push_back(UV[i]);

      twovector q=project_ray_to_image_plane(nhat[i]);
      d_chisq.push_back(sqr(UV[i].get(0)-q.get(0))+
                        sqr(UV[i].get(1)-q.get(1)));
      chisq += d_chisq.back();
   }
   
   templatefunc::Quicksort(d_chisq,label,nhat_sorted,UV_sorted);

   unsigned int n_inputs=input_frac_to_use*nhat.size();
   for (unsigned int i=0; i<n_inputs; i++)
   {
      twovector q=project_ray_to_image_plane(nhat_sorted[i]);

      if (print_flag)
      {
         cout << "i = " << i << " label = " << label[i] 
              << " nx = " << nhat_sorted[i].get(0)
              << " ny = " << nhat_sorted[i].get(1)
              << " nz = " << nhat_sorted[i].get(2) << endl;
         cout << "Input U = " << UV_sorted[i].get(0) 
              << " projected U = " << q.get(0)
              << " input V = " << UV_sorted[i].get(1) 
              << " projected V = " << q.get(1) 
              << " sqrt(d_chisq) = " << sqrt(d_chisq[i]) 
              << endl << endl;
      }
      
   } // loop over index i 
   double RMS_residual=sqrt(chisq/n_inputs);

   cout << endl;
   cout << "===============================================" << endl;
   cout << "RMS residual between measured and calculated UV points = " 
        << RMS_residual << endl;
   cout << "===============================================" << endl;
   cout << endl;

   return RMS_residual;
}

// ==========================================================================
// Camera parameter estimation member functions
// ==========================================================================

// Member function compute_camera_params_from_zplane_homography() takes in 
// imageplane center point (u0,v0).  It also works with the current
// homography which is assumed to have been derived from matching an
// aerial image of a ground Z-plane.  It computes a 7-parameter
// pinhole model approximation for the camera (f, rotation,
// translation) and returns its projection matrix P.

void homography::compute_camera_params_from_zplane_homography(
   double u0,double v0,
   double& f,double& az,double& el,double& roll,threevector& camera_posn,
   genmatrix& P)
{
//   cout << "inside homography::compute_camera_params_from_zplane_homography()"
//        << endl;
   P.clear_values();
   genmatrix H_curr(3,3);
   double H_22=get_H_ptr()->get(2,2);
   H_curr=*(get_H_ptr()) / H_22;
   threevector p0,p1,p3;
   H_curr.get_column(0,p0);
   P.put_column(0,p0);
   H_curr.get_column(1,p1);
   P.put_column(1,p1);
   H_curr.get_column(2,p3);
   P.put_column(3,p3);
      
   double r3=P.get(2,0);
   double r6=P.get(2,1);
   double t3=P.get(2,3);
   double fr1=P.get(0,0)-u0*r3;
   double fr2=P.get(1,0)-v0*r3;
   double fr4=P.get(0,1)-u0*r6;
   double fr5=P.get(1,1)-v0*r6;
   double ft1=P.get(0,3)-u0*t3;
   double ft2=P.get(1,3)-v0*t3;
//   double sqr_f=-( fr1*fr4 + fr2*fr5 ) / (r3*r6);
   double sqr_f=fabs( ( fr1*fr4 + fr2*fr5 ) / (r3*r6) );
   cout << "sqr_f = " << sqr_f << endl;
   f=-sqrt(sqr_f);

   double r1=fr1/f;
   double r2=fr2/f;
   double r4=fr4/f;
   double r5=fr5/f;
   double t1=ft1/f;
   double t2=ft2/f;
      
   genmatrix K(3,3),Kinv(3,3);
   K.clear_values();
   K.put(0,0,f);
   K.put(1,1,f);
   K.put(2,2,1);
   K.put(0,2,u0);
   K.put(1,2,v0);
   K.inverse(Kinv);

   threevector ahat(r1,r2,r3);
   threevector bhat(r4,r5,r6);
   double lambda=0.5*(ahat.magnitude()+bhat.magnitude());
   ahat=ahat.unitvector();
   bhat=bhat.unitvector();

//   cout << "ahat = " << ahat << endl;
//   cout << "bhat = " << bhat << endl;
//   cout << "ahat.ahat = " << ahat.dot(ahat) << endl;
//   cout << "bhat.bhat = " << bhat.dot(bhat) << endl;
//   cout << "ahat.bhat = " << ahat.dot(bhat) << endl;
//   cout << "lambda = " << lambda << endl;

   threevector chat=ahat.cross(bhat);
   threevector t(t1,t2,t3);
   t /= lambda;

// We assume aerial camera always has positive altitude!  So
// effectively fix homography's sign via this requirement:

   double camera_posn_z=-chat.dot(t);
   if (camera_posn_z < 0)
   {
      ahat *= -1;
      bhat *= -1;
      t *= -1;
   }

   rotation Rcamera;
   Rcamera.put_column(0,ahat);
   Rcamera.put_column(1,bhat);
   Rcamera.put_column(2,chat);

   rotation R0;
   R0.clear_values();
   R0.put(0,2,-1);
   R0.put(1,0,-1);
   R0.put(2,1,1);

   rotation R=Rcamera.transpose() * (R0.transpose());
   R.az_el_roll_from_rotation(az,el,roll);

   camera_posn=-Rcamera.transpose()*t;

   genmatrix Rt(3,4);
   Rt.put_column(0,ahat);
   Rt.put_column(1,bhat);
   Rt.put_column(2,chat);
   Rt.put_column(3,t);

   P=K*Rt;

   cout << "K = " << K << endl;
   cout << "R = " << R << endl;
   cout << "R*Rtrans = " << R*R.transpose() << endl;
   cout << "Az = " << az*180/PI
        << " el = " << el*180/PI
        << " roll = " << roll*180/PI << endl;
   cout << "t = " << t << endl;
   cout << "camera_posn = " << camera_posn << endl;
   cout << "Recovered  P = " << P << endl;
}

// ---------------------------------------------------------------------
void homography::compute_extrinsic_params(
   double f,double U0,double V0,
   rotation& R,threevector& camera_posn)
{
   genmatrix K(3,3),Kinv(3,3);
   K.identity();
   K.put(0,0,f);
   K.put(0,2,U0);
   K.put(1,1,f);
   K.put(0,2,V0);

   K.inverse(Kinv);

   threevector h0,h1,h2;
   H_ptr->get_column(0,h0);
   H_ptr->get_column(1,h1);
   H_ptr->get_column(1,h2);
   
   double lambda0=1.0/(Kinv*h0).magnitude();
   double lambda1=1.0/(Kinv*h1).magnitude();
   double lambda2=0.5*(lambda0+lambda1);

   threevector r0_hat=lambda0*Kinv*h0;
   threevector r1_hat=lambda1*Kinv*h1;
   threevector r2_hat=r0_hat.cross(r1_hat);
   
   R.put_column(0,r0_hat);
   R.put_column(1,r1_hat);
   R.put_column(2,r2_hat);

   threevector t=lambda2*Kinv*h2;
   camera_posn=-R.transpose()*t;
}

// ==========================================================================
// Import/export member functions
// ==========================================================================

void homography::export_matrix(string output_filename)
{
   H_ptr->export_to_dense_text_format(output_filename);
}

void homography::import_matrix(string input_filename)
{
   genmatrix* imported_H_ptr=
      mathfunc::import_from_dense_text_format(input_filename);
   *H_ptr=*imported_H_ptr;
   delete imported_H_ptr;
   compute_homography_inverse();
}

// ---------------------------------------------------------------------
// Member function check_tieline_homography_matrix()

double homography::check_tieline_homography_matrix(
   const vector<threevector>& abc,const vector<threevector>& def)
{
   unsigned int n_lines=abc.size();
   for (unsigned int i=0; i<n_lines; i++)
   {
      threevector abc_calc=H_ptr->transpose()*def[i];      
      abc_calc /= abc_calc.get(2);
      cout << "line i = " << i << endl;
      cout << "input abc = " << abc[i]/abc[i].get(2) << endl;
      cout << "calculated abc = " << abc_calc << endl;
   }

   double RMS_residual=0;
   return RMS_residual;
}
