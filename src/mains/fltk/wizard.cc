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

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// Simple 'wizard' using fltk's new Fl_Wizard widget

Fl_Window *G_win = 0;
Fl_Wizard *G_wiz = 0;

void back_cb(Fl_Widget*,void*) { G_wiz->prev(); }
void next_cb(Fl_Widget*,void*) { G_wiz->next(); }
void done_cb(Fl_Widget*,void*) { exit(0); }

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   G_win = new Fl_Window(100,200);
   G_wiz = new Fl_Wizard(0,0,100,200);

   // Wizard: page 1
   {
      Fl_Group *g = new Fl_Group(0,0,200,100);
      Fl_Button *next = new Fl_Button(20,55,60,25,"Next>>"); 
      next->callback(next_cb);
      Fl_Multiline_Output *out = new Fl_Multiline_Output(10,110,80,80);
      out->color(FL_RED); out->value("This is\nFirst\npage");
      g->end();
   }
   // Wizard: page 2
   {
      Fl_Group *g = new Fl_Group(0,0,200,100);
      Fl_Button *next = new Fl_Button(20,55,60,25,"Next>>"); 
      next->callback(next_cb);
      Fl_Button *back = new Fl_Button(20,25,60,25,"<<Back"); 
      back->callback(back_cb);
      Fl_Multiline_Output *out = new Fl_Multiline_Output(10,110,80,80);
      out->color(FL_GREEN); out->value("This is\nSecond\npage");
      g->end();
   }
   // Wizard: page 3
   {
      Fl_Group *g = new Fl_Group(0,0,200,100);
      Fl_Button *done = new Fl_Button(20,55,60,25,"Finish"); 
      done->callback(done_cb);
      Fl_Button *back = new Fl_Button(20,25,60,25,"<<Back"); 
      back->callback(back_cb);
      Fl_Multiline_Output *out = new Fl_Multiline_Output(10,110,80,80);
      out->color(FL_WHITE);
      out->value("This is\nLast\npage");
      g->end();
   }
   G_wiz->end();
   G_win->end();
   G_win->show(argc, argv);
   return Fl::run();

}

   
