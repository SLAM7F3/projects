// ==========================================================================
// CHARACTERFUNCS stand-alone methods
// ==========================================================================
// Last modified on 1/27/05; 5/22/06; 7/30/06; 8/3/06
// ==========================================================================

#include <iostream>
#include <vector>
#include "threeDgraphics/character.h"
#include "threeDgraphics/characterfuncs.h"
#include "math/constant_vectors.h"
#include "geometry/linesegment.h"

using std::cin;
using std::cout;
using std::endl;
using std::vector;

namespace characterfunc
{
   const threevector origin(Zero_vector);
   
   linesegment horiz(origin,x_hat);
   linesegment vert(origin,y_hat);
   linesegment rise(origin,x_hat+y_hat);
   linesegment fall(y_hat,x_hat);
   linesegment l0,l1,l2,l3,l4,l5,l6,l7,l8,l9,l10,l11,l12,l13,l14,l15;
   
   character* generate_ascii_characters()
      {
         character* ascii_char=new character[128];
         generate_numeral_characters(ascii_char);
         generate_letter_characters(ascii_char);
         generate_special_characters(ascii_char);
         return ascii_char;
      }
   
   void generate_numeral_characters(character* ascii_char)
      {

// Zero (ascii = 48)

         l0=linesegment(origin,4*y_hat);
         l0.translate(y_hat);
         l1=linesegment(l0);
         l1.translate(3*x_hat);

         linesegment l2a(rise);
         linesegment l2b(horiz);
         linesegment l2c(fall);
         l2a.translate(5*y_hat);
         l2b.translate(x_hat+6*y_hat);
         l2c.translate(2*x_hat+5*y_hat);

         linesegment l3a(fall);
         linesegment l3b(horiz);
         linesegment l3c(rise);
         l3b.translate(x_hat);
         l3c.translate(2*x_hat);

         ascii_char[48].push_back_segment(l0);
         ascii_char[48].push_back_segment(l1);
         ascii_char[48].push_back_segment(l2a);
         ascii_char[48].push_back_segment(l2b);
         ascii_char[48].push_back_segment(l2c);
         ascii_char[48].push_back_segment(l3a);
         ascii_char[48].push_back_segment(l3b);
         ascii_char[48].push_back_segment(l3c);

// One (ascii = 49)

         l0=linesegment(origin,6*y_hat);
         l0.translate(1.5*x_hat);
         l1=linesegment(origin,3*x_hat);
         l2=linesegment(origin,x_hat+y_hat);
         l2.translate(0.5*x_hat+5*y_hat);

         ascii_char[49].push_back_segment(l0);
         ascii_char[49].push_back_segment(l1);
         ascii_char[49].push_back_segment(l2);

// Two (ascii = 50)

         l0=linesegment(origin,3*x_hat);
         l1=linesegment(origin,1.5*y_hat);
         l2=linesegment(1.5*y_hat,3*x_hat+4.5*y_hat);
         l3=linesegment(origin,0.5*y_hat);
         l3.translate(3*x_hat+4.5*y_hat);
         l4=linesegment(fall);
         l4.translate(2*x_hat+5*y_hat);
         l5=linesegment(horiz);
         l5.translate(x_hat+6*y_hat);
         l6=linesegment(rise);
         l6.translate(5*y_hat);
         ascii_char[50].push_back_segment(l0);
         ascii_char[50].push_back_segment(l1);
         ascii_char[50].push_back_segment(l2);
         ascii_char[50].push_back_segment(l3);
         ascii_char[50].push_back_segment(l4);
         ascii_char[50].push_back_segment(l5);
         ascii_char[50].push_back_segment(l6);
         
// Three (ascii = 51)

         l0=fall;
         l1=horiz;
         l1.translate(x_hat);
         l2=rise;
         l2.translate(2*x_hat);
         l3=vert;
         l3.translate(3*x_hat+y_hat);
         l4=fall;
         l4.translate(2*x_hat+2*y_hat);
         l5=linesegment(origin,1.5*x_hat);
         l5.translate(0.5*x_hat+3*y_hat);
         l6=rise;
         l6.translate(2*x_hat+3*y_hat);
         l7=linesegment(vert);
         l7.translate(3*x_hat+4*y_hat);
         l8=linesegment(fall);
         l8.translate(2*x_hat+5*y_hat);
         l9=linesegment(horiz);
         l9.translate(x_hat+6*y_hat);
         l10=linesegment(rise);
         l10.translate(5*y_hat);

         ascii_char[51].push_back_segment(l0);
         ascii_char[51].push_back_segment(l1);
         ascii_char[51].push_back_segment(l2);
         ascii_char[51].push_back_segment(l3);
         ascii_char[51].push_back_segment(l4);
         ascii_char[51].push_back_segment(l5);
         ascii_char[51].push_back_segment(l6);
         ascii_char[51].push_back_segment(l7);
         ascii_char[51].push_back_segment(l8);
         ascii_char[51].push_back_segment(l9);
         ascii_char[51].push_back_segment(l10);

// Four (ascii = 52)

         l0=linesegment(origin,6*y_hat);
         l0.translate(3*x_hat);
         l1=linesegment(origin,3*x_hat);
         l1.translate(3*y_hat);
         l2=linesegment(origin,3*(x_hat+y_hat));
         l2.translate(3*y_hat);
         ascii_char[52].push_back_segment(l0);
         ascii_char[52].push_back_segment(l1);
         ascii_char[52].push_back_segment(l2);

// Five (ascii = 53)

         l0=linesegment(origin,2*x_hat);
         l1=rise;
         l1.translate(2*x_hat);
         l2=vert;
         l2.translate(3*x_hat+y_hat);
         l3=fall;
         l3.translate(2*x_hat+2*y_hat);
         l4=linesegment(l0);
         l4.translate(3*y_hat);
         l5=linesegment(origin,3*y_hat);
         l5.translate(3*y_hat);
         l6=linesegment(origin,3*x_hat);
         l6.translate(6*y_hat);
         ascii_char[53].push_back_segment(l0);
         ascii_char[53].push_back_segment(l1);
         ascii_char[53].push_back_segment(l2);
         ascii_char[53].push_back_segment(l3);
         ascii_char[53].push_back_segment(l4);
         ascii_char[53].push_back_segment(l5);
         ascii_char[53].push_back_segment(l6);

// Six (ascii = 54)

         l0=fall;
         l1=horiz;
         l1.translate(x_hat);
         l2=rise;
         l2.translate(2*x_hat);
         l3=vert;
         l3.translate(3*x_hat+y_hat);
         l4=fall;
         l4.translate(2*x_hat+2*y_hat);
         l5=l1;
         l5.translate(3*y_hat);
         l6=rise;
         l6.translate(2*y_hat);
         l7=linesegment(origin,4*y_hat);
         l7.translate(y_hat);
         l8=rise;
         l8.translate(5*y_hat);
         l9=linesegment(origin,2*x_hat);
         l9.translate(x_hat+6*y_hat);
         ascii_char[54].push_back_segment(l0);
         ascii_char[54].push_back_segment(l1);
         ascii_char[54].push_back_segment(l2);
         ascii_char[54].push_back_segment(l3);
         ascii_char[54].push_back_segment(l4);
         ascii_char[54].push_back_segment(l5);
         ascii_char[54].push_back_segment(l6);
         ascii_char[54].push_back_segment(l7);
         ascii_char[54].push_back_segment(l8);
         ascii_char[54].push_back_segment(l9);

// Seven (ascii = 55)

         l0=linesegment(origin,3*x_hat);
         l0.translate(6*y_hat);
         l1=linesegment(origin,3*x_hat+6*y_hat);
         ascii_char[55].push_back_segment(l0);
         ascii_char[55].push_back_segment(l1);

// Eight (ascii = 56)

         l0=fall;
         l1=horiz;
         l1.translate(x_hat);
         l2=rise;
         l2.translate(2*x_hat);
         l3=vert;
         l3.translate(3*x_hat+y_hat);
         l4=fall;
         l4.translate(2*x_hat+2*y_hat);
         l5=l1;
         l5.translate(3*y_hat);
         l6=rise;
         l6.translate(2*y_hat);
         l7=vert;
         l7.translate(y_hat);

         l8=fall;
         l9=rise;
         l9.translate(2*x_hat);
         l10=vert;
         l10.translate(3*x_hat+y_hat);
         l11=linesegment(fall);
         l11.translate(2*x_hat+2*y_hat);
         l12=linesegment(l1);
         l12.translate(3*y_hat);
         l13=linesegment(rise);
         l13.translate(2*y_hat);
         l14=linesegment(vert);
         l14.translate(y_hat);
         l8.translate(3*y_hat);
         l9.translate(3*y_hat);
         l10.translate(3*y_hat);
         l11.translate(3*y_hat);
         l12.translate(3*y_hat);
         l13.translate(3*y_hat);
         l14.translate(3*y_hat);

         ascii_char[56].push_back_segment(l0);
         ascii_char[56].push_back_segment(l1);
         ascii_char[56].push_back_segment(l2);
         ascii_char[56].push_back_segment(l3);
         ascii_char[56].push_back_segment(l4);
         ascii_char[56].push_back_segment(l5);
         ascii_char[56].push_back_segment(l6);
         ascii_char[56].push_back_segment(l7);
         ascii_char[56].push_back_segment(l8);
         ascii_char[56].push_back_segment(l9);
         ascii_char[56].push_back_segment(l10);
         ascii_char[56].push_back_segment(l11);
         ascii_char[56].push_back_segment(l12);
         ascii_char[56].push_back_segment(l13);        
         ascii_char[56].push_back_segment(l14);

// Nine (ascii = 57)

         l0=fall;
         l1=horiz;
         l1.translate(x_hat);
         l2=rise;
         l2.translate(2*x_hat);
         l3=vert;
         l3.translate(3*x_hat+y_hat);
         l4=fall;
         l4.translate(2*x_hat+2*y_hat);
         l5=l1;
         l5.translate(3*y_hat);
         l6=rise;
         l6.translate(2*y_hat);
         l7=vert;
         l7.translate(y_hat);
         l8=linesegment(origin,4*y_hat);

         l0.translate(3*y_hat);
         l1.translate(3*y_hat);
         l2.translate(3*y_hat);
         l3.translate(3*y_hat);
         l4.translate(3*y_hat);
         l5.translate(3*y_hat);
         l6.translate(3*y_hat);
         l7.translate(3*y_hat);
         l8.translate(3*x_hat);

         ascii_char[57].push_back_segment(l0);
         ascii_char[57].push_back_segment(l1);
         ascii_char[57].push_back_segment(l2);
         ascii_char[57].push_back_segment(l3);
         ascii_char[57].push_back_segment(l4);
         ascii_char[57].push_back_segment(l5);
         ascii_char[57].push_back_segment(l6);
         ascii_char[57].push_back_segment(l7);
         ascii_char[57].push_back_segment(l8);
      }
   
   void generate_special_characters(character* ascii_char)
      {

// space (ascii = 32)
         
         ascii_char[32].set_extent(linesegment(origin,4*x_hat));

// minus sign (ascii = 45)
         
         l0=linesegment(origin,3*x_hat);
         l0.translate(3*y_hat);

         ascii_char[45].push_back_segment(l0);

// period (ascii = 46)
         
         l0=linesegment(fall);
         l1=linesegment(horiz);
         l1.translate(x_hat);
         l2=linesegment(rise);
         l2.translate(2*x_hat);
         l3=linesegment(vert);
         l3.translate(3*x_hat+y_hat);
         l4=linesegment(fall);
         l4.translate(2*x_hat+2*y_hat);
         l5=linesegment(l1);
         l5.translate(3*y_hat);
         l6=linesegment(rise);
         l6.translate(2*y_hat);
         l7=linesegment(vert);
         l7.translate(y_hat);

         ascii_char[46].push_back_segment(l0);
         ascii_char[46].push_back_segment(l1);
         ascii_char[46].push_back_segment(l2);
         ascii_char[46].push_back_segment(l3);
         ascii_char[46].push_back_segment(l4);
         ascii_char[46].push_back_segment(l5);
         ascii_char[46].push_back_segment(l6);
         ascii_char[46].push_back_segment(l7);
         ascii_char[46].scale(Zero_vector,0.333);

// For numbers with decimals, we want the spacing before the period to
// be the same as that after the period.  We therefore translate the
// period backwards by 1/3 of a standard character extent:

         ascii_char[46].translate(-0.333*x_hat);

// Heart (ascii taken to equal 64 = at sign)

         l0=linesegment(5*y_hat,2*x_hat);
         l1=linesegment(2*x_hat,4*x_hat+5*y_hat);
         l2=linesegment(origin,0.666*x_hat+y_hat);
         l2.translate(5*y_hat);
         l3=linesegment(origin,0.666*x_hat);
         l3.translate(0.666*x_hat+6*y_hat);
         l4=linesegment(y_hat,0.666*x_hat);
         l4.translate(1.333*x_hat+5*y_hat);
         l5=l2;
         l6=l3;
         l7=l4;
         l5.translate(2*x_hat);
         l6.translate(2*x_hat);
         l7.translate(2*x_hat);
         ascii_char[64].push_back_segment(l0);
         ascii_char[64].push_back_segment(l1);
         ascii_char[64].push_back_segment(l2);
         ascii_char[64].push_back_segment(l3);
         ascii_char[64].push_back_segment(l4);
         ascii_char[64].push_back_segment(l5);
         ascii_char[64].push_back_segment(l6);
         ascii_char[64].push_back_segment(l7);
      }

   void generate_letter_characters(character* ascii_char)
      {

// A (ascii = 65)

         l0=linesegment(origin,5*y_hat);
         l1=linesegment(l0);
         l1.translate(3*x_hat);
         l2=linesegment(3*y_hat,3*x_hat+3*y_hat);
         l3=linesegment(rise);
         l3.translate(5*y_hat);
         l4=linesegment(horiz);
         l4.translate(x_hat+6*y_hat);
         l5=linesegment(fall);
         l5.translate(2*x_hat+5*y_hat);
         ascii_char[65].push_back_segment(l0);
         ascii_char[65].push_back_segment(l1);
         ascii_char[65].push_back_segment(l2);
         ascii_char[65].push_back_segment(l3);
         ascii_char[65].push_back_segment(l4);
         ascii_char[65].push_back_segment(l5);

// B (ascii = 66)
     
         l0=linesegment(origin,6*y_hat);
         l1=linesegment(origin,2*x_hat);
         l2=rise;
         l2.translate(2*x_hat);
         l3=vert;
         l3.translate(3*x_hat+y_hat);
         l4=fall;
         l4.translate(2*x_hat+2*y_hat);
         l5=linesegment(l1);
         l5.translate(3*y_hat);

         l6=linesegment(l2);
         l7=linesegment(l3);
         l8=linesegment(l4);
         l9=linesegment(l5);
         
         l6.translate(3*y_hat);
         l7.translate(3*y_hat);
         l8.translate(3*y_hat);
         l9.translate(3*y_hat);

         ascii_char[66].push_back_segment(l0);
         ascii_char[66].push_back_segment(l1);
         ascii_char[66].push_back_segment(l2);
         ascii_char[66].push_back_segment(l3);
         ascii_char[66].push_back_segment(l4);
         ascii_char[66].push_back_segment(l5);
         ascii_char[66].push_back_segment(l6);
         ascii_char[66].push_back_segment(l7);
         ascii_char[66].push_back_segment(l8);
         ascii_char[66].push_back_segment(l9);

// C (ascii = 67)
         
         l0=fall;
         l1=horiz;
         l1.translate(x_hat);
         l2=rise;
         l2.translate(2*x_hat);
         l3=linesegment(origin,0.75*y_hat);
         l3.translate(3*x_hat+y_hat);
         l4=linesegment(origin,4*y_hat);
         l4.translate(y_hat);
         l5=linesegment(rise);
         l5.translate(5*y_hat);
         l6=linesegment(horiz);
         l6.translate(x_hat+6*y_hat);
         l7=linesegment(fall);
         l7.translate(2*x_hat+5*y_hat);
         l8=linesegment(origin,0.75*y_hat);
         l8.translate(3*x_hat+4.25*y_hat);

         ascii_char[67].push_back_segment(l0);
         ascii_char[67].push_back_segment(l1);
         ascii_char[67].push_back_segment(l2);
         ascii_char[67].push_back_segment(l3);
         ascii_char[67].push_back_segment(l4);
         ascii_char[67].push_back_segment(l5);
         ascii_char[67].push_back_segment(l6);
         ascii_char[67].push_back_segment(l7);
         ascii_char[67].push_back_segment(l8);

// D (ascii = 68)
         
         l0=linesegment(origin,6*y_hat);
         l1=linesegment(origin,2*x_hat);
         l2=rise;
         l2.translate(2*x_hat);
         l3=l1;
         l3.translate(6*y_hat);
         l4=fall;
         l4.translate(2*x_hat+5*y_hat);
         l5=linesegment(origin,4*y_hat);
         l5.translate(3*x_hat+y_hat);
         ascii_char[68].push_back_segment(l0);
         ascii_char[68].push_back_segment(l1);
         ascii_char[68].push_back_segment(l2);
         ascii_char[68].push_back_segment(l3);
         ascii_char[68].push_back_segment(l4);
         ascii_char[68].push_back_segment(l5);

// E (ascii = 69)
         
         l0=linesegment(origin,3*x_hat);
         l1=linesegment(l0);
         l1.translate(3*y_hat);
         l2=linesegment(l0);
         l2.translate(6*y_hat);
         l3=linesegment(origin,6*y_hat);

         ascii_char[69].push_back_segment(l0);
         ascii_char[69].push_back_segment(l1);
         ascii_char[69].push_back_segment(l2);
         ascii_char[69].push_back_segment(l3);

// F (ascii = 70)
         
         l0=linesegment(origin,3*x_hat);
         l0.translate(3*y_hat);
         l1=linesegment(l0);
         l1.translate(3*y_hat);
         l2=linesegment(origin,6*y_hat);

         ascii_char[70].push_back_segment(l0);
         ascii_char[70].push_back_segment(l1);
         ascii_char[70].push_back_segment(l2);

// G (ascii = 71)
         
         l0=fall;
         l1=horiz;
         l1.translate(x_hat);
         l2=rise;
         l2.translate(2*x_hat);
         l3=linesegment(origin,1.5*y_hat);
         l3.translate(3*x_hat+y_hat);
         l4=linesegment(origin,1.5*x_hat);
         l4.translate(2*x_hat+2.5*y_hat);
         l5=linesegment(origin,4*y_hat);
         l5.translate(y_hat);
         l6=linesegment(rise);
         l6.translate(5*y_hat);
         l7=linesegment(horiz);
         l7.translate(x_hat+6*y_hat);
         l8=linesegment(fall);
         l8.translate(2*x_hat+5*y_hat);
         l9=linesegment(origin,0.5*y_hat);
         l9.translate(3*x_hat+4.5*y_hat);

         ascii_char[71].push_back_segment(l0);
         ascii_char[71].push_back_segment(l1);
         ascii_char[71].push_back_segment(l2);
         ascii_char[71].push_back_segment(l3);
         ascii_char[71].push_back_segment(l4);
         ascii_char[71].push_back_segment(l5);
         ascii_char[71].push_back_segment(l6);
         ascii_char[71].push_back_segment(l7);
         ascii_char[71].push_back_segment(l8);
         ascii_char[71].push_back_segment(l9);

// H (ascii = 72)
         
         l0=linesegment(origin,6*y_hat);
         l1=linesegment(l0);
         l1.translate(3*x_hat);
         l2=linesegment(origin,3*x_hat);
         l2.translate(3*y_hat);
         ascii_char[72].push_back_segment(l0);
         ascii_char[72].push_back_segment(l1);
         ascii_char[72].push_back_segment(l2);

// I (ascii = 73)
         
         l0=linesegment(origin,6*y_hat);
         l0.translate(1.5*x_hat);
         l1=linesegment(origin,1.5*x_hat);
         l1.translate(0.75*x_hat);
         l2=l1;
         l2.translate(6*y_hat);
         ascii_char[73].push_back_segment(l0);
         ascii_char[73].push_back_segment(l1);
         ascii_char[73].push_back_segment(l2);
         ascii_char[73].translate(-0.375*x_hat);
         ascii_char[73].set_extent(linesegment(origin,2.75*x_hat));

// J (ascii = 74)
         
         l0=fall;
         l1=horiz;
         l1.translate(x_hat);
         l2=rise;
         l2.translate(2*x_hat);
         l3=linesegment(origin,5*y_hat);
         l3.translate(3*x_hat+y_hat);
         l4=linesegment(origin,y_hat);
         l4.translate(y_hat);
         ascii_char[74].push_back_segment(l0);
         ascii_char[74].push_back_segment(l1);
         ascii_char[74].push_back_segment(l2);
         ascii_char[74].push_back_segment(l3);
         ascii_char[74].push_back_segment(l4);

// K (ascii = 75)
         
         l0=linesegment(origin,6*y_hat);
         l1=linesegment(3*y_hat,3*x_hat);
         l2=linesegment(3*y_hat,3*x_hat+6*y_hat);
         ascii_char[75].push_back_segment(l0);
         ascii_char[75].push_back_segment(l1);
         ascii_char[75].push_back_segment(l2);

// L (ascii = 76)
         
         l0=linesegment(origin,6*y_hat);
         l1=linesegment(origin,3*x_hat);
         ascii_char[76].push_back_segment(l0);
         ascii_char[76].push_back_segment(l1);

// M (ascii = 77)
     
         l0=linesegment(origin,6*y_hat);
         l1=linesegment(origin,6*y_hat);
         l1.translate(4*x_hat);
         l2=linesegment(l0.get_v2(),2*x_hat+3*y_hat);
         l3=linesegment(l1.get_v2(),2*x_hat+3*y_hat);
         ascii_char[77].push_back_segment(l0);
         ascii_char[77].push_back_segment(l1);
         ascii_char[77].push_back_segment(l2);
         ascii_char[77].push_back_segment(l3);
         ascii_char[77].set_extent(linesegment(origin,5*x_hat));

// N (ascii = 78)
     
         l0=linesegment(origin,6*y_hat);
         l1=linesegment(origin,6*y_hat);
         l1.translate(3*x_hat);
         l2=linesegment(l0.get_v2(),l1.get_v1());
         ascii_char[78].push_back_segment(l0);
         ascii_char[78].push_back_segment(l1);
         ascii_char[78].push_back_segment(l2);

// O (ascii = 79)
     
         ascii_char[79]=ascii_char[48];	// same as zero

// P (ascii = 80)
     
         l0=linesegment(origin,2*x_hat);
         l1=rise;
         l1.translate(2*x_hat);
         l2=vert;
         l2.translate(3*x_hat+y_hat);
         l3=fall;
         l3.translate(2*x_hat+2*y_hat);
         l4=linesegment(l0);
         l4.translate(3*y_hat);
         l0.translate(3*y_hat);
         l1.translate(3*y_hat);
         l2.translate(3*y_hat);
         l3.translate(3*y_hat);
         l4.translate(3*y_hat);
         l5=linesegment(origin,6*y_hat);
         ascii_char[80].push_back_segment(l0);
         ascii_char[80].push_back_segment(l1);
         ascii_char[80].push_back_segment(l2);
         ascii_char[80].push_back_segment(l3);
         ascii_char[80].push_back_segment(l4);
         ascii_char[80].push_back_segment(l5);

// Q (ascii = 81)
     
         ascii_char[81]=ascii_char[48];	
         l0=linesegment(1.75*x_hat+1.25*y_hat,3*x_hat);
         ascii_char[81].push_back_segment(l0);

// R (ascii = 82)
     
         ascii_char[82]=ascii_char[80];
         l0=linesegment(0.5*x_hat+3*y_hat,3*x_hat);
         ascii_char[82].push_back_segment(l0);

// S (ascii = 83)

         l0=linesegment(origin,2*x_hat);
         l1=rise;
         l1.translate(l0.get_v2());
         l2=vert;
         l2.translate(l1.get_v2());
//         l3=fall;
         l3=linesegment(0.5*x_hat+y_hat,x_hat);
         l3.translate(2*x_hat+2*y_hat);
//         l4=horiz;
         l4=linesegment(origin,2*x_hat);
         l4.translate(0.5*x_hat+3*y_hat);
         l5=linesegment(y_hat,0.5*x_hat);
//         l5=fall;
         l5.translate(3*y_hat);
         l6=linesegment(vert);
         l6.translate(4*y_hat);
         l7=linesegment(rise);
         l7.translate(5*y_hat);
         l8=linesegment(l0);
         l8.translate(x_hat+6*y_hat);
         
         ascii_char[83].push_back_segment(l0);
         ascii_char[83].push_back_segment(l1);
         ascii_char[83].push_back_segment(l2);
         ascii_char[83].push_back_segment(l3);
         ascii_char[83].push_back_segment(l4);
         ascii_char[83].push_back_segment(l5);
         ascii_char[83].push_back_segment(l6);
         ascii_char[83].push_back_segment(l7);
         ascii_char[83].push_back_segment(l8);

// T (ascii = 84)
     
         l0=linesegment(origin,3*x_hat);
         l0.translate(6*y_hat);
         l1=linesegment(origin,6*y_hat);
         l1.translate(1.5*x_hat);
         ascii_char[84].push_back_segment(l0);
         ascii_char[84].push_back_segment(l1);

// U (ascii = 85)

         l0=linesegment(y_hat,6*y_hat);
         l1=linesegment(l0);
         l1.translate(3*x_hat);
         l2=linesegment(fall);
         l3=linesegment(horiz);
         l3.translate(x_hat);
         l4=linesegment(rise);
         l4.translate(2*x_hat);
         ascii_char[85].push_back_segment(l0);
         ascii_char[85].push_back_segment(l1);
         ascii_char[85].push_back_segment(l2);
         ascii_char[85].push_back_segment(l3);
         ascii_char[85].push_back_segment(l4);

// V (ascii = 86)

         l0=linesegment(6*y_hat,1.5*x_hat);
         l1=linesegment(1.5*x_hat,3*x_hat+6*y_hat);
         ascii_char[86].push_back_segment(l0);
         ascii_char[86].push_back_segment(l1);

// W (ascii = 87)
     
         l0=linesegment(6*y_hat,0.5*x_hat);
         l1=linesegment(l0.get_v2(),2*x_hat+3*y_hat);
         l2=linesegment(l1.get_v2(),3.5*x_hat);
         l3=linesegment(l2.get_v2(),4*x_hat+6*y_hat);

         ascii_char[87].push_back_segment(l0);
         ascii_char[87].push_back_segment(l1);
         ascii_char[87].push_back_segment(l2);
         ascii_char[87].push_back_segment(l3);
         ascii_char[87].set_extent(linesegment(origin,5*x_hat));

// X (ascii = 88)

         l0=linesegment(6*y_hat,3*x_hat);
         l1=linesegment(origin,3*x_hat+6*y_hat);
         ascii_char[88].push_back_segment(l0);
         ascii_char[88].push_back_segment(l1);

// Y (ascii = 89)

         l0=linesegment(6*y_hat,1.5*x_hat+3*y_hat);
         l1=linesegment(3*x_hat+6*y_hat,1.5*x_hat+3*y_hat);
         l2=linesegment(origin,3*y_hat);
         l2.translate(1.5*x_hat);
         ascii_char[89].push_back_segment(l0);
         ascii_char[89].push_back_segment(l1);
         ascii_char[89].push_back_segment(l2);

// Z (ascii = 90)

         l0=linesegment(origin,3*x_hat);
         l1=linesegment(origin,3*x_hat+6*y_hat);
         l2=linesegment(l0);
         l2.translate(6*y_hat);
         ascii_char[90].push_back_segment(l0);
         ascii_char[90].push_back_segment(l1);
         ascii_char[90].push_back_segment(l2);
      }   

} // characterfunc namespace




