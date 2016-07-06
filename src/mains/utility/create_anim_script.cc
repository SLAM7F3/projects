// ========================================================================
// Program CREATE_ANIM_SCRIPT
// ========================================================================
// Last updated on 11/30/05; 8/17/06
// ========================================================================

#include <iostream>
#include <string>
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

// ==========================================================================
int main( int argc, char** argv )
{
   int n_start,n_stop,n_images;
   cout << "Enter starting image number:" << endl;
   cin >> n_start;
   cout << "Enter stopping image number:" << endl;
   cin >> n_stop;
   n_images=n_stop-n_start+1;
//   cout << "Enter number of images:" << endl;
//   cin >> n_images;

   string basefilename;
   cout << "Enter base filename:" << endl;
   cin >> basefilename;
  
//   string imagedir="./spase_fuse/";
   string imagedir="./";
//   string imagedir="./recorded_video/"+basefilename+"/";

   int skip=20;
   int delay=30;
//   int skip=17;
//   int delay=20;
//   string suffix="jpg";
   string suffix="rgb";
//   string suffix="PNG";
//   string suffix="png";

   string scriptfilename="view_total_movie";
   outputfunc::generate_animation_script(
      n_start,n_stop,basefilename,imagedir,scriptfilename,delay,suffix,
      skip);

//   delay=4;
//   delay=10;
   delay=5;
//   skip=2;
   skip=1;

   int max_i=n_images/100;
   cout << "max_i = " << max_i << endl;

   if (max_i==0)
   {
      scriptfilename="view_movie0";
      outputfunc::generate_animation_script(
         n_start,n_stop,basefilename,imagedir,
         scriptfilename,delay,suffix,skip);
   }
   else
   {
      for (int i=0; i<max_i; i++)
      {
         scriptfilename="view_movie"+stringfunc::number_to_string(i);  
         outputfunc::generate_animation_script(
            n_start+i*100,n_start+(i+1)*100,basefilename,imagedir,
            scriptfilename,delay,suffix,skip);
      }
      scriptfilename="view_movie"+stringfunc::number_to_string(n_images/100);
      outputfunc::generate_animation_script(
         n_start+100*(n_images/100),n_images,basefilename,imagedir,
         scriptfilename,delay,suffix);
   } // max_i==0 conditional
}
