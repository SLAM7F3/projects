// Wii button values:

// Right cross button: 512
// Left cross button: 256
// Up cross button : 2048
// Down cross button: 1024

// Clear A button: 8

// + button: 4096
// - button: 16
// home button: 128

// 1 button: 2
// 2 button: 1

// Button on back of Wii back: 4

// =========================================================================
// Wiimote class member function definitions
// =========================================================================
// Last modified on 9/8/11; 9/9/11; 9/11/11; 6/5/12
// =========================================================================

#include <cstdlib>
#include "general/outputfuncs.h"
#include "time/timefuncs.h"
#include "ins/wiimote.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

#define toggle_bit(bf,b)                        \
   (bf) = ((bf) & b)                            \
      ? ((bf) & ~(b))                           \
      : ((bf) | (b))

cwiid_err_t err;

void err(cwiid_wiimote_t* wiimote, const char* s, va_list ap)
{ 
   if (wiimote) printf("%d:", cwiid_get_id(wiimote));
   else printf("-1:");
   vprintf(s, ap);
   printf("\n");
}

/*
Prototype cwiid_callback with cwiid_callback_t, define it with the
actual type - this will cause a compile error (rather than some
undefined bizarre behavior) if cwiid_callback_t changes.

cwiid_mesg_callback_t has undergone a few changes lately, hopefully
this will be the last.  Some programs need to know which messages were
received simultaneously (e.g. for correlating accelerometer and IR
data), and the sequence number mechanism used previously proved
cumbersome, so we just pass an array of messages, all of which were
received at the same time.  The id is to distinguish between multiple
wiimotes using the same callback.
*/

cwiid_mesg_callback_t cwiid_callback;

void cwiid_callback(cwiid_wiimote_t *wiimote, int mesg_count,
	union cwiid_mesg mesg[], struct timespec *timestamp)
{
   int i, j;
   int valid_source;

   for (i=0; i < mesg_count; i++)
   {
      switch (mesg[i].type) {
         case CWIID_MESG_STATUS:
            printf("Status Report: battery=%d extension=",
            mesg[i].status_mesg.battery);
            switch (mesg[i].status_mesg.ext_type) {
               case CWIID_EXT_NONE:
                  printf("none");
                  break;
               case CWIID_EXT_NUNCHUK:
                  printf("Nunchuk");
                  break;
               case CWIID_EXT_CLASSIC:
                  printf("Classic Controller");
                  break;
               default:
                  printf("Unknown Extension");
                  break;
            }
            printf("\n");
            break;
         case CWIID_MESG_BTN:
            printf("Button Report: %.4X\n", mesg[i].btn_mesg.buttons);
            break;
         case CWIID_MESG_ACC:
            printf("Acc Report: x=%d, y=%d, z=%d\n",
            mesg[i].acc_mesg.acc[CWIID_X],
            mesg[i].acc_mesg.acc[CWIID_Y],
            mesg[i].acc_mesg.acc[CWIID_Z]);
            break;
         case CWIID_MESG_IR:
            printf("IR Report: ");
            valid_source = 0;
            for (j = 0; j < CWIID_IR_SRC_COUNT; j++) {
               if (mesg[i].ir_mesg.src[j].valid) {
                  valid_source = 1;
                  printf("(%d,%d) ", mesg[i].ir_mesg.src[j].pos[CWIID_X],
                  mesg[i].ir_mesg.src[j].pos[CWIID_Y]);
               }
            }
            if (!valid_source) {
               printf("no sources detected");
            }
            printf("\n");
            break;
         case CWIID_MESG_NUNCHUK:
            printf("Nunchuk Report: btns=%.2X stick=(%d,%d) acc.x=%d acc.y=%d "
            "acc.z=%d\n", mesg[i].nunchuk_mesg.buttons,
            mesg[i].nunchuk_mesg.stick[CWIID_X],
            mesg[i].nunchuk_mesg.stick[CWIID_Y],
            mesg[i].nunchuk_mesg.acc[CWIID_X],
            mesg[i].nunchuk_mesg.acc[CWIID_Y],
            mesg[i].nunchuk_mesg.acc[CWIID_Z]);
            break;
         case CWIID_MESG_CLASSIC:
            printf("Classic Report: btns=%.4X l_stick=(%d,%d) r_stick=(%d,%d) "
            "l=%d r=%d\n", mesg[i].classic_mesg.buttons,
            mesg[i].classic_mesg.l_stick[CWIID_X],
            mesg[i].classic_mesg.l_stick[CWIID_Y],
            mesg[i].classic_mesg.r_stick[CWIID_X],
            mesg[i].classic_mesg.r_stick[CWIID_Y],
            mesg[i].classic_mesg.l, mesg[i].classic_mesg.r);
            break;
         case CWIID_MESG_ERROR:
            if (cwiid_close(wiimote)) {
               fprintf(stderr, "Error on wiimote disconnect\n");
               exit(-1);
            }
            exit(0);
            break;
         default:
            printf("Unknown Report");
            break;
      }
   }
}

// ---------------------------------------------------------------------
// Initialization, constructor and destructor methods:

void wiimote::allocate_member_objects()
{
}

void wiimote::initialize_member_objects()
{
   rpt_mode=0;
   t_previous_button_click=t_previous_button_value=NEGATIVEINFINITY;
   min_dt_between_button_clicks=3;	// secs
   min_dt_between_button_values=0.1;	// secs
   CM_3D_ptr=NULL;
   OBSFRUSTAGROUP_ptr=NULL;
   PointCloudsGroup_ptr=NULL;
}		 

// ---------------------------------------------------------------------
wiimote::wiimote()
{
   initialize_member_objects();
   allocate_member_objects();
}

// ---------------------------------------------------------------------
// Copy constructor:

wiimote::wiimote(const wiimote& w)
{
//   cout << "inside wiimote copy constructor, this(wiimote) = " << this << endl;
   initialize_member_objects();
   allocate_member_objects();
   docopy(w);
}

wiimote::~wiimote()
{
}

// ---------------------------------------------------------------------
void wiimote::docopy(const wiimote& w)
{
//   cout << "inside wiimote::docopy()" << endl;
//   cout << "this = " << this << endl;
}

// Overload = operator:

wiimote& wiimote::operator= (const wiimote& w)
{
//   cout << "inside wiimote::operator=" << endl;
//   cout << "this(wiimote) = " << this << endl;
   if (this==&w) return *this;
   docopy(w);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

   ostream& operator<< (ostream& outstream,const wiimote& w)
   {
      outstream << endl;
//   outstream << "Wiimote ID = " << w.ID << endl;
      outstream << endl << endl;
   
      return outstream;
   }

// =========================================================================
// Set & get member functions

void wiimote::set_rpt_mode(cwiid_wiimote_t* wiimote, unsigned char rpt_mode)
{
   if (cwiid_set_rpt_mode(wiimote, rpt_mode)) 
   {
      cout << "Error setting report mode" << endl;
   }
}

int wiimote::get_n_IR_sources() const
{
   return CWIID_IR_SRC_COUNT;
}

bool wiimote::get_IR_source_detected_flag(int i) const
{
   return static_cast<bool>(state.ir_src[i].valid);
}

twovector wiimote::get_IR_source_posn(int i) const
{
   return twovector(
      state.ir_src[i].pos[CWIID_X],state.ir_src[i].pos[CWIID_Y]);
}

int wiimote::get_IR_source_size(int i) const
{
   return static_cast<int>(state.ir_src[i].size);
}

/*
// ---------------------------------------------------------------------
// Member function get_button_clicked_flag() 

bool wiimote::get_button_clicked_flag() 
{
   bool button_clicked_flag=false;
   if (CM_3D_ptr != NULL)
   {
      CM_3D_ptr->set_hmi_select_flag(false);
   }

   double curr_t=timefunc::elapsed_timeofday_time();
   
// Ignore any Wii input made less than delta_t since previous input:

   double dt=curr_t-t_previous_button_click;
   if (dt < min_dt_between_button_clicks) return button_clicked_flag;

   int button_value=static_cast<int>(state.buttons);
   if (button_value > 0)
   {
      cout << "Clicked button value = " << button_value << endl;
      t_previous_button_click=curr_t;
      if (CM_3D_ptr != NULL)
      {
         CM_3D_ptr->set_hmi_select_flag(true);
         button_clicked_flag=true;
      }
   }
   return button_clicked_flag;
}
*/

// ---------------------------------------------------------------------
// Member function get_curr_button_value() 

int wiimote::get_curr_button_value() 
{
   int button_value=-1;
   if (CM_3D_ptr != NULL)
   {
      CM_3D_ptr->set_hmi_select_flag(false);
      CM_3D_ptr->set_hmi_select_value(-1);
   }

   double curr_t=timefunc::elapsed_timeofday_time();
   
// Ignore any Wii input made less than delta_t since previous input:

   double dt=curr_t-t_previous_button_value;
   if (dt < min_dt_between_button_values) return button_value;

   button_value=static_cast<int>(state.buttons);
   if (button_value > 0)
   {
      cout << "Clicked button value = " << button_value << endl;
      t_previous_button_value=curr_t;
      if (CM_3D_ptr != NULL)
      {
         CM_3D_ptr->set_hmi_select_flag(true);
         CM_3D_ptr->set_hmi_select_value(button_value);
      }
   }

   return button_value;
}

// =========================================================================
void wiimote::initialize_wiimote()
{
   cwiid_set_err(err);

// As of 6/5/12, we have found that this next line yields the
// following compiler error

// error: taking address of temporary [-fpermissive]

//   bdaddr = *BDADDR_ANY;

// So as of June 2012, we have to give up supporting Wiimote
// functionality...
   
   string banner=
	"Put Wiimote into discoverable mode now by simultaneously pressing its lowest 1 & 2 buttons:";
   outputfunc::write_big_banner(banner);

   if (!(wiimote_ptr = cwiid_open(&bdaddr, 0))) 
   {
      cout << "Unable to connect to wiimote" << endl;
      exit(-1);
   }

   if (cwiid_set_mesg_callback(wiimote_ptr, cwiid_callback)) 
   {
      cout << "Unable to set message callback" << endl;
   }

// Toggle button output from wiimote:

   toggle_bit(rpt_mode, CWIID_RPT_BTN);
   set_rpt_mode(wiimote_ptr, rpt_mode);

// Toggle IR output from wiimote:

// libwiimote picks the highest quality IR mode available with the
// other options selected (not including as-yet-undeciphered
// interleaved mode 

   toggle_bit(rpt_mode, CWIID_RPT_IR);
   set_rpt_mode(wiimote_ptr, rpt_mode);
}

// ---------------------------------------------------------------------
bool wiimote::update_state()
{
   bool flag=cwiid_get_state(wiimote_ptr, &state);
   if (!flag)
   {
//      cout << "Trouble in wiimote::update_state()" << endl;
   }
}

// ---------------------------------------------------------------------
void wiimote::print_state()
{
   struct cwiid_state* state_ptr=&state;

   printf("Report Mode:");
   if (state_ptr->rpt_mode & CWIID_RPT_STATUS) printf(" STATUS");
   if (state_ptr->rpt_mode & CWIID_RPT_BTN) printf(" BTN");
   if (state_ptr->rpt_mode & CWIID_RPT_ACC) printf(" ACC");
   if (state_ptr->rpt_mode & CWIID_RPT_IR) printf(" IR");
   if (state_ptr->rpt_mode & CWIID_RPT_NUNCHUK) printf(" NUNCHUK");
   if (state_ptr->rpt_mode & CWIID_RPT_CLASSIC) printf(" CLASSIC");
   printf("\n");
	
   printf("Active LEDs:");
   if (state_ptr->led & CWIID_LED1_ON) printf(" 1");
   if (state_ptr->led & CWIID_LED2_ON) printf(" 2");
   if (state_ptr->led & CWIID_LED3_ON) printf(" 3");
   if (state_ptr->led & CWIID_LED4_ON) printf(" 4");
   printf("\n");

   printf("Rumble: %s\n", state_ptr->rumble ? "On" : "Off");

   printf("Battery: %d%%\n",
   (int)(100.0 * state_ptr->battery / CWIID_BATTERY_MAX));



   printf("Acc: x=%d y=%d z=%d\n", 
   state_ptr->acc[CWIID_X], state_ptr->acc[CWIID_Y],
   state_ptr->acc[CWIID_Z]);

   printf("Buttons: %X\n", state_ptr->buttons);

   bool valid_source = false;

   printf("IR: ");
   for (int i = 0; i < CWIID_IR_SRC_COUNT; i++) 
   {
      if (state_ptr->ir_src[i].valid) 
      {
         valid_source = true;
         cout << "IR source: valid = " << state_ptr->ir_src[i].valid
              << " X = " << state_ptr->ir_src[i].pos[CWIID_X]
              << " Y = " << state_ptr->ir_src[i].pos[CWIID_Y]
              << " size = " << static_cast<int>(state_ptr->ir_src[i].size) 
              << endl;
      }
   }
   if (!valid_source) 
   {
      cout << "No IR sources detected" << endl;
   }

/*
   switch (state_ptr->ext_type) 
   {
      case CWIID_EXT_NONE:
         printf("No extension\n");
         break;
      case CWIID_EXT_UNKNOWN:
         printf("Unknown extension attached\n");
         break;
      case CWIID_EXT_NUNCHUK:
         printf("Nunchuk: btns=%.2X stick=(%d,%d) acc.x=%d acc.y=%d "
         "acc.z=%d\n", state_ptr->ext.nunchuk.buttons,
         state_ptr->ext.nunchuk.stick[CWIID_X],
         state_ptr->ext.nunchuk.stick[CWIID_Y],
         state_ptr->ext.nunchuk.acc[CWIID_X],
         state_ptr->ext.nunchuk.acc[CWIID_Y],
         state_ptr->ext.nunchuk.acc[CWIID_Z]);
         break;
      case CWIID_EXT_CLASSIC:
         printf("Classic: btns=%.4X l_stick=(%d,%d) r_stick=(%d,%d) "
         "l=%d r=%d\n", state_ptr->ext.classic.buttons,
         state_ptr->ext.classic.l_stick[CWIID_X],
         state_ptr->ext.classic.l_stick[CWIID_Y],
         state_ptr->ext.classic.r_stick[CWIID_X],
         state_ptr->ext.classic.r_stick[CWIID_Y],
         state_ptr->ext.classic.l, state_ptr->ext.classic.r);
         break;
   }
*/

}

// =========================================================================
// Custom3DManipulator manipulation via Wii member functions
// =========================================================================

// Member function check_for_ultrasound_control_input() was designed
// in Aug 2011 for manipulating Laura Brattain's 3D ultrasound imagery
// via a Wiimote.

void wiimote::check_for_ultrasound_control_input() 
{
   if (CM_3D_ptr==NULL) return;

   int button_value=static_cast<int>(state.buttons);

   bool home_flag=false;
   double delta_az=0;
   double delta_el=0;
   double ds=0;
   if (button_value==512)
   {
      cout << "Right cross button clicked" << endl;
      delta_az=0.5*PI/180;
   }
   else if (button_value==256)
   {
      cout << "Left cross button clicked" << endl;
      delta_az=-0.5*PI/180;
   }
   else if (button_value==2048)
   {
      cout << "Up cross button clicked" << endl;
      delta_el=0.5*PI/180;
   }
   else if (button_value==1024)
   {
      cout << "Down cross button clicked" << endl;
      delta_el=-0.5*PI/180;
   }
   else if (button_value==4096)
   {
      cout << "+ button clicked" << endl;
      ds=-0.001;
   }
   else if (button_value==16)
   {
      cout << "- button clicked" << endl;
      ds=0.001;
   }
   else if (button_value==8)
   {
      cout << "clear A button clicked" << endl;
      home_flag=true;
   }
   else if (button_value==128)
   {
      cout << "home button clicked" << endl;
      home_flag=true;
   }
   else if (button_value==2)
   {
      cout << "1 button clicked" << endl;
//      if (PointCloudsGroup_ptr != NULL)
//         PointCloudsGroup_ptr->increase_min_threshold();
      if (PointCloudsGroup_ptr != NULL)
         PointCloudsGroup_ptr->decrease_max_threshold();
   }
   else if (button_value==1)
   {
      cout << "2 button clicked" << endl;
//      if (PointCloudsGroup_ptr != NULL)
//         PointCloudsGroup_ptr->decrease_min_threshold();
      if (PointCloudsGroup_ptr != NULL)
         PointCloudsGroup_ptr->increase_max_threshold();
   }

   CM_3D_ptr->set_mouse_input_device_flag(false);
   if (!nearly_equal(delta_az,0) || !nearly_equal(delta_el,0))
   {
      CM_3D_ptr->reset_az_el(delta_az,delta_el);
   }
   if (!nearly_equal(ds,0))
   {
      CM_3D_ptr->reset_zoom(ds);
   }
   if (home_flag)
   {
      CM_3D_ptr->reset_view_to_home();
   }
   

   CM_3D_ptr->set_mouse_input_device_flag(true);
}

// ---------------------------------------------------------------------
// Member function check_for_NYC_1Kdemo_control_input() was designed
// in Sep 2011 for manipulating the NYC 1K demo via a Wiimote for
// Family Day 2011.

void wiimote::check_for_NYC_1Kdemo_control_input() 
{
   if (CM_3D_ptr==NULL) return;
   if (!CM_3D_ptr->get_active_control_flag()) return;

   int button_value=button_value=static_cast<int>(state.buttons);
   bool rotate_about_current_eyepoint_flag=
      CM_3D_ptr->get_rotate_about_current_eyepoint_flag();

   bool home_flag=false;
   bool flyin_flag=false;
   bool flyout_flag=false;
   double fx_curr=0;
   double fx_prev=0;
   double fy_curr=0;
   double fy_prev=0;
   double delta_az=0;
   double ds=0;

   const double f_curr=0.020;
   const double ds0=0.03;
   const double daz0=1.0*PI/180;

//   cout << "Active control flag = "
//        << CM_3D_ptr->get_active_control_flag() << endl;
   if (button_value==512)
   {
      cout << "Right cross button clicked" << endl;
      if (rotate_about_current_eyepoint_flag)
      {
         OBSFRUSTAGROUP_ptr->move_to_right_OBSFRUSTUM_neighbor();
      }
      else
      {
         fx_curr=-f_curr;
         fx_prev=0;
      }
   }
   else if (button_value==256)
   {
      cout << "Left cross button clicked" << endl;
      if (rotate_about_current_eyepoint_flag)
      {
         OBSFRUSTAGROUP_ptr->move_to_left_OBSFRUSTUM_neighbor();
      }
      else
      {
         fx_curr=f_curr;
         fx_prev=0;
      }
   }
   else if (button_value==2048)
   {
      cout << "Up cross button clicked" << endl;
      if (rotate_about_current_eyepoint_flag)
      {
         vector<Movie*> Movie_ptrs=
            OBSFRUSTAGROUP_ptr->find_Movies_in_OSGsubPAT();
         for (int i=0; i<int(Movie_ptrs.size()); i++)
         {
            Movie* Movie_ptr=Movie_ptrs[i];
            if (Movie_ptr != NULL)
            {
               double alpha=Movie_ptr->get_alpha();
               cout << "alpha = " << alpha << endl;
               alpha=basic_math::min(1.0,alpha+0.05);
               Movie_ptr->set_alpha(alpha);
            }
         } // loop over Movies within Movie_ptrs
      }
      else
      {
         fy_curr=-f_curr;
         fy_prev=0;
      }
   }
   else if (button_value==1024)
   {
      cout << "Down cross button clicked" << endl;
      if (rotate_about_current_eyepoint_flag)
      {
         vector<Movie*> Movie_ptrs=
            OBSFRUSTAGROUP_ptr->find_Movies_in_OSGsubPAT();
         for (int i=0; i<int(Movie_ptrs.size()); i++)
         {

            Movie* Movie_ptr=Movie_ptrs[i];
            if (Movie_ptr != NULL)
            {
               double alpha=Movie_ptr->get_alpha();
               cout << "alpha = " << alpha << endl;
               alpha=basic_math::max(0.0,alpha-0.05);
               Movie_ptr->set_alpha(alpha);
            }
         }
      }
      else
      {
         fy_curr=f_curr;
         fy_prev=0;
      }
   }
   else if (button_value==4096)
   {
      cout << "+ button clicked" << endl;
      ds=-ds0;
   }
   else if (button_value==16)
   {
      cout << "- button clicked" << endl;
      ds=ds0;
   }
   else if (button_value==128)
   {
      cout << "home button clicked" << endl;
      home_flag=true;
   }
   else if (button_value==2 && !rotate_about_current_eyepoint_flag)
   {
      cout << "1 button clicked" << endl;

      delta_az=daz0;
   }
   else if (button_value==1 && !rotate_about_current_eyepoint_flag)
   {
      cout << "2 button clicked" << endl;
      delta_az=-daz0;
   }

   else if (button_value==8)
   {
      cout << "clear A button clicked" << endl;
      flyin_flag=true;
   }
   else if (button_value==4)
   {
      cout << "back button clicked" << endl;
      flyout_flag=true;
   }

   CM_3D_ptr->set_mouse_input_device_flag(false);
   if (!nearly_equal(fx_curr,0) || !nearly_equal(fy_curr,0))
   {
      CM_3D_ptr->reset_translation(fx_curr,fy_curr,fx_prev,fy_prev);
   }
   if (!nearly_equal(delta_az,0))
   {
      CM_3D_ptr->reset_az_el(delta_az,0);
   }
   if (!nearly_equal(ds,0) && !rotate_about_current_eyepoint_flag)
   {
      CM_3D_ptr->reset_zoom(ds);
   }

// Following Delsey's suggestion, allow user to press "home" button even when 
// virtual camera is flown into some OBSFRUSTUM's image plane.  But be
// sure to reset selected OBSFRUSTUM ID to -1 when home is pressed:

   if (home_flag)
   {
      CM_3D_ptr->set_rotate_about_current_eyepoint_flag(false);
      CM_3D_ptr->reset_view_to_home();
      int selected_OBSFRUSTUM_ID=OBSFRUSTAGROUP_ptr->
         get_selected_Graphical_ID();
      if (selected_OBSFRUSTUM_ID >= 0)
      {
         OBSFRUSTAGROUP_ptr->unerase_all_OBSFRUSTA();
      }
      OBSFRUSTAGROUP_ptr->set_selected_Graphical_ID(-1);
   }
   if (flyin_flag && OBSFRUSTAGROUP_ptr != NULL)
   {
      OBSFRUSTAGROUP_ptr->
         select_and_alpha_vary_OBSFRUSTUM_closest_to_screen_center();
   }
   if (flyout_flag && OBSFRUSTAGROUP_ptr != NULL)
   {
      OBSFRUSTAGROUP_ptr->deselect_OBSFRUSTUM();
   }

   CM_3D_ptr->set_mouse_input_device_flag(true);
}
