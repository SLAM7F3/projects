// ==========================================================================
// Camerafuncs namespace method definitions
// ==========================================================================
// Last modified on 3/24/13; 4/24/13; 7/6/13; 4/3/14
// ==========================================================================

#include <iostream>
#include <vector>
#include "video/camera.h"
#include "video/camerafuncs.h"
#include "general/filefuncs.h"
#include "structmotion/fundamental.h"
#include "math/genmatrix.h"
#include "math/mathfuncs.h"
#include "numrec/nrfuncs.h"
#include "numerical/param_range.h"
#include "math/rotation.h"
#include "video/texture_rectangle.h"
#include "math/threevector.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::vector;

namespace camerafunc
{

// Method f_and_aspect_ratio_from_horiz_vert_FOVs() returns f < 0
// and aspect ratio Nu/Nv given FOV_u and FOV_v measured in radians.

   void f_and_aspect_ratio_from_horiz_vert_FOVs(
      double FOV_u,double FOV_v,double& f,double& aspect_ratio)
   {
//   cout << "inside camerafunc::f_and_aspect_ratio_from_horiz_vert_FOVs()" << endl;
//      cout << "FOV_u = " << FOV_u*180/PI << endl;
//      cout << "FOV_v = " << FOV_v*180/PI << endl;

      f=-0.5/tan(0.5*FOV_v);
      aspect_ratio=tan(0.5*FOV_u)/tan(0.5*FOV_v);

//      cout << "f = " << f << endl;
//      cout << "Nu/Nv = " << aspect_ratio << endl;
   }

// --------------------------------------------------------------------------
   double aspect_ratio_from_horiz_vert_FOVs(double FOV_u,double FOV_v)
   {
//   cout << "inside camerafunc::aspect_ratio_from_horiz_vert_FOVs()" << endl;
//      cout << "FOV_u = " << FOV_u*180/PI << endl;
//      cout << "FOV_v = " << FOV_v*180/PI << endl;

      double aspect_ratio=tan(0.5*FOV_u)/tan(0.5*FOV_v);
      return aspect_ratio;
//      cout << "f = " << f << endl;
//      cout << "Nu/Nv = " << aspect_ratio << endl;
   }

// --------------------------------------------------------------------------
// Method vert_FOV_from_horiz_FOV_and_aspect_ratio() takes in
// horizontal FOV_u measured in radians along with aspect ratio Nu/Nv.
// It returns the vertical field-of-view FOV_v in radians.

   double vert_FOV_from_horiz_FOV_and_aspect_ratio(
      double FOV_u,double aspect_ratio)
   {
      double term1=tan(0.5*FOV_u)/aspect_ratio;
      double FOV_v=2*atan(term1);
      return FOV_v;
   }

// --------------------------------------------------------------------------
   void f_and_vert_FOV_from_horiz_FOV_and_aspect_ratio(
      double FOV_u,double aspect_ratio,double& f,double& FOV_v)
   {
      FOV_v=vert_FOV_from_horiz_FOV_and_aspect_ratio(FOV_u,aspect_ratio);
      f_and_aspect_ratio_from_horiz_vert_FOVs(FOV_u,FOV_v,f,aspect_ratio);
   }

// --------------------------------------------------------------------------
// Method horiz_vert_FOVs_from_f_and_aspect_ratio() takes in f < 0 and
// aspect ratio Nu/Nv.  It returns the corresponding horizontal and
// vertical fields-of-view FOV_u and FOV_v measured in radians.

   void horiz_vert_FOVs_from_f_and_aspect_ratio(
      double f,double aspect_ratio,double& FOV_u,double& FOV_v)
   {
//   cout << "inside camerafunc::horiz_vert_FOVs_from_f_and_aspect_ratio()" << endl;

      FOV_v=-2*atan(0.5/f);
      FOV_u=2*atan(aspect_ratio*tan(0.5*FOV_v));

//      cout << "FOV_u = " << FOV_u*180/PI << endl;
//      cout << "FOV_v = " << FOV_v*180/PI << endl;
   }

// ==========================================================================
// Non-square pixel methods
// ==========================================================================
   
// Method fv_from_vert_FOV() returns fv < 0 given FOV_v measured in
// radians.

   double fv_from_vert_FOV(double FOV_v)
   {
//   cout << "inside camerafunc::fv_from_vert_FOV()" << endl;
      double fv=-0.5/(tan(0.5*FOV_v));
      return fv;
   }
   
// --------------------------------------------------------------------------
// Method fu_from_horiz_FOV_and_Umax() returns fu < 0 given FOV_u
// measured in radians and Umax=Npu/Npv.

   double fu_from_horiz_FOV_and_Umax(double FOV_u,double Umax)
   {
//   cout << "inside camerafunc::fu_from_horiz_FOV_and_umax()" << endl;
      double fu=-0.5*Umax/(tan(0.5*FOV_u));
      return fu;
   }

// ==========================================================================
// Fitting methods
// ==========================================================================

// Method fit_seven_params()

   void fit_seven_params(
      camera* camera_ptr,double aspect_ratio,
      genmatrix* XYZUV_ptr,genmatrix* XYZABC_ptr)
   {
      cout << "inside camerafunc::fit_seven_params()" << endl;
   
// Recall aspect_ratio=Nu/Nv

      double u0=0.5*aspect_ratio;
      double v0=0.5;
      const double theta=90*PI/180;

      double f_0=camera_ptr->get_fu();
      threevector world_posn_0=camera_ptr->get_world_posn();
      double az_0,el_0,roll_0;
      camera_ptr->get_az_el_roll_from_Rcamera(az_0,el_0,roll_0);

      cout << "f_0 = " << f_0 << endl;
      cout << "world_posn_0 = " << world_posn_0 << endl;
      cout << "az_0 = " << az_0*180/PI << endl;
      cout << "el_0 = " << el_0*180/PI << endl;
      cout << "roll_0 = " << roll_0*180/PI << endl;

      param_range f(f_0-0.3,f_0+0.3,5);
      param_range x(world_posn_0.get(0)-7,world_posn_0.get(0)+7,5);
      param_range y(world_posn_0.get(1)-7,world_posn_0.get(1)+7,5);
      param_range z(world_posn_0.get(2)-7,world_posn_0.get(2)+7,5);
      param_range az(az_0-10*PI/180,az_0+10*PI/180,5);
      param_range el(el_0-5*PI/180,el_0+5*PI/180,5);
      param_range roll(roll_0-0.25*PI/180,roll_0+0.25*PI/180,5);

      double min_score=POSITIVEINFINITY;
      camera candidate_camera;
   
      int n_iters=10;
      for (int iter=0; iter<n_iters; iter++)
      {
         cout << "iter = " << iter+1 << " of " << n_iters << endl;

// Begin while loop over camera parameters

         while (f.prepare_next_value())
         {
            candidate_camera.set_internal_params(
               f.get_value(),f.get_value(),u0,v0,theta);

            while (az.prepare_next_value())
            {
               while (el.prepare_next_value())
               {
                  while (roll.prepare_next_value())
                  {
                     candidate_camera.set_Rcamera(
                        az.get_value(),el.get_value(),roll.get_value());

                     while (x.prepare_next_value())
                     {
                        while (y.prepare_next_value())
                        {
                           while (z.prepare_next_value())
                           {
                              candidate_camera.set_world_posn(
                                 threevector(x.get_value(),y.get_value(),
                                 z.get_value()));

                              candidate_camera.construct_projection_matrix();
                              double chisq_xyzuv=candidate_camera.
                                 fast_check_projection_matrix(XYZUV_ptr);
                              double chisq_xyzabc=candidate_camera.
                                 fast_check_projection_matrix_for_tielines(
                                    XYZABC_ptr);
//                              double score=chisq_xyzuv;
//                              double score=chisq_xyzabc;
//                              double score=chisq_xyzuv*chisq_xyzabc;
                              double score=chisq_xyzuv+chisq_xyzabc;
                              if (score < min_score)
                              {
                                 min_score=score;
                                 f.set_best_value();
                                 az.set_best_value();
                                 el.set_best_value();
                                 roll.set_best_value();
                                 x.set_best_value();
                                 y.set_best_value();
                                 z.set_best_value();
                              }

                           } // loop over z
                        } // loop over y
                     } // loop over x
                  } // loop over roll
               } // loop over el
            } // loop over az
         } // loop over f

// End while loop over camera parameters

         double frac=0.45;
         f.shrink_search_interval(f.get_best_value(),frac);
         az.shrink_search_interval(az.get_best_value(),frac);
         el.shrink_search_interval(el.get_best_value(),frac);
         roll.shrink_search_interval(roll.get_best_value(),frac);
         x.shrink_search_interval(x.get_best_value(),frac);
         y.shrink_search_interval(y.get_best_value(),frac);
         z.shrink_search_interval(z.get_best_value(),frac);

         cout << "Best f value = " << f.get_best_value() << endl;
         cout << "Best x value = " << x.get_best_value() << endl;
         cout << "Best y value = " << y.get_best_value() << endl;
         cout << "Best z value = " << z.get_best_value() << endl;
         cout << "Best az value = " << az.get_best_value()*180/PI << endl;
         cout << "Best el value = " << el.get_best_value()*180/PI << endl;
         cout << "Best roll value = " << roll.get_best_value()*180/PI << endl;
         cout << "min_score = " << min_score << endl << endl;

      } // loop over iter index

// Reset intrinsic and extrinsic parameters for *camera_ptr to best
// fit values.  Then recompute camera's projection matrix.

      camera_ptr->set_internal_params(
         f.get_best_value(),f.get_best_value(),u0,v0);
      camera_ptr->set_Rcamera(az.get_best_value(),el.get_best_value(),
			      roll.get_best_value());
      threevector world_posn(x.get_best_value(),y.get_best_value(),
		             z.get_best_value());
      camera_ptr->set_world_posn(world_posn);
      camera_ptr->construct_projection_matrix();
   }

// ==========================================================================
// Epipolar geometry methods
// ==========================================================================

// Method recover_camera_posn_from_projection_matrix()
// This method overlaps strongly with camera::decompose_projection_matrix()

   threevector recover_camera_posn_from_projection_matrix(genmatrix* P_ptr)
   {
      cout << "inside camerafunc::recover_camera_posn_from_projection_matrix()" << endl;
      cout << "*P_ptr = " << *P_ptr << endl;
   
      threevector p4;
      genmatrix M(3,3),Minv(3,3);

      for (int i=0; i<3; i++)
      {
         for (int j=0; j<3; j++)
         {
            M.put(i,j,P_ptr->get(i,j));               
         }
         p4.put(i,P_ptr->get(i,3));
      }
      
      cout << "M = " << M << endl;
      cout << "M.det = " << M.determinant() << endl;
      cout << "p4 = " << p4 << endl;

      threevector camera_world_posn(
         NEGATIVEINFINITY,NEGATIVEINFINITY,NEGATIVEINFINITY);
      if (M.inverse(Minv))
      {
         cout << "Minv = " << Minv << endl;
         cout << "M*Minv = " << M*Minv << endl;
         camera_world_posn=-Minv*p4;
      }

      cout << "camera_world_posn = " << camera_world_posn << endl;
      return camera_world_posn;
   }

// --------------------------------------------------------------------------
// Method calculate_fundamental_matrix() takes in two cameras whose
// 3x4 projection matrices are assumed to be known.  It returns the
// fundamental matrix corresponding to the cameras' two views.

   genmatrix* calculate_fundamental_matrix(
      camera* cameraprime_ptr,camera* camera_ptr)
   {
      return calculate_fundamental_matrix(
         cameraprime_ptr->get_P_ptr(),
         camera_ptr->get_P_ptr());
   }

// --------------------------------------------------------------------------
// This overloaded version of calculate_fundamental_matrix()
// computes F from two cameras' 3x4 projection matrices.  It implements
// eqn (17.4) in "Multiple View Geometry in Computer Vision" by
// Hartley and Zisserman (2nd edition).  

// Note: This method is essentially identical to
// fundamental::compute_from_projection_matrices().

   genmatrix* calculate_fundamental_matrix(
      genmatrix* P_ptr,genmatrix* Pprime_ptr)
   {
//      cout << "inside calculate_fundamental_matrix()" << endl;

      genmatrix* A_ptr=P_ptr;
      genmatrix* B_ptr=Pprime_ptr;

      genmatrix* F_ptr=new genmatrix(3,3);

      genmatrix M(4,4);

      for (int i=0; i<3; i++)
      {
         for (int j=0; j<3; j++)
         {
            double F_ji=0;
            for (int p=0; p<3; p++)
            {
               for (int q=0; q<3; q++)
               {
                  int eps_ipq=mathfunc::LeviCivita(i,p,q);
                  if (eps_ipq==0) continue;
                  fourvector a_p,a_q;
                  A_ptr->get_row(p,a_p);
                  A_ptr->get_row(q,a_q);

                  M.put_row(0,a_p);
                  M.put_row(1,a_q);
                  
                  for (int r=0; r<3; r++)
                  {
                     for (int s=0; s<3; s++)
                     {
                        int eps_jrs=mathfunc::LeviCivita(j,r,s);
                        if (eps_jrs==0) continue;

                        fourvector b_r,b_s;
                        B_ptr->get_row(r,b_r);
                        B_ptr->get_row(s,b_s);

                        M.put_row(2,b_r);
                        M.put_row(3,b_s);

                        F_ji += eps_ipq*eps_jrs*M.determinant();
                     } // loop over index s
                  } // loop over index r

               } // loop over index q
            } // loop over index p

            F_ptr->put(j,i,F_ji);

         } // loop over index j labeling fundamental matrix columns
      } // loop over index i labeling fundamental matrix rows

      double F_22=F_ptr->get(2,2);
      if (!nearly_equal(F_22,0))
      {
         *F_ptr /= F_22;
      }
      
//      cout << "*F_ptr = " << *F_ptr << endl;
      return F_ptr;
   }

// ==========================================================================
// Radial undistortion methods
// ==========================================================================

// Method radially_undistort_image() implements a minor variant of
// Noah Snavely's RadialUndistort program.  It imports intrinsic
// camera calibration parameters and exports a radially undistorted
// version of the input image within *undistorted_texture_rectangle_ptr.

   void radially_undistort_image(
      unsigned int Npu,unsigned int Npv,double fu_pixels,double fv_pixels,
      double cu_pixels,double cv_pixels,double k2,double k4,
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* undistorted_texture_rectangle_ptr)
   {
//      cout << "inside camerafunc::radially_undistort_image()" << endl;
      
      double f_pixels=0.5*(fu_pixels+fv_pixels);
      radially_undistort_image(
         Npu,Npv,f_pixels,cu_pixels,cv_pixels,k2,k4,
         texture_rectangle_ptr,undistorted_texture_rectangle_ptr);
   }

   void radially_undistort_image(
      unsigned int Npu,unsigned int Npv,double f_pixels,
      double cu_pixels,double cv_pixels,double k2,double k4,
      texture_rectangle* texture_rectangle_ptr,
      texture_rectangle* undistorted_texture_rectangle_ptr)
   {
      double fsqr_pixels_inv=1/sqr(f_pixels);
      double f=-f_pixels/Npv;

      cout << "inside camerafunc::radially_undistort_image() #2" << endl;
//      cout << "cu_pixels = " << cu_pixels
//           << " cv_pixels = " << cv_pixels
//           << " k2 = " << k2
//           << " k4 = " << k4 << endl;
      cout << "f = " << f 
           << " k2 = " << k2
           << " k4 = " << k4 << endl;

// Loop over pixels within undistorted image and compute their
// distorted image pixel counterparts.  Then transfer RGB values from
// distorted to undistorted image:

      int R,G,B;
      for (unsigned int pv=0; pv<Npv; pv++)
      {
         for (unsigned int pu=0; pu<Npu; pu++)
         {
            double pu_trans=pu-cu_pixels;
            double pv_trans=pv-cv_pixels;

            double r_sqr=(sqr(pu_trans)+sqr(pv_trans))*fsqr_pixels_inv;
            double radial_distortion=1+k2*r_sqr+k4*sqr(r_sqr);
         
//            cout << "pu = " << pu << " pv = " << pv
//                 << " radial_distortion = " << radial_distortion << endl;

            pu_trans *= radial_distortion;
            pv_trans *= radial_distortion;
            pu_trans += cu_pixels;
            pv_trans += cv_pixels;

            R=G=B=0;
            if (pu_trans >= 0 && pu_trans < Npu 
            && pv_trans >= 0 && pv_trans < Npv)
            {
               texture_rectangle_ptr->get_pixel_RGB_values(
                  pu_trans,pv_trans,R,G,B);
            }
            undistorted_texture_rectangle_ptr->set_pixel_RGB_values(
               pu,pv,R,G,B);
         } // loop over pu index
      } // loop over pv index
   }

// ==========================================================================
// Image rectification methods
// ==========================================================================

   void rectify_image(
      const texture_rectangle* texture_rectangle_ptr,
      int rectified_width,int rectified_height,homography& H,
      string rectified_image_filename,bool image_to_world_flag)
   {
      rectify_image(
         texture_rectangle_ptr,
         rectified_width,rectified_height,
         0,rectified_width-1,0,rectified_height-1,
         H,rectified_image_filename,image_to_world_flag);
   }

// --------------------------------------------------------------------------
// Method rectify_image() takes in some image within
// *texture_rectangle_ptr which is to be rectified via homography H.
// The new, rectified image's width and height are passed as input
// parameters along with pixel bounding box limits in the warped image
// plane.  Rectify_image exports a warped version of the input image
// to output rectified_image_filename.

   void rectify_image(
      const texture_rectangle* texture_rectangle_ptr,
      int rectified_width,int rectified_height,
      int pu_min,int pu_max,int pv_min,int pv_max,
      homography& H,string rectified_image_filename,
      bool image_to_world_flag)
   {
//      cout << "inside camerafunc::rectify_image()" << endl;

      double width=texture_rectangle_ptr->getWidth();
      double height=texture_rectangle_ptr->getHeight();
      double umax=double(width)/double(height);

      texture_rectangle* rectified_texture_rectangle_ptr=new texture_rectangle(
         rectified_width,rectified_height,1,3,NULL);

      string blank_filename="blank.jpg";
      rectified_texture_rectangle_ptr->generate_blank_image_file(
         rectified_width,rectified_height,blank_filename,0.5);
      rectified_texture_rectangle_ptr->import_photo_from_file(blank_filename);

      double x,y;
      for (int pu=pu_min; pu<pu_max; pu++)
      {
         outputfunc::update_progress_fraction(pu,100,rectified_width);
         double u=double(pu)/(height-1);
         for (int pv=pv_min; pv<pv_max; pv++)
         {
            double v=1-double(pv)/(height-1);
            if (image_to_world_flag)
            {
               H.project_image_plane_to_world_plane(u,v,x,y);
            }
            else
            {
               H.project_world_plane_to_image_plane(u,v,x,y);
            }
            
//            cout.precision(12);
//            cout << "u = " << u << " v = " << v 
//                 << " x = " << x << " y = " << y << endl;

            if (x < 0 || x > umax || y < 0 || y > 1) continue;

            int R,G,B;
            texture_rectangle_ptr->get_RGB_values(x,y,R,G,B);
//         cout << "R = " << R << " G = " << G << " B = " << B << endl;
            rectified_texture_rectangle_ptr->set_pixel_RGB_values(
               pu-pu_min,pv-pv_min,R,G,B);
         } // loop over pv index
      } // loop over pu index
 
      rectified_texture_rectangle_ptr->write_curr_frame(
         rectified_image_filename);
      string banner="Exported "+rectified_image_filename;
      outputfunc::write_banner(banner);

      delete rectified_texture_rectangle_ptr;
   }

// --------------------------------------------------------------------------
// Method orthorectify_image() takes in some image within
// *texture_rectangle_ptr which is to be rectified via homography H.
// The new, rectified image's width and height are passed as input
// parameters along with UTM easting and northing bounding box limits
// in the ground Z-plane.  Orthorectify_image exports a warped version
// of the input image to output orthorectified_image_filename.

   void orthorectify_image(
      const texture_rectangle* texture_rectangle_ptr,
      int rectified_width,int rectified_height,
      double Emin,double Emax,double Nmin,double Nmax,
      homography& H,string orthorectified_image_filename,
      bool world_to_image_flag)
   {
//      cout << "inside camerafunc::orthorectify_image()" << endl;

      string blank_filename="blank.jpg";

      texture_rectangle* rectified_texture_rectangle_ptr=new texture_rectangle(
         rectified_width,rectified_height,1,3,NULL);
      rectified_texture_rectangle_ptr->generate_blank_image_file(
         rectified_width,rectified_height,blank_filename,0.5);
      delete rectified_texture_rectangle_ptr;

      double alpha=1;
      orthorectify_image(
         texture_rectangle_ptr,rectified_width,rectified_height,
         blank_filename,alpha,Emin,Emax,Nmin,Nmax,
         H,orthorectified_image_filename,world_to_image_flag);
   }

   void orthorectify_image(
      const texture_rectangle* texture_rectangle_ptr,
      int rectified_width,int rectified_height,
      string background_image_filename,double alpha,
      double Emin,double Emax,double Nmin,double Nmax,
      homography& H,string orthorectified_image_filename,
      bool world_to_image_flag)
   {
//      cout << "inside camerafunc::orthorectify_image()" << endl;
//      cout << "Emin = " << Emin << " Emax = " << Emax << endl;
//      cout << "Nmin = " << Nmin << " Nmax = " << Nmax << endl;

      texture_rectangle* rectified_texture_rectangle_ptr=new texture_rectangle(
         rectified_width,rectified_height,1,3,NULL);
      rectified_texture_rectangle_ptr->import_photo_from_file(
         background_image_filename);

      double umax=double(rectified_width)/double(rectified_height);
      double umin=0;
      double vmax=1;
      double vmin=0;

      double x,y;
      double xmax=double(texture_rectangle_ptr->getWidth())/
         double(texture_rectangle_ptr->getHeight());
      for (int pu=0; pu<rectified_width; pu++)
      {
         double u=double(pu)/(rectified_height-1);
         double E=Emin+(Emax-Emin)*(u-umin)/(umax-umin);

         for (int pv=0; pv<rectified_height; pv++)
         {
            double v=1-double(pv)/(rectified_height-1);
            double N=Nmin+(Nmax-Nmin)*(v-vmin)/(vmax-vmin);

            if (world_to_image_flag)
            {
               H.project_world_plane_to_image_plane(E,N,x,y);
            }
            else
            {
               H.project_image_plane_to_world_plane(E,N,x,y);
            }
            
//            cout << "pu = " << pu << " pv = " << pv
//                 << " E = " << E << " N = " << N 
//                 <<  "x = " << x << " y = " << y
//                 << endl;

            if (x < 0 || x > xmax || y < 0 || y > 1) continue;

            int Rforeground,Gforeground,Bforeground;
            texture_rectangle_ptr->get_RGB_values(
               x,y,Rforeground,Gforeground,Bforeground);

            int Rbackground,Gbackground,Bbackground;
            rectified_texture_rectangle_ptr->get_pixel_RGB_values(
               pu,pv,Rbackground,Gbackground,Bbackground);

            if (alpha > 1)
            {
               alpha=2-alpha;
            }

            int R=alpha*Rforeground+(1-alpha)*Rbackground;
            int G=alpha*Gforeground+(1-alpha)*Gbackground;
            int B=alpha*Bforeground+(1-alpha)*Bbackground;

//         cout << "R = " << R << " G = " << G << " B = " << B << endl;
            rectified_texture_rectangle_ptr->set_pixel_RGB_values(
               pu,pv,R,G,B);
         } // loop over pv index
      } // loop over pu index
 
      rectified_texture_rectangle_ptr->write_curr_frame(
         orthorectified_image_filename);
      delete rectified_texture_rectangle_ptr;

      string banner="Exported "+orthorectified_image_filename;
      outputfunc::write_banner(banner);
   }

// --------------------------------------------------------------------------
// This overloaded version of method orthorectify_image()

   void orthorectify_image(
      string image_filename,
      double Emin,double Emax,double Nmin,double Nmax,homography& H,
      string rectified_images_subdir,int rectified_width,int rectified_height,
      bool world_to_image_flag)
   {
      cout << "inside camerafunc::orthorectify_image() #2" << endl;
      
      texture_rectangle* texture_rectangle_ptr=new texture_rectangle(
         image_filename,NULL);
      
      string basename=filefunc::getbasename(image_filename);
      string orthorectified_image_filename=rectified_images_subdir+
         "orthorectified_"+basename;

      camerafunc::orthorectify_image(
         texture_rectangle_ptr,
         rectified_width,rectified_height,
         Emin,Emax,Nmin,Nmax,H,orthorectified_image_filename,
         world_to_image_flag);

      delete texture_rectangle_ptr;
   }
   
// ==========================================================================
// Wisp imagery manipulation methods
// ==========================================================================

// Method vcorrect_WISP_image() takes in a raw WISP-360 panorama along
// with parameters A and phi for its sinusoidal horizon fit.  This
// method vertically translates each column of pixels within the input
// panorama by a U-dependent V-offset so that the physical horizon
// appears as a horizontal line in the output image.

   void vcorrect_WISP_image(
      texture_rectangle* input_texture_rectangle_ptr,
      texture_rectangle* output_texture_rectangle_ptr,
      double A,double phi)
   {
      cout << "inside camerafunc::vcorrect_WISP_image()" << endl;

      int width=input_texture_rectangle_ptr->getWidth();
      int height=input_texture_rectangle_ptr->getHeight();
//      cout << "width = " << width << " height = " << height << endl;

      double umax=double(width)/double(height);
      double vmin=0;
      double vmax=1;
//      cout << "umax = " << umax << endl;

      int red,green,blue;
      for (int pu=0; pu<width; pu++)
      {
         if (pu%1000==0) cout << pu << " " << flush;
         double u=double(pu)/(height-1);

         double rotated_u=u;
         double dv=A*sin(2*PI*u/umax+phi);	// v_horizon-v_avg

         for (int pv=0; pv<height; pv++)
         {
            double v=1-double(pv)/(height-1);
            double rotated_v=v+dv;

            if (rotated_v < vmin || rotated_v > vmax) 
            {
               red=green=blue=0;
               output_texture_rectangle_ptr->set_pixel_RGB_values(
                  pu,pv,red,green,blue);
            }
            else
            {
               input_texture_rectangle_ptr->get_RGB_values(
                  rotated_u,rotated_v,red,green,blue);
//               cout << "pu = " << pu << " pv = " << pv
//                    << "   R = " << red 
//                    <<  " G = " << green 
//                    << " B = " << blue
//                    << endl;
               output_texture_rectangle_ptr->set_pixel_RGB_values(
                  pu,pv,red,green,blue);
            }

         } // loop over pv index
      } // loop over pu index
      cout << endl;
   }

// --------------------------------------------------------------------------
// Method sinusoid_func()

   double sinusoid_func(
      double u,int n_harmonic,double A,double phi,double Umax)
   {
      double arg=2*PI*n_harmonic*u/Umax+phi;
      return A*sin(arg);
   }

// --------------------------------------------------------------------------
// Method ucorrect_WISP_image() takes in a previously v-corrected WISP
// panorama generated by program VCORRECT_PANOS.  It also imports
// parameters for a periodic U-coordinate fit that were derived via
// SIFT/ASIFT feature matching between the 0th and some later WISP
// frame stored in *input_texture_rectangle_ptr.  Looping over all
// pixels within an "un-U-warped" version of
// the later panorama frame panorama, this method finds their
// progenitor pixels within the later frame.  Progenitor pixel RGB
// values are transfered to their "un-U-warped" counterparts within
// *output_texture_rectangle_ptr.  The "un-U-warped" image returned
// in *output_texture_rectangle_ptr should (hopefully) closely match
// the 0th WISP panorama.

   void ucorrect_WISP_image(
      texture_rectangle* input_texture_rectangle_ptr,
      texture_rectangle* output_texture_rectangle_ptr,
      double avg_Delta,double v_avg,double beta,double phi)
   {
      cout << "inside camerafunc::ucorrect_WISP_image()" << endl;

      unsigned int width=input_texture_rectangle_ptr->getWidth();
      unsigned int height=input_texture_rectangle_ptr->getHeight();
//      cout << "width = " << width << " height = " << height << endl;

      double umin=0;
      double umax=double(width)/double(height);
//      cout << "umax = " << umax << endl;

      int red,green,blue;
      for (unsigned int px=0; px<width; px++)
      {
         if (px%1000==0) cout << px << " " << flush;
         double x=double(px)/(height-1);
         
         for (unsigned int pv=0; pv<height; pv++)
         {
            double v=1-double(pv)/double(height-1);
            double u=x+avg_Delta
               +beta*(v-v_avg)*cos(2*PI*x/umax+phi);
         
            if (u < umin) u += umax;
            if (u > umax) u -= umax;
            int pu=(height-1)*u;

            input_texture_rectangle_ptr->get_pixel_RGB_values(
               pu,pv,red,green,blue);
            output_texture_rectangle_ptr->set_pixel_RGB_values(
               px,pv,red,green,blue);
         } // loop over pv index
      } // loop over px index
      cout << endl;
   }
   
// --------------------------------------------------------------------------
// Method uv_translate_WISP_image() takes in a WISP image whose
// horizon has previously been straightened.  It also imports U and V
// translation parameters that were derived via SIFT/ASIFT feature
// matching between the 0th and some later WISP frame stored in
// *input_texture_rectangle_ptr.  Looping over all pixels within a "2D
// translated" version of the later panorama, this method finds their
// progenitor pixels within the later frame.  Progenitor pixel RGB
// values are transfered to their "2D translated" counterparts within
// *output_texture_rectangle_ptr. The "2D translated" image returned
// in *output_texture_rectangle_ptr should (hopefully) closely match
// the 0th WISP panorama.

   void uv_translate_WISP_image(
      texture_rectangle* input_texture_rectangle_ptr,
      texture_rectangle* output_texture_rectangle_ptr,
      double avg_Delta_u,double avg_Delta_v)
   {
      cout << "inside camerafunc::uv_translate_WISP_image()" << endl;

      int width=input_texture_rectangle_ptr->getWidth();
      int height=input_texture_rectangle_ptr->getHeight();
//      cout << "width = " << width << " height = " << height << endl;

      double umin=0;
      double umax=double(width)/double(height);
      double vmin=0;
      double vmax=1;
//      cout << "umax = " << umax << endl;

      int red,green,blue;
      for (int px=0; px<width; px++)
      {
         if (px%1000==0) cout << px << " " << flush;
         double x=double(px)/double(height-1);

         double u=x+avg_Delta_u;
         if (u < umin) u += umax;
         if (u > umax) u -= umax;
         int pu=(height-1)*u;

         for (int py=0; py<height; py++)
         {
            double y=1-double(py)/double(height-1);

            double v=y+avg_Delta_v;
            if (v < vmin) v=vmin;
            if (v > vmax) v=vmax;
            int pv=(height-1)*(1-v);

            input_texture_rectangle_ptr->get_pixel_RGB_values(
               pu,pv,red,green,blue);
            output_texture_rectangle_ptr->set_pixel_RGB_values(
               px,py,red,green,blue);
         } // loop over py index
      } // loop over px index
      cout << endl;
   }
   
   
} // camerafunc namespace
