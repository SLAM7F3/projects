// ==========================================================================
// Program COLOR_METRIC
// ==========================================================================
// Last updated on 5/10/14
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>

#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/texture_rectangle.h"
#include "video/RGB_analyzer.h"


using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);
   RGB_analyzer* RGB_analyzer_ptr=new RGB_analyzer();
   RGB_analyzer_ptr->import_quantized_RGB_lookup_table();

   int n_colors=RGB_analyzer_ptr->get_n_color_indices();

   genmatrix M_color(n_colors,n_colors);
   M_color.identity();
   double alpha = 0.5;

/*
   for (int color_index=0; color_index <28; color_index++)
   {
      vector<double> C;
      for (int i=0; i<n_colors; i++)
      {
         C.push_back(0);
      }
      C[color_index] = 1;

//   C[2]=1;	// yellow
//   C[9]=1;	// lightyellow
//   C[16]=1;	// dark yellow
//   C[23]=1;	// grey yellow

      vector<double> C_prop = RGB_analyzer_ptr->propagate_color_hist(C);
      vector<double> C_prop2 = RGB_analyzer_ptr->propagate_color_hist(C_prop);
      for (int i=0; i<C_prop2.size(); i++)
      {
         C_prop2[i] = 0.5 * C_prop2[i];
         if (C_prop2[i] < 0.2) C_prop2[i]=0;

         if (C_prop2[i] >0.2 && C_prop2[i] < 0.3)
         {
            cout << "M_color_ptr->put(" << color_index << "," << i 
                 << ",sqr(alpha));"
                 << endl;
         }
      }

   } // loop over color_index
*/

   M_color.put(
      RGB_analyzer_ptr->get_color_index("lightred"), 
      RGB_analyzer_ptr->get_color_index("darkgrey"), alpha);
   M_color.put(
      RGB_analyzer_ptr->get_color_index("greyred"), 
      RGB_analyzer_ptr->get_color_index("lightgrey"), alpha);

   M_color.put(
      RGB_analyzer_ptr->get_color_index("lightorange"), 
      RGB_analyzer_ptr->get_color_index("darkgrey"), alpha);
   M_color.put(
      RGB_analyzer_ptr->get_color_index("greyorange"), 
      RGB_analyzer_ptr->get_color_index("lightgrey"), alpha);

   M_color.put(
      RGB_analyzer_ptr->get_color_index("lightyellow"), 
      RGB_analyzer_ptr->get_color_index("darkgrey"), alpha);
   M_color.put(
      RGB_analyzer_ptr->get_color_index("greyyellow"), 
      RGB_analyzer_ptr->get_color_index("lightgrey"), alpha);

   M_color.put(
      RGB_analyzer_ptr->get_color_index("lightgreen"), 
      RGB_analyzer_ptr->get_color_index("darkgrey"), alpha);
   M_color.put(
      RGB_analyzer_ptr->get_color_index("greygreen"), 
      RGB_analyzer_ptr->get_color_index("lightgrey"), alpha);

   M_color.put(
      RGB_analyzer_ptr->get_color_index("lightcyan"), 
      RGB_analyzer_ptr->get_color_index("darkgrey"), alpha);
   M_color.put(
      RGB_analyzer_ptr->get_color_index("greycyan"), 
      RGB_analyzer_ptr->get_color_index("lightgrey"), alpha);

   M_color.put(
      RGB_analyzer_ptr->get_color_index("lightblue"), 
      RGB_analyzer_ptr->get_color_index("darkgrey"), alpha);
   M_color.put(
      RGB_analyzer_ptr->get_color_index("greyblue"), 
      RGB_analyzer_ptr->get_color_index("lightgrey"), alpha);

   M_color.put(
      RGB_analyzer_ptr->get_color_index("lightpurple"), 
      RGB_analyzer_ptr->get_color_index("darkgrey"), alpha);
   M_color.put(
      RGB_analyzer_ptr->get_color_index("greypurple"), 
      RGB_analyzer_ptr->get_color_index("lightgrey"), alpha);


// Export nonzero entries within M_color:

   for (int i=0; i<n_colors; i++)
   {
//      for (int j=0; j<n_colors; j++)
      for (int j=i+1; j<n_colors; j++)
      {
         if (M_color.get(i,j) > 0)
         {
//            cout << "offdiag_entry = " << off_diag_entry++
//                 << " i = " << i << " j = " << j 
//                 << " M(i,j) = " << M_color.get(i,j) << endl;
            cout << "M_color_ptr->put(" << i << "," << j << ",sqr(alpha));"
                 << endl;
         }
      }
   }


   delete RGB_analyzer_ptr;
}

