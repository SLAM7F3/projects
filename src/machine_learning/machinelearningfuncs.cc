// ==========================================================================
// Header file for stand-alone machinelearning methods
// ==========================================================================
// Last updated on 2/9/16; 10/5/16; 10/12/16; 10/15/16
// ==========================================================================

#ifndef MACHINELEARNING_H
#define MACHINELEARNING_H

#include <iostream>
#include <math.h>
#include <set>
#include <string>
#include <vector>

#include "machine_learning/machinelearningfuncs.h"
#include "math/mathfuncs.h"

namespace machinelearning_func
{

   using std::cout;
   using std::endl;
   using std::vector;

// ==========================================================================
// Inlined methods:
// ==========================================================================
   
   double sigmoid(double z)
   {
      return 1.0/(1+exp(-z));
   }

   genvector sigmoid(genvector& z)
   {
      genvector output(z.get_mdim());
      for(unsigned int i = 0; i < z.get_mdim(); i++)
      {
         output.put(i, sigmoid( z.get(i) ));
      }
      return output;
   }

   void sigmoid(genmatrix& Zin, genmatrix& Zout)
   {
      for(unsigned int j = 0; j < Zin.get_ndim(); j++)
      {
         for(unsigned int i = 0; i < Zin.get_mdim(); i++)
         {
            Zout.put(i, j, sigmoid( Zin.get(i,j) ));
         }
      }
   }
   

   double deriv_sigmoid(double z)
   {
      double f=sigmoid(z);
      return f*(1-f);
   }

   genvector deriv_sigmoid(genvector& z)
   {
      genvector output(z.get_mdim());
      for(unsigned int i = 0; i < z.get_mdim(); i++)
      {
         output.put(i, deriv_sigmoid( z.get(i) ) );
      }
      return output;
   }

   void ReLU(genvector& X)
   {
      for(unsigned int i = 0; i < X.get_mdim(); i++)
      {
         if(X.get(i) < 0) X.put(i,0);
      }
   }

   void ReLU(genmatrix& Z)
   {
      for(unsigned int i = 0; i < Z.get_mdim(); i++)
      {
         for(unsigned int j = 0; j < Z.get_ndim(); j++)
         {
            if(Z.get(i,j) < 0) Z.put(i,j,0);
         }
      }
   }

// --------------------------------------------------------------------------
// Method generate_data_samples() implements a toy example where
// training vectors are random integers and output labels are {0,1}
// depending upon whether random integers are even or odd.
   
   void generate_data_samples(
      int n_samples, vector<neural_net::DATA_PAIR>& samples)
   {
      const int Din = 1;
      for(int n = 0; n < n_samples; n++)
      {
         int curr_x = mathfunc::getRandomInteger(1000);
         int curr_y = 0;
         if(curr_x%2 == 1)
         {
            curr_y = 1;
         }

         neural_net::DATA_PAIR curr_data_pair;
         curr_data_pair.first = new genvector(Din);
         curr_data_pair.first->put(0, curr_x);
         curr_data_pair.second = curr_y;
         samples.push_back(curr_data_pair);

      } // loop over index n labeling training samples
   }

// --------------------------------------------------------------------------
// Method print_data_samples() 
   
   void print_data_samples(
      const vector<neural_net::DATA_PAIR>& samples)
   {
      for(unsigned int n = 0; n < samples.size(); n++)
      {
         neural_net::DATA_PAIR curr_sample = samples[n];
         cout << "Sample n = " << n << endl;
         cout << " y = " << curr_sample.second 
              << "   x = " << *curr_sample.first << endl;
      } // loop over index n labeling data samples
   }
   
} // machinelearning_func namespace

#endif  // machinelearning_funcs.h
