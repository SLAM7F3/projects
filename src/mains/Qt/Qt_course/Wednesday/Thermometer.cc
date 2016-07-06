// ==========================================================================
// Thermometer class member function definitions
// ==========================================================================
// Last modified on 7/25/07
// ==========================================================================

#include "Thermometer.h"
#include <QtDebug>

using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Thermometer::allocate_member_objects()
{

// Recall parent for ThermoDisplay and slider widgets is *this ( =
// Thermometer widget):

   ThermoDisplay_ptr=new ThermoDisplay(this);
   slider_ptr=new QSlider(Qt::Vertical,this);
   Layout_ptr=new QHBoxLayout;
}		       

void Thermometer::initialize_member_objects()
{
   const int init_temperature=35;
   set_temperature(init_temperature);
   slider_ptr->setValue(init_temperature);

   slider_ptr->setRange(0,100);
   slider_ptr->setTickInterval(10);
}

Thermometer::Thermometer(QWidget* parent_ptr):
   QWidget(parent_ptr)
{
   allocate_member_objects();
   initialize_member_objects();

   setup_layout();
   setup_signal_slot_connections();
}

Thermometer::~Thermometer()
{
}

// ---------------------------------------------------------------------
QSize Thermometer::minimumSizeHint()
{
   cout << "inside Thermometer::minimumSizeHint()" << endl;

   return QSize(400,1000);	// width, height
}

// ---------------------------------------------------------------------
QSize Thermometer::sizeHint()
{
   cout << "inside Thermometer::SizeHint()" << endl;

   return QSize(400,1000);	// width, height
}

// ---------------------------------------------------------------------
void Thermometer::setup_layout()
{
   Layout_ptr->addWidget(ThermoDisplay_ptr);
   Layout_ptr->addWidget(slider_ptr);
   setLayout(Layout_ptr);
//   setSizePolicy(QSizePolicy());
}

// ---------------------------------------------------------------------
void Thermometer::setup_signal_slot_connections()
{
   QObject::connect(slider_ptr, SIGNAL(valueChanged(int)),
                    this, SLOT(set_temperature(int)));
}

// ---------------------------------------------------------------------
void Thermometer::set_temperature(int temp)
{
//   cout << "inside Thermometer::set_temp, temp = " << temp << endl;
   temperature=temp;
   ThermoDisplay_ptr->set_temperature(temperature);
   ThermoDisplay_ptr->update();
}
