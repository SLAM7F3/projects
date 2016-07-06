// ==========================================================================
// Osgfuncs namespace method definitions
// ==========================================================================
// Last modified on 3/25/09; 11/27/09; 1/31/10; 4/5/14
// ==========================================================================

#include <iostream>
#include <osg/ApplicationUsage>
#include <osg/ArgumentParser>
#include <osgText/Font>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osgText/Text>
#include <vector>

#include "osg/AbstractOSGCallback.h"
#include "osg/osgGraphicals/AnimationController.h"
#include "math/basic_math.h"
#include "math/genmatrix.h"
#include "osg/ModeController.h"
#include "osg/ModeHUD.h"
#include "osg/osgWindow/MyViewerEventHandler.h"
#include "osg/osgfuncs.h"
#include "general/outputfuncs.h"
#include "math/rotation.h"
#include "math/threevector.h"
#include "osg/osgWindow/WindowManager.h"
#include "osg/ViewFrustum.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

namespace osgfunc
{

// ---------------------------------------------------------------------
// Method create_Mode_HUD creates a Heads-Up-Display (HUD) for
// interaction mode text display and set up its display callback:

   osg::Projection* create_Mode_HUD(
      int ndims,ModeController* ModeController_ptr,
      bool generate_AVI_movie_flag)
   {

// Note added on 10/14/07:  Eliminated dependence of this method on ndims...

      ModeHUD* opModeHUD_ptr=new ModeHUD(
         ModeController_ptr,generate_AVI_movie_flag);
      osg::Projection* mode_projection_ptr = opModeHUD_ptr->getProjection();
      mode_projection_ptr->setUpdateCallback( 
         new AbstractOSGCallback<ModeHUD>( 
            opModeHUD_ptr, &ModeHUD::showMode) );
      return mode_projection_ptr;
   }
   
// ==========================================================================
// Argument parsing methods
// ==========================================================================

// Method parse_arguments uses an osg::ArgumentParser object to manage
// main program arguments:

   void parse_arguments(osg::ArgumentParser& arguments)
   {
      osg::ApplicationUsage* appUsage = arguments.getApplicationUsage();
      appUsage->addCommandLineOption(
         "-h or --help","Display this information");

// If user request help write it out to cout:

      if (arguments.read("-h") || arguments.read("--help"))
      {
         appUsage->write(cout);
      }

// Any option left unread are converted into errors to write out later:

      arguments.reportRemainingOptionsAsUnrecognized();
   
// Report any errors if they have occured when parsing the program
// arguments:

      if (arguments.errors())
      {
         arguments.writeErrorMessages(cout);
      }
   }

// ==========================================================================
// MatrixTransform methods
// ==========================================================================

   osg::Matrix transpose_matrix(const osg::Matrix& matrix) 
   {
      osg::Matrix matrix_transpose;
      for (int row=0; row<4; row++)
      {
         for (int column=0; column<4; column++)
         {
            matrix_transpose(row,column)=
               matrix(column,row);
         }
      }
      return matrix_transpose;
   }

// ---------------------------------------------------------------------
// Method rotation_to_quaternion takes in a 3x3 rotation R where the
// rotated coordinate system's basis vectors are loaded into the
// columns.  It first transposes the input orthogonal matrix in order
// to conform with OpenGL's row based (rather than column based)
// matrix conventions.  It then constructs and returns the OSG
// quaternion corresponding to the input rotation matrix R.

   osg::Quat rotation_to_quaternion(const rotation& R)
   {
      rotation Rtrans=R.transpose();

      osg::Matrix R4;
      for (int i=0; i<3; i++)
      {
         for (int j=0; j<3; j++)
         {
            R4(i,j)=Rtrans.get(i,j);
         }
      }

//   cout << "R4 = " << endl;
//   osgfunc::print_matrix(R4);

      osg::Quat q;
      q.set(R4);
      return q;
   }
   
// ---------------------------------------------------------------------
// Method generate_scale instantiates a MatrixTransform corresponding
// to a uniform scaling of the X, Y and Z axes by input factor s.

   osg::MatrixTransform* generate_scale(double s)
   {
      threevector trans(0,0,0);
      return generate_scale_and_trans(s,trans);
   }

   osg::MatrixTransform* generate_scale(
      double s,const threevector& scale_origin)
   {
      osg::MatrixTransform* Trans1Transform_ptr=
         generate_trans(-scale_origin);
      osg::MatrixTransform* Trans2Transform_ptr=
         generate_trans(scale_origin);

      osg::MatrixTransform* ScaleTransform_ptr=generate_scale(s);

      osg::Matrixd total_transform_mat=Trans1Transform_ptr->getMatrix() * 
         ScaleTransform_ptr->getMatrix() * 
         Trans2Transform_ptr->getMatrix();
      osg::MatrixTransform* TotalTransform_ptr=
         new osg::MatrixTransform(total_transform_mat);
      return TotalTransform_ptr;
   }

   osg::MatrixTransform* generate_trans(const threevector& trans)
   {
      double scale=1;
      return generate_scale_and_trans(scale,trans);
   }
   
// ---------------------------------------------------------------------
// Method generate_scale_and_trans instantiates a MatrixTransform
// corresponding to a uniform scaling of the X, Y and Z axes by input
// factor s and a translation by input threevector trans.

   osg::MatrixTransform* generate_scale_and_trans(
      double s,const threevector& trans)
   {
      osg::Matrix S,T,SplusT;
      S=S.scale(s,s,s);
      T=T.translate(trans.get(0),trans.get(1),trans.get(2));

      for (int i=0; i<4; i++)
      {
         for (int j=0; j<4; j++)
         {
            if (i==j)
            {
               SplusT(i,j)=S(i,j);
            }
            else
            {
               SplusT(i,j)=T(i,j);
            }
//               cout << SplusT(i,j) << " ";
         }
         cout << endl;
      }
   
      osg::MatrixTransform* MatrixTransform_ptr=new 
         osg::MatrixTransform(SplusT);
      return MatrixTransform_ptr;
   }

// ---------------------------------------------------------------------
// Method generate_rot_scale_and_trans instantiates a MatrixTransform
// corresponding to a rotation by orthogonal input matrix R, uniform
// scaling of the X, Y and Z axes by input factor s, and a translation
// by input threevector trans.

   osg::MatrixTransform* generate_rot(const rotation& R)
   {
      threevector trans(0,0,0);
      return generate_rot_scale_and_trans(1,R,trans);
   }

   osg::MatrixTransform* generate_rot(
      const rotation& R,const threevector& rotation_origin)
   {

      osg::MatrixTransform* Trans1Transform_ptr=
         generate_trans(-rotation_origin);
      osg::MatrixTransform* Trans2Transform_ptr=
         generate_trans(rotation_origin);

      threevector trans(0,0,0);
      osg::MatrixTransform* RotTransform_ptr=
         generate_rot_scale_and_trans(1,R,trans);

      osg::Matrixd total_transform_mat=Trans1Transform_ptr->getMatrix() * 
         RotTransform_ptr->getMatrix() * Trans2Transform_ptr->getMatrix();
      osg::MatrixTransform* TotalTransform_ptr=
         new osg::MatrixTransform(total_transform_mat);
      return TotalTransform_ptr;
   }

   osg::MatrixTransform* generate_rot_and_trans(
      const rotation& R,const threevector& trans)
   {
      return generate_rot_scale_and_trans(1,R,trans);
   }

   osg::MatrixTransform* generate_rot_scale_and_trans(
      double s,const rotation& R,const threevector& trans)
   {
//         cout << "inside osgfunc::generate_rot_scale_and_trans()" << endl;
         
      genmatrix SRT(4,4);
      SRT.clear_values();
      for (int i=0; i<3; i++)
      {
         for (int j=0; j<3; j++)
         {
            SRT.put(i,j,s*R.get(i,j));
            SRT.put(i,3,trans.get(i));
         }
      }
      SRT.put(3,3,1);
//         cout << "SRT = " << SRT << endl;

      osg::Matrix SRT_transpose;
      for (int i=0; i<4; i++)
      {
         for (int j=0; j<4; j++)
         {
            SRT_transpose(i,j)=SRT.transpose().get(i,j);
         }
      }

// Note added on Monday, Jan 29, 2007: The following dynamic
// allocation looks pretty dangerous.  Who will take responsibility
// for destroying the MatrixTransform when it's no longer needed?
// Should probably use a reference pointer here...

      osg::MatrixTransform* MatrixTransform_ptr=new osg::MatrixTransform(
         SRT_transpose);

//         cout << "SRT_trans = " << endl;
//         print_matrix(SRT_transpose);
         
      return MatrixTransform_ptr;
   }

// ---------------------------------------------------------------------
// Method generate_rot_scale_and_trans() performs a rotation by matrix
// R about the specified rotation_origin, then scales by input factor
// scale about rotation_origin, and finally translates by input vector
// trans.

   osg::MatrixTransform* generate_rot_scale_and_trans(
      const threevector& rotation_origin,const rotation& R,
      double scale,const threevector& trans)
   {
      osg::MatrixTransform* RotTransform_ptr=generate_rot(
         R,rotation_origin);

      osg::MatrixTransform* ScaleTransform_ptr=generate_scale(
         scale,rotation_origin);

      osg::MatrixTransform* TransTransform_ptr=generate_trans(trans);

      osg::Matrixd total_transform_mat=
         RotTransform_ptr->getMatrix() * 
         ScaleTransform_ptr->getMatrix() * 
         TransTransform_ptr->getMatrix();
      osg::MatrixTransform* TotalTransform_ptr=
         new osg::MatrixTransform(total_transform_mat);

      return TotalTransform_ptr;
   }

// ---------------------------------------------------------------------
   void print_matrix(const osg::Matrix& matrix) 
   {
      cout.precision(10);
      cout << "4x4 matrix = " << endl;
      for (int row=0; row<4; row++)
      {
         for (int column=0; column<4; column++)
         {
            cout << matrix(row,column) << "\t";
         }
         cout << endl;
      }
      cout << endl;
   }

} // osgfunc namespace
