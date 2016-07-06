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
#ifndef OSG_GEN_PICKVISITOR_H
#define OSG_GEN_PICKVISITOR_H

#include <iostream>
#include <osg/NodeVisitor>
#include "osg/osgPicker/picker.h"

namespace osgIsect
{

/// PickVisitor traverses whole scene and checks below all Projection
//nodes

// The specialized class must implement: - in constructor: _piv=new
// <Gen>IntersectVisitor();

// for example = new LinesegIntersectVisitor(); - the getHits method

   template<class Gen>
      class GenPickVisitor : public osg::NodeVisitor
      {
         public:

            GenPickVisitor()
            {
               xp=yp=0;
               _piv=new typename 
                  IntersectVisitor_trait<Gen>::SpecIntersectVisitor();
               setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
            }
            ~GenPickVisitor() 
            {
               delete _piv;
            }

            void setTraversalMask(osg::Node::NodeMask traversalMask)
            {
               NodeVisitor::setTraversalMask(traversalMask);
               _piv->setTraversalMask(traversalMask);
            }

            // Aug 2003 added to pass the nodemaskOverride to the
            // osgPickosgIntersectVisitor may be used make the visitor
            // override the nodemask to visit invisible actions

            inline void setNodeMaskOverride(osg::Node::NodeMask mask) 
            {
               _piv->setNodeMaskOverride(mask);
               _nodeMaskOverride = mask; 
            }

            typedef typename 
            IntersectVisitor_trait<Gen>::SpecIntersectVisitor::HitList 
            IVHitList;

/*
  virtual IVHitList& getHits(osg::Node *, const osg::Matrixd &, const Gen* )=0;//{DBG_ERR(IGS_VDB,IGS_VLOW,"getHits have no sens in the generic class");};
*/

            virtual IVHitList& getHits(
               osg::Node* node, const osg::Matrixd&, const Gen* gen)
            {
               std::cout << "inside getHits()" << std::endl;
            
   
               // first get the standard hits in un-projected nodes
               _PIVgenHitList=
                  ((typename 
                    IntersectVisitor_trait<Gen>::SpecIntersectVisitor*)_piv)->
                  getIntersections(node,gen); // fill hitlist

               std::cout << "Before call to traverse(*node)" << std::endl;

               // then get hits in projection nodes
               traverse(*node); // check for projection nodes

               return _PIVgenHitList;
            };


            inline void setxy(float xpt, float ypt) { xp=xpt; yp=ypt; }
            inline bool hits() const { return _PIVgenHitList.size()>0;}

         protected:

            GenIntersectVisitor<Gen>* _piv;
            float xp, yp; // start point in viewport fraction coordiantes
            typename GenIntersectVisitor<Gen>::HitList       _PIVgenHitList;
      };

}
#endif
