// ==========================================================================
// BOXESGROUP class member function definitions
// ==========================================================================
// Last modified on 2/10/08; 6/15/08; 12/2/11
// ==========================================================================

#include <iomanip>
#include <vector>
#include <osg/Geode>
#include <osg/Light>
#include <osg/LightModel>
#include <osg/LightSource>

#include "math/basic_math.h"
#include "osg/osgGeometry/BoxesGroup.h"
#include "color/colorfuncs.h"
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::ostream;
using std::setw;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void BoxesGroup::allocate_member_objects()
{
}		       

void BoxesGroup::initialize_member_objects()
{
   GraphicalsGroup_name="BoxesGroup";
   width=length=height=1.0;
   selected_face_displacement=0.01;
   size[3]=1.0;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<BoxesGroup>(
         this, &BoxesGroup::update_display));
}		       

BoxesGroup::BoxesGroup(Pass* PI_ptr,threevector* GO_ptr):
   GeometricalsGroup(3,PI_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

BoxesGroup::BoxesGroup(
   Pass* PI_ptr,threevector* GO_ptr,AnimationController* AC_ptr):
   GeometricalsGroup(3,PI_ptr,AC_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

BoxesGroup::BoxesGroup(
   Pass* PI_ptr,Clock* clock_ptr,Ellipsoid_model* EM_ptr,threevector* GO_ptr):
   GeometricalsGroup(3,PI_ptr,clock_ptr,EM_ptr,GO_ptr)
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

BoxesGroup::~BoxesGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const BoxesGroup& f)
{
   int node_counter=0;
   for (unsigned int n=0; n<f.get_n_Graphicals(); n++)
   {
      Box* Box_ptr=f.get_Box_ptr(n);
      outstream << "Box node # " << node_counter++ << endl;
      outstream << "Box = " << *Box_ptr << endl;
   }
   return(outstream);
}

// ==========================================================================
// Box creation and manipulation methods
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_Box from all other graphical insertion
// and manipulation methods...

Box* BoxesGroup::generate_new_Box(const threevector& V,int ID)
{
   Box* curr_Box_ptr=generate_new_canonical_Box(ID);
   initialize_Graphical(V,curr_Box_ptr);

   update_mybox(curr_Box_ptr);
   return curr_Box_ptr;
}

Box* BoxesGroup::generate_new_canonical_Box(int ID)
{
   if (ID==-1) ID=get_next_unused_ID();
   Box* curr_Box_ptr=new Box(
      width,length,height,selected_face_displacement,ID);
   GraphicalsGroup::insert_Graphical_into_list(curr_Box_ptr);
   insert_graphical_PAT_into_OSGsubPAT(curr_Box_ptr,0);

   osg::Group* group_ptr=curr_Box_ptr->generate_drawable_group();
   curr_Box_ptr->get_PAT_ptr()->addChild(group_ptr);

   int face_number=4;	// Initially select top face
   threevector origin(0,0,0);
   curr_Box_ptr->reset_selected_face_drawable(face_number,origin);
   update_mybox(curr_Box_ptr);
   reset_colors();

   return curr_Box_ptr;
}

// --------------------------------------------------------------------------
// Member function change_size multiplies the size parameter for
// Box objects corresponding to the current dimension by input
// parameter factor.

void BoxesGroup::change_size(double factor)
{   
   size[get_ndims()] *= factor;
   GraphicalsGroup::change_size(factor);
}

// --------------------------------------------------------------------------
// Member function change_color retrieves the ID for the currently
// selected Box graphical.  It then queries the user to enter a new
// color string for the selected Box in the console window.  This
// method changes the selected Box's permanent, intrinsic color to the
// user entered one.

void BoxesGroup::change_color()
{   
   int selected_ID=get_selected_Graphical_ID();
   Box* selected_Box_ptr=get_ID_labeled_Box_ptr(selected_ID);

   if (selected_Box_ptr != NULL)
   {
//      osg::Vec4 curr_color=selected_Box_ptr->get_color();
//      cout << "Selected box " << selected_ID 
//           << " has color = " << curr_color.x() << ","
//           << curr_color.y() << "," << curr_color.z() << endl;

      string colorstring;
      cout << "Enter new color for selected box " << selected_ID
           << endl;
      cin >> colorstring;

      double alpha=1.0;
      cout << "Enter alpha for selected box" << endl;
      cin >> alpha;
      
      colorfunc::Color c=colorfunc::string_to_color(colorstring);
      selected_Box_ptr->set_permanent_color(c);
      reset_colors();
   } // selected_Box_ptr != NULL conditional
}

// --------------------------------------------------------------------------
// Member function deselect_all_faces

void BoxesGroup::deselect_all_faces()
{   
   threevector Box_posn;
   for (unsigned int n=0; n<get_n_Graphicals(); n++)
   {
      Box* curr_box_ptr=get_Box_ptr(n);
      curr_box_ptr->get_selected_face_ptr()->set_curr_color(
         curr_box_ptr->get_permanent_color());
      curr_box_ptr->set_selected_face_number(-2);
//      curr_Box_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),Box_posn);
//      Box_posn  -= get_grid_world_origin();
//      curr_Box_ptr->reset_selected_face_drawable(selected_face,Box_posn);
   }
}

// --------------------------------------------------------------------------
// Member function move_z translates the current selected Box in the z
// direction and updates its mybox member's vertices.

void BoxesGroup::move_z(int sgn)
{
   Graphical* curr_Graphical_ptr=GraphicalsGroup::move_z(sgn);
   if (curr_Graphical_ptr != NULL)
   {
      update_mybox(dynamic_cast<Box*>(curr_Graphical_ptr));
   }
}

// --------------------------------------------------------------------------
// Member function update_mybox resets the corner vertices of the
// mybox member *Box_ptr to their current values relative to the grid
// world origin.

void BoxesGroup::update_mybox(Box* Box_ptr)
{
   if (Box_ptr != NULL)
   {
      threevector abs_Box_posn;
      Box_ptr->get_UVW_coords(get_curr_t(),get_passnumber(),abs_Box_posn);

// Compute Box's current position relative to grid world origin:

      threevector rel_Box_posn=abs_Box_posn-get_grid_world_origin();
//      cout << "Box posn relative to grid origin = " 
//           << rel_Box_posn << endl;
      mybox* mybox_ptr=Box_ptr->get_mybox_ptr();
      mybox_ptr->absolute_position(rel_Box_posn);
   } // Box_ptr != NULL conditional
}

// --------------------------------------------------------------------------
// Member function update_display()

void BoxesGroup::update_display()
{
   reset_colors();
   GraphicalsGroup::update_display();
}

// ==========================================================================
// Ascii feature file I/O methods:
// ==========================================================================

void BoxesGroup::save_info_to_file()
{
   outputfunc::write_banner("Saving box information to ascii file:");
   string output_filename="boxes.txt";
   ofstream outstream;
   filefunc::openfile(output_filename,outstream);

   for (unsigned int imagenumber=get_first_framenumber(); 
        imagenumber <= get_last_framenumber(); imagenumber++)
   {
      double curr_t=static_cast<double>(imagenumber);
      for (unsigned int n=0; n<get_n_Graphicals(); n++)
      {
         Box* Box_ptr=get_Box_ptr(n);
         mybox* curr_mybox_ptr=Box_ptr->get_mybox_ptr();

         threevector min_corner,max_corner;
         curr_mybox_ptr->locate_extremal_xyz_corners(min_corner,max_corner);
//            cout << "min_corner = " << min_corner
//                 << " max_corner = " << max_corner << endl;
         osg::Vec4 color=Box_ptr->get_permanent_color();
         
         const int column_width=11;
         outstream.setf(ios::showpoint);
         outstream << setw(3) << curr_t 
                   << setw(4) << Box_ptr->get_ID() 
                   << setw(3) << get_passnumber();
         outstream << setw(column_width) 
                   << stringfunc::number_to_string(min_corner.get(0),5)
                   << setw(column_width) 
                   << stringfunc::number_to_string(min_corner.get(1),5)
                   << setw(column_width) 
                   << stringfunc::number_to_string(min_corner.get(2),5);
         outstream << setw(column_width) 
                   << stringfunc::number_to_string(max_corner.get(0),5)
                   << setw(column_width) 
                   << stringfunc::number_to_string(max_corner.get(1),5)
                   << setw(column_width) 
                   << stringfunc::number_to_string(max_corner.get(2),5);
         outstream << setw(column_width) 
                   << stringfunc::number_to_string(color.r(),5)
                   << setw(column_width) 
                   << stringfunc::number_to_string(color.g(),5)
                   << setw(column_width) 
                   << stringfunc::number_to_string(color.b(),5)
                   << setw(column_width) 
                   << stringfunc::number_to_string(color.a(),5) << endl;
      } // loop over index n labeling Box
   } // loop over image numbers

   filefunc::closefile(output_filename,outstream);
}

// ---------------------------------------------------------------------
osg::Group* BoxesGroup::createBoxLight(const threevector& light_posn)
{
    osg::LightSource* BoxLightSource = new osg::LightSource;

    osg::Light* BoxLight = BoxLightSource->getLight();
    BoxLight->setPosition( osg::Vec4( light_posn.get(0),light_posn.get(1),
                                      light_posn.get(2),1.0));
//    BoxLight->setAmbient( osg::Vec4( 0.35f, 0.35f, 0.35f, 1.0f ) );
//    BoxLight->setAmbient( osg::Vec4( 0.5f, 0.5f, 0.5f, 1.0f ) );
//    BoxLight->setAmbient( osg::Vec4( 0.65f, 0.65f, 0.65f, 1.0f ) );
    BoxLight->setAmbient( osg::Vec4( 0.7f, 0.7f, 0.7f, 1.0f ) );
//    BoxLight->setAmbient( osg::Vec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
//    BoxLight->setAmbient( osg::Vec4( 0.9f, 0.9f, 0.9f, 1.0f ) );

//    BoxLight->setDiffuse( osg::Vec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
//    BoxLight->setDiffuse( osg::Vec4( 0.95f, 0.95f, 0.95f, 1.0f ) );

    BoxLightSource->setLight( BoxLight );
    BoxLightSource->setLocalStateSetModes( osg::StateAttribute::ON );
    BoxLightSource->getOrCreateStateSet()->setMode(
       GL_LIGHTING, osg::StateAttribute::ON);

    osg::LightModel* lightModel = new osg::LightModel;
//    lightModel->setAmbientIntensity(osg::Vec4(0.25f,0.25f,0.25f,0.5f));
//    lightModel->setAmbientIntensity(osg::Vec4(0.5f,0.5f,0.5f,1.0f));
//    lightModel->setAmbientIntensity(osg::Vec4(0.75f,0.75f,0.75f,1.0f));
    lightModel->setAmbientIntensity(osg::Vec4(0.9f,0.9f,0.9f,1.0f));
    BoxLightSource->getOrCreateStateSet()->setAttribute(lightModel);


    return BoxLightSource;
}
    
