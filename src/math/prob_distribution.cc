// ==========================================================================
// Probability distribution class member function definitions
// ==========================================================================
// Last modified on 2/6/13; 10/20/13; 3/23/14; 1/2/17
// ==========================================================================

#include <vector>
#include "math/basic_math.h"
#include "math/constants.h"
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "math/mypolynomial.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"

using std::cout;
using std::endl;
using std::ios;
using std::ostream;
using std::pair; 
using std::string; 
using std::vector;

//const int prob_distribution::nbins_max=600;
//const int prob_distribution::nbins_max=10001;
const int prob_distribution::nbins_max=100001;

// ---------------------------------------------------------------------
// Initialization functions:

void prob_distribution::allocate_member_objects()
{
   x.reserve(nbins);
   p.reserve(nbins);
   pcum.reserve(nbins);
   xhist.reserve(nbins);
}		 

// ---------------------------------------------------------------------
void prob_distribution::initialize_member_objects()
{
   n_extrainfo_lines=50;
   for (int i=0; i<n_extrainfo_lines; i++) extrainfo[i]="";

   freq_histogram=false;
   title=subtitle=xlabel="";
   color=colorfunc::blue;
   xtic=0;
   ymax=ytic=-1;
   ninputs=0;
   nbins=nbins_max;

   densityfilenamestr="prob_density.meta";
   cumulativefilenamestr="prob_cum.meta";
}

// ---------------------------------------------------------------------
bool prob_distribution::check_dist_io() 
{
   bool dist_OK=true;
   if (ninputs <= 0)
   {
      cout << "Trouble in prob_distribution constructor!" << endl;
      cout << "ninputs = " << ninputs << endl;
      dist_OK=false;
   }

   if (nbins > nbins_max)
   {
      cout << "Error inside prob_distribution::check_dist_io()!" << endl;
      cout << "nbins = " << nbins << " > nbins_max = " << nbins_max
           << endl;
      cout << "Resetting number of output bins to nbins_max" << endl;
      nbins=nbins_max;
   }
   return dist_OK;
}

// ---------------------------------------------------------------------
// Member function compute_indep_var_limits_and_binsize sets the
// values of maximum_x,minimum_x and dx:

void prob_distribution::compute_indep_var_limits_and_binsize(double xarray[])
{
// First find minimum and maximum data values:

   maximum_x=NEGATIVEINFINITY;
   minimum_x=POSITIVEINFINITY;
   for (int i=0; i<ninputs; i++)
   {
      maximum_x=basic_math::max(maximum_x,xarray[i]);
      minimum_x=basic_math::min(minimum_x,xarray[i]);
   }
   dx=double(maximum_x-minimum_x)/double(nbins-1);

// dx=0 if n_inputs=1.  In order to avoid division by zero errors
// below, we artificially set dx=1 in this case...

   if (nearly_equal(dx,0)) dx=1;

// Extend probability distribution -/+ dx beyond minimum/maximum data
// values:

   minimum_x -= dx;
   maximum_x += dx;

   dx=double(maximum_x-minimum_x)/double(nbins-1);
}

// ---------------------------------------------------------------------
// Private member function fill_distribution takes in the input
// dataarray xarray containing n_input_vals entries.  It generates a
// frequency histogram containing n_output_bins.  After normalizing
// the histogram to convert it to a probability density distribution,
// the cumulative probability distribution is computed.

void prob_distribution::fill_distribution(double xarray[])
{
//   cout << "inside prob_distribution::fill_dist()" << endl;
//   cout << "nbins = " << nbins << " ninputs = " << ninputs << endl;

// Clear and then fill x histogram with data values.  Recall clearing
// a vector sets its size to zero.  So we do not want to clear xhist.
// Instead, we need to explicitly reset all of its entries to zero:

   for (int j=0; j<nbins; j++) xhist[j]=0;
   
   for (int i=0; i<ninputs; i++)
   {
      int j=basic_math::round((xarray[i]-minimum_x)/dx);
      if (j < 0 || j >= nbins)
      {
         cout << "Trouble in prob_distribution::fill_histogram()" << endl;
         cout << "While filling x histogram, j = " << j << endl;
         cout << "i = " << i << " xarray[i] = " << xarray[i] << endl;
         cout << "minimum_x = " << minimum_x << " dx = " << dx << endl;
      }
      else
      {
         xhist[j]++;
      }
   }

// Normalize histogramed data values to turn them into probabilities:

   fill_x_values();
   pmax=NEGATIVEINFINITY;
   double inverse_ninputs=1.0/static_cast<double>(ninputs);

   p.clear();
   for (int j=0; j<nbins; j++)
   {
      p.push_back(xhist[j]*inverse_ninputs);
      pmax=basic_math::max(pmax,p[j]);
   }

   compute_cumulative_distribution();
//   for (int j=0; j<nbins; j++)
//   {
//      cout << "j = " << j << " x = " << x[j] 
//           << " xhist = " << xhist[j] 
//           << " p = " << p[j] 
//           << " pcum = " << pcum[j] << endl;
//   }
//   cout << "mean = " << mean() << " std_dev = " << std_dev() << endl;
}

// ---------------------------------------------------------------------
void prob_distribution::fill_x_values()
{
//   cout << "inside prob_distribution::fill_x_values()" << endl;
   x.clear();
   for (int n=0; n<nbins; n++)
   {
      x.push_back(minimum_x+n*dx);
//      cout << "n = " << n << " x.back() = " << x.back() << endl;
   }
}

// ---------------------------------------------------------------------
// Member function fill_existing_distribution sets some parameters and
// fills the contents of an existing prob_distribution object which
// was created, for example, using the null constructor.  It is
// similar in spirit to the constructor

void prob_distribution::fill_existing_distribution(
   int n_input_vals,double xarray[],int n_output_bins)
{
   ninputs=n_input_vals;
   nbins=n_output_bins;
   if (check_dist_io())
   {
      compute_indep_var_limits_and_binsize(xarray);
      initialize_metafile_parameters();
      fill_distribution(xarray);
   }
}

// ==========================================================================
// Constructors:

prob_distribution::prob_distribution()
{
   initialize_member_objects();
   allocate_member_objects();
}

// Copy constructor:

prob_distribution::prob_distribution(const prob_distribution& p)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(p);
}

// ---------------------------------------------------------------------
// This constructor takes in the input dataarray xarray containing
// n_input_vals entries.  It generates a frequency histogram
// containing n_output_bins.  After normalizing the histogram to
// convert it to a probability density distribution, the cumulative
// probability distribution is computed.

prob_distribution::prob_distribution(
   int n_input_vals,double xarray[],int n_output_bins)
{
   initialize_member_objects();
   ninputs=n_input_vals;
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
prob_distribution::prob_distribution(
   int n_input_vals,double xarray[],int n_output_bins,double xlo)
{
   initialize_member_objects();
   ninputs=n_input_vals;
   nbins=n_output_bins;
   
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
prob_distribution::prob_distribution(
   int n_input_vals,int n_output_bins,double xlo,double deltax)
{
   initialize_member_objects();
   ninputs=n_input_vals;
   nbins=n_output_bins;

   if (check_dist_io())
   {
      allocate_member_objects();
      minimum_x=xlo;
      dx=deltax;
      maximum_x=minimum_x+(nbins-1)*dx;
      fill_x_values();
      initialize_metafile_parameters();
   }
}

// ---------------------------------------------------------------------
// This constructor takes in an array containing values which are
// supposed to be proportional to the probability distribution's
// density.  It first renormalizes the values so that they correspond
// to a genuine density and then computes the corresponding cumulative
// distribution values.

prob_distribution::prob_distribution(
   int n_bins,double xlo,double xhi,double density_values[])
{
   initialize_member_objects();
   ninputs=nbins=n_bins;
   
   if (check_dist_io())
   {
      allocate_member_objects();

      maximum_x=xhi;
      minimum_x=xlo;
      dx=(maximum_x-minimum_x)/(nbins-1);
      initialize_metafile_parameters();

//   cout << "max_x = " << maximum_x << " min_x = " << minimum_x
//        << " dx = " << dx << endl;

      double integral=0;
      for (int n=0; n<nbins; n++)
      {
         integral += density_values[n];
      }

      pmax=NEGATIVEINFINITY;
      fill_x_values();
      p.clear();
      for (int n=0; n<nbins; n++)
      {
         p.push_back(density_values[n]/integral);
         pmax=basic_math::max(pmax,p[n]);
//      cout << "n = " << n << " x = " << x[n] << " p = " << p[n] << endl;
      }
      compute_cumulative_distribution();
   }
}

// ---------------------------------------------------------------------
// This constructor can be used to regrid an already existing
// probability distribution onto an x interval defined by input
// parameters xlo, deltax and n_output_bins:

prob_distribution::prob_distribution(
   int n_input_vals,double xarray[],int n_output_bins,
   double xlo,double deltax)
{
   initialize_member_objects();
   ninputs=n_input_vals;
   nbins=n_output_bins;
   
   if (check_dist_io())
   {
      allocate_member_objects();

      minimum_x=xlo;
      dx=deltax;
      maximum_x=minimum_x+(nbins-1)*dx;
      initialize_metafile_parameters();
      fill_distribution(xarray);
   }
}

prob_distribution::prob_distribution(
   double xlo,double xhi,int n_input_vals,double xarray[],int n_output_bins)
{
   initialize_member_objects();
   ninputs=n_input_vals;
   nbins=n_output_bins;
   if (check_dist_io())
   {
      allocate_member_objects();

      minimum_x=xlo;
      maximum_x=xhi;
      dx=(maximum_x-minimum_x)/(nbins-1);
      initialize_metafile_parameters();
      fill_distribution(xarray);
   }
}

// ---------------------------------------------------------------------
// This constructor takes an STL vector of fourvectors as well as a
// particular dimension label d.  It forms a 1D prob distribution for
// the dth-dimension data after copying them from the fourvectors into
// a temporary 1D STL vector.

prob_distribution::prob_distribution(
   int d,const vector<fourvector>& V,int n_output_bins)
{
   initialize_member_objects();
   ninputs=V.size();
   nbins=n_output_bins;
   if (check_dist_io())
   {
      allocate_member_objects();

// Note added on 9/29/05: Can probably eliminate following xarray STL
// vector and just temporarily use member STL vector x...

      vector<double> xarray;
      xarray.reserve(ninputs);
      for (int i=0; i<ninputs; i++)
      {
         xarray.push_back(V[i].get(d));
      }
      compute_indep_var_limits_and_binsize(xarray);
      initialize_metafile_parameters();
      fill_distribution(xarray);
   }
}

// ---------------------------------------------------------------------
// This constructor takes an OSG array of Vec3s as well as a
// particular dimension label d.  It forms a 1D prob distribution for
// the dth-dimension data after copying them from the Vec3s into a
// temporary 1D STL vector.

prob_distribution::prob_distribution(
   int d,const osg::Vec3Array* vertices_ptr,int n_output_bins)
{
//   cout << "inside prob dist constructor" << endl;

   initialize_member_objects();
   ninputs=vertices_ptr->size();
   nbins=n_output_bins;
   if (check_dist_io())
   {
      allocate_member_objects();

// Note added on 9/29/05: Can probably eliminate following xarray STL
// vector and just temporarily use member STL vector x...

      vector<double> xarray;
      xarray.reserve(ninputs);

      double minimum=POSITIVEINFINITY;
      double maximum=NEGATIVEINFINITY;
      
      for (int i=0; i<ninputs; i++)
      {
         if (d==0)
         {
            xarray.push_back(vertices_ptr->at(i).x());
         }
         else if (d==1)
         {
            xarray.push_back(vertices_ptr->at(i).y());
         }
         else if (d==2)
         {
            xarray.push_back(vertices_ptr->at(i).z());
         }

         minimum=basic_math::min(minimum,xarray.back());
         maximum=basic_math::max(maximum,xarray.back());
         
      } // loop over i index labeling input entries

      compute_indep_var_limits_and_binsize(xarray);
      initialize_metafile_parameters();
      fill_distribution(xarray);
   }
}

prob_distribution::~prob_distribution()
{
}

// ---------------------------------------------------------------------
void prob_distribution::docopy(const prob_distribution & d)
{
   freq_histogram=d.freq_histogram;
   densityfilenamestr=d.densityfilenamestr;
   cumulativefilenamestr=d.cumulativefilenamestr;

   for (int i=0; i<n_extrainfo_lines; i++)
   {
      extrainfo[i]=d.extrainfo[i];
   }
   title=d.title;
   subtitle=d.subtitle;
   xlabel=d.xlabel;
   color=d.color;
   ninputs=d.ninputs;
   nbins=d.nbins;
   maximum_x=d.maximum_x;
   minimum_x=d.minimum_x;
   dx=d.dx;
   pmax=d.pmax;
   xmax=d.xmax;
   xmin=d.xmin;
   xtic=d.xtic;
   xsubtic=d.xsubtic;
   ytic=d.ytic;
   ysubtic=d.ysubtic;
   
   x.clear();
   p.clear();
   pcum.clear();
   for (int i=0; i<nbins; i++)
   {
      x.push_back(d.x[i]);
      p.push_back(d.p[i]);
      pcum.push_back(d.pcum[i]);
   }
}
   
// Overload = operator:

prob_distribution& prob_distribution::operator= (const prob_distribution& d)
{
   if (this==&d) return *this;
   docopy(d);
   return *this;
}

// Overload << operator:

ostream& operator<< (ostream& outstream,const prob_distribution& p)
{
   outstream << endl;
   outstream << "nbins = " << p.nbins << endl;
   outstream << "minimum_x = " << p.minimum_x
             << " maximum_x = " << p.maximum_x
             << " dx = " << p.dx << " pmax = " << p.pmax << endl;
   outstream << "mean = " << p.mean() << " std_dev = " << p.std_dev() 
             << endl;
   for (int i=0; i<p.nbins; i++)
   {
      outstream << "i = " << i << " x[i] = " << p.x[i]
                << " p[i] = " << p.p[i] 
                << " pcum[i] = " << p.pcum[i] 
                << " ninputs*p[i] = " << p.ninputs*p.p[i] 
                << endl;
   }
   outstream << endl;

   return outstream;
}

// ---------------------------------------------------------------------
// Boolean member function check_index_validity returns true if the
// input index lies within the allowed range for the current
// probability distribution object:

bool prob_distribution::check_index_validity(int n) const
{
   cout << "inside prob_distribution::check_index_validity()" << endl;
   bool index_OK=true;
   if (n < 0 || n > nbins)
   {
      cout << "Error inside prob_distribution::check_index_validity()!" 
           << endl;
      cout << "n = " << n << " nbins = " << nbins << endl;
      index_OK=false;
   }
   return index_OK;
}

// ---------------------------------------------------------------------
// Note added on 3/5/99: The compute_cumulative_distribution and
// compute_density_distribution functions below are NOT mirror images
// of one another.  In compute_cumulative_distribution, we regard p[i]
// as a *discrete* probability prob_distribution.  The cumulative
// prob_distribution is then given simply as the straight sum of the
// density values.  On the other hand, we regard p[i] as a
// *continuous* prob_distribution in routine
// compute_density_distribution.  We therefore divide by the measure
// dx in order to convert the cumulative to the density
// prob_distributions in routine compute_density_distribution.
// ---------------------------------------------------------------------

void prob_distribution::compute_cumulative_distribution()
{
   pcum.clear();
   pcum.push_back(p[0]);
   for (int i=1; i<nbins; i++)
   {
      pcum.push_back(pcum[i-1]+p[i]);
   }
}

// ---------------------------------------------------------------------
void prob_distribution::compute_density_distribution()
{
   p.clear();
   p.push_back(pcum[0]);
   pmax=p[0];
   for (int i=1; i<nbins; i++)
   {
      p.push_back((pcum[i]-pcum[i-1])/dx);
      pmax=basic_math::max(pmax,p[i]);
   }
}

// ---------------------------------------------------------------------
double prob_distribution::mean() const
{
   double avg=0;
   for (int i=0; i<nbins; i++)
   {
      avg += p[i]*x[i];
   }
   
   return avg;
}

double prob_distribution::std_dev() const
{
   double avg=mean();
   double sigmasqr=0;
   for (int i=0; i<nbins; i++)
   {
      sigmasqr += sqr(x[i]-avg)*p[i];
   }
   return sqrt(sigmasqr);
}

// ---------------------------------------------------------------------
// Member function find_x_corresponding_to_pcum returns the value of x
// for which pcum[x]=cumprob:

double prob_distribution::find_x_corresponding_to_pcum(double cumprob) const
{
   return mathfunc::find_x_corresponding_to_pcum(x, pcum, cumprob);
}

// ---------------------------------------------------------------------
// Member function find_minimal_interval_containing_specified_prob
// takes in some specified integrated probability value.  It returns
// the smallest interval [x1,x2] with minimum_x <= x1,x2 <= maximum_x
// over which the integrated probability density equals
// integrated_prob.

void prob_distribution::find_minimal_interval_containing_specified_prob(
   double integrated_prob,double& x1,double& x2)
{
   if (integrated_prob <= 0 || integrated_prob > 1)
   {
      cout << 
         "Error in prob_distribution::find_minimal_interval_containing_specified_prob()" << endl;
      cout << "integrated_prob = " << integrated_prob << " lies outside"
           << endl;
      cout << "allowed [0,1] interval." << endl;
   }
   
   double min_interval=POSITIVEINFINITY;
   for (int n=0; n<nbins; n++)
   {
      double a=minimum_x+n*dx;
      double Pa=pcum[n];
      double Pb=Pa+integrated_prob;
      double b;
      if (Pb >=0 && Pb <= 1)
      {
         b=find_x_corresponding_to_pcum(Pb);
         double curr_interval=fabs(b-a);
         if (curr_interval < min_interval)
         {
            x1=a;
            x2=b;
            min_interval=curr_interval;
         }
      }
   } // loop over index n
}

// ---------------------------------------------------------------------
double prob_distribution::median() const
{
   return find_x_corresponding_to_pcum(0.5);
}

// ---------------------------------------------------------------------
// Member function quartile_width returns one measure of the width of
// the probability distribution which is hopefully less susceptible to
// outliers than the standard deviation:

double prob_distribution::quartile_width() const
{
//   cout << "inside prob_distribution::quartile_width()" << endl;
   double level_25=find_x_corresponding_to_pcum(0.25);
   double level_75=find_x_corresponding_to_pcum(0.75);
//   cout << "level_25 = " << level_25
//        << " level_75 = " << level_75 << endl;
   return 0.5*(level_75-level_25);
}

// ---------------------------------------------------------------------
// Member function entropy returns S= - sum_n p_n log(p_n) where p_n =
// p[n]*dx = probability associated with nth bin.

double prob_distribution::entropy() const
{
   double S=0;
   for (int n=0; n<nbins; n++)
   {
//      double curr_p=p[n]*dx; // curr_p is a pure probability & NOT a density!
      double curr_p=p[n]; 
      if (curr_p > 0) 
      {
         S -= curr_p*log(curr_p);
      }
   }
   return S;
}

// ---------------------------------------------------------------------
double prob_distribution::peak_density_value() const
{
   return mathfunc::maximal_value(p);
}

// ---------------------------------------------------------------------
// This overloaded version of peak_density_value returns the bin
// location of the peak as well as the peak value.

double prob_distribution::peak_density_value(int& n_max_bin) const
{
//   cout << "inside prob_distribution::peak_density_value()" << endl;

   double p_peak_value=NEGATIVEINFINITY;
   for (int n=0; n<nbins; n++)
   {
//      cout << "n = " << n << " p[n] = " << p[n] << endl;
      if (p[n] > p_peak_value)
      {
         p_peak_value=p[n];
         n_max_bin=n;
//         cout << "n = " << n << " p_peak_value = " << p_peak_value << endl;
      }
   }
//   cout << "n_max_bin = " << n_max_bin
//        << " p_peak_value = " << p_peak_value << endl;
   
   return p_peak_value;
}

// ---------------------------------------------------------------------
// Member function FWHM computes the full width at half maximum for
// the tallest peak within the probability density distribution.  

double prob_distribution::FWHM() const
{
   int n_max_bin=-1;
   double p_peak=peak_density_value(n_max_bin);
   int n=n_max_bin;
   while (n+1 < nbins && !(p[n] >= 0.5*p_peak && p[n+1] <= 0.5*p_peak))
   {
      n++;
   } // while loop
   double xhi=x[n]+(x[n+1]-x[n])/(p[n+1]-p[n])*(0.5*p_peak-p[n]);
   cout << "xhi = " << xhi << endl;

   n=n_max_bin;
   while (n-1 >= 0 && !(p[n-1] <= 0.5*p_peak && p[n] >= 0.5*p_peak))
   {
      n--;
   }
   double xlo=x[n-1]+(x[n]-x[n-1])/(p[n]-p[n-1])*(0.5*p_peak-p[n-1]);

   cout << "xlo = " << xlo << endl;
   return (xhi-xlo);
}

// ---------------------------------------------------------------------
// Member function compute_percentile_p_value is intended to help
// summarize information about the density distribution itself.  It
// first computes the peak value of the density values.  It
// subsequently discards all p values whose values are less than
// min_frac_of_peak_p_value_to_retain*p_peak_value.  This method
// generates a probability distribution from the surviving
// non-negligible p values and returns the input percentile of its
// cumulative distribution.

double prob_distribution::compute_percentile_p_value(
   double min_frac_of_peak_p_value_to_retain,double percentile)
{
   double p_peak_value=peak_density_value();

// Copy elements from p density array which are greater than some
// small fraction of p_peak_value into a new q array:

   int m=0;
   double *q=new double[nbins];
   for (int n=0; n<nbins; n++)
   {
      if (p[n] > min_frac_of_peak_p_value_to_retain*p_peak_value) q[m++]=p[n];
   }
   
// Next generate probability distribution from elements in q array:

   prob_distribution p_prob(m,q,m/3);
   delete [] q;

//   p_prob.densityfilenamestr="p_prob.meta";
//   p_prob.xlabel="p_density";
//   p_prob.write_density_dist();

   return p_prob.find_x_corresponding_to_pcum(percentile);
}

// ---------------------------------------------------------------------
// Member function compute_density_overlap returns the integral of the
// product of the current probability distribution object's density
// with that of input probability distribution prob2:

double prob_distribution::compute_density_overlap(prob_distribution& prob2)
{
   double integral;
   double product[nbins];

   for (int i=0; i<nbins; i++)
   {
      product[i]=p[i]*prob2.p[i];
   }
   integral=mathfunc::simpsonsum(product,0,nbins-1)*dx;
   return integral;
}

// ---------------------------------------------------------------------
// Member function renormalize_histogram multiplies all probability
// values by input parameter a. 

void prob_distribution::renormalize_histogram(double a)
{
   for (int i=0; i<nbins; i++)
   {
      p[i] *= a;
   }
}

// ---------------------------------------------------------------------
// Member function regrid_histogram takes in two probability
// distributions.  The second distribution prob2 is regridded so that
// its bin spacing and size precisely match those of the first
// distribution prob1.  The current prob_distribution object is set
// equal to the regridded version of prob2.

void prob_distribution::regrid_histograms(
   prob_distribution& prob1,prob_distribution& prob2)
{
   const int max_freq_sum=1000000;

   double currfreq;
   double xarray[max_freq_sum];
   int k=0;

   for (int i=0; i<prob2.nbins; i++)
   {
      currfreq=prob2.p[i]*prob2.ninputs;
      for (int j=0; j<basic_math::round(currfreq); j++)
      {
         xarray[k++]=prob2.x[i];
      }
   }
   *this=prob_distribution(k,xarray,prob1.nbins,prob1.minimum_x,prob1.dx);
}

// ---------------------------------------------------------------------
// Member function add_histograms takes in two probability
// distributions prob1 and prob2.  After regridding prob2 so that its
// bin spacing is identical to that of prob1, add_histograms sums
// together prob1 and prob2 with weighting factors w1 and w2.  The
// final sum is returned within the current prob_distribution object.

void prob_distribution::add_histograms(
   double w1,prob_distribution& prob1,double w2,prob_distribution& prob2)
{
   if (freq_histogram)
   {
//   regrid_histograms(prob1,prob2);

      int counts_integral=0;
      int curr_counts[nbins];
      for (int n=0; n<nbins; n++)
      {
         curr_counts[n]=basic_math::round(
            w1*prob1.p[n]*prob1.ninputs+w2*prob2.p[n]*prob2.ninputs);
         counts_integral += curr_counts[n];
      }

      ninputs=counts_integral;
      for (int n=0; n<nbins; n++)
      {
         p[n]=curr_counts[n]/double(counts_integral);
         pmax=basic_math::max(pmax,p[n]);
      }
      compute_cumulative_distribution();
   } // freq_histogram conditional
}

// ==========================================================================
// Metafile display member functions
// ==========================================================================

void prob_distribution::general_header_info(string filenamestr)
{
   probstream << "story 'Filename = "+filenamestr+"'" << endl;
   probstream << "story '"+timefunc::getcurrdate()+"'" << endl;
//   probstream << "story 'ninputs = "+stringfunc::number_to_string(ninputs)
//      +"'" << endl;
//   probstream << "story 'nbins = "+stringfunc::number_to_string(nbins)+"'" 
//              << endl;
//   probstream << "story 'mean = "+stringfunc::number_to_string(mean(),6)+"'" 
//              << endl;
//   probstream << "story 'std dev = "+stringfunc::number_to_string(std_dev(),6)
//      +"'" << endl;
//   probstream << "story 'median = "+stringfunc::number_to_string(median(),4)
//      +"'" << endl;
//   probstream << "story 'quartile width = "+
//      stringfunc::number_to_string(quartile_width(),4)+"'" << endl;

   probstream << "story 'mean +/- std_dev = "
      +stringfunc::number_to_string(mean(),6)+" +/- "
      +stringfunc::number_to_string(std_dev(),6)+"'" << endl;
   probstream << "story 'median +/- quartile_width = "
      +stringfunc::number_to_string(median(),4)+" +/- "
      +stringfunc::number_to_string(quartile_width(),4)+"'" << endl;
   probstream << "story 'entropy = "+stringfunc::number_to_string(entropy(),4)
      +"'" << endl;
   probstream << "storyloc -1.4 6.25 " << endl;
}

// ---------------------------------------------------------------------
void prob_distribution::density_dist_header()
{
   const int nsubtics=5;	// # of subtics between major tics 

// Two sets of "story" commands cannot be requested within one plot.
// So to get more than one column of story output to appear, we need
// to create dummy plots with no content and then call the story
// command.  Title, x axis and y axis cal ls are mandatory when setting
// up a plot.

   if (extrainfo[0] != "")
   {
      probstream << "title ''" << endl;
      probstream << "x axis min 0 max 0.0001" << endl;
      probstream << "y axis min 0 max 0.0001" << endl;
      
      int i=0;
      while (i < n_extrainfo_lines)
      {
         if (extrainfo[i] != "" )
         {
            probstream << "story '"+extrainfo[i]+"'" << endl;
         }
         i++;
      }
      probstream << "charsize 0.8" << endl;
      probstream << "storyloc 6 6.25" << endl;
      probstream << "" << endl;
   }

// Probability density file header

   if (title=="")
   {
      if (freq_histogram)
      {
         title="Frequency Histogram";
      }
      else
      {
         title="Probability Density Distribution";
      }
   }
   probstream << "title '"+title+"'" << endl;

   if (subtitle != "")
   {
      probstream << "subtitle '"+subtitle+"'" << endl;
   }
   probstream <<  "x axis min "+stringfunc::number_to_string(xmin)+" max "
      +stringfunc::number_to_string(xmax) << endl;
   probstream << "label '"+xlabel+"'" << endl;

// Allow for manual overriding of automatic x axis tic settings

   if (xtic==0)
   {
      probstream << "tics "+stringfunc::number_to_string(trunclog(xmax-xmin))
         +" "+stringfunc::number_to_string(trunclog(xmax-xmin)/nsubtics) 
                 << endl;
   }
   else
   {
      probstream << "tics "+stringfunc::number_to_string(xtic)
         +" "+stringfunc::number_to_string(xsubtic) << endl;
   }

// probstream << "grid" << endl;
   if (freq_histogram)
   {
      if (ymax < 0) ymax=1.2*ninputs*pmax;
      probstream << "yaxis min 0 max "+stringfunc::number_to_string(
         ymax) << endl;
      probstream << "label 'Histogram Frequency'" << endl;

// Allow for manual overriding of automatic y axis tic settings

      if (ytic < 0)
      {
         probstream << "tics "+stringfunc::number_to_string(
            trunclog(1.2*ninputs*pmax))
            +" "+stringfunc::number_to_string(trunclog(1.2*ninputs*pmax)) 
                    << endl;
      }
      else
      {
         probstream << "tics "+stringfunc::number_to_string(ytic)
            +" "+stringfunc::number_to_string(ysubtic) << endl;
      }
   }
   else
   {
      if (ymax < 0) ymax=1.2*pmax;
      probstream << "yaxis min 0 max "+stringfunc::number_to_string(ymax) 
                 << endl;
      probstream << "label 'Probability'" << endl;
      probstream << "tics "+stringfunc::number_to_string(trunclog(1.2*pmax))+" "
         +stringfunc::number_to_string(trunclog(1.2*pmax)/nsubtics) << endl;
   }

   general_header_info(densityfilenamestr);
/*   
     probstream << "story 'Filename = "+densityfilenamestr+"'" << endl;
     probstream << "story '"+getcurrdate()+"'" << endl;
     probstream << "story 'nbins = "+stringfunc::number_to_string(nbins)+"'" << endl;
     probstream << "story 'median = "+stringfunc::number_to_string(median())+"'" << endl;
     probstream << "story 'mean = "+stringfunc::number_to_string(mean())+"'" << endl;
     probstream << "story 'std dev = "+stringfunc::number_to_string(std_dev())+"'" << endl;
     probstream << "storyloc -1 6.25 " << endl;
*/
}

// ---------------------------------------------------------------------
void prob_distribution::stacked_histograms_header(prob_distribution& prob2)
{
   const int nsubtics=5;	// # of subtics between major tics 

// Two sets of "story" commands cannot be requested within one plot.
// So to get more than one column of story output to appear, we need
// to create dummy plots with no content and then call the story
// command.  Title, x axis and y axis cal ls are mandatory when setting
// up a plot.

   if (extrainfo[0] != "")
   {
      probstream << "title ''" << endl;
      probstream << "x axis min 0 max 0.0001" << endl;
      probstream << "y axis min 0 max 0.0001" << endl;
      
      int i=0;
      while (i < n_extrainfo_lines)
      {
         if (extrainfo[i] != "" )
         {
            probstream << "story '"+extrainfo[i]+"'" << endl;
         }
         i++;
      }
      probstream << "charsize 0.8" << endl;
      probstream << "storyloc 6 6.25" << endl;
      probstream << "" << endl;
   }

// Probability density file header

   if (title=="") title="Frequency Histogram";
   probstream << "title '"+title+"'" << endl;

   if (subtitle != "")
   {
      probstream << "subtitle '"+subtitle+"'" << endl;
   }
   probstream <<  "x axis min "+stringfunc::number_to_string(xmin)+" max "
      +stringfunc::number_to_string(xmax) << endl;
   probstream << "label '"+xlabel+"'" << endl;

// Allow for manual overriding of automatic x axis tic settings

   if (xtic==0)
   {
      probstream << "tics "+stringfunc::number_to_string(trunclog(xmax-xmin))
         +" "+stringfunc::number_to_string(trunclog(xmax-xmin)/nsubtics) 
                 << endl;
   }
   else
   {
      probstream << "tics "+stringfunc::number_to_string(xtic)
         +" "+stringfunc::number_to_string(xsubtic) << endl;
   }

   double max_freqvalue=1.2*(ninputs*pmax+prob2.ninputs*prob2.pmax);
   probstream << "yaxis min 0 max "+stringfunc::number_to_string(
      max_freqvalue)
              << endl;
   probstream << "label 'Histogram Frequency'" << endl;

// Allow for manual overriding of automatic y axis tic settings

   if (ytic < 0)
   {
      probstream << "tics "+stringfunc::number_to_string(
         trunclog(1.2*max_freqvalue))
         +" "+stringfunc::number_to_string(trunclog(1.2*max_freqvalue)) 
                 << endl;
   }
   else
   {
      probstream << "tics "+stringfunc::number_to_string(ytic)
         +" "+stringfunc::number_to_string(ysubtic) << endl;
   }

   general_header_info(densityfilenamestr);
/*
  probstream << "story 'Filename = "+densityfilenamestr+"'" << endl;
  probstream << "story '"+getcurrdate()+"'" << endl;
  probstream << "story 'ninputs = "+stringfunc::number_to_string(ninputs)+"'" << endl;
  probstream << "story 'nbins = "+stringfunc::number_to_string(nbins)+"'" << endl;
  probstream << "story 'mean = "+stringfunc::number_to_string(mean())+"'" << endl;
  probstream << "story 'std dev = "+stringfunc::number_to_string(std_dev())+"'" << endl;
  probstream << "storyloc -1 6.25 " << endl;
*/
}

// ---------------------------------------------------------------------
void prob_distribution::cumulative_dist_header()
{
   const int nsubtics=5;	// # of subtics between major tics 

// Two sets of "story" commands cannot be requested within one plot.
// So to get more than one column of story output to appear, we need
// to create dummy plots with no content and then call the story
// command.  Title, x axis and y axis cal ls are mandatory when setting
// up a plot.

   if (extrainfo[0] != "")
   {
      probstream << "title ''" << endl;
      probstream << "x axis min 0 max 0.0001" << endl;
      probstream << "y axis min 0 max 0.0001" << endl;
      
      int i=0;
      while (i < n_extrainfo_lines)
      {
         if (extrainfo[i] != "" )
         {
            probstream << "story '"+extrainfo[i]+"'" << endl;
         }
         i++;
      }
      probstream << "charsize 0.8" << endl;
      probstream << "storyloc 6 6.25" << endl;
      probstream << "" << endl;
   }

// Cumulative probability file header

   if (title=="" || title=="Probability Density Distribution") 
      title="Cumulative Distribution";
   probstream << "title '"+title+"'" << endl;

   if (subtitle != "")
   {
      probstream << "subtitle '"+subtitle+"'" << endl;
   }
   probstream <<  "x axis min "+stringfunc::number_to_string(xmin)+" max "
      +stringfunc::number_to_string(xmax) << endl;
   probstream << "label '"+xlabel+"'" << endl;

// Allow for manual overriding of automatic x axis tic settings

   if (xtic==0)
   {
      probstream << "tics "+stringfunc::number_to_string(trunclog(xmax-xmin))
         +" "+stringfunc::number_to_string(trunclog(xmax-xmin)/nsubtics) 
                 << endl;
   }
   else
   {
      probstream << "tics "+stringfunc::number_to_string(xtic)
         +" "+stringfunc::number_to_string(xsubtic) << endl;
   }
//   probstream << "grid" << endl;
   probstream << "yaxis min 0 max 1" << endl;
   probstream << "label 'Cumulative probability'" << endl;
   probstream << "tics 0.25 0.125" << endl;

   general_header_info(cumulativefilenamestr);
/*
  probstream << "story 'Filename = "+cumulativefilenamestr+"'" << endl;
  probstream << "story '"+getcurrdate()+"'" << endl;
  probstream << "story 'ninputs = "+stringfunc::number_to_string(ninputs)+"'" << endl;
  probstream << "story 'nbins = "+stringfunc::number_to_string(nbins)+"'" << endl;
  probstream << "story 'mean = "+stringfunc::number_to_string(mean())+"'" << endl;
  probstream << "story 'std dev = "+stringfunc::number_to_string(std_dev())+"'"
  << endl;
  probstream << "storyloc -1 6.25 " << endl;
*/
}

// ---------------------------------------------------------------------
void prob_distribution::write_density_data()
{
   const int NPRECISION=10;
   
   probstream.setf(ios::fixed);
   probstream.setf(ios::showpoint);
   probstream.precision(NPRECISION);

   if (freq_histogram)
   {
      probstream << "barchart" << endl;
   }
   else
   {
      probstream << "curve color "+colorfunc::get_colorstr(color) 
                 << endl << endl;
      probstream << "thick 2" << endl;
   }
   
   for (int i=0; i<nbins; i++)
   {
      if (freq_histogram)
      {
         probstream << "group loc " << x[i] << " width " << 0.9*dx 
                    << " "+colorfunc::get_colorstr(color)+" 0 "
                    << ninputs*p[i] << endl;
      }
      else
      {

// Note added on Fri, Nov 4, 2011: We believe that writing out p[i]
// rather than p[i]/dx is simpler and more useful...


         probstream << x[i] << "\t\t" << p[i] << endl;
//         probstream << x[i] << "\t\t" << p[i]/dx << endl;
      }
   }
   if (freq_histogram)
   {
      probstream << "end" << endl;
   }
}

// ---------------------------------------------------------------------
// Member function write_stacked_histogram_data forms a single
// histogram frequency plot from the current prob_distribution object
// along with a second prob_distribution prob2.  The data within prob2
// is "stacked" on top of that for the current object.

void prob_distribution::write_stacked_histogram_data(
   prob_distribution& prob2)
{
   const int NPRECISION=5;
   
   probstream.setf(ios::fixed);
   probstream.setf(ios::showpoint);
   probstream.precision(NPRECISION);
   probstream << "barchart" << endl;
   
   for (int i=0; i<nbins; i++)
   {
      if (prob2.p[i]==0)
      {
         probstream << "group loc " << x[i] << " width " << 0.9*dx 
                    << " "+colorfunc::get_colorstr(color)+" 0 "
                    << ninputs*p[i] << endl;
      }
      else if (p[i]==0)
      {
         probstream << "group loc " << x[i] << " width " << 0.9*dx 
                    << " "+colorfunc::get_colorstr(prob2.color)+" 0 "
                    << prob2.ninputs*prob2.p[i] << endl;
      }
      else
      {
         probstream << "group loc " << x[i] << " width " << 0.9*dx 
                    << " "+colorfunc::get_colorstr(color)+" 0 " 
                    << ninputs*p[i] << " " 
                    << colorfunc::get_colorstr(prob2.color) 
                    << " stack " << ninputs*p[i] << " " 
                    << basic_math::round(ninputs*p[i]+prob2.ninputs*prob2.p[i])
                    << endl;
      }
   }
   probstream << "end" << endl;
}

// ---------------------------------------------------------------------
void prob_distribution::write_cumulative_data(
   bool pcum_to_one_minus_pcum_flag)
{
   const int NPRECISION=5;
   
   probstream.setf(ios::fixed);
   probstream.setf(ios::showpoint);
   probstream.precision(NPRECISION);

   probstream << "curve color "+colorfunc::get_colorstr(color) << endl;
   probstream << "thick 2" << endl;
   probstream << endl;

   for (int i=0; i<nbins; i++)
   {
      if (pcum_to_one_minus_pcum_flag)
      {
         probstream << x[i] << "\t\t" << 1-pcum[i] << endl;
      }
      else
      {
         probstream << x[i] << "\t\t" << pcum[i] << endl;
      }
   }
}

// ---------------------------------------------------------------------
void prob_distribution::write_density_dist(
   bool gzip_flag,bool generate_jpg_output_flag)
{
   filefunc::openfile(densityfilenamestr,probstream);
   density_dist_header();
   write_density_data();
   filefunc::closefile(densityfilenamestr,probstream);

   if (generate_jpg_output_flag)
   {
      filefunc::meta_to_jpeg(densityfilenamestr);
      if (gzip_flag) filefunc::gzip_file(densityfilenamestr);
   }
}

// ---------------------------------------------------------------------
void prob_distribution::write_stacked_histograms(
   prob_distribution& prob2,bool generate_jpg_output_flag)
{
   filefunc::openfile(densityfilenamestr,probstream);
   stacked_histograms_header(prob2);
   write_stacked_histogram_data(prob2);
   filefunc::closefile(densityfilenamestr,probstream);

   if (generate_jpg_output_flag)
   {
      filefunc::meta_to_jpeg(densityfilenamestr);
      filefunc::gzip_file(densityfilenamestr);
   }
}

// ---------------------------------------------------------------------
void prob_distribution::write_cumulative_dist(
   bool gzip_flag,bool generate_jpg_output_flag,
   bool pcum_to_one_minus_pcum_flag)
{
   filefunc::openfile(cumulativefilenamestr,probstream);
   cumulative_dist_header();
   write_cumulative_data(pcum_to_one_minus_pcum_flag);
   filefunc::closefile(cumulativefilenamestr,probstream);

   if (generate_jpg_output_flag)
   {
      filefunc::meta_to_jpeg(cumulativefilenamestr);
      if (gzip_flag) filefunc::gzip_file(cumulativefilenamestr);
   }
}

// ---------------------------------------------------------------------
void prob_distribution::writeprobdists(
   bool gzip_flag,bool generate_jpg_output_flag)
{
   write_density_dist(gzip_flag,generate_jpg_output_flag);
   if (!freq_histogram) write_cumulative_dist(
      gzip_flag,generate_jpg_output_flag);
}

// ---------------------------------------------------------------------
// Member function smooth_density_distribution applies a
// Savitzky-Golay filter to the probability density distribution.
// This method overwrites the noisy values in the p member array are
// with the smoothed ones.

void prob_distribution::smooth_density_distribution()
{
//   const int poly_order=4;
//   const int n_filter_points=16;

   double* p_array=new double[nbins];
   double* p_smooth=new double[nbins];

   for (int n=0; n<nbins; n++)
   {
      p_array[n]=p[n];
   }
   
   cout << "Error in prob_distribution::smooth_density_distribution()"
        << endl;
   cout << "See comment in this member function" << endl;
   exit(-1);

// In order to avoid linked dependency problems, we temporarily
// commented out 2 calls to filterfunc::savitzsky_smooth() on Tuesday,
// Dec 27, 2005.  Eventually, we should split src/math into two
// separate subdirs: src/math and src/advmath.  Then prob_distribution
// (along with mypolynomial and adv_mathfuncs) should be moved into
// src/advmath.  The remaining files do not depend upon the
// filterfuncs namespace.  So the order of linking should then be
// libmath, libfilter and libadvmath.  As of late Dec 05, we are
// frantically trying to prepare for our NTI talk in early Jan 06.  So
// we do not take the necessary time to perform this housecleaning...

//   filterfunc::savitzky_smooth
//      (nbins,p_array,p_smooth,0,poly_order,n_filter_points);

   double* p_2ndderiv=new double[nbins];
//   filterfunc::savitzky_smooth
//      (nbins,p_array,p_2ndderiv,2,poly_order,n_filter_points);

   p.clear();
   for (int n=0; n<nbins; n++)
   {
      p.push_back(p_smooth[n]);
   }

   delete [] p_array;
   delete [] p_smooth;
   delete [] p_2ndderiv;
}

// ---------------------------------------------------------------------
// Member function locate_density_peaks looks for local maxima in the
// probability density distribution within the input x range [x_start,
// x_stop].  A peak must be maximal within an x_interval centered
// about its location.  The bin centers of such peaks are returned
// within the first entries in a vector of <int,mypolynomial> pairs.
// [Quadratic polynomial fits to the logarithm of the density
// distribution in the vicinity of each local peak are stored within
// the 2nd entries in the vector within member function
// fit_gaussians_to_density_peaks(). ]

vector<pair<int,mypolynomial> >* prob_distribution::locate_density_peaks(
   double x_interval,double x_start,double x_stop)
{

// First compute maximum density value within range x_start <= x <=
// x_stop:
  
   double p_maximum=0;
   for (int n=0; n<nbins; n++)
   {
      if (x[n] >= x_start && x[n] <= x_stop)
      {
         p_maximum=basic_math::max(p_maximum,p[n]);
      }
   }

// Ignore any density value within the interval x_start <= x <= x_stop
// which is less than 1/2 of maximal value in this range:

   double p_minimal=0.5*p_maximum;

   int n=1;
   const int n_interval=basic_math::round(x_interval/dx);
   vector<pair<int,mypolynomial> >* peak_bin_ptr=
      new vector<pair<int,mypolynomial> >;
   while (n < nbins-1)
   {
      if (x[n] >= x_start && x[n] <= x_stop &&
          p[n] >= p[n-1] && p[n] >= p[n+1] && p[n] > p_minimal)
      {
         bool local_peak_found=true;          
         int nlo=basic_math::max(0,n-n_interval/2);
         int nhi=basic_math::min(nbins-1,n+n_interval/2);
         for (int i=nlo; i<=nhi; i++)
         {
            if (p[i] > p[n]) 
            {
               local_peak_found=false;
               break;
            }
         } // loop over index i 
         if (local_peak_found)
         {
//            cout << "Local peak found at x = " 
//                 << x[n] << " p = " << p[n] << endl;
            pair<int,mypolynomial> p;
            p.first=n;
            peak_bin_ptr->push_back(p);
         }
      } 
      n++;
   } // while loop
   return peak_bin_ptr;
}

// ---------------------------------------------------------------------
// Member function fit_gaussians_to_density_peaks takes in a vector
// containing the bin location of peaks within the probability density
// distribution.  It fits quadratic polynomials to the density
// distribution for x within the interval
// [x_peak-0.5*x_interval,x_peak+0.5*x_interval].  This method returns
// these quadratic polynomials within the second member of the input
// vector's pair for each peak.

void prob_distribution::fit_gaussians_to_density_peaks(
   vector<pair<int,mypolynomial> >* peak_bin_ptr,double x_interval)
{
   const int n_interval=basic_math::round(x_interval/dx);
   for (unsigned int i=0; i<peak_bin_ptr->size(); i++)
   {
      int n_peak=((*peak_bin_ptr)[i]).first;
      double x_peak=x[n_peak];

      int nlo=basic_math::max(0,n_peak-n_interval/2);
      int nhi=basic_math::min(nbins-1,n_peak+n_interval/2);

      int counter=0;
      double* x_value=new double[nhi-nlo+1];
      double* log_p=new double[nhi-nlo+1];
      for (int j=nlo; j<=nhi; j++)
      {
         x_value[counter]=x[j]-x_peak;
         log_p[counter]=log(p[j]);
         counter++;
      }

      double chisq;
      mypolynomial quad_poly(2);
      quad_poly.fit_coeffs(counter,x_value,log_p,chisq);
      delete [] x_value;
      delete [] log_p;

      ((*peak_bin_ptr)[i]).second=quad_poly;

//      for (int j=nlo; j<=nhi; j++)
//      {
//         double curr_x=x[j]-x_peak;
//         double curr_p=quad_poly.value(curr_x);
//         cout << curr_x+x_peak << "\t\t" << exp(curr_p) << endl;
//      }

   }
}
