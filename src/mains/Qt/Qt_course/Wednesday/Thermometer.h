// ==========================================================================
// Header file for Thermometer class
// ==========================================================================
// Last modified on 7/25/07
// ==========================================================================

#ifndef THERMOMETER_H
#define THERMOMETER_H

#include <iostream>
#include <string>
#include <vector>
#include <QtGui>
#include "ThermoDisplay.h"

class Thermometer: public QWidget
{

   Q_OBJECT

  public:

// Initialization, constructor and destructor functions:

   Thermometer(QWidget* parent_ptr=NULL);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~Thermometer();

// Set and get member functions:

   virtual QSize minimumSizeHint();
   virtual QSize sizeHint();
   void setup_layout();
   void setup_signal_slot_connections();

//   void mousePressEvent(QMouseEvent* );

  private: 

   int temperature;
   ThermoDisplay* ThermoDisplay_ptr;
   QSlider* slider_ptr;
   QHBoxLayout* Layout_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
//   void docopy(const Thermometer& p);

   private slots:

      void set_temperature(int temp);
   
};

// ==========================================================================
// Inlined methods:
// ==========================================================================

inline void ThermoDisplay::set_temperature(int temp)
{
   temperature=temp;
}

#endif  // Thermometer.h



