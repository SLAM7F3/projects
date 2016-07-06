// ==========================================================================
// Templatized Mynode class member function definitions
// ==========================================================================
// Last modified on 6/7/04
// ==========================================================================

// ==========================================================================
// Initialization, constructor and destructor methods:
// ==========================================================================

template <class T> Mynode<T>::Mynode()
{
   initialize_member_objects();
}

template <class T> Mynode<T>::Mynode(const T& d)
{
   initialize_member_objects();
   data=d;
}

// Copy constructor:

template <class T> Mynode<T>::Mynode(const Mynode<T>& node)
{
   initialize_member_objects();
   docopy(node);
}

template <class T> Mynode<T>::~Mynode()
{
   prev_ptr=NULL;
   next_ptr=NULL;
}

// ---------------------------------------------------------------------
template <class T> void Mynode<T>::docopy(const Mynode<T>& node)
{
   ID=node.ID;
   data=node.data;
   order=node.order;
}

// Overload = operator:

template <class T> Mynode<T>& Mynode<T>::operator= 
(const Mynode<T>& n)
{
   if (this==&n) return *this;
   docopy(n);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

template <class T> std::ostream& operator<< 
(std::ostream& outstream,const Mynode<T>& node)
{
   outstream << std::endl;
   outstream << "ID = " << node.ID << std::endl;
//   outstream << "order = " << node.order << std::endl;
   outstream << "data = " << node.get_data() << std::endl;

// On 2/22/02, we learned from Tara Dennis that we must always return
// the output stream which was the overloaded << operator takes as
// input in order for concatenation such as 

// 	"std::cout << *currnode_ptr << << *nextnode_ptr << foo << std::endl;"

// to work.

   return outstream;
}








