// ========================================================================
// EventTracker header file 
// ========================================================================
// Last updated on 7/1/05
// ========================================================================

#ifndef _EVENT_TRACKER_H
#define _EVENT_TRACKER_H

#include <osgGA/GUIEventHandler>
#include <map>
#include <ctype.h>

class EventTracker
{
  public:

   EventTracker();
   virtual ~EventTracker();

   virtual bool handle(const osgGA::GUIEventAdapter & ea,
                       osgGA::GUIActionAdapter &);
   void setKey(int i,bool state=true);
   void setMouse(int i,osgGA::GUIEventAdapter::EventType
                 state=osgGA::GUIEventAdapter::PUSH);
   void flushKey();
   void flushMouse();
   int keyPressed();
   bool keyboardState(int i);
   osgGA::GUIEventAdapter::EventType mouseState(int i);
   void printKeys();


  protected:

   int getLowerKey( const osgGA::GUIEventAdapter *ea);

// index is a osgGA::KeySymbol (or an int representing ASCII, e.g.,
// 'a', or 65)

   std::map<int,bool> _keyboardState;    // true if button is currently
					 // being pressed
    
// index is osgGA::MouseButtonMask (LEFT_MOUSE_BUTTON,
// MIDDLE_MOUSE_BUTTON, RIGHT_MOUSE_BUTTON)

   std::map<int,osgGA::GUIEventAdapter::EventType> _mouseState; 

   // valid values: NONE,PUSH,DRAG 

   int _keycount;
};

#endif


