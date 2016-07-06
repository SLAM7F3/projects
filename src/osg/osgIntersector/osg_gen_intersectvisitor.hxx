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

#include <osg/Transform>
#include <osg/Geode>
#include <osg/LOD>
#include <osg/Billboard>
#include <osg/Notify>

#include <osg/Geometry>
#include <osg/Drawable>

#include <float.h>
#include <algorithm>
#include <map>

#include "osg/osgIntersector/osg_gen_index_functor.h"
#include "osg/osgIntersector/osg_gen_intersect.h"


using namespace osg;
using namespace osgIsect;





template<class Gen> GenIntersectVisitor<Gen>::GenIntersectState::GenIntersectState()
{
    _genMaskStack.push_back(0xffffffff);
}


template<class Gen> GenIntersectVisitor<Gen>::GenIntersectState::~GenIntersectState()
{
}


template<class Gen> bool GenIntersectVisitor<Gen>::GenIntersectState::isCulled(const BoundingSphere& bs,GenMask& genMaskOut)
{
    bool hit = false;
    GenMask mask = 0x00000001;
    genMaskOut = 0x00000000;
    GenMask genMaskIn = _genMaskStack.back();
    //    notify(INFO) << << "GenIntersectState::isCulled() mask in "<<genMaskIn<<"  ";
    for(typename GenIntersectState::GenList::iterator sitr=_genList.begin();
        sitr!=_genList.end();
        ++sitr)
    {
        if ((genMaskIn & mask) && (sitr->second)->intersect(bs))
        {
            //            notify(INFO) << << "Hit ";
            genMaskOut = genMaskOut| mask;
            hit = true;
        }
        mask = mask << 1;
    }
    //    notify(INFO) << << "mask = "<<genMaskOut<< std::endl;
    return !hit;
}


template<class Gen> bool GenIntersectVisitor<Gen>::GenIntersectState::isCulled(const BoundingBox& bb,GenMask& genMaskOut)
{
    bool hit = false;
    GenMask mask = 0x00000001;
    genMaskOut = 0x00000000;
    GenMask genMaskIn = _genMaskStack.back();
    for(typename GenIntersectState::GenList::iterator sitr=_genList.begin();
        sitr!=_genList.end();
        ++sitr)
    {
        if ((genMaskIn & mask) && (sitr->second)->intersect(bb))
        {
            genMaskOut = genMaskOut| mask;
            hit = true;
        }
        mask = mask << 1;
    }
    return !hit;
}

template<class Gen> GenIntersectVisitor<Gen>::GenIntersectVisitor()
{
    // overide the default node visitor mode.
    setTraversalMode(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);

    reset();
}


template<class Gen> GenIntersectVisitor<Gen>::~GenIntersectVisitor()
{
}


template<class Gen> void GenIntersectVisitor<Gen>::reset()
{
    _intersectStateStack.clear();

    // create a empty IntersectState on the the intersectStateStack.
    _intersectStateStack.push_back(new GenIntersectState);

    _nodePath.clear();
    _genHitList.clear();

}


template<class Gen> bool GenIntersectVisitor<Gen>::hits()
{
    for(typename HitListMap::iterator itr = _genHitList.begin();
        itr != _genHitList.end();
        ++itr)
    {
        if (!(itr->second.empty())) return true;
    }
    return false;
}


template<class Gen> void GenIntersectVisitor<Gen>::addGen(Gen* gen)
{
    if (!gen) return;

    if (!gen->valid())
    {
        DBG_WAR("Warning: invalid Gen passed to GenIntersectVisitor::addGen(..)");
        //DBG_WAR"         "<<seg->start()<<" "<<seg->end()<<" gen ignored..");
        return;
    }

    // first check to see if segment has already been added.
    for(typename GenIntersectVisitor<Gen>::HitListMap::iterator itr = _genHitList.begin();
        itr != _genHitList.end();
        ++itr)
    {
        if (itr->first == gen) return;
    }

    // create a new Gen transformed to local coordintes.
    GenIntersectState* cis = _intersectStateStack.back().get();
    Gen* ns = new Gen;

    if (cis->_inverse.valid()) ns->mult(*gen,*(cis->_inverse));
    else *ns = *gen;

    cis->addPair(gen,ns);

}


template<class Gen> void GenIntersectVisitor<Gen>::pushMatrix(const Matrix& matrix)
{
    GenIntersectState* nis = new GenIntersectState;

    GenIntersectState* cis = _intersectStateStack.back().get();

    if (cis->_matrix.valid())
    {
        nis->_matrix = new RefMatrix;
        nis->_matrix->mult(matrix,*(cis->_matrix));
    }
    else
    {
        nis->_matrix = new RefMatrix(matrix);
    }

    RefMatrix* inverse_world = new RefMatrix;
    inverse_world->invert(*(nis->_matrix));
    nis->_inverse = inverse_world;

    typename GenIntersectState::GenMask genMaskIn = cis->_genMaskStack.back();
    typename GenIntersectState::GenMask mask = 0x00000001;
    for(typename GenIntersectState::GenList::iterator sitr=cis->_genList.begin();
        sitr!=cis->_genList.end();
        ++sitr)
    {
        if ((genMaskIn & mask))
        {
            Gen* gen = new Gen;
            gen->mult(*(sitr->first),*inverse_world);
            nis->addPair(sitr->first.get(),gen);
        }
        mask = mask << 1;
    }

    _intersectStateStack.push_back(nis);
}


template<class Gen> void GenIntersectVisitor<Gen>::popMatrix()
{
    if (!_intersectStateStack.empty())
    {
        _intersectStateStack.pop_back();
    }
}


template<class Gen> bool GenIntersectVisitor<Gen>::enterNode(Node& node)
{
    const BoundingSphere& bs = node.getBound();
    if (bs.valid())
    {
        GenIntersectState* cis = _intersectStateStack.back().get();
        typename GenIntersectState::GenMask sm=0xffffffff;
        if (cis->isCulled(bs,sm)) return false;
        cis->_genMaskStack.push_back(sm);
        _nodePath.push_back(&node);
        return true;
    }
    else
    {
        return false;
    }
}


template<class Gen> void GenIntersectVisitor<Gen>::leaveNode()
{
    GenIntersectState* cis = _intersectStateStack.back().get();
    cis->_genMaskStack.pop_back();
    _nodePath.pop_back();
}


template<class Gen> void GenIntersectVisitor<Gen>::apply(Node& node)
{
    if (!enterNode(node)) return;

    traverse(node);

    leaveNode();
}



template <class Gen> bool GenIntersectVisitor<Gen>::intersect(osg::Drawable& drawable)
{
    bool hitFlag = false;

    GenIntersectState* cis = _intersectStateStack.back().get();

    const BoundingBox& bb = drawable.getBound();


    for( typename GenIntersectState::GenList::iterator sitr=cis->_genList.begin();
        sitr!=cis->_genList.end();
        ++sitr)
    {
        if (sitr->second->intersect(bb))
        {
//		DBG_MSG(IGS_VDB,IGS_VLOW,"hit on BB:"<<(void*)&drawable);


	    GenIndexFunctor<typename Intersect_trait<Gen>::SpecIntersect> ti;

	    ti.set(*sitr->second,drawable.asGeometry()->getVertexArray());
            drawable.accept(ti);
            if (ti._hit)
            {
//		DBG_MSG(IGS_VDB,IGS_VLOW,"hit on triangle:"<<(void*)&drawable);

                osg::Geometry* geometry = drawable.asGeometry();


                for(typename GenGeomIntersect<Gen>::GeomHitList::iterator thitr=ti._thl.begin();
                    thitr!=ti._thl.end();
                    ++thitr)
                {
//			DBG_MSG(IGS_VDB,IGS_VLOW,"hit on triangle: create hits");

                    Hit hit;
                    hit._nodePath = _nodePath;
                    hit._matrix = cis->_matrix;
                    hit._inverse = cis->_inverse;
                    hit._drawable = &drawable;
                    if (_nodePath.empty()) hit._geode = NULL;
                    else hit._geode = dynamic_cast<Geode*>(_nodePath.back());

                    GeomHit& triHit = *thitr;

                    hit._ratio = triHit._ratio;
                    hit._primitiveIndex = triHit._index;
                    hit._originalGen = sitr->first;
                    hit._localGen = sitr->second;

//FIXME: was for lineseg intersection
//                    hit._intersectPoint = sitr->second->start()*(1.0f-hit._ratio)+
//                        sitr->second->end()*hit._ratio;
		    hit._intersectPoint = triHit._ipt;

                    hit._intersectNormal = triHit._normal;


                    if (geometry)
                    {
//			DBG_MSG("hit on triangle: it is a geometry");
                        osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
                        if (vertices)
                        {
//				DBG_MSG("hit on triangle: it is a geometry with vertices ="<<vertices);
                            osg::Vec3* first = &(vertices->front());
                            if (triHit._v1) {hit._vecIndexList.push_back((triHit._v1-first));
//				DBG_MSG("hit on triangle: it is a geometry with vertices 1");
			    }
                            if (triHit._v2) {hit._vecIndexList.push_back((triHit._v2-first));
//				DBG_MSG("hit on triangle: it is a geometry with vertices 2");
			    }

                            if (triHit._v3) {hit._vecIndexList.push_back((triHit._v3-first));
//				DBG_MSG("hit on triangle: it is a geometry with vertices 3");
			    }

                        }
			else
			{
				DBG_MSG("hit on triangle: it is a geometry WITHOUT vertices");
			}
                    }
		    else
		    {
//			DBG_MSG(IGS_VDB,IGS_VLOW,"hit on triangle: it is NOT a geometry");
		    }


                    _genHitList[sitr->first.get()].push_back(hit);

		    //too slow to sort all here
                    //std::sort(_genHitList[sitr->first.get()].begin(),_genHitList[sitr->first.get()].end());

                    hitFlag = true;

                }
            }
        }
    }


    return hitFlag;

}

template<class Gen> void GenIntersectVisitor<Gen>::apply(Geode& geode)
{
    if (!enterNode(geode)) return;

    for(unsigned int i = 0; i < geode.getNumDrawables(); i++ )
    {
        intersect(*geode.getDrawable(i));
    }

    leaveNode();
}


template<class Gen> void GenIntersectVisitor<Gen>::apply(Billboard& node)
{
    if (!enterNode(node)) return;

    leaveNode();
}


template<class Gen> void GenIntersectVisitor<Gen>::apply(Group& node)
{
    if (!enterNode(node)) return;

    traverse(node);

    leaveNode();
}


template<class Gen> void GenIntersectVisitor<Gen>::apply(Transform& node)
{
    if (!enterNode(node)) return;

    osg::ref_ptr<RefMatrix> matrix = new RefMatrix;
    node.computeLocalToWorldMatrix(*matrix,this);

    pushMatrix(*matrix);

    traverse(node);

    popMatrix();

    leaveNode();
}


template<class Gen> void GenIntersectVisitor<Gen>::apply(Switch& node)
{
    apply((Group&)node);
}


template<class Gen> void GenIntersectVisitor<Gen>::apply(LOD& node)
{
    apply((Group&)node);
}
