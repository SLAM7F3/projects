// ==========================================================================
// OBSFRUSTAKeyHandler class member function definitions
// ==========================================================================
// Last modified on 5/25/12; 1/4/13; 1/28/13
// ==========================================================================

#include "video/camera.h"
#include "color/colorfuncs.h"
#include "osg/ModeController.h"
#include "numrec/nrfuncs.h"
#include "osg/osg2D/Movie.h"
#include "osg/osgModels/OBSFRUSTAGROUP.h"
#include "osg/osgModels/OBSFRUSTAKeyHandler.h"
#include "osg/osg3D/PointCloudsGroup.h"

#include "ladar/featurefuncs.h"
#include "urban/urbanfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void OBSFRUSTAKeyHandler::allocate_member_objects()
{
}

void OBSFRUSTAKeyHandler::initialize_member_objects()
{
   n_still_images=n_frustum=toggle_counter=0;
   Terrain_Manipulator_ptr=NULL;

   z_ground=0;
   Grid_ptr=NULL;
   OBSFRUSTAGROUP_ptr=NULL;
   SignPostsGroup_ptr=NULL;
   photogroup_ptr=NULL;
   PolyhedraGroup_ptr=NULL;
}

OBSFRUSTAKeyHandler::OBSFRUSTAKeyHandler(
   OBSFRUSTAGROUP* OFG_ptr,ModeController* MC_ptr):
   GraphicalsKeyHandler(OFG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   OBSFRUSTAGROUP_ptr=OFG_ptr;
}

OBSFRUSTAKeyHandler::OBSFRUSTAKeyHandler(
   OBSFRUSTAGROUP* OFG_ptr,ModeController* MC_ptr,
   osgGA::Terrain_Manipulator* AM_ptr):
   GraphicalsKeyHandler(OFG_ptr,MC_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   Terrain_Manipulator_ptr=AM_ptr;
   OBSFRUSTAGROUP_ptr=OFG_ptr;
}

OBSFRUSTAKeyHandler::~OBSFRUSTAKeyHandler()
{
}

// ------------------------------------------------------
bool OBSFRUSTAKeyHandler::handle( 
   const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
{
   if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
   {
      if (get_ModeController_ptr()->getState()==
          ModeController::MANIPULATE_FUSED_DATA)
      {
         if (ea.getKey()=='a')
         {
            OBSFRUSTUM* OBSFRUSTUM_ptr=
               OBSFRUSTAGROUP_ptr->get_ID_labeled_OBSFRUSTUM_ptr(0);
//            OBSFRUSTUM_ptr->crop_ESB_region_voxels(
//               OBSFRUSTAGROUP_ptr->get_PointCloud_ptr());
            OBSFRUSTUM_ptr->project_ground_info_into_imageplane(
               OBSFRUSTAGROUP_ptr->get_PointCloud_ptr());
            return true;
         }

/*
            OBSFRUSTUM* OBSFRUSTUM_ptr=
               OBSFRUSTAGROUP_ptr->get_ID_labeled_OBSFRUSTUM_ptr(0);
            OBSFRUSTUM* OBSFRUSTUM2_ptr=
               OBSFRUSTAGROUP_ptr->get_ID_labeled_OBSFRUSTUM_ptr(1);

            OBSFRUSTUM_ptr->estimate_z_ground(
               OBSFRUSTAGROUP_ptr->get_PointCloud_ptr());
            OBSFRUSTUM2_ptr->estimate_z_ground(
               OBSFRUSTAGROUP_ptr->get_PointCloud_ptr());
            
//            double overlap_hue=0;	// red
            double overlap_hue=15;	// "rose" 
//            double overlap_hue=30;	// orange    looks ugly
            OBSFRUSTUM_ptr->backproject_pixels_into_second_imageplane(
               OBSFRUSTUM2_ptr,overlap_hue);

            overlap_hue=15;	// "rose"
//            overlap_hue=30;	// orange  - looks ugly !
//            overlap_hue=60;	// yellow
            OBSFRUSTUM2_ptr->backproject_pixels_into_second_imageplane(
               OBSFRUSTUM_ptr,overlap_hue);
         }
*/
         else if (ea.getKey()=='b')	
         {
            OBSFRUSTAGROUP_ptr->set_play_OBSFRUSTA_as_movie_flag(true);
            OBSFRUSTAGROUP_ptr->set_project_frames_onto_zplane_flag(true);
         }
         else if (ea.getKey()=='c')	
         {
            for (unsigned int i=0; i<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); 
                 i++)
            {
               Graphical* Graphical_ptr=OBSFRUSTAGROUP_ptr->
                  get_Graphical_ptr(i);
               OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(
                  Graphical_ptr->get_ID());

//            OBSFRUSTAGROUP_ptr->destroy_all_SUBFRUSTA();
               OBSFRUSTAGROUP_ptr->load_SUBFRUSTA();
               OBSFRUSTAGROUP_ptr->get_SUBFRUSTAGROUP_ptr()->
                  update_subfrusta(z_ground);
            }
            

            return true;
/*
// Recall that the OSGgroup for the Pyramids within each static movie
// OBSFRUSTA is initially masked.  Whenever the user presses 'c', we
// unmask that OSGgroup for the n_frustum OBSFRUSTUM and then
// increment n_frustum:

            OBSFRUSTAGROUP_ptr->unerase_OBSFRUSTUM(n_frustum++);
*/

         }
         else if (ea.getKey()=='d')
         {
            int Arrow_ID=0;
//            cout << "Enter Arrow ID:" << endl;
//            cin >> Arrow_ID;
            OBSFRUSTAGROUP_ptr->triangulate_rays(Arrow_ID);
         }
         else if (ea.getKey()=='f')
         {
            int selected_OBSFRUSTUM_ID;
            cout << "Enter OBSFRUSTUM ID:" << endl;
            cin >> selected_OBSFRUSTUM_ID;
//            OBSFRUSTAGROUP_ptr->flyto_camera_location(selected_OBSFRUSTUM_ID);

            OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(
               selected_OBSFRUSTUM_ID);
            OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->
               get_selected_OBSFRUSTUM_ptr();

            double max_blink_period=5;		// secs
            OBSFRUSTUM_ptr->set_blinking_color(colorfunc::red);
            OBSFRUSTAGROUP_ptr->blink_OBSFRUSTUM(
               selected_OBSFRUSTUM_ID,max_blink_period);


//            return OBSFRUSTAGROUP_ptr->
//               select_and_alpha_vary_OBSFRUSTUM_closest_to_screen_center();
            
/*
            int selected_ID=OBSFRUSTAGROUP_ptr->get_selected_Graphical_ID();
            if (selected_ID >= 0)
            {
               if (Terrain_Manipulator_ptr->
                   get_rotate_about_current_eyepoint_flag())
               {
                  cout << "Start cross fading" << endl;
                  OBSFRUSTAGROUP_ptr->set_cross_fading_flag(true);
//                  Terrain_Manipulator_ptr->
//                     set_rotate_about_current_eyepoint_flag(false);
               }
//               else
//               {
//                  OBSFRUSTAGROUP_ptr->flyto_camera_location(selected_ID);
//               }
            } // select_OBSFRUSTUM_number > 0 conditional
            return true;

*/
         }
/*
         else if (ea.getKey()=='g')
         {
            return OBSFRUSTAGROUP_ptr->deselect_OBSFRUSTUM();
         }
*/

// For the static panorama/dynamic video project, we want to force the
// selected OBSFRUSTUM to correspond to the video so that it can be
// faded up/down wrt the background panorama.  We also color the video
// OBSFRUSTUM red while all other static panorama OBSFRUSTA are
// colored white:

         else if (ea.getKey()=='g')
         {
            unsigned int n_OBSFRUSTA=OBSFRUSTAGROUP_ptr->get_n_Graphicals();
            int video_OBSFRUSTUM_ID=n_OBSFRUSTA-1;
            OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(
               video_OBSFRUSTUM_ID);

            OBSFRUSTUM* selected_OBSFRUSTUM_ptr=
               dynamic_cast<OBSFRUSTUM*>(
                  OBSFRUSTAGROUP_ptr->get_last_Graphical_ptr());
            selected_OBSFRUSTUM_ptr->set_selected_color(colorfunc::yellow);
//            selected_OBSFRUSTUM_ptr->set_selected_color(colorfunc::white);
            OBSFRUSTAGROUP_ptr->reset_colors();
            OBSFRUSTAGROUP_ptr->flyto_camera_location(video_OBSFRUSTUM_ID);
            return true;
         }
/*
         else if (ea.getKey()=='i')
         {
            OBSFRUSTAGROUP_ptr->get_PointCloudsGroup_ptr()->
               erase_all_Graphicals();
            OBSFRUSTAGROUP_ptr->get_PointCloudsGroup_ptr()->
               unerase_Graphical(0);
            return true;
         }
*/

         else if (ea.getKey()=='j')
         {
            unsigned int n_OBSFRUSTA=OBSFRUSTAGROUP_ptr->get_n_Graphicals();
            for (unsigned int n=0; n<n_OBSFRUSTA; n++)
            {
               OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->
                  get_OBSFRUSTUM_ptr(n);
               OBSFRUSTUM_ptr->set_movie_downrange_distance(
                  OBSFRUSTUM_ptr->get_movie_downrange_distance()*0.99);
               OBSFRUSTUM_ptr->set_relative_Movie_window();
            }
            return true;
         }
         else if (ea.getKey()=='k')
         {
            unsigned int n_OBSFRUSTA=OBSFRUSTAGROUP_ptr->get_n_Graphicals();
            for (unsigned int n=0; n<n_OBSFRUSTA; n++)
            {
               OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->
                  get_OBSFRUSTUM_ptr(n);
               OBSFRUSTUM_ptr->set_movie_downrange_distance(
                  OBSFRUSTUM_ptr->get_movie_downrange_distance()*1.01);
               OBSFRUSTUM_ptr->set_relative_Movie_window();
            }
            return true;
         }
         else if (ea.getKey()=='l')
         {
            return OBSFRUSTAGROUP_ptr->move_to_left_OBSFRUSTUM_neighbor();
/*
            cout << "Before call destroy all points" << endl;
            OBSFRUSTAGROUP_ptr->get_PointsGroup_ptr()->
               destroy_all_Points();
            return true;
*/
         }
         else if (ea.getKey()=='p')
         {
            OBSFRUSTAGROUP_ptr->display_OBSFRUSTA_as_time_sequence();
            return true;
         }
         else if (ea.getKey()=='r')
         {
            return OBSFRUSTAGROUP_ptr->move_to_right_OBSFRUSTUM_neighbor();

//            int ID;
//            cout << "Enter ID of OBSFRUSTUM to fly virtual camera towards:"
//                 << endl;
//            cin >> ID;

/*
            if (photogroup_ptr==NULL) return false;
            int order;
            cout << "Enter order of photo sorted according to polyhedron score:" << endl;
            cin >> order;
            photograph* photo_ptr=
               photogroup_ptr->get_score_ordered_photo(order);
            cout << "Photo score = " << photo_ptr->get_score() << endl;

//            int n_anim_steps=-1;
//            int n_anim_steps=0;
            int n_anim_steps=5;
            cout << "geolocation = " << photo_ptr->get_geolocation() << endl;
            return OBSFRUSTAGROUP_ptr->fly_to_entered_OBSFRUSTUM(
               photo_ptr->get_ID(),n_anim_steps);
*/

         }
         else if (ea.getKey()=='s')
         {
            if (SignPostsGroup_ptr==NULL) return false;
            OBSFRUSTAGROUP_ptr->project_SignPosts_into_imageplanes(
               SignPostsGroup_ptr);
         }
         else if (ea.getKey()=='t')
         {
            for (unsigned int i=0; i<OBSFRUSTAGROUP_ptr->get_n_Graphicals(); 
                 i++)
            {
               if (is_even(toggle_counter))
               {
                  OBSFRUSTAGROUP_ptr->unerase_OBSFRUSTUM(i);
                  OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(-1);
                  OBSFRUSTAGROUP_ptr->unmask_all_OSGsubPATs();
                  if (Grid_ptr != NULL) Grid_ptr->set_mask(1);
               }
               else
               {
                  OBSFRUSTAGROUP_ptr->erase_OBSFRUSTUM(i);
               }
            }
            OBSFRUSTAGROUP_ptr->reset_frustum_colors_based_on_Zcolormap();

// If all OBSFRUSTA are masked, we pause all videos and mask the
// ImageNumberHUD as well:

            if (is_odd(toggle_counter))
            {
               OBSFRUSTAGROUP_ptr->pause_all_videos();
               OBSFRUSTAGROUP_ptr->get_ImageNumberHUD_ptr()->
                  set_display_movie_number_flag(false);
               OBSFRUSTAGROUP_ptr->get_ImageNumberHUD_ptr()->
                  set_display_movie_state_flag(false);
            }
            
            toggle_counter++;
            return true;
         }
         else if (ea.getKey()=='u')
         {
            if (SignPostsGroup_ptr==NULL) return false;

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
                        rooftop_posn,
                        OBSFRUSTAGROUP_ptr->get_DTED_ztwoDarray_ptr(),
                        COM,theta,max_roof_z);
                  polyhedron bbox_3D=urbanfunc::construct_3D_bldg_bbox(
                     theta,max_roof_z,COM,p_roof_binary_twoDarray_ptr);

                  if (PolyhedraGroup_ptr != NULL)
                  {
                     PolyhedraGroup_ptr->destroy_all_Polyhedra();
                     double alpha=0.1;
                     PolyhedraGroup_ptr->generate_skyscraper_bbox(
                        bbox_3D,colorfunc::red,alpha);
                  }
                  OBSFRUSTAGROUP_ptr->compute_photo_views_of_polyhedra_bboxes(
                     photogroup_ptr);
                  photogroup_ptr->order_photos_by_their_scores();
               } // SignPost UVW coords conditional
            } // SignPost_ptr != NULL conditional
            return true;
         }
         else if (ea.getKey()=='v')
         {
            if (SignPostsGroup_ptr==NULL) return false;
            OBSFRUSTAGROUP_ptr->project_SignPosts_into_video_plane(
               SignPostsGroup_ptr);
         }

// Specialized inputs hardwired for NYC (touchtable) demo:

// "1" key on keypad brings up canonical Rockefeller center photo:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_End)
         {
            OBSFRUSTAGROUP_ptr->erase_all_OBSFRUSTA();
            PolyhedraGroup_ptr->set_OSGgroup_nodemask(0);
            OBSFRUSTAGROUP_ptr->pause_all_videos();

            OBSFRUSTAGROUP_ptr->unerase_OBSFRUSTUM(0);
            OBSFRUSTAGROUP_ptr->reset_frustum_colors_based_on_Zcolormap();
            OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(-1);
            OBSFRUSTAGROUP_ptr->unmask_all_OSGsubPATs();
            return true;
         }

// "2" key on keypad brings up hazy, daytime Empire State Building photo:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Down)
         {
            OBSFRUSTAGROUP_ptr->erase_all_OBSFRUSTA();
            PolyhedraGroup_ptr->set_OSGgroup_nodemask(0);
            OBSFRUSTAGROUP_ptr->pause_all_videos();

            OBSFRUSTAGROUP_ptr->unerase_OBSFRUSTUM(1);
            OBSFRUSTAGROUP_ptr->
               reset_frustum_colors_based_on_Zcolormap();
            OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(-1);
            OBSFRUSTAGROUP_ptr->unmask_all_OSGsubPATs();
            return true;
         }

// "3" key on keypad toggles on/off colored skyscraper bounding boxes

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Page_Down)
         {
            OBSFRUSTAGROUP_ptr->pause_all_videos();
            if (PolyhedraGroup_ptr != NULL)
            {
               PolyhedraGroup_ptr->toggle_OSGgroup_nodemask();
            }
         }
         
// "4" key on keypad brings up Rockfeller & Empire State Building
// skyscraper photos:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Left)
         {
            OBSFRUSTAGROUP_ptr->erase_all_OBSFRUSTA();
            OBSFRUSTAGROUP_ptr->pause_all_videos();

            OBSFRUSTAGROUP_ptr->unerase_OBSFRUSTUM(1);
            OBSFRUSTAGROUP_ptr->unerase_OBSFRUSTUM(2);
            OBSFRUSTAGROUP_ptr->
               reset_frustum_colors_based_on_Zcolormap();
            OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(-1);
            OBSFRUSTAGROUP_ptr->unmask_all_OSGsubPATs();
            return true;
         }

// "5" key on keypad brings up Brooklyn Bridge panorama photos:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Begin)
         {
            OBSFRUSTAGROUP_ptr->erase_all_OBSFRUSTA();
            PolyhedraGroup_ptr->set_OSGgroup_nodemask(0);
            OBSFRUSTAGROUP_ptr->pause_all_videos();

            OBSFRUSTAGROUP_ptr->unerase_OBSFRUSTUM(4);
            OBSFRUSTAGROUP_ptr->unerase_OBSFRUSTUM(5);
            OBSFRUSTAGROUP_ptr->unerase_OBSFRUSTUM(6);
            OBSFRUSTAGROUP_ptr->unerase_OBSFRUSTUM(7);
            OBSFRUSTAGROUP_ptr->reset_frustum_colors_based_on_Zcolormap();
            OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(-1);
            OBSFRUSTAGROUP_ptr->unmask_all_OSGsubPATs();
            return true;
         }

// "6" key on keypad brings up Empire State Building cars video

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Right)
         {
            OBSFRUSTAGROUP_ptr->erase_all_OBSFRUSTA();
            PolyhedraGroup_ptr->set_OSGgroup_nodemask(0);

            OBSFRUSTAGROUP_ptr->unerase_OBSFRUSTUM(8);
            
            OBSFRUSTAGROUP_ptr->reset_frustum_colors_based_on_Zcolormap();
            OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(-1);
            OBSFRUSTAGROUP_ptr->unmask_all_OSGsubPATs();
            return true;
         }

/*

// "7" key on keypad brings up daytime & nighttime Empire State
// Building photos:


         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Home)
         {
            OBSFRUSTAGROUP_ptr->erase_all_OBSFRUSTA();
            PolyhedraGroup_ptr->set_OSGgroup_nodemask(0);
            OBSFRUSTAGROUP_ptr->pause_all_videos();

            OBSFRUSTAGROUP_ptr->unerase_OBSFRUSTUM(2);
            OBSFRUSTAGROUP_ptr->unerase_OBSFRUSTUM(3);
            OBSFRUSTAGROUP_ptr->
               reset_frustum_colors_based_on_Zcolormap();
            OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(-1);
            OBSFRUSTAGROUP_ptr->unmask_all_OSGsubPATs();
            return true;
         }
*/

// "7" key on keypad displays OpenCV video tracking results for movers
// on 34th Street as seen from video camera atop Empire State Building:
       
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Home)
         {
            OBSFRUSTAGROUP_ptr->erase_all_OBSFRUSTA();
            int last_OBSFRUSTUM_number=
               OBSFRUSTAGROUP_ptr->get_n_Graphicals()-1;
            OBSFRUSTAGROUP_ptr->unerase_OBSFRUSTUM(last_OBSFRUSTUM_number);

            OBSFRUSTUM* ESB_frustum_ptr=OBSFRUSTAGROUP_ptr->
               get_OBSFRUSTUM_ptr(last_OBSFRUSTUM_number);
            ESB_frustum_ptr->get_Movie_ptr()->generate_ESB_car_tracks();
            ESB_frustum_ptr->set_permanent_color(colorfunc::white);
            ESB_frustum_ptr->set_color(
               colorfunc::get_OSG_color(colorfunc::yellow),0);
//               colorfunc::get_OSG_color(colorfunc::white),0);

            OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(-1);
            OBSFRUSTAGROUP_ptr->unmask_all_OSGsubPATs();
      
// Change colormap from (presumably) its current grey scale setting to
// large value sans white.  Cyan colored cars, stationary building
// background and grey colored rays show up better against this latter
// color map:

            OBSFRUSTAGROUP_ptr->get_PointCloudsGroup_ptr()->toggle_colormap();

            return true;
         }

// "9" on keypad

//         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Page_Up)

//         else if (ea.getKey()=='i')
//         {
//            cout << "Before call to compute_polyhedra_intersections()" 
//                 << endl;
//            OBSFRUSTAGROUP_ptr->compute_polyhedra_intersections();
//            return true;
//         }
//         else if (ea.getKey()=='d')
//         {
//            OBSFRUSTAGROUP_ptr->display_intersection_footprint();
//            return true;
//         }

// Change transperency of OBSFRUSTA image planes by pressing Up and
// Down arrows:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
         {
//            cout << "inside OBSFRUSTAKeyHandler::key_up" << endl;
            vector<Movie*> Movie_ptrs=
               OBSFRUSTAGROUP_ptr->find_Movies_in_OSGsubPAT();
            if (Movie_ptrs.size()==0) return false;

            for (unsigned int i=0; i<Movie_ptrs.size(); i++)
            {
               Movie* Movie_ptr=Movie_ptrs[i];
               if (Movie_ptr != NULL)
               {
                  double alpha=Movie_ptr->get_alpha();
//                  cout << "alpha = " << alpha << endl;
                  alpha=basic_math::min(1.0,alpha+0.05);
                  Movie_ptr->set_alpha(alpha);
               }
            } // loop over Movies within Movie_ptrs
            return true;
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
         {
//            cout << "inside OBSFRUSTAKeyHandler::key_down" << endl;
            vector<Movie*> Movie_ptrs=
               OBSFRUSTAGROUP_ptr->find_Movies_in_OSGsubPAT();

            if (Movie_ptrs.size()==0) return false;
            for (unsigned int i=0; i<Movie_ptrs.size(); i++)
            {
               Movie* Movie_ptr=Movie_ptrs[i];
               if (Movie_ptr != NULL)
               {
                  double alpha=Movie_ptr->get_alpha();
                  alpha=basic_math::max(0.0,alpha-0.05);
//                  cout << "alpha = " << alpha << endl;
                  Movie_ptr->set_alpha(alpha);
               }
            } // loop over Movies within *Movie_ptrs_ptr
            return true;
         } 
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Right)
         {
            vector<Movie*> Movie_ptrs=
               OBSFRUSTAGROUP_ptr->find_Movies_in_OSGsubPAT();
            if (Movie_ptrs.size()==0) return false;

            OBSFRUSTAGROUP_ptr->zoom_virtual_camera(0.97); 	// zoom in
            return true;
         } 
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Left)
         {
            vector<Movie*> Movie_ptrs=
               OBSFRUSTAGROUP_ptr->find_Movies_in_OSGsubPAT();
            if (Movie_ptrs.size()==0) return false;

            OBSFRUSTAGROUP_ptr->zoom_virtual_camera(1.03);	// zoom out
            return true;
         } 
         
      } // mode==MANIPULATE_FUSED_DATA conditional

      else if (get_ModeController_ptr()->getState()==
      ModeController::MANIPULATE_OBSFRUSTUM)
      {

// "u" key on keypad 

         if (ea.getKey()=='u')
         {
            OBSFRUSTAGROUP_ptr->adjust_frustum_angles(0.2*PI/180,0,0);
            return true;
         }

// "i" key on keypad 

         else if (ea.getKey()=='i')
         {
            OBSFRUSTAGROUP_ptr->adjust_frustum_angles(-0.2*PI/180,0,0);
            return true;
         }

// "j" key on keypad 

         else if (ea.getKey()=='j')
         {
            OBSFRUSTAGROUP_ptr->adjust_frustum_angles(0,0.2*PI/180,0);
         }
         
// "k" key on keypad 

         else if (ea.getKey()=='k')
         {
            OBSFRUSTAGROUP_ptr->adjust_frustum_angles(0,-0.2*PI/180,0);
            return true;
         }

// "m" key on keypad

         else if (ea.getKey()=='m')
         {
            OBSFRUSTAGROUP_ptr->adjust_frustum_angles(0,0,0.2*PI/180);
            return true;
         }

// "," key on keypad

         else if (ea.getKey()==',')
         {
            OBSFRUSTAGROUP_ptr->adjust_frustum_angles(0,0,-0.2*PI/180);
            return true;
         }

// Change transperency of OBSFRUSTA image planes by pressing Up and
// Down arrows:

         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
         {
            vector<Movie*> Movie_ptrs=
               OBSFRUSTAGROUP_ptr->find_Movies_in_OSGsubPAT();

            if (Movie_ptrs.size()==0) return false;
            for (unsigned int i=0; i<Movie_ptrs.size(); i++)
            {
               Movie* Movie_ptr=Movie_ptrs[i];
               if (Movie_ptr != NULL)
               {
                  double alpha=Movie_ptr->get_alpha();
                  alpha=basic_math::min(1.0,alpha+0.05);
                  Movie_ptr->set_alpha(alpha);
               }
            } // loop over Movies within Movie_ptrs
            return true;
         }
         else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
         {
            vector<Movie*> Movie_ptrs=
               OBSFRUSTAGROUP_ptr->find_Movies_in_OSGsubPAT();

            if (Movie_ptrs.size()==0) return false;
            for (unsigned int i=0; i<Movie_ptrs.size(); i++)
            {
               Movie* Movie_ptr=Movie_ptrs[i];
               if (Movie_ptr != NULL)
               {
                  double alpha=Movie_ptr->get_alpha();
                  alpha=basic_math::max(0.0,alpha-0.05);
                  Movie_ptr->set_alpha(alpha);
               }
            } // loop over Movies within *Movie_ptrs_ptr
            return true;
         }
         
      } // mode==MANIPULATE_OBSFRUSTUM conditional
      
   } // key down conditional
   
   return false;
}

