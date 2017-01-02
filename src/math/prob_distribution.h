// ==========================================================================
// Probability distribution class
// ==========================================================================
// Last modified on 7/5/12; 10/20/13; 11/19/13; 1/2/17
// ==========================================================================

#ifndef PROB_DISTRIBUTION_H
#define PROB_DISTRIBUTION_H

#include <osg/Array>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>
#include "color/colorfuncs.h"
#include "math/basic_math.h"
#include "math/constants.h"
#include "math/fourvector.h"
#include "math/mypolynomial.h"

class prob_distribution
{

  public:

// Initialization, constructor and destructor functions:

   void fill_existing_distribution(
      int n_input_vals,double xarray[],int n_output_bins);
   
   prob_distribution();
   prob_distribution(const prob_distribution& p);
   prob_distribution(int n_input_vals,double xarray[],int n_output_bins);
   template <class T> prob_distribution(
      const std::vector<T>& xarray,int n_output_bins);
   prob_distribution(int n_input_vals,double xarray[],int n_output_bins,
                     double xlo);
   template <class T> prob_distribution(
      const std::vector<T>& xarray,int n_output_bins,double xlo);
   prob_distribution(
      int n_input_vals,int n_output_bins,double xlo,double deltax);
   template <class T> prob_distribution(
      const std::vector<T>& xarray,int n_output_bins,double xlo,
      double deltax);
   template <class T> prob_distribution(
      int n_output_bins,double xlo,double xhi,const std::vector<T>& xarray);

   prob_distribution(
      int n_input_vals,double xarray[],int n_output_bins,
      double xlo,double deltax);
   prob_distribution(
      double xlo,double xhi,int n_input_vals,double xarray[],
      int n_output_bins);
   prob_distribution(int n_bins,double xlo,double xhi,
                     double density_values[]);
   prob_distribution(
      int d,const std::vector<fourvector>& V,int n_output_bins);
   prob_distribution(
      int d,const osg::Vec3Array* vertices_ptr,int n_output_bins);
   ~prob_distribution();

   prob_distribution& operator= (const prob_distribution& d);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const prob_distribution& p);

// Set and get methods:

   int get_bin_number(double x) const;
   double get_x(int n) const;
   double get_p(int n) const;
   double get_pcum(int n) const;
   int get_nbins() const;
   double get_minimum_x() const;
   double get_maximum_x() const;
   double get_dx() const;

   void set_freq_histogram(bool flag);
   void set_densityfilenamestr(std::string filename);
   void set_cumulativefilenamestr(std::string filename);
   void set_title(std::string title);
   void set_subtitle(std::string subtitle);
   void set_xlabel(std::string label);
   void set_xmin(double min_x);
   void set_xmax(double max_x);
   void set_xtic(double x_tic);
   void set_xsubtic(double x_subtic);
   void set_color(colorfunc::Color c);

   void set_ymax(double max_y);
   double get_pmax() const;
   void set_ytic(double dy);

   void compute_cumulative_distribution();
   void compute_density_distribution();
   double mean() const;
   double std_dev() const;
   double find_x_corresponding_to_pcum(double cumprob) const;
   void find_minimal_interval_containing_specified_prob(
      double integrated_prob,double& x1,double& x2);
   double median() const;
   double quartile_width() const;
   double entropy() const;
   double peak_density_value() const;
   double peak_density_value(int& n_max_bin) const;
   double FWHM() const;
   double compute_percentile_p_value(
      double min_frac_of_peak_p_value_to_retain,double percentile);

   double compute_density_overlap(prob_distribution& prob2);
   void renormalize_histogram(double a);
   void regrid_histograms(prob_distribution& prob1,prob_distribution& prob2);
   void add_histograms(
      double w1,prob_distribution& prob1,double w2,prob_distribution& prob2);

   void smooth_density_distribution();
   std::vector<std::pair<int,mypolynomial> >* locate_density_peaks(
      double x_interval,double x_start,double x_stop);
   void fit_gaussians_to_density_peaks(
      std::vector<std::pair<int,mypolynomial> >* peak_ptr,double x_interval);

// Metafile display member functions:

   void general_header_info(std::string filenamestr);
   void density_dist_header();
   void stacked_histograms_header(prob_distribution& prob2);
   void cumulative_dist_header();
   void general_header_info();

   void write_density_data();
   void write_stacked_histogram_data(prob_distribution& prob2);
   void write_cumulative_data(bool pcum_to_one_minus_pcum_flag=false);

   void write_density_dist(bool gzip_flag=true,
                           bool generate_jpg_output_flag=true);
   void write_stacked_histograms(
      prob_distribution& prob2,bool generate_jpg_output_flag=true);
   void write_cumulative_dist(
      bool gzip_flag=true,bool generate_jpg_output_flag=true,
      bool pcum_to_one_minus_pcum_flag=false);
   void writeprobdists(bool gzip_flag=true,bool generate_jpg_output_flag=true);

  private:

   static const int nbins_max;

   bool freq_histogram;	// true if density distribution is to be plotted
			//  as a barchart frequency histogram
   int ninputs,nbins;
   int n_extrainfo_lines;
   double maximum_x,minimum_x,dx,pmax;	// distribution parameters
   double xmax,xmin,xtic,xsubtic;	// meta file parameters
   double ymax,ytic,ysubtic;

   std::string densityfilenamestr,cumulativefilenamestr;
   std::string extrainfo[50];
   std::string title,subtitle,xlabel;
   colorfunc::Color color;

   std::vector<double> x,p,pcum,xhist;
   std::ofstream probstream;

   void allocate_member_objects();
   void initialize_member_objects();
   void initialize_metafile_parameters();
   bool check_index_validity(int n) const;

   void compute_indep_var_limits_and_binsize(double xarray[]);
   template <class T> void compute_indep_var_limits_and_binsize(
      const std::vector<T>& xarray);

   bool check_dist_io();
   void fill_x_values();
   void fill_distribution(double xarray[]);
   template <class T> void fill_distribution(const std::vector<T>& xarray);
   void docopy(const prob_distribution& d);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void prob_distribution::initialize_metafile_parameters()
{
   xmin=minimum_x;	// xmin/xmax = min/max x values for meta file plots
   xmax=maximum_x;
}

// ---------------------------------------------------------------------
// Set and get member functions:

inline int prob_distribution::get_bin_number(double x) const
{
   int n=basic_math::round((x-minimum_x)/dx);
   n=basic_math::max(0,n);
   n=basic_math::min(nbins-1,n);
   return n;
}

inline double prob_distribution::get_x(int n) const
{
   if (n >=0 && n < nbins)
   {
      return x.at(n);
   }
   else 
   {
      std::cout << "Error inside prob_distribution::get_x()!" << std::endl;
      std::cout << "n = " << n << " nbins = " << nbins << std::endl;
      return NEGATIVEINFINITY;
   }
}

inline double prob_distribution::get_p(int n) const
{
   if (n >=0 && n < nbins)
   {
      return p.at(n);
   }
   else 
   {
      std::cout << "Error inside prob_distribution::get_p()!" << std::endl;
      std::cout << "n = " << n << " nbins = " << nbins << std::endl;
      return NEGATIVEINFINITY;
   }
}

inline double prob_distribution::get_pcum(int n) const
{
   if (n >=0 && n < nbins)
   {
      return pcum.at(n);
   }
   else 
   {
      std::cout << "Error inside prob_distribution::get_pcum()!" << std::endl;
      std::cout << "n = " << n << " nbins = " << nbins << std::endl;
      return NEGATIVEINFINITY;
   }
}

inline int prob_distribution::get_nbins() const
{
   return nbins;
}

inline double prob_distribution::get_minimum_x() const
{
   return minimum_x;
}

inline double prob_distribution::get_maximum_x() const
{
   return maximum_x;
}

inline double prob_distribution::get_dx() const
{
   return dx;
}

inline void prob_distribution::set_freq_histogram(bool flag)
{
   freq_histogram=flag;
}

inline void prob_distribution::set_densityfilenamestr(std::string filename)
{
   densityfilenamestr=filename;
}

inline void prob_distribution::set_cumulativefilenamestr(std::string filename)
{
   cumulativefilenamestr=filename;
}

inline void prob_distribution::set_title(std::string title)
{
   this->title=title;
}

inline void prob_distribution::set_subtitle(std::string subtitle)
{
   this->subtitle=subtitle;
}

inline void prob_distribution::set_xlabel(std::string label)
{
   xlabel=label;
}

inline void prob_distribution::set_xmin(double min_x)
{
   xmin=min_x;
}

inline void prob_distribution::set_xmax(double max_x)
{
   xmax=max_x;
}

inline void prob_distribution::set_xtic(double x_tic)
{
   xtic=x_tic;
}

inline void prob_distribution::set_xsubtic(double x_subtic)
{
   xsubtic=x_subtic;
}

inline void prob_distribution::set_ymax(double max_y)
{
   ymax=max_y;
}

inline double prob_distribution::get_pmax() const
{
   return pmax;
}

inline void prob_distribution::set_ytic(double dy)
{
   ytic=dy;
}

inline void prob_distribution::set_color(colorfunc::Color c)
{
   color=c;
}

// ---------------------------------------------------------------------
template <class T> void prob_distribution::compute_indep_var_limits_and_binsize(const std::vector<T>& xarray)
{
//   std::cout << 
//      "inside prob_distribution::compute_indep_var_limits_and_binsize()" 
//             << std::endl;

// First find minimum and maximum data values:

   maximum_x=NEGATIVEINFINITY;
   minimum_x=POSITIVEINFINITY;
   for (int i=0; i<ninputs; i++)
   {
      maximum_x=basic_math::max(maximum_x,static_cast<double>(xarray[i]));
      minimum_x=basic_math::min(minimum_x,static_cast<double>(xarray[i]));
   }
   dx=double(maximum_x-minimum_x)/double(nbins-1);

// dx=0 if n_input_vals=1.  In order to avoid division by zero errors
// below, we artificially set dx=1 in this case...

   if (nearly_equal(dx,0)) dx=1;

// Extend probability distribution -/+ dx beyond minimum/maximum data
// values:

   minimum_x -= dx;
   maximum_x += dx;

   dx=double(maximum_x-minimum_x)/double(nbins-1);
}

// ---------------------------------------------------------------------
template <class T> void prob_distribution::fill_distribution(
   const std::vector<T>& xarray)
{
//   std::cout << "inside prob_distribution::fill_distribution()" << std::endl;
//   std::cout << "nbins = " << nbins << std::endl;
   
// Clear and then fill x histogram with data values:
   
   double* xhist=new double[nbins];
   for (int j=0; j<nbins; j++) xhist[j]=0;

   int n_valid_inputs=0;
   for (int i=0; i<ninputs; i++)
   {
      int j=basic_math::round((static_cast<double>(xarray[i])-minimum_x)/dx);
      if (j < 0 || j >= nbins)
      {
         std::cout << "Trouble in prob_distribution::fill_distribution()" 
                   << std::endl;
         std::cout << "While filling x histogram, j = " << j << std::endl;
         std::cout << "i = " << i << " ninputs = " << ninputs
                   << " xarray[i] = " << xarray[i] 
                   << std::endl;
         std::cout << "minimum_x = " << minimum_x 
                   << " maximum_x = " << maximum_x 
                   << " dx = " << dx 
                   << std::endl;
      }
      else
      {
         xhist[j]++;
         n_valid_inputs++;
      }
   }
//   std::cout << "ninputs = " << ninputs << " n_valid_inputs = "
//             << n_valid_inputs << std::endl;

// Normalize histogramed data values to turn them into probabilities:

   fill_x_values();
   pmax=NEGATIVEINFINITY;
   double inverse_n_valid_inputs=1.0/static_cast<double>(n_valid_inputs);

   p.clear();
   for (int j=0; j<nbins; j++)
   {
      p.push_back(xhist[j]*inverse_n_valid_inputs);
      pmax=basic_math::max(pmax,p[j]);
   }
   delete [] xhist;

   compute_cumulative_distribution();
//   compute_density_distribution();
//   for (int j=0; j<nbins; j++)
//   {
//      if (p[j] > 0)
//      {
//         std::cout << "j = " << j << " x = " << x[j] << " p = " << p[j] 
//                   << " pcum = " << pcum[j] << std::endl;
//      }
//   }
//   std::cout << "mean = " << mean() << " std_dev = " << std_dev() 
//             << std::endl;
}

// ---------------------------------------------------------------------
template <class T> prob_distribution::prob_distribution(
   const std::vector<T>& xarray,int n_output_bins)
{
//   std::cout << "inside prob_distribution constructor taking in STL vector"
//             << std::endl;
   initialize_member_objects();
   ninputs=xarray.size();
   nbins=n_output_bins;
   if (check_dist_io())
   {
      allocate_member_objects();
      compute_indep_var_limits_and_binsize(xarray);
      initialize_metafile_parameters();
      fill_distribution(xarray);
   }
}

// ---------------------------------------------------------------------
template <class T> prob_distribution::prob_distribution(
   const std::vector<T>& xarray,int n_output_bins,double xlo)
{
//   std::cout << "inside prob dist constructor" << std::endl;
   initialize_member_objects();
   ninputs=xarray.size();
   nbins=n_output_bins;
//   std::cout << "check_dist_io() = " << check_dist_io() << std::endl;
   if (check_dist_io())
   {
      allocate_member_objects();
      compute_indep_var_limits_and_binsize(xarray);
      minimum_x=xlo;
      dx=(maximum_x-minimum_x)/(nbins-1);
      fill_x_values();
      initialize_metafile_parameters();
      fill_distribution(xarray);
   }
}

// ---------------------------------------------------------------------
template <class T> prob_distribution::prob_distribution(
   const std::vector<T>& xarray,int n_output_bins,double xlo,double deltax)
{
   initialize_member_objects();
   ninputs=xarray.size();
   nbins=n_output_bins;
   if (check_dist_io())
   {
      allocate_member_objects();
      compute_indep_var_limits_and_binsize(xarray);
      minimum_x=xlo;
      dx=deltax;
      maximum_x=minimum_x+(nbins-1)*dx;
      fill_x_values();
      initialize_metafile_parameters();
      fill_distribution(xarray);
   }
}

// ---------------------------------------------------------------------
template <class T> prob_distribution::prob_distribution(
   int n_output_bins,double xlo,double xhi,const std::vector<T>& xarray)
{
   initialize_member_objects();
   ninputs=xarray.size();
   nbins=n_output_bins;
   if (check_dist_io())
   {
      allocate_member_objects();
      minimum_x=xlo;
      maximum_x=xhi;
      dx=(maximum_x-minimum_x)/(nbins-1);
      fill_x_values();
      initialize_metafile_parameters();
      fill_distribution(xarray);
   }
}


#endif  // math/prob_distribution.h
