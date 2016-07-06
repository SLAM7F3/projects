// ==========================================================================
// Header for templatized ObjectPool class
// ==========================================================================
// Last updated on 6/21/05; 12/9/06; 7/5/12
// ==========================================================================

// This code was adapated from source taken from
// www.codeproject.com/cpp/objpool.asp.  It is described in "Object
// pooling for generic C++ classes" by Thomas George, 5 May 2003.  

// Notes from this online article:

// Introduction:
// ------------

// The default memory allocator is not efficient when it comes to
// frequent new and delete operations.  There are a number of general
// purpose allocators that replace the standard one.  There are
// certain situations when you want to improve performance at the cost
// of using more memory.  This is especially true for small objects
// that are frequently created and destroyed in processing intensive
// applications.  I decided to create a class that pools instances of
// classes so that new and delete work on an array of already existing
// objects.

// Usage:
// -----

// To enable pooling for a class, you publically inherit from this class:

// class myclasstobepooled : public ObjectPool< myclasstobepooled >

// Execute call to myclasstobepooled::delete_pooled_memory() after all
// objects are to be genuinely deallocated.

// Implementation:
// --------------

// The ObjectPool class maintains a static vector of pointers of the
// template type.  The overridden new and delete operators manage the
// list.

#ifndef T_OBJPOOL_H
#define T_OBJPOOL_H

#include <iostream>
#include <string>
#include <vector>

template <class T > class ObjectPool
{
  public:

   ObjectPool() {}
   virtual ~ObjectPool() {}

   static std::vector<T* >& get_list()
      {
         static std::vector<T* > m_free;
         return m_free;
      }

   static void create_pooled_memory(int n_new_objects)
      {
//         std::cout << "inside objpool::create_pooled_memory() #1" << std::endl;
         for (int n=0; n<n_new_objects; n++)
         {
            T* p = ::new T;
            get_list().push_back(p);
         }
//         std::cout << "get_list().size() = "
//                   << get_list().size() << std::endl;
      }

   static void create_pooled_memory()
      {
//         static int new_call_counter=0;

//         std::cout << "inside objpool::create_pooled_memory() #2" << std::endl;
//         std::cout << "get_list().size() = "
//                   << get_list().size() << std::endl;
//         const int n_new_objects=2;
         const int n_new_objects=128;
         for (int n=0; n<n_new_objects; n++)
         {
            T* p = ::new T;
            get_list().push_back(p);
//            std::cout << "Number of actual new calls = " << new_call_counter++
//                      << std::endl;
         }
      }

   static void delete_pooled_memory()
      {
//         std::cout << "inside objpool::delete_pooled_memory()" << std::endl;
//         std::cout << "get_list().size() = "
//                   << get_list().size() << std::endl;

         typename std::vector<T* >::iterator first = get_list().begin();
         typename std::vector<T* >::iterator last = get_list().end();
         while (first != last)
         {
            T* p = *first; 
            ++first;
            ::delete p;
         }
         get_list().erase(get_list().begin(), get_list().end());
      }

   inline void* operator new( size_t stAllocateBlock)
      {
//         std::cout << "inside objpool::new()" << std::endl;
//         std::cout << "get_list().size() = "
//                   << get_list().size() << std::endl;

         if (get_list().size() <= 0) create_pooled_memory();
         T* p = get_list().back();
         get_list().pop_back();
         return p;
      }

   inline void operator delete( void* p )
      {
         get_list().push_back((T*)p);
//         std::cout << "inside objpool::delete()" << std::endl;
//         std::cout << "get_list().size() = "
//                   << get_list().size() << std::endl;
      }

};

#endif  // T_Objpool.h

