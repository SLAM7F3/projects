/******************************************************************************\
* EventHandling                                                                *
* OSG event handling (using OSG Producer).                                     *
* Leandro Motta Barros (based on Tutorials from Terse Solutions)               *
\******************************************************************************/


#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <osgProducer/Viewer>
#include <osgGA/GUIEventHandler>



// - TranslateMouseButton ------------------------------------------------------
std::string TranslateMouseButton (int button)
{
   switch (button)
   {
      case osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON:
         return "Left";
      case osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON:
         return "Middle";
      case osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON:
         return "Right";
      default:
         return "Unknown";
   }
}



// - TranslateKey --------------------------------------------------------------
std::string TranslateKey (int key)
{
   switch (key)
   {
      case osgGA::GUIEventAdapter::KEY_Space:
         return "Space";
      case osgGA::GUIEventAdapter::KEY_BackSpace:
         return "BackSpace";
      case osgGA::GUIEventAdapter::KEY_Tab:
         return "Tab";
      case osgGA::GUIEventAdapter::KEY_Linefeed:
         return "Linefeed";
      case osgGA::GUIEventAdapter::KEY_Clear:
         return "Clear";
      case osgGA::GUIEventAdapter::KEY_Return:
         return "Return";
      case osgGA::GUIEventAdapter::KEY_Pause:
         return "Pause";
      case osgGA::GUIEventAdapter::KEY_Scroll_Lock:
         return "Scroll_Lock";
      case osgGA::GUIEventAdapter::KEY_Sys_Req:
         return "Sys_Req";
      case osgGA::GUIEventAdapter::KEY_Escape:
         return "Escape";
      case osgGA::GUIEventAdapter::KEY_Delete:
         return "Delete";
      case osgGA::GUIEventAdapter::KEY_Home:
         return "Home";
      case osgGA::GUIEventAdapter::KEY_Left:
         return "Left";
      case osgGA::GUIEventAdapter::KEY_Up:
         return "Up";
      case osgGA::GUIEventAdapter::KEY_Right:
         return "Right";
      case osgGA::GUIEventAdapter::KEY_Down:
         return "Down";
      case osgGA::GUIEventAdapter::KEY_Page_Up:
         return "Page_Up";
      case osgGA::GUIEventAdapter::KEY_Page_Down:
         return "Page_Down";
      case osgGA::GUIEventAdapter::KEY_End:
         return "End";
      case osgGA::GUIEventAdapter::KEY_Begin:
         return "Begin";
      case osgGA::GUIEventAdapter::KEY_Select:
         return "Select";
      case osgGA::GUIEventAdapter::KEY_Print:
         return "Print";
      case osgGA::GUIEventAdapter::KEY_Execute:
         return "Execute";
      case osgGA::GUIEventAdapter::KEY_Insert:
         return "Insert";
      case osgGA::GUIEventAdapter::KEY_Undo:
         return "Undo";
      case osgGA::GUIEventAdapter::KEY_Redo:
         return "Redo";
      case osgGA::GUIEventAdapter::KEY_Menu:
         return "Menu";
      case osgGA::GUIEventAdapter::KEY_Find:
         return "Find";
      case osgGA::GUIEventAdapter::KEY_Cancel:
         return "Cancel";
      case osgGA::GUIEventAdapter::KEY_Help:
         return "Help";
      case osgGA::GUIEventAdapter::KEY_Break:
         return "Break";
      case osgGA::GUIEventAdapter::KEY_Mode_switch:
         return "Mode_switch";
      case osgGA::GUIEventAdapter::KEY_Num_Lock:
         return "Num_Lock";
      case osgGA::GUIEventAdapter::KEY_KP_Space:
         return "KP_Space";
      case osgGA::GUIEventAdapter::KEY_KP_Tab:
         return "KP_Tab";
      case osgGA::GUIEventAdapter::KEY_KP_Enter:
         return "KP_Enter";
      case osgGA::GUIEventAdapter::KEY_KP_F1:
         return "KP_F1";
      case osgGA::GUIEventAdapter::KEY_KP_F2:
         return "KP_F2";
      case osgGA::GUIEventAdapter::KEY_KP_F3:
         return "KP_F3";
      case osgGA::GUIEventAdapter::KEY_KP_F4:
         return "KP_F4";
      case osgGA::GUIEventAdapter::KEY_KP_Home:
         return "KP_Home";
      case osgGA::GUIEventAdapter::KEY_KP_Left:
         return "KP_Left";
      case osgGA::GUIEventAdapter::KEY_KP_Up:
         return "KP_Up";
      case osgGA::GUIEventAdapter::KEY_KP_Right:
         return "KP_Right";
      case osgGA::GUIEventAdapter::KEY_KP_Down:
         return "KP_Down";
      case osgGA::GUIEventAdapter::KEY_KP_Page_Up:
         return "KP_Page_Up";
      case osgGA::GUIEventAdapter::KEY_KP_Page_Down:
         return "KP_Page_Down";
      case osgGA::GUIEventAdapter::KEY_KP_End:
         return "KP_End";
      case osgGA::GUIEventAdapter::KEY_KP_Begin:
         return "KP_Begin";
      case osgGA::GUIEventAdapter::KEY_KP_Insert:
         return "KP_Insert";
      case osgGA::GUIEventAdapter::KEY_KP_Delete:
         return "KP_Delete";
      case osgGA::GUIEventAdapter::KEY_KP_Equal:
         return "KP_Equal";
      case osgGA::GUIEventAdapter::KEY_KP_Multiply:
         return "KP_Multiply";
      case osgGA::GUIEventAdapter::KEY_KP_Add:
         return "KP_Add";
      case osgGA::GUIEventAdapter::KEY_KP_Separator:
         return "KP_Separator";
      case osgGA::GUIEventAdapter::KEY_KP_Subtract:
         return "KP_Subtract";
      case osgGA::GUIEventAdapter::KEY_KP_Decimal:
         return "KP_Decimal";
      case osgGA::GUIEventAdapter::KEY_KP_Divide:
         return "KP_Divide";
      case osgGA::GUIEventAdapter::KEY_KP_0:
         return "KP_0";
      case osgGA::GUIEventAdapter::KEY_KP_1:
         return "KP_1";
      case osgGA::GUIEventAdapter::KEY_KP_2:
         return "KP_2";
      case osgGA::GUIEventAdapter::KEY_KP_3:
         return "KP_3";
      case osgGA::GUIEventAdapter::KEY_KP_4:
         return "KP_4";
      case osgGA::GUIEventAdapter::KEY_KP_5:
         return "KP_5";
      case osgGA::GUIEventAdapter::KEY_KP_6:
         return "KP_6";
      case osgGA::GUIEventAdapter::KEY_KP_7:
         return "KP_7";
      case osgGA::GUIEventAdapter::KEY_KP_8:
         return "KP_8";
      case osgGA::GUIEventAdapter::KEY_KP_9:
         return "KP_9";
      case osgGA::GUIEventAdapter::KEY_F1:
         return "F1";
      case osgGA::GUIEventAdapter::KEY_F2:
         return "F2";
      case osgGA::GUIEventAdapter::KEY_F3:
         return "F3";
      case osgGA::GUIEventAdapter::KEY_F4:
         return "F4";
      case osgGA::GUIEventAdapter::KEY_F5:
         return "F5";
      case osgGA::GUIEventAdapter::KEY_F6:
         return "F6";
      case osgGA::GUIEventAdapter::KEY_F7:
         return "F7";
      case osgGA::GUIEventAdapter::KEY_F8:
         return "F8";
      case osgGA::GUIEventAdapter::KEY_F9:
         return "F9";
      case osgGA::GUIEventAdapter::KEY_F10:
         return "F10";
      case osgGA::GUIEventAdapter::KEY_F11:
         return "F11";
      case osgGA::GUIEventAdapter::KEY_F12:
         return "F12";
      case osgGA::GUIEventAdapter::KEY_F13:
         return "F13";
      case osgGA::GUIEventAdapter::KEY_F14:
         return "F14";
      case osgGA::GUIEventAdapter::KEY_F15:
         return "F15";
      case osgGA::GUIEventAdapter::KEY_F16:
         return "F16";
      case osgGA::GUIEventAdapter::KEY_F17:
         return "F17";
      case osgGA::GUIEventAdapter::KEY_F18:
         return "F18";
      case osgGA::GUIEventAdapter::KEY_F19:
         return "F19";
      case osgGA::GUIEventAdapter::KEY_F20:
         return "F20";
      case osgGA::GUIEventAdapter::KEY_F21:
         return "F21";
      case osgGA::GUIEventAdapter::KEY_F22:
         return "F22";
      case osgGA::GUIEventAdapter::KEY_F23:
         return "F23";
      case osgGA::GUIEventAdapter::KEY_F24:
         return "F24";
      case osgGA::GUIEventAdapter::KEY_F25:
         return "F25";
      case osgGA::GUIEventAdapter::KEY_F26:
         return "F26";
      case osgGA::GUIEventAdapter::KEY_F27:
         return "F27";
      case osgGA::GUIEventAdapter::KEY_F28:
         return "F28";
      case osgGA::GUIEventAdapter::KEY_F29:
         return "F29";
      case osgGA::GUIEventAdapter::KEY_F30:
         return "F30";
      case osgGA::GUIEventAdapter::KEY_F31:
         return "F31";
      case osgGA::GUIEventAdapter::KEY_F32:
         return "F32";
      case osgGA::GUIEventAdapter::KEY_F33:
         return "F33";
      case osgGA::GUIEventAdapter::KEY_F34:
         return "F34";
      case osgGA::GUIEventAdapter::KEY_F35:
         return "F35";
      case osgGA::GUIEventAdapter::KEY_Shift_L:
         return "Shift_L";
      case osgGA::GUIEventAdapter::KEY_Shift_R:
         return "Shift_R";
      case osgGA::GUIEventAdapter::KEY_Control_L:
         return "Control_L";
      case osgGA::GUIEventAdapter::KEY_Control_R:
         return "Control_R";
      case osgGA::GUIEventAdapter::KEY_Caps_Lock:
         return "Caps_Lock";
      case osgGA::GUIEventAdapter::KEY_Shift_Lock:
         return "Shift_Lock";
      case osgGA::GUIEventAdapter::KEY_Meta_L:
         return "Meta_L";
      case osgGA::GUIEventAdapter::KEY_Meta_R:
         return "Meta_R";
      case osgGA::GUIEventAdapter::KEY_Alt_L:
         return "Alt_L";
      case osgGA::GUIEventAdapter::KEY_Alt_R:
         return "Alt_R";
      case osgGA::GUIEventAdapter::KEY_Super_L:
         return "Super_L";
      case osgGA::GUIEventAdapter::KEY_Super_R:
         return "Super_R";
      case osgGA::GUIEventAdapter::KEY_Hyper_L:
         return "Hyper_L";
      case osgGA::GUIEventAdapter::KEY_Hyper_R:
         return "Hyper_R";
      default:
         return std::string() + static_cast<char>(key);
   }
}



// - TranslateModifiers --------------------------------------------------------
std::string TranslateModifiers (unsigned int mask)
{
   std::string ret;

   if (mask & osgGA::GUIEventAdapter::MODKEY_LEFT_SHIFT)
      ret += "Left shift   ";
   if (mask & osgGA::GUIEventAdapter::MODKEY_RIGHT_SHIFT)
      ret += "Right shift   ";
   if (mask & osgGA::GUIEventAdapter::MODKEY_LEFT_CTRL)
      ret += "Left control   ";
   if (mask & osgGA::GUIEventAdapter::MODKEY_RIGHT_CTRL)
      ret += "Right control   ";
   if (mask & osgGA::GUIEventAdapter::MODKEY_LEFT_ALT)
      ret += "Left alt   ";
   if (mask & osgGA::GUIEventAdapter::MODKEY_RIGHT_ALT)
      ret += "Right alt   ";
   if (mask & osgGA::GUIEventAdapter::MODKEY_LEFT_META)
      ret += "Left meta   ";
   if (mask & osgGA::GUIEventAdapter::MODKEY_RIGHT_META)
      ret += "Right meta   ";
   if (mask & osgGA::GUIEventAdapter::MODKEY_NUM_LOCK)
      ret += "Num lock   ";
   if (mask & osgGA::GUIEventAdapter::MODKEY_CAPS_LOCK)
      ret += "Caps lock   ";

   return ret;
}



// - FormatMouseCoords ---------------------------------------------------------
std::string FormatMouseCoords (const osgGA::GUIEventAdapter& ea)
{
   using boost::lexical_cast;

   const std::string ret = "Mouse position = ("
      + lexical_cast<std::string>(ea.getX()) + ", "
      + lexical_cast<std::string>(ea.getY())
      + ")\nNormalized = (" + lexical_cast<std::string>(ea.getXnormalized())
      + ", " + lexical_cast<std::string>(ea.getYnormalized()) + ')';
   return ret;
}



// - MyEventHandler ------------------------------------------------------------
class MyEventHandler: public osgGA::GUIEventHandler
{
   public:
      virtual bool handle (const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&)
      {
         switch (ea.getEventType())
         {
            case osgGA::GUIEventAdapter::NONE:
            {
               std::cout << "NONE\n\n";
               break;
            }

            case osgGA::GUIEventAdapter::PUSH:
            {
               std::cout << "PUSH mouse button: " << TranslateMouseButton (ea.getButton()) << '\n'
                         << "Modifiers: " << TranslateModifiers (ea.getModKeyMask()) << '\n';
               break;
            }

            case osgGA::GUIEventAdapter::RELEASE:
            {
               std::cout << "RELEASE mouse button: " << TranslateMouseButton (ea.getButton()) << '\n'
                         << "Modifiers: " << TranslateModifiers (ea.getModKeyMask()) << '\n';
               break;
            }

            case osgGA::GUIEventAdapter::DOUBLECLICK:
            {
               // doesn't seem to work...
               std::cout << "DOUBLECLICK mouse button " << TranslateMouseButton (ea.getButton()) << '\n'
                         << "Modifiers: " << TranslateModifiers (ea.getModKeyMask()) << '\n';

               break;
            }

            case osgGA::GUIEventAdapter::DRAG:
            {
               // mouse button is not to be trusted for drag events!
               std::cout << "DRAG mouse button "  << TranslateMouseButton (ea.getButton()) << '\n'
                         << "Modifiers: " << TranslateModifiers (ea.getModKeyMask()) << '\n';
               break;
            }

            case osgGA::GUIEventAdapter::MOVE:
            {
               std::cout << "MOVE\n" << FormatMouseCoords (ea) << '\n'
                         << "Modifiers: " << TranslateModifiers (ea.getModKeyMask()) << '\n';
               break;
            }

            case osgGA::GUIEventAdapter::KEYDOWN:
            {
               std::cout << "KEYDOWN key " << TranslateKey (ea.getKey()) << '\n'
                         << "Modifiers: " << TranslateModifiers (ea.getModKeyMask()) << '\n';
               break;
            }

            case osgGA::GUIEventAdapter::KEYUP:
            {
               std::cout << "KEYUP key " << TranslateKey (ea.getKey()) << '\n'
                         << "Modifiers: " << TranslateModifiers (ea.getModKeyMask()) << '\n';
               break;
            }

            case osgGA::GUIEventAdapter::FRAME:
            {
               // std::cout << "FRAME\n"; // once per frame...
               return false;
            }

            case osgGA::GUIEventAdapter::RESIZE:
            {
               std::cout << "RESIZE\n";  // doesn't seem to work...
               break;
            }

/*
            case osgGA::GUIEventAdapter::SCROLLUP:
            {
               std::cout << "SCROLLUP\n"
                         << "Modifiers: " << TranslateModifiers (ea.getModKeyMask()) << '\n';
               break;
            }

            case osgGA::GUIEventAdapter::SCROLLDOWN:
            {
               std::cout << "SCROLLDOWN\n"
                         << "Modifiers: " << TranslateModifiers (ea.getModKeyMask()) << '\n';
               break;
            }

            case osgGA::GUIEventAdapter::SCROLLLEFT:
            {
               std::cout << "SCROLLLEFT\n"
                         << "Modifiers: " << TranslateModifiers (ea.getModKeyMask()) << '\n';
               break;
            }

            case osgGA::GUIEventAdapter::SCROLLRIGHT:
            {
               std::cout << "SCROLLRIGHT\n"
                         << "Modifiers: " << TranslateModifiers (ea.getModKeyMask()) << '\n';
               break;
            }
*/

            default:
            {
               std::cout << "Oooops, default! (" << ea.getEventType() << ")\n";
               return false;
            }
         } // switch (...)

         // Time in seconds since the program started
         std::cout << "Time = " << ea.time() << "\n\n";
         return true;
      }

//    ????? What's this? This was in the tutorial from Terse solutions...
//       virtual void accept(osgGA::GUIEventHandlerVisitor& v)
//       {
//          v.visit (*this);
//       }

};



// - main ----------------------------------------------------------------------
int main (int argc, char* argv[])
{

   // Create non-full screen camera (so that we can see the terminal...)
   osg::ref_ptr<Producer::RenderSurface> rsWindow (new Producer::RenderSurface);
   rsWindow->setScreenNum(0);
   rsWindow->setWindowName ("OSG Event Handling");
   rsWindow->setWindowRectangle (0, 0, 640, 480);
   osg::ref_ptr<Producer::Camera> camera1 (new Producer::Camera);
   camera1->setRenderSurface (rsWindow.get());
   camera1->setProjectionRectangle (0, 0, 640, 480);
   osg::ref_ptr<Producer::CameraConfig> cfg (new Producer::CameraConfig());
   cfg->addCamera ("The only camera", camera1.get());

   // Create a Producer-based viewer, using the camera defined above
   osgProducer::Viewer viewer (cfg.get());
   viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

   // Instantiate our custom event handler and add it the list of event
   // handlers maintained by the viewer.
   osg::ref_ptr<osgGA::GUIEventHandler> eh (new MyEventHandler);
   viewer.getEventHandlerList().push_front (eh);

   // Enter rendering loop
   viewer.realize();

   while (!viewer.done())
   {
      viewer.sync();
      viewer.update();
      viewer.frame();
   }

   viewer.sync();
}
