// ==========================================================================
// Header file for TEXTDIALOGBOX class
// ==========================================================================
// Last modified on 7/22/05; 10/14/06
// ==========================================================================

#ifndef TEXTDIALOGBOX_H
#define TEXTDIALOGBOX_H

#include <fstream>
#include <string>
#include <vector>

class TextDialogBox
{

  public:

   TextDialogBox(); 
   ~TextDialogBox();

   void set_tty_devname(std::string devname);
   std::string get_tty_devname() const;

   void open(std::string window_title);
   void connect(std::string existing_tty_devname);
   void kill_xterm();
   void close();
   void reinitialize(
      std::string titleline,const std::vector<std::string>& column_headers);
   void write_single_row(int row_ID,std::vector<double>& column_data);
   
  private:

   int column_width;
   std::string tty_devname;
   std::ofstream output_stream;

   void allocate_member_objects();
   void initialize_member_objects();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

inline void TextDialogBox::set_tty_devname(std::string devname)
{
   tty_devname=devname;
}

inline std::string TextDialogBox::get_tty_devname() const
{
   return tty_devname;
}


#endif // TextDialogBox.h



