// ==========================================================================
// Header file for ann_analyzer class
// ==========================================================================
// Last modified on 12/12/08; 1/4/09; 1/15/09; 1/17/09; 5/31/13
// ==========================================================================

#ifndef ANN_ANALYZER_H
#define ANN_ANALYZER_H

#include <iostream>
#include <set>
#include <vector>
#include <ANN/ANN.h>		  
#include "datastructures/descriptor.h"

class ann_analyzer
{

  public:

   typedef std::pair<descriptor*,descriptor*> feature_pair;

   ann_analyzer(int dim);
   ann_analyzer(int dim,int n_nearest_neighbors,int maxPts,double eps);
   ann_analyzer(const ann_analyzer& a);
   ~ann_analyzer();
   ann_analyzer& operator= (const ann_analyzer& a);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const ann_analyzer& a);

// Set and get member functions:

   void set_number_nearest_neighbors(int k);
   void set_epsilon(double eps);
   void set_max_data_points(int maxPts);
   ANNidxArray get_nnIdx();
   ANNdistArray get_sqrd_dists();

// Kdtree initialization member functions:

   bool readPt(std::istream &in, ANNpoint p);
   void printPt(std::ostream &out, ANNpoint p);
   void getArgs(int argc, char **argv);	 

   int load_data_points(const std::vector<feature_pair>& data);

// Kdtree search member functions:

   void find_nearest_neighbors();
   void find_nearest_neighbors(descriptor* D_ptr,bool print_flag=false);
   feature_pair match_feature_descriptor(
      descriptor* input_D_ptr,
      const std::vector<feature_pair>& currimage_feature_info,
      double sqrd_max_ratio);
   bool match_feature_descriptor(
      descriptor* input_D_ptr,
      const std::vector<feature_pair>& currimage_feature_info,
      double sqrd_max_ratio,feature_pair& matching_feature_pair);

  private: 

   int k,dim,maxPts,nPts;
   double eps;
   std::istream *dataIn,*queryIn;

   ANNpointArray dataPts;
   ANNpoint queryPt;
   ANNidxArray nnIdx;
   ANNdistArray sqrd_dists;
   ANNkd_tree* kdtree_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const ann_analyzer& p);

   void reset_kdtree();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void ann_analyzer::set_number_nearest_neighbors(int k)
{
   this->k=k;
}

inline void ann_analyzer::set_epsilon(double eps)
{
   this->eps=eps;
}

inline void ann_analyzer::set_max_data_points(int maxPts)
{
   this->maxPts=maxPts;
}

inline ANNidxArray ann_analyzer::get_nnIdx()
{
   return nnIdx;
}

inline ANNdistArray ann_analyzer::get_sqrd_dists()
{
   return sqrd_dists;
}

#endif  // ann_analyzer.h
