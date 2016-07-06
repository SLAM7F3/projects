
// Note added on 2/5/07: Can probably eliminate MoviesGroup_ptr member
// of this class

// ==========================================================================
// Header file for MOVIEPICKHANDLER class
// ==========================================================================
// Last modfied on 12/29/06; 1/3/07; 2/9/11
// ==========================================================================

#ifndef MOVIE_PICK_HANDLER_H
#define MOVIE_PICK_HANDLER_H

#include <iostream>
#include "datastructures/Linkedlist.h"
#include "osg/osgGraphicals/GraphicalPickHandler.h"

class Movie;
class MoviesGroup;
class ModeController;
class PointCloud;
class WindowManager;

class MoviePickHandler : public GraphicalPickHandler
{

  public: 

   MoviePickHandler(
      const int p_ndims,Pass* PI_ptr,
      osgGA::CustomManipulator* CM_ptr,MoviesGroup* MG_ptr,
      ModeController* MC_ptr,WindowManager* WCC_ptr);

// Movie generation, manipulation and annihiilation methods:

   virtual float get_max_distance_to_Graphical();
      
  protected:

   virtual ~MoviePickHandler();

   MoviesGroup* const get_MoviesGroup_ptr();
   Linkedlist<Movie*>* get_Movielist_ptr();

// Mouse event handling methods:

   virtual bool pick(const osgGA::GUIEventAdapter& ea);
   virtual bool drag(const osgGA::GUIEventAdapter& ea);
   virtual bool rotate(float oldX,float oldY,const osgGA::GUIEventAdapter& ea);
   virtual bool release();

  private:

   MoviesGroup* MoviesGroup_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline MoviesGroup* const MoviePickHandler::get_MoviesGroup_ptr()
{
   return MoviesGroup_ptr;
}

#endif // MoviePickHandler.h



