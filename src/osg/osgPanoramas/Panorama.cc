// ==========================================================================
// Panorama class member function definitions
// ==========================================================================
// Last updated on 7/31/11; 8/16/11; 8/17/11
// ==========================================================================

#include "color/colorfuncs.h"
#include "osg/osgPanoramas/Panorama.h"
#include "math/rotation.h"

using std::cout;
using std::endl;
using std::ostream;
using std::string;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Panorama::allocate_member_objects()
{
}		       

void Panorama::initialize_member_objects()
{
   n_text_messages=1;
}		       

Panorama::Panorama(OBSFRUSTAGROUP* OFG_ptr,osgText::Font* f_ptr,int ID):
   Geometrical(3,ID)
{	
   allocate_member_objects();
   initialize_member_objects();
   font_refptr=f_ptr;

   OBSFRUSTAGROUP_ptr=OFG_ptr;
}		       

Panorama::~Panorama()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const Panorama& p)
{
//   outstream << "inside Panorama::operator<<" << endl;
   outstream << static_cast<const Geometrical&>(p) << endl;

   outstream << "Camera posn = " << p.get_posn() << endl;
   for (int i=0; i<p.get_n_OBSFRUSTA(); i++)
   {
      cout << "i = " << i << " OBSFRUSTUM_ID = " << p.OBSFRUSTA_IDs[i]
           << endl;
   }
   
   return outstream;
}

// ==========================================================================
// Set & get member functions
// ==========================================================================

void Panorama::pushback_OBSFRUSTUM_ID(int id)
{
   OBSFRUSTA_IDs.push_back(id);
}

// Member function get_OBSFRUSTUM_ptr() takes in index i which ranges
// from 1 to number_of_OBSFRUSTA_per_panorama.  

OBSFRUSTUM* Panorama::get_OBSFRUSTUM_ptr(int i)
{
   return OBSFRUSTAGROUP_ptr->get_ID_labeled_OBSFRUSTUM_ptr(
      get_OBSFRUSTUM_ID(i));
}

// ---------------------------------------------------------------------
// Member function get_OBSFRUSTUM_index() takes in the OBSFRUSTAGROUP
// ID for some OBSFRUSTUM within the current circular panorama.  It
// returns the panorama index ranging from 0 to get_n_OBSFRUSTA()-1
// for the input OBSFRUSTUM:

int Panorama::get_OBSFRUSTUM_index(int ID)
{
   int index=-1;
   for (int i=0; i<get_n_OBSFRUSTA(); i++)
   {
      if (get_OBSFRUSTUM_ID(i)==ID)
      {
         index=i;
      }
   }
   return index;
}

// ==========================================================================
// Drawing methods
// ==========================================================================

// Member function generate_drawable_geode instantiates an osg::Geode
// containing a single Panorama drawable.

osg::Geode* Panorama::generate_drawable_geode()
{
//   cout << "inside Panorama::generate_drawable_geode()" << endl;
   
   geode_refptr = new osg::Geode();
   generate_text(0,geode_refptr.get());
   return geode_refptr.get();
}

// ---------------------------------------------------------------------
// Member function generate_text()

void Panorama::generate_text(int i,osg::Geode* geode_ptr)
{
   cout << "inside Panorama::generate_text(), i = " << i << endl;

   osg::ref_ptr<osgText::Text> curr_text_refptr=new osgText::Text;
   text_refptr.push_back(curr_text_refptr);
   Geometrical::initialize_text(i);
   text_refptr[i]->setAxisAlignment(osgText::Text::SCREEN);


/*
   text_refptr[i]->setFont("fonts/times.ttf");

   text_refptr[i]->setAlignment(osgText::Text::CENTER_CENTER);
   text_refptr[i]->setBackdropType(osgText::Text::OUTLINE);
*/

/*
   text_refptr.push_back(static_cast<osgText::Text*>(NULL));
   text_refptr[i]=new osgText::Text;
   text_refptr[i]->setFont("fonts/times.ttf");
   text_refptr[i]->setCharacterSize(1);
   text_refptr[i]->setAxisAlignment(osgText::Text::SCREEN);

   text_refptr[i]->setAlignment(osgText::Text::CENTER_CENTER);
   text_refptr[i]->setBackdropType(osgText::Text::OUTLINE);
*/

   geode_ptr->addDrawable(text_refptr[i].get());
}

// ---------------------------------------------------------------------
// Member function generate_ID_label() adds a text label to
// the Panorama located at its center.  The ID text label is returned
// by this method.

string Panorama::generate_ID_label(double delta_z,double text_size)
{
//   cout << "inside Panorama::generate_ID_label()" << endl;

   string ID_label=stringfunc::number_to_string(get_ID());
//   cout << "ID_label = " << ID_label << endl;
   set_text_label(0,ID_label);

//   threevector label_posn(get_posn()+1*z_hat);
   threevector label_posn(get_posn()+delta_z*z_hat);
//   cout << "label_posn = " << label_posn << endl;
   set_text_posn(0,label_posn);
   set_text_size(0,text_size);
   set_text_color(0,colorfunc::cyan);

   return ID_label;
}

// ==========================================================================
// Imageplane proximity member functions
// ==========================================================================

// Member function find_closest_imageplane_to_world_posn() takes in
// world-space point XYZ and computes its distance and direction
// vector r_hat wrt the current Panorama's center.  It then loops over
// the OBSFRUSTA within the Panorama and returns the ID for the one
// whose pointing vector has maximal dotproduct with r_hat.  This
// method also returns the UV coordinates for the XYZ point within
// closest_UV.

double Panorama::find_closest_imageplane_to_world_posn(
   const threevector& XYZ,int& closest_OBSFRUSTUM_ID,twovector& closest_UV)
{
//   cout << "inside Panorama::find_closest_imageplane_to_world_posn()" << endl;
//   cout << "World posn XYZ = " << XYZ << endl;
//   cout << "Panorama posn = " << get_posn() << endl;

   double distance_to_camera=(XYZ-get_posn()).magnitude();
   closest_OBSFRUSTUM_ID=-1;
   double max_dotproduct=NEGATIVEINFINITY;

   threevector r_hat=(XYZ-get_posn()).unitvector();
//   cout << "r_hat = " << r_hat << endl;

   for (unsigned int n=0; n<OBSFRUSTA_IDs.size(); n++)
   {
      int curr_ID=OBSFRUSTA_IDs[n];
//      cout << "OBSFRUSTUM index n = " << n 
//           << " ID = " << curr_ID << endl;
      OBSFRUSTUM* OBSFRUSTUM_ptr=OBSFRUSTAGROUP_ptr->
         get_ID_labeled_OBSFRUSTUM_ptr(curr_ID);

      Movie* Movie_ptr=OBSFRUSTUM_ptr->get_Movie_ptr();
      camera* camera_ptr=Movie_ptr->get_camera_ptr();

// Perform pointing direction sanity check:

//      cout << "camera pointing dir = " << camera_ptr->get_pointing_dir()
//           << endl;
      
      double dotproduct=r_hat.dot(camera_ptr->get_pointing_dir());
//      cout << "dotproduct = " << dotproduct << endl;
      if (dotproduct < 0) continue;

      const genmatrix* P_ptr=camera_ptr->get_P_ptr();
      threevector UVW=(*P_ptr) * fourvector(XYZ,1);
      double U=UVW.get(0)/UVW.get(2);
      double V=UVW.get(1)/UVW.get(2);

      if (U > Movie_ptr->get_minU() && U < Movie_ptr->get_maxU() &&
          V > Movie_ptr->get_minV() && V < Movie_ptr->get_maxV())
      {

         if (dotproduct > max_dotproduct)
         {
            max_dotproduct=dotproduct;
            closest_OBSFRUSTUM_ID=curr_ID;
            closest_UV=twovector(U,V);
//            cout << "closest_OBSFRUSTUM_ID = " << closest_OBSFRUSTUM_ID 
//                 << endl;
         }
      } // U,V within valid ranges for OBSFRUSTUM n               
   } // loop over index n labeling OBSFRUSTA within current Panorama

//   cout << "closest_OBSFRUSTUM_ID = " << closest_OBSFRUSTUM_ID << endl;
//   cout << "distance_to_camera = " << distance_to_camera << endl;
//   cout << "closest_UV = " << closest_UV << endl;

//   outputfunc::enter_continue_char();

   return distance_to_camera;
}

// ---------------------------------------------------------------------
// Member function spin_panorama()

void Panorama::azimuthally_spin(double t,int pass_number,double phi)
{
   cout << "inside Panorama::azimuthally_spin()" << endl;

   double az,el,roll;
   rotation R;

   osg::Quat attitude(0,0,0,1);
   get_quaternion(t,pass_number,attitude);
   fourvector q_OSG(attitude.x(),attitude.y(),attitude.z(),attitude.w());

   R=R.rotation_corresponding_to_OSG_quat(q_OSG);
   R.az_el_roll_from_rotation(az,el,roll);
   az += phi;
   R=R.rotation_from_az_el_roll(az,el,roll);

   q_OSG=R.OSG_quat_corresponding_to_rotation();
   attitude=osg::Quat(q_OSG.get(0),q_OSG.get(1),q_OSG.get(2),q_OSG.get(3));
   
//   cout << "az = " << az*180/PI << " el = " << el*180/PI
//        << " roll = " << roll*180/PI << endl;

   set_quaternion(t,pass_number,attitude);
   set_PAT(t,pass_number);

//   update_PAT_attitude(attitude);

//   outputfunc::enter_continue_char();
}
