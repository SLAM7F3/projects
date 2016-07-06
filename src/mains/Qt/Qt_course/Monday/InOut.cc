// ==========================================================================
// InOut class member function definitions
// ==========================================================================
// Last modified on 7/23/07
// ==========================================================================

#include "InOut.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void InOut::allocate_member_objects()
{
   slider_ptr=new QSlider(Qt::Horizontal);
   lcdnumber_ptr=new QLCDNumber();
   Layout_ptr=new QHBoxLayout;
}		       

void InOut::initialize_member_objects()
{
   slider_ptr->setRange(0,100);
}

InOut::InOut(QWidget* parent_ptr):
   QWidget(parent_ptr)
{
   allocate_member_objects();
   initialize_member_objects();

   setup_layouts();
   setup_signal_slot_connections();
}

InOut::~InOut()
{
}

// ---------------------------------------------------------------------
QSlider* InOut::get_slider_ptr() 
{
   return slider_ptr;
}

QLCDNumber* InOut::get_lcdnumber_ptr() 
{
   return lcdnumber_ptr;
}

QHBoxLayout* InOut::get_layout_ptr() 
{
   return Layout_ptr;
}

// ---------------------------------------------------------------------
void InOut::setup_layouts()
{
   Layout_ptr->addWidget(slider_ptr);
   Layout_ptr->addWidget(lcdnumber_ptr);
}

// ---------------------------------------------------------------------
void InOut::setup_signal_slot_connections()
{
   QObject::connect(get_slider_ptr(), SIGNAL(valueChanged(int)),
                    get_lcdnumber_ptr(), SLOT(display(int)));

}
