// ==========================================================================
// Program COVARMATRIX imports color or HOG descriptors within
// descriptors_subdir which were generated via pr_gui_extremal_regions().  It
// computes the mean descriptor as well as the descriptors' covariance matrix.
// Upper triangle entries within the inverse square-root covariance
// matrix are exported to output text files.

//				./covarmatrix

// ==========================================================================
// Last updated on5/13/14; 5/14/14; 5/20/14; 7/22/14
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/RGB_analyzer.h"


using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   char descriptor_char;
   cout << "Enter 'c' or 'h' for color or HOG descriptor processing"
        << endl;
   cin >> descriptor_char;
   
   string descriptor_type = "color";
   if (descriptor_char == 'h')
   {
      descriptor_type = "HOG";
   }
   cout << "Descriptor_type = " << descriptor_type << endl;
  
   char polarity_char;
   cout << "Enter 'b' or 'd' for bright or dark extremal region polarity"
	<< endl;
   cin >> polarity_char;
   string polarity_type="bright";
   if (polarity_char == 'd')
   {
      polarity_type="dark";
   }
   cout << "Polarity_type = " << polarity_type << endl;

   string all_descriptors_subdir = 
     "/home/pcho/sputnik/peter_stuff/text_in_wild/descriptors/all_descriptors/";
   string color_histograms_subdir=all_descriptors_subdir+"color_histograms/";
   string HOG_subdir=all_descriptors_subdir+"HOG/";

   string bright_descriptors_subdir, dark_descriptors_subdir;
   unsigned int n_bins;
   if (descriptor_type=="color")
   {
      bright_descriptors_subdir=color_histograms_subdir+"bright/";
      dark_descriptors_subdir=color_histograms_subdir+"dark/";
      n_bins=33;
   }
   else if (descriptor_type=="HOG")
   {
      bright_descriptors_subdir=HOG_subdir+"bright/";
      dark_descriptors_subdir=HOG_subdir+"dark/";
      n_bins=128;
   }
   
   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();
   
   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("txt");
   
   string curr_descriptors_subdir=bright_descriptors_subdir;
   if (polarity_type=="dark")
   {
      curr_descriptors_subdir=dark_descriptors_subdir;
   }
   vector<string> descriptor_files=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,curr_descriptors_subdir);
   cout << "descriptor_files.size() = " << descriptor_files.size() << endl;

   genvector* X_ptr = new genvector(n_bins);
   genvector* mean_ptr = new genvector(n_bins);
   mean_ptr->clear_values();

   genmatrix* covar_ptr=new genmatrix(n_bins,n_bins);
   genmatrix* inverse_covar_ptr=new genmatrix(n_bins,n_bins);
   covar_ptr->clear_values();

   int counter=0;
   for (unsigned int i=0; i<descriptor_files.size(); i++)
   {
      outputfunc::update_progress_fraction(i, 4, descriptor_files.size());
      vector< vector<double> > RowNumbers=filefunc::ReadInRowNumbers(
         descriptor_files[i]);

      for (unsigned int r=0; r<RowNumbers.size(); r++)
      {
         for (unsigned int c=0; c<n_bins; c++)
         {
            X_ptr->put(c,RowNumbers[r].at(c));
         }
         mathfunc::recursive_mean(n_bins,counter,X_ptr,mean_ptr);

         mathfunc::recursive_covariance_matrix(
            counter,X_ptr,mean_ptr,covar_ptr);

         counter++;
      } // loop over index r labeling rows within current color hist file
   } // loop over index i labeling color histogram files

   cout << "Descriptor counter = " << counter << endl;
   cout << "*mean_ptr = " << *mean_ptr << endl;
   
   double mean_norm=0;
   for (unsigned int c=0; c<n_bins; c++)
   {
      mean_norm += mean_ptr->get(c);
   }

   vector<double> mean_descriptor;
   for (unsigned int c=0; c<n_bins; c++)
   {
      mean_ptr->put(c,mean_ptr->get(c)/mean_norm);
      mean_descriptor.push_back(mean_ptr->get(c));
   }

   if (descriptor_type=="color")
   {
     RGB_analyzer_ptr->print_color_histogram(mean_descriptor);
   }
   
   covar_ptr->inverse(*inverse_covar_ptr);
//   cout << "covar = " << *covar_ptr << endl;
//   cout << "inverse_covar = " << *inverse_covar_ptr << endl;

   genmatrix U(n_bins,n_bins), W(n_bins,n_bins), V(n_bins,n_bins);
   inverse_covar_ptr->sorted_singular_value_decomposition(U,W,V);

   genmatrix sqrt_W(n_bins,n_bins);
   sqrt_W.clear_values();
   for (unsigned int c=0; c<n_bins; c++)
   {
      cout << "c = " << c << " W[c] = " << W.get(c,c) << endl;
      sqrt_W.put(c,c,sqrt(W.get(c,c)));
   }

   genmatrix sqrt_inverse_covar(n_bins,n_bins);
   sqrt_inverse_covar = U * sqrt_W * U.transpose();
   
   genmatrix diff(n_bins,n_bins);
   diff = *inverse_covar_ptr - sqrt_inverse_covar * sqrt_inverse_covar;
   
   double max_diff = NEGATIVEINFINITY;
   for (unsigned int r=0; r<n_bins; r++)
   {
      for (unsigned int c=0; c<n_bins; c++)
      {
         if (fabs(diff.get(r,c)) > max_diff)
         {
            max_diff=diff.get(r,c);
         }
      }
   }
   cout << "max_diff = " << max_diff << endl;


/*
   genmatrix UVtrans(n_bins,n_bins);
   UVtrans = U * V.transpose();
   const double TINY=1E-9;
   for (unsigned int r=0; r<n_bins; r++)
   {
      for (unsigned int c=0; c<n_bins; c++)
      {
         if (fabs(UVtrans.get(r,c)) < TINY) UVtrans.put(r,c,0);
      }
   }
   cout << "U * V.trans = " << UVtrans << endl;
*/

   double min_value=POSITIVEINFINITY;
   double max_value=NEGATIVEINFINITY;
   for (unsigned int c=0; c<n_bins; c++)
   {
      for (unsigned int d=c+1; d<n_bins; d++)
      {
         double curr_value = fabs(sqrt_inverse_covar.get(c,d));
         min_value = basic_math::min(min_value,curr_value);
         max_value = basic_math::max(max_value,curr_value);
      }
   }
   
   cout << "min off-diagonal value for |sqrt_inverse_covar| = " 
        << min_value << endl;
   cout << "max off-diagonal value for |sqrt_inverse_covar| = " 
        << max_value << endl;

// Export upper-triangular entries in sqrt_inverse_covar in C format
// for pwin purposes:

   double min_offdiag_frac;
   if (descriptor_type=="color")
   {
      min_offdiag_frac = 0.01;
   }
   else if (descriptor_type=="HOG")
   {
      min_offdiag_frac = 0.1;
   }

   string output_filename=polarity_type+"_"+descriptor_type+
     "_sqrt_inverse_covar.dat";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);
   int n_nontrivial_entries=0;
   for (unsigned int c=0; c<n_bins; c++)
   {
      for (unsigned int d=c; d<n_bins; d++)
      {
         double curr_value = fabs(sqrt_inverse_covar.get(c,d));
         if (c != d && curr_value < min_offdiag_frac * max_value) continue;
         
         n_nontrivial_entries++;
         outstream << "  sqrt_inv_covar[" << c << "][" << d << "] = "
		   << stringfunc::scinumber_to_string(
		     sqrt_inverse_covar.get(c,d),5) << ";" << endl;
      }
   }
   filefunc::closefile(output_filename,outstream);
   string banner="Exported sqrt inverse covar matrix to "+
      output_filename;
   outputfunc::write_big_banner(banner);

   cout << "n_nontrivial_entries within sqrt_inverse_covar = "
        << n_nontrivial_entries << endl;
   cout << "n_total entries within sqrt_inverse_covar = "
        << n_bins * (n_bins+1) / 2 << endl;

   delete RGB_analyzer_ptr;
   delete X_ptr;
   delete mean_ptr;
   delete covar_ptr;
   delete inverse_covar_ptr;
}

