// ========================================================================
// Program CHOGTEST

// ========================================================================
// Last updated on 3/14/12; 3/18/12; 3/20/12
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "osg/osgGraphicals/AnimationController.h"
#include "video/camera.h"
#include "video/camerafuncs.h"
#include "math/constant_vectors.h"
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "image/imagefuncs.h"
#include "templates/mytemplates.h"
#include "passes/PassesGroup.h"
#include "video/photogroup.h"
#include "video/sift_detector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::string;
using std::vector;

// ==========================================================================
int main( int argc, char** argv )
{

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   const int ndims=3;
   PassesGroup passes_group(&arguments);

// Read photographs from input video passes:

   photogroup* photogroup_ptr=new photogroup;
   photogroup_ptr->read_photographs(passes_group);

   sift_detector* sift_detector_ptr=new sift_detector(photogroup_ptr);
  
   int n_requested_features=10000;
   sift_detector_ptr->extract_CHOG_features(n_requested_features);

   double max_ratio=0.80;
//   double max_ratio=0.90;
   cout << "Enter max ratio for KL divergence comparison:" << endl;
   cin >> max_ratio;
   
   double worst_frac_to_reject=0.10;
   double max_scalar_product=0.0005;
   sift_detector_ptr->identify_CHOG_feature_matches_via_fundamental_matrix(
      max_ratio,worst_frac_to_reject,max_scalar_product);

   fundamental* fundamental_ptr=sift_detector_ptr->get_fundamental_ptr();
   genmatrix* F_ptr=fundamental_ptr->get_F_ptr();

   cout << "*fundamental_ptr = " << *fundamental_ptr << endl;
   cout << "fundamental rank = " << F_ptr->rank() << endl;

   int n_iters=3;
//   cout << "Enter n_iters:" << endl;
//   cin >> n_iters;

   for (int iter=0; iter<n_iters; iter++)
   {
      threevector e0_hat=fundamental_ptr->get_null_vector();
      fundamental_ptr->solve_for_fundamental(e0_hat);
   } // loop over iter index
   *F_ptr = *F_ptr/F_ptr->get(2,2);

   cout << "Refined fundamental matrix = " << *fundamental_ptr << endl;
}

