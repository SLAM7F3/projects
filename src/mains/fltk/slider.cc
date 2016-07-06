// ==========================================================================
// Program HELLO
// ==========================================================================
// Last updated on 7/3/06
// ==========================================================================

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

#include <stdio.h>
#include <stdlib.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Wizard.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Slider.H>

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// sliderinput -- simple example of tying an fltk slider and input widget together
// 1.00 erco 10/17/04

class SliderInput : public Fl_Group {
   Fl_Int_Input *input;
   Fl_Slider    *slider;

   // CALLBACK HANDLERS
   //    These 'attach' the input and slider's values together.
   //
   void Slider_CB2() {
      static int recurse = 0;
      if ( recurse ) { 
         return;
      } else {
         recurse = 1;
         char s[80];
         sprintf(s, "%d", (int)(slider->value() + .5));
         // fprintf(stderr, "SPRINTF(%d) -> '%s'\n", (int)slider->value(), s);
         input->value(s);    // pass slider's value to input
         recurse = 0;
      }
   }

   static void Slider_CB(Fl_Widget *w, void *data) {
      ((SliderInput*)data)->
         Slider_CB2();
   }

   void Input_CB2() {
      static int recurse = 0;
      if ( recurse ) {
         return;
      } else {
         recurse = 1;
         int val = 0;
         if ( sscanf(input->value(), "%d", &val) != 1 ) {
            val = 0;
         }
         // fprintf(stderr, "SCANF('%s') -> %d\n", input->value(), val);
         slider->value(val);         // pass input's value to slider
         recurse = 0;
      }
   }
   static void Input_CB(Fl_Widget *w, void *data) {
      ((SliderInput*)data)->
         Input_CB2();
   }

  public:
   // CTOR
   SliderInput(int x, int y, int w, int h, const char *l=0) : Fl_Group(x,y,w,h,l) {
      int in_w = 40;
      int in_h = 25;

      input  = new Fl_Int_Input(x+10, y+10, in_w, in_h);
      input->callback(Input_CB, (void*)this);
      input->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);

      slider = new Fl_Slider(x+10+in_w, y+10, w - 20 - in_w, in_h);
      slider->type(1);
      slider->callback(Slider_CB, (void*)this);

      bounds(1, 10);          // some usable default
      value(5);               // some usable default
      end();			// close the group
   }

   // MINIMAL ACCESSORS --  Add your own as needed
   int value() { return((int)(slider->value() + 0.5)); }
   void value(int val) { slider->value(val); Slider_CB2(); }
   void minumum(int val) { slider->minimum(val); }
   int minumum() { return((int)slider->minimum()); }
   void maximum(int val) { slider->maximum(val); }
   int maximum() { return((int)slider->maximum()); }
   void bounds(int low, int high) { slider->bounds(low, high); }
};


// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   Fl_Window win(240,90);
   SliderInput *si = new SliderInput(20,20,200,50,"Slider Input");
   si->color(167);
   si->box(FL_FLAT_BOX);
   si->bounds(1,100);
   si->value(50);
   win.show();
   return(Fl::run());
}

   
