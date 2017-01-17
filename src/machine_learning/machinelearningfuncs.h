// ==========================================================================
// Stand-alone machinelearning methods
// ==========================================================================
// Last updated on 11/8/16; 12/13/16; 12/14/16; 1/17/17
// ==========================================================================

#include <vector>
#include "math/genvector.h"
#include "machine_learning/neural_net.h"

namespace machinelearning_func
{

// Sigmoid methods:

   double sigmoid(double z);
   genvector sigmoid(genvector& z);
   void sigmoid(genmatrix& Zin, genmatrix& Zout);

   double deriv_sigmoid(double z);
   genvector deriv_sigmoid(genvector& z);

// Batch normalization methods

   void batch_normalization(
      genvector& Z, const genvector& gamma, const genvector& beta);
   void batch_normalization_transform(
      const std::vector<double>& x, double gamma, double beta,
      double& mu, double& sqr_sigma, std::vector<double>& y);

// ReLU methods:

   void ReLU(genvector& X);
   void ReLU(const genvector& Z, genvector& A);
   void ReLU(genmatrix& Z);
   void ReLU(const genmatrix& Z, genmatrix& A);
   void ReLU(int zcol, const genmatrix& Z, genmatrix& A);

   void set_leaky_ReLU_small_slope(double slope);
   double get_leaky_ReLU_small_slope();
   void leaky_ReLU(genvector& X);
   void leaky_ReLU(const genvector& Z, genvector& A);

   void softmax(const genvector& Z, genvector& A);
   void softmax(const genmatrix& Z, genmatrix& A);
   void softmax(int zcol, const genmatrix& Z, genmatrix& A);

   void hardwire_output_action(int zcol, int output_action, genmatrix& A);
   void constrained_softmax(int zcol, const genvector& x_input, 
                            const genmatrix& Z, genmatrix& A);
   void constrained_identity(int zcol, const genvector& x_input, 
                             const genmatrix& Z, genmatrix& A);

   void generate_data_samples(
      int n_samples, std::vector<neural_net::DATA_PAIR>& samples);
   void generate_2d_spiral_data_samples(
      int n_samples, std::vector<neural_net::DATA_PAIR>& samples);
   void generate_2d_circular_data_samples(
      int n_samples, std::vector<neural_net::DATA_PAIR>& samples);

   void print_data_samples(
      const std::vector<neural_net::DATA_PAIR>& samples);
   void remove_data_samples_mean(std::vector<neural_net::DATA_PAIR>& samples);


} // machine_learning_func namespace


