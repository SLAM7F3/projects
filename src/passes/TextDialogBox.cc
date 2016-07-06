// ==========================================================================
// TEXTDIALOGBOX class member function definitions
// ==========================================================================
// Last modified on 7/22/05; 12/20/05; 10/14/06; 9/7/08
// ==========================================================================

#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>	
#include <unistd.h>
#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "passes/TextDialogBox.h"

using std::cin;
using std::cout;
using std::endl;
using std::ios;
using std::setw;
using std::string;
using std::vector;


void TextDialogBox::allocate_member_objects()
{
}

// --------------------------------------------------------------------------
void TextDialogBox::initialize_member_objects()
{
   column_width=13;
}

// --------------------------------------------------------------------------
TextDialogBox::TextDialogBox()
{
   allocate_member_objects();
   initialize_member_objects();
}

// --------------------------------------------------------------------------
TextDialogBox::~TextDialogBox()
{
}

// --------------------------------------------------------------------------
// Member function open pops open a new xterm in which feature
// information can be displayed.

void TextDialogBox::open(string window_title)
{
//   string window_title="Feature Information";
//   string fontname="10x20";
   string fontname="8x16";
   string geometry_coords="60x40-0-50";
   string fg_color="blue";
   string bg_color="white";
   tty_devname=sysfunc::launch_new_xterm(
      window_title,fontname,geometry_coords,fg_color,bg_color,true);
//   cout << "tty_devname = " << tty_devname << endl;
   filefunc::openfile(tty_devname,output_stream);
}

// --------------------------------------------------------------------------
// Member function connect takes in the full device name path to some
// already opened xterm.  It connects output to this existing xterm...

void TextDialogBox::connect(string existing_tty_devname)
{

// Should test whether xterm corresponding to tty devname actually is
// open.  If so, we want to connect onto it...

   tty_devname=existing_tty_devname;
   filefunc::openfile(tty_devname,output_stream);
}

// --------------------------------------------------------------------------
void TextDialogBox::close()
{
   filefunc::closefile(tty_devname,output_stream);
   kill_xterm();
}

// --------------------------------------------------------------------------
// Member function kill_xterm is a cluge which finds the process ID
// for the xterm using unix ps command.  We then execute a brute force
// kill -9 PID in order to kill the xterminal.

void TextDialogBox::kill_xterm()
{
   string terminal_number=filefunc::getbasename(tty_devname);

   string tmp_filename=filefunc::generate_tmpfilename();
   string unixcommandstr="ps --no-headers -t "+terminal_number+" -o pid > "
      +tmp_filename;
//   cout << "unixcommandstr = " << unixcommandstr << endl;
   sysfunc::unix_command(unixcommandstr);

   vector<string> line;
   filefunc::ReadInfile(tmp_filename,line);
   filefunc::deletefile(tmp_filename);
   string killcommandstr="kill -9 "+line[0];
   sysfunc::unix_command(killcommandstr);
}

// --------------------------------------------------------------------------
// Member function reinitialize refreshes the feature xterminal and
// writes out the title and column header lines.

void TextDialogBox::reinitialize(
   string titleline,const vector<string>& column_headers)
{
   if (tty_devname != "")
   {
      sysfunc::clear_xterm(tty_devname);
      output_stream << titleline << endl << endl;
      output_stream.setf(ios::left, ios::adjustfield);


// --------------------------------------------------------------------------
// Member function write_single_row

void TextDialogBox::write_single_row(int row_ID,vector<double>& column_data)
{
   if (tty_devname != "")
   {
      output_stream.setf(ios::left, ios::adjustfield);
      string rowID_string="#"+stringfunc::number_to_string(row_ID);
      output_stream << setw(column_width) << rowID_string;

//      const int n_precision=3;
      const int n_precision=5;
      for (unsigned int j=0; j<column_data.size(); j++)
      {
         output_stream << setw(column_width) << stringfunc::number_to_string(
            column_data[j],n_precision);
      }
      output_stream.unsetf(ios::left);
      output_stream << endl;
   } // tty_devname != "" conditional
}
