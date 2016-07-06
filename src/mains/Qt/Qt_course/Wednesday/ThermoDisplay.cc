// ==========================================================================
// ThermoDisplay class member function definitions
// ==========================================================================
// Last modified on 7/25/07
// ==========================================================================

#include "ThermoDisplay.h"
#include <QtDebug>

using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void ThermoDisplay::allocate_member_objects()
{
}		       

void ThermoDisplay::initialize_member_objects()
{
//   resize(200,400);
}

ThermoDisplay::ThermoDisplay(QWidget* parent_ptr):
   QWidget(parent_ptr)
{
   allocate_member_objects();
   initialize_member_objects();
   setMinimumSize(200,400);
}

ThermoDisplay::~ThermoDisplay()
{
}

/*
// ---------------------------------------------------------------------
QSize ThermoDisplay::minimumSizeHint()
{
   cout << "inside ThermoDisplay::minimumSizeHint()" << endl;
   return QSize(100,400);	// width, height
}

// ---------------------------------------------------------------------
QSize ThermoDisplay::sizeHint()
{
   cout << "inside ThermoDisplay::SizeHint()" << endl;
   return QSize(100,400);	// width, height
}
*/

// ---------------------------------------------------------------------
// Member function paintEvent overrides default virtual paintEvent
// method of QWidget.  It is called repeatedly in the infinite Event
// loop...

void ThermoDisplay::paintEvent(QPaintEvent* )
{
//   cout << "inside ThermoDisplay::paintEvent, temperature = "
//        << temperature << endl;
   
   QPainter paint(this);	
   paint.setPen(Qt::NoPen);
   paint.setBrush(QBrush(Qt::red));

   const int max_temp=100;
   const int min_temp=0;
   int curr_y=(temperature-min_temp)*(height()-0)/(max_temp-min_temp);

   int startx=0;
   int starty=height()-curr_y;
   int stopx=width();
   int stopy=height();

   paint.drawRect(
      startx,
      starty,
      stopx,
      curr_y);
}
