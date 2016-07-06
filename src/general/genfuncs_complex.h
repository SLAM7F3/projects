// ==========================================================================
// Header file for some useful general functions for C++ programs (in
// alphabetical order within this file) which involve our complex
// class.  This header file must be included at the top of all .cc
// files which call these general functions, since the functions
// themselves are compiled and sit inside the library libgen.a
// ==========================================================================
// Last updated on 6/18/03
// ==========================================================================

#ifndef GENFUNCS_COMPLEX_H
#define GENFUNCS_COMPLEX_H

#include <stdlib.h> 	// Needed for system() function to perform Unix calls
#include <fstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <string>
#include "math/complex.h"

// ==========================================================================
// Function definitions 
// ==========================================================================

extern double abs(complex z);
extern complex average_complex(complex A[], int Nsize);
extern complex* new_clear_carray(int nsize);
extern complex polycoeff(int n, int k, complex f[], complex g[]);
extern complex simpsonsum_complex(complex f[],int startbin,int stopbin);
extern complex sqr(complex z);
extern complex variance_complex(complex A[], int Nsize);

#endif  // genfuncs_complex.h










