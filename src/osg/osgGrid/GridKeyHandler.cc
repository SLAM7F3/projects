// ==========================================================================
// GridKeyHandler class member function definitions
// ==========================================================================
// Last modified on 3/27/06; 10/1/07; 11/19/09; 11/20/09
// ==========================================================================

#include <iostream>
#include <string>
#include <osgDB/WriteFile>
#include "general/filefuncs.h"
#include "osg/osgGrid/Grid.h"
#include "osg/osgGrid/GridKeyHandler.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void GridKeyHandler::allocate_member_objects()
{
}

void GridKeyHandler::initialize_member_objects()
{
   ModeController_ptr=NULL;
   grid_ptr=NULL;
}

GridKeyHandler::GridKeyHandler(
   ModeController* MC_ptr,Grid* G_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   ModeController_ptr=MC_ptr;
   grid_ptr=G_ptr;
}

GridKeyHandler::~GridKeyHandler()
{
}

// ------------------------------------------------------
bool GridKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (ModeController_ptr->getState()==ModeController::VIEW_DATA)
      {
         switch ( ea.getKey() )
         {
            case 'l' :
               grid_ptr->toggle_mask();
               return true;

            case 'j' :
               grid_ptr->increase_delta_x();
               return true;

            case 'J' :
               grid_ptr->decrease_delta_x();
               return true;

            case 'k' :
               grid_ptr->increase_delta_y();
               return true;

            case 'K' :
               grid_ptr->decrease_delta_y();
               return true;
/*
            case 'p' :
               grid_ptr->increase_z_plane();
               return true;

            case 'P' :
               grid_ptr->decrease_z_plane();
               return true;
*/

//            case 'm' :
//               cout << "Pressed m in GridKeyHandler" << endl;
//               grid_ptr->change_color();

            case 'y' :
               osg::Node* root=grid_ptr->get_root_ptr();
               HiresDataVisitor* HiresDataVisitor_ptr=
                  grid_ptr->get_HiresDataVisitor_ptr();
               if (HiresDataVisitor_ptr==NULL) return false;

               HiresDataVisitor_ptr->set_application_type( 
                  HiresDataVisitor::reset_pageLOD_child_filenames);
               cout << "Pressed 'y' key" << endl;
               root->accept(*HiresDataVisitor_ptr);

               ofstream binary_outstream;
//               string output_filename="root.ive";
               string output_filename="root.osg";
               filefunc::deletefile(output_filename);
            
               if ( osgDB::writeNodeFile( *root, output_filename) )
               {
                  cout << "Wrote file: " << output_filename << endl;
               }
               else
               {
                  cout << "Could not write output file" << endl;
               }
               return true;

         } // switch statement
      } // Mode conditional
   } // key down conditional
   
   return false;
}


