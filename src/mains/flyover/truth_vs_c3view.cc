// ==========================================================================
// Program TRUTH_VS_C3VIEW

//				./color_dists

// ==========================================================================
// Last updated on 6/4/15
// ==========================================================================

#include  <iostream>
#include  <string>
#include  <vector>
#include "geometry/bounding_box.h"
#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "math/prob_distribution.h"
#include "video/texture_rectangle.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;
using std::vector;

int main(int argc, char* argv[])
{
   cout.precision(12);
   
   vector<double> truth_value, c3view_value, delta_value;

// 1   
   truth_value.push_back(8.5);
   c3view_value.push_back(8.4);

// 2
   truth_value.push_back(1.53);
   c3view_value.push_back(1.59);

// 3a
   truth_value.push_back(1.32);
   c3view_value.push_back(1.34);

// 3b
   truth_value.push_back(0.15);
   c3view_value.push_back(0.15);

// 3c
   truth_value.push_back(1.38);
   c3view_value.push_back(1.40);

// 3d
   truth_value.push_back(0.15);
   c3view_value.push_back(0.15);

// 3e
   truth_value.push_back(5.01);
   c3view_value.push_back(5.01);

// 3f
   truth_value.push_back(2.32);
   c3view_value.push_back(2.33);


// 4a
   truth_value.push_back(1.28);
   c3view_value.push_back(1.27);

// 4b
   truth_value.push_back(0.15);
   c3view_value.push_back(0.15);

// 4c
   truth_value.push_back(1.38);
   c3view_value.push_back(1.39);

// 4d
   truth_value.push_back(0.15);
   c3view_value.push_back(0.15);

// 4e
   truth_value.push_back(4.94);
   c3view_value.push_back(5.03);

// 4f
   truth_value.push_back(2.27);
   c3view_value.push_back(2.35);

// 5
   truth_value.push_back(1.39);
   c3view_value.push_back(1.33);

// 6
   truth_value.push_back(0.29);
   c3view_value.push_back(0.28);

// 7
   truth_value.push_back(0.37);
   c3view_value.push_back(0.38);

// 8
   truth_value.push_back(0.12);
   c3view_value.push_back(0.12);

// 9
   truth_value.push_back(2.37);
   c3view_value.push_back(2.38);

// 10
   truth_value.push_back(5.18);
   c3view_value.push_back(5.22);

// 11a
   truth_value.push_back(2.43);
   c3view_value.push_back(2.54);

// 11b
   truth_value.push_back(0.52);
   c3view_value.push_back(0.48);

// 12
   truth_value.push_back(0.29);
   c3view_value.push_back(0.27);

// 13
   truth_value.push_back(2.45);
   c3view_value.push_back(2.43);

// 14
   truth_value.push_back(0.61);
   c3view_value.push_back(0.57);

// 15a
   truth_value.push_back(2.04);
   c3view_value.push_back(2.09);

// 15b
   truth_value.push_back(2.05);
   c3view_value.push_back(2.16);

// 15c
   truth_value.push_back(0.29);
   c3view_value.push_back(0.26);

// 15d
   truth_value.push_back(0.32);
   c3view_value.push_back(0.30);

// 15e
   truth_value.push_back(0.92);
   c3view_value.push_back(0.87);

   for(unsigned int i = 0; i < truth_value.size(); i++){
     delta_value.push_back(c3view_value.at(i) - truth_value.at(i));

     cout << truth_value[i] << "  " << fabs(delta_value[i]) << endl;
   }

   prob_distribution prob_truth(truth_value,20);
//   prob_reds.set_xmin(0);
//   prob_reds.set_xmax(1);
//   prob_reds.set_xtic(0.1);
//   prob_reds.set_xsubtic(0.002);
   prob_truth.set_densityfilenamestr("truth_density.meta");
   prob_truth.write_density_dist(false,true);

   prob_distribution prob_c3view(c3view_value,20);
//   prob_reds.set_xmin(0);
//   prob_reds.set_xmax(1);
//   prob_reds.set_xtic(0.1);
//   prob_reds.set_xsubtic(0.002);
   prob_c3view.set_densityfilenamestr("c3view_density.meta");
   prob_c3view.write_density_dist(false,true);


   prob_distribution prob_delta(delta_value,20);
//   prob_reds.set_xmin(0);
//   prob_reds.set_xmax(1);
//   prob_reds.set_xtic(0.1);
//   prob_reds.set_xsubtic(0.002);
   prob_delta.set_densityfilenamestr("delta_density.meta");
   prob_delta.write_density_dist(false,true);


}

