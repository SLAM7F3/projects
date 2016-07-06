// ==========================================================================
// Program FILTER_PROBS performs simple gaussian filtering to
// eliminate the worst of temporal noise fluctuations in probability
// values from a sequence of XYZP files.  It takes one of the multiple
// XYZP files as a command line argument and extracts the assumed
// static XYZ information from that file.  It also reads in all of the
// p values from every XYZP file and gaussian averages them together.
// The resultant smoothed p values are written to output XYZP files
// with an "avgd_" prefix.
// ==========================================================================
// Last updated on 12/19/06; 12/20/06; 4/23/07; 12/4/10
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "osg/osgSceneGraph/ColorMap.h"
#include "filter/filterfuncs.h"
#include "passes/PassesGroup.h"
#include "osg/osg3D/PointCloudsGroup.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "templates/mytemplates.h"
#include "threeDgraphics/xyzpfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   
// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);

// Read input ladar point cloud file:

   PassesGroup passes_group(&arguments);
   int cloudpass_ID=passes_group.get_curr_cloudpass_ID();
   string cloud_filename=passes_group.get_pass_ptr(cloudpass_ID)->
      get_first_filename();
   PassInfo* PassInfo_ptr=passes_group.get_passinfo_ptr(cloudpass_ID);

// Instantiate group to hold pointcloud information:

   PointCloudsGroup clouds_group(passes_group.get_pass_ptr(cloudpass_ID));
   PointCloud* cloud_ptr=clouds_group.generate_new_Cloud();
   int npoints=cloud_ptr->get_npoints();

// --------------------------------------------------------------------------
// Sequentially read in all XYZP files to be temporally averaged.
// Then fill a nimages x npoints matrix holding all their p values.
// In the future, we should redo this next part so that only
// n_images_to_avg images' probabilities are held in memory at any one
// time...

   string subdir="./xyzp_files/";
   string prefix="output";
//   string prefix="instant_sunny_";
   const int ndigits=1;
//      const int ndigits=2;
   string suffix=".xyzp";

   int imagenumber_start=1;
//   int imagenumber_start=0;
   int imagenumber_stop=9;
//   int imagenumber_stop=48;
   int imagenumber_skip=1;
   genmatrix prob(imagenumber_stop-imagenumber_start+1,npoints);

   for (int imagenumber=imagenumber_start; imagenumber <= imagenumber_stop;
        imagenumber += imagenumber_skip)
   {
      string imagenumber_str=
         stringfunc::integer_to_string(imagenumber,ndigits);

      string banner="Reading in image "+imagenumber_str;
      outputfunc::write_big_banner(banner);

      string xyzp_filename=subdir+prefix+imagenumber_str+suffix;
//      cout << "xyzp_filename = " << xyzp_filename << endl;
      vector<fourvector>* xyzp_ptr=
         xyzpfunc::read_xyzp_float_data(xyzp_filename);
      for (int n=0; n<npoints; n++)
      {
         prob.put(imagenumber,n,(*xyzp_ptr)[n].get(3));
      }
      delete xyzp_ptr;
   } // loop over imagenumber index

// --------------------------------------------------------------------------
// Store gaussian weights for maximal number of images to temporally
// average together.  Then store actual numbers of images to be
// averaged as well as their relative weight factors (which must sum
// to unity) as a function of central image number over which we
// iterate:

   int n_images_to_avg=5;
//   int n_images_to_avg=7;
   vector<double> weight;
   weight.reserve(n_images_to_avg);
   double sigma=1;
   double dx=0.85*sigma;
   filterfunc::gaussian_filter(dx,sigma,weight);

// Read and store static XYZ geometry information:


   vector<threevector>* xyz_ptr=xyzpfunc::read_just_xyz_float_data(
      cloud_filename);
   vector<fourvector>* xyzp_pnt_ptr=new vector<fourvector>;
   for (int imagenumber=imagenumber_start; imagenumber <= imagenumber_stop;
        imagenumber += imagenumber_skip)
   {
//      cout << "center imagenumber = " << imagenumber << endl;

      vector<int> imagenumber_to_avg;
      for (int j=basic_math::max(
         imagenumber_start,imagenumber-n_images_to_avg/2); 
           j<=basic_math::min(
              imagenumber_stop, imagenumber+n_images_to_avg/2); j++)
      {
         imagenumber_to_avg.push_back(j);
      } 

      int delta_neg=imagenumber-imagenumber_to_avg.front();
      int delta_pos=imagenumber_to_avg.back()-imagenumber;
      int jstart=n_images_to_avg/2-delta_neg;
      int jstop=n_images_to_avg/2+delta_pos;

      vector<double> rel_weight;
      for (int j=jstart; j<=jstop; j++)
      {
         rel_weight.push_back(weight[j]);
      }

// Fill *xyzp_pnt_ptr with static XYZ geometry information and
// time-averaged P intensity information:

      xyzp_pnt_ptr->clear();
      xyzp_pnt_ptr->reserve(npoints);
      for (int n=0; n<npoints; n++)
      {
         float avg_prob=0;
         double weight_sum=0;
         for (int j=0; j<imagenumber_to_avg.size(); j++)
         {
            int i=imagenumber_to_avg[j];
            double curr_prob=prob.get(i,n);
            if (curr_prob >= 0)
            {
               weight_sum += rel_weight[j];
               avg_prob += rel_weight[j]*prob.get(i,n);
//               cout << "imagenumber = " << imagenumber 
//                    << " n = " << n << " j = " << j 
//                    << " rel weight = " << rel_weight[j]
//                    << " p = " << prob.get(i,n) << endl;
            } // curr_prob >= 0 conditional
         } // loop over "temporal" index j labeling images to average
         if (weight_sum > 0)
         {
            avg_prob /= weight_sum;
         }
         else
         {
            avg_prob=0;
//            avg_prob=NEGATIVEINFINITY;
         }
         
         fourvector new_xyzp( (*xyz_ptr)[n],avg_prob);

         xyzp_pnt_ptr->push_back(new_xyzp);
      } // loop over "spatial" index n labeling points in clouds
    
      string avgd_prefix="avgd_"+prefix;
      string imagenumber_str=
         stringfunc::integer_to_string(imagenumber,ndigits);
      string avgd_xyzp_filename=subdir+avgd_prefix+imagenumber_str+suffix;
      filefunc::deletefile(avgd_xyzp_filename);
      xyzpfunc::write_xyzp_data(avgd_xyzp_filename,xyzp_pnt_ptr,false);

   } // loop over imagenumber index

   delete xyzp_pnt_ptr;
   delete xyz_ptr;
}
