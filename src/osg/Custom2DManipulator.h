// ========================================================================
// Header file for Custom2DManipulator class
// ========================================================================
// Last updated on 8/19/07; 9/20/07; 9/2/08; 2/28/11; 8/9/16
// ========================================================================

#ifndef OSGGA_CUSTOM2DMANIPULATOR
#define OSGGA_CUSTOM2DMANIPULATOR 1

#include <string>
#include "osg/CustomManipulator.h"

class ModeController;
class WindowManager;

namespace osgGA {

   class OSGGA_EXPORT Custom2DManipulator : public osgGA::CustomManipulator
      {
        public:

         Custom2DManipulator(ModeController* MC_ptr,WindowManager* WM_ptr);

         virtual void reset_Manipulator_control();
         void maintain_rel_image_size(double curr_diag, double next_diag);

        protected:

         virtual ~Custom2DManipulator();

         virtual bool parse_mouse_events(const GUIEventAdapter& ea);
         virtual void Rotate(float px1,float py1,float px0,float py0);

        private:

         void allocate_member_objects();
         void initialize_member_objects();
      };

} // osgGA namespace

#endif

