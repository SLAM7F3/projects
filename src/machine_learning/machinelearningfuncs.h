// ==========================================================================
// Stand-alone machinelearning methods
// ==========================================================================
// Last updated on 11/4/16; 11/6/16; 11/8/16; 12/13/16
// ==========================================================================

#include <vector>
#include "math/genvector.h"
#include "machine_learning/neural_net.h"

namespace machinelearning_func
{
   double sigmoid(double z);
   genvector sigmoid(genvector& z);
   void sigmoid(genmatrix& Zin, genmatrix& Zout);

   double deriv_sigmoid(double z);
   genvector deriv_sigmoid(genvector& z);

   void batch_normalization(
      genvector& Z, const genvector& gamma, const genvector& beta);

   void ReLU(genvector& X);
   void ReLU(const genvector& Z, genvector& A);
   void ReLU(genmatrix& Z);
   void ReLU(const genmatrix& Z, genmatrix& A);
   void ReLU(int zcol, const genmatrix& Z, genmatrix& A);
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


