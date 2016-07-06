// ==========================================================================
// Header file for table class
// ==========================================================================
// Last modified on 10/17/08; 10/20/08
// ==========================================================================

#ifndef TABLE_H
#define TABLE_H

#include <iostream>
#include <string>
#include <vector>
#include "color/colorfuncs.h"
#include "math/Genarray.h"

class threevector;
class twovector;

class table
{

  public:

// Initialization, constructor and destructor functions:

   table(int nrows,int ncolumns);
   table(const table& t);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~table();
   table& operator= (const table& t);
   friend std::ostream& operator<< (std::ostream& outstream,
                                    const table& t);

// Set and get member functions:

   int get_nrows() const;
   int get_ncolumns() const;
   void set(int m,int n,std::string entry);
   void set_caption(std::string caption);
   std::string get(int m,int n) const;

   void set_caption_text_color(colorfunc::Color c);
   void set_caption_background_color(colorfunc::Color c);
   void set_header_text_color(colorfunc::Color c);
   void set_header_background_color(colorfunc::Color c);
   void set_body_text_color(colorfunc::Color c);
   void set_body_background_color(colorfunc::Color c);

// Table input member functions

   void set_header_label(int n,std::string label);
   void set(int m,int n,double x);
   void set(int m,int n,double x,double y);
   void set(int m,int n,const twovector& xy);
   void set(int m,int n,double x,double y,double z);
   void set(int m,int n,const threevector& xyz);

// Table output member functions

   void write_html_file(std::string filename);
   std::vector<std::string> generate_html();

  private:

   int n_rows,n_columns,n_header_labels;
   genstringarray* header_labels_ptr;
   genstringarray* genstringarray_ptr;
   std::string caption;
   colorfunc::Color caption_background_color,caption_text_color;
   colorfunc::Color header_background_color,header_text_color;
   colorfunc::Color body_background_color,body_text_color;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const table& t);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline int table::get_nrows() const
{
   return n_rows;
}

inline int table::get_ncolumns() const
{
   return n_columns;
}

inline void table::set_caption(std::string caption)
{
   this->caption=caption;
}

inline void table::set(int m,int n,std::string entry)
{
   if (genstringarray_ptr != NULL)
   {
      genstringarray_ptr->put(m,n,entry);
   }
}

inline std::string table::get(int m,int n) const
{
   std::string entry;
   if (genstringarray_ptr != NULL)
   {
      entry=genstringarray_ptr->get(m,n);
   }
   return entry;
}

inline void table::set_caption_text_color(colorfunc::Color c)
{
   caption_text_color=c;
}

inline void table::set_caption_background_color(colorfunc::Color c)
{
   caption_background_color=c;
}

inline void table::set_header_text_color(colorfunc::Color c)
{
   header_text_color=c;
}

inline void table::set_header_background_color(colorfunc::Color c)
{
   header_background_color=c;
}

inline void table::set_body_text_color(colorfunc::Color c)
{
   body_text_color=c;
}

inline void table::set_body_background_color(colorfunc::Color c)
{
   body_background_color=c;
}
 
#endif  // table.h



