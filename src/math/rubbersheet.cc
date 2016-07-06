// ==========================================================================
// Rubbersheet class member function definitions
// ==========================================================================
// Last modified on 2/6/08; 9/14/08
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "math/genmatrix.h"
#include "math/rubbersheet.h"
#include "templates/mytemplates.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void rubbersheet::allocate_member_objects()
{
   M_ptr=new genmatrix(2,2);
   Minv_ptr=new genmatrix(2,2);
}		       

void rubbersheet::initialize_member_objects()
{
}

rubbersheet::rubbersheet()
{
   allocate_member_objects();
   initialize_member_objects();
}

rubbersheet::rubbersheet(const vector<twovector>& xy,
                         const vector<twovector>& uv)
{
   allocate_member_objects();
   initialize_member_objects();

   n_features=xy.size();
   for (unsigned int i=0; i<n_features; i++)
   {
      int ID=i;
      XYID.push_back(threevector(xy[i],ID));
   }

   for (unsigned int i=0; i<uv.size(); i++)
   {
      UV.push_back(uv[i]);
   }

   if (n_features != uv.size())
   {
      cout << "Warning in rubbersheet constructor!" << endl;
      cout << "xy.size() = " << xy.size() << " differs from uv.size() = "
           << uv.size() << endl;
   }
}

rubbersheet::rubbersheet(const vector<threevector>& xyid,
                         const vector<twovector>& uv)
{
   allocate_member_objects();
   initialize_member_objects();

   n_features=xyid.size();
   for (unsigned int i=0; i<n_features; i++)
   {
      XYID.push_back(xyid[i]);
   }

   for (unsigned int i=0; i<uv.size(); i++)
   {
      UV.push_back(uv[i]);
   }

   if (n_features != uv.size())
   {
      cout << "Warning in rubbersheet constructor!" << endl;
      cout << "xyid.size() = " << xyid.size() 
           << " differs from uv.size() = " << uv.size() << endl;
   }
}

// Copy constructor:

rubbersheet::rubbersheet(const rubbersheet& r)
{
   allocate_member_objects();
   initialize_member_objects();
   docopy(r);
}

rubbersheet::~rubbersheet()
{
   delete M_ptr;
   delete Minv_ptr;
}

// ---------------------------------------------------------------------
void rubbersheet::docopy(const rubbersheet& r)
{
}

// Overload = operator:

rubbersheet& rubbersheet::operator= (const rubbersheet& r)
{
   if (this==&r) return *this;
   docopy(r);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const rubbersheet& r)
{
   outstream << endl;

//   cout << "inside rubbersheet::operator<<" << endl;
//   cout << "&p = " << &p << endl;
//   cout << "sizeof(p) = " << sizeof(p) << endl;

   return(outstream);
}

// ---------------------------------------------------------------------
void rubbersheet::copy_XYID(const vector<threevector>& xyid)
{
   XYID.clear();
   for (unsigned int i=0; i<xyid.size(); i++)
   {
      XYID.push_back(xyid[i]);
   }
   n_features=XYID.size();
}

void rubbersheet::copy_UV(const vector<twovector>& uv)
{
   UV.clear();
   for (unsigned int i=0; i<uv.size(); i++)
   {
      UV.push_back(uv[i]);
   }
   if (UV.size() != n_features)
   {
      cout << "Warning in rubbersheet::copy_UV()" << endl;
      cout << "n_features = " << n_features << " differs from uv.size() = "
           << uv.size() << endl;
   }
}

// ==========================================================================
// Warping member functions
// ==========================================================================

// Member function fit_linear_warp implements the closed-form least
// squares solution to the linear system UV = M XY + T for the 4
// parameters in M and the 2 translations in T:

void rubbersheet::fit_linear_warp(vector<bool>& retained_feature_flag)
{
//   cout << "inside rubbersheet::fit_linear_warp()" << endl;
   
   double X,Y,U,V;
   double Asq,Bsq,AB,AC,AD,BC,BD;
   X=Y=U=V=0;
   Asq=Bsq=AB=AC=AD=BC=BD=0;
   
   unsigned int n_used_features=0;
   for (unsigned int i=0; i<n_features; i++)
   {
      if (!retained_feature_flag[i]) continue;
      n_used_features++;
      double x=XYID[i].get(0);
      double y=XYID[i].get(1);
      double u=UV[i].get(0);
      double v=UV[i].get(1);

      X += x;
      Y += y;
      U += u;
      V += v;

      Asq += sqr(x);
      Bsq += sqr(y);
      AB += x*y;
      AC += x*u;
      AD += x*v;
      BC += y*u;
      BD += y*v;
   } // loop over index i labeling features
   
   double Isq=Asq-sqr(X)/n_used_features;
   double IJ=AB-X*Y/n_used_features;
   double Jsq=Bsq-sqr(Y)/n_used_features;
   double IK=AC-X*U/n_used_features;
   double JK=BC-Y*U/n_used_features;
   double IL=AD-X*V/n_used_features;
   double JL=BD-Y*V/n_used_features;
   
   double denom=Isq*Jsq-sqr(IJ);

   M_ptr->put(0,0,(Jsq*IK-IJ*JK)/denom);
   M_ptr->put(0,1,(-IJ*IK+Isq*JK)/denom);
   M_ptr->put(1,0,(Jsq*IL-IJ*JL)/denom);
   M_ptr->put(1,1,(-IJ*IL+Isq*JL)/denom);
   cout << "*M_ptr = " << *M_ptr << endl;

   if (M_ptr->inverse(*Minv_ptr))
   {
      cout << "*Minv_ptr = " << *Minv_ptr << endl;
      cout << "M*Minv = " << (*M_ptr) * (*Minv_ptr) << endl;
   }

   trans.put(0,(U-M_ptr->get(0,0)*X-M_ptr->get(0,1)*Y)/n_used_features);
   trans.put(1,(V-M_ptr->get(1,0)*X-M_ptr->get(1,1)*Y)/n_used_features);
   cout << "Trans = " << trans << endl;
}

// ---------------------------------------------------------------------
// Method compare_measured_and_linearly_transformed_UVs takes in the
// fitted entries in matrix *M_ptr and vector trans.  It then computes
// the differences between the fitted and manually measured UV points.
// This method returns the RMS residual between the fitted and
// measured positions which provides a quantitative measure of the
// goodness-of-fit.

double rubbersheet::compare_measured_and_linearly_transformed_UVs(
   int npoints_to_discard,vector<bool>& retained_feature_flag)
{
   string banner="Comparing measured and linearly transformed UVs";
   outputfunc::write_big_banner(banner);

   unsigned int n_used_features=0;
   vector<double> d_chisq,delta_mag;
   d_chisq.reserve(n_features);
   delta_mag.reserve(n_features);
   for (unsigned int i=0; i<n_features; i++)
   {
      if (!retained_feature_flag[i]) continue;
      n_used_features++;
      twovector curr_XY ( XYID[i].get(0) , XYID[i].get(1) );
      twovector UV_transformed( *M_ptr * curr_XY + trans );
      twovector Delta(UV[i] - UV_transformed);
      d_chisq.push_back(Delta.sqrd_magnitude());

//      cout << "i = " << i 
//           << " ID = " << XYZID[i].get(2)
//           << " Utrans = " << UV_transformed.get(0)
//           << " U = " << UV[i].get(0)
//           << " Vtrans = " << UV_transformed.get(1)
//           << " V = " << UV[i].get(1) 
//           << " Delta.mag = " << delta_mag.back()
//           << endl;

   } // loop over index i 
   cout << endl;

   templatefunc::Quicksort(d_chisq,XYID,UV);
   for (unsigned int i=0; i<n_features; i++)
   {
      if (!retained_feature_flag[i]) continue;
      twovector curr_XY ( XYID[i].get(0) , XYID[i].get(1) );
      twovector UV_transformed( *M_ptr * curr_XY + trans );
      twovector Delta(UV[i] - UV_transformed);
      delta_mag.push_back(Delta.magnitude());

      cout << "i = " << i 
           << " ID = " << XYID[i].get(2)
           << " Utrans = " << UV_transformed.get(0)
           << " U = " << UV[i].get(0)
           << " Vtrans = " << UV_transformed.get(1)
           << " V = " << UV[i].get(1) 
           << " Delta.mag = " << delta_mag.back()
           << endl;
   } // loop over index i 
   cout << endl;

   double mean=mathfunc::mean(delta_mag);
   double std_dev=mathfunc::std_dev(delta_mag);
   cout << "Residual = " << mean << " +/- " << std_dev << endl;

   retained_feature_flag.clear();
   for (unsigned int i=0; i<n_features; i++)
   {
      retained_feature_flag.push_back(false);
   }
   
   double chisq=0;
   for (unsigned int i=0; i<n_features-npoints_to_discard; i++)
   {
      chisq += d_chisq[i];
      retained_feature_flag[i]=true;
   }

   double MS_residual=chisq/(n_features-npoints_to_discard);
   double RMS_residual=sqrt(MS_residual);
   cout << "RMS_residual = " << RMS_residual << endl;
   
   return RMS_residual;
}
