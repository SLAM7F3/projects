// ==========================================================================
// Isectfuncs namespace method definitions
// ==========================================================================
// Last modified on 6/28/05
// ==========================================================================

#include <iostream>
#include <string>
#include <osg/BlendFunc>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/LightModel>
#include <osg/LineWidth>
#include <osg/Point>
#include <osg/PolygonOffset>
#include <osg/Vec3>
#include "osg/osgPicker/isectfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

namespace isectfunc
{
   osg::Group* picker_shape=NULL;
   osg::Group* picked_shape=NULL;
   
// --------------------------------------------------------------------------
   void clearPickedShape()
      {
         if (picked_shape != NULL) picked_shape->removeChild(
            0,isectfunc::picked_shape->getNumChildren());
      }

   void clearPickerShape()
      {
         if (picker_shape != NULL) picker_shape->removeChild(
               0,isectfunc::picker_shape->getNumChildren());
      }

// --------------------------------------------------------------------------
// Method dist_point_to_segment returns the distance from input Vec3 p
// to the line segment defined by [s0,s1]:

   double dist_point_to_segment( osg::Vec3& p, osg::Vec3& s0,osg::Vec3& s1)
      {
         osg::Vec3 v = s1 - s0;
         osg::Vec3 w = p - s0;

         double c1 = w*v;
         if ( c1 <= 0 )
            return (p-s0).length();

         double c2 = v*v;
         if ( c2 <= c1 )
            return (p-s1).length();

         double b = c1 / c2;
         osg::Vec3 pb = s0+v*b;
         return (p-pb).length();
      }

// --------------------------------------------------------------------------
   void show_picked_isect(HitBase& hit,osg::Geode* pickedGeode)
      {
         osg::Geometry* pickedPoint=new osg::Geometry();
         osg::Vec4Array* ptCol=new osg::Vec4Array(1);
         (*ptCol)[0]=osg::Vec4(0.0,0.0,1.0,0.8);

         pickedPoint->setColorArray(ptCol);
         pickedPoint->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);

         osg::Vec3Array* vertices=new osg::Vec3Array(1);
         (*vertices)[0]=hit._intersectPoint;
         pickedPoint->setVertexArray(vertices);

         pickedPoint->addPrimitiveSet(
            new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,1));
         pickedGeode->addDrawable(pickedPoint);
      }

// --------------------------------------------------------------------------
   void show_picked_geom(HitBase& hit,osg::Geometry* pickedGeometry,
                         osg::Vec3& world_coords)
      {
//         cout << "inside isectfunc::show_picked_geom" << endl;
//         cout << "hit._vecIndexList.size() = "
//              << hit._vecIndexList.size() << endl;

         if (hit._drawable.valid())
         {
            //DBG_MSG("Show picked geom");
            switch (hit._vecIndexList.size())
            {
               case 1:
                  //		DBG_MSG("Show picked geom POINT");
                  isectfunc::show_picked_point(
                     hit,pickedGeometry,world_coords);
                  break;

// As of 6/28/05, we disable picking of edges to prevent inadvertently
// selecting parts of crosshairs used to mark 3D features:

               case 2:
                  //		DBG_MSG("Show picked geom LINE");
//                  isectfunc::show_picked_edge(hit,pickedGeometry);
                  break;
               case 3:
                  //		DBG_MSG("Show picked geom TRIANGLE");
                  isectfunc::show_picked_triangle(hit,pickedGeometry);
                  break;
               default:
                  DBG_MSG("IndexList.size=" << hit._vecIndexList.size()
                          <<" is not handled");
                  break;
            }
         }
      }

// --------------------------------------------------------------------------
   void show_picked_point(HitBase& hit,osg::Geometry* pickedGeometry,
                          osg::Vec3& world_coords)
      {
         string drawable_classname=hit._drawable->className();
//         cout << "_drawable->className() = " << drawable_classname << endl;
         if (drawable_classname=="Geometry")
         {
            world_coords=hit.getWorldIntersectPoint();
//            cout << "world_coords.x = " << world_coords.x()
//                 << " world_coords.y = " << world_coords.y()
//                 << " world_coords.z = " << world_coords.z() << endl;
            pickedGeometry->addPrimitiveSet(
               new osg::DrawArrays(
                  osg::PrimitiveSet::POINTS,hit._vecIndexList[0],1));
         } // drawable_classname=="Geometry" conditional
      }
   
// --------------------------------------------------------------------------
   void show_picked_edge(HitBase& hit,osg::Geometry* pickedGeometry)
      {
         unsigned int ptr[2]={hit._vecIndexList[0],hit._vecIndexList[1]};
         pickedGeometry->addPrimitiveSet(
            new osg::DrawElementsUInt(osg::PrimitiveSet::LINES,2,ptr));
      }

// --------------------------------------------------------------------------
   void show_picked_triangle(HitBase& hit,osg::Geometry* pickedGeometry)
      {
         pickedGeometry->addPrimitiveSet(
            new osg::DrawArrays(
               osg::PrimitiveSet::TRIANGLES,0,hit._vecIndexList.size()));
      }

// --------------------------------------------------------------------------
   void createPickerShape(RefPolytopeIsector* polytope)
      {
         if (picker_shape != NULL)
         {
            unsigned int nbPlans=polytope->getPlaneList().size();
            Vec3 eye=polytope->getEye();

            osg::Geode*  polytopeGeode=new osg::Geode();
            osg::Geometry* polytg=new osg::Geometry();
            osg::Vec4Array* ptCol=new osg::Vec4Array(nbPlans*4+2);
            (*ptCol)[0]=osg::Vec4(1.0,0.2,0.2,0.9);
            (*ptCol)[1]=osg::Vec4(0.2,1.0,1.0,0.9);
            for (unsigned int i=0;i<nbPlans;i++)
            {
               (*ptCol)[2+i*4+0]=osg::Vec4(1.0,0.2,0.2,0.2);
               (*ptCol)[2+i*4+1]=osg::Vec4(1.0,1.0,1.0,0.1);
               (*ptCol)[2+i*4+2]=osg::Vec4(1.0,1.0,1.0,0.1);
               (*ptCol)[2+i*4+3]=osg::Vec4(1.0,0.2,0.2,0.2);
            }
            polytg->setColorArray(ptCol);
            polytg->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

            osg::Vec3Array*	vertices=new osg::Vec3Array(nbPlans*4+2);
            (*vertices)[0]=eye;
            (*vertices)[1]=polytope->getForward();
            for (unsigned int i=0;i<nbPlans*4;i++)
            {
               (*vertices)[2+i]=polytope->getReferenceVertexList()[i];
            }

            polytg->setVertexArray(vertices);

            osg::Vec3Array*	normals=new osg::Vec3Array(nbPlans+2);
            (*normals)[0]=osg::Vec3(0.0,0.0,0.0);
            (*normals)[1]=osg::Vec3(0.0,0.0,0.0);
            for (unsigned int i=0;i<nbPlans;i++)
            {
               (*normals)[2+i]=((*vertices)[2+i*4+0]-(*vertices)[2+i*4+2])^(
                  (*vertices)[2+i*4+0]-(*vertices)[2+i*4+1]);
               (*normals)[2+i].normalize();
            }
            polytg->setNormalArray(normals);
            polytg->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);

            polytg->addPrimitiveSet(
               new osg::DrawArrays(
                  osg::PrimitiveSet::POINTS,0,1));
            polytg->addPrimitiveSet(
               new osg::DrawArrays(
                  osg::PrimitiveSet::LINES,0,2));
            polytg->addPrimitiveSet(
               new osg::DrawArrays(
                  osg::PrimitiveSet::QUADS,2,nbPlans*4));

            polytopeGeode->addDrawable(polytg);
            StateSet*	ss=new StateSet();
            ss->setAttributeAndModes(
               new osg::LineWidth(2.0),osg::StateAttribute::OVERRIDE|
               osg::StateAttribute::ON);
            ss->setRenderBinDetails(
               isectfunc::picker_shape->getStateSet()->getBinNumber()+10,
               isectfunc::picker_shape->getStateSet()->getBinName());
            polytopeGeode->setStateSet(ss);
            isectfunc::picker_shape->addChild(polytopeGeode);
         } // picker_shape != NULL conditional
      }

// --------------------------------------------------------------------------
// Picking visualization:
	
   void visualize_picking()
      {
         osg::LightModel* lightModel=new osg::LightModel();
         lightModel->setTwoSided(true);

         osg::StateSet* pickedSS=new osg::StateSet();
         pickedSS->setAttributeAndModes(
            lightModel,osg::StateAttribute::OVERRIDE|
            osg::StateAttribute::OFF);
         pickedSS->setMode(
            GL_LIGHTING,osg::StateAttribute::OVERRIDE|
            osg::StateAttribute::OFF);
         pickedSS->setAttributeAndModes(
            new osg::PolygonOffset(-1.0f,-1.0f),
            osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

         if (1)
         {
            //set the transparent functions
            pickedSS->setAttributeAndModes(new osg::BlendFunc(
               osg::BlendFunc::SRC_ALPHA,osg::BlendFunc::ONE_MINUS_SRC_ALPHA),
                                           osg::StateAttribute::ON);

            //tell to sort the mesh
            pickedSS->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            pickedSS->setRenderBinDetails(pickedSS->getBinNumber()+10,
                                          pickedSS->getBinName());
         }

         osg::Point* point=new osg::Point();
         point->setSize(10.0);
         point->setMinSize(3.0);
         point->setMaxSize(10.0);
         pickedSS->setAttributeAndModes(point,osg::StateAttribute::ON);
         pickedSS->setAttributeAndModes(
            new osg::LineWidth(10.0),osg::StateAttribute::ON);

         isectfunc::picked_shape=new osg::Group();
         isectfunc::picked_shape->setStateSet(pickedSS);
//         isectfunc::picker_shape=new osg::Group();
//         isectfunc::picker_shape->setStateSet(pickedSS);
      }
   
} // isectfunc namespace

