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

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_File_Browser.H>

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::ios;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   Fl_Browser browser(0,0,500,500);

// CLEAR BROWSER
   browser.clear();

// ADD LINES TO BROWSER
   browser.add("One");		// fltk does strdup() internally       
   browser.add("Two");
   browser.add("Three");

// FORMAT CHARACTERS: CHANGING TEXT COLORS IN LINES
//    Warning: format chars are returned to you via ::text()
//    @C# - text color             @b  - bold
//    @B# - background color       @i  - italic
//    @F# - set font number        @f  - fixed pitch font
//    @S# - set point size         @u  - underline
//    @.  - terminate '@' parser   @-  - strikeout
//
   browser.add("Black @C1Red @C2Green @C3Yellow");

// DISABLING FORMAT CHARACTERS
   browser.format_char(0);

// PRINT ALL SELECTED ITEMS IN BROWSER
   for ( int t=1; t<=browser.size(); t++ ) {
      if ( browser.selected(t) ) {
         printf("%d) '%s'\n", t, browser.text(t));
      }
   }

// PRE-SELECT ALL LINES IN BROWSER
//   Note: index numbers are 1 based..!
//
   for ( int t=1; t<=browser.size(); t++ ) {
      browser.select(t);
   }

// DE-SELECT ALL LINES IN BROWSER
   browser.deselect();

// GET SINGLE SELECTED ITEM FROM BROWSER
   int index = browser.value();

// USING INDEX NUMBER, RETURN TEXT
//   Note: index numbers are 1 based..!
//
   if ( index > 0 ) {
      const char *s = browser.text(index);
   }

}

   
