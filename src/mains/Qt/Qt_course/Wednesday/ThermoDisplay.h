// ==========================================================================
// Header file for ThermoDisplay class
// ==========================================================================
// Last modified on 7/25/07
// ==========================================================================

#ifndef THERMODISPLAY_H
#define THERMODISPLAY_H

#include <iostream>
#include <string>
#include <vector>
#include <QtGui>

class ThermoDisplay: public QWidget
{

   Q_OBJECT

  public:

// Initialization, constructor and destructor functions:

   ThermoDisplay(QWidget* parent_ptr=NULL);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~ThermoDisplay();

// Set and get member functions:

   void set_temperature(int temp);

//   virtual QSize minimumSizeHint();
//   virtual QSize sizeHint();
   void paintEvent(QPaintEvent* );

  private: 

   int temperature;

   void allocate_member_objects();
   void initialize_member_objects();
//   void docopy(const ThermoDisplay& p);

//   private slots:

   
};

// ==========================================================================
// Inlined methods:
// ==========================================================================


#endif  // ThermoDisplay.h



