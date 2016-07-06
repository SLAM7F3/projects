// ==========================================================================
// Table class member function definitions
// ==========================================================================
// Last modified on 10/17/08; 10/20/08; 11/17/08
// ==========================================================================

#include "general/filefuncs.h"
#include "general/stringfuncs.h"
#include "xml/table.h"
#include "math/threevector.h"

using std::cout;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void table::allocate_member_objects()
{
   header_labels_ptr=new genstringarray(1,n_columns);
   genstringarray_ptr=new genstringarray(n_rows,n_columns);
}		       

void table::initialize_member_objects()
{
   n_header_labels=0;
   header_labels_ptr=NULL;
   genstringarray_ptr=NULL;

   caption_text_color=colorfunc::black;
   caption_background_color=colorfunc::white;
   header_text_color=colorfunc::black;
   header_background_color=colorfunc::white;
   body_text_color=colorfunc::black;
   body_background_color=colorfunc::white;
}

table::table(int nrows,int ncolumns)
{
   n_rows=nrows;
   n_columns=ncolumns;

   initialize_member_objects();
   allocate_member_objects();
}

// Copy constructor:

table::table(const table& t)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(t);
}

table::~table()
{
   delete header_labels_ptr;
   delete genstringarray_ptr;
}

// ---------------------------------------------------------------------
void table::docopy(const table& t)
{
   n_rows=t.n_rows;
   n_columns=t.n_columns;
}

// Overload = operator:

table& table::operator= (const table& t)
{
   if (this==&t) return *this;
   docopy(t);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

ostream& operator<< (ostream& outstream,const table& t)
{
   outstream << endl;
   outstream << "n_rows = " << t.n_rows << endl;
   outstream << "n_columns = " << t.n_columns << endl;
   if (t.genstringarray_ptr != NULL)
   {
      outstream << "*genstringarray_ptr = " << *t.genstringarray_ptr << endl;
   }
   
   return outstream;
}

// =====================================================================
// Table input member functions
// =====================================================================

void table::set_header_label(int n,string label)
{
   header_labels_ptr->put(0,n,label);
   n_header_labels++;
}

void table::set(int m,int n,double x)
{
   set(m,n,stringfunc::number_to_string(x));
}

void table::set(int m,int n,double x,double y)
{
   string entry="(";
   entry += stringfunc::number_to_string(x);
   entry += ",";
   entry += stringfunc::number_to_string(y);
   entry += ")";
   set(m,n,entry);
}

void table::set(int m,int n,const twovector& xy)
{
   set(m,n,xy.get(0),xy.get(1));
}

void table::set(int m,int n,double x,double y,double z)
{
   string entry="(";
   entry += stringfunc::number_to_string(x);
   entry += ",";
   entry += stringfunc::number_to_string(y);
   entry += ",";
   entry += stringfunc::number_to_string(z);
   entry += ")";
   set(m,n,entry);
}

void table::set(int m,int n,const threevector& xyz)
{
   set(m,n,xyz.get(0),xyz.get(1),xyz.get(2));
}

// =====================================================================
// Table output member functions
// =====================================================================

void table::write_html_file(string filename)
{
   ofstream outstream;
   filefunc::openfile(filename,outstream);

   vector<string> html_lines=generate_html();
   for (int i=0; i<int(html_lines.size()); i++)
   {
      outstream << html_lines[i] << endl;
   }
   filefunc::closefile(filename,outstream);
}

vector<string> table::generate_html()
{
   vector<string> html_lines;

   html_lines.push_back("<html>");
   html_lines.push_back(
      "<!-- for more help see http://www.w3schools.com/html/html_tables.asp -->");

// Write out html header information which contains Cascading Style
// Sheet (CSS) templates:

   html_lines.push_back("<head>");
   html_lines.push_back("<style>");

   if (caption.size() > 0)
   {
      html_lines.push_back("	caption {");
      html_lines.push_back("		text-align: center;");
      html_lines.push_back(
         "		background-color: "+
         colorfunc::get_colorstr(caption_background_color)+";");
      html_lines.push_back(
         "		color: "+
         colorfunc::get_colorstr(caption_text_color)+";");
      html_lines.push_back("	}");
   }

   if (n_header_labels > 0)
   {
      html_lines.push_back("	th {");
      html_lines.push_back("		text-align: center;");
      html_lines.push_back(
         "		background-color: "+
         colorfunc::get_colorstr(header_background_color)+";");
      html_lines.push_back(
         "		color: "+
         colorfunc::get_colorstr(header_text_color)+";");
      html_lines.push_back("	}");
   }
   
   html_lines.push_back("	td {");
   html_lines.push_back("		text-align: center;");
   html_lines.push_back("		background-color: "+
                        colorfunc::get_colorstr(body_background_color)+";");
   html_lines.push_back("		color: "+
                        colorfunc::get_colorstr(body_text_color)+";");
   html_lines.push_back("	}");

   html_lines.push_back("</style>");
   html_lines.push_back("</head>");

// Write out html body information:

   html_lines.push_back("<body>");

   html_lines.push_back("<table border=\"1\">");
//   html_lines.push_back("<table>");

// Output caption if it exists:

   if (caption.size() > 0)
   {
      html_lines.push_back("<caption>");
      html_lines.push_back(caption);
      html_lines.push_back("</caption>");
   }
   

// Output header labels if they exist:

   if (n_header_labels > 0)
   {
      html_lines.push_back("<tr>");
      for (int c=0; c<n_columns; c++)
      {
         string curr_line="<th>";
//      curr_line += "Header "+stringfunc::number_to_string(c);
         curr_line += header_labels_ptr->get(0,c);
         curr_line += "</th>";
         html_lines.push_back(curr_line);
      }
      html_lines.push_back("</tr>");
   }
   
// Loop over all rows and columns and write out table entries:

   for (int r=0; r<n_rows; r++)
   {

// Check whether current row has any nontrivial XML content.  If not,
// don't bother to print it out:

      bool empty_row_flag=true;
      for (int c=0; c<n_columns; c++)
      {
         string curr_entry=get(r,c);
         if (curr_entry.size() > 0) empty_row_flag=false;
      }
      if (empty_row_flag) continue;

      html_lines.push_back("<tr>");
      for (int c=0; c<n_columns; c++)
      {
         string curr_line="<td>";
         curr_line += get(r,c);
         curr_line += "</td>";
         html_lines.push_back(curr_line);
      }
      html_lines.push_back("<tr>");
   }

   html_lines.push_back("</table>");
   html_lines.push_back("<body>");
   html_lines.push_back("</html>");

   return html_lines;
}
