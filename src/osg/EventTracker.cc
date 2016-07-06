// =====================================================================
// EventTracker.cc
// =====================================================================
// Last updated on 7/2/05
// =====================================================================

#include <iostream>
#include "osg/EventTracker.h"

using std::cout;
using std::endl;

EventTracker::EventTracker()
{ 
   _keycount=0; 
}

EventTracker::~EventTracker()
{
}

bool EventTracker::handle(const osgGA::GUIEventAdapter & ea,
                          osgGA::GUIActionAdapter &)
{
   switch (ea.getEventType())
   {
      case (osgGA::GUIEventAdapter::KEYDOWN):
      {
         int key = getLowerKey(&ea);
         if (key)
         {
            _keyboardState[ key ]=true;
            _keycount++;
            return true;
         }
         else return false;
      }

      case (osgGA::GUIEventAdapter::KEYUP):
      {
         int key = getLowerKey(&ea);
         if (key)
         {
            if (_keyboardState[ key ]) /* some keys (printscreen)
                                          issue KEYUP but no KEYDOWN */
               _keycount--;
            _keyboardState[ key ]=false;
            return true;
         }
         else return false;
      }

      case (osgGA::GUIEventAdapter::PUSH):
      case (osgGA::GUIEventAdapter::DRAG):
      {
         _mouseState[ ea.getButton() ] = ea.getEventType();
         return true;
      }

      case (osgGA::GUIEventAdapter::RELEASE):
      {
         _mouseState[ ea.getButton() ] = osgGA::GUIEventAdapter::NONE;
         return true;
      }
              
      default:
         return false;
              
   }
}

void EventTracker::setKey(int i,bool state)
{ 
   _keyboardState[i] = state; 
   _keycount += (state ? 1 : -1);
}

void EventTracker::setMouse(int i,osgGA::GUIEventAdapter::EventType state)
{ 
   _mouseState[i] = state; 
}

void EventTracker::flushKey()
{ 
   _keyboardState.clear(); 
   _keycount = 0; 
}

void EventTracker::flushMouse()
{ 
   _mouseState.clear(); 
}
    
int EventTracker::keyPressed()
{ 
   return _keycount; 
}

bool EventTracker::keyboardState(int i)
{ 
   return _keyboardState[i]; 
}
 
osgGA::GUIEventAdapter::EventType EventTracker::mouseState(int i)
{ 
   return _mouseState[i]; 
}

void EventTracker::printKeys()
{
   std::map<int,bool>::iterator k;
   for (k=_keyboardState.begin(); k!=_keyboardState.end(); ++k)
      if (k->second)
         cout << std::hex << "0x" << k->first << " ";
   cout << endl;
};

int EventTracker::getLowerKey(const osgGA::GUIEventAdapter *ea)
{
   int c = ea->getKey();
   if (c < 0 || c > 255)
   {
      return c;
   }
   else
   {
      // alpha-numeric?
      if ( isalpha(c) ) return tolower(c);
            
      if ( isdigit(c) ) return c;
            
      switch(c)
      {
         case '!':
            return '1';
         case '@':
            return '2';
         case '#':
            return '3';
         case '$':
            return '4';
         case '%':
            return '5';
         case '^':
            return '6';
         case '&':
            return '7';
         case '*':
            return '8';
         case  '(':
            return '9';
         case ')':
            return '0';
         case '~':
            return '`';
         case '_':
            return '-';
         case '+':
            return '+';
         case '{':
            return '[';
         case '}':
            return ']';
         case '|':
            return '\\';
         case ':':
            return ';';
         case '"':
            return '\'';
         case '<':
            return ',';
         case '>':
            return '.';
         case '?':
            return '/';
      }
      return c;
   }
}

