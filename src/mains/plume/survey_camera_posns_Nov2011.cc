// ========================================================================
// Program SURVEY_CAMERA_POSNS contains survey measurements of radii
// and azimuthal angles for the 10 camera tripod locations at
// the EARTH and VENUS sites in South Carolina campaign #1.  It
// exports text file bundler_IO_subdir/measured_camera_posns.dat which
// has XYZ camera locations in the square-pad's coordinate system.

//			 run_survey_camera_posns

// ========================================================================
// Last updated on 1/26/12; 4/23/12; 4/24/12; 1/2/13
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "passes/PassesGroup.h"
#include "general/sysfuncs.h"
#include "astro_geo/latlong2utmfuncs.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::vector;


// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// Use an ArgumentParser object to manage the program arguments:

   osg::ArgumentParser arguments(&argc,argv);
   PassesGroup passes_group(&arguments);
   string image_list_filename=passes_group.get_image_list_filename();
   cout << " image_list_filename = " << image_list_filename << endl;
   string bundler_IO_subdir=filefunc::getdirname(image_list_filename);
   cout << "bundler_IO_subdir = " << bundler_IO_subdir << endl;

   string expt_daynumber,expt_label;
   cout << endl;
   cout << "Enter Nov 2011 experiment day number (e.g. 1,2,3,4,5):" << endl;
   cin >> expt_daynumber;
   cout << "Enter experiment label (e.g. A,B,C)" << endl;
   cin >> expt_label;
   string expt_descriptor=expt_daynumber+expt_label;
   cout << "Experiment descriptor = " << expt_descriptor << endl;

   vector<threevector> reconstructed_camera_posn;

   vector<int> camera_ID;
   camera_ID.push_back(17);
   camera_ID.push_back(18);
   camera_ID.push_back(19);
   camera_ID.push_back(20);
   camera_ID.push_back(21);
   camera_ID.push_back(22);
   camera_ID.push_back(23);
   camera_ID.push_back(24);
   camera_ID.push_back(25);
   camera_ID.push_back(26);
   
   vector<double> Rm;
   vector<int> Adeg,Amin,Asec;  // Angles are measured clockwise 
				// wrt +y_hat (north) axis:

// Day 2 Nov 2012 : VENUS



   if (expt_descriptor=="2C" || expt_descriptor=="4B" ||
       expt_descriptor=="4C")	// 50 meter EARTH
   {
      Rm.push_back(49.792);
      Rm.push_back(50.122);
      Rm.push_back(50.324);
      Rm.push_back(50.446);
      Rm.push_back(50.775);
      Rm.push_back(-1);
      Rm.push_back(50.667);
      Rm.push_back(50.515);
      Rm.push_back(50.359);
      Rm.push_back(50.084);

      Adeg.push_back(352);
      Adeg.push_back(28);
      Adeg.push_back(68);
      Adeg.push_back(98);
      Adeg.push_back(140);
      Adeg.push_back(-1);
      Adeg.push_back(208);
      Adeg.push_back(245);
      Adeg.push_back(278);
      Adeg.push_back(319);
   
      Amin.push_back(27);
      Amin.push_back(22);
      Amin.push_back(57);
      Amin.push_back(22);
      Amin.push_back(6);
      Amin.push_back(-1);
      Amin.push_back(20);
      Amin.push_back(58);
      Amin.push_back(36);
      Amin.push_back(0);

      Asec.push_back(31);
      Asec.push_back(31);
      Asec.push_back(57);
      Asec.push_back(16);
      Asec.push_back(6);
      Asec.push_back(-1);
      Asec.push_back(50);
      Asec.push_back(59);
      Asec.push_back(50);
      Asec.push_back(21);
   }
   else if (expt_descriptor=="none")	// 40 meter EARTH
   {
      Rm.push_back(39.953);
      Rm.push_back(40.136);
      Rm.push_back(40.338);
      Rm.push_back(40.499);
      Rm.push_back(40.669);
      Rm.push_back(40.028);
      Rm.push_back(40.700);
      Rm.push_back(40.502);
      Rm.push_back(40.049);
      Rm.push_back(40.209);
      
      Adeg.push_back(352);
      Adeg.push_back(28);
      Adeg.push_back(69);
      Adeg.push_back(98);
      Adeg.push_back(140);
      Adeg.push_back(172);
      Adeg.push_back(208);
      Adeg.push_back(248);
      Adeg.push_back(278);
      Adeg.push_back(319);

      Amin.push_back(21);
      Amin.push_back(23);
      Amin.push_back(15);
      Amin.push_back(24);
      Amin.push_back(12);
      Amin.push_back(10);
      Amin.push_back(26);
      Amin.push_back(57);
      Amin.push_back(28);
      Amin.push_back(51);

      Asec.push_back(43);
      Asec.push_back(10);
      Asec.push_back(52);
      Asec.push_back(29);
      Asec.push_back(32);
      Asec.push_back(3);
      Asec.push_back(13);
      Asec.push_back(25);
      Asec.push_back(59);
      Asec.push_back(49);
   }
   else if (expt_descriptor=="4E_B")	// 30 meter EARTH
   {
      Rm.push_back(30.138);
      Rm.push_back(30.209);
      Rm.push_back(30.359);
      Rm.push_back(30.490);
      Rm.push_back(30.687);
      Rm.push_back(30.177);
      Rm.push_back(30.670);
      Rm.push_back(30.540);
      Rm.push_back(30.322);
      Rm.push_back(30.019);
      
      Adeg.push_back(352);
      Adeg.push_back(28);
      Adeg.push_back(69);
      Adeg.push_back(98);
      Adeg.push_back(140);
      Adeg.push_back(172);
      Adeg.push_back(208);
      Adeg.push_back(248);
      Adeg.push_back(278);
      Adeg.push_back(319);

      Amin.push_back(24);
      Amin.push_back(21);
      Amin.push_back(35);
      Amin.push_back(28);
      Amin.push_back(24);
      Amin.push_back(14);
      Amin.push_back(35);
      Amin.push_back(45);
      Amin.push_back(37);
      Amin.push_back(53);

      Asec.push_back(27);
      Asec.push_back(37);
      Asec.push_back(34);
      Asec.push_back(19);
      Asec.push_back(45);
      Asec.push_back(34);
      Asec.push_back(14);
      Asec.push_back(21);
      Asec.push_back(50);
      Asec.push_back(52);
   }

// -------------------------------------------------------------------

   else if (expt_descriptor=="4D_A")	// 50 meter VENUS
   {
      Rm.push_back(50.100);
      Rm.push_back(50.141);
      Rm.push_back(50.315);
      Rm.push_back(50.498);
      Rm.push_back(50.126);
      Rm.push_back(49.847);
      Rm.push_back(50.074);
      Rm.push_back(49.873);
      Rm.push_back(50.072);
      Rm.push_back(50.261);
      
      Adeg.push_back(355);
      Adeg.push_back(25);
      Adeg.push_back(63);
      Adeg.push_back(99);
      Adeg.push_back(144);
      Adeg.push_back(175);
      Adeg.push_back(208);
      Adeg.push_back(244);
      Adeg.push_back(284);
      Adeg.push_back(327);

      Amin.push_back(24);
      Amin.push_back(51);
      Amin.push_back(47);
      Amin.push_back(46);
      Amin.push_back(58);
      Amin.push_back(23);
      Amin.push_back(2);
      Amin.push_back(19);
      Amin.push_back(6);
      Amin.push_back(40);

      Asec.push_back(45);
      Asec.push_back(2);
      Asec.push_back(19);
      Asec.push_back(3);
      Asec.push_back(35);
      Asec.push_back(36);
      Asec.push_back(46);
      Asec.push_back(5);
      Asec.push_back(2);
      Asec.push_back(59);
   }
   else if (expt_descriptor=="5A" || expt_descriptor=="5B"
   || expt_descriptor=="5C")	// 40 meter VENUS
   {
      Rm.push_back(40.009);
      Rm.push_back(40.038);
      Rm.push_back(40.253);
      Rm.push_back(40.489);
      Rm.push_back(40.138);
      Rm.push_back(39.934);
      Rm.push_back(40.168);
      Rm.push_back(39.931);
      Rm.push_back(40.056);
      Rm.push_back(40.262);

      Adeg.push_back(355);
      Adeg.push_back(25);
      Adeg.push_back(63);
      Adeg.push_back(99);
      Adeg.push_back(145);
      Adeg.push_back(175);
      Adeg.push_back(207);
      Adeg.push_back(244);
      Adeg.push_back(283);
      Adeg.push_back(327);
   
      Amin.push_back(30);
      Amin.push_back(44);
      Amin.push_back(45);
      Amin.push_back(58);
      Amin.push_back(18);
      Amin.push_back(22);
      Amin.push_back(50);
      Amin.push_back(8);
      Amin.push_back(54);
      Amin.push_back(41);
   
      Asec.push_back(39);
      Asec.push_back(9);
      Asec.push_back(21);
      Asec.push_back(52);
      Asec.push_back(34);
      Asec.push_back(5);
      Asec.push_back(50);
      Asec.push_back(48);
      Asec.push_back(18);
      Asec.push_back(57);
   }
   else if (expt_descriptor=="1A" || expt_descriptor=="1B" || 
   expt_descriptor=="2A" || expt_descriptor=="2B" ||
   expt_descriptor=="4A")	// 30 meter VENUS
   {
      Rm.push_back(30.000);
      Rm.push_back(30.163);
      Rm.push_back(30.298);
      Rm.push_back(30.475);
      Rm.push_back(30.152);
      Rm.push_back(30.054);
      Rm.push_back(30.173);
      Rm.push_back(29.995);
      Rm.push_back(29.984);
      Rm.push_back(30.297);

      Adeg.push_back(355);
      Adeg.push_back(26);
      Adeg.push_back(64);
      Adeg.push_back(100);
      Adeg.push_back(145);
      Adeg.push_back(175);
      Adeg.push_back(207);
      Adeg.push_back(244);
      Adeg.push_back(284);
      Adeg.push_back(328);

      Amin.push_back(36);
      Amin.push_back(13);
      Amin.push_back(29);
      Amin.push_back(5);
      Amin.push_back(37);
      Amin.push_back(39);
      Amin.push_back(47);
      Amin.push_back(11);
      Amin.push_back(1);
      Amin.push_back(5);

      Asec.push_back(41);
      Asec.push_back(32);
      Asec.push_back(27);
      Asec.push_back(31);
      Asec.push_back(24);
      Asec.push_back(42);
      Asec.push_back(9);
      Asec.push_back(16);
      Asec.push_back(19);
      Asec.push_back(53);
   } // expt_descriptor conditional
   
// Z values for cameras = 1.422 meters (approximate actual tripod height) +
// additional z offset adjusted so that 10% value for cumulative Z
// distribution for thresholded_xyz_points.tdp basically equals 0.  Used
// program mains/plume/GROUNDPLANE to find this 10% value.

   double eff_z_camera=1;	// estimate as of 4/24/12

//   cout << "Enter eff_Z_camera value calculated by program GROUNDPLANE:"
//        << endl;
//   cin >> eff_z_camera;

/*
   if (expt_descriptor=="2B")
   {
      eff_z_camera=3.2;	
   }
   else if (expt_descriptor=="5C")
   {
      eff_z_camera=2.1;	
   }
*/

// Export surveyed camera positions to text file to be read in by
// program SURVEYFIT:

   string output_filename=bundler_IO_subdir+"measured_camera_posns.dat";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   vector<threevector> measured_camera_posn;
   for (int i=0; i<camera_ID.size(); i++)
   {
      double r=Rm.at(i);
      double theta=latlongfunc::dms_to_decimal_degs(
         Adeg.at(i),Amin.at(i),Asec.at(i))*PI/180;

// After comparing measured theta angles with reconstructed camera
// posns on 12/16/11, we believe our theta = - measured angle!

      theta *= -1;

      double x=r*cos(theta);
      double y=r*sin(theta);

//      cout << camera_ID.at(i) << "  "
//           << x << "  "
//           << y << "  " << endl;

//      measured_camera_posn.push_back(threevector(x,y,0));
      measured_camera_posn.push_back(threevector(-y,x,0));

// Next line generates output which goes into
// measured_camera_posns.dat

      outstream << "Camera " << i+17 << "   "
                << measured_camera_posn.back().get(0) << "   " 
                << measured_camera_posn.back().get(1) << "   "
                << eff_z_camera << endl;

// Next line generates output for 3D cones marking surveyed tripod
// locations which goes into mains/plume/VIEWTRIPODS:

//      cout << " XYZ.push_back(threevector( " 
//           << measured_camera_posn.back().get(0) << " , " 
//           << measured_camera_posn.back().get(1) << " , " 
//           << eff_z_camera-1.422 << " ));" << endl;

   } // loop over index i labeling cameras

   filefunc::closefile(output_filename,outstream);

   string banner="Wrote surveyed positions for tripod cameras to "+
      output_filename;
   outputfunc::write_big_banner(banner);
}
