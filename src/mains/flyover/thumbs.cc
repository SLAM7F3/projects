// ==========================================================================
// Program THUMBS reads in a text file containing a list of orthologo
// image chip filenames and their manually-assigned classes (HOV,
// right_turn, left_turn, straight_arrow, STOP, incorrect).  It
// randomly resets the "incorrect" labels to one of the five other
// labels.  It also randomly resets some fraction of the
// non-"incorrect" labels to one of the five other labels.  THUMBS
// then exports a CSV file containing 4 columns: image chip filename,
// proposed chip label, icon matching proposed label, and true label.

// We wrote this little program in Nov 2015 in order to supply a test
// set of orthologo image chips plus a proposed set of class labels to
// a group outside of Apple for thumbs up/down testing.
// ==========================================================================
// Last updated on 11/12/15; 11/17/15
// ==========================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "general/filefuncs.h"
#include "numrec/nrfuncs.h"
#include "general/outputfuncs.h"
#include "general/stringfuncs.h"
#include "general/sysfuncs.h"
#include "time/timefuncs.h"

// ==========================================================================
int main(int argc, char *argv[])
// ==========================================================================
{
   using std::cin;
   using std::cout;
   using std::endl;
   using std::flush;
   using std::ios;
   using std::ofstream;
   using std::ostream;
   using std::string;
   using std::vector;
   std::set_new_handler(sysfunc::out_of_memory);

// ==========================================================================

   string shuffled_all_chips_filename="./shuffled_all_chips.dat";
   filefunc::ReadInfile(shuffled_all_chips_filename);

   string output_filename="csv.output";
   ofstream outstream;
   filefunc::openfile(output_filename, outstream);
   outstream << "# Image chip filename, proposed chip label, icon matching proposed label, true label" 
             << endl << endl;

   vector<string> icon_filenames;
   icon_filenames.push_back("HOV.jpg");
   icon_filenames.push_back("left_turn.jpg");
   icon_filenames.push_back("right_turn.jpg");
   icon_filenames.push_back("STOP.jpg");
   icon_filenames.push_back("straight_arrow.jpg");

   vector<string> chip_labels;
   chip_labels.push_back("HOV");
   chip_labels.push_back("left_turn");
   chip_labels.push_back("right_turn");
   chip_labels.push_back("STOP");
   chip_labels.push_back("straight_arrow");
   chip_labels.push_back("incorrect");

   int n_HOV = 0;
   int n_left_turn = 0;
   int n_right_turn = 0;
   int n_STOP = 0;
   int n_straight_arrow = 0;
   int n_incorrect = 0;

   int n_wrong_proposed_labels = 0;

   for(unsigned int i = 0; i < filefunc::text_line.size(); i++)
   {
      string curr_line = filefunc::text_line[i];
      vector<string> substrings = stringfunc::decompose_string_into_substrings(
         curr_line);
      cout << i << " " << substrings[0] << " " << substrings[1] << endl;

      string image_chip_filename=substrings[0];
      string exported_label, real_label = substrings[1];
      if(real_label == "incorrect")
      {
         int false_label_index = 4.99 * nrfunc::ran1();
         exported_label = chip_labels[false_label_index];
         n_incorrect++;
         n_wrong_proposed_labels++;
      }
      else
      {

        if(real_label == "HOV")
        {
           n_HOV++;
        }
        else if (real_label == "left_turn")
        {
           n_left_turn++;
        }
        else if (real_label == "right_turn")
        {
           n_right_turn++;
        }
        else if (real_label == "STOP")
        {
           n_STOP++;
        }
        else if (real_label == "straight_arrow")
        {
           n_straight_arrow++;
        }

        double false_threshold = 0.25;
        if(nrfunc::ran1() < false_threshold)
        {
           int random_label_index = 4.99 * nrfunc::ran1();
           exported_label = chip_labels[random_label_index];
           if(exported_label != filefunc::getbasename(real_label))
           {
              n_wrong_proposed_labels++;
           }
        }
        else
        {
           exported_label = filefunc::getbasename(real_label);
        }
      }
      string exported_icon = exported_label+".jpg";
  
      outstream << image_chip_filename << " , "
                << exported_label << " , "
                << exported_icon << " , "
                << filefunc::getbasename(real_label)
                << endl;
   }

   filefunc::closefile(output_filename, outstream);

   string banner="Exported CSV file to "+output_filename;
   outputfunc::write_banner(banner);

   cout << "n_HOV = " << n_HOV << endl;
   cout << "n_left_turn = " << n_left_turn << endl;
   cout << "n_right_turn = " << n_right_turn << endl;
   cout << "n_STOP = " << n_STOP << endl;
   cout << "n_straight_arrow = " << n_straight_arrow << endl;
   cout << "n_incorrect = " << n_incorrect << endl;

   int n_total = n_HOV + n_left_turn + n_right_turn
      + n_STOP + n_straight_arrow + n_incorrect;
   cout << "n_total = " << n_total << endl;
   cout << "n_wrong_proposed_labels = " << n_wrong_proposed_labels
        << endl;
   cout << "wrog_proposed_label fraction = " 
        << double(n_wrong_proposed_labels)/n_total
        << endl;
}
