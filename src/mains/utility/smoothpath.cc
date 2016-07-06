// ==========================================================================
// Program SMOOTHPATH reads in an OSG animation path which contains
// camera positions and attitudes as functions of time extracted via
// waypoint selection.  Unless the waypoints are chosen with extreme
// care, the animation path tends to be jerky.  So this program
// temporally filters the raw input path in order to smooth the
// camera's motion and yield final movies which are more pleasing to
// the eye.

//			smoothpath

// ==========================================================================
// Last updated on 2/26/06; 9/12/07
// ==========================================================================

#include <iostream>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "math/fourvector.h"
#include "plot/metafile.h"
#include "templates/mytemplates.h"
#include "filter/piecewise_linear.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   std::set_new_handler(sysfunc::out_of_memory);
   
   string anim_path_filename="newlowell.path";
//   string anim_path_filename="low.path";
   cout << "Enter OSG animation path filename:" << endl;
   cin >> anim_path_filename;

   vector<double> time;
   vector<twovector> TX,TY,TZ,TA,TE,TR;

   filefunc::ReadInfile(anim_path_filename);

   double curr_az,curr_el,curr_roll;
   double prev_az = 0, prev_el = 0, prev_roll = 0;
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> text_line_value=stringfunc::string_to_numbers(
         filefunc::text_line[i]);

      double curr_time=text_line_value[0];
      time.push_back(curr_time);

      threevector curr_posn(
         text_line_value[1],text_line_value[2],text_line_value[3]);
      TX.push_back(twovector(curr_time,curr_posn.get(0)));
      TY.push_back(twovector(curr_time,curr_posn.get(1)));
      TZ.push_back(twovector(curr_time,curr_posn.get(2)));

      fourvector curr_Q(text_line_value[4],text_line_value[5],
                        text_line_value[6],text_line_value[7]);
      mathfunc::az_el_roll_corresponding_to_quaternion(
         curr_Q,curr_az,curr_el,curr_roll);
      if (i > 0)
      {
         curr_az=basic_math::phase_to_canonical_interval(
            curr_az,prev_az-PI,prev_az+PI);
         curr_el=basic_math::phase_to_canonical_interval(
            curr_el,prev_el-PI,prev_el+PI);
         curr_roll=basic_math::phase_to_canonical_interval(
            curr_roll,prev_roll-PI,prev_roll+PI);
      }
      prev_az=curr_az;
      prev_el=curr_el;
      prev_roll=curr_roll;

      TA.push_back(twovector(curr_time,curr_az));
      TE.push_back(twovector(curr_time,curr_el));
      TR.push_back(twovector(curr_time,curr_roll));
      
      cout << "t = " << curr_time << endl;
      cout << "X = " << TX.back()
           << " Y = " << TY.back() 
           << " Z = " << TZ.back() << endl;
      cout << " az = " << curr_az*180/PI 
           << " el = " << curr_el*180/PI 
           << " rl = " << curr_roll*180/PI << endl;
   }

   const double t_start=time[0];
   const double t_stop=time.back();
//   const double dt=0.1;
   const double dt=0.25;
//   const double dt=0.5;
   int nbins=(t_stop-t_start)/dt+1;
   vector<double> t_reg;
   for (int n=0; n<nbins; n++)
   {
      t_reg.push_back(t_start+n*dt);
   }

   piecewise_linear PL_X(TX);
   piecewise_linear PL_Y(TY);
   piecewise_linear PL_Z(TZ);
   piecewise_linear PL_az(TA);
   piecewise_linear PL_el(TE);
   piecewise_linear PL_roll(TR);
   
//   double sigma=0.25;	// sec
   double sigma=0.5;	// sec
   PL_X.filter_values(dt,sigma);
   PL_Y.filter_values(dt,sigma);
   PL_Z.filter_values(dt,sigma);
   PL_az.filter_values(dt,sigma);
   PL_el.filter_values(dt,sigma);
   PL_roll.filter_values(dt,sigma);
   
   vector<fourvector> interp_Q;
   for (int n=0; n<nbins; n++)
   {
      double filtered_az=PL_az.get_filtered_value(n);
      double filtered_el=PL_el.get_filtered_value(n);
      double filtered_roll=PL_roll.get_filtered_value(n);

      cout.precision(5);
      cout << "n = " << n << " t = " << t_reg[n]
           << " az = " << filtered_az*180/PI
           << " el = " << filtered_el*180/PI
           << " roll = " << filtered_roll*180/PI << endl;

      interp_Q.push_back(mathfunc::quaternion_corresponding_to_az_el_roll(
         filtered_az,filtered_el,filtered_roll));
   }

   string outfilename="interpolated.path";
   ofstream outstream;
   filefunc::openfile(outfilename,outstream);
   
// On 9/12/07, Ross Anderson realized that we didn't have enough
// precision in our output animation paths which causes horrible
// wobbling when the virtual camera is close to the ground.  The
// wobbling is elimiminated provided we write enough digits to the
// output file...

   outstream.precision(12);
   for (int n=0; n<nbins; n++)
   {
      outstream << t_reg[n] << "  " 
                << PL_X.get_filtered_value(n) << "  "
                << PL_Y.get_filtered_value(n) << "  "
                << PL_Z.get_filtered_value(n) << "  "
                << interp_Q[n].get(0) << "  " 
                << interp_Q[n].get(1) << "  " 
                << interp_Q[n].get(2) << "  " 
                << interp_Q[n].get(3) << endl;
   }
   filefunc::closefile(outfilename,outstream);
}
