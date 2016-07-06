// ==========================================================================
// MyClass class member function definitions
// ==========================================================================
// Last modified on 7/23/07
// ==========================================================================

#include "MyClass.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void MyClass::allocate_member_objects()
{
}		       

void MyClass::initialize_member_objects()
{
}

MyClass::MyClass(QWidget* parent_ptr):
   QWidget(parent_ptr)
{
   allocate_member_objects();
   initialize_member_objects();

   setup_layouts();
}

MyClass::~MyClass()
{
}

// ---------------------------------------------------------------------
void MyClass::setup_layouts()
{
//   setLayout(top_layout);
}
