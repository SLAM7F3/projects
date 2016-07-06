// ==========================================================================
// Header file for stand-alone compression math functions
// ==========================================================================
// Last updated on 2/11/16; 2/12/16; 2/23/16
// ==========================================================================

#ifndef COMPRESSFUNCS_H
#define COMPRESSFUNCS_H

#include "math/genmatrix.h"

class genmatrix;

namespace compressfunc
{
   void generate_random_projection_matrix(genmatrix& R);
}

#endif  // compressfunc namespace

