// ==========================================================================
// Header file for MyClass class
// ==========================================================================
// Last modified on 7/23/07
// ==========================================================================

#ifndef MYCLASS_H
#define MYCLASS_H

#include <iostream>
#include <string>
#include <vector>
#include <QtGui>

class MyClass: public QWidget
{

   Q_OBJECT

  public:

// Initialization, constructor and destructor functions:

   MyClass(QWidget* parent_ptr);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~MyClass();

// Set and get member functions:



   void setup_layouts();

  private: 

   void allocate_member_objects();
   void initialize_member_objects();
//   void docopy(const MyClass& p);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================

/*
inline int MyClass::get_n_upward_rays() const
{
   return n_upward_rays;
}
*/


#endif  // MyClass.h



