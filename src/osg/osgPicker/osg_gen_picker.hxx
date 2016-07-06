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

template<class Gen> GenPicker<Gen>::GenPicker()
{
	_iv=NULL;
	_nmask=0xffffffff;
}

template<class Gen> GenPicker<Gen>::~GenPicker()
{
}


template<class Gen> bool GenPicker<Gen>::pick_gen(
   float rx,float ry,const osg::Matrix& viewMat,const osg::Matrix& projMat,
   osg::Node* node,typename GenIntersectVisitor<Gen>::HitList& hits,
   float rsx,float rsy)
{
   {
      std::cout << "inside GenPicker<Gen>::pick_gen()"
                << std::endl;
      std::cout << "_iv = " << _iv << std::endl;
      
// _iv must be instantiated (and deleted) in the derived function, in
// pick()

      DBG_PTR(_iv);

      osg::Matrixd vum;
      vum.set(viewMat*projMat);

      std::cout << "inside GenPicker<Gen>::before setTraversalMask()"
                << std::endl;

      _iv->setTraversalMask(_nmask);

      std::cout << "inside GenPicker<Gen>::before create_pick_obj()"
                << std::endl;
      std::cout << "rx = " << rx << " ry = " << ry
                << " rsx = " << rsx << " rsy = " << rsy << std::endl;
      
      Gen* gen=create_pick_obj(viewMat,projMat,rx,ry,rsx,rsy);

//	typename GenPickVisitor<Gen>::IVHitList localHits;
//	localHits = _iv->getHits(node, vum, gen);
//	if (localHits.empty()) return false;
//	hits.insert(hits.begin(),localHits.begin(),localHits.end());

      std::cout << "node = " << node << std::endl;
      std::cout << "gen = " << gen << std::endl;

      std::cout << "vum = " << std::endl;
      for (int row=0; row<4; row++)
      {
         for (int column=0; column<4; column++)
         {
            std::cout << vum(row,column) << "\t";
         }
         std::cout << std::endl;
      }
      std::cout << std::endl;

      std::cout << "inside GenPicker<Gen>::before getHits()"
                << std::endl;

      hits = _iv->getHits(node, vum, gen);

      std::cout << "Before gen->unref()" << std::endl;

      gen->unref();

      std::cout << "Before checking whether hits.empty()" << std::endl;
      
      if (hits.empty()) return false;
      //DBG_MSG(IGS_VDB,IGS_HIGH,"Sort picked Hits start");

      std::cout << "Before sort()" << std::endl;
      std::sort(hits.begin(),hits.end());
      //DBG_MSG(IGS_VDB,IGS_HIGH,"Sort picked Hits end");

      std::cout << "Before returning true" << std::endl;
      
      return true;
   }

      std::cout << "Before returning false" << std::endl;

   return false;
}


