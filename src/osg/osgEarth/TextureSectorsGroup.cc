// ==========================================================================
// TEXTURESECTORSGROUP class member function definitions
// ==========================================================================
// Last modified on 8/21/08; 8/22/08; 5/7/09; 8/14/09
// ==========================================================================

#include <iostream>
#include <vector>
#include "osg/osg2D/MoviesGroup.h"
#include "video/texture_rectangle.h"
#include "osg/osgEarth/TextureSectorsGroup.h"

using std::cin;
using std::cout;
using std::endl;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void TextureSectorsGroup::allocate_member_objects()
{
}		       

void TextureSectorsGroup::initialize_member_objects()
{
   GraphicalsGroup_name="TextureSectorsGroup";
   curr_TextureSector_ID=-1;

   get_OSGgroup_ptr()->setUpdateCallback( 
      new AbstractOSGCallback<TextureSectorsGroup>(
         this, &TextureSectorsGroup::update_display));
}		       

TextureSectorsGroup::TextureSectorsGroup(Pass* PI_ptr,MoviesGroup* MG_ptr):
   GeometricalsGroup(3,PI_ptr)
{	
//   cout << "inside TextureSectorsGroup constructor" << endl;
   
   initialize_member_objects();
   allocate_member_objects();

   MoviesGroup_ptr=MG_ptr;
}		       

TextureSectorsGroup::~TextureSectorsGroup()
{
}

// ---------------------------------------------------------------------
// Overload << operator

ostream& operator<< (ostream& outstream,const TextureSectorsGroup& TSG)
{
   return(outstream);
}

// ==========================================================================
// TextureSector creation methods:
// ==========================================================================

// Following Vadim's advice on 7/18/05, we separate off member
// function generate_new_TextureSector from all other graphical
// insertion and manipulation methods...

TextureSector* TextureSectorsGroup::generate_new_TextureSector(
   Earth* Earth_ptr,int ID)
{
//   cout << "inside TextureSectorsGroup::generate_new_TextureSector()" << endl;
   if (ID==-1) ID=get_next_unused_ID();
   curr_TextureSector_ID=ID;

   TextureSector* curr_TextureSector_ptr=new TextureSector(ID,Earth_ptr);
   GraphicalsGroup::insert_Graphical_into_list(curr_TextureSector_ptr);
   insert_graphical_PAT_into_OSGsubPAT(curr_TextureSector_ptr,0);

   return curr_TextureSector_ptr;
}

// --------------------------------------------------------------------------
void TextureSectorsGroup::generate_TextureSector_geode(
   TextureSector* TextureSector_ptr,string texture_filename)
{
   cout << "inside TextureSectorsGroup::generate_TextureSector_geode()" 
        << endl;
   cout << "texture_filename = " << texture_filename << endl;
   cout << "MoviesGroup_ptr = " << MoviesGroup_ptr << endl;

   vector<threevector> video_corner_vertices;

   if (MoviesGroup_ptr != NULL)
   {
      video_corner_vertices=
         MoviesGroup_ptr->get_pass_ptr()->get_PassInfo_ptr()->
         get_video_corner_vertices();
   }
   generate_TextureSector_geode(video_corner_vertices,TextureSector_ptr,
                                texture_filename);
}

// --------------------------------------------------------------------------
void TextureSectorsGroup::generate_TextureSector_geode(
   const vector<threevector>& video_corner_vertices,
   TextureSector* TextureSector_ptr,string texture_filename)
{
   cout << "inside TextureSectorsGroup::generate_TextureSector_geode() #2" 
        << endl;
   cout << "texture_filename = " << texture_filename << endl;
   cout << "video_corner_vertices.size() = "
        << video_corner_vertices.size() << endl;

   if (video_corner_vertices.size() > 0)
   {
      TextureSector_ptr->push_back_video_corner_vertices(
         video_corner_vertices);
      generate_TextureSector_texture_rectangle(
         TextureSector_ptr,texture_filename);
   }
   else
   {
      osg::Geode* geode_ptr=TextureSector_ptr->generate_drawable_geode(
         texture_filename);
      TextureSector_ptr->get_PAT_ptr()->addChild(geode_ptr);
   }
   
// Recall that as of July 2007, we store relative vertex information
// with respect to the average of the TextureSector's corner positions
// within *TextureSector_ptr to avoid floating point problems.  So we
// need to translate the TextureSector by its corners_average_location
// in order to globally position it:

//   TextureSector_ptr->compute_corners_avg_location(video_corner_vertices);

   TextureSector_ptr->set_average_posn(get_curr_t(),get_passnumber());
}

// --------------------------------------------------------------------------
Movie* TextureSectorsGroup::generate_TextureSector_video_chip(
   TextureSector* TextureSector_ptr,double alpha)
{
//   cout << "inside TextureSectorsGroup::generate_TextureSector_video_chip #1()"
//        << endl;
   
   Movie* Movie_ptr=generate_TextureSector_Movie(TextureSector_ptr,alpha);
//   cout << "Movie_ptr->get_video_filename() = "
//        << Movie_ptr->get_video_filename() << endl;
   
//   cout << "Movie_ptr = " << Movie_ptr << endl;
//      bool force_display_flag=true;
//      Movie_ptr->adjust_depth_buffering(force_display_flag,geode_ptr);

   generate_TextureSector_video_chip(Movie_ptr,TextureSector_ptr);
   return Movie_ptr;
}

// --------------------------------------------------------------------------
void TextureSectorsGroup::generate_TextureSector_video_chip(
   Movie* Movie_ptr,TextureSector* TextureSector_ptr)
{
//   cout << "inside TextureSectorsGroup::generate_TextureSector_video_chip() #2"
//        << endl;
   
   TextureSector_ptr->get_PAT_ptr()->addChild(Movie_ptr->get_PAT_ptr());

// Chip out sub-image(s) from full video if video_corner_vertices are
// defined in input package and longitude-latitude bbox is specified
// here:

   TextureSector_ptr->initialize_video_chip(Movie_ptr);

// Recall that as of July 2007, we store relative vertex information
// with respect to the average of the TextureSector's corner positions
// within *TextureSector_ptr to avoid floating point problems.  So we
// need to translate the TextureSector by its corners_average_location
// in order to globally position it:

   TextureSector_ptr->set_average_posn(get_curr_t(),get_passnumber());
}

// --------------------------------------------------------------------------
texture_rectangle* 
TextureSectorsGroup::generate_TextureSector_texture_rectangle(
   TextureSector* TextureSector_ptr,string texture_filename)
{
//   cout << "inside TextureSectorsGroup::generate_TextureSector_texture_rectangle()" 
//        << endl;
//   cout << "texture_filename = " << texture_filename << endl;

   if (MoviesGroup_ptr == NULL)
   {
      cout << "Error in TextureSectorsGroup::generate_TextureSector_texture_rectangle()!" << endl;
      cout << "MoviesGroup_ptr = NULL!" << endl;
      return NULL;
   }

   texture_rectangle* texture_rectangle_ptr=
      MoviesGroup_ptr->generate_new_texture_rectangle(texture_filename);

   TextureSector_ptr->set_texture_rectangle_ptr(texture_rectangle_ptr);
   return texture_rectangle_ptr;
}

// --------------------------------------------------------------------------
Movie* TextureSectorsGroup::generate_TextureSector_Movie(
   TextureSector* TextureSector_ptr,double alpha)
{
//   cout << "inside TextureSectorsGroup::generate_TextureSector_Movie()" 
//        << endl;
//   cout << "*texture_rectangle_ptr() = " << 
//      *(TextureSector_ptr->get_texture_rectangle_ptr()) << endl;
//   cout << "alpha = " << alpha << endl;

   if (MoviesGroup_ptr == NULL)
   {
      cout << "Error in TextureSectorsGroup::generate_TextureSector_Movie()!" 
           << endl;
      cout << "MoviesGroup_ptr = NULL!" << endl;
      return NULL;
   }

   Movie* movie_ptr=MoviesGroup_ptr->generate_new_Movie(
      TextureSector_ptr->get_texture_rectangle_ptr(),alpha);
   TextureSector_ptr->push_back_Movie_ptr(movie_ptr);
   return movie_ptr;
}

// --------------------------------------------------------------------------
void TextureSectorsGroup::destroy_TextureSector_Movie(
   TextureSector* TextureSector_ptr,Movie* Movie_ptr)
{
//   cout << "inside TextureSectorsGroup::destroy_TextureSector_Movie()"
//        << endl;
   TextureSector_ptr->get_PAT_ptr()->removeChild(Movie_ptr->get_PAT_ptr());
   TextureSector_ptr->pop_off_Movie_ptr(Movie_ptr);
   MoviesGroup_ptr->destroy_Movie(Movie_ptr);
}

// ==========================================================================
// Animation member functions
// ==========================================================================

// Member function update_display()

void TextureSectorsGroup::update_display()
{
//   cout << "inside TextureSectorsGroup::update_display()" << endl;

//   for (int n=0; n<get_n_Graphicals(); n++)
//   {
//      update_TextureSector_video_chips(get_TextureSector_ptr(n));
//   }

   GraphicalsGroup::update_display();
}

/*
// --------------------------------------------------------------------------
// Member function update_TextureSector_video_chips is an experimental
// method created for algorithm development purposes only.

void TextureSectorsGroup::update_TextureSector_video_chips(
   TextureSector* TextureSector_ptr)
{
//   cout << "inside TextureSectorsGroup::update_TextureSector_video_chips()"
//        << endl;
//   cout << "TextureSector_ptr = " << TextureSector_ptr << endl;

   if (TextureSector_ptr->get_Movie_ptrs().size()==0) return;
   
   AnimationController* AC_ptr=get_AnimationController_ptr();

   threevector bottom_left(224850.6743 , 3709063.364);	// real
//   threevector bottom_left(227850.6743 , 3711063.364);   // fake
//   threevector bottom_left(0,0);   // fake

//   threevector bottom_right(229077.4406 , 3709053.427); // real
   threevector bottom_right(231077.4406 , 3708053.427); // fake
//   threevector bottom_right(0,10); // fake

   threevector top_right(229068.8468 , 3713283.141);	// real
//   threevector top_right(10,10); // real

//   threevector top_left(224842.0804 , 3713293.078);  // real
   threevector top_left(227842.0804 , 3722293.078);  // fake
//   threevector top_left(0,10);  // fake

   int imagenumber=3*AC_ptr->get_curr_framenumber();
   bottom_left += threevector(imagenumber,2*imagenumber);
   bottom_right += threevector(imagenumber,imagenumber);
   top_right += threevector(imagenumber,2*imagenumber);
   top_left += threevector(imagenumber,imagenumber);

   vector<threevector> BL_BR_TR_TL_posns;
   BL_BR_TR_TL_posns.push_back(bottom_left);
   BL_BR_TR_TL_posns.push_back(bottom_right);
   BL_BR_TR_TL_posns.push_back(top_right);
   BL_BR_TR_TL_posns.push_back(top_left);
   
   vector< vector<threevector> > multi_BL_BR_TR_TL_posns;
   multi_BL_BR_TR_TL_posns.push_back(BL_BR_TR_TL_posns);

   TextureSector_ptr->update_movie_chips(multi_BL_BR_TR_TL_posns);
}
*/
