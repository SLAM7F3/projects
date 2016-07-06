// =======================================================================
// Program FORM_IMAGE_EDGELIST imports features extracted from a
// transcripted and non-transcripted Boston Bombing YouTube video
// clip.  It computes and plots the distribution for feature image
// frequencies.  FORM_IMAGE_EDGELIST next forms a matrix whose rows
// are labeled by feature ID and whose columns are labeled by image
// ID. Entries in this matrix equal 1 if a particular feature was
// extracted from a particular image and 0 otherwise.  Inner products
// between different columns of this matrix correspond to number of
// edges within an image graph.  FORM_IMAGE_EDGELIST exports an edge
// list to an output text file.
// =======================================================================
// Last updated on 6/1/13; 6/3/13; 12/23/13
// =======================================================================

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "gmm/gmm.h"
#include "gmm/gmm_matrix.h"
#include <flann/flann.hpp>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "passes/PassesGroup.h"
#include "math/prob_distribution.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

int main( int argc, char** argv ) 
{
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);
   int videopass_ID=passes_group.get_videopass_ID();
//   cout << "videopass_ID = " << videopass_ID << endl;
   string image_list_filename=passes_group.get_image_list_filename();
   cout << "image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;
   string features_subdir=bundler_IO_subdir+"features/";

   vector<string> image_filenames,feature_filenames;
   filefunc::ReadInfile(image_list_filename);
   int n_images=filefunc::text_line.size();
   cout << "n_images = " << n_images << endl;
   
   for (int i=0; i<n_images; i++)
   {
      string separator_chars="/";
      vector<string> substrings=stringfunc::decompose_string_into_substrings(
         filefunc::text_line[i],separator_chars);
      string curr_image_filename=substrings[1];
      image_filenames.push_back(curr_image_filename);

      string prefix=stringfunc::prefix(curr_image_filename);
      feature_filenames.push_back(
         features_subdir+"features_2D_"+prefix+".txt");
      cout << i << "   " << image_filenames.back() << "   "
           << filefunc::getbasename(feature_filenames.back()) << endl;
   } // loop over index i labeling image filenames
   
   typedef map<int,int> FEATUREID_IMAGEFREQ_MAP;
// independent int var = feature ID
// dependent int var = number of images in which feature was extracted

   FEATUREID_IMAGEFREQ_MAP featureid_imagefreq_map;
   FEATUREID_IMAGEFREQ_MAP::iterator iter1;

   typedef map<int,vector<int> > FEATUREID_IMAGEIDS_MAP;
// independent int var = feature ID
// dependent vector<int> var = image IDs in which feature was extracted   

   FEATUREID_IMAGEIDS_MAP featureid_imageids_map;
   FEATUREID_IMAGEIDS_MAP::iterator iter2;

   typedef map<int,int> FEATUREID_FEATUREINDEX_MAP;
// independent int var = feature ID
// dependent int var = feature index

   FEATUREID_FEATUREINDEX_MAP featureid_featureindex_map;
   FEATUREID_FEATUREINDEX_MAP::iterator iter3;

   int feature_index=0;
   for (int i=0; i<feature_filenames.size(); i++)
   {
      string curr_feature_filename=feature_filenames[i];
      filefunc::ReadInfile(curr_feature_filename);
      cout << i << " " 
           << filefunc::getbasename(curr_feature_filename) << endl;
      
      for (int f=0; f<filefunc::text_line.size(); f++)
      {
         vector<string> substrings=
            stringfunc::decompose_string_into_substrings(
               filefunc::text_line[f]);
         int feature_ID=stringfunc::string_to_number(substrings[1]);
         iter1=featureid_imagefreq_map.find(feature_ID);
         iter2=featureid_imageids_map.find(feature_ID);
         if (iter1==featureid_imagefreq_map.end())
         {
            featureid_imagefreq_map[feature_ID]=1;
            vector<int> V;
            V.push_back(i);
            featureid_imageids_map[feature_ID]=V;
            featureid_featureindex_map[feature_ID]=feature_index;
            feature_index++;
         }
         else
         {
            iter1->second=iter1->second+1;
            iter2->second.push_back(i);
         }
      } // loop over index f labeling features within current image
   } // loop over index i labeling images


   int n_feature_tracks=featureid_imagefreq_map.size();
   cout << "featureid_imagefreq_map.size() = " 
        << n_feature_tracks << endl;
   cout << "featureid_imageids_map.size() =  "
        << featureid_imageids_map.size() << endl;

// Generate probability distribution for feature image frequencies:

   vector<int> feature_frequency;
   for (iter1=featureid_imagefreq_map.begin(); iter1 != 
           featureid_imagefreq_map.end(); iter1++)
   {
      int feature_ID=iter1->first;
      feature_frequency.push_back(iter1->second);
      iter2=featureid_imageids_map.find(feature_ID);
      int n_images=iter2->second.size();
//      cout << "Feature ID = " << feature_ID
//           << " image frequency = " << iter1->second
//           << " n_images = " << n_images 
//           << endl;
   }

   prob_distribution prob_features(feature_frequency,400,0);
   prob_features.writeprobdists(false);

// Construct feature_imageID matrix:

   flann::Matrix<int>* feature_imageID_matrix_ptr=
      new flann::Matrix<int>(new int[n_feature_tracks*n_images],
      n_feature_tracks,n_images);
   
   for (iter2=featureid_imageids_map.begin(); iter2 != 
           featureid_imageids_map.end(); iter2++)
   {
      int feature_ID=iter2->first;
//      cout << "feature_ID = " << feature_ID << endl;
      iter3=featureid_featureindex_map.find(feature_ID);
      int feature_index=iter3->second;
//      cout << "feature_index = " << feature_index << endl;

      vector<int> image_IDs=iter2->second;
      for (int i=0; i<image_IDs.size(); i++)
      {
         int curr_image_ID=image_IDs[i];
         (*feature_imageID_matrix_ptr)[feature_index][curr_image_ID]=1;
      }
   } // loop over iter2 

// Inner products between different columns of
// *feature_imageID_matrix_ptr yields number of edges between
// different images within an image graph.  Ignore links between
// images whose number of edges are less than some minimal threshold.
// Export edge list to output text file.

   int n_edges_threshold=25;
   cout << "Enter n_edges_threshold:" << endl;
   cin >> n_edges_threshold;

   ofstream edges_stream;
   string edges_filename=bundler_IO_subdir+"image_edges.dat";
   filefunc::openfile(edges_filename,edges_stream);
   edges_stream << "# n_edges_threshold = " << n_edges_threshold << endl;
   edges_stream << endl;

   vector<int> curr_column;
   curr_column.reserve(n_feature_tracks);
   for (int c1=0; c1<n_images; c1++)
   {
      curr_column.clear();
      for (int r=0; r<n_feature_tracks; r++)
      {
         curr_column.push_back( (*feature_imageID_matrix_ptr)[r][c1] );
      }

      for (int c2=c1+1; c2<n_images; c2++)
      {
         int n_edges=0;
         for (int r=0; r<n_feature_tracks; r++)
         {
            if (curr_column[r]==0) continue;
            n_edges += (*feature_imageID_matrix_ptr)[r][c2];
         }
         if (n_edges < n_edges_threshold) continue;

         if (n_edges==0) continue;
         cout << c1 << " image = " << image_filenames[c1]
              << " " << c2 << " image = " << image_filenames[c2]
              << " n_edges = " << n_edges 
              << endl;
         edges_stream << c1 << " "
                      << image_filenames[c1] << " "
                      << c2 << " "
                      << image_filenames[c2] << " "
                      << n_edges << endl;
      }
      cout << endl;
      edges_stream << endl;
   }

   filefunc::closefile(edges_filename,edges_stream);

   delete feature_imageID_matrix_ptr;
}
