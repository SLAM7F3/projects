// ==========================================================================
// Stand-alone machinelearning methods
// ==========================================================================
// Last updated on 2/19/13; 2/8/16; 2/9/16; 10/5/16
// ==========================================================================

#include "math/genvector.h"

namespace machinelearning_func
{
   double sigmoid(double z);
   genvector sigmoid(genvector& z);
   void sigmoid(genmatrix& Zin, genmatrix& Zout);

   double deriv_sigmoid(double z);
   genvector deriv_sigmoid(genvector& z);

} // machine_learning_func namespace


