// ==========================================================================
// SignPostsKeyHandler class member function definitions
// ==========================================================================
// Last modified on 3/2/10; 11/1/10; 5/17/11
// ==========================================================================

#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "osg/ModeController.h"
#include "osg/osgAnnotators/SignPostsGroup.h"
#include "osg/osgAnnotators/SignPostsKeyHandler.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void SignPostsKeyHandler::allocate_member_objects()
{
}

void SignPostsKeyHandler::initialize_member_objects()
{
   SignPostsGroup_ptr=NULL;
   AnimationController_ptr=NULL;
}

SignPostsKeyHandler::SignPostsKeyHandler(
   SignPostsGroup* SPG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(SPG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   SignPostsGroup_ptr=SPG_ptr;
}

SignPostsKeyHandler::~SignPostsKeyHandler()
{
}

// ------------------------------------------------------
bool SignPostsKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{

   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_ANNOTATION)
      {

// Press "e" to edit a SignPost:

         if (ea.getKey()=='e')
         {
            SignPostsGroup_ptr->edit_SignPost_label();
            return true;
         }
         else if (ea.getKey()=='h')
         {
            SignPostsGroup_ptr->generate_ground_target_w_track();
            return true;
         }

// Press "r" to restore SignPost information from Postgres database:
      
         else if (ea.getKey()=='r')
         {
//            colorfunc::Color signposts_color=colorfunc::white;
//            colorfunc::Color signposts_color=colorfunc::red;

// Note added on 3/2/10: In order for Karl Ni to put SignPosts into
// his SIGMA results, we need to comment out following line and
// uncomment its successor:

//            return SignPostsGroup_ptr->
//               retrieve_all_signposts_from_PostGIS_databases(signposts_color);
            return SignPostsGroup_ptr->read_info_from_file();
         }

// Press "s" to save SignPost information to ascii text file:

         else if (ea.getKey()=='s')
         {
            SignPostsGroup_ptr->save_info_to_file();            
            return true;
         }

         else if (ea.getKey()=='x')
         {
            SignPostsGroup_ptr->read_OSG_file();            
            return true;
         }
         else if (ea.getKey()=='y')
         {
            SignPostsGroup_ptr->write_OSG_file();            
            return true;
         }

/*
// Press "t" to retrieve current SignPost information stored within
// Postgres world_model database:

         else if (ea.getKey()=='t')
         {
            SignPostsGroup_ptr->
               retrieve_signposts_from_SKS_worldmodel_database();
            return true;
         }
         else if (ea.getKey()=='v')
         {
//            return SignPostsGroup_ptr->project_SignPosts_into_video_plane();
            SignPost* SignPost_ptr=SignPostsGroup_ptr->gazetteer();
            if (SignPost_ptr != NULL)
            {
               threevector rooftop_posn;
               if (SignPost_ptr->get_UVW_coords(
                  SignPostsGroup_ptr->get_curr_t(),
                  SignPostsGroup_ptr->get_passnumber(),rooftop_posn))
               {
//                  cout << "rooftop posn = " << rooftop_posn << endl;

                  double theta,max_roof_z;
                  threevector COM;
                  twoDarray* p_roof_binary_twoDarray_ptr=
                     featurefunc::construct_rooftop_binary_mask(
                        rooftop_posn,DTED_ztwoDarray_ptr,COM,theta,
                        max_roof_z);
                  polyhedron bbox_3D=urbanfunc::construct_3D_bldg_bbox(
                     theta,max_roof_z,COM,p_roof_binary_twoDarray_ptr);

                  if (PolyhedraGroup_ptr != NULL)
                  {
                     PolyhedraGroup_ptr->destroy_all_Polyhedra();
                     PolyhedraGroup_ptr->generate_skyscraper_bbox(
                        bbox_3D,colorfunc::red);
                  }
               }
            }
            return true;
         }
*/

         else if (ea.getKey()=='4')
         {
            SignPostsGroup_ptr->read_info_from_file();

// Following SignPost parameters are specialized for RASR hallyway demo:

            SignPostsGroup_ptr->set_size(0.02,0.1);
            SignPostsGroup_ptr->set_colors(colorfunc::red,colorfunc::red);


// Following SignPost parameters are specialized for MIT lobby demo:

//            SignPostsGroup_ptr->set_colors(colorfunc::red,colorfunc::yellow);
            SignPostsGroup_ptr->set_max_text_width(100);	
            return true;
         }

// Press "Delete" key to completely destroy a SignPost:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Delete)
         {
            SignPostsGroup_ptr->destroy_SignPost();
            return true;
         }

// Press ">" ["<"] key to increase [decrease] a SignPost's size:

// FAKE FAKE:  for Baghdad movie generation only...Tues, Mar 13 at 4:37 pm..
// Change '>'--> ']' and '<' --> '['

         else if (ea.getKey()=='>')
         {
//            SignPostsGroup_ptr->change_size(1.2);
            SignPostsGroup_ptr->change_size(2.0);
//            SignPostsGroup_ptr->set_size(3.0);
            return true;
         }
         else if (ea.getKey()=='<')
         {
//            SignPostsGroup_ptr->change_size(1.0/1.2);
            SignPostsGroup_ptr->change_size(0.5);
//            SignPostsGroup_ptr->set_size(0.3);
            return true;
         }
         
// Press "Up arrow" or "Down arrow" to move a selected SignPost up or
// down in the world-z direction:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
         {
            SignPostsGroup_ptr->move_z(1);
            return true;
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
         {
            SignPostsGroup_ptr->move_z(-1);
            return true;
         }
         
      } // mode = MANIPULATE_ANNOTATION conditional
   } // key down conditional
   
   return false;
}


