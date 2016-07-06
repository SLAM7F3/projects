// ==========================================================================
// INPUTFUNCS stand-alone methods
// ==========================================================================
// Last modified on 8/16/05; 5/16/06; 7/30/06; 2/5/07
// ==========================================================================

#include "general/filefuncs.h"
#include "general/inputfuncs.h"
#include "general/stringfuncs.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

namespace inputfunc
{

// --------------------------------------------------------------------------
// Method enter_nonnegative_label queries the user to enter an integer
// within the main text console.  It performs some simple sanity
// checks on the user's input and returns a genuine non-negative
// integer.

   int enter_nonnegative_integer(string label_command)
      {   
         bool redo_flag=false;
         string input_string;
         int nonnegative_integer=-1;
   
         do
         {
            redo_flag=false;
            do
            {
               redo_flag=false;
               cout << label_command << endl;
               cin >> input_string;
               cout << endl;
               if (!stringfunc::is_number(input_string))
               {
                  cout << "Input is not a number; try again" << endl;
                  redo_flag=true;
               }
            }
            while (redo_flag);

            nonnegative_integer=stringfunc::string_to_number(input_string);
            if (nonnegative_integer < 0)
            {
               cout << "Invalid negative value entered; try again" << endl;
               redo_flag=true;
            }
         }
         while (redo_flag);

//   cout << "nonnegative_integer = " << nonnegative_integer << endl;
         return nonnegative_integer;
      }

// --------------------------------------------------------------------------
// Method enter_string queries the user to enter a string (which can
// contain white space) within the main text console.  It waits until
// the user has entered some string with a nonzero length.
   
   string enter_string(string label_command)
      {   
         cout << label_command << endl;
         string input_string;
         while (input_string.size()==0)
         {
            input_string=filefunc::getsingleline();
//            cout << "input_string.size() = " << input_string.size() << endl;
         }
         return input_string;
      }

// --------------------------------------------------------------------------
// Method enter_threevector queries the user to enter the X, Y and Z
// components of a threevector from a text console.
   
   threevector enter_threevector(string label_command)
      {   
         cout << label_command << endl;

         double X,Y,Z;
         cout << "Enter X:" << endl;
         cin >> X;
         cout << "Enter Y:" << endl;
         cin >> Y;
         cout << "Enter Z:" << endl;
         cin >> Z;

         return threevector(X,Y,Z);
      }

   


} // inputfunc namespace
