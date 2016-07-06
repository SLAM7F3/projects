// ==========================================================================
// Webtable class member function definitions which allow one to
// create HTML tables using C++.
// ==========================================================================
// Last modified on 3/24/02
// ==========================================================================

#include "webtable.h"

using std::string;
using std::ostream;
using std::endl;

const int webtable::BIGFONT=0;
const int webtable::SMALLFONT=0;

// ---------------------------------------------------------------------
// Constructor functions:
// ---------------------------------------------------------------------

webtable::webtable(void)
{
   int i,j;
   
   entry_alignment=new string[NMAXROWS][NMAXCOLUMNS];
   entry_valignment=new string[NMAXROWS][NMAXCOLUMNS];
   entry_fontsize=new int[NMAXROWS][NMAXCOLUMNS];
   entry_fontcolor=new string[NMAXROWS][NMAXCOLUMNS];
   entry_style=new string[NMAXROWS][NMAXCOLUMNS];
   entry_colspan=new int[NMAXROWS][NMAXCOLUMNS];
   entry=new string[NMAXROWS][NMAXCOLUMNS];

   border=false;
   cellspacing=cellpadding=0;
   bgcolor=caption=caption_style="";
   caption_fontcolor="black";
   caption_fontsize=0;
   
   for (i=0; i<NMAXROWS; i++)
   {
      for (j=0; j<NMAXCOLUMNS; j++)
      {
         entry_fontsize[i][j]=0;
         entry_fontcolor[i][j]="black";
         entry_style[i][j]="";
         entry_alignment[i][j]=entry_valignment[i][j]=entry[i][j]="";
         entry_colspan[i][j]=1;
      }
   }
}

// When an object is initialized with an object of the same type, the
// following function is called.  This next constructor is apparently
// called whenever a function is passed an object as an argument:

webtable::webtable(const webtable& t)
{
   int i,j;

   entry_alignment=new string[NMAXROWS][NMAXCOLUMNS];
   entry_valignment=new string[NMAXROWS][NMAXCOLUMNS];
   entry_fontsize=new int[NMAXROWS][NMAXCOLUMNS];
   entry_fontcolor=new string[NMAXROWS][NMAXCOLUMNS];
   entry_style=new string[NMAXROWS][NMAXCOLUMNS];
   entry_colspan=new int[NMAXROWS][NMAXCOLUMNS];
   entry=new string[NMAXROWS][NMAXCOLUMNS];
   
   for (i=0; i<NMAXROWS; i++)
   {
      for (j=0; j<NMAXCOLUMNS; j++)
      {
         entry_alignment[i][j]=entry_valignment[i][j]=entry[i][j]="";
      }
   }
   docopy(t);
}

webtable::~webtable()
{
   delete [] entry_alignment;
   delete [] entry_valignment;
   delete [] entry_fontsize;
   delete [] entry_fontcolor;
   delete [] entry_style;
   delete [] entry_colspan;
   delete [] entry;
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

void webtable::docopy(const webtable& t)
{
   int i,j;

   border=t.border;
   cellspacing=t.cellspacing;
   cellpadding=t.cellpadding;
   nrows=t.nrows;
   ncolumns=t.ncolumns;
   width=t.width;

   table_alignment=t.table_alignment;
   bgcolor=t.bgcolor;
   caption=t.caption;
   caption_style=t.caption_style;
   caption_fontcolor=t.caption_fontcolor;
   caption_fontsize=t.caption_fontsize;
   
   for (i=0; i<NMAXROWS; i++)
   {
      for (j=0; j<NMAXCOLUMNS; j++)
      {
         entry_alignment[i][j]=t.entry_alignment[i][j];
         entry_valignment[i][j]=t.entry_valignment[i][j];
         entry_fontsize[i][j]=t.entry_fontsize[i][j];
         entry_fontcolor[i][j]=t.entry_fontcolor[i][j];
         entry_style[i][j]=t.entry_style[i][j];
         entry_colspan[i][j]=t.entry_colspan[i][j];
         entry[i][j]=t.entry[i][j];
      }
   }
}	

// Overload = operator:

webtable webtable::operator= (const webtable t)
{
   docopy(t);
   return *this;
}



