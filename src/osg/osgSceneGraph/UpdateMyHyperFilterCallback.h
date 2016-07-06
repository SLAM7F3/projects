// ========================================================================
// Ross' UpdateMyHyperFilterCallback header file
// ========================================================================
// Last updated on 11/27/11; 12/2/11
// ========================================================================

#ifndef UPDATECOLORMAPCALLBACK_H
#define UPDATECOLORMAPCALLBACK_H

#include <osg/NodeCallback>
#include <osg/NodeVisitor>
#include "osg/osgSceneGraph/MyHyperFilter.h"

namespace model
{
   
   class UpdateMyHyperFilterCallback : public osg::NodeCallback
   {

     public:
		
      UpdateMyHyperFilterCallback(MyHyperFilter* hf_ptr);
      virtual ~UpdateMyHyperFilterCallback();

      std::string get_Callback_name() const;

      virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

     protected:
   
     private:

      std::string Callback_name;
      MyHyperFilter* MyHyperFilter_ptr;

      void allocate_member_objects();
      void initialize_member_objects();

      friend class MyHyperFilter;
   };

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

   inline std::string UpdateMyHyperFilterCallback::get_Callback_name() const
   {
      return Callback_name;
   }

} // namespace model

#endif // UpdateMyHyperFilterCallback.h
