// ==========================================================================
// Header file for LINESEGMENTPICKHANDLER class
// ==========================================================================
// Last modfied on 12/29/06; 1/3/07; 1/21/07; 6/15/08; 9/2/08
// ==========================================================================

#ifndef LINESEGMENT_PICK_HANDLER_H
#define LINESEGMENT_PICK_HANDLER_H

#include "osg/osgGeometry/GeometricalPickHandler.h"
#include "datastructures/Linkedlist.h"
#include "osg/osgGeometry/LineSegmentsGroup.h"

class LineSegment;
class ModeController;
// class Transformer;
class WindowManager;

class LineSegmentPickHandler : public GeometricalPickHandler
{

  public: 

   LineSegmentPickHandler(
      const int p_ndims,Pass* PI_ptr,
      osgGA::CustomManipulator* CM_ptr,LineSegmentsGroup* LSG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr);

// LineSegment generation, manipulation and annihiilation methods:
      
  protected:

   virtual ~LineSegmentPickHandler();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool rotate(const osgGA::GUIEventAdapter& ea);
   virtual bool release();

  private:

   LineSegmentsGroup* LineSegmentsGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();

   LineSegmentsGroup* get_LineSegmentsGroup_ptr();
   Linkedlist<LineSegment*>* get_LineSegmentlist_ptr();
   LineSegment* get_linesegment_ptr();

   bool instantiate_LineSegment();
   bool select_LineSegment();
   bool find_closest_vertex_nearby_mouse_posn(int& closest_LineSegment_ID);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline LineSegment* LineSegmentPickHandler::get_linesegment_ptr() 
{
//   std::cout << "inside RPH::get_linesegment_ptr()" << std::endl;
   int line_number=get_LineSegmentsGroup_ptr()->
      get_selected_Graphical_ID();
   if (line_number < 0)
   {
      return NULL;
   }
   else
   {
      return dynamic_cast<LineSegment*>(
         get_LineSegmentsGroup_ptr()->
         get_ID_labeled_Graphical_ptr(line_number));
   }
}


#endif // LineSegmentPickHandler.h



