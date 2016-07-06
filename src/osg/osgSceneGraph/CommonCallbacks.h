// ==========================================================================
// Header file for COMMONCALLBACKS class
// ==========================================================================
// Last modified on 12/2/11
// ==========================================================================

#ifndef COMMONCALLBACKS_H
#define COMMONCALLBACKS_H

#include <osg/NodeCallback>

class CommonCallbacks 
{
  public:
		
   CommonCallbacks();
   ~CommonCallbacks();

// Set & get member functions:
	
   osg::NodeCallback* get_CommonUpdateCallback_ptr();
   const osg::NodeCallback* get_CommonUpdateCallback_ptr() const;
   osg::NodeCallback* get_CommonCullCallback_ptr();
   const osg::NodeCallback* get_CommonCullCallback_ptr() const;

  private:

   osg::ref_ptr<osg::NodeCallback> commonUpdateCallback_refptr;
   osg::ref_ptr<osg::NodeCallback> commonCullCallback_refptr;

   void allocate_member_objects();
   void initialize_member_objects();
};



#endif // COMMONCALLBACKS_H
