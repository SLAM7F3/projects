// ==========================================================================
// Header file for purely virtual ISECTPICKHANDLER class
// ==========================================================================
// Last modified on 6/28/05
// ==========================================================================

#ifndef ISECT_PICK_HANDLER_H
#define ISECT_PICK_HANDLER_H

#include "osg/PickHandler.h"

class isectPickHandler : public PickHandler
{
  private:

   void allocate_member_objects();
   void initialize_member_objects();
   
  public: 

   isectPickHandler();
   virtual ~isectPickHandler();

// Keyboard and mouse event handling methods:

   virtual void pick(
      osgProducer::Viewer* viewer_ptr,const osgGA::GUIEventAdapter& ea)=0;
   virtual void pick(osgProducer::Viewer* viewer_ptr,
                     float sx,float sy,float ex,float ey);
   virtual void drag(osgProducer::Viewer* viewer_ptr, 
                     const osgGA::GUIEventAdapter& ea)=0;
   virtual void release(osgProducer::Viewer* viewer_ptr, 
                        const osgGA::GUIEventAdapter& ea)=0;
   virtual void erase_object(osgProducer::Viewer* viewer_ptr, 
                             const osgGA::GUIEventAdapter& ea)=0;
   virtual void destroy_object(osgProducer::Viewer* viewer_ptr, 
                               const osgGA::GUIEventAdapter& ea)=0;

// Feature generation, manipulation and annihilation methods:

   virtual void instantiate_feature(osg::Group* root)=0;
   virtual void set_pick_handler_pixel_coords()=0;
   virtual void select_feature(osg::Group* root)=0;
//   bool find_closest_crosshairs_nearby_curr_posn(
//      Mynode<feature*>*& closest_node_ptr);
//   void reset_feature_colors();
//   void move_feature(osg::Group* root);
//   void erase_feature(osg::Group* root);
//   void destroy_feature(osg::Group* root);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

#endif // isectPickHandler.h



