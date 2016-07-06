// ==========================================================================
// WindowManager class member function definitions
// ==========================================================================
// Last modified on 9/19/07; 9/20/07; 9/21/07; 3/20/09; 3/25/09; 2/28/11
// ==========================================================================

#include <iostream>
#include <vector>
#include <osgGA/GUIEventHandler>
#include <osgGA/MatrixManipulator>
#include "osg/AnimationPathCreator.h"
#include "osg/CustomManipulator.h"
#include "osg/osgWindow/WindowManager.h"

using std::cout;
using std::endl;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void WindowManager::allocate_member_objects()
{
   AnimationPathCreator_ptr=new AnimationPathCreator(this);
}		       

void WindowManager::initialize_member_objects()
{
   EventHandlers_ptr=NULL;
}		       

WindowManager::WindowManager()
{
   allocate_member_objects();
   initialize_member_objects();
}

WindowManager::~WindowManager()
{
   delete AnimationPathCreator_ptr;
}

// ---------------------------------------------------------------------
void WindowManager::set_EventHandlers_ptr(EventHandlers* EH_ptr)
{
   EventHandlers_ptr=EH_ptr;
}

WindowManager::EventHandlers* WindowManager::get_EventHandlers_ptr()
{
   return EventHandlers_ptr;
}

const WindowManager::EventHandlers* WindowManager::get_EventHandlers_ptr() 
   const
{
   return EventHandlers_ptr;
}

// ---------------------------------------------------------------------
// Member function remove_EventHandler_refptr() iterates over the STL
// list of GUIEventHandler reference pointers.  It searches for an
// entry in the list matching the input EventHandler_refptr.  If such
// an entry is found, it is erased from the STL list and this boolean
// method returns true.

bool WindowManager::remove_EventHandler_refptr(
   osg::ref_ptr<osgGA::GUIEventHandler> EventHandler_refptr)
{
//   cout << "inside WindowManager::remove_EventHandler_refptr()" << endl;

   vector<EventHandlers::iterator> iters_to_delete;
   for (EventHandlers::iterator EH_iter=EventHandlers_ptr->begin();
        EH_iter != EventHandlers_ptr->end(); EH_iter++)
   {
      if (EH_iter->get()==EventHandler_refptr.get())
      {
         iters_to_delete.push_back(EH_iter);
      }
   }

   for (int i=0; i<int(iters_to_delete.size()); i++)
   {
      EventHandlers_ptr->erase(iters_to_delete[i]);
   }

   return (iters_to_delete.size() > 0);
}

// ==========================================================================
// Camera manipulator member functions
// ==========================================================================

unsigned int WindowManager::set_CameraManipulator(
   osgGA::CustomManipulator* CM_ptr)
{
   CameraManipulator_ptr=CM_ptr;
   return 0;
}
