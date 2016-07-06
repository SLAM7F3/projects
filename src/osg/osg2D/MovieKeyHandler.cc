// ==========================================================================
// MovieKeyHandler class member function definitions
// ==========================================================================
// Last modified on 10/9/09; 6/30/10; 10/3/12
// ==========================================================================

#include <iostream>
#include <string>
#include "general/filefuncs.h"
#include "general/inputfuncs.h"
#include "osg/ModeController.h"
#include "osg/osg2D/MoviesGroup.h"
#include "osg/osg2D/MovieKeyHandler.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void MovieKeyHandler::allocate_member_objects()
{
}		       

void MovieKeyHandler::initialize_member_objects()
{
   Movie_ptr=NULL;
   MoviesGroup_ptr=NULL;
}		       

MovieKeyHandler::MovieKeyHandler(ModeController* p_mode):
   GraphicalsKeyHandler(p_mode)
{
   allocate_member_objects();
   initialize_member_objects();
}

MovieKeyHandler::MovieKeyHandler( 
   ModeController* p_mode,MoviesGroup* MG_ptr):
   GraphicalsKeyHandler(MG_ptr,p_mode)
{
   allocate_member_objects();
   initialize_member_objects();

   MoviesGroup_ptr=MG_ptr;
}

// ------------------------------------------------------

bool MovieKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if ((get_ModeController_ptr()->getState() == ModeController::RUN_MOVIE ||
        get_ModeController_ptr()->getState() == 
        ModeController::GENERATE_AVI_MOVIE) && MoviesGroup_ptr != NULL)
   {
      if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
      {
         switch ( ea.getKey() )
         {
            case 'a':  // Enter video frame annotation
               Movie_ptr=MoviesGroup_ptr->get_Movie_ptr(0);
               Movie_ptr->annotate_curr_frame();

               return true;
               break;

            case 'e':  // Export video annotations to CSV file
               Movie_ptr=MoviesGroup_ptr->get_Movie_ptr(0);
               Movie_ptr->export_video_annotations();

               return true;
               break;
               
            case 'f' :	// Reset first frame to display
               int first_frame;
               cout << "Enter first frame to display:" << endl;
               cin >> first_frame;
               Movie_ptr=MoviesGroup_ptr->get_Movie_ptr(0);
               Movie_ptr->get_texture_rectangle_ptr()->
                  set_first_frame_to_display(first_frame);
               return true;
               break;

/*
            case 'j':
               int curr_frame;
               cout << "Enter frame number to jump to:" << endl;
               cin >> curr_frame;
               Movie* Movie_ptr=MoviesGroup_ptr->get_Movie_ptr(0);
               return Movie_ptr->jump_to_frame(curr_frame);
               break;
*/
             
/*
            case 'l' :	// Reset last frame to display
               int last_frame;
               cout << "Enter last frame to display:" << endl;
               cin >> last_frame;
               Movie* Movie_ptr=MoviesGroup_ptr->get_Movie_ptr(0);
               Movie_ptr->get_texture_rectangle_ptr()->
                  set_last_frame_to_display(last_frame);
               return true;
               break;
*/

            case 'm':
               Movie_ptr=MoviesGroup_ptr->get_Movie_ptr(0);
               Movie_ptr->change_color_map();
               return true;
               break;

/*
            case 's':
               MoviesGroup_ptr->identify_sky_pixels();
               return true;
               break;
*/
             
            case 't':
               MoviesGroup_ptr->identify_ocean_pixels();
               return true;
               break;

//            case '1' :	// 
//               MoviesGroup_ptr->read_future_photo(true);
//               return true;
//               break;

//            case '2' :	
//               MoviesGroup_ptr->read_future_photo(false);
//               return true;
//               break;
               
            case 'u' :	// Reset horizontal U-axis scale factor
               MoviesGroup_ptr->reset_Vscale();
               return true;
               break;
               
            case 'v' :	// Reset vertical V-axis scale factor
               MoviesGroup_ptr->reset_Vscale();
               return true;
               break;

            case ';':
            {
//               cout << "MoviesGroup_ptr->get_OSGgroup_nodemask() = "
//                    << MoviesGroup_ptr->get_OSGgroup_nodemask() << endl;
               
               if (MoviesGroup_ptr->get_OSGgroup_nodemask()==0)
               {
                  cout << "Toggling movie from off to on" << endl;
                  for (unsigned int i=0; i<MoviesGroup_ptr->
                          get_n_Graphicals(); i++)
                  {
                     Movie_ptr=MoviesGroup_ptr->get_Movie_ptr(i);

// Reset CH video alpha to zero so that it can be faded in via the up
// arrow key.  But set alphas for all other UAV video chips equal to
// one:

                     if (i==0)
                     {
                        Movie_ptr->set_alpha(0.0);
                     }
                     else
                     {
                        Movie_ptr->set_alpha(1.0);
                     }
                  }
               }
               else
               {
                  cout << "Toggling movie from on to off" << endl;
                  for (unsigned int i=0; i<MoviesGroup_ptr->
                          get_n_Graphicals(); i++)
                  {
                     Movie_ptr=MoviesGroup_ptr->get_Movie_ptr(i);
                     Movie_ptr->set_alpha(0.0);
                  }
                  
               }
               MoviesGroup_ptr->toggle_OSGgroup_nodemask();
//               cout << "finally, MoviesGroup_ptr->get_OSGgroup_nodemask() = "
//                    << MoviesGroup_ptr->get_OSGgroup_nodemask() << endl;
               return true;
               break;
            }
            
// Press Up [Down] arrow key to decrease [increase] alpha blending of
// foreground movie texture with background point cloud and/or
// surface:

            case osgGA::GUIEventAdapter::KEY_Up:
            {
               cout << "Key_up pressed in MovieKeyHandler" << endl;
//               cout << "MoviesGroup_ptr->get_n_Graphicals() = "
//                    << MoviesGroup_ptr->get_n_Graphicals() << endl;
//               cout << "Selected_Movie_ptr = "
//                    << MoviesGroup_ptr->get_selected_Movie_ptr() << endl;
               Movie_ptr=MoviesGroup_ptr->get_selected_Movie_ptr();
               if (Movie_ptr != NULL)
               {
                  Movie_ptr->increase_alpha();
               }

               return true;
               break;
            }
            
            case osgGA::GUIEventAdapter::KEY_Down:
            {
               cout << "Key_down pressed in MovieKeyHandler" << endl;
//               cout << "MoviesGroup_ptr->get_n_Graphicals() = "
//                    << MoviesGroup_ptr->get_n_Graphicals() << endl;
//               cout << "Selected_Movie_ptr = "
//                    << MoviesGroup_ptr->get_selected_Movie_ptr() << endl;
               Movie_ptr=MoviesGroup_ptr->get_selected_Movie_ptr();
//               cout << "Movie_ptr = " << Movie_ptr << endl;

               if (Movie_ptr != NULL)
               {
                  Movie_ptr->decrease_alpha();
               }
               
               return true;
               break;
            }
            
         } // switch ( ea.getKey() )
      } // ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN conditional
   } // Mode = RUN_MOVIE or GENERATE_AVI_MOVIE conditional

   if ((get_ModeController_ptr()->getState() == 
        ModeController::MANIPULATE_MOVIE) && MoviesGroup_ptr != NULL)
   {
      if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
      {
         switch ( ea.getKey() )
         {

            case 'a':
               Movie_ptr=MoviesGroup_ptr->get_Movie_ptr(0);
               Movie_ptr->generate_average_background_image();
               return true;
               break;

            case 'n':
               MoviesGroup_ptr->null_region_outside_poly();
               return true;
               break;

            case 's':	

/*
//               MoviesGroup_ptr->save_to_file();

//               double min_long=-101.933704;
//               double min_lat=33.512337;
//               double max_long=-101.931001;
//               double max_lat=33.514500;

               double min_long=-101.924798;
               double max_long=-101.920367;
               double min_lat=33.518227;
               double max_lat=33.520614;

//               double min_long=-101.94;
//               double max_long=-101.93;
//               double min_lat=33.492;
//               double max_lat=33.50;

//               string output_subdir="./subframes";
//               Movie_ptr->export_current_subframe(
//                  min_long,max_long,min_lat,max_lat,output_subdir);
*/

               Movie_ptr=MoviesGroup_ptr->get_Movie_ptr(0);
               Movie_ptr->export_current_frame();

/*
               string output_subdir="./subframes/";
               filefunc::dircreate(output_subdir);

               int curr_framenumber=Movie_ptr->get_curr_framenumber();
               string suffix=".png";
               string output_filename=output_subdir+"frame_"
                  +stringfunc::number_to_string(curr_framenumber)+suffix;
               cout << "output_filename = " << output_filename << endl;
               Movie_ptr->get_texture_rectangle_ptr()->write_curr_frame(
                  output_filename);
*/

               return true;
               break;

//            case 'y':
//               Movie_ptr=MoviesGroup_ptr->get_Movie_ptr(0);
//               Movie_ptr->rescale_image(0.5);
//               Movie_ptr->get_texture_rectangle_ptr()->
//                  convert_color_image_to_greyscale();
//               Movie_ptr->get_texture_rectangle_ptr()->
//                  convert_greyscale_image_to_hue_colored();
//               return true;
//               break;

// Press "Up arrow" or "Down arrow" to move movie up or down in the
// world-z direction:

            case osgGA::GUIEventAdapter::KEY_Up:
               MoviesGroup_ptr->move_z(0.5, 0 );
               return true;
               break;
               
            case osgGA::GUIEventAdapter::KEY_Down:
               int Movie_ID=0;
               MoviesGroup_ptr->move_z(-0.5, Movie_ID );
               return true;
               break;
         }
      }
   }

   return false;
}


