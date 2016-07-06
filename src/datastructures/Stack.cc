// ==========================================================================
// Templatized Stack class member function definitions
// ==========================================================================
// Last modified on 2/5/04
// ==========================================================================

// ==========================================================================
// Initialization, constructor and destructor methods:
// ==========================================================================

template <class T> 
void Stack<T>::initialize_member_objects(int s)
{
   size=s > 0 ? s : 10;
   top=-1;
   stack_ptr=NULL;
}

template <class T> 
void Stack<T>::allocate_member_objects()
{
   stack_ptr=new T[size];
}

template <class T> 
Stack<T>::Stack(int s)
{
   initialize_member_objects(s);
   allocate_member_objects();
}

// When an object is initialized with an object of the same type, the
// following function is called.  This next constructor is apparently
// called whenever a function is passed an object as an argument.  As
// James Wanken reminded us on 10/19/00, any time we have dynamic
// member variables within a class, we must first be sure to allocate
// memory space for them first before we call the docopy command below!

template <class T> 
Stack<T>::Stack(const Stack<T>& s)
{
   docopy(s);
}

template <class T> 
Stack<T>::~Stack()
{
   delete [] stack_ptr;
   stack_ptr=NULL;
}

// ---------------------------------------------------------------------
template <class T> 
void Stack<T>::docopy(const Stack<T>& s)
{
   size=s.size;
   top=s.top;
}

// Overload = operator:

template <class T> 
Stack<T>& Stack<T>::operator= (const Stack<T>& s)
{
   if (this==&s) return *this;
   docopy(s);
   return *this;
}

// ---------------------------------------------------------------------
// Overload << operator:

/*
template <class T>
std::ostream& operator<< (ostream& outstream,Stack<T> const & s)
{
   std::outstream << std::endl;
   std::outstream << "size = " << s.size << std::endl;
   std::outstream << "top = " << s.top << std::endl;

   return std::outstream;
}
*/

// ==========================================================================
// ==========================================================================

template <class T>
bool Stack<T>::push(const T &pushValue)
{
   if (!isFull())
   {
      stack_ptr[++top]=pushValue;
      return true;
   }
   return false;
}

template <class T>
bool Stack<T>::pop(T &popValue)
{
   if (!isEmpty())
   {
      popValue=stack_ptr[top--];
      return true;
   }
   return false;
}

