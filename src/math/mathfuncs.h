// Note added on 5/25/14: Should really globally rename
// recursive_mean, recursive_second_moment etc as incremental_mean,
// incremental_second_moment, ...


// Note added on 12/6/11: Need to add a mean method for STL vector of
// threevectors !!!

// ==========================================================================
// Header file for stand-alone "primitive" math functions.
// ==========================================================================
// Last updated on 8/23/16; 10/18/16; 12/19/16; 12/23/16
// ==========================================================================

#ifndef MATHFUNCS_H
#define MATHFUNCS_H

#include <deque>
#include <iostream>
#include <vector>
#include <flann/flann.hpp>
#include "math/basic_math.h"
#include "math/constants.h"
#include "gmm/gmm.h"
#include "gmm/gmm_matrix.h"
#include "numrec/nr.h"

class descriptor;
class genmatrix;
class genvector;
class threevector;
class twovector;
class fourvector;

namespace mathfunc
{

// Base-32 methods:

   std::string encode_base32(int i);
   int decode_base32(std::string s);

// Casting methods:

   float double_to_float(const double& d);

// Combinatoric methods:

   double correlation_coeff(int npoints,double x[],double y[]);
   int choose_numerator(int n,int i);
   int choose(int n,int i);
   int factorial(int n);
   double Gamma(double x);
   double lower_incomplete_gamma(double s, double z);
   double cumulative_chisq_prob(double x, double k);
   double stirling(double x);

// Cubic polynomial methods:

   double cubic_poly_discriminant(const std::vector<double>& coeffs);
   double cubic_poly_discriminant(double a,double b,double c,double d);
   int n_real_cubic_roots(double Delta);
   std::vector<double> real_cubic_roots(const std::vector<double>& coeffs);
   std::vector<double> real_cubic_roots(double a,double b,double c,double d);
   
// Digit methods:

   std::vector<int> digit_decomposition(int number);
   int digit_sum(int number);
   int ndigits_before_decimal_point(double x);
   int ndigits_after_decimal_point(double x);

// Discrete integer methods:

   long long unique_integer(long long i,long long j,long long Imax);
   void decompose_unique_integer(
      long long unique_int,long long Imax,
      long long& i, long long& j);
   long long unique_integer(
      long long i,long long j,long long k,long long l,
      long long Imax,long long Jmax,long long Kmax);
   void decompose_unique_integer(
      long long unique_int,long long Imax,long long Jmax,long long Kmax,
      long long& i, long long& j,long long& k,long long& l);

// Distribution methods:

   double gaussian(double x,double mu,double sig);   
   double gaussian_random_var(double mu, double sigma);
   void lognormal(double mu,double sigma,double& r1,double& r2);

// Factoring methods:

   int* factor_integer(int number,int& nfactors);
   int gcd(int m, int n);
   bool is_prime(int n);
   int lcm(int m, int n);
   std::vector<int> prime_factors(int number);

// Fitting methods:

   double linefit(double x,double x1,double y1,double x2,double y2);
   double linefit(double x,double x1,double y1,double m);
   void fit_plane(int n,double x[],double y[],double z[],
                  double& a,double& b,double &c);
   void simple_linear_fit(
      std::vector<double>& x,std::vector<double>& y,double& a,double& b);
//   void quadratic_fit(const std::vector<twovector>& P,double& a,double& b,
//                      double& c);
/*
   double fit_2D_affine_transformation(
      const std::vector<twovector>& q_vecs,
      const std::vector<twovector>& p_vecs,genmatrix& A,twovector& trans);
   double score_2D_affine_transformation_fit(
      const std::vector<twovector>& q_vecs,
      const std::vector<twovector>& p_vecs,
      const genmatrix& A,const twovector& trans);
*/

   double fit_2D_translation(
      const std::vector<twovector>& q_vecs,
      const std::vector<twovector>& p_vecs,genmatrix& A,twovector& trans);

// Quadrature methods:

   double simpsonsum(double f[],int startbin,int stopbin);
   double simpsonsum(const std::vector<double>& f);
   double simpsonsum(const std::vector<double>& f,int startbin,int stopbin);

// Angle computation methods:

   double phase_to_continuous_value(
      double phase,double prev_phase,bool degrees=true);
   double absolute_angle_between_unitvectors(
      const threevector& u_hat,const threevector& v_hat);
   std::pair<double,threevector> angle_and_axis_between_unitvectors(
      const threevector& u_hat,const threevector& v_hat);
   double angle_between_unitvectors(
      const threevector& u_hat,const threevector& v_hat);

// Direction vector & basis construction methods:

   void decompose_direction_vector(
      const threevector& n_hat,double& phi,double& theta);
   threevector construct_direction_vector(double phi,double theta);
   void generate_orthogonal_basis(const threevector& n_hat,threevector& p_hat,
                                  threevector& q_hat);

// Orthogonal matrix and quaternion methods:

   void decompose_orthogonal_matrix(
      const genmatrix& R,double& chi,threevector& n_hat,double TINY=1E-9);
   void decompose_orthogonal_matrix(
      const genmatrix& R,double& theta,double& phi,double& chi,
      double TINY=1E-9);
   void construct_orthogonal_matrix(
      double det,const threevector& n_hat,double chi,genmatrix& R);
   void construct_orthogonal_matrix(
      double det,double theta,double phi,double chi,genmatrix& R);

   void az_el_roll_corresponding_to_quaternion(
      const fourvector& q,double& az,double& el,double& roll);
   fourvector quaternion_corresponding_to_az_el_roll(
      double az,double el,double roll);

// Probability distribution methods:

   std::vector<double> find_local_peaks(
      const std::vector<double>& Z,double peak_width,unsigned int n_max_peaks);
   double find_local_peak(const std::vector<double>& Z);
   double find_x_corresponding_to_pcum(
     const std::vector<double>& x, const std::vector<double>& pcum, 
     double cumprob);
   void contrast_normalize_histogram(unsigned int H,float* histogram);

// Random integer sequence methods:

   int getRandomInteger(int N);
   std::vector<int> random_sequence(int nsize);   
   std::vector<int> random_sequence(int nsize,int sequence_length);
   std::vector<int> random_sequence(int istart,int istop,int sequence_length);
   std::vector<int> get_next_random_int_sequence(int n_integers);
   std::vector<int> get_next_random_int_sequence(
      int n_integers,int prime_number);

// Set methods:

   void intersect_sorted_integers(
      int *A, int *B, int l_a, int l_b, 
      int* A_and_B, int* both_count_ptr,
      int* only_A, int* only_A_count_ptr,
      int* only_B, int* only_B_count_ptr);
   int difference_between_sorted_integers(
      int *A, int *B, int l_a, int l_b, int* only_A);

// Sparse matrix methods:

   genvector* import_genvector_from_dense_text_format(
      std::string input_filename);
   genmatrix* import_from_dense_text_format(std::string input_filename);
   genmatrix* import_from_diagonal_text_format(
      int mdim,std::string input_filename);
   void export_to_sparse_text_format(
      gmm::row_matrix< gmm::wsvector<float> >* sparse_matrix_ptr,
      int n_nonzero_values,std::string output_filename);
   void export_to_sparse_binary_format(
      gmm::row_matrix< gmm::wsvector<float> >* sparse_matrix_ptr,
      int n_nonzero_values,std::string output_filename);
   void sparse_SVD_approximation(
      gmm::row_matrix< gmm::wsvector<float> >* sparse_matrix_ptr,
      int k_dims,int n_nonzero_values,std::string output_subdir);

// Spline methods:

   void spline_interp(
      std::vector<double>& t,std::vector<threevector>& XYZ,
      std::vector<double>& t_reg,std::vector<threevector>& interp_XYZ);
   void spline_interp(
      std::vector<double>& t,std::vector<fourvector>& WXYZ,
      std::vector<double>& t_reg,std::vector<fourvector>& interp_WXYZ);
   double smoothed_ramp(double frac,double t0,double t4,double y0,double y4,
                        double t);

// Median value and filter methods:

   int median_value(int a,int b,int c);
   double median_value(const std::vector<double>& A);
   float median_value(const std::vector<float>& A);
   void lo_hi_values(
      const std::vector<double>& A,double lo_frac, double hi_frac, 
      double& lo_value,double& hi_value);
   void lo_hi_values(
      const std::vector<double>& A,double& level_25,double& level_75);
   void percentile_values(
      const std::vector<double>& A, 
      const std::vector<double>& percentile_fracs, 
      std::vector<double>& values);
   void median_value_and_percentile_width(
      const std::vector<double>& A,double frac_from_median, 
      double& median,double& percentile_width);
   void median_value_and_quartile_width(
      const std::vector<double>& A,double& median,double& quartile_width);
   void median_value_and_quartile_width(
      const std::vector<threevector>& RGB,
      threevector& median,threevector& quartile_width);
   double median_value(int nbins,double zarray[]);
   void median_filter(int nbins,int window_size,double ain[],double amed[]);
   std::vector<double> circular_median_filter(
      const std::vector<double>& A,int window_size);

// Statistical methods:

   double error_function(double x);
   double poor_man_erf(double x);

   double maximal_value(const std::vector<double>& A);
   double minimal_value(const std::vector<double>& A);
   float maximal_value(const std::vector<float>& A);
   float minimal_value(const std::vector<float>& A);

   double mean(const std::vector<double>& A);
   double std_dev(const std::vector<double>& A);
   void mean_and_std_dev(
      const std::vector<double>& A,double& mean,double& std_dev);
   void mean_and_std_dev(
      const std::deque<double>& A,double& mean,double& std_dev);

   double variance(double A[],const int Nsize);
   double variance(double A[],double w[],const int Nsize);
   double weighted_mean(
      const std::vector<double>& X,const std::vector<double>& sigma);
   double weighted_std_dev(
      const std::vector<double>& X,const std::vector<double>& sigma);
   void moment_of_inertia_2D(
      const twovector& origin,
      double& Ixx,double& Iyy,double& Ixy,double& Imin,double& Imax,
      const std::vector<twovector>& R);
   void moment_of_inertia_2D(
      const twovector& origin,double& Imin,double& Imax,
      twovector& Imin_hat,twovector& Imax_hat,
      const std::vector<twovector>& R);
   double moment_of_inertia_bbox_area(
      const twovector& origin,
      const twovector& Imin_hat,const twovector& Imax_hat,
      const std::vector<twovector>& R);

// 1D incremental statistical methods:

   double incremental_mean(int n, double curr_x, double prev_mean);
   double incremental_std_dev(
      int n, double curr_x, double prev_mean, double prev_std_dev);
   double combined_mean(unsigned int a, unsigned int b, 
                        double mean_a, double mean_b);
   double combined_std_dev(unsigned int a, unsigned int b, 
                          double mean_a, double mean_b,
			  double std_dev_a, double std_dev_b);
   void mean_std_dev_for_subset(
      unsigned int a, unsigned int b,
      double mean_a, double std_dev_a,
      double mean_ab, double std_dev_ab,
      double& mu_b, double& sigma_b);

// N-dimensional statistical methods:

   genvector* mean(const std::vector<genvector*>& A);
   void recursive_mean(int n,const genvector* A_ptr,genvector* recur_mean_ptr);
   void recursive_mean(
      int n,const descriptor* A_ptr,descriptor* recur_mean_ptr);
   void recursive_mean(
      int mdim,int n,const genvector* A_ptr,genvector* recur_mean_ptr);
   void recursive_mean(
      int mdim,int n,const descriptor* A_ptr,descriptor* recur_mean_ptr);

   void recursive_second_moment(
      int n,const genvector* A_ptr,genmatrix* recur_second_moment_ptr);
   void recursive_2nd_moment(
      int n,const descriptor* A_ptr,genmatrix* recur_second_moment_ptr);
   void recursive_second_moment(
      int mdim,int n,const genvector* A_ptr,
      genmatrix* recur_second_moment_ptr);
   void recursive_2nd_moment(
      int mdim,int n,const descriptor* A_ptr,
      genmatrix* recur_second_moment_ptr);
   void recursive_second_moment(
      int n,const descriptor* A_ptr,
      flann::Matrix<float>* recur_second_moment_ptr);

   genmatrix* covariance_matrix(const std::vector<genvector*>& A);
   void recursive_covariance_matrix(
      int n,const genvector* A_ptr,const genvector* mean_ptr,
      genmatrix* recur_covar_ptr);
   void recursive_covariance_matrix(
      int n,const genvector* A_ptr,const genmatrix& mean_outerprod,
      genmatrix* recur_covar_ptr);

// N-dimensional statistical methods working on C arrays:

   void incremental_mean_vec(
      unsigned int mdim, unsigned int n, float *X, double *incremental_mean);
   void incremental_second_vec_moment(
      unsigned int mdim, unsigned int n, float *X, 
      double **incremental_second_moment);
   void covar_matrix(
      unsigned int mdim, double *mean, double **second_moment, 
      double **covar_matrix);

   double Mahalanobis_distance(
      const genvector* X_ptr,const genvector* Y_ptr,
      const genmatrix* inverse_covariance_matrix_ptr);

// Tensor methods:

   int LeviCivita(int i,int j,int k);

// Trigonometry methods:

   double cosinv(double x);
   double coshinv(double x);
   double myatan2(double y, double x, double prevangle);

// Miscellaneous methods:

   void addarray(double A1[],double A2[],double Asum[],int Nsize);
   double approx_ellipse_circumference(double a,double b);
   int binary_locate(const std::vector<double>& data,
                     int startbin,int stopbin,double x_in);
   int binary_locate(double data[],int startbin,int stopbin,double x_in);
   int brute_force_auto_correlation(
      int min_shift,int max_shift,const std::vector<double>& f);
   double cum_value(double frac,const std::vector<double>& z);
   double dB(double x);
   double dBinv(double y);
   void find_ordering(int nbins,double A[],int order[]);
   int generate_skip_sequence(int jbegin,int jend,int jperiod,int jsequence[],
                              bool ascending_sequence=true);

   void minmax(double x1,double x2,double& xmin,double& xmax);
   void multarray(double A1[],double A2[],double Aprod[],int Nsize);
   template <class T> int mylocate(const std::vector<T>& data,T x_in);
   template <class T> int mylocate(
      const std::vector<T>& data,int startbin,int stopbin,T x_in);
   bool my_isnan(double x);
   int negonepower(int n);
   double power_of_two(int n);

   double real_power(double x,int n);
   void second_derivs(double xstep,double ystep,double f[3][3],
                      double& fxx,double& fyy,double& fxy);

   namespace errorfunc
      {
         void initialize_fast_error_function();
         double fast_error_function(double x);
      }

// ==========================================================================
// Inlined methods:
// ==========================================================================

   inline double dBinv(double y)
      {
         return pow(10.0,y/10.0);
      }

// ---------------------------------------------------------------------
// Note: Our original simple minded recursive factorial routine
// overflows for n > 12.  So on 8/4/99, we replaced it with a call to
// Gamma(n+1) which appears to return valid factorials up to n=170.
// For n > 170, Gamma(n+1) returns "Infinity".  

// In order to speed up processing, we hard-wire in factorial results
// for the first 12 integers:

   inline int factorial(int n)
      {
         const int nfactorials=12;
         static int f[nfactorials+1]=
         {1,1,2,6,24,120,720,5040,40320,362880,3628800,39916800,479001600
         };
         
         if (n >= 0 && n <= nfactorials)
         {
            return f[n];
         }
//         else if (n >= nfactorials+1 && n < 170)
//         {
//            return Gamma(n+1);
//         }
         else
         {
            std::cout << 
               "Method mathfunc::factorial() can't return value for ";
            std::cout << "n = " << n << " !!!" << std::endl;
            return POSITIVEINFINITY;
         }
      }

// ---------------------------------------------------------------------
// Method Gamma returns the special function value Gamma(x)

// Note added on 8/10/15: C's math library provides gamma function by
// calling tgamma(x).  Its returned values empirically match those
// from Numerical Recipe's routine to at least 8 significant digits.

   inline double Gamma(double x)
      {
         if (x > 1)
         {
            return exp(numrec::gammln(x));
         }
         else 
         {
            return Gamma(x+1)/x;
         }
      }

// ---------------------------------------------------------------------
// Method gcd recursively computes the greatest common divisor of any 
// two nonzero integers m and n using Euclid's algorithm.	

   inline int gcd(int m, int n)
      {
         if (m%n==0) 
         {
            return(n);
         }
         else
         {
            return(gcd(n,m%n));
         }
      }

// ---------------------------------------------------------------------
// Method lcm returns the least common multiple of any two integers by 
// calling Euclid's greatest common divisor algorithm

   inline int lcm(int m, int n)
      {
         return(m*n/gcd(m,n));
      }

// ---------------------------------------------------------------------
// Methods linefit take in either two 2D points on a line or else one
// 1D point plus a slope m of a line along with an x coordinate.  They
// return the corresponding y coordinate:

   inline double linefit(double x,double x1,double y1,double x2,double y2)
      {
         double m=(y2-y1)/(x2-x1);
         return linefit(x,x1,y1,m);
      }

   inline double linefit(double x,double x1,double y1,double m)
      {
         return y1+m*(x-x1);
      }

// ---------------------------------------------------------------------
   inline void minmax(double x1,double x2,double& xmin,double& xmax)
      {
         if (x1 < x2)
         {
            xmin=x1;
            xmax=x2;
         }
         else
         {
            xmin=x2;
            xmax=x1;
         }
      }

// ---------------------------------------------------------------------
// Method mylocate takes in an STL vector data which is monotonically
// increasing or decreasing over a range between starting and ending
// bins startbin and stopbin.  It performs a binary search to locate
// the index i for which the specified value x_in is bracketed as
// data[i] <= x_in < data[i+1] or data[i] >= x > data[i+1].  If x <
// data[startbin] or x > data[stopbin], then startbin-1 and stopbin+1
// are respectively returned.  This method is basically a rewrite of
// the Numerical Recipes "locate" routine (which is badly written and
// sometimes buggy!)

   template <class T> int mylocate(const std::vector<T>& data,T x_in)
      {
         return mylocate(data,0,data.size()-1,x_in);
      }

   template <class T> int mylocate(
      const std::vector<T>& data,int startbin,int stopbin,T x_in)
      {
         int nsize=stopbin-startbin+1;
         std::vector<double> datacopy;
   
         T maxvaluediff=data[stopbin]-data[startbin];
         for (int i=0; i<nsize; i++)
         {
            if (maxvaluediff >=0)
            {
               datacopy.push_back(double(data[startbin+i]));
            }
            else
            {
               datacopy.push_back(-double(data[startbin+i]));
            }
         }

         double x=x_in;
         if (maxvaluediff < 0) x=-x_in;

//         std::cout << "x = " << x << std::endl;
//         std::cout << "nsize = " << nsize << std::endl;
//         std::cout << "datacopy[0] = " << datacopy[0] 
//                   << " datacopy[nsize-1] = " << datacopy[nsize-1] 
//                   << std::endl;

// Make sure x is not less than datacopy[startbin] or greater than
// datacopy[nsize]:

         if (x < datacopy[0])
         {
            return startbin-1;
         }
         else if (x > datacopy[nsize-1])
         {
            return stopbin+1;
         }

// Check whether x==datacopy[nsize-1].  If so, immediately return
// startbin+nsize-1 without performing binary search:

         if (x==datacopy[nsize-1])
         {
            return startbin+nsize-1;
         }
         else
         {
            int lo_bin=0;
            int hi_bin=nsize-1;
            int nsteps=0;
            while(hi_bin-lo_bin > 1)
            {
               int curr_bin=basic_math::round(0.5*(lo_bin+hi_bin));
               if (x >= datacopy[curr_bin])
               {
                  lo_bin=curr_bin;
               }
               else
               {
                  hi_bin=curr_bin;
               }
               nsteps++;
            }
            return startbin+lo_bin;
         }
      }

// ---------------------------------------------------------------------
// Method negonepower returns (-1)**n:

   inline int negonepower(int n)
      {
         if (is_even(n))
         {
            return 1;
         }
         else
         {
            return -1;
         }
      }


// ---------------------------------------------------------------------
   inline double phase_to_continuous_value(
      double phase,double prev_phase,bool degrees)
      {
         double half_rotation=PI;
         if (degrees)
         {
            half_rotation=180;
         }
         return basic_math::phase_to_canonical_interval(
            phase,prev_phase-half_rotation,prev_phase+half_rotation);
      }

// ---------------------------------------------------------------------
// Method poor_man_erf returns a "discriminator-like" approximation to
// the error function built up out of linear segments.  Recall that
// Erf'(x) = 2/sqrt(PI).  We constructed this approximation for speed
// purposes.

   inline double poor_man_erf(double x)
      {
         const double half_sqrt_pi=0.8863;	// Sqrt(PI)/2
         const double two_over_sqrt_pi=1.1283;	// 2/Sqrt(PI)
         if (x > half_sqrt_pi)
         {
            return 1;
         }
         else if (x < -half_sqrt_pi)
         {
            return -1;
         }
         else
         {
            return two_over_sqrt_pi*x;
         }
      }

// ---------------------------------------------------------------------
// We hardwire in the first 20 powers of two for speed purposes

   inline double power_of_two(int n)
      {
         const int npowers=20;
         static double p[npowers+1]=
         {1,2,4,8,16,32,64,128,256,512,1024,
          2048,4096,8192,16384,32768,
          65536,131072,262144,524288,1048576
         };
         
         if (n >= 0 && n <= npowers)
         {
            return p[n];
         }
         else
         {
            std::cout << "Method mathfunc::power_of_two() can't return value for ";
            std::cout << "n = " << n << " !!!" << std::endl;
            return POSITIVEINFINITY;
         }
      }

// ---------------------------------------------------------------------
   inline double real_power(double x, int n)
      {
         double product=1;
         for (int i=0; i<n; i++)
         {
            product *= x;
         }
         return product;
      }




} // mathfunc namespace

#endif  // math/mathfuncs.h





