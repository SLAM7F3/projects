// ==========================================================================
// Program COMPSAT reads in two greyscale XYZRGBA files.  It first
// converts their grey [colored] RGB values to p values ranging
// between 0 and 1 [-1].  It next averages together nonnegative p
// values whereever they exist within the point cloud.  The averaged
// composite result is written to an output XYZP file.
// ==========================================================================
// Last updated on 3/28/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);

   string input_subdir="./composite/";

// First read in satellite model surface normal information from a
// specialized XYZP point file:

   const double null_p_value=-1;

   string new_xyzp_filename;
   outputfunc::newline();
   cout << "Enter new constituent XYZP or XYZRBA filename:" << endl;
   cout << "( i.e. p_N(rvec) )" << endl;
   cin >> new_xyzp_filename;
   new_xyzp_filename=input_subdir+new_xyzp_filename;
   string suffix=stringfunc::suffix(new_xyzp_filename);
   vector<fourvector>* new_XYZP_ptr = NULL;
   if (suffix=="xyzp" || suffix=="fxyz")
   {
      new_XYZP_ptr=xyzpfunc::read_xyzp_float_data(new_xyzp_filename);
   }
   else if (suffix=="xyzrgba")
   {
      new_XYZP_ptr=xyzpfunc::convert_xyzrgba_to_xyzp_data(
         new_xyzp_filename,null_p_value);
   }

   string new_shading_factors_filename;
   outputfunc::newline();
   cout << "Enter new constituent shading factors P filename:" << endl;
   cout << "( i.e. nhat(rvec) . Ihat_N )" << endl;
   cin >> new_shading_factors_filename;
   new_shading_factors_filename=input_subdir+new_shading_factors_filename;
   vector<float>* new_shading_factors_ptr=new vector<float>;
   xyzpfunc::read_p_data(
      new_shading_factors_filename,new_shading_factors_ptr);

   string prev_avg_xyzp_filename;   
   outputfunc::newline();
   cout << "Enter previous composite XYZP or XYZRGBA filename:" << endl;
   cout << "( i.e. <p(rvec)>^N-1 )" << endl;
   cin >> prev_avg_xyzp_filename;
   prev_avg_xyzp_filename=input_subdir+prev_avg_xyzp_filename;
   suffix=stringfunc::suffix(prev_avg_xyzp_filename);
   vector<fourvector>* prev_avg_xyzp_ptr = NULL;
   if (suffix=="xyzp" || suffix=="fxyz")
   {
      prev_avg_xyzp_ptr=xyzpfunc::read_xyzp_float_data(
         prev_avg_xyzp_filename);
   }
   else if (suffix=="xyzrgba")
   {
      prev_avg_xyzp_ptr=xyzpfunc::convert_xyzrgba_to_xyzp_data(
         prev_avg_xyzp_filename,null_p_value);
   }

   string prev_shading_factors_filename;
   outputfunc::newline();
   cout << "Enter previous constituent shading factors P filename:" << endl;
   cout << "( i.e. nhat(rvec) . Ihat_N-1 )" << endl;
   cin >> prev_shading_factors_filename;
   prev_shading_factors_filename=input_subdir+prev_shading_factors_filename;
   vector<float>* prev_shading_factors_ptr=new vector<float>;
   xyzpfunc::read_p_data(prev_shading_factors_filename,
                         prev_shading_factors_ptr);
   
   string prev_weights_filename;
   outputfunc::newline();
   cout << "Enter previous weights P filename:" << endl;
   cout << "( i.e. w_N-1 )" << endl;
   cin >> prev_weights_filename;
   prev_weights_filename=input_subdir+prev_weights_filename;
   vector<float>* prev_weights_ptr=new vector<float>;
   xyzpfunc::read_p_data(prev_weights_filename,prev_weights_ptr);

// Instantiate STL vectors to hold new fused XYZP and weight
// information:

   vector<fourvector>* avg_XYZP_ptr=new vector<fourvector>;
   vector<float>* new_weights_ptr=new vector<float>;

   int null_p=0;
   int brand_new_p=0;
   int old_avg_p=0;
   int avg_new_old_p=0;
   int bad_case1=0;
   int bad_case2=0;

   for (unsigned int i=0; i<new_XYZP_ptr->size(); i++)
   {
      fourvector xyzp_new( (*new_XYZP_ptr)[i] );
      fourvector xyzp_avg_prev( (*prev_avg_xyzp_ptr)[i] );
      double p_new=xyzp_new.get(3);
      double p_avg_prev=xyzp_avg_prev.get(3);
      float w_prev=(*prev_weights_ptr)[i];

      double p_avg=null_p_value;
      float w_new=0;
      if (p_new < 0 && p_avg_prev < 0)
      {
         p_avg=-1;
         w_new=0;
         null_p++;
      }
      else if (p_new >= 0 && p_avg_prev < 0)
      {
         p_avg=p_new;
         w_new=1;
         brand_new_p++;
      }
      else if (p_new < 0 && p_avg_prev >= 0)
      {
         p_avg=p_avg_prev;
         w_new=w_prev;
         old_avg_p++;
      }
      else if (p_new >= 0 && p_avg_prev >= 0)
      {
         double prev_shade_factor=(*prev_shading_factors_ptr)[i];
         double new_shade_factor=(*new_shading_factors_ptr)[i];

         if (prev_shade_factor > 0 && new_shade_factor > 0)
         {
            double ratio=prev_shade_factor/new_shade_factor;
            float w_new_inv=1+ratio/w_prev;
            w_new=1.0/w_new_inv;
//            cout << "p_new = " << p_new << " p_avg_prev = " << p_avg_prev 
//                 << endl;
//         cout << "w_prev = " << w_prev << " ratio = " << ratio 
//              << " w_new = " << w_new << " (1-w_new) = " << 1-w_new << endl;
            p_avg=w_new*p_new+(1-w_new)*p_avg_prev;
         }
         else if (prev_shade_factor > 0 && new_shade_factor < 0)
         {
            w_new=w_prev;
            p_avg=p_avg_prev;
         }
         else if (prev_shade_factor < 0 && new_shade_factor > 0)
         {

// This next case is a total fudge.  In theory, this case should never
// occur...

            double ratio=1;
            float w_new_inv=1+ratio/w_prev;
            w_new=1.0/w_new_inv;
            p_avg=w_new*p_new+(1-w_new)*p_avg_prev;
            bad_case1++;
         }
         else if (prev_shade_factor <= 0 && new_shade_factor <= 0)
         {
            w_new=w_prev;
            p_avg=p_avg_prev;
            bad_case2++;
         }
         
         avg_new_old_p++;
      }
      avg_XYZP_ptr->push_back(
         fourvector(xyzp_new.get(0),xyzp_new.get(1),xyzp_new.get(2),p_avg));
      new_weights_ptr->push_back(w_new);
   } // loop over index i labeling XYZP points

   cout << "null_p = " << null_p << endl;
   cout << "brand_new_p = " << brand_new_p << endl;
   cout << "old_avg_p = " << old_avg_p << endl;
   cout << "avg_new_old_p = " << avg_new_old_p << endl;
   cout << "sum = " << null_p+ brand_new_p + old_avg_p + avg_new_old_p 
        << endl;
   cout << "bad_case1 = " << bad_case1 << " bad_case2 = " << bad_case2 
        << endl;
   cout << "total npoints = " << avg_XYZP_ptr->size() << endl;

   string avg_xyzp_filename=input_subdir+"avg_probs.xyzp";
   filefunc::deletefile(avg_xyzp_filename);
   xyzpfunc::write_xyzp_data(avg_xyzp_filename,avg_XYZP_ptr,false);

   string weights_filename=input_subdir+"weights.p";
   filefunc::deletefile(weights_filename);
   xyzpfunc::write_p_data(weights_filename,new_weights_ptr,false);

   delete new_XYZP_ptr;
   delete prev_avg_xyzp_ptr;
   delete prev_weights_ptr;
   delete avg_XYZP_ptr;
   delete new_weights_ptr;
}
