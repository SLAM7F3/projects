// ==========================================================================
// ColorSelector class member function definitions
// ==========================================================================
// Last modified on 7/23/07
// ==========================================================================

#include "ColorSelector.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ColorSelector::allocate_member_objects()
{
   descriptor_label_ptr=new QLabel("RGB color values:");
   red_label_ptr=new QLabel("0");
   green_label_ptr=new QLabel("0");
   blue_label_ptr=new QLabel("0");
   pushbutton_ptr=new QPushButton("Select Color");
   Layout_ptr=new QVBoxLayout;
}		       

void ColorSelector::initialize_member_objects()
{
}

ColorSelector::ColorSelector(QWidget* parent_ptr):
   QWidget(parent_ptr)
{
   allocate_member_objects();
   initialize_member_objects();

   setup_layouts();
   setup_signal_slot_connections();
}

ColorSelector::~ColorSelector()
{
}

// ---------------------------------------------------------------------
QVBoxLayout* ColorSelector::get_layout_ptr() 
{
   return Layout_ptr;
}

// ---------------------------------------------------------------------
void ColorSelector::setup_layouts()
{
   QVBoxLayout* color_layout_ptr=new QVBoxLayout();
   color_layout_ptr->addWidget(descriptor_label_ptr);
   color_layout_ptr->addWidget(red_label_ptr);
   color_layout_ptr->addWidget(green_label_ptr);
   color_layout_ptr->addWidget(blue_label_ptr);

   Layout_ptr->addLayout(color_layout_ptr);
   Layout_ptr->addWidget(pushbutton_ptr);
}

// ---------------------------------------------------------------------
void ColorSelector::setup_signal_slot_connections()
{
   QObject::connect(pushbutton_ptr, SIGNAL(clicked(bool)),
                    this, SLOT(get_color()));
   QObject::connect(pushbutton_ptr, SIGNAL(clicked(bool)),
                    this, SLOT(set_color_label()));

}

// ---------------------------------------------------------------------
void ColorSelector::get_color()
{
   cout << "inside ColorSelector::get_color()" << endl;
   picked_color=QColorDialog::getColor();

   cout << "picked_color.red() = " << picked_color.red() << endl;
   cout << "picked_color.green() = " << picked_color.green() << endl;
   cout << "picked_color.blue() = " << picked_color.blue() << endl;
}

// ---------------------------------------------------------------------
void ColorSelector::set_color_label()
{
//   cout << "inside ColorSelector::set_color_label()" << endl;
   red_label_ptr->setNum(picked_color.red());
   green_label_ptr->setNum(picked_color.green());
   blue_label_ptr->setNum(picked_color.blue());
}
