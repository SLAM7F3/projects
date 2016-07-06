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
#ifndef OSG_GEN_PICKER_H
#define OSG_GEN_PICKER_H

#include <iostream>
#include <osgUtil/SceneView>
#include "osg/osgPicker/picker.h"
#include "osg/osgPicker/osg_gen_pickvisitor.h"
#include "osg/osgIntersector/osg_gen_intersectvisitor.h"


namespace osgIsect
{

///Generic picker
///specialized class must implement:
///pick() (same as pick_gen) and in it:
///_iv=new <GEN>PickVisitor(); //for exemple: _iv=new LineSegPickVisitor();

   template<class Gen>
      class GenPicker
      {
         public:
            GenPicker();
            virtual ~GenPicker();

            ///get the picked object list
            ///rx and ry are window coordinate in normalized environment i.e. window size is (-1,-1)(1,1)
            ///scene is what we want to pick (basicly a subgraph and a camera)
            ///hits is the list of drawable picked (sorted from nearest to farest
            ///WARNING: you must instanciate _iv before calling pick_gen, and delete it after!
            virtual bool pick(float rx,float ry,const osg::Matrix& viewMat,const osg::Matrix& projMat,osg::Node* node,typename GenIntersectVisitor<Gen>::HitList& hits,float rsx,float rsy)=0;


            void setTraversalMask(
               Node::NodeMask mask){_nmask=mask;
               if (_iv) _iv->setTraversalMask(_nmask);}

         protected:
            ///really do the pick() job
            ///rx,ry,rs = normalized screen coordinates and pixel size
            bool pick_gen(float rx,float ry,const osg::Matrix& viewMat,
                          const osg::Matrix& projMat,osg::Node* node,
                          typename GenIntersectVisitor<Gen>::HitList& hits,
                          float rsx,float rsy);


            ///here, we create the object that will intersect the world
            ///must be implemented in the specialized class (for LineSeg, polytope, ...)
            virtual Gen* create_pick_obj(
               const osg::Matrix& viewMat,const osg::Matrix& projMat,
               float rx,float ry,float rsx,float rsy)=0;

            GenPickVisitor<Gen>* 	_iv;
            Node::NodeMask		_nmask;
      };

}
#include "osg/osgPicker/osg_gen_picker.hxx"
#endif
