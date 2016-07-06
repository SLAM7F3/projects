// ==========================================================================
// Purely virtual isectPickHandler class to handle events with a pick
// ==========================================================================
// Last modified on 6/28/05
// ==========================================================================

#include <iostream>
#include <osgProducer/Viewer>
#include "math/basic_math.h"
#include "osg/osgPicker/isectfuncs.h"
#include "osg/osgPicker/isectPickHandler.h"
#include "osg/osgPicker/osg_polytope_picker.h"

using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void isectPickHandler::allocate_member_objects()
{
}		       

void isectPickHandler::initialize_member_objects()
{
}		       

isectPickHandler::isectPickHandler() 
{
   allocate_member_objects();
   initialize_member_objects();
}

isectPickHandler::~isectPickHandler() 
{
}

// ==========================================================================
// Member function pick takes in the following input rectangle coordinates:

// sx,sy = Window coordinates of initial mouse click normalized from -1 to 1.
// ex,ey = Window coordinates of final mouse release normalized from -1 to 1.

void isectPickHandler::pick(
   osgProducer::Viewer* viewer_ptr,float sx,float sy,float ex,float ey)
{
   cout << "inside isectPickHandler::pick()" << endl;
   cout << "sx = " << sx << " sy = " << sy 
        << " ex = " << ex << " ey = " << ey << endl;

// only to show the picker and picked geometries
    
   float x=( sx<ex ? sx : ex) + fabs(ex-sx)/2.0;
   float y=( sy<ey ? sy : ey) + fabs(ey-sy)/2.0;

// convert into clip coords. (it is already in good space here)

   float rx = x;
   float ry = y;

// Rectangle size:

   float rsx=fabs(ex-sx);
   float rsy=fabs(ey-sy);

   rsx=max(rsx,0.01f);
   rsy=max(rsy,0.01f);

   //cout << "    rx "<<rx<<"  "<<ry<<endl;

   Producer::Camera* camera=viewer_ptr->getCamera(0);
   osgProducer::OsgSceneHandler* sh = 
      dynamic_cast<osgProducer::OsgSceneHandler*>(camera->getSceneHandler());

   osg::Matrixd matView;
   osg::Matrixd matProj;
   osgUtil::SceneView* sv = sh?sh->getSceneView():NULL;
   if (sv != NULL)
   {
      matView=sv->getViewMatrix();
      matProj=sv->getProjectionMatrix();
   }
   else
   {
      matView=osg::Matrixd(camera->getViewMatrix());
      matProj=osg::Matrixd(camera->getProjectionMatrix());
   }

   isectfunc::clearPickerShape();
   isectfunc::clearPickedShape();
   PolytopeIntersectVisitor::HitList hlist;

   PolytopePicker picker_polytope;
   bool found=picker_polytope.pick(
      rx,ry,matView,matProj,viewer_ptr->getSceneData(),hlist,rsx,rsy);
   cout << "found = " << found << endl;
   
   if (found)
   {
      osg::Group* root = dynamic_cast<osg::Group*>(
         viewer_ptr->getSceneData()); 
      if (!root) return;

      osg::Geode* pickedGeode=new osg::Geode();
      isectfunc::picked_shape->addChild(pickedGeode);
      isectfunc::createPickerShape(hlist.begin()->_originalGen.get());

      cout << "hlist.size() = " << hlist.size() << endl;

      int counter=1;
      for (PolytopeIntersectVisitor::HitList::iterator 
              hitr=hlist.begin(); hitr!=hlist.end(); hitr++)
      {
         osgIsect::HitBase hit=*(hitr);
         osg::Geometry*	pickedGeometry=new osg::Geometry(
            (*(hit._drawable.get()->asGeometry())));
         if (pickedGeometry)
         {
            osg::UIntArray* pickedIndices=new osg::UIntArray(
               hit._vecIndexList.size());
            HitBase::VecIndexList& vil = hit._vecIndexList;
            for(unsigned int i=0;i<vil.size();++i)
            {
               (*pickedIndices)[i]=vil[i];
            }

            if (vil.size()>2)
            {
               pickedGeometry->setVertexIndices(pickedIndices);
            }
            pickedGeometry->setTexCoordArray(0,NULL);
            pickedGeometry->setTexCoordArray(1,NULL);
            pickedGeometry->setTexCoordArray(2,NULL);
            pickedGeometry->setTexCoordArray(3,NULL);
            pickedGeometry->setTexCoordIndices(0,NULL);
            pickedGeometry->setTexCoordIndices(1,NULL);
            pickedGeometry->setTexCoordIndices(2,NULL);
            pickedGeometry->setTexCoordIndices(3,NULL);
//				pickedGeometry->setTexCoordData(0,NULL);
//				pickedGeometry->setTexCoordData(1,NULL);
//				pickedGeometry->setTexCoordData(2,NULL);
//				pickedGeometry->setTexCoordData(3,NULL);
            if (pickedGeometry->getNormalIndices())
               if (vil.size()>2)
               {
                  pickedGeometry->setNormalIndices(pickedIndices);
               }

            pickedGeometry->setColorIndices(NULL);
            osg::Vec4Array* colors=new osg::Vec4Array(1);
            (*colors)[0]=osg::Vec4(1.0,0.0,0.0,0.5);

            cout << "counter = " << counter << endl;
            counter++;
            cout << "Before setting pickedGeometry colors to red" << endl;
               
            pickedGeometry->setColorArray(colors);
            pickedGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

            pickedGeometry->removePrimitiveSet(
               0,pickedGeometry->getNumPrimitiveSets());
            isectfunc::show_picked_geom(hit,pickedGeometry,world_coords);
            set_pick_handler_pixel_coords();
            pickedGeode->addDrawable(pickedGeometry);

//            cout << "vil.size() = " << vil.size() << endl;
            if (vil.size()>=2) isectfunc::show_picked_isect(hit,pickedGeode);
         } // pickedGeometry conditional
      } // loop over elements in hitlist

      cout << "insert_feature_info_flag = " << insert_feature_info_flag 
           << endl;
      if (insert_feature_info_flag)
      {
         cout << "Before call to virtual instantiate_feature()" << endl;
         instantiate_feature(root);
      }
      else
      {
         cout << "Before call to virtual select_feature()" << endl;
         select_feature(root);
      }

   } // found conditional

   cout << "at end of isectPickHandler::pick()" << endl << endl;
}
