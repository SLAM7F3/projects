// ==========================================================================
// AnimationPathCreator header file 
// ==========================================================================
// Last modified on 10/11/05; 1/4/07; 9/20/07
// ==========================================================================

/*
 * AnimationPathCreator is the class which records all
 * the information about a path and is able to create
 * a new path file which can be played back in the
 * dataviewer.
 * Instructions:
 *   'w' adds new waypoints for the animation path
 *   'W' creates a new .path file from these points, enter in name
 */

#ifndef ANIMATION_PATH_CREATOR_H
#define ANIMATION_PATH_CREATOR_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <osgGA/GUIEventHandler>

class WindowManager;

class AnimationPathCreator : public osgGA::GUIEventHandler
{
  public:

   AnimationPathCreator(WindowManager* WM_ptr);

   //struct to keep track of all the pertinent camera
   //information for any waypoint
   typedef struct PathInfo
   {
         double time;
         double xcoord;
         double ycoord;
         double zcoord;
         double quatx; 
         double quaty; 
         double quatz; 
         double quatw; 
   };

   //adds the current camera configuration at a certain point
   //in time to the waypoint list
   void add_current_waypoint();

   //writes out the path that you have created
   void write_out_path_file(std::string filename);

   //method to add key handling for the animation path
   virtual bool handle(const osgGA::GUIEventAdapter& ea, 
                       osgGA::GUIActionAdapter& aa);
   virtual void accept(osgGA::GUIEventHandlerVisitor& v)
      { v.visit(*this); };

  private:

   WindowManager* WindowManager_ptr;

   //current time
   /* NOTE:   Time is currently implemented very minimally
    *         Each waypoint is always 5 seconds from one another
    *         Thus, if a user creates lots of waypoints spatially
    *         "close" to one another, the animation will play much
    *         slower through that area than it would if the user
    *         created points far away from one another.  Optimally,
    *         the user would be able to choose how many seconds are
    *         between each waypoint, but this would require lots of
    *         interaction between the console and the video window,
    *         which without widgets was just not the best idea.
    */
   double time;

   //The vector keeping track of all waypoints in the path

   std::vector<AnimationPathCreator::PathInfo*> path_vector; 

   void allocate_member_objects();
   void initialize_member_objects();
};

#endif
