// ==========================================================================
// Header file for stand-alone machinelearning methods
// ==========================================================================
// Last updated on 10/16/16; 10/17/16; 10/19/16l 10/20/16
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
      A.clear_values();
      for(unsigned int i = 0; i < Z.get_mdim(); i++)
      {
         double Zi = Z.get(i);
         if(Zi > 0)
         {
            A.put(i, Z.get(i));
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

   void ReLU(const genmatrix& Z, genmatrix& A)
   {
      A.clear_values();
      for(unsigned int i = 0; i < Z.get_mdim(); i++)
      {
         for (unsigned int j = 0; j < Z.get_ndim(); j++)
         {
            double Zij = Z.get(i,j);
            if(Zij > 0)
            {
               A.put(i, j, Zij);
            }
         }
      }
   }

   void ReLU(int zcol, const genmatrix& Z, genmatrix& A)
   {
      for(unsigned int i = 0; i < Z.get_mdim(); i++)
      {
         double curr_Z = Z.get(i,zcol);
         if(curr_Z > 0)
         {
            A.put(i, zcol, curr_Z);
         }
      }
   }

// --------------------------------------------------------------------------
   void softmax(const genvector& Z, genvector& A)
   {
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
      
//      double psum = 0;
      for(unsigned int i = 0; i < A.get_mdim(); i++)
      {
         A.put(i, A.get(i) / denom);
//         psum += A.get(i);
      }
//      cout << "psum = " << psum << endl;

/*
      if(psum < 0.99 || psum > 1.01)
      {
         cout << "Trouble in machinelearningfunc::softmax()" << endl;

         for(unsigned int i = 0; i < A.get_mdim(); i++)
         {
            cout << "i = " << i << " A.get(i) = " << A.get(i) << endl;
         }
         cout << "denom = " << denom << endl;
         cout << "psum = " << psum << endl;
         outputfunc::enter_continue_char();
      }
*/
   }

// --------------------------------------------------------------------------
   void softmax(const genmatrix& Z, genmatrix& A)
   {
      for(unsigned int j = 0; j < Z.get_ndim(); j++)
      {
         double Zmax = NEGATIVEINFINITY;
         for(unsigned int i = 0; i < Z.get_mdim(); i++)
         {
            Zmax = basic_math::max(Zmax, Z.get(i,j));
         }

         double denom = 0;
         for(unsigned int i = 0; i < Z.get_mdim(); i++)
         {
            double curr_exp = exp(Z.get(i,j) - Zmax);
            denom += curr_exp;
            A.put(i, j, curr_exp);
         }
      
         for(unsigned int i = 0; i < A.get_mdim(); i++)
         {
            A.put(i, j, A.get(i, j) / denom);
         }
      } // loop over index j labeling columns of genmatrices Z and A
   }

// --------------------------------------------------------------------------
   void softmax(int zcol, const genmatrix& Z, genmatrix& A)
   {
      double Zmax = NEGATIVEINFINITY;
      for(unsigned int i = 0; i < Z.get_mdim(); i++)
      {
         Zmax = basic_math::max(Zmax, Z.get(i,zcol));
      }

      double denom = 0;
      for(unsigned int i = 0; i < Z.get_mdim(); i++)
      {
         double curr_exp = exp(Z.get(i,zcol) - Zmax);
         denom += curr_exp;
         A.put(i, zcol, curr_exp);
      }
      
      for(unsigned int i = 0; i < A.get_mdim(); i++)
      {
         A.put(i, zcol, A.get(i, zcol) / denom);
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
// Generate_2d_spiral_data_samples() randomly populates 2D
// points lying within an inner and beyond an outer circular region.  

   void generate_2d_spiral_data_samples(
      int n_samples, vector<neural_net::DATA_PAIR>& samples)
   {
      const int Din = 2;

// Class 0:

      int curr_label = 0;
      for(int n = 0; n < n_samples/2; n++)
      {
         double theta = 2*PI*nrfunc::ran1();
         double rmin = 0.9 * fabs(theta) / (2 * PI);
         double rmax = 1.1 * fabs(theta) / (2 * PI);
         double r = rmin + nrfunc::ran1() * (rmax - rmin);
         
         neural_net::DATA_PAIR curr_data_pair;
         curr_data_pair.first = new genvector(Din);
         curr_data_pair.first->put(0, r * cos(theta));
         curr_data_pair.first->put(1, r * sin(theta));
         curr_data_pair.second = curr_label;
         samples.push_back(curr_data_pair);
      } // loop over index n labeling training samples

// Class 1:

      curr_label = 1;
      for(int n = 0; n < n_samples/2; n++)
      {
         double theta = PI + 2*PI*nrfunc::ran1();
         double rmin = 0.9 * fabs(theta-PI) / (2 * PI);
         double rmax = 1.1 * fabs(theta-PI) / (2 * PI);
         double r = rmin + nrfunc::ran1() * (rmax - rmin);
         
         neural_net::DATA_PAIR curr_data_pair;
         curr_data_pair.first = new genvector(Din);
         curr_data_pair.first->put(0, r * cos(theta));
         curr_data_pair.first->put(1, r * sin(theta));
         curr_data_pair.second = curr_label;
         samples.push_back(curr_data_pair);
      } // loop over index n labeling training samples
   }


// --------------------------------------------------------------------------
// Generate_2d_circular_data_samples() randomly populates 2D
// points lying within an inner and beyond an outer circular region.  

   void generate_2d_circular_data_samples(
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
