// ==========================================================================
// Stand-alone machinelearning methods
// ==========================================================================
// Last updated on 10/5/16; 10/12/16; 10/15/16; 10/16/16
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

   void ReLU(genvector& X);
   void ReLU(const genvector& Z, genvector& A);
   void ReLU(genmatrix& Z);
   void softmax(const genvector& Z, genvector& A);

   void generate_data_samples(
      int n_samples, std::vector<neural_net::DATA_PAIR>& samples);
   void print_data_samples(
      const std::vector<neural_net::DATA_PAIR>& samples);
   void remove_data_samples_mean(std::vector<neural_net::DATA_PAIR>& samples);


} // machine_learning_func namespace


