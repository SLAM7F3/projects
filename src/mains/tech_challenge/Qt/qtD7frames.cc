// ========================================================================
// Program QTD7FRAMES utilizes FFMPEG to decompose an input D7 .flv or
// .mkv file into individual JPG frames.  It prompts the user to enter
// a D7 flash movie using a Qt file finder widget.  QTD7FRAMES
// generates an output subdirectory within the input subdirectory with
// the same basename and _jpg_frames suffix.  JPG frames are written
// to this output subdir.
// ========================================================================
// Last updated on 8/28/10; 8/29/10; 9/4/10
// ========================================================================

#include <iostream>
#include <string>

#include <QtCore/QtCore>
#include <QtGui/QFileDialog>
#include <QtGui/QApplication>

#include "general/filefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;

// ==========================================================================
int main( int argc, char** argv )
{
   sysfunc::clearscreen();

// Initialize Qt application:

   QApplication app(argc,argv);

// In July 2010, we learned the hard way that javascript will never
// transmit the full path for any selected file from a client to a
// server for security reasons.  So Zach Sun suggested that we use a
// Qt file dialog box instead to enable a user to effectively select a 
// local subdirectory containing a set of video frames.  

   QWidget* window_ptr=new QWidget;
   window_ptr->move(835,0);
//   cout << "window_ptr->x() = " << window_ptr->x() << endl;
//   cout << "window_ptr->y() = " << window_ptr->y() << endl;
   window_ptr->setWindowTitle("D7 flash file picker");

   string starting_flash_subdir="/data/tech_challenge/field_tests/";
   QString fileName = QFileDialog::getOpenFileName(window_ptr,
   "Select input D7 flash file", starting_flash_subdir.c_str(), 
   "Flash Files (*.flv *.mkv)");
   string D7_flash_filename=fileName.toStdString();
   cout << "Selected D7 flash filename = " << D7_flash_filename << endl 
        << endl;
   
   string D7_flash_movie_subdir=filefunc::getdirname(D7_flash_filename);
//   cout << "D7_flash_movie_subdir = " << D7_flash_movie_subdir << endl;

   string D7_filename_prefix=stringfunc::prefix(filefunc::getbasename(
      D7_flash_filename));
//   cout << "D7_filename_prefix = " << D7_filename_prefix << endl;

   string output_subdir=D7_flash_movie_subdir+D7_filename_prefix+
	"_jpg_frames/";
//   cout << "output_subdir = " << output_subdir << endl;
   

   cout << "Program EXTRACT_D7FRAMES will convert selected flash movie into a"
        << endl;
   cout << "sequence of JPEG images which will be written to "+output_subdir 
        << endl << endl;
   
   outputfunc::enter_continue_char();
   
   filefunc::dircreate(output_subdir);
   string unix_cmd=
      "ffmpeg -i "+D7_flash_filename+
      " -sameq -r 2 "+output_subdir+"image-%05d.jpg";
//      " -r 5 "+output_subdir+"image-%05d.png";

   cout << "unix_cmd = " << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);
}


//   ./ffmpeg -i in.avi -vf "crop=0:0:0:240" out.avi

// -sameq

// -qscale 1
