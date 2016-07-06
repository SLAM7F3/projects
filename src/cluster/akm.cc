// =========================================================================
// Approximate K-Means (AKM) class member function definitions
// =========================================================================
// Last modified on 8/30/13; 9/8/13; 4/4/14; 11/28/15
// =========================================================================

#include <iostream>
#include <set>
#include <string>
#include <string.h>
#include "cluster/akm.h"
#include "datastructures/descriptor.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "math/mathfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "time/timefuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::map;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void akm::allocate_member_objects()
{
   forward_matches_map_ptr=new FEATURE_MATCHES_MAP;
   backward_matches_map_ptr=new FEATURE_MATCHES_MAP;
}

void akm::initialize_member_objects()
{
   n_features_in_cluster=NULL;
   image_word_count=NULL;
   multi_image_word_occurrence=NULL;
   term_frequency=NULL;
   tfidf=NULL;

   inverse_document_frequency_matrix_ptr=NULL;
   tfidf_sparse_matrix_ptr=NULL;

   SIFT_descriptors=NULL;
   cluster_centers=NULL;
   new_cluster_centers=NULL;
   inverse_sqrt_covar_matrix_ptr=NULL;

   indices_matrix_ptr=NULL;
   dists_matrix_ptr=NULL;
   SIFT_descriptors_matrix_ptr=NULL;
   SIFT_descriptors2_matrix_ptr=NULL;
   cluster_centers_matrix_ptr=NULL;
   new_cluster_centers_matrix_ptr=NULL;

   n_queries_times_nn=-1;
   indices_ptr=NULL;
   dists_ptr=NULL;
   int_array_ptr=NULL;
   float_array_ptr=NULL;
   cluster_index_nn_ptr=NULL;
   forward_nn_ptr=NULL;
   backward_nn_ptr=NULL;
}		 

// ---------------------------------------------------------------------
akm::akm(bool flann_flag)
{
   FLANN_flag=flann_flag;
   allocate_member_objects();
   initialize_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

akm::akm(const akm& a)
{
   docopy(a);
}

// ---------------------------------------------------------------------
akm::~akm()
{
   delete [] n_features_in_cluster;
   delete [] image_word_count;
   delete [] multi_image_word_occurrence;
   delete [] term_frequency;
   delete [] tfidf;

   if (inverse_document_frequency_matrix_ptr != NULL)
   {
      delete [] inverse_document_frequency_matrix_ptr->ptr();
      delete inverse_document_frequency_matrix_ptr;
   }

   if (tfidf_sparse_matrix_ptr != NULL)
   {
      delete tfidf_sparse_matrix_ptr;
   }

   delete [] cluster_centers;
   delete [] new_cluster_centers;

   if (indices_matrix_ptr != NULL)
   {
      delete [] indices_matrix_ptr->ptr();
      delete indices_matrix_ptr;
   }
   
   if (dists_matrix_ptr != NULL)
   {
      delete [] dists_matrix_ptr->ptr();
      delete dists_matrix_ptr;
   }
   
   if (SIFT_descriptors_matrix_ptr != NULL)
   {
      delete [] SIFT_descriptors_matrix_ptr->ptr();
      delete SIFT_descriptors_matrix_ptr;
   }
   
   if (SIFT_descriptors2_matrix_ptr != NULL)
   {
      delete [] SIFT_descriptors2_matrix_ptr->ptr();
      delete SIFT_descriptors2_matrix_ptr;
   }

   if (cluster_centers_matrix_ptr != NULL)
   {
      delete [] cluster_centers_matrix_ptr->ptr();
      delete cluster_centers_matrix_ptr;
   }

   if (new_cluster_centers_matrix_ptr != NULL)
   {
      delete [] new_cluster_centers_matrix_ptr->ptr();
      delete new_cluster_centers_matrix_ptr;
   }
   
   delete indices_ptr;
   delete dists_ptr;
   delete [] int_array_ptr;
   delete [] float_array_ptr;
   delete cluster_index_nn_ptr;
   delete forward_nn_ptr;
   delete backward_nn_ptr;

// Need to loop over kdtree_index_ptrs and delete each of its entries:

   delete forward_matches_map_ptr;
   delete backward_matches_map_ptr;
}

// ---------------------------------------------------------------------
void akm::docopy(const akm& a)
{
}

// Overload = operator:

akm& akm::operator= (const akm& a)
{
   if (this==&a) return *this;
   docopy(a);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const akm& a)
{
   outstream << endl;
   return outstream;
}

// =========================================================================
// Approximate K-means member functions
// =========================================================================

// Member function reset_params() destroys and dynamically
// instantiates most previously existing arrays and FLANN matrices.  

void akm::reset_params(int n,int d,int k,int num_iters)
{
//   cout << "inside akm::reset_params() #1" << endl;
   
   this->N=n;
   this->D=d;
   this->K=basic_math::min(k,n);
   this->n_iters=num_iters;

//   cout << "FLANN_flag = " << FLANN_flag << endl;
   if (FLANN_flag)
   {
      if (indices_matrix_ptr != NULL)
      {
         delete [] indices_matrix_ptr->ptr();
         delete indices_matrix_ptr;
      }

      if (dists_matrix_ptr != NULL)
      {
         delete [] dists_matrix_ptr->ptr();
         delete dists_matrix_ptr;
      }
      
      indices_matrix_ptr=new flann::Matrix<int>(new int[N*D],N,D);
      dists_matrix_ptr=new flann::Matrix<float>(new float[N*D],N,D);
      if (n_iters > 0)
      {
         if (cluster_centers_matrix_ptr != NULL)
         {
            delete [] cluster_centers_matrix_ptr->ptr();
            delete cluster_centers_matrix_ptr;
         }
         
         if (new_cluster_centers_matrix_ptr != NULL)
         {
            delete [] new_cluster_centers_matrix_ptr->ptr();
            delete new_cluster_centers_matrix_ptr;
         }
         
         cluster_centers_matrix_ptr=
            new flann::Matrix<float>(new float[K*D],K,D);
         new_cluster_centers_matrix_ptr=
            new flann::Matrix<float>(new float[K*D],K,D);
      }
   }
   else
   {
      delete [] cluster_centers;
      delete [] new_cluster_centers;

      cluster_centers=new float[K*D];
      new_cluster_centers=new float[K*D];
   } // FLANN_flag conditional

   delete [] n_features_in_cluster;
   n_features_in_cluster=new int[K];

// Reset arrays which count word frequency within each image document.
// But do NOT reset arrays which accumulate word frequency within
// documents across entire image database:

   delete [] image_word_count;
   image_word_count=new int[K];
}

// ---------------------------------------------------------------------
// Member function reset_params() destroys and dynamically
// instantiates most previously existing arrays and FLANN matrices.  

void akm::reset_params(int n,int d,int k)
{
//   cout << "inside akm::reset_params() #2" << endl;
   
   this->N=n;
   this->D=d;
   this->K=k;

//   cout << "N = " << N << " D = " << D << " K = " << K << endl;
//   cout << "FLANN_flag = " << FLANN_flag << endl;

   if (FLANN_flag)
   {
      if (indices_matrix_ptr != NULL)
      {
         delete [] indices_matrix_ptr->ptr();
         delete indices_matrix_ptr;
      }

      if (dists_matrix_ptr != NULL)
      {
         delete [] dists_matrix_ptr->ptr();
         delete dists_matrix_ptr;
      }
      
      indices_matrix_ptr=new flann::Matrix<int>(new int[N*D],N,D);
      dists_matrix_ptr=new flann::Matrix<float>(new float[N*D],N,D);
   }

// Reset arrays which count word frequency within each image document.
// But do NOT reset arrays which accumulate word frequency within
// documents across entire image database:

   if (image_word_count != NULL) delete [] image_word_count;
   image_word_count=new int[K];
}

// ---------------------------------------------------------------------
// Member function randomly_initialize_cluster_centers() randomly
// picks K descriptors from all N descriptors as starting cluster
// centers.

void akm::randomly_initialize_cluster_centers()
{
   string banner="Randomly initializing cluster centers";
   outputfunc::write_banner(banner);

   vector<int> cluster_center_IDs=mathfunc::random_sequence(N,K);
   cout << "cluster_center_IDs.size() = " << cluster_center_IDs.size()
        << endl;
   
   if (FLANN_flag)
   {
      randomly_initialize_FLANN_cluster_centers(cluster_center_IDs);
   }
   else
   {
      randomly_initialize_FASTANN_cluster_centers(cluster_center_IDs);
   }
}

void akm::randomly_initialize_FLANN_cluster_centers(
   const vector<int>& cluster_center_IDs)
{
//   cout << "inside akm::randomly_initialize_FLANN_cluster_centers()" << endl;

   for (unsigned int c=0; c<K; c++)
   {
//      cout << "Cluster center c = " << c << endl;
      int center_ID=cluster_center_IDs[c];
      for (unsigned int d=0; d<D; d++)
      {
         (*cluster_centers_matrix_ptr)[c][d]=
            (*SIFT_descriptors_matrix_ptr)[center_ID][d];

//         if ( (c < 4 || c > K-4) )
//            if ( (c < 4 || c > K-4) && (d < 4 || d > 124) )
//            {
//               cout << "c = " << c
//                    << " d = " << d 
//                    << " cluster_center = " 
//                    << (*cluster_centers_matrix_ptr)[c][d] << endl;
//            }
      } // loop over index d 
   } // loop over index c labeling cluster centers
}

void akm::randomly_initialize_FASTANN_cluster_centers(
   const vector<int>& cluster_center_IDs)
{
   for (unsigned int c=0; c<K; c++)
   {
//      cout << "Cluster center c = " << c << endl;
      for (unsigned int d=0; d<D; d++)
      {
         cluster_centers[c*D+d]=
            SIFT_descriptors[cluster_center_IDs[c]*D+d];
//         if (d < 4 || d > 124)
//         {
//            cout << "     d = " << d 
//                 << " cluster_center = " << cluster_centers[c*D+d] << endl;
//         }
      } // loop over index d
   } // loop over index c labeling cluster centers
}

// ---------------------------------------------------------------------
// Member function iteratively_refine_clusters() loops over all N SIFT
// descriptors and finds their closest cluster centers.  It then
// resets the cluster centers as the centroid of the SIFT features
// which belong to a cluster's Voronoi cell.  This
// evaluation-minimization procedure is performed for n_iters
// iterations.

void akm::iteratively_refine_clusters()
{
   if (FLANN_flag)
   {
      iteratively_refine_FLANN_clusters();
   }
   else
   {
      iteratively_refine_FASTANN_clusters();
   }
}

// ---------------------------------------------------------------------
void akm::iteratively_refine_FLANN_clusters() 
{
   cout << "Iteratively refining FLANN clusters" << endl;
   for (unsigned int iter=0; iter<n_iters; iter++)
   {
      cout << "========================================================="
           << endl;
      cout << "iter = " << iter << endl;
      cout << "========================================================="
           << endl;

/*
// Create index automatically tuned to offer best performance by
// choosing optimal index type (randomized kd-trees, hierarchical kmeans,
// linear) and parameters for provided dataset:

      float target_precision=0.9;
//   float target_precision=0.99;
      float build_weight=0.01;
      float memory_weight=0;
      float sample_fraction=0.1;

      flann::Index<flann::L2<float> > index(
         *cluster_centers_matrix_ptr, flann::AutotunedIndexParams(
            target_precision,build_weight,memory_weight,sample_fraction));
*/

// Construct a randomized kd-tree index using 8 kd-trees

      delete cluster_index_nn_ptr;
      cluster_index_nn_ptr=new flann::Index<flann::L2<float> >
         (*cluster_centers_matrix_ptr, flann::KDTreeIndexParams(8));

      cout << "Building cluster index" << endl;
      cluster_index_nn_ptr->buildIndex();                
      cout << "Finished building cluster index" << endl;

// Find (approximate) closest cluster centers for all N SIFT
// descriptors:

      cout << "N = " << N << endl;
      outputfunc::print_elapsed_time();

/*
      flann::SearchParams search_params;
      search_params.checks=128;
      search_params.cores=4;
      search_params.use_heap=flann::FLANN_False;
      cluster_index_nn_ptr->knnSearch(
         *SIFT_descriptors_matrix_ptr, *indices_matrix_ptr, *dists_matrix_ptr,
         1, search_params);
*/

      cluster_index_nn_ptr->knnSearch(
         *SIFT_descriptors_matrix_ptr, *indices_matrix_ptr, *dists_matrix_ptr,
         1, flann::SearchParams(64));


      cout << "Finished performing knnSearch for N SIFT features" << endl;

// Count number of features assigned to each cluster.  Also update
// cost function which equals integral of squared feature distances to
// cluster centers:

      memset(new_cluster_centers_matrix_ptr->ptr(),0,K*D*sizeof(float));
      memset(n_features_in_cluster,0,K*sizeof(int));

      cost_function=0;
      for (unsigned int n=0; n<N; n++)
      {
         int curr_cluster_center_ID=(*indices_matrix_ptr)[n][0];
         if (n%100==0)
         {
//            cout << "n = " << n 
//                 << " curr_cluster_center_ID = " << curr_cluster_center_ID
//                 << endl;
         }
         n_features_in_cluster[curr_cluster_center_ID] += 1;
      
         for (unsigned int d=0; d<D; d++)
         {
            (*new_cluster_centers_matrix_ptr)[curr_cluster_center_ID][d] += 
               (*SIFT_descriptors_matrix_ptr)[n][d];

            float curr_delta= 
               (*SIFT_descriptors_matrix_ptr)[n][d]- 
               (*cluster_centers_matrix_ptr)[curr_cluster_center_ID][d];
            cost_function += sqr(curr_delta/256.0);
         } // loop over index d
      } // loop over index n labeling SIFT descriptors

      cost_function /= N;
      cout << "cost_function = " << cost_function << endl;


// Recompute cluster centers as centroids of SIFT descriptors which
// share common cluster index:

      for (unsigned int c=0; c<K; c++)
      {

// Make sure number of features in current cluster at least equals 2!

         if (n_features_in_cluster[c] <= 1) continue;
         
         for (unsigned int d=0; d<D; d++)
         {
            (*cluster_centers_matrix_ptr)[c][d]=
               (*new_cluster_centers_matrix_ptr)[c][d]/
               n_features_in_cluster[c];
            if ( (c < 4 || c > K-4) && (d < 4 || d > 124) )
            {
//               cout << "c = " << c
//                    << " d = " << d 
//                    << " cluster_centers[c][d] = " 
//                    << (*cluster_centers_matrix_ptr)[c][d] << endl;
            }
         } // loop over index d 
         if (c < 4 || c > K-4)
         {
            cout << "cluster = " << c << " # features in cluster = "
                 << n_features_in_cluster[c] << endl;
//            cout << endl;
         }

      } // loop over index c labeling cluster centers

// Check that integral of all clustered features equals total number
// of features N:

      int n_features_in_cluster_integral=0;
      vector<double> n_features_per_cluster;
      for (unsigned int c=0; c<K; c++)
      {
//         cout << "c = " << c
//              << " n_features_in_cluster = "
//              << n_features_in_cluster[c] << endl;
         n_features_in_cluster_integral += n_features_in_cluster[c];
         n_features_per_cluster.push_back(n_features_in_cluster[c]);
      }
      mu_n_features_per_cluster=mathfunc::mean(n_features_per_cluster);
      sigma_n_features_per_cluster=mathfunc::std_dev(n_features_per_cluster);

      cout << "Total number of features N = " << N << endl;
      cout << "n_features_in_cluster_integral = " 
           << n_features_in_cluster_integral << endl;
      cout << "n_features_per_cluster = "
           << mu_n_features_per_cluster << " +/- "
           << sigma_n_features_per_cluster << endl;
      outputfunc::print_elapsed_time();

   } // loop over iter index
}

// ---------------------------------------------------------------------
// Member function iteratively_refine_clusters()

void akm::iteratively_refine_FASTANN_clusters()
{
   
   for (unsigned int iter=0; iter<n_iters; iter++)
   {
      cout << "========================================================="
           << endl;
      cout << "iter = " << iter << endl;
      cout << "========================================================="
           << endl;

      fastann::nn_obj<float>* nnobj_kdt = fastann::nn_obj_build_kdtree(
         cluster_centers, K, D, 8, 768);

// Find (approximate) closest cluster centers for all N SIFT
// descriptors:

      vector<unsigned> argmins_kdt(N);
      vector<float> mins_kdt(N);
      nnobj_kdt->search_nn(SIFT_descriptors, N, &argmins_kdt[0], &mins_kdt[0]);

// Recompute cluster centers as centroids of SIFT descriptors which
// share common cluster index:

      memset(new_cluster_centers,0,K*D*sizeof(float));
      memset(n_features_in_cluster,0,K*sizeof(int));

      cost_function=0;
      for (unsigned int n=0; n<N; n++)
      {
         int curr_cluster_center_ID=argmins_kdt[n];
//      cout << "n = " << n 
//           << " curr_cluster_center_ID = " << curr_cluster_center_ID
//           << endl;
         n_features_in_cluster[curr_cluster_center_ID] += 1;
      
         for (unsigned int d=0; d<D; d++)
         {
            new_cluster_centers[curr_cluster_center_ID*D+d] += 
               SIFT_descriptors[n*D+d];
            cost_function += 
               sqr(SIFT_descriptors[n*D+d]-
               cluster_centers[curr_cluster_center_ID*D+d]);
            
         } // loop over index d
      } // loop over index n labeling SIFT descriptors

      cost_function /= N;
      cout << "cost_function = " << cost_function << endl;

/*
  int sum=0;
  for (unsigned int c=0; c<K; c++)
  {
  cout << "c = " << c
  << " n_features_in_cluster = "
  << n_features_in_cluster[c] << endl;
  sum += n_features_in_cluster[c];
  }
  cout << "sum = " << sum << endl;
*/

      for (unsigned int c=0; c<K; c++)
      {
         for (unsigned int d=0; d<D; d++)
         {
            cluster_centers[c*D+d]=new_cluster_centers[c*D+d]/
               n_features_in_cluster[c];
            if (c < 5 && (d < 4 || d > 124))
            {
               cout << "c = " << c 
                    << " d = " << d 
                    << " cluster_center = " << cluster_centers[c*D+d] << endl;
            }
         } // loop over index d 
         if (c < 5) cout << endl;
      } // loop over index c labeling cluster centers
   
   } // loop over iter index
}

// =========================================================================
// Approximate SIFT feature matching member functions
// =========================================================================

// Member function load_SIFT_descriptors() loads input SIFT feature
// information into member FLANN matrix *SIFT_descriptors_matrix_ptr.
// If the number of input features equals 0, this boolean method
// returns false.

bool akm::load_SIFT_descriptors(
   const vector<feature_pair>* currimage_feature_info_ptr)
{
//   cout << "inside akm::load_SIFT_descriptors() #1" << endl;

   N=currimage_feature_info_ptr->size();
   if (N==0) return false;

   unsigned Nprev=0;
   if (SIFT_descriptors_matrix_ptr != NULL)
   {
      Nprev=SIFT_descriptors_matrix_ptr->rows;
   }
   D=currimage_feature_info_ptr->back().second->get_mdim(); // = 128 for SIFT

   if (N <= Nprev)
   {

// Reuse existing *SIFT_descriptors_matrix_ptr.  Update its number of
// rows:
      
      SIFT_descriptors_matrix_ptr->rows=N;
   }
   else
   {
      if (SIFT_descriptors_matrix_ptr != NULL)
      {
         delete [] SIFT_descriptors_matrix_ptr->ptr();
         delete SIFT_descriptors_matrix_ptr;
      }
      SIFT_descriptors_matrix_ptr=
         new flann::Matrix<float>(new float[N*D],N,D);
   }

   for (unsigned int f=0; f<N; f++)
   {
      for (unsigned int d=0; d<D; d++)
      {
         (*SIFT_descriptors_matrix_ptr)[f][d]=
            currimage_feature_info_ptr->at(f).second->get(d);
      } // loop over index d
   } // loop over index f labeling SIFT features
   return true;
}

// ---------------------------------------------------------------------
void akm::pushback_SIFT_descriptors_matrix_ptr()
{
   SIFT_descriptor_matrices_ptrs.push_back(SIFT_descriptors_matrix_ptr);
}

// ---------------------------------------------------------------------
bool akm::load_SIFT_descriptors(const vector<descriptor*>* D_ptrs_ptr)
{
//   cout << "inside akm::load_SIFT_descriptors() #2" << endl;

   N=D_ptrs_ptr->size();
   if (N==0) return false;

   if (SIFT_descriptors_matrix_ptr != NULL)
   {
      delete [] SIFT_descriptors_matrix_ptr->ptr();
      delete SIFT_descriptors_matrix_ptr;
   }


   D=D_ptrs_ptr->back()->get_mdim();
//   cout << "N = " << N << " D = " << D << endl;
   SIFT_descriptors_matrix_ptr=
      new flann::Matrix<float>(new float[N*D],N,D);

   for (unsigned int f=0; f<N; f++)
   {
      descriptor* curr_D_ptr=D_ptrs_ptr->at(f);
      for (unsigned int d=0; d<D; d++)
      {
         (*SIFT_descriptors_matrix_ptr)[f][d]=curr_D_ptr->get(d);
//         cout << "f = " << f << " d = " << d << " val = " 
//              << curr_D_ptr->get(d) << endl;
      } // loop over index d
   } // loop over index f labeling SIFT features

   return true;
}

// ---------------------------------------------------------------------
// This overloaded version of load_SIFT_descriptors() takes in an
// unsigned char array which presumably was read in from a binary file
// wherein SIFT descriptors are represented as byte strings.  It also
// takes in the number of SIFT features contained within the
// byte_array.  If input parameter n_skip > 1, then this method only
// loads every n_skip-th SIFT feature into
// *SIFT_descriptors_matrix_ptr.

bool akm::load_SIFT_descriptors(
   int n_total_features,int d_dims,const unsigned char* char_array,int n_skip)
{
   cout << "inside akm::load_SIFT_descriptors() #3" << endl;

   unsigned int n_features_to_load=n_total_features/n_skip;
   cout << "n_features to load = n_total_SIFT_features/n_skip = "
        << n_features_to_load << endl;
   if (n_features_to_load <= 0) return false;

   cout << "n_total_SIFT_features = " << n_total_features << endl;
   cout << "n_skip = " << n_skip << endl;

   if (SIFT_descriptors_matrix_ptr != NULL)
   {
      delete [] SIFT_descriptors_matrix_ptr->ptr();
      delete SIFT_descriptors_matrix_ptr;
   }

   N=n_features_to_load;
   D=d_dims;
   SIFT_descriptors_matrix_ptr=
      new flann::Matrix<float>(new float[N*D],N,D);

   unsigned int char_counter=0;
   unsigned int n_all_zero_descriptors=0;
   unsigned int n_loaded_features=0;
   for (unsigned int f=0; f<n_features_to_load; f++)
   {
      if (f%1000000==0) cout << f << " " << flush;
      int byte_skip=f*(n_skip-1)*D;

      int n_zero_values=0;
      for (unsigned int d=0; d<D; d++)
      {
         unsigned char c=char_array[char_counter+byte_skip];
         int descriptor=stringfunc::unsigned_char_to_ascii_integer(c);
         if (descriptor==0) n_zero_values++;
         (*SIFT_descriptors_matrix_ptr)[n_loaded_features][d]=descriptor;
         char_counter++;
      } // loop over index d

// On 4/3/12, we empirically found the hard way that some raw SIFT
// descriptors read in from a cumulative binary file have all zero
// entries.  Since SIFT descriptors should never equal (0,0,...,0,0),
// we discard such erroneous features:

      if (n_zero_values==128)
      {
         n_all_zero_descriptors++;
      }
      else
      {
         n_loaded_features++;
      }
   } // loop over index f labeling SIFT features

   cout << endl;
   cout << "Total number of SIFT features supposedly stored in cumulative binary file = " << n_total_features << endl;
   cout << "Number of SIFT features attempted to load = "
        << n_features_to_load << endl;
   cout << "Number of erroneous all-zero valued descriptors = "
        << n_all_zero_descriptors << endl;
   cout << "Actual number of loaded SIFT features = "
        << n_loaded_features << endl;
   N=n_loaded_features;

   return true;
}

// ---------------------------------------------------------------------
bool akm::load_SIFT_descriptors(
   int currimage_ID,const vector<int>* image_IDs_ptr,
   const vector<descriptor*>* D_ptrs_ptr)
{
   cout << "inside akm::load_SIFT_descriptors() #4" << endl;

   N=0;
   for (unsigned int n=0; n<image_IDs_ptr->size(); n++)
   {
      if (image_IDs_ptr->at(n)==currimage_ID) N++;
   }
   if (N==0) return false;

   if (SIFT_descriptors_matrix_ptr != NULL)
   {
      delete [] SIFT_descriptors_matrix_ptr->ptr();
      delete SIFT_descriptors_matrix_ptr;
   }

   D=D_ptrs_ptr->back()->get_mdim();
   SIFT_descriptors_matrix_ptr=
      new flann::Matrix<float>(new float[N*D],N,D);

   int f=0;
   for (unsigned int n=0; n<D_ptrs_ptr->size(); n++)
   {
      if (image_IDs_ptr->at(n) != currimage_ID) continue;

      descriptor* curr_D_ptr=D_ptrs_ptr->at(n);
      for (unsigned int d=0; d<D; d++)
      {
         (*SIFT_descriptors_matrix_ptr)[f][d]=curr_D_ptr->get(d);
      } // loop over index d
      f++;
   } // loop over index n labeling all SIFT features

   return true;
}

// ---------------------------------------------------------------------
// Member function whiten_SIFT_descriptors()

void akm::whiten_SIFT_descriptors()
{
//    cout << "inside akm::whiten_SIFT_descriptors()" << endl;
   string banner="Whitening SIFT descriptors";
   outputfunc::write_banner(banner);
   cout << "n_features = " << N << endl;

   if (inverse_sqrt_covar_matrix_ptr==NULL)
   {
      cout << "Error: inverse_sqrt_covar_matrix_ptr=NULL!" << endl;
      return;
   }

   flann::Matrix<float>* inverse_sqrt_covar=
      new flann::Matrix<float>(new float[D*D],D,D);
   for (unsigned int i=0; i<D; i++)
   {
      for (unsigned int j=0; j<D; j++)
      {
         (*inverse_sqrt_covar)[i][j]=inverse_sqrt_covar_matrix_ptr->
            get(i,j);
      }
   }

   float* transformed_row=new float[D];
   for (unsigned int f=0; f<N; f++)
   {
//      if (f%1000000==0) cout << endl;
//      if (f%100000==0) cout << f << " " << flush;
      outputfunc::update_progress_fraction(f,100000,N);

      int zero_component_counter=0;      
      for (unsigned int i=0; i<D; i++)
      {
         transformed_row[i]=0;
         for (unsigned int j=0; j<D; j++)
         {
            transformed_row[i] += (*inverse_sqrt_covar)[i][j] * 
               (*SIFT_descriptors_matrix_ptr)[f][j];
         } // loop over index j
         if (nearly_equal(transformed_row[i],0)) zero_component_counter++;
      } // loop over index i
      
      for (unsigned int d=0; d<D; d++)
      {
         (*SIFT_descriptors_matrix_ptr)[f][d]=transformed_row[d];
      } // loop over index d
   } // loop over index n labeling SIFT features

   delete [] inverse_sqrt_covar->ptr();
   delete inverse_sqrt_covar;

   delete [] transformed_row;
   cout << endl;
}

// ---------------------------------------------------------------------
// Member function load_SIFT_descriptors2() load input SIFT feature
// information into member FLANN matrix *SIFT_descriptors2_matrix_ptr.

bool akm::load_SIFT_descriptors2(
   const vector<feature_pair>* nextimage_feature_info_ptr)
{
//   cout << "inside akm::load_SIFT_descriptors2()" << endl;

   N=nextimage_feature_info_ptr->size();
   if (N==0) return false;
   
   unsigned Nprev=0;
   if (SIFT_descriptors2_matrix_ptr != NULL)
   {
      Nprev=SIFT_descriptors2_matrix_ptr->rows;
   }

   D=nextimage_feature_info_ptr->back().second->get_mdim(); // = 128 for SIFT

   if (N <= Nprev)
   {

// Reuse existing *SIFT_descriptors2_matrix_ptr.  Update its number of
// rows:
      
      SIFT_descriptors2_matrix_ptr->rows=N;
   }
   else
   {
      if (SIFT_descriptors2_matrix_ptr != NULL)
      {
         delete [] SIFT_descriptors2_matrix_ptr->ptr();
         delete SIFT_descriptors2_matrix_ptr;
      }
      SIFT_descriptors2_matrix_ptr=
         new flann::Matrix<float>(new float[N*D],N,D);
   }

   for (unsigned int f=0; f<N; f++)
   {
      for (unsigned int d=0; d<D; d++)
      {
         (*SIFT_descriptors2_matrix_ptr)[f][d]=
            nextimage_feature_info_ptr->at(f).second->get(d);
      } // loop over index d
   } // loop over index f labeling SIFT features

   return true;
}

// ---------------------------------------------------------------------
// This overloaded version of load_SIFT_descriptors2() takes in
// several STL vectors of feature_pairs.  It loads all the input
// feature_pairs into member FLANN matrix *SIFT_descriptors2_matrix_ptr.

bool akm::load_SIFT_descriptors2(
   const vector<vector<feature_pair> >& currimage_feature_infos)
{
//   cout << "inside akm::load_SIFT_descriptors2()" << endl;

   N=0;
   for (unsigned int i=0; i<currimage_feature_infos.size(); i++)
   {
      N += currimage_feature_infos[i].size();
   }
   if (N==0) return false;

   if (SIFT_descriptors2_matrix_ptr != NULL)
   {
      delete [] SIFT_descriptors2_matrix_ptr->ptr();
      delete SIFT_descriptors2_matrix_ptr;
   }
   
   D=currimage_feature_infos.back().back().second->get_mdim();
   SIFT_descriptors2_matrix_ptr=
      new flann::Matrix<float>(new float[N*D],N,D);

   for (unsigned int i=0; i<currimage_feature_infos.size(); i++)
   {
      for (unsigned int f=0; f<currimage_feature_infos[i].size(); f++)
      {
         descriptor* curr_D_ptr=currimage_feature_infos[i].at(f).second;
         for (unsigned int d=0; d<D; d++)
         {
            (*SIFT_descriptors2_matrix_ptr)[f][d]=curr_D_ptr->get(d);
         } // loop over index d
      } // loop over index f labeling SIFT features
   } // loop over index i

   return true;
}

// ---------------------------------------------------------------------
// Member function initialize_randomized_kdtree_index() imports a
// FLANN matrix of floats containing SIFT descriptors for some image.
// It constructs a randomized kd-tree index using 8 kd-trees and
// appends it to member STL vector kdtree_index_ptrs.

void akm::initialize_randomized_kdtree_index()
{
   cout << "inside akm::initialize_randomized_kdtree_index()" << endl;
   
   if (!FLANN_flag) return;

   kdtree_index_ptrs.push_back(
      new flann::Index<flann::L2<float> >
      (*SIFT_descriptors_matrix_ptr, flann::KDTreeIndexParams(8)) );
   kdtree_index_ptrs.back()->buildIndex();                             

   outputfunc::enter_continue_char();
}

// ---------------------------------------------------------------------
// Member function initialize_SIFT_feature_search() 
// resets FLANN index pointers forward_nn_ptr and backward_nn_ptr to
// equal entries within member STL vector kdtree_index_ptrs.  It
// similarly resets SIFT_descriptors[2]_matrix_ptr to equal entries
// within member STL vector SIFT_descriptor_matrices_ptrs.

void akm::initialize_SIFT_feature_search(int i_forward,int j_backward)
{
   cout << "inside akm::initialize_SIFT_feature_search()" << endl;

   forward_nn_ptr=kdtree_index_ptrs[i_forward];
   backward_nn_ptr=kdtree_index_ptrs[j_backward];
//   cout << "forward_nn_ptr = " << forward_nn_ptr
//        << " backward_nn_ptr = " << backward_nn_ptr << endl;

//   SIFT_descriptors_matrix_ptr=
//      SIFT_descriptor_matrices_ptrs[i_forward];
 //  SIFT_descriptors2_matrix_ptr=
//      SIFT_descriptor_matrices_ptrs[j_backward];

   SIFT_descriptors2_matrix_ptr=
      SIFT_descriptor_matrices_ptrs[i_forward];
   SIFT_descriptors_matrix_ptr=
      SIFT_descriptor_matrices_ptrs[j_backward];
}

// ---------------------------------------------------------------------
// Member function initialize_forward_SIFT_feature_search() deletes, 
// re-instantiates and rebuilds FLANN index object *forward_nn_ptr
// from *SIFT_descriptors2_matrix_ptr.

void akm::initialize_forward_SIFT_feature_search()
{
//   cout << "inside akm::initialize_forward_SIFT_feature_search()" << endl;

   if (!FLANN_flag) return;

   delete forward_nn_ptr;

// Construct a randomized kd-tree index using 4 (8) kd-trees

// On 4/30/13, we empirically observed that SIFT/ASIFT feature
// matching rates for GEO pass 20120105_1402 noticeably increase if
// the number of kd-trees is reduced from 8 to 4.  And the resulting
// 3D reconstruction is not adversely affected by this change.

//   unsigned int seed=1;
//   unsigned int seed=timefunc::secs_since_Y2K();
   int large_prime=179424673;
   unsigned int seed=timefunc::secs_since_Y2K()%large_prime;
//   cout << "seed = " << seed << endl;

   flann::seed_random(seed);   

   forward_nn_ptr=new flann::Index<flann::L2<float> >
      (*SIFT_descriptors2_matrix_ptr, 
      flann::KDTreeIndexParams(4));
//      flann::KDTreeIndexParams(8));
//      flann::KDTreeIndexParams(12));
   forward_nn_ptr->buildIndex();          
//   cout << "Finished building forward nearest neighbor indexes" << endl;
}

// ---------------------------------------------------------------------
// Member function initialize_backward_SIFT_feature_search() deletes, 
// re-instantiates and rebuilds FLANN index object *backward_nn_ptr
// from *SIFT_descriptors_matrix_ptr.

void akm::initialize_backward_SIFT_feature_search()
{
//   cout << "inside akm::initialize_backward_SIFT_feature_search()" << endl;
   
   if (!FLANN_flag) return;

   delete backward_nn_ptr;

//   unsigned int seed=1;
//   unsigned int seed=timefunc::secs_since_Y2K();
   int large_prime=179424673;
   unsigned int seed=timefunc::secs_since_Y2K()%large_prime;
   flann::seed_random(seed);   

// Construct a randomized kd-tree index using 4 (8) kd-trees

   backward_nn_ptr=new flann::Index<flann::L2<float> >
      (*SIFT_descriptors_matrix_ptr, 
      flann::KDTreeIndexParams(4));
//      flann::KDTreeIndexParams(8));
   backward_nn_ptr->buildIndex();                             
//   cout << "Finished building backward nearest neighbor indexes" << endl;
}

// ---------------------------------------------------------------------
// Member function find_only_forward_SIFT_matches() finds
// one-directional candidate matches between the features within
// *SIFT_descriptors_matrix_ptr and *SIFT_descriptors2_matrix_ptr.
// The IDs for such one-directional tiepoint pairs are stored within
// member STL vector initial_SIFT_matches.  Call this method when
// matching image i with image j > i.

void akm::find_only_forward_SIFT_matches(double max_distance_ratio)
{
//   cout << "inside akm::find_forward_SIFT_matches()" << endl;
//   outputfunc::print_elapsed_time();
//   cout << "SIFT_descriptors_matrix_ptr->rows = "
//        << SIFT_descriptors_matrix_ptr->rows << endl;
  
   const int nn=2;
   unsigned int n_queries=SIFT_descriptors_matrix_ptr->rows;
   unsigned int curr_n_queries_times_nn=n_queries*nn;
//   cout << "n_queries = " << n_queries << endl;

   if (int_array_ptr==NULL || curr_n_queries_times_nn > n_queries_times_nn)
   {
      n_queries_times_nn=curr_n_queries_times_nn;

      delete [] int_array_ptr;
      delete [] float_array_ptr;
      delete indices_ptr;
      delete dists_ptr;

      int_array_ptr=new int[curr_n_queries_times_nn];
      float_array_ptr=new float[curr_n_queries_times_nn];
      indices_ptr=new flann::Matrix<int>(int_array_ptr,n_queries,nn);
      dists_ptr=new flann::Matrix<float>(float_array_ptr,n_queries,nn);
   }
  
   flann::SearchParams fsp;
//   fsp.checks=128;
   fsp.checks=96;
//   fsp.checks=64;
   fsp.cores=0;
/* 
   cout << "fsp.checks = " << fsp.checks << endl;
   cout << "fsp.eps = " << fsp.eps << endl;
   cout << "fsp.sorted = " << fsp.sorted << endl;
   cout << "fsp.max_neighbors = " << fsp.max_neighbors << endl;
   cout << "fsp.use_heap = " << fsp.use_heap << endl;
   cout << "fsp.cores = " << fsp.cores << endl;
   cout << "fsp.matrices_in_gpu_ram = " << fsp.matrices_in_gpu_ram << endl;
//   outputfunc::enter_continue_char();
*/

//   cout << "Before call to forward_nn_ptr->knnSearch()" << endl;
   forward_nn_ptr->knnSearch(
      *SIFT_descriptors_matrix_ptr, 
      *indices_ptr,*dists_ptr,nn, 
      fsp);
   
// flann::SearchParams(128));

   initial_SIFT_matches.clear();
   for (unsigned int f=0; f<n_queries; f++)
   {
      float nearest_neighbor_dist=(*dists_ptr)[f][0];
      float next_to_nearest_neighbor_dist=(*dists_ptr)[f][1];
//      cout << "nearest_neighbor_dist = " << nearest_neighbor_dist
//           << " next_to_nearest_neighbor_dist = "
//           << next_to_nearest_neighbor_dist << endl;
//      float ratio=nearest_neighbor_dist/next_to_nearest_neighbor_dist;
//      cout << "ratio = " << ratio << endl;
      if (nearest_neighbor_dist/next_to_nearest_neighbor_dist > 
      max_distance_ratio) continue;

      int nearest_neighbor_index=(*indices_ptr)[f][0];
      pair<int,int> P(f,nearest_neighbor_index);
      initial_SIFT_matches.push_back(P);
   }
}

// ---------------------------------------------------------------------
// Member function find_only_backward_SIFT_matches() finds
// one-directional candidate matches between the features within
// *SIFT_descriptors2_matrix_ptr and *SIFT_descriptors_matrix_ptr.
// The IDs for such one-directional tiepoint pairs are stored within
// member STL vector initial_SIFT_matches.  Call this method when
// matching image j > i with image i.

void akm::find_only_backward_SIFT_matches(double max_distance_ratio)
{
//   cout << "inside akm::find_only_backward_SIFT_matches()" << endl;
  
   const int nn=2;
   unsigned int n_queries=SIFT_descriptors2_matrix_ptr->rows;
   unsigned int curr_n_queries_times_nn=n_queries*nn;
//   cout << "n_queries = " << n_queries << endl;

   if (int_array_ptr==NULL || curr_n_queries_times_nn > n_queries_times_nn)
   {
      n_queries_times_nn=curr_n_queries_times_nn;

      delete [] int_array_ptr;
      delete [] float_array_ptr;
      delete indices_ptr;
      delete dists_ptr;

      int_array_ptr=new int[curr_n_queries_times_nn];
      float_array_ptr=new float[curr_n_queries_times_nn];
      indices_ptr=new flann::Matrix<int>(int_array_ptr,n_queries,nn);
      dists_ptr=new flann::Matrix<float>(float_array_ptr,n_queries,nn);
   }
  
   flann::SearchParams fsp;
//   fsp.checks=128;
   fsp.checks=96;
//   fsp.checks=64;
   fsp.cores=0;
/* 
   cout << "fsp.checks = " << fsp.checks << endl;
   cout << "fsp.eps = " << fsp.eps << endl;
   cout << "fsp.sorted = " << fsp.sorted << endl;
   cout << "fsp.max_neighbors = " << fsp.max_neighbors << endl;
   cout << "fsp.use_heap = " << fsp.use_heap << endl;
   cout << "fsp.cores = " << fsp.cores << endl;
   cout << "fsp.matrices_in_gpu_ram = " << fsp.matrices_in_gpu_ram << endl;
//   outputfunc::enter_continue_char();
*/

//   cout << "Before call to backward_nn_ptr->knnSearch()" << endl;
   backward_nn_ptr->knnSearch(
      *SIFT_descriptors2_matrix_ptr, 
      *indices_ptr,*dists_ptr,nn,fsp);

   initial_SIFT_matches.clear();
   for (unsigned int f=0; f<n_queries; f++)
   {
      float nearest_neighbor_dist=(*dists_ptr)[f][0];
      float next_to_nearest_neighbor_dist=(*dists_ptr)[f][1];
      if (nearest_neighbor_dist/next_to_nearest_neighbor_dist > 
      max_distance_ratio) continue;

      int nearest_neighbor_index=(*indices_ptr)[f][0];
      pair<int,int> P(f,nearest_neighbor_index);
      initial_SIFT_matches.push_back(P);
   }
//   cout << "At end of find_only_backward_SIFT_matches()" << endl;
}

// ---------------------------------------------------------------------
// Member function find_bijective_SIFT_matches() first finds
// one-directional candidate matches between the features within
// *SIFT_descriptors_matrix_ptr and *SIFT_descriptors2_matrix_ptr.  If
// a match is one-to-one and onto, the IDs for the tiepoint pair are
// added to STL vector bijective_SIFT_matches.  

vector<pair<int,int> > 
   akm::find_bijective_SIFT_matches(double max_distance_ratio)
{
//   cout << "inside akm::find_bijective_SIFT_matches()" << endl;
   
   find_forward_SIFT_matches(max_distance_ratio);
   find_backward_SIFT_matches(max_distance_ratio);
   
   vector<pair<int,int> > bijective_SIFT_matches;
   for (FEATURE_MATCHES_MAP::iterator itr=forward_matches_map_ptr->begin();
        itr != forward_matches_map_ptr->end(); ++itr)
   {
      int sift1_index=itr->first;
      int sift2_index=itr->second;

      FEATURE_MATCHES_MAP::iterator itr2=backward_matches_map_ptr->find(
         sift2_index);
      if (itr2==backward_matches_map_ptr->end()) continue;
//      int sift2b_index=itr2->first;
      int sift1b_index=itr2->second;
      
      if (sift1_index==sift1b_index)
      {
         pair<int,int> P(sift1_index,sift2_index);
         bijective_SIFT_matches.push_back(P);
      }
   } // loop over iterator 
//   cout << "bijective_SIFT_matches.size() = " << bijective_SIFT_matches.size()
//        << endl;
   return bijective_SIFT_matches;
}

// ---------------------------------------------------------------------
// Member function find_forward_SIFT_matches() uses the FLANN library
// to (approximately) find the closest two SIFT features within
// *SIFT_descriptors2_matrix_ptr for each SIFT feature within
// *SIFT_descriptors_matrix_ptr.  If the two closest features satisfy
// Lowe's ratio test, the ID for the nearest neighbor in
// *SIFT_descriptors2_matrix_ptr is add to *forward_matches_map_ptr.

void akm::find_forward_SIFT_matches(double max_distance_ratio)
{
//   cout << "inside akm::find_forward_SIFT_matches()" << endl;
//   outputfunc::print_elapsed_time();
//   cout << "SIFT_descriptors_matrix_ptr->rows = "
//        << SIFT_descriptors_matrix_ptr->rows << endl;
  
   const int nn=2;
   unsigned int n_queries=SIFT_descriptors_matrix_ptr->rows;
   unsigned int curr_n_queries_times_nn=n_queries*nn;
//   cout << "n_queries = " << n_queries << endl;

   if (int_array_ptr==NULL || curr_n_queries_times_nn > n_queries_times_nn)
   {
      n_queries_times_nn=curr_n_queries_times_nn;

      delete [] int_array_ptr;
      delete [] float_array_ptr;
      delete indices_ptr;
      delete dists_ptr;

      int_array_ptr=new int[curr_n_queries_times_nn];
      float_array_ptr=new float[curr_n_queries_times_nn];
      indices_ptr=new flann::Matrix<int>(int_array_ptr,n_queries,nn);
      dists_ptr=new flann::Matrix<float>(float_array_ptr,n_queries,nn);
   }
   
//   cout << "Before call to forward_nn_ptr->knnSearch()" << endl;
   forward_nn_ptr->knnSearch(
      *SIFT_descriptors_matrix_ptr, 
      *indices_ptr,*dists_ptr,nn, flann::SearchParams(128));
//   cout << "After call to forward_nn_ptr->knnSearch()" << endl;

   forward_matches_map_ptr->clear();

   for (unsigned int f=0; f<n_queries; f++)
   {
      float nearest_neighbor_dist=(*dists_ptr)[f][0];
      float next_to_nearest_neighbor_dist=(*dists_ptr)[f][1];
//      cout << "nearest_neighbor_dist = " << nearest_neighbor_dist
//           << " next_to_nearest_neighbor_dist = "
//           << next_to_nearest_neighbor_dist << endl;
//      float ratio=nearest_neighbor_dist/next_to_nearest_neighbor_dist;
//      cout << "ratio = " << ratio << endl;
      if (nearest_neighbor_dist/next_to_nearest_neighbor_dist > 
      max_distance_ratio) continue;

      int nearest_neighbor_index=(*indices_ptr)[f][0];
      (*forward_matches_map_ptr)[f]=nearest_neighbor_index;
   }

//   cout << "At end of find_forward(), forward_matches_map_ptr->size() = "
//        << forward_matches_map_ptr->size() << endl;
}

// ---------------------------------------------------------------------
// Member function find_backward_SIFT_matches() uses the FLANN library
// to (approximately) find the closest two SIFT features within
// *SIFT_descriptors_matrix_ptr for each SIFT feature within
// *SIFT_descriptors2_matrix_ptr.  If the two closest features satisfy
// Lowe's ratio test, the ID for the nearest neighbor in
// *SIFT_descriptors_matrix_ptr is add to *backward_matches_map_ptr.

void akm::find_backward_SIFT_matches(double max_distance_ratio)
{
//   cout << "inside akm::find_backward_SIFT_matches()" << endl;
//   outputfunc::print_elapsed_time();
//   cout << "SIFT_descriptors2_matrix_ptr->rows = "
//        << SIFT_descriptors2_matrix_ptr->rows << endl;
   
   const int nn=2;
   unsigned int n_queries=SIFT_descriptors2_matrix_ptr->rows;
   unsigned int curr_n_queries_times_nn=n_queries*nn;

   if (int_array_ptr==NULL || curr_n_queries_times_nn > n_queries_times_nn)
   {
      n_queries_times_nn=curr_n_queries_times_nn;

      delete [] int_array_ptr;
      delete [] float_array_ptr;
      delete indices_ptr;
      delete dists_ptr;

      int_array_ptr=new int[curr_n_queries_times_nn];
      float_array_ptr=new float[curr_n_queries_times_nn];
      indices_ptr=new flann::Matrix<int>(int_array_ptr,n_queries,nn);
      dists_ptr=new flann::Matrix<float>(float_array_ptr,n_queries,nn);
   }

   backward_nn_ptr->knnSearch(
      *SIFT_descriptors2_matrix_ptr, 
      *indices_ptr,*dists_ptr,nn, flann::SearchParams(128));

   backward_matches_map_ptr->clear();
   for (unsigned int f=0; f<n_queries; f++)
   {
//      cout << "f = " << f << endl;
      float nearest_neighbor_dist=(*dists_ptr)[f][0];
      float next_to_nearest_neighbor_dist=(*dists_ptr)[f][1];
//      cout << "nearest_neighbor_dist = " << nearest_neighbor_dist
//           << " next_to_nearest_neighbor_dist = "
//           << next_to_nearest_neighbor_dist << endl;
      if (nearest_neighbor_dist/next_to_nearest_neighbor_dist > 
      max_distance_ratio) continue;

      int nearest_neighbor_index=(*indices_ptr)[f][0];
      (*backward_matches_map_ptr)[f]=nearest_neighbor_index;
   }

//   cout << "At end of find_backward(), backward_matches_map_ptr->size() = "
//        << backward_matches_map_ptr->size() << endl;
}

// =========================================================================
// Term frequency-inverse document frequency member functions
// =========================================================================

void akm::clear_document_word_frequencies(int K)
{
//   cout << "inside akm::clear_document_word_frequencies()" << endl;
//   cout << " K = " << K << endl;
   this->K = K;

   delete [] term_frequency;
   term_frequency=new float[K];
   memset(term_frequency,0,K*sizeof(float));


   delete [] image_word_count;
   image_word_count=new int[K];

   delete [] multi_image_word_occurrence;
   multi_image_word_occurrence=new int[K];
   memset(multi_image_word_occurrence,0,K*sizeof(int));

   if (inverse_document_frequency_matrix_ptr != NULL)
   {
      delete [] inverse_document_frequency_matrix_ptr->ptr();
      delete inverse_document_frequency_matrix_ptr;
   }
   inverse_document_frequency_matrix_ptr=
      new flann::Matrix<float>(new float[K],1,K);
   memset(inverse_document_frequency_matrix_ptr->ptr(),0,K*sizeof(float));

   delete [] tfidf;
   tfidf=new float[K];
   memset(tfidf,0,K*sizeof(float));
}

// ---------------------------------------------------------------------
// Member function clear_document_word_frequencies()
// deletes and reinstantiates arrays and FLANN matrices associated
// with TF-IDF computations.  

void akm::clear_document_word_frequencies(int n_images,int K)
{
   cout << "inside akm::clear_document_word_frequencies()" << endl;
   cout << "n_images = " << n_images << " K = " << K << endl;

   delete [] multi_image_word_occurrence;
   multi_image_word_occurrence=new int[K];
   memset(multi_image_word_occurrence,0,K*sizeof(int));

   delete [] term_frequency;
   term_frequency=new float[K];
   memset(term_frequency,0,K*sizeof(float));

   if (inverse_document_frequency_matrix_ptr != NULL)
   {
      delete [] inverse_document_frequency_matrix_ptr->ptr();
      delete inverse_document_frequency_matrix_ptr;
   }
   inverse_document_frequency_matrix_ptr=
      new flann::Matrix<float>(new float[K],1,K);
   memset(inverse_document_frequency_matrix_ptr->ptr(),0,K*sizeof(float));

   delete [] tfidf;
   tfidf=new float[K];
   memset(tfidf,0,K*sizeof(float));

   long long n_ll=n_images;
   long long K_ll=K;
   long long n_floats=n_ll*K_ll;
   long long n_bytes=n_floats*sizeof(float);
   cout << "n_floats = long long (n_images * K) = " << n_floats << endl;
   cout << "n_bytes = " << n_bytes << endl;

   initialize_tfidf_sparse_matrix(n_images,K);
}

// ---------------------------------------------------------------------
// Member function initialize_tfidf_sparse_matrix()

void akm::initialize_tfidf_sparse_matrix(int n_images,int K)
{
   cout << "inside akm::initialize_tfidf_sparse_matrix()" << endl;
   cout << "n_images = " << n_images << " K = " << K << endl;

//   long long n_ll=n_images;
//   long long K_ll=K;
//   long long n_floats=n_ll*K_ll;
//   long long n_bytes=n_floats*sizeof(float);
//   cout << "n_floats = long long (n_images * K) = " << n_floats << endl;
//   cout << "n_bytes = " << n_bytes << endl;

   tfidf_sparse_matrix_ptr=
      new gmm::row_matrix< gmm::wsvector<float> >(n_images+1,K);
   gmm::clear(*tfidf_sparse_matrix_ptr);
}

// ---------------------------------------------------------------------
// Member function compute_image_words() calculates the number of
// occurrences of each vocabulary word (n_id) within the image
// specified by input parameter curr_image_ID.  It returns the number
// of distinct words contained within the input image.

int akm::compute_image_words(
   int curr_image_ID,string image_basename,string image_words_subdir)
{
//   cout << "inside akm::compute_image_word_count()" << endl;

// Construct a randomized kd-tree index using 8 kd-trees

   if (cluster_index_nn_ptr==NULL)
   {
      cluster_index_nn_ptr=new flann::Index<flann::L2<float> >
         (*cluster_centers_matrix_ptr, flann::KDTreeIndexParams(8));
//      cout << "Building cluster index" << endl;
      cluster_index_nn_ptr->buildIndex();                
//      cout << "Finished building cluster index" << endl;
   }
   
// Find (approximate) closest cluster centers for all SIFT descriptors
// for current image:

//   cout << "before knnSearch" << endl;
   cluster_index_nn_ptr->knnSearch(
      *SIFT_descriptors_matrix_ptr, *indices_matrix_ptr, *dists_matrix_ptr,
      1, flann::SearchParams(128));
//   cout << "after knnSearch" << endl;

// Clear image_word_count array's contents:

   memset(image_word_count,0,K*sizeof(int));

   for (unsigned int n=0; n<N; n++)
   {
      int curr_cluster_center_ID=(*indices_matrix_ptr)[n][0];
//      cout << "n = " << n 
//           << " curr_cluster_center_ID = " << curr_cluster_center_ID
//           << endl;
      image_word_count[curr_cluster_center_ID] += 1;
   } // loop over index n labeling SIFT descriptors for current image

   int n_distinct_words=0;
   for (unsigned int k=0; k<K; k++)
   {
      if (image_word_count[k] > 0) n_distinct_words++;
   }
   double word_frac=double(n_distinct_words)/K;

// Export text file containing non-zero word counts for current image:

   string image_word_filename=image_words_subdir+image_basename+".words";
   ofstream outstream;
   filefunc::openfile(image_word_filename,outstream);
   outstream << "# Curr image ID = " << curr_image_ID << endl;
   outstream << "# n_distinct_words = " << n_distinct_words << endl;
   outstream << "# word fraction in current image = " << word_frac << endl;
   outstream << "# Word_ID   word_count" << endl << endl;

   for (unsigned int c=0; c<K; c++)
   {
      int curr_image_word_count=image_word_count[c];
      if (curr_image_word_count==0) continue;
      outstream << c << "  " 
                << curr_image_word_count << endl;
   }

   filefunc::closefile(image_word_filename,outstream);
   return n_distinct_words;
}

// ---------------------------------------------------------------------
// Member function compute_term_frequencies() calculates the
// number of occurrences of each vocabulary word (n_id) within one
// particular image.  It renormalizes individual word frequency by
// the total number of words within the current image n_d=sum_i n_id.
// This method exports non-zero word term frequencies for each image
// to output text files.

void akm::compute_term_frequencies(
   int curr_image_ID,string image_words_filename,string term_freqs_subdir)
{
//   cout << "inside akm::compute_term_frequencies()" << endl;
//   cout << "curr_image_ID = " << curr_image_ID << endl;
//   cout << "image_words_filename = " << image_words_filename << endl;
//   cout << "K = " << K << endl;

// Clear image_word_count array's contents:

   memset(image_word_count,0,K*sizeof(int));
   int image_word_integral=0;

   filefunc::ReadInfile(image_words_filename);
//   cout << "filefunc::text_line.size() = "
//        << filefunc::text_line.size() << endl;
   
// Recall zeroth line within image_word_filename contains image's basename:

   string image_basename=filefunc::text_line[0];
//   cout << "image_basename = " << image_basename << endl;

   for (unsigned int l=1; l<filefunc::text_line.size(); l++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[l]);
      int c=column_values[0];
      image_word_count[c]=column_values[1];
      image_word_integral += image_word_count[c];
   } // loop over index l labeling lines in text_line
//   cout << "image word integral = " << image_word_integral << endl;

   for (unsigned int c=0; c<K; c++)
   {
      int curr_image_word_count=image_word_count[c];
      if (curr_image_word_count==0) continue;
      
      term_frequency[c]=
         float(curr_image_word_count)/image_word_integral;
      multi_image_word_occurrence[c] += 1;
   } // loop over index c labeling vocabulary words

// Export text file containing non-zero term frequencies for current image:

   string term_freqs_filename=term_freqs_subdir+
      "term_freqs_"+stringfunc::integer_to_string(curr_image_ID,5)+".dat";
   ofstream outstream;
   outstream.precision(12);
   filefunc::openfile(term_freqs_filename,outstream);
   outstream << image_basename << endl;

   for (unsigned int c=0; c<K; c++)
   {
      int curr_image_word_count=image_word_count[c];
      if (curr_image_word_count==0) continue;
      outstream << c << "  " 
                << term_frequency[c] << endl;
   }

   filefunc::closefile(term_freqs_filename,outstream);
}

/*
// ---------------------------------------------------------------------
// Member function compute_term_frequencies() calculates the
// number of occurrences of each vocabulary word (n_id) within one
// particular image.  It renormalizes individual word frequency by
// the total number of words within the current image n_d=sum_i n_id.
// Word term frequencies within the document labeled by input
// curr_image_ID are temporarily stored into *tfidf_matrix_ptr.

void akm::compute_term_frequencies(int curr_image_ID)
{
//   cout << "inside akm::compute_term_frequencies()" << endl;

// Construct a randomized kd-tree index using 8 kd-trees

   if (cluster_index_nn_ptr==NULL)
   {
      cluster_index_nn_ptr=new flann::Index<flann::L2<float> >
         (*cluster_centers_matrix_ptr, flann::KDTreeIndexParams(8));
      cout << "Building cluster index" << endl;
      cluster_index_nn_ptr->buildIndex();                
      cout << "Finished building cluster index" << endl;
   }
   
// Find (approximate) closest cluster centers for all SIFT descriptors
// for current image:

   cluster_index_nn_ptr->knnSearch(
      *SIFT_descriptors_matrix_ptr, *indices_matrix_ptr, *dists_matrix_ptr,
      1, flann::SearchParams(128));

// Clear image_word_count array's contents:

   memset(image_word_count,0,K*sizeof(int));

   for (unsigned int n=0; n<N; n++)
   {
      int curr_cluster_center_ID=(*indices_matrix_ptr)[n][0];
//      cout << "n = " << n 
//           << " curr_cluster_center_ID = " << curr_cluster_center_ID
//           << endl;
      image_word_count[curr_cluster_center_ID] += 1;
   } // loop over index n labeling SIFT descriptors for current image

   int image_word_integral=0;
   for (unsigned int c=0; c<K; c++)
   {
//      cout << "Cluster index c = " << c << " image word count = "
//           << image_word_count[c] << endl;
      image_word_integral += image_word_count[c];
   } // loop over index c labeling vocabulary words
//   cout << "image word integral = " << image_word_integral << endl;

   for (unsigned int c=0; c<K; c++)
   {
      int curr_image_word_count=image_word_count[c];
      if (curr_image_word_count==0) continue;
      
      float curr_term_frequency=
         float(curr_image_word_count)/image_word_integral;

      multi_image_word_occurrence[c] += 1;

      (*tfidf_sparse_matrix_ptr)(curr_image_ID,c)=curr_term_frequency;
   } // loop over index c labeling vocabulary words
}
*/

// ---------------------------------------------------------------------
// Member function compute_inverse_document_frequencies() takes in the
// number of documents (=images) in which a vocabulary word appears
// within integer array multi_image_word_occurrence[].  It forms the
// ratio of the total number of documents to the number of documents
// containing a vocabulary word.  The inverse document frequency is
// set equal to the log of this ratio.

void akm::compute_inverse_document_frequencies(
   int n_images,string sift_keys_subdir)
{
   cout << "inside akm::compute_inverse_document_frequencies()" << endl;
   cout << "n_images = " << n_images << endl;
   cout << "K = " << K << endl;

   for (unsigned int c=0; c<K; c++)
   {
      int denom=basic_math::max(1,multi_image_word_occurrence[c]);
      (*inverse_document_frequency_matrix_ptr)[0][c] =
         log(double(n_images)/denom);
   } // loop over index c labeling vocabulary words

// As of April 2012, we have found that FLANN throws an exception if
// output_filename already exists.  So we need to first delete
// output_filename before attempting to export it:

   string output_filename=sift_keys_subdir+"inverse_doc_freqs.hdf5";
   filefunc::deletefile(output_filename);
   flann::save_to_file(
      *inverse_document_frequency_matrix_ptr,output_filename.c_str(),
      "inverse_doc_freqs");
//   cout << "Export complete" << endl;
}

// ---------------------------------------------------------------------
// Member function import_inverse_doc_freq_matrix() reads in previously saved
// inverse document frequencies from a binary HDF5 file.

void akm::import_inverse_doc_freq_matrix(string sift_keys_subdir)
{
   cout << "inside akm::import_inverse_doc_freq_matrix()" << endl;

   string inverse_doc_freqs_filename=sift_keys_subdir+"inverse_doc_freqs.hdf5";
//   cout << "inverse_doc_freqs_filename = " << inverse_doc_freqs_filename << endl;

   flann::load_from_file(
      *inverse_document_frequency_matrix_ptr, 
      inverse_doc_freqs_filename.c_str(),"inverse_doc_freqs");
}

// ---------------------------------------------------------------------
// Member function compute_tfidfs_for_single_image() takes in the ID
// for some particular image.  It computes the K-dimensional vector
// representing the image from products of term frequencies and inverse
// document frequencies.  After normalizing the K-dimensional vector
// so that it has unit norm, this method stores the document's vector
// in the row of *tfidf_matrix_ptr labeled by image_ID.

void akm::compute_tfidfs_for_single_image(int image_ID)
{
   double sqr_mag=0;
   for (unsigned int c=0; c<K; c++)
   {
      (*tfidf_sparse_matrix_ptr)(image_ID,c) =
         (*tfidf_sparse_matrix_ptr)(image_ID,c) *
         (*inverse_document_frequency_matrix_ptr)[0][c];
      sqr_mag += sqr ( (*tfidf_sparse_matrix_ptr)(image_ID,c) ) ;
   } 

// Renormalize tfidf vector so that it has unit magnitude:

   double mag=sqrt(sqr_mag);
   for (unsigned int c=0; c<K; c++)
   {
      (*tfidf_sparse_matrix_ptr)(image_ID,c) /= mag;
   } 
}

// ---------------------------------------------------------------------
// Member function compute_tfidfs() loops over images "1 thru N" and
// computes their K-dimensional "vocab word" vectors.  Upon
// completion, *tfidf_matrix_ptr holds the all the K-dimensional
// vectors within its first n_images rows.

void akm::compute_tfidfs(unsigned int n_images)
{
//   cout << "inside akm::compute_tfidfs()" << endl;
//   cout << "n_images = " << n_images << endl;

   for (unsigned int i=0; i<n_images; i++)
   {
      compute_tfidfs_for_single_image(i);
   } // loop over index i labeling images
}

// ---------------------------------------------------------------------
// Member function compare_Nplusone_image_with_N_images() works with
// member FLANN matrix *tfidf_matrix_ptr which is assumed to have its
// first "N" rows filled with normalized K-dimensional vectors
// representing the first "N" images.  It is also assumed to have its
// last "N+1" row filled with a normalized K-dimensional vector
// representing some "N+1st" image.  This method computes dotproducts
// between vectors "1 thru N" and vector "N+1".  The dotproduct values
// are sorted in descending order.  This method returns an STL vector
// of integers which contain image IDs and dotproducts for
// images "1 through N" which indicate their similarity to image "N+1".

void akm::compare_Nplusone_image_with_N_images(
   vector<int>& image_indices,vector<float>& dotproducts)
{
//   cout << "inside akm::compare_Nplusone_image_with_N_images()" << endl;
   
   unsigned int n_images=gmm::mat_nrows(*tfidf_sparse_matrix_ptr)-1;
   for (unsigned int i=0; i<n_images; i++)
   {
      image_indices.push_back(i);
      float curr_dotproduct=0;
      for (unsigned int c=0; c<K; c++)
      {
         curr_dotproduct += 
            (*tfidf_sparse_matrix_ptr)(i,c) *
            (*tfidf_sparse_matrix_ptr)(n_images,c);
      }
      dotproducts.push_back(curr_dotproduct);
   } // loop over index i labeling images "1 thru N"

   templatefunc::Quicksort_descending(dotproducts,image_indices);
}

// =========================================================================
// Binary HDF5 file import/export member functions
// =========================================================================

// Member function export_SIFT_features_in_HDF5_format() writes all SIFT
// descriptors for a single image which are assumed to reside in
// *D_ptrs_ptr to a binary file in HDF5 format.  It returns the name
// of the export HDF5 binary file.

string akm::export_SIFT_features_in_HDF5_format(
   string sift_keys_filename,string hdf5_subdir,string output_file_prefix,
   const vector<descriptor*>* D_ptrs_ptr)
{
//   cout << "inside akm::export_SIFT_features_in_HDF5_format()" << endl;

   string basename=filefunc::getbasename(sift_keys_filename);
//   cout << "basename = " << basename << endl;

// Check if import SIFT keys filename is gzipped.  If so, ignore .gz
// suffix when determining hdf5 filename:

   string suffix=stringfunc::suffix(basename);
   if (suffix=="gz")
   {
      basename=stringfunc::prefix(basename);
   }
//   cout << "basename = " << basename << endl;

   string output_filename=hdf5_subdir+output_file_prefix+
      stringfunc::prefix(basename)+".hdf5";
//   cout << "output_filename = " << output_filename << endl;

   load_SIFT_descriptors(D_ptrs_ptr);

// As of April 2012, we have found that FLANN throws an exception if
// output_filename already exists.  So we need to first delete
// output_filename before attempting to export it:

   filefunc::deletefile(output_filename);
   flann::save_to_file(
      *SIFT_descriptors_matrix_ptr,output_filename.c_str(),"sift_features");
//   cout << "Export complete" << endl;
   return output_filename;
}

// ---------------------------------------------------------------------
// Member function export_SIFT_features() writes all SIFT descriptors
// within input *D_ptrs_ptr to binary files in HDF5 format.

void akm::export_SIFT_features(
   vector<string>& sift_keys_filenames,string output_file_prefix,
   const vector<int>* image_IDs_ptr,const vector<descriptor*>* D_ptrs_ptr)
{
   cout << "inside akm::export_SIFT_features()" << endl;

   string sift_keys_subdir=filefunc::getdirname(sift_keys_filenames.back());

// First export SIFT descriptors for entire imagery database to single
// binary file:

   string output_filename=sift_keys_subdir+output_file_prefix+
      "all_sift_features.hdf5";
   export_all_SIFT_features(output_filename,"sift_features",D_ptrs_ptr);

// Next export SIFT descriptors corresponding to individual images to
// separate binary files:

   unsigned int n_images=sift_keys_filenames.size();
   for (unsigned int i=0; i<n_images; i++)
   {
      string basename=filefunc::getbasename(sift_keys_filenames[i]);
      output_filename=sift_keys_subdir+output_file_prefix+
         stringfunc::prefix(basename)+".hdf5";

      load_SIFT_descriptors(i,image_IDs_ptr,D_ptrs_ptr);

      filefunc::deletefile(output_filename);
      flann::save_to_file(
         *SIFT_descriptors_matrix_ptr,output_filename.c_str(),"sift_features");
   } // loop over index i labeling images
}

// ---------------------------------------------------------------------
// Member function export_all_SIFT_features() writes all SIFT
// descriptors within input *D_ptrs_ptr to a single binary file in
// HDF5 format.

void akm::export_all_SIFT_features(
   string output_filename,string HDF5_label,
   const vector<descriptor*>* D_ptrs_ptr)
{
//   cout << "inside akm::export_all_SIFT_features()" << endl;
//   cout << "output_filename = " << output_filename << endl;
//   cout << "HDF5_label = " << HDF5_label << endl;

   load_SIFT_descriptors(D_ptrs_ptr);

   filefunc::deletefile(output_filename);
   flann::save_to_file(
      *SIFT_descriptors_matrix_ptr,output_filename.c_str(),HDF5_label.c_str());
}

// ---------------------------------------------------------------------
// Member function export_cluster_centers() writes all cluster center
// descriptors within *cluster_centers_matrix_ptr to a binary file in
// HDF5 format.

int akm::export_cluster_centers(
   int min_features_per_cluster,
   string output_file_prefix,string sift_keys_subdir)
{
   cout << "inside akm::export_cluster_centers()" << endl;

   string output_filename=sift_keys_subdir+output_file_prefix+
      "cluster_centers.hdf5";
   cout << "output_filename = " << output_filename << endl;

   filefunc::deletefile(output_filename);
   flann::save_to_file(
      *cluster_centers_matrix_ptr,output_filename.c_str(),"cluster_centers");

   int K_large_clusters=0;
   for (unsigned int c=0; c<K; c++)
   {
      if (n_features_in_cluster[c] < min_features_per_cluster) continue;
      K_large_clusters++;
   } // loop over index c labeling clusters
   
   flann::Matrix<float>* large_cluster_centers_matrix_ptr=new
      flann::Matrix<float>(new float[K_large_clusters*D],K_large_clusters,D);

   cout << "K_large_clusters = " << K_large_clusters << endl;

   int large_cluster_counter=0;
   for (unsigned int c=0; c<K; c++)
   {
      if (n_features_in_cluster[c] < min_features_per_cluster) continue;
      for (unsigned int d=0; d<D; d++)
      {
         (*large_cluster_centers_matrix_ptr)[large_cluster_counter][d]=
            (*cluster_centers_matrix_ptr)[c][d];
      }
      large_cluster_counter++;
   } // loop over index c labeling clusters
   cout << "large_cluster_counter = " << large_cluster_counter << endl;

   cout << "large_cluster_centers_matrix_ptr->rows = "
        << large_cluster_centers_matrix_ptr->rows << endl;
   cout << "large_cluster_centers_matrix_ptr->cols = "
        << large_cluster_centers_matrix_ptr->cols << endl;
   cout << "large_cluster_centers_matrix_ptr->stride = "
        << large_cluster_centers_matrix_ptr->stride << endl;

   output_filename=sift_keys_subdir+output_file_prefix+
      "large_cluster_centers.hdf5";
   cout << "output_filename = " << output_filename << endl;
   
   filefunc::deletefile(output_filename);
   flann::save_to_file(
      *large_cluster_centers_matrix_ptr,output_filename.c_str(),
      "cluster_centers");

   string banner="Exported cluster centers to "+output_filename;
   outputfunc::write_big_banner(banner);

   delete [] cluster_centers_matrix_ptr->ptr();
   delete cluster_centers_matrix_ptr;
   cluster_centers_matrix_ptr=large_cluster_centers_matrix_ptr;

   return K_large_clusters;
}

// ---------------------------------------------------------------------
// Member function export_vocabulary() writes out inverse document
// frequencies as well as the contents of *tfidf_matrix_ptr to output
// binary files in HDF5 format.

void akm::export_vocabulary(
   string output_file_prefix,string sift_keys_subdir,
   string& inverse_doc_freq_filename,string& vocab_filename)
{
   cout << "inside akm::export_vocabulary()" << endl;

   inverse_doc_freq_filename=sift_keys_subdir+output_file_prefix+
      "inverse_doc_freq.hdf5";
   filefunc::deletefile(inverse_doc_freq_filename);
   flann::save_to_file(
      *inverse_document_frequency_matrix_ptr,
      inverse_doc_freq_filename.c_str(),"inverse_doc_freq");

   vocab_filename=sift_keys_subdir+output_file_prefix+
      "vocabulary.sparse_matrix";
   filefunc::deletefile(vocab_filename);

   gmm::csc_matrix<double>* M_ptr=new gmm::csc_matrix<double>;
   gmm::copy(*tfidf_sparse_matrix_ptr,*M_ptr);
   gmm::MatrixMarket_save(vocab_filename.c_str(),*M_ptr);
   delete M_ptr;
}

// ---------------------------------------------------------------------
// Member function export_partial_tfidf_and_multi_image_words()

void akm::export_partial_tfidf_and_multi_image_words(
   int n_processed_images,string sift_keys_subdir)
{
   cout << "inside akm::export_partial_vocabulary()" << endl;

   string tfidf_subdir=sift_keys_subdir+"tfidf/";
   filefunc::dircreate(tfidf_subdir);
   string tfidf_filename=tfidf_subdir+"tfidf_"+
      stringfunc::number_to_string(n_processed_images)+".sparse_matrix";

   gmm::csc_matrix<double>* M_ptr=new gmm::csc_matrix<double>;
   gmm::copy(*tfidf_sparse_matrix_ptr,*M_ptr);
   gmm::MatrixMarket_save(tfidf_filename.c_str(),*M_ptr);
   delete M_ptr;

   string banner="Exported "+tfidf_filename;
   outputfunc::write_banner(banner);
   
   string multi_image_word_filename=tfidf_subdir+
      "multi_image_word_occurrence_"+
      stringfunc::number_to_string(n_processed_images)+".dat";

   ofstream outstream;
   filefunc::openfile(multi_image_word_filename,outstream);
   outstream << "# K = " << K << endl << endl;
   for (unsigned int c=0; c<K; c++)
   {
      if (multi_image_word_occurrence[c]==0) continue;
      outstream << c << "  " << multi_image_word_occurrence[c] << endl;
   }
   filefunc::closefile(multi_image_word_filename,outstream);

   banner="Exported "+multi_image_word_filename;
   outputfunc::write_banner(banner);
}

// ---------------------------------------------------------------------
// Member function import_vocabulary() reads in previously saved
// inverse document frequencies and tfidf values from binary files in
// HDF5 format.

void akm::import_vocabulary(string output_file_prefix,string sift_keys_subdir)
{
   cout << "inside akm::import_vocabulary()" << endl;

   string inverse_doc_freq_filename=sift_keys_subdir+output_file_prefix+
      "inverse_doc_freq.hdf5";
//   cout << "inverse_doc_freq_filename = " << inverse_doc_freq_filename << endl;
   flann::load_from_file(
      *inverse_document_frequency_matrix_ptr, 
      inverse_doc_freq_filename.c_str(),"inverse_doc_freq");

   string vocab_filename=sift_keys_subdir+output_file_prefix+
      "vocabulary.sparse_matrix";
//   cout << "vocab_filename = " << vocab_filename << endl;
   
   gmm::MatrixMarket_load(
      vocab_filename.c_str(),*tfidf_sparse_matrix_ptr);
}

// ---------------------------------------------------------------------
// Member function whiten_SIFT_descriptors()

void akm::whiten_SIFT_descriptors(
   const flann::Matrix<float>& inverse_sqrt_covar)
{
//    cout << "inside akm::whiten_SIFT_descriptors()" << endl;
   string banner="Whitening SIFT descriptors";
   outputfunc::write_banner(banner);
   cout << "n_features = " << N << endl;

   float* whitened_row=new float[D];
   for (unsigned int f=0; f<N; f++)
   {
      if (f%1000000==0) cout << endl;
      if (f%100000==0) cout << f << " " << flush;

      for (unsigned int i=0; i<D; i++)
      {
         whitened_row[i]=0;
         for (unsigned int j=0; j<D; j++)
         {
            whitened_row[i] += inverse_sqrt_covar[i][j] * 
               (*SIFT_descriptors_matrix_ptr)[f][j];
         } // loop over index j
      } // loop over index i
      
      for (unsigned int d=0; d<D; d++)
      {
         (*SIFT_descriptors_matrix_ptr)[f][d]=whitened_row[d];
      } // loop over index d
   } // loop over index f labeling SIFT features

   delete [] whitened_row;
   cout << endl;
}

// ---------------------------------------------------------------------
// Member function compute_tfidfs()

void akm::compute_tfidfs(
   int curr_image_ID,string term_freqs_filename,string tfidfs_subdir)
{
//   cout << "inside akm::compute_tfidfs()" << endl;
//   cout << "K = " << K << endl;
//   cout << "term_freqs_filename = " << term_freqs_filename << endl;

// Clear term_frequency array's contents:

   memset(term_frequency,0,K*sizeof(float));

   filefunc::ReadInfile(term_freqs_filename);
//   cout << "filefunc::text_line.size() = "
//        << filefunc::text_line.size() << endl;

// Recall zeroth line within term_freqs_filename contains image's basename:

   string image_basename=filefunc::text_line[0];

   for (unsigned int l=1; l<filefunc::text_line.size(); l++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[l]);
      int c=column_values[0];
      term_frequency[c]=column_values[1];
   } // loop over index l labeling lines in text_line

   memset(tfidf,0,K*sizeof(float));

   double sqr_mag=0;
   for (unsigned int c=0; c<K; c++)
   {
      double curr_term_freq=term_frequency[c];
      if (nearly_equal(curr_term_freq,0,1E-10)) continue;
      
      tfidf[c]=curr_term_freq*
         (*inverse_document_frequency_matrix_ptr)[0][c];
      sqr_mag += sqr ( tfidf[c] ) ;
   } // loop over index c labeling vocabulary words

// Renormalize tfidf vector so that it has unit magnitude:

   double mag=sqrt(sqr_mag);
   for (unsigned int c=0; c<K; c++)
   {
      tfidf[c]=tfidf[c]/mag;
   } 

// Export text file containing non-zero TFIDFs for current image:

   string tfidfs_filename=tfidfs_subdir+
      "tfidfs_"+stringfunc::integer_to_string(curr_image_ID,5)+".dat";
   ofstream outstream;
   outstream.precision(12);
   filefunc::openfile(tfidfs_filename,outstream);
   outstream << image_basename << endl;

   for (unsigned int c=0; c<K; c++)
   {
      double curr_tfidf=tfidf[c];
      if (nearly_equal(curr_tfidf,0,1E-10)) continue;
      outstream << c << "  " << curr_tfidf << endl;
   }

   filefunc::closefile(tfidfs_filename,outstream);
}


// ---------------------------------------------------------------------
// Member function import_tfidfs()

void akm::import_tfidfs(int curr_image_ID,string tfidfs_filename)
{
//   cout << "inside akm::import_tfidfs()" << endl;
//   cout << "curr_image_ID = " << curr_image_ID << endl;
//   cout << "K = " << K << endl;
//   cout << "tfidfs_filename = " << tfidfs_filename << endl;

   filefunc::ReadInfile(tfidfs_filename);
//   cout << "filefunc::text_line.size() = "
//        << filefunc::text_line.size() << endl;

// Recall zeroth line within term_frequs_filename contains image's basename:

//   string image_basename=filefunc::text_line[0];

   for (unsigned int l=1; l<filefunc::text_line.size(); l++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[l]);
      int c=column_values[0];
      float curr_tfidf=column_values[1];
      (*tfidf_sparse_matrix_ptr)(curr_image_ID,c)=curr_tfidf;
   } // loop over index l labeling lines in text_line

}

// ---------------------------------------------------------------------
// Member function export_tfidf_sparse_matrix() writes out 
// the contents of *tfidf_sparse_matrix_ptr to output file.

string akm::export_tfidf_sparse_matrix(string sift_keys_subdir)
{
   cout << "inside akm::export_tfidf_sparse_matrix()" << endl;

   string tfidf_sparse_matrix_filename=sift_keys_subdir+
      "tfidf_sparse_matrix.dat";
   filefunc::deletefile(tfidf_sparse_matrix_filename);

   gmm::csc_matrix<double>* M_ptr=new gmm::csc_matrix<double>;
   gmm::copy(*tfidf_sparse_matrix_ptr,*M_ptr);
   gmm::MatrixMarket_save(tfidf_sparse_matrix_filename.c_str(),*M_ptr);
   delete M_ptr;

   return tfidf_sparse_matrix_filename;
}

// ---------------------------------------------------------------------
// Member function compute_term_frequencies() calculates the
// number of occurrences of each vocabulary word (n_id) within one
// particular image.  It renormalizes individual word frequency by
// the total number of words within the current image n_d=sum_i n_id.
// Word term frequencies within the document labeled by input
// curr_image_ID are temporarily stored into *tfidf_matrix_ptr.

void akm::compute_term_frequencies(int curr_image_ID)
{
//   cout << "inside akm::compute_term_frequencies()" << endl;

// Construct a randomized kd-tree index using 8 kd-trees

   if (cluster_index_nn_ptr==NULL)
   {
      cluster_index_nn_ptr=new flann::Index<flann::L2<float> >
         (*cluster_centers_matrix_ptr, flann::KDTreeIndexParams(8));
      cout << "Building cluster index" << endl;
      cluster_index_nn_ptr->buildIndex();                
      cout << "Finished building cluster index" << endl;
   }
   
// Find (approximate) closest cluster centers for all SIFT descriptors
// for current image:

   cluster_index_nn_ptr->knnSearch(
      *SIFT_descriptors_matrix_ptr, *indices_matrix_ptr, *dists_matrix_ptr,
      1, flann::SearchParams(128));

// Clear image_word_count array's contents:

   memset(image_word_count,0,K*sizeof(int));

   for (unsigned int n=0; n<N; n++)
   {
      int curr_cluster_center_ID=(*indices_matrix_ptr)[n][0];
//      cout << "n = " << n 
//           << " curr_cluster_center_ID = " << curr_cluster_center_ID
//           << endl;
      image_word_count[curr_cluster_center_ID] += 1;
   } // loop over index n labeling SIFT descriptors for current image

   int image_word_integral=0;
   for (unsigned int c=0; c<K; c++)
   {
//      cout << "Cluster index c = " << c << " image word count = "
//           << image_word_count[c] << endl;
      image_word_integral += image_word_count[c];
   } // loop over index c labeling vocabulary words
//   cout << "image word integral = " << image_word_integral << endl;

   for (unsigned int c=0; c<K; c++)
   {
      int curr_image_word_count=image_word_count[c];
      if (curr_image_word_count==0) continue;
      
      float curr_term_frequency=
         float(curr_image_word_count)/image_word_integral;
      term_frequency[c]=curr_term_frequency;
   } // loop over index c labeling vocabulary words
}


// ---------------------------------------------------------------------
// Member function compute_tfidfs_for_nplusone_image()

void akm::compute_tfidfs_for_nplusone_image(
   int curr_image_ID,string nplusone_subdir)
{
   cout << "inside akm::compute_tfidfs_for_nplusone_image()" << endl;
//   cout << "K = " << K << endl;

   memset(tfidf,0,K*sizeof(float));

   double sqr_mag=0;
   for (unsigned int c=0; c<K; c++)
   {
      double curr_term_freq=term_frequency[c];
      if (nearly_equal(curr_term_freq,0,1E-10)) continue;
      
      tfidf[c]=curr_term_freq*
         (*inverse_document_frequency_matrix_ptr)[0][c];
      sqr_mag += sqr ( tfidf[c] ) ;
   } // loop over index c labeling vocabulary words

// Renormalize tfidf vector so that it has unit magnitude:

   double mag=sqrt(sqr_mag);
   for (unsigned int c=0; c<K; c++)
   {
      tfidf[c]=tfidf[c]/mag;
   } 

// Export text file containing non-zero TFIDFs for current image:

   string tfidfs_filename=nplusone_subdir+
      "new_image_tfidfs_"+stringfunc::integer_to_string(curr_image_ID,5)
      +".dat";
   ofstream outstream;
   outstream.precision(12);
   filefunc::openfile(tfidfs_filename,outstream);

   for (unsigned int c=0; c<K; c++)
   {
      double curr_tfidf=tfidf[c];
      if (nearly_equal(curr_tfidf,0,1E-10)) continue;
      outstream << c << "  " << curr_tfidf << endl;
   }

   filefunc::closefile(tfidfs_filename,outstream);
}

// ---------------------------------------------------------------------
// Member function compare_Nplusone_image_with_archive_image()

double akm::compare_Nplusone_image_with_archive_image(
   string archive_tfidfs_filename,string& image_basename)
{
//   cout << "inside akm::compare_Nplusone_image_with_archive_image()" << endl;
//   cout << "curr_image_ID = " << curr_image_ID << endl;
//   cout << "K = " << K << endl;
//   cout << "tfidfs_filename = " << tfidfs_filename << endl;

   filefunc::ReadInfile(archive_tfidfs_filename);
//   cout << "filefunc::text_line.size() = "
//        << filefunc::text_line.size() << endl;

// Recall zeroth line within tfidfs_filename contains image's basename:

   image_basename=filefunc::text_line[0];

   double dotproduct=0;
   for (unsigned int l=1; l<filefunc::text_line.size(); l++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[l]);
      int c=column_values[0];
      float curr_archive_tfidf=column_values[1];
      dotproduct += curr_archive_tfidf*tfidf[c];
   } // loop over index l labeling lines in text_line
   return dotproduct;
}


