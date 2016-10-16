// ==========================================================================
// Header file for neural_net class 
// ==========================================================================
// Last modified on 2/8/16; 2/9/16
// ==========================================================================

#ifndef NEURAL_NET_H
#define NEURAL_NET_H

#include <set>
#include <string>
#include <vector>

class genmatrix;
class genvector;

class neural_net
{
   
  public:

   typedef std::pair<genvector*, int> DATA_PAIR;

// First genvector holds input training/testing data vector
// Second int holds class ID corresponding to input data vector

// Recall isomorphism between integer class IDs and unit vectors
// e.g. class 0  <--->  [1, 0, 0, ..., 0]
//      class 1  <--->  [0, 1, 0, ..., 0]

// Initialization, constructor and destructor functions:

   neural_net(const std::vector<int>& sizes);
   neural_net(const neural_net& NN);
   ~neural_net();
//   neural_net operator= (const neural_net& NN);
   friend std::ostream& operator<< 
      (std::ostream& outstream,const neural_net& NN);

// Set and get member functions

   int get_layer_size(int l) const;
   genvector* get_biases(int l) const;
   genmatrix* get_weights(int l) const;

   void import_training_data(const std::vector<DATA_PAIR>& data);
   void import_test_data(const std::vector<DATA_PAIR>& data);
   std::vector<DATA_PAIR> randomly_shuffle_training_data();
   std::vector< std::vector<neural_net::DATA_PAIR> > 
      generate_training_mini_batches(
         int mini_batch_size, 
         const std::vector<DATA_PAIR>& shuffled_training_data);
   void print_data_pair(int t, const DATA_PAIR& curr_data);
   genvector feedforward(const genvector& a_input);
   void sgd(int n_epochs, int mini_batch_size, double learning_rate,
            double lambda);

   double evaluate();

  private: 

   unsigned int num_layers, n_training_samples, n_test_samples;
   unsigned int n_classes;
   std::vector<unsigned int> layer_sizes;

   std::vector<genvector*> biases, nabla_biases, delta_nabla_biases;
//	Bias STL vectors have size num_layers
   std::vector<genmatrix*> weights, nabla_weights, delta_nabla_weights;
//	Weight STL vectors have size num_layers-1

   std::vector<DATA_PAIR> training_data;
   std::vector<DATA_PAIR> test_data;

   void update_mini_batch(
      std::vector<DATA_PAIR>& mini_batch, double learning_rate, double lambda);
   void backpropagate(const DATA_PAIR& curr_data_pair);

   genvector cost_derivative(genvector& output_activation, int y);

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const neural_net& N);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set and get member functions:

inline int neural_net::get_layer_size(int l) const
{
   return layer_sizes[l];
}

inline genvector* neural_net::get_biases(int l) const
{
   return biases[l];
}

inline genmatrix* neural_net::get_weights(int l) const
{
   return weights[l];
}

#endif  // neural_net.h


