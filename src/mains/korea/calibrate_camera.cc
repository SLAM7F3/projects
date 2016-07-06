// ========================================================================
// Program CALIBRATE_CAMERA imports a set of 2D features that lie
// along line segments within an open source ground photo.  It
// generates and exports homogeneous coefficients for each image plane
// line.  CALIBRATE_CAMERA next works with pairs of 3D points that lie
// along the corresponding world-space lines.  The relationships
// between the 2D line segments and 3D points are stored within STL
// vectors.

// CALIBRATE_CAMERA takes reasonable initial ranges for 7 camera
// calibration parameters.  It minimizes the sum l^T P X where l =
// homogeneous 3-vector for a line segment, P = 3x4 camera calibration
// matrix and X = homogeneous 4-vector for a world-space point.  

// Best-fit values for the 7 camera calibration parameters are
// reported by this program.

// ========================================================================
// Last updated on 8/6/13
// ========================================================================

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "video/camera.h"
#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "math/genmatrix.h"
#include "numerical/param_range.h"
#include "math/rotation.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "math/threevector.h"

// ==========================================================================
int main( int argc, char** argv )
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::ifstream;
   using std::ofstream;
   using std::pair;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);
   
   string calib_subdir="./camera_calib/";
   string features_filename=calib_subdir+"features_2D.txt";
   filefunc::ReadInfile(features_filename);

   cout.precision(10);
   vector<double> weights_2D;
   vector<threevector> uv_pairs;
   for (int i=0; i<filefunc::text_line.size(); i++)
   {
      vector<double> column_values=stringfunc::string_to_numbers(
         filefunc::text_line[i]);
      double u=column_values[3];
      double v=column_values[4];
      uv_pairs.push_back(threevector(u,v,1));
      weights_2D.push_back(column_values[5]);
//      cout << "i = " << i << " uv_pair = " << uv_pairs.back() << endl;
   }

   ofstream outstream;
   outstream.precision(10);
   string output_filename=calib_subdir+"linesegments_2D.dat";
   filefunc::openfile(output_filename,outstream);
   outstream << "# Segment_ID   a                b             c       weight"
             << endl << endl;

// Store 2D line segments within STL vector twoD_linesegments:

   vector<threevector> twoD_linesegments;
   for (int j=0; j<uv_pairs.size(); j += 2)
   {
      threevector curr_l=uv_pairs[j].cross(uv_pairs[j+1]);
      twoD_linesegments.push_back(curr_l);
//      cout << "j = " << j << " curr_l = " << curr_l << endl;
      int segment_ID=j/2;
      outstream << segment_ID << "\t "
                << curr_l.get(0) << "\t "
                << curr_l.get(1) << "\t "
                << curr_l.get(2) << "\t "
                << weights_2D[j] 
                << endl;
   }
   filefunc::closefile(output_filename,outstream);
   string banner="Exported 2D linesegments to "+output_filename;
   outputfunc::write_banner(banner);
   int n_linesegments=twoD_linesegments.size();

// Store 3D world-space corners within STL vector corner_3D:

   vector<threevector> corner_3D;

   double Z_ground=19;
   double fitted_bldg_height=19;	// meters
//   double sketchup_bldg_height=28;
//   cout << "Sketchup bldg height = " << bldg_height << endl;
//   cout << "Enter bldg height estimate:" << endl;
   double bldg_height=fitted_bldg_height;
//   cin >> bldg_height;
 
   double Z_rooftop=Z_ground+bldg_height;

   corner_3D.push_back(
      threevector(738300.4917,4322670.24,Z_rooftop));
   corner_3D.push_back(
      threevector(738365.4,4322657,Z_rooftop));
   corner_3D.push_back(
      threevector(738381.6083,4322736.46,Z_rooftop));
   corner_3D.push_back(
      threevector(738316.7,4322749.7,Z_rooftop));

   corner_3D.push_back(
      threevector(738300.4917,4322670.24,Z_ground));
   corner_3D.push_back(
      threevector(738365.4,4322657,Z_ground));
   corner_3D.push_back(
      threevector(738381.6083,4322736.46,Z_ground));
   corner_3D.push_back(
      threevector(738316.7,4322749.7,Z_ground));

// White street line along main parade route:

   corner_3D.push_back(
      threevector(738238.7,4322641.8,Z_ground));
   corner_3D.push_back(
      threevector(738209.0,4322505.4,Z_ground));

// Dark grey square pattern along parade route:

   corner_3D.push_back(
      threevector(738256.4,4322637.5,Z_ground));
   corner_3D.push_back(
      threevector(738229.8,4322516.0,Z_ground));

   int n_corners=corner_3D.size();

   output_filename=calib_subdir+"points_3D.dat";
   filefunc::openfile(output_filename,outstream);
   outstream << "# Corner_ID  Easting Northing Altitude" << endl << endl;
   
   for (int c=0; c<corner_3D.size(); c++)
   {
      outstream << c << "  "
                << corner_3D[c].get(0) << "   "
                << corner_3D[c].get(1) << "   "
                << corner_3D[c].get(2) << endl;
   }
   filefunc::closefile(output_filename,outstream);   

   banner="Exported 3D points to "+output_filename;
   outputfunc::write_banner(banner);

// Store relationship between 2D line segment and 3D corners within
// STL vector of integer pairs:

   cout << "n_linesegments = " << n_linesegments << endl;
   cout << "n_corners = " << n_corners << endl;

   typedef pair<int,int> DUPLE;
   vector<DUPLE > lines_and_corners;
   lines_and_corners.push_back(DUPLE(0,4));   // line segment 0 (bldg side)
   lines_and_corners.push_back(DUPLE(0,1));   // line segment 1 (bldg roof)
   lines_and_corners.push_back(DUPLE(0,3));   // line segment 2 (bldg roof)
   lines_and_corners.push_back(DUPLE(1,5));   // line segment 3 (bldg side)
   lines_and_corners.push_back(DUPLE(3,7));   // line segment 4 (bldg side)
   lines_and_corners.push_back(DUPLE(8,9));   // line segment 5 (white street)
   lines_and_corners.push_back(DUPLE(4,5));   // line segment 6 (bldg grnd)
   lines_and_corners.push_back(DUPLE(4,7));   // line segment 7 (bldg grnd)
   lines_and_corners.push_back(DUPLE(10,11));   // line segment 8 (grey street)

// Load correspondences between 2D line segments and 3D corners into
// genmatrix *XYZABC_ptr:

   genmatrix* XYZABC_ptr=new genmatrix(2*n_linesegments,6);

   vector<threevector> line_segments;
   vector<fourvector> corner_points;

   for (int l=0; l<lines_and_corners.size(); l++)
   {
      threevector curr_segment=twoD_linesegments[l];
      int corner_index_1=lines_and_corners[l].first;
      int corner_index_2=lines_and_corners[l].second;
      
      threevector corner_1=corner_3D[corner_index_1];
      threevector corner_2=corner_3D[corner_index_2];

      int r=2*l;
      XYZABC_ptr->put(r,0,corner_1.get(0));
      XYZABC_ptr->put(r,1,corner_1.get(1));
      XYZABC_ptr->put(r,2,corner_1.get(2));
      XYZABC_ptr->put(r,3,curr_segment.get(0));
      XYZABC_ptr->put(r,4,curr_segment.get(1));
      XYZABC_ptr->put(r,5,curr_segment.get(2));

      line_segments.push_back(curr_segment);
      corner_points.push_back(fourvector(corner_1,1));

      r=2*l+1;
      XYZABC_ptr->put(r,0,corner_2.get(0));
      XYZABC_ptr->put(r,1,corner_2.get(1));
      XYZABC_ptr->put(r,2,corner_2.get(2));
      XYZABC_ptr->put(r,3,curr_segment.get(0));
      XYZABC_ptr->put(r,4,curr_segment.get(1));
      XYZABC_ptr->put(r,5,curr_segment.get(2));

      line_segments.push_back(curr_segment);
      corner_points.push_back(fourvector(corner_2,1));
      
   } // loop over index l labeling line segments

   cout << "XYZABC = " << *XYZABC_ptr << endl;

// Ground_photo1.jpg:

   int width=1000;
   int height=564;
   double aspect_ratio=double(width)/double(height);
   double u0=0.5*aspect_ratio;
   double v0=0.5;
   camera* camera_ptr=new camera();
   cout << "u0 = " << u0
        << " v0 = " << v0 << endl;

//   genmatrix* XYZUV_ptr=NULL;
//   camera_ptr->compute_tiepoint_projection_matrix(XYZUV_ptr,XYZABC_ptr);

// Perform brute-force search over 7 camera calibration parameters:

   double min_score=POSITIVEINFINITY;
      
   double FOV_start=20*PI/180;
   double FOV_stop=50*PI/180;
   param_range FOV(FOV_start,FOV_stop,9);

   double az_start=35*PI/180;
   double az_stop=65*PI/180;
   param_range az(az_start,az_stop,7);

   double el_start=0*PI/180;
   double el_stop=10*PI/180;
   param_range el(el_start,el_stop,5);

   double roll_start=-2*PI/180;
   double roll_stop=2*PI/180;
   param_range roll(roll_start,roll_stop,3);

//   double X_start=738190;
   double X_start=738200;
   double X_stop=738220;
   param_range X(X_start,X_stop,5);
   double Xmax=738220;

//   double Y_start=4322490;
//   double Y_start=4322520;
   double Y_start=4322535;
   double Y_stop= 4322570;
//   double Y_stop= 4322575;
//   double Y_stop= 4322580;
   param_range Y(Y_start,Y_stop,7);
   double Ymax=4322570;

   double Z_start=20;
   double Z_stop=25;
   param_range Z(Z_start,Z_stop,3);

//   int n_iters=3;
//   int n_iters=5;
//   int n_iters=7;
   int n_iters=8;
//   int n_iters=15;
   double best_FOV,best_az,best_el,best_roll,best_X,best_Y,best_Z;
   double best_f,best_FOV_v;
   double FOV_v,f;
   rotation R;
   for (int iter=0; iter<n_iters; iter++)
   {
      cout << "=========================================================="
           << endl;
      cout << "Iteration = " << iter << " of " << n_iters << endl;

      while (FOV.prepare_next_value())
      {
         double FOV_u=FOV.get_value();
         camerafunc::f_and_vert_FOV_from_horiz_FOV_and_aspect_ratio(
            FOV_u,aspect_ratio,f,FOV_v);

         camera_ptr->set_internal_params(f,f,u0,v0);
//         cout << "Input f = " << f << endl;

         while (az.prepare_next_value())
         {
            double curr_az=az.get_value();
            while (el.prepare_next_value())
            {
               double curr_el=el.get_value();
               while (roll.prepare_next_value())
               {
                  double curr_roll=roll.get_value();
                  camera_ptr->set_Rcamera(curr_az,curr_el,curr_roll);

//                  cout << "Input az = " << curr_az*180/PI
//                       << " input el = " << curr_el*180/PI
//                       << " input roll = " << curr_roll*180/PI
//                       << endl;

                  while (X.prepare_next_value())
                  {
                     double curr_X=X.get_value();
                     while (Y.prepare_next_value())
                     {
                        double curr_Y=Y.get_value();
                        while (Z.prepare_next_value())
                        {
                           double curr_Z=Z.get_value();
                           threevector world_posn(curr_X,curr_Y,curr_Z);
                           camera_ptr->set_world_posn(world_posn);
//                           cout << "Input world posn = " << world_posn
//                                << endl;
                           
                           camera_ptr->
                              construct_seven_param_projection_matrix();
                           genmatrix* P_ptr=camera_ptr->get_P_ptr();
                           
                           double curr_score=0;
                           for (int j=0; j<line_segments.size(); j++)
                           {
                              threevector PC=*P_ptr * corner_points[j];
                              threevector PX=*P_ptr * corner_points[j];
                              double lPX=line_segments[j].dot(PX);
                              curr_score += weights_2D[j]*fabs(lPX);

                           }
//                           cout << "curr_score = " << curr_score << endl;

/*
                           cout << "P = " << *P_ptr << endl;
                           camera_ptr->decompose_projection_matrix();
                           double derived_az,derived_el,derived_roll;
                           camera_ptr->get_az_el_roll_from_Rcamera(
                              derived_az,derived_el,derived_roll);
                           cout << "Derived az = " << derived_az*180/PI
                                << " derived el = " << derived_el*180/PI
                                << " derived roll = " << derived_roll*180/PI
                                << endl;
*/

                           if (curr_score < min_score)
                           {
                              min_score=curr_score;
                              FOV.set_best_value();
                              az.set_best_value();
                              el.set_best_value();
                              roll.set_best_value();
                              X.set_best_value();
                              Y.set_best_value();

                              if (X.get_best_value() > Xmax)
                              {
                                 X.set_best_value(Xmax);
                              }
                              if (Y.get_best_value() > Ymax)
                              {
                                 Y.set_best_value(Ymax);
                              }
                              
                              Z.set_best_value();
                              cout << "min_score = " << min_score << endl;
                           }

                        } // Z while loop
                     } // Y while loop
                  } // X while loop

               } // roll while loop
            } // el while loop
         } // az while loop
      } // FOV while loop

//      double frac=0.55;
//      double frac=0.66;
      double frac=0.71;
      FOV.shrink_search_interval(FOV.get_best_value(),frac);
      az.shrink_search_interval(az.get_best_value(),frac);
      el.shrink_search_interval(el.get_best_value(),frac);
      roll.shrink_search_interval(roll.get_best_value(),frac);
      X.shrink_search_interval(X.get_best_value(),frac);
      Y.shrink_search_interval(Y.get_best_value(),frac);
      Z.shrink_search_interval(Z.get_best_value(),frac);

      best_FOV=FOV.get_best_value();
      best_az=az.get_best_value();
      best_el=el.get_best_value();
      best_roll=roll.get_best_value();
      best_X=X.get_best_value();
      best_Y=Y.get_best_value();
      best_Z=Z.get_best_value();

      camerafunc::f_and_vert_FOV_from_horiz_FOV_and_aspect_ratio(
         best_FOV,aspect_ratio,best_f,best_FOV_v);
   
      cout << endl;
      cout << "best_FOV_u = " << best_FOV*180/PI << endl;
      cout << "best_FOV_v = " << best_FOV_v*180/PI << endl;
      cout << "best_f = " << best_f << endl;
      cout << "best_az = " << best_az*180/PI << endl;
      cout << "best_el = " << best_el*180/PI << endl;
      cout << "best_roll = " << best_roll*180/PI << endl;
      cout << "best_X = " << best_X << endl;
      cout << "best_Y = " << best_Y << endl;
      cout << "best_Z = " << best_Z << endl;

   } // loop over iter index

   delete camera_ptr;
   delete XYZABC_ptr;

// Export package file with calibrated camera parameters:

   string bundler_IO_subdir="./bundler/korea/";
   output_filename=bundler_IO_subdir+"packages/photo_0000.pkg";
   filefunc::openfile(output_filename,outstream);
   outstream << "./bundler/korea/images/ground_photo1.jpg" << endl;
   outstream << "--photo_ID 0" << endl;
   outstream << "--Uaxis_focal_length " << best_f << endl;
   outstream << "--Vaxis_focal_length " << best_f << endl;
   outstream << "--U0 " << u0 << endl;
   outstream << "--V0 " << v0 << endl;
   outstream << "--relative_az " << best_az*180/PI << endl;
   outstream << "--relative_el " << best_el*180/PI << endl;
   outstream << "--relative_roll " << best_roll*180/PI << endl;
   outstream << "--camera_x_posn " << best_X << endl;
   outstream << "--camera_y_posn " << best_Y << endl;
   outstream << "--camera_z_posn " << best_Z << endl;
   outstream << "--frustum_sidelength 25" << endl;
   outstream << "--downrange_distance -1" << endl;
   filefunc::closefile(output_filename,outstream);

   banner="Building height estimate = "+stringfunc::number_to_string(
      bldg_height);
   outputfunc::write_banner(banner);

   banner="Exported calibrated camera package to "+output_filename;
   outputfunc::write_big_banner(banner);
}

/* 
Optimal camera parameters for ground_photo1.jpg:

iter = 14 of 15

min_score = 16.70802079

best_FOV_u = 25.82647169
best_FOV_v = = 14.73610549
best_f = -3.866665778
best_az = 51.54475057
best_el = 4.9405933
best_roll = 1.009442098
best_X = 738199.5018
best_Y = 4322506.073
best_Z = 21.00219236

*/

