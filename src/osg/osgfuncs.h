// ==========================================================================
// Header file for OSGFUNCS namespace
// ==========================================================================
// Last modified on 10/29/08; 11/27/09; 1/31/10; 4/5/14
// ==========================================================================

#ifndef OSGFUNCS_H
#define OSGFUNCS_H

#include <iostream>
#include <string>
#include <osg/Group>

// class osg::ArgumentParser;
// class osg::Projection;
class AnimationController;
class genmatrix;
class ModeController;
class rotation;
class threevector;
class WindowManager;
class ViewFrustum;

namespace osgfunc
{
   osg::Projection* create_Mode_HUD(
      int ndims,ModeController* ModeController_ptr,
      bool generate_AVI_movie_flag=false);

// Argument parsing methods:

   void parse_arguments(osg::ArgumentParser& arguments);

// MatrixTransform methods:

   osg::Matrix transpose_matrix(const osg::Matrix& matrix);
   osg::Quat rotation_to_quaternion(const rotation& R);

   osg::MatrixTransform* generate_scale(double s);
   osg::MatrixTransform* generate_scale(
      double s,const threevector& scale_origin);
   osg::MatrixTransform* generate_trans(const threevector& trans);
   osg::MatrixTransform* generate_scale_and_trans(
      double s,const threevector& trans);
   osg::MatrixTransform* generate_rot(const rotation& R);
   osg::MatrixTransform* generate_rot(
      const rotation& R,const threevector& rotation_origin);
   osg::MatrixTransform* generate_rot_and_trans(
      const rotation& R,const threevector& trans);
   osg::MatrixTransform* generate_rot_scale_and_trans(
      double s,const rotation& R,const threevector& trans);
   osg::MatrixTransform* generate_rot_scale_and_trans(
      const threevector& rotation_origin,const rotation& R,
      double scale, const threevector& trans);

// Vector & matrix printing methods:

   void print_Vec3(const osg::Vec3& V);
   void print_Vec4(const osg::Vec4& V);
   void print_quaternion(const osg::Quat& q);
   void print_quaternion_matrix(const osg::Quat& q);
   void print_matrix(const osg::Matrix& matrix);

// ==========================================================================
// Inlined methods:
// ==========================================================================

   inline void print_Vec3(const osg::Vec3& V) 
      {
         std::cout << "Vec3 = " << V._v[0] << ","<< V._v[1] << "," << V._v[2] 
                   << std::endl;
      }

   inline void print_Vec4(const osg::Vec4& V) 
      {
         std::cout << "Vec4 = " << V._v[0] << ","<< V._v[1] << "," << V._v[2] 
                   << "," << V._v[3] << std::endl;
      }

   inline void print_quaternion(const osg::Quat& q)
      {
         std::cout << "Quat = " << q._v[0] << ","<< q._v[1] << "," << q._v[2] 
                   << "," << q._v[3] << std::endl;
      }

   inline void print_quaternion_matrix(const osg::Quat& q)
      {
         osg::Matrix rotation_matrix;
         rotation_matrix.set(q);
         print_matrix(rotation_matrix);
      }

} // osgfunc namespace

#endif // osgfuncs.h



