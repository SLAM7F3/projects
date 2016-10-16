// ==========================================================================
// Header file for stand-alone machinelearning methods
// ==========================================================================
// Last updated on 10/5/16; 10/12/16; 10/15/16; 10/16/16
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
#include "numrec/nrfuncs.h"

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

   void ReLU(const genvector& Z, genvector& A)
   {
      for(unsigned int i = 0; i < Z.get_mdim(); i++)
      {
         if(Z.get(i) < 0) 
         {
            A.put(i,0);
         }
         else
         {
            A.put(i,Z.get(i));
         }
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
   void softmax(const genvector& Z, genvector& A)
   {
//      cout << "inside softmax" << endl;

      double Zmax = NEGATIVEINFINITY;
      for(unsigned int i = 0; i < Z.get_mdim(); i++)
      {
         Zmax = basic_math::max(Zmax, Z.get(i));
      }
//      cout << "Zmax = " << Zmax << endl;

      double denom = 0;
      for(unsigned int i = 0; i < Z.get_mdim(); i++)
      {
         double curr_exp = exp(Z.get(i) - Zmax);
         denom += curr_exp;
         A.put(i, curr_exp);
      }
      
      for(unsigned int i = 0; i < A.get_mdim(); i++)
      {
         A.put(i, A.get(i) / denom);
      }
   }

// --------------------------------------------------------------------------
// Method generate_data_samples() implements a toy example where
// training vectors are random integers and output labels are {0,1}
// depending upon whether random integers exceed a threshold value.
   
   void generate_data_samples(
      int n_samples, vector<neural_net::DATA_PAIR>& samples)
   {
      const int Din = 1;
      const int max_int_value = 2000;
      const int threshold = max_int_value / 2;

      for(int n = 0; n < n_samples; n++)
      {
         int curr_x = mathfunc::getRandomInteger(max_int_value);
         int curr_y = 0;
         if(curr_x > threshold)
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
// Method generate_2d_data_samples() 

   void generate_2d_data_samples(
      int n_samples, vector<neural_net::DATA_PAIR>& samples)
   {
      const int Din = 2;
      const double rmax = 2;
      double r_inner = 0.95;
      double r_outer = 1.05;

      for(int n = 0; n < n_samples; n++)
      {
         double theta = 2*PI*nrfunc::ran1();
         bool r_OK = false;
         double r;
         while(!r_OK)
         {
            r = rmax * nrfunc::ran1();
            if(r < r_inner || r > r_outer)
            {
               r_OK = true;
            }
         }

         int curr_label = 0;
         if(r > r_outer){
            curr_label = 1;
         }
        
         neural_net::DATA_PAIR curr_data_pair;
         curr_data_pair.first = new genvector(Din);
         curr_data_pair.first->put(0, r * cos(theta));
         curr_data_pair.first->put(1, r * sin(theta));
         curr_data_pair.second = curr_label;
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

// --------------------------------------------------------------------------
// Method remove_data_samples_mean() 
   
   void remove_data_samples_mean(vector<neural_net::DATA_PAIR>& samples)
   {
      const int Din = 1;
      genvector *mean = new genvector(Din);
      genvector *sqrmean = new genvector(Din);
      genvector *variance = new genvector(Din);
      genvector *std_dev = new genvector(Din);

      for(unsigned int n = 0; n < samples.size(); n++)
      {
//         cout << "n = " << n << " samples[n] = " << *samples[n].first << endl;
         *mean += *samples[n].first;
         *sqrmean += samples[n].first->hadamard_product(*samples[n].first);
      }
      *mean /= samples.size();
      *sqrmean /= samples.size();
      *variance = *sqrmean - *mean;
      *std_dev = variance->hadamard_power(*variance, 0.5);

//      cout << "*mean = " << *mean << endl;
//      cout << "*sqrmean = " << *sqrmean << endl;
//      cout << "*variance = " << *variance << endl;
//      cout << "*std_dev = " << *std_dev << endl;

      for(unsigned int n = 0; n < samples.size(); n++)
      {
         *samples[n].first -= *mean;
         *samples[n].first = samples[n].first->hadamard_division(*std_dev);
      }

      delete mean;
      delete sqrmean;
   }
   

   
} // machinelearning_func namespace

#endif  // machinelearning_funcs.h
