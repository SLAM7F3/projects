// ==========================================================================
// Stand-alone machinelearning methods
// ==========================================================================
// Last updated on 2/9/16; 10/5/16; 10/12/16; 10/15/16
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
   void ReLU(genmatrix& Z);

   void generate_data_samples(
      int n_samples, std::vector<neural_net::DATA_PAIR>& samples);


} // machine_learning_func namespace


