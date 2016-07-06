// ==========================================================================
// Header file for templatized Stack class
// ==========================================================================
// Last modified on 1/12/04; 8/13/12
// ==========================================================================

#ifndef STACK_H
#define STACK_H

template <class T>
class Stack
{

  public:

// Initialization, constructor and destructor functions:

   Stack(int s=10);
   Stack(const Stack<T>& s);
   virtual ~Stack();
   Stack& operator= (const Stack<T>& s);
//   friend std::ostream& operator<< (std::ostream& outstream,
//                                    Stack<T> const & s);

   bool push(const T &pushValue);
   bool pop(T &popValue);
   
   bool isEmpty() const;
   bool isFull() const;

  protected:

  private:

   int size;
   int top;
   T* stack_ptr;

   void allocate_member_objects();
   void initialize_member_objects(int s);
   void docopy(const Stack<T>& s);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

template <class T> inline bool Stack<T>::isEmpty() const
{
   return top==-1;
}

template <class T> inline bool Stack<T>::isFull() const
{
   return top==size-1;
}

#include "Stack.cc"

#endif  // datastructures/Stack.h


