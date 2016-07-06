// ==========================================================================
// Templatized Strand class member function definitions
// ==========================================================================
// Last modified on 3/18/05
// ==========================================================================

#include <iostream>

using std::cout;
using std::endl;
using std::ostream;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

template <class T_ptr> void Strand<T_ptr>::allocate_member_objects()
{
}		       

template <class T_ptr> void Strand<T_ptr>::initialize_member_objects()
{
   ID=-1;
}		       

template <class T_ptr> Strand<T_ptr>::Strand()
{	
   initialize_member_objects();
   allocate_member_objects();
}		       

template <class T_ptr> Strand<T_ptr>::Strand(int identification)
{	
   initialize_member_objects();
   allocate_member_objects();
   ID=identification;
}		       

// Copy constructor:

template <class T_ptr> Strand<T_ptr>::Strand(const Strand<T_ptr>& s)
{
   initialize_member_objects();
   allocate_member_objects();
   docopy(s);
}

template <class T_ptr> Strand<T_ptr>::~Strand()
{
}

// ---------------------------------------------------------------------
template <class T_ptr> void Strand<T_ptr>::docopy(const Strand<T_ptr>& s)
{
   ID=s.ID;
}

// ---------------------------------------------------------------------
// Overload = operator:

template <class T_ptr> Strand<T_ptr>& Strand<T_ptr>::operator= 
(const Strand<T_ptr>& s)
{
   if (this==&s) return *this;
   docopy(s);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class T_ptr> ostream& operator<< 
(std::ostream& outstream,const Strand<T_ptr>& s)
{
   outstream << std::endl;
   outstream << "Strand #" << s.ID << " contains " 
             << s.size() << " sites:" << endl;
   int i=0;
   for (const Mynode<T_ptr>* currnode_ptr=s.get_start_ptr(); 
        currnode_ptr != NULL; currnode_ptr=currnode_ptr->get_nextptr())
   {
      cout << "   " << i 
           << " Site info = " << currnode_ptr->get_data() << endl;
      i++;
   }
   return outstream;
}
