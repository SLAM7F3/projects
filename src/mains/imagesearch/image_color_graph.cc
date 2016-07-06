// ==========================================================================
// Program IMAGE_COLOR_GRAPH imports a set of thumbnails from a
// specified subdirectory of /data/ImageEngine.  It computes quantized
// color histograms for each input thumbnail.  IMAGE_COLOR_GRAPH
// subsequently computes TF-IDF edge weights between all image nodes.  
// After discarding all edges whose weights are less than some
// user-specified threshold, this program exports an edge list which
// corresponds to the color graph for the imagery corpus.
// ==========================================================================
// Last updated on 8/1/13; 8/11/13
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>
#include "gmm/gmm.h"
#include "gmm/gmm_matrix.h"

#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "math/genvector.h"
#include "video/descriptorfuncs.h"
#include "general/outputfuncs.h"
#include "math/prob_distribution.h"
#include "video/RGB_analyzer.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   timefunc::initialize_timeofday_clock(); 

   cout.precision(12);
   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();
   string liberalized_color="";
   RGB_analyzer_ptr->import_quantized_RGB_lookup_table(liberalized_color);

   string images_subdir="/data/ImageEngine/tidmarsh/";
   string thumbnails_subdir=images_subdir+"thumbnails/";
   string colorhist_subdir=images_subdir+"color_histograms/";
   string gist_subdir=images_subdir+"gist_files/";

   vector<string> image_filenames=filefunc::image_files_in_subdir(
      thumbnails_subdir);
   unsigned int n_images=image_filenames.size();
   unsigned int n_colors=RGB_analyzer_ptr->get_n_color_indices();

// Import GIST descriptors into genmatrix *GIST_descriptor_matrix_ptr:

   vector<string> allowed_suffixes;
   allowed_suffixes.push_back("gist");
   vector<string> gist_filenames=
      filefunc::files_in_subdir_matching_specified_suffixes(
         allowed_suffixes,gist_subdir);
   genmatrix* GIST_matrix_ptr=descriptorfunc::GIST_descriptor_matrix(gist_filenames);
   int n_fields=GIST_matrix_ptr->get_ndim();
   genvector* gist_descriptor1_ptr=new genvector(n_fields);
   genvector* gist_descriptor2_ptr=new genvector(n_fields);

   if (n_images != GIST_matrix_ptr->get_mdim())
   {
      cout << "Error!" << endl;
      cout << "n_images = " << n_images << endl;
      cout << "n_GIST_descriptors = " << GIST_matrix_ptr->get_mdim()
           << endl;
      exit(-1);
   }   

// Compute quantized color histograms for all input images.  Store
// histogram results within rows of *term_freq_sparse_matrix_ptr:

   gmm::row_matrix< gmm::wsvector<float> >* term_freq_sparse_matrix_ptr=
      new gmm::row_matrix< gmm::wsvector<float> >(n_images,n_colors);
   gmm::clear(*term_freq_sparse_matrix_ptr);

   genvector* ndocs_containing_color_ptr=new genvector(n_images);
   ndocs_containing_color_ptr->clear_values();

   const double min_color_frac_threshold=0.01;
   texture_rectangle* texture_rectangle_ptr=new texture_rectangle();
   for (unsigned int i=0; i<n_images; i++)
   {
      string image_filename=image_filenames[i];
      string basename=filefunc::getbasename(image_filename);
      string prefix=stringfunc::prefix(basename);
      if (prefix.substr(0,10)=="thumbnail_")
      {
         prefix=prefix.substr(10,prefix.size()-10);
      }
      string color_histogram_filename=colorhist_subdir+prefix+".color_hist";

      cout << "i = " << i 
           << " image_filename = " << image_filename << endl;

      vector<double> color_histogram;
      if (filefunc::fileexist(color_histogram_filename))
      {
         color_histogram=filefunc::ReadInNumbers(color_histogram_filename);
//         for (int c=0; c<color_histogram.size(); c++)
//         {
//            cout << "c = " << c << " color_hist[c] =  "
//                 << color_histogram[c] << endl;
//         }
      }
      else
      {
         texture_rectangle_ptr->reset_texture_content(image_filename);
         color_histogram=RGB_analyzer_ptr->compute_color_histogram(
            texture_rectangle_ptr,liberalized_color);
      }

      for (unsigned int c=0; c<n_colors; c++)
      {
         double curr_color_frac=color_histogram[c];
         (*term_freq_sparse_matrix_ptr)(i,c)=curr_color_frac;

// Increment *ndocs_containing_color_ptr for current image:

         if (curr_color_frac > min_color_frac_threshold)
            ndocs_containing_color_ptr->put(
               c,ndocs_containing_color_ptr->get(c)+1);
      }
   } // loop over index i labeling image filenames

   delete texture_rectangle_ptr;

// Compute relative importance of quantized colors based upon the
// frequency with which they appear in all input images:

   genvector* inverse_doc_frequency_ptr=new genvector(n_colors);

   vector<int> ndocs;
   vector<double> color_importance;
   vector<string> sorted_color_list;
   for (unsigned int c=0; c<n_colors; c++)
   {
      ndocs.push_back(basic_math::round(ndocs_containing_color_ptr->get(c)));

      double quotient=double(n_images)/(1+ndocs_containing_color_ptr->get(c));
      double idf=log(quotient);
      if (ndocs.back() <= 1) idf=0;

      inverse_doc_frequency_ptr->put(c,idf);
      color_importance.push_back(idf);

      string curr_color_name=RGB_analyzer_ptr->get_color_name(c);
      sorted_color_list.push_back(curr_color_name);

      cout << "c = " << c << "  "
           << curr_color_name
           << "  nimages_containing_color = " 
           << ndocs_containing_color_ptr->get(c) 
           << " idf = " << idf 
           << endl;
   }
   
// Export file containing colors sorted according to their inverse
// document frequencies:

   templatefunc::Quicksort_descending(
      color_importance,ndocs,sorted_color_list);

   string importance_filename=colorhist_subdir+"color_importance.dat";
   ofstream importance_stream;
   filefunc::openfile(importance_filename,importance_stream);
   importance_stream << "# Color  ndocs_frac  importance" << endl << endl;
   for (unsigned int c=0; c<n_colors; c++)
   {
      double ndocs_frac=double(ndocs[c])/double(n_images);
      importance_stream << sorted_color_list[c] << "      "
                        << ndocs_frac << "      "
                        << color_importance[c] << endl;
   }
   filefunc::closefile(importance_filename,importance_stream);
   string banner="Exported "+importance_filename;
   outputfunc::write_banner(banner);

   delete ndocs_containing_color_ptr;
   delete RGB_analyzer_ptr;

// Compute term-frequency inverse document frequency (TF-IDF) ratios:

   banner="Calculating TF-IDF ratios";
   outputfunc::write_banner(banner);
   genvector* tfidf_ptr=new genvector(n_colors);
   genvector* normalized_tfidf_ptr=new genvector(n_colors);

   gmm::row_matrix< gmm::wsvector<float> >* color_doc_sparse_matrix_ptr=
      new gmm::row_matrix< gmm::wsvector<float> >(n_colors,n_images);
   gmm::clear(*color_doc_sparse_matrix_ptr);

   int n_nonzero_values=0;
   for (unsigned int n=0; n<n_images; n++)
   {
      tfidf_ptr->clear_values();
      for (unsigned int c=0; c<n_colors; c++)
      {
         tfidf_ptr->put(
            c,(*term_freq_sparse_matrix_ptr)(n,c)*
            inverse_doc_frequency_ptr->get(c));
      }

// Renormalize *tfidf_ptr so that it has unit length:

      *normalized_tfidf_ptr=tfidf_ptr->unitvector();

      double TINY=1E-9;
      for (unsigned int c=0; c<n_colors; c++)
      {
         double curr_tfidf=normalized_tfidf_ptr->get(c);
         if (fabs(curr_tfidf) > TINY)
         {
            (*color_doc_sparse_matrix_ptr)(c,n)=curr_tfidf;
            n_nonzero_values++;
         }
      }

   } // loop over index n labeling input images

   delete inverse_doc_frequency_ptr;
   delete normalized_tfidf_ptr;
   delete term_freq_sparse_matrix_ptr;
   delete tfidf_ptr;

   outputfunc::print_elapsed_time();

// Export sparse color-doc matrix:

   string sparse_matrix_filename=colorhist_subdir+"sparse_matrix_txt.dat";
   mathfunc::export_to_sparse_text_format(
      color_doc_sparse_matrix_ptr,n_nonzero_values,
      sparse_matrix_filename);

   sparse_matrix_filename=colorhist_subdir+"sparse_matrix_bin.dat";
   mathfunc::export_to_sparse_binary_format(
      color_doc_sparse_matrix_ptr,n_nonzero_values,
      sparse_matrix_filename);

   banner="Exported sparse color-doc matrix";
   outputfunc::write_banner(banner);

   genmatrix* reduced_docs_matrix_ptr=new genmatrix(n_colors,n_images);
   genvector curr_doc(n_colors),next_doc(n_colors);
   for (unsigned int c=0; c<n_images; c++)
   {
      for (unsigned int r=0; r<n_colors; r++)
      {
         double tfidf=(*color_doc_sparse_matrix_ptr)(r,c);
         curr_doc.put(r,tfidf);
      } // loop over index r labeling colors
      reduced_docs_matrix_ptr->put_column(c,curr_doc.unitvector());
   } // loop over index c labeling text files

// Compute edge weight probability distribution:

   banner="Calculating edge weight probability distribution";
   outputfunc::write_banner(banner);
   vector<double>* edge_weights_ptr=new vector<double>;

   double max_GIST_Delta=0.1;
   cout << "Enter max GIST Delta (e.g. 0.1):" << endl;
   cin >> max_GIST_Delta;

//   double weight_scale_factor=1;
   double weight_scale_factor=1000;
   cout << "weight_scale_factor = " << weight_scale_factor << endl;
   double min_edge_weight=weight_scale_factor*0.5;
   cout << "Enter minimum color edge weight (e.g. 990):" << endl;
   cin >> min_edge_weight;

   for (unsigned int i=0; i<n_images; i++)
   {
      outputfunc::update_progress_fraction(i,100,n_images);
      reduced_docs_matrix_ptr->get_column(i,curr_doc);
      GIST_matrix_ptr->get_row(i,*gist_descriptor1_ptr);

      for (unsigned int j=i+1; j<n_images; j++)
      {
         GIST_matrix_ptr->get_row(j,*gist_descriptor2_ptr);
         double GIST_Delta=(
            *gist_descriptor1_ptr - *gist_descriptor2_ptr).magnitude()/3.0;
         if (GIST_Delta > max_GIST_Delta) continue;

         reduced_docs_matrix_ptr->get_column(j,next_doc);
         double curr_edge_weight=curr_doc.dot(next_doc);
         edge_weights_ptr->push_back(curr_edge_weight);
      } // loop over index j
   } // loop over index i 
   cout << endl;
   cout << "edge_weights.size() = " << edge_weights_ptr->size() << endl;
   
   prob_distribution prob_weights(*edge_weights_ptr,1000);
   delete edge_weights_ptr;
   prob_weights.writeprobdists(false);
   string unix_cmd="mv prob_density.* "+colorhist_subdir;
   sysfunc::unix_command(unix_cmd);
   unix_cmd="mv prob_cum.* "+colorhist_subdir;
   sysfunc::unix_command(unix_cmd);

   outputfunc::print_elapsed_time();

// Export quantized image colorings edge list:

   string output_filename=colorhist_subdir+"image_colors_edgelist.dat";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   outstream << "# Edge weight threshold = " << min_edge_weight << endl;
   outstream << "# NodeID  NodeID'  Edge weight" << endl;
   outstream << endl;

   int n_edges=0;
   double max_edge_weight=0;
   for (unsigned int i=0; i<n_images; i++)
   {
      reduced_docs_matrix_ptr->get_column(i,curr_doc);
      for (unsigned int j=i+1; j<n_images; j++)
      {
         reduced_docs_matrix_ptr->get_column(j,next_doc);
         double edge_weight=weight_scale_factor*curr_doc.dot(next_doc);
         if (edge_weight < min_edge_weight) continue;
         max_edge_weight=basic_math::max(max_edge_weight,edge_weight);
         if (edge_weight < 10)
         {
            outstream << i << "  " << j << "  " << edge_weight << endl;
         }
         else
         {
            outstream << i << "  " << j << "  " << int(edge_weight) << endl;
         }
         n_edges++;
      } // loop over index j
   } // loop over index i 

   filefunc::closefile(output_filename,outstream);

   cout << "Number of images = " << n_images << endl;
   cout << "Number of colors in image histograms = " << n_colors << endl;
   cout << "Number of edges = " << n_edges << endl;
   cout << "Min edge weight = " << min_edge_weight << endl;
   cout << "Max edge weight = " << max_edge_weight << endl;
   banner="Exported image color graph edge list to "+output_filename;
   outputfunc::write_big_banner(banner);

   delete gist_descriptor1_ptr;
   delete gist_descriptor2_ptr;
   delete GIST_matrix_ptr;

   cout << "At end of program IMAGE_COLOR_GRAPH" << endl;
   outputfunc::print_elapsed_time();
}

