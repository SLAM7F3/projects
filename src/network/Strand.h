// ==========================================================================
// Header file for templatized STRAND class
// ==========================================================================
// Last modified on 3/18/05; 6/15/06
// ==========================================================================

#ifndef T_STRAND_H
#define T_STRAND_H

#include "datastructures/Linkedlist.h"

template <class T_ptr>
class Strand: public Linkedlist<T_ptr>
{

  public:

// Initialization, constructor and destructor functions:

   Strand();
   Strand(int identification);
   Strand(const Strand<T_ptr>& s);
   virtual ~Strand();
   Strand& operator= (const Strand<T_ptr>& s);

   template <class T1_ptr>
   friend std::ostream& operator<< 
      (std::ostream& outstream,const Strand<T1_ptr>& s);

// Set & get member functions:

   void set_ID(const int id);
   int get_ID() const;

  private:

   int ID;

   void allocate_member_objects();
   void initialize_member_objects();
   void docopy(const Strand<T_ptr>& s);
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

// Set & get member functions:

template <class T_ptr> inline void Strand<T_ptr>::set_ID(const int id)
{
   ID=id;
}

template <class T_ptr> inline int Strand<T_ptr>::get_ID() const
{
   return ID;
}

#include "Strand.cc"

#endif // T_Strand.h



