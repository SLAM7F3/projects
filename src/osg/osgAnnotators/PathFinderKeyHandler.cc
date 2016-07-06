// ==========================================================================
// PathFinderKeyHandler class member function definitions
// ==========================================================================
// Last modified on 12/2/10; 12/4/10; 6/29/11
// ==========================================================================

#include "osg/osgAnnotators/PathFinderKeyHandler.h"

using std::cin;
using std::cout;
using std::endl;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void PathFinderKeyHandler::allocate_member_objects()
{
}

void PathFinderKeyHandler::initialize_member_objects()
{
   ModeController_ptr=NULL;
   PathFinder_ptr=NULL;
}

PathFinderKeyHandler::PathFinderKeyHandler(
   ModeController* MC_ptr,PathFinder* PF_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ModeController_ptr=MC_ptr;
   PathFinder_ptr=PF_ptr;
}

PathFinderKeyHandler::~PathFinderKeyHandler()
{
}

// ------------------------------------------------------
bool PathFinderKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
//      cout << "inside PathFinderKeyHandler::handle()" << endl;

      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_ANNOTATION)
      {
         if (ea.getKey()=='a')
         {
            PathFinder_ptr->compute_Astar_path();
//            PathFinder_ptr->compute_Astar_paths_vs_alpha_beta();
            return true;
         }
         else if (ea.getKey()=='b')
         {
            PathFinder_ptr->fit_Astar_paths_vs_alpha_beta();
            return true;
         }

         else if (ea.getKey()=='g')
         {
            double alpha=10;
            cout << "Enter alpha:" << endl;
            cin >> alpha;
            PathFinder_ptr->compute_Dijkstra_DTED_graph(alpha);
            return true;
         }
         else if (ea.getKey()=='d')
         {
            PathFinder_ptr->compute_Dijkstra_field();
            return true;
         }
         else if (ea.getKey()=='p')
         {
            PathFinder_ptr->purge_paths();
            return true;
         }
         else if (ea.getKey()=='t')
         {
            PathFinder_ptr->purge_SignPosts();
            return true;
         }
      } // Mode conditional
   } // key down conditional
   
   return false;
}


