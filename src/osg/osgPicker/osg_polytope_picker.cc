/* -*-c++-*- osgIntersect - Copyright (C) 2005 IGEOSS (www.igeoss.com)
 *                                             frederic.marmond@igeoss.com
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

#include <iostream>
#include <osg/Projection>
#include "osg/osgPicker/osg_polytope_picker.h"
#include "osg/osgPicker/osg_polytope_pickvisitor.h"

using namespace osgIsect;
using std::cout;
using std::endl;

PolytopePicker::PolytopePicker()
{
   _iv=NULL;
}

bool PolytopePicker::pick(
   float rx,float ry,const osg::Matrix& viewMat,const osg::Matrix& projMat,
   osg::Node* node,GenIntersectVisitor<RefPolytopeIsector>::HitList& hits,
   float rsx,float rsy)
{
   cout << "inside PolytopePicker::pick()" << endl;
   _iv=new PolytopePickVisitor();

   cout << "Before call to pick_gen()" << endl;
   
   bool rc=pick_gen(rx,ry,viewMat,projMat,node,hits,rsx,rsy);

   cout << "Before call to delete _iv" << endl;
   
   delete _iv;
   _iv=NULL;
   cout << "at end of PolytopePicker::pick()" << endl << endl;
   return rc;
}


RefPolytopeIsector* PolytopePicker::create_pick_obj(
   const osg::Matrix& viewMat,const osg::Matrix& projMat,
   float rx,float ry,float rsx,float rsy)
{

   //keep nbPlans=4, or else, update all the following code!
   const int nbPlans=4;

   _iv->setxy(rx,ry);

   Vec3	eye,center,up,forward,right;
   viewMat.getLookAt(eye,center,up);
   forward=center-eye;
   forward.normalize();
   up.normalize();
   right=forward^up;
   double	zNear,zFar;
   double	fovy,aspectRatio=1.0;
   double	mleft,mright,mbottom,mtop;

   float 	fw_far;
   float 	fw_near;
   Vec3	forward_far;
   Vec3	forward_near;

   Vec3	up_far;
   Vec3	up_near;
   Vec3	right_far;
   Vec3	right_near;
   Vec3	scrz;
   Vec3	eyez;

   float 	tgt_far,tgt_near;

   if (projMat.getPerspective(fovy,aspectRatio,zNear,zFar))
   {
      zNear=1.0;
      fw_far=zFar;
      fw_near=zNear;
      forward_far=forward*fw_far;
      forward_near=forward*fw_near;

      tgt_far=fw_far*tanf(fovy/2.0/180.0*osg::PI);
      tgt_near=fw_near*tanf(fovy/2.0/180.0*osg::PI);
      up_far=up*tgt_far;
      up_near=up*tgt_near;
      right_far=right*tgt_far;
      right_near=right*tgt_near;

      scrz=eye+forward_far+up_far*ry+right_far*aspectRatio*rx;
      eyez=eye+forward_near+up_near*ry+right_near*aspectRatio*rx;

   }
   else
      if (projMat.getOrtho(mleft,mright,mbottom,mtop,zNear,zFar))
      {
         zNear=1.0;
         fw_far=zFar;
         fw_near=zNear;
         forward_far=forward*fw_far;
         forward_near=forward*fw_near;

         tgt_far=-(mbottom-mtop)*0.5;
         tgt_near=tgt_far;//(mtop-mbottom);


         up_far=up*tgt_far;
         up_near=up*tgt_near;
         right_far=right*tgt_far;
         right_near=right*tgt_near;



         aspectRatio=(mleft-mright)/(mbottom-mtop);
         scrz=eye+forward_far+up_far*ry+right_far*aspectRatio*rx;
         eyez=eye+forward_near+up_near*ry+right_near*aspectRatio*rx;
         eye=eye+up_near*ry+right_near*aspectRatio*rx;


      }
      else
      {
         DBG_ERR("Not a projection, neither a ortho matrix...");
      }

   right_far*=aspectRatio;
   right_near*=aspectRatio;

   Vec3	far_points[nbPlans]={scrz+up_far*rsy/2.0+right_far*rsx/2.0,
                             scrz+up_far*rsy/2.0-right_far*rsx/2.0,
                             scrz-up_far*rsy/2.0-right_far*rsx/2.0,
                             scrz-up_far*rsy/2.0+right_far*rsx/2.0
   };
   Vec3	near_points[nbPlans]={	eyez+up_near*rsy/2.0+right_near*rsx/2.0,
                                eyez+up_near*rsy/2.0-right_near*rsx/2.0,
                                eyez-up_near*rsy/2.0-right_near*rsx/2.0,
                                eyez-up_near*rsy/2.0+right_near*rsx/2.0
   };


   RefPolytope::PlaneList pl;
   RefPolytope::VertexList vl;

   for (int i=0;i<nbPlans;i++)
   {
      pl.push_back(Plane(near_points[i],far_points[(i+1)&0x3],far_points[(i)&0x3]));

      //the quad for drawing that plan
      vl.push_back(near_points[(i+1)&0x3]);
      vl.push_back(far_points[(i+1)&0x3]);
      vl.push_back(far_points[(i)&0x3]);
      vl.push_back(near_points[(i)&0x3]);
   }

   Vec3	fwd(0.0,0.0,0.0);
   for (int i=0;i<nbPlans;i++)
      fwd+=far_points[i];
   fwd/=(float)nbPlans;


   RefPolytopeIsector* polyt=new RefPolytopeIsector(pl);
   polyt->setReferenceVertexList(vl);
   polyt->setEye(eye);
   polyt->setForward(fwd);
//	DBG_MSG(IGS_TRC,IGS_VLOW,"Eye="<<polyt->getEye());
   return polyt;



}
