// ==========================================================================
// Program FILTERPATH
// ==========================================================================
// Last updated on 7/14/06
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "filter/filterfuncs.h"
#include "math/fourvector.h"
#include "plot/metafile.h"
#include "templates/mytemplates.h"
#include "numrec/nr.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
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
   
   string anim_path_filename="mit.path";
//   cout << "Enter OSG animation path filename:" << endl;
//   cin >> anim_path_filename;

   vector<double> time,X;
   vector<threevector> XYZ;
   vector<fourvector> Q;

   filefunc::ReadInfile(anim_path_filename);
   for (unsigned int i=0; i<filefunc::text_line.size(); i++)
   {
      cout << "i = " << i << endl;
      cout << filefunc::text_line[i] << endl;

      double X[8];
      stringfunc::string_to_n_numbers(8,filefunc::text_line[i],X);

//      vector<double> text_line_value=stringfunc::string_to_n_numbers(
//         filefunc::text_line[i]);

//      time.push_back(text_line_value[0]);
      time.push_back(X[0]);
      cout << "t = " << time.back() << endl;
      XYZ.push_back(threevector(X[1],X[2],X[3]));
//      XYZ.push_back(threevector(text_line_value[1],text_line_value[2],
//                                text_line_value[3]));
      cout << "XYZ = " << XYZ.back() << endl;
      Q.push_back(fourvector(X[4],X[5],X[6],X[7]));
      
//      Q.push_back(fourvector(text_line_value[4],text_line_value[5],
//                             text_line_value[6],text_line_value[7]));
      cout << "Q = " << Q.back() << endl;
//      X.push_back(XYZ.back().get(0));
   }

   const double t_start=time[0];
   const double t_stop=time.back();
//   const double dt=0.5;	// sec
   const double dt=1.0;	// sec
   int nbins=(t_stop-t_start)/dt+1;
   vector<double> t_reg,x_reg,y_reg,z_reg,Q0_reg,Q1_reg,Q2_reg,Q3_reg;
   for (int n=0; n<nbins; n++)
   {
      cout << "n = " << n << endl;
      t_reg.push_back(t_start+n*dt);

      int bin_number=mathfunc::mylocate(time,t_reg.back());
      double t_lo=time[bin_number];
      double t_hi=time[bin_number+1];
      threevector XYZ_lo(XYZ[bin_number]);
      threevector XYZ_hi(XYZ[bin_number+1]);
      x_reg.push_back(mathfunc::linefit(
         t_reg.back(),t_lo,XYZ_lo.get(0),t_hi,XYZ_hi.get(0)));
      y_reg.push_back(mathfunc::linefit(
         t_reg.back(),t_lo,XYZ_lo.get(1),t_hi,XYZ_hi.get(1)));
      z_reg.push_back(mathfunc::linefit(
         t_reg.back(),t_lo,XYZ_lo.get(2),t_hi,XYZ_hi.get(2)));

      fourvector Q_lo(Q[bin_number]);
      fourvector Q_hi(Q[bin_number+1]);
      Q0_reg.push_back(mathfunc::linefit(
         t_reg.back(),t_lo,Q_lo.get(0),t_hi,Q_hi.get(0)));
      Q1_reg.push_back(mathfunc::linefit(
         t_reg.back(),t_lo,Q_lo.get(1),t_hi,Q_hi.get(1)));
      Q2_reg.push_back(mathfunc::linefit(
         t_reg.back(),t_lo,Q_lo.get(2),t_hi,Q_hi.get(2)));
      Q3_reg.push_back(mathfunc::linefit(
         t_reg.back(),t_lo,Q_lo.get(3),t_hi,Q_hi.get(3)));
   }

   double sigma=1;	// sec
   cout << "Enter sigma in secs:" << endl;
   cin >> sigma;

   int nhbins=2*4*sigma/dt;
   cout << "nhbins = " << nhbins << endl;
   vector<double> h;
   h.reserve(nhbins);

   filterfunc::gaussian_filter(dt,sigma,h);

   cout << "Gaussian filter h = " << endl;
   templatefunc::printVector(h);
   
   vector<double> x_filter,y_filter,z_filter;
   vector<double> Q0_filter,Q1_filter,Q2_filter,Q3_filter;

   filterfunc::brute_force_filter(x_reg,h,x_filter,false);
   filterfunc::brute_force_filter(y_reg,h,y_filter,false);
   filterfunc::brute_force_filter(z_reg,h,z_filter,false);
   filterfunc::brute_force_filter(Q0_reg,h,Q0_filter,false);
   filterfunc::brute_force_filter(Q1_reg,h,Q1_filter,false);
   filterfunc::brute_force_filter(Q2_reg,h,Q2_filter,false);
   filterfunc::brute_force_filter(Q3_reg,h,Q3_filter,false);
   
   string outfilename="interpolated.path";
   ofstream outstream;
   filefunc::openfile(outfilename,outstream);
   
   for (int n=0; n<nbins; n++)
   {
      outstream << t_reg[n] << "  " 
                << x_filter[n] << "  "
                << y_filter[n] << "  "
                << z_filter[n] << "  "
                << Q0_filter[n] << "  "
                << Q1_filter[n] << "  "
                << Q2_filter[n] << "  "
                << Q3_filter[n] << "  " << endl;
   }
   filefunc::closefile(outfilename,outstream);

}
