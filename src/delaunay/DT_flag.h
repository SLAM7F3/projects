// ==========================================================================
// Header file for DT_FLAG class
// ==========================================================================
// Last modified on 12/14/05
// ==========================================================================

#ifndef DT_FLAG_H
#define DT_FLAG_H

class DT_flag
{

   typedef unsigned char Index;	// used for flag and Index in array

   public :

      DT_flag();

      void infinite(int i)     { f |= (Index) i ; }
      void last_finite()       { f |= 8;}
      void kill()              { f |= 16;}
      Index is_infinite()   	 { return f & 7;}
      Index is_last_finite()   { return f & 8;}
      Index is_dead()      	 { return f & 16;}
      Index is_degenerate()    { return f & 32;}

   private :

      Index f;
};

#endif  // DT_FLAG_H
