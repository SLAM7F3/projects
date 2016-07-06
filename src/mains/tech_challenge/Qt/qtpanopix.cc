// ========================================================================
// Program QTPANOPIX pops open a Qt file picker dialog box which
// allows the user to easily select a subdirectory of sail plane
// images for mosaicing.  QTPANOPIX next queries the user to enter the
// starting and stopping image numbers for photos to be mosaiced as
// well as the image number skip.  Scanning through the subdirectory
// containing all input JPEG images, this program generates and
// executes a unix command which copies the requested mosaic photo
// constituents to a subdirectory of
// /data/tech_challenge_local/pano_pix/.

//				qtpanopix

// ========================================================================
// Last updated on 9/1/10; 9/4/10; 9/12/10
// ========================================================================

#include <iostream>
#include <string>
#include <vector>

#include <QtCore/QtCore>
#include <QtGui/QFileDialog>
#include <QtGui/QApplication>

#include "general/filefuncs.h"
#include "image/imagefuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "video/videofuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

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
   window_ptr->setWindowTitle("Panorama pictures picker");

   string starting_pano_subdir="/data/tech_challenge/field_tests";
   QString fileName = QFileDialog::getOpenFileName(window_ptr,
   "Select just one input panorama picture", starting_pano_subdir.c_str(), 
   "Pano Files (*.jpg *.JPG *.jpeg *.JPEG)");

   window_ptr->hide();
   window_ptr->close();

   string pano_pix_filename=fileName.toStdString();
   cout << "Selected panorama picture filename = " 
        << pano_pix_filename << endl << endl;
   
   string imagedir=filefunc::getdirname(pano_pix_filename);
   cout << "Input image subdirectory = " << imagedir << endl << endl;

   string basefilename="image";
   string suffix=".jpg";

   int n_start,n_stop;
   videofunc::find_min_max_photo_numbers(imagedir,n_start,n_stop);
   int ndigits=mathfunc::ndigits_before_decimal_point(n_stop);
//   cout << "ndigits = " << ndigits << endl;

   cout << endl;
   cout << "Enter starting image number for mosaic:" << endl;
   cin >> n_start;
   cout << endl;

   int n_skip=1;
   cout << "Enter image number skip:" << endl;
   cout << "(Recommended image number skip value = 6)" << endl;
   cin >> n_skip;
   cout << endl;

   int n_images_to_mosaic;
   cout << "Enter number of images to mosaic together:" << endl;
   cout << "(Maximum recommended number of images to mosaic = 23)" << endl;
   cin >> n_images_to_mosaic;
   cout << endl;

//   cout << "Enter stopping image number for mosaic:" << endl;
//   cin >> n_stop;
   n_stop=n_start+(n_images_to_mosaic-1)*n_skip;

   string pano_basedir="/data/tech_challenge_local/pano_pix/";
   string subdir=stringfunc::integer_to_string(n_start,ndigits)+"-"+
      stringfunc::integer_to_string(n_stop,ndigits)+"/";
   string pano_subdir=pano_basedir+subdir;
   filefunc::dircreate(pano_subdir);

   vector<string> previous_image_files=filefunc::files_in_subdir(
      pano_subdir);
//   cout << "previous_image_files.size() = " << previous_image_files.size()
//        << endl;
   if (previous_image_files.size() > 0)
   {
      string unix_cmd="/bin/rm "+pano_subdir+"*.*";
      cout << "unix_cmd = " << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
   }

   int image_counter=0;
   for (int n=n_start; n <= n_stop; n += n_skip)
   {
      string curr_filename=
         imagedir+"IMG_"+stringfunc::integer_to_string(n,ndigits)+".JPG";
      string unix_cmd="cp "+curr_filename+" "+pano_subdir;
//      cout << unix_cmd << endl;
      sysfunc::unix_command(unix_cmd);
      image_counter++;
   }

   string banner="Wrote "+stringfunc::number_to_string(image_counter)
      +" images to "+pano_subdir;
   outputfunc::write_big_banner(banner);


   cout << endl;
   string message="Enter any non-white character to end program...";
   outputfunc::enter_continue_char(message);
}
