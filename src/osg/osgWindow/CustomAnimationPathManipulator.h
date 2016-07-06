// ==========================================================================
// Header file for CustomAnimationPathManipulator class 
// ==========================================================================
// Last modified on 2/21/10; 12/21/10; 2/28/11
// ==========================================================================

/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#ifndef OSGGA_CUSTOM_ANIMATION_PATH_MANIPULATOR
#define OSGGA_CUSTOM_ANIMATION_PATH_MANIPULATOR 1

#include <string>
#include <osg/AnimationPath>
#include <osg/Notify>
#include "osg/Custom2DManipulator.h"
#include "osg/Custom3DManipulator.h"
#include "math/threevector.h"

class WindowManager;

// AnimationPathManipulator is a Matrix Manipulator that reads an
// animation path from a file and plays it back.  The file is expected
// to be ascii and a succession of lines with 8 floating point values
// per line.  The succession of values are: time px py pz ax ay az aw
// where: time = elapsed time in seconds from the begining of the
// animation px py pz = World position in catesian coordinates ax ay
// az aw = Orientation (attitude) defined as a quaternion

namespace osgGA {

   class OSGGA_EXPORT CustomAnimationPathManipulator : 
      public Custom3DManipulator
   {
      public:
    
         CustomAnimationPathManipulator( 
            osg::AnimationPath* animationPath,WindowManager* WM_ptr,
            Custom3DManipulator* CM_3D_ptr,const threevector& final_posn);
         CustomAnimationPathManipulator( 
            const std::string& filename,WindowManager* WM_ptr,
            Custom3DManipulator* CM_3D_ptr);
         CustomAnimationPathManipulator( 
            osg::AnimationPath* animationPath,WindowManager* WM_ptr,
            Custom2DManipulator* CM_2D_ptr,const threevector& final_posn);
         CustomAnimationPathManipulator( 
            const std::string& filename,WindowManager* WM_ptr,
            Custom2DManipulator* CM_2D_ptr);
        
         virtual const char* className() const { return "AnimationPath"; }

         void setPrintOutTimingInfo(bool printOutTiminInfo) { 
            _printOutTiminInfo=printOutTiminInfo; }
         bool getPrintOutTimingInfo() const { return _printOutTiminInfo; }
        
         /** set the position of the matrix manipulator using a 4x4 Matrix.*/
         virtual void setByMatrix(const osg::Matrixd& matrix) { 
            _matrix = matrix; }

         /** set the position of the matrix manipulator using a 4x4 Matrix.*/
         virtual void setByInverseMatrix(const osg::Matrixd& matrix) { 
            _matrix.invert(matrix); }

         /** get the position of the manipulator as 4x4 Matrix.*/
         virtual osg::Matrixd getMatrix() const { return _matrix; }

         /** get the position of the manipulator as a inverse matrix
             of the manipulator, typically used as a model view
             matrix.*/
         virtual osg::Matrixd getInverseMatrix() const { 
            return osg::Matrixd::inverse(_matrix); } 

         void setAnimationPath( osg::AnimationPath* animationPath ) { 
            _animationPath=animationPath; }
        
         osg::AnimationPath* getAnimationPath() { 
            return _animationPath.get(); }
        
         const osg::AnimationPath* getAnimationPath() const { 
            return _animationPath.get(); }

         bool valid() const { return _animationPath.valid(); }

         void init(const GUIEventAdapter& ea,GUIActionAdapter& us);

         void home(const GUIEventAdapter& ea,GUIActionAdapter& us);
         void home(double currentTime);

         virtual bool handle(const GUIEventAdapter& ea,GUIActionAdapter& us);

         /** Get the keyboard and mouse usage of this manipulator.*/
         virtual void getUsage(osg::ApplicationUsage& usage) const;

         Custom3DManipulator* get_CM3D_ptr()
            {
               return CM_3D_ptr;
            };

         const Custom3DManipulator* get_CM3D_ptr() const 
            {
               return CM_3D_ptr;
            }

         Custom2DManipulator* get_CM2D_ptr()
            {
               return CM_2D_ptr;
            };

         const Custom2DManipulator* get_CM2D_ptr() const 
            {
               return CM_2D_ptr;
            }

         void set_initial_FOV_u(double FOV_u)
            {
               initial_FOV_u=FOV_u;
            }

         void set_initial_FOV_v(double FOV_v)
            {
               initial_FOV_v=FOV_v;
            }

         void set_final_FOV_u(double FOV_u)
            {
               final_FOV_u=FOV_u;
            }

         void set_final_FOV_v(double FOV_v)
            {
               final_FOV_v=FOV_v;
            }

         double get_frac_path_completed() const
            {
               return frac_path_completed;
            }

         threevector get_eye_world_posn() const;

      protected:

         bool _valid;
         bool _printOutTiminInfo;
         osg::ref_ptr<osg::AnimationPath> _animationPath;
        
         double  _timeOffset;
         double  _timeScale;
         double  _pauseTime;
         bool    _isPaused;
        
         double  _realStartOfTimedPeriod;
         double  _animStartOfTimedPeriod;
         int     _numOfFramesSinceStartOfTimedPeriod;
         osg::Matrixd _matrix;

         void handleFrame( double time );

     private:

         double frac_path_completed;
         double initial_FOV_u,initial_FOV_v,final_FOV_u,final_FOV_v;
         threevector final_posn;
         Custom2DManipulator* CM_2D_ptr;
         Custom3DManipulator* CM_3D_ptr;

         void allocate_member_objects();
         void initialize_member_objects();
         void reset_CM_posn_and_orientation(bool final_frame_flag);
         void update_virtual_camera_FOV();
         int get_ndims() const;
   };
} // osgGA namespace


#endif
