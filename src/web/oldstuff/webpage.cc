// ==========================================================================
// Webpage class member function definitions which allow one to
// generate HTML web pages using C++
// ==========================================================================
// Last modified on 2/17/04
// ==========================================================================

#include "general/stringfuncs.h"
#include "webpage.h"

using std::string;
using std::ostream;
using std::endl;

const int webpage::NMAXMETALINES=5;
const int webpage::NMAXFORMS=5;
const int webpage::NMAXIMAGES=5;
const int webpage::NMAXLINKS=10;
const int webpage::NMAXTABLES=10;
const int webpage::BIGFONT=2;

// ---------------------------------------------------------------------
// Constructor functions:
// ---------------------------------------------------------------------

webpage::webpage(void)
{
   int i;
   metaline=new string[NMAXMETALINES];
   form=new webimage[NMAXFORMS];
   image=new webimage[NMAXIMAGES];
   link=new weblink[NMAXLINKS];
   table=new webtable[NMAXTABLES];

   backgroundcolor="black";
   textcolor="cyan";
   linkcolor="red";
   vlinkcolor="green";

   for (i=0; i<NMAXMETALINES; i++)
   {
      metaline[i]="";
   }
}

// When an object is initialized with an object of the same type, the
// following function is called.  This next constructor is apparently
// called whenever a function is passed an object as an argument:

webpage::webpage(const webpage& w)
{
   metaline=new string[NMAXMETALINES];
   form=new webimage[NMAXFORMS];
   image=new webimage[NMAXIMAGES];
   link=new weblink[NMAXLINKS];
   table=new webtable[NMAXTABLES];

   docopy(w);
}

webpage::~webpage()
{
   delete [] metaline;
   delete [] form;
   delete [] image;
   delete [] link;
   delete [] table;
}

// ---------------------------------------------------------------------
// As Ed Broach has pointed out, the default C++ =operator for objects
// may simply equate pointers to arrays within objects when one object
// is equated with another.  Individual elements within the arrays
// apparently are not equated to one another by the default C++
// =operator.  This can lead to segmentation errors if the arrays are
// dynamically rather than statically allocated, for the pointer to
// the original array may be destroyed before the elements within the second
// array are copied over.  So we need to write an explicit copy function 
// which transfers all of the subfields within an object to another object
// whenever the object in question has dynamically allocated arrays rather 
// than relying upon C++'s default =operator:

void webpage::docopy(const webpage& w)
{
   int i;

   filenamestr=w.filenamestr;
   title=w.title;
   for (i=0; i<NMAXMETALINES; i++)
   {
      metaline[i]=w.metaline[i];
   }
   backgroundcolor=w.backgroundcolor;
   textcolor=w.textcolor;
   linkcolor=w.linkcolor;
   vlinkcolor=w.vlinkcolor;
//   *webpagestream=w.*webpagestream;


   for (i=0; i<NMAXIMAGES; i++)
   {
      image[i]=w.image[i];
   }
   for (i=0; i<NMAXLINKS; i++)
   {
      link[i]=w.link[i];
   }
   for (i=0; i<NMAXTABLES; i++)
   {
      table[i]=w.table[i];
   }
}	

// Overload = operator:

webpage webpage::operator= (const webpage w)
{
   docopy(w);
   return *this;
}

// --------------------------------------------------------------------- 
void webpage::comment(string commentstring)
{
   *webpagestream << "<!-- ";
   *webpagestream << commentstring;
   *webpagestream << " -->" << endl;
   *webpagestream << endl;
}

// --------------------------------------------------------------------- 
void webpage::horizrule()
{
   string hrule="<hr>";

   *webpagestream << hrule << endl;
   *webpagestream << endl;
}

// --------------------------------------------------------------------- 
void webpage::paragraph()
{
   string parastring="<p>";

   *webpagestream << parastring << endl;
   *webpagestream << endl;
}

// --------------------------------------------------------------------- 
void webpage::vertspace()
{
   *webpagestream << "<br>&nbsp;" << endl;
}

// ---------------------------------------------------------------------
// Routine header writes out preliminary information at the top of the
// webpage file:

  void webpage::header() 
  { 
     int i; 

     *webpagestream << "<html>" << endl; 
     *webpagestream << "<head>" << endl; 
     for (i=0; i<NMAXMETALINES; i++) 
     { 
        if (metaline[i] != "") 
        { 
           *webpagestream << "<meta "+metaline[i]+">" << endl;
        } 
     } 
     *webpagestream << "<title> "+title+" </title>" << endl; 
     *webpagestream << "</head>" << endl; 
     *webpagestream << endl;
  } 

// --------------------------------------------------------------------- 
void webpage::startbody()
{
   string bodyline;

   bodyline="<body bgcolor="+backgroundcolor+" text="+textcolor
      +" link="+linkcolor+" vlink="+vlinkcolor+">";
   *webpagestream << bodyline << endl;
   *webpagestream << endl;
}


// --------------------------------------------------------------------- 
string webpage::insertimage(webimage& i)
{
   string imagestring;

   if (i.horizposn=="center")
   {
      imagestring="<center>";
   }
   else
   {
      imagestring="";
   }
   imagestring+="<img src=\""+i.src+"\"";
   if (i.border != 0)
   {
      imagestring+=" border="+stringfunc::number_to_string(i.border);
   }
   if (i.height != 0)
   {
      imagestring += " height="+stringfunc::number_to_string(i.height);
   }
   if (i.width != 0)
   {
      imagestring += " width="+stringfunc::number_to_string(i.width);
   }
   imagestring += ">";
   
   if (i.horizposn=="center")
   {
      imagestring+="<center>";
   }
   return imagestring;
}

// --------------------------------------------------------------------- 
string webpage::insertlink(weblink& l)
{
   string linkstring;

   linkstring="<a href=\""+l.url+"\">";
   if (l.style=="bold")
   {
      linkstring+="<b>";
   }
   linkstring+=l.descriptor;
   if (l.style=="bold")
   {
      linkstring+="</b>";
   }
   linkstring+="</a>";
   return linkstring;
}

// ---------------------------------------------------------------------
// Subroutine writeform wraps a form header around an output table t
// and writes out the result to the current webpage:

void webpage::writeform(webform& f)
{
   string formstring;
   
   formstring="<form method="+f.method;
   formstring+=" action=\""+f.action+"\"";
   formstring+=" >";
   *webpagestream << formstring << endl;
   writetable(f.formtable);
   *webpagestream << "</form>" << endl;
}

// --------------------------------------------------------------------- 
void webpage::writetable(webtable& t)
{
   int i,j;

   comment("===============================================================");

// Initialize table

   *webpagestream << "<"+t.table_alignment+">" << endl;
   *webpagestream << "<table";
   if (t.border)
   {
      *webpagestream << " border";
   }
   if (t.cellspacing != 0)
   {
      *webpagestream << " cellspacing="+stringfunc::number_to_string(t.cellspacing);
   }
   if (t.cellpadding != 0)
   {
      *webpagestream << " cellpadding="+stringfunc::number_to_string(t.cellpadding);
   }
   *webpagestream << " cols="+stringfunc::number_to_string(t.ncolumns)
      +" width=\""+stringfunc::number_to_string(t.width)+"%\"";
   if (t.bgcolor != "")
   {
      *webpagestream << " bgcolor="+t.bgcolor;
   }
   *webpagestream << " nosave>" << endl;

// Write out table caption:
   
   if (t.caption != "")
   {
      *webpagestream << "<caption>";

      if (t.caption_fontsize > 0)
      {
         *webpagestream << "<font size=+"
            +stringfunc::number_to_string(t.caption_fontsize)+">";
      }
      else if (t.caption_fontsize < 0)
      {
         *webpagestream << "<font size="
            +stringfunc::number_to_string(t.caption_fontsize)+">";
      }

      if (t.caption_fontcolor != "black")
      {
         *webpagestream << "<font color="+t.caption_fontcolor+">";
      }

      if (t.caption_style=="bold")
      {
         *webpagestream << "<b>";
      }

      *webpagestream << t.caption;


      if (t.caption_style=="bold")
      {
         *webpagestream << "</b>";
      }

      if (t.caption_fontcolor != "black")
      {
         *webpagestream << "</font>";
      }
      
      if (t.caption_fontsize != 0)
      {
         *webpagestream << "</font>";
      }

      *webpagestream << "</caption>" << endl;
   }

// Fill in table entries:

   for (i=0; i<t.nrows; i++)
   {
      *webpagestream << "<tr>" << endl;

      j=0;
      while(j<t.ncolumns)
      {
         
//      for (j=0; j<t.ncolumns; j++)
//      {

         *webpagestream << "<td";

         if (t.entry_colspan[i][j] > 1)
         {
            *webpagestream << " colspan="
               +stringfunc::number_to_string(t.entry_colspan[i][j]);
         }
         *webpagestream << " align="+t.entry_alignment[i][j];

         if (t.entry_valignment[i][j] != "")
         {
            *webpagestream << " valign="+t.entry_valignment[i][j];
         }
         *webpagestream << ">";

         if (t.entry_fontsize[i][j] > 0)
         {
            *webpagestream << "<font size=+"
               +stringfunc::number_to_string(t.entry_fontsize[i][j])+">";
         }
         else if (t.entry_fontsize[i][j] < 0)
         {
            *webpagestream << "<font size="
               +stringfunc::number_to_string(t.entry_fontsize[i][j])+">";
         }

         if (t.entry_fontcolor[i][j] != "black")
         {
            *webpagestream << "<font color="+t.entry_fontcolor[i][j]+">";
         }

         if (t.entry_style[i][j]=="bold")
         {
            *webpagestream << "<b>";
         }

         *webpagestream << t.entry[i][j];

         if (t.entry_style[i][j]=="bold")
         {
            *webpagestream << "</b>";
         }

         if (t.entry_fontcolor[i][j] != "black")
         {
            *webpagestream << "</font>";
         }

         if (t.entry_fontsize[i][j] != 0)
         {
            *webpagestream << "</font>";
         }
         *webpagestream << "</td>" << endl;

         j += t.entry_colspan[i][j];
      }
      *webpagestream << "</tr>" << endl;
      *webpagestream << endl;
   }
   *webpagestream << "</table>" << endl;
   *webpagestream << "</"+t.table_alignment+">" << endl;

}

// ---------------------------------------------------------------------
// Subroutine writeparagraph writes out lines of text to the current
// webpage:

void webpage::writeparagraph(bool html_line_breaks,int vertspace,
                             string line[],int nlines)
{
   int i,j;

   j=1;
   for (i=0; i<nlines; i++)
   {
      if (html_line_breaks)
      {
         *webpagestream << line[i]+" <br>" << endl;
      }
      else
      {
         *webpagestream << line[i]  << endl;
      }

// Insert a blank line after every vertspace'th line if vertspace > 0:

      if (vertspace>0 && j%vertspace==0)
      {
         *webpagestream << "<br>" << endl;
      }
      j++;
   }
   paragraph();
}

// --------------------------------------------------------------------- 
void webpage::footer()
{
   *webpagestream << "</body>" << endl;
   *webpagestream << "</html>" << endl;
}

