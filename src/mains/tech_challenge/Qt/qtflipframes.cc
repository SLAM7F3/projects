 // ========================================================================
// Program QTFLIPFRAMES utilizes FFMPEG to decompose an input
// Flip .mp4 file into individual JPG frames.

//				qtflipframes

// ========================================================================
// Last updated on 8/28/10; 8/29/10; 9/5/10
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
   window_ptr->setWindowTitle("Flip flash file picker");

   string starting_MP4_subdir="/data/tech_challenge/field_tests/";
   QString fileName = QFileDialog::getOpenFileName(window_ptr,
   "Select input Flip MP4 file", starting_MP4_subdir.c_str(), 
   "MP4 Files (*.mp4 *.MP4)");
   string Flip_MP4_filename=fileName.toStdString();
   cout << "Selected Flip MP4 filename = " << Flip_MP4_filename << endl 
        << endl;
   
   string Flip_MP4_movie_subdir=filefunc::getdirname(Flip_MP4_filename);
//   cout << "Flip_MP4_movie_subdir = " << Flip_MP4_movie_subdir << endl;

   string Flip_filename_prefix=stringfunc::prefix(filefunc::getbasename(
      Flip_MP4_filename));
//   cout << "Flip_filename_prefix = " << Flip_filename_prefix << endl;

   string output_subdir=Flip_MP4_movie_subdir+Flip_filename_prefix+
	"_jpg_frames/";
//   cout << "output_subdir = " << output_subdir << endl;
   
   cout << "Program EXTRACT_FLIP_FRAMES will convert selected MP4 movie into a"
        << endl;
   cout << "sequence of JPEG images which will be written to "+output_subdir 
        << endl << endl;
   outputfunc::enter_continue_char();


   filefunc::dircreate(output_subdir);

   string unix_cmd=
      "ffmpeg -i "+Flip_MP4_filename+
      " -sameq -r 10 "+output_subdir+"image-%05d.jpg";
//      " -r 5 "+output_subdir+"image-%05d.png";

   cout << "unix_cmd = " << unix_cmd << endl;
   sysfunc::unix_command(unix_cmd);
}


//   ./ffmpeg -i in.avi -vf "crop=0:0:0:240" out.avi

// -sameq

// -qscale 1
