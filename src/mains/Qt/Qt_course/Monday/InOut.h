// ==========================================================================
// Header file for InOut class
// ==========================================================================
// Last modified on 7/23/07
// ==========================================================================

#ifndef INOUT_H
#define INOUT_H

#include <iostream>
#include <string>
#include <vector>
#include <QtGui>

class InOut: public QWidget
{

   Q_OBJECT

  public:

// Initialization, constructor and destructor functions:

   InOut(QWidget* parent_ptr=NULL);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~InOut();

// Set and get member functions:

   QSlider* get_slider_ptr();
   QLCDNumber* get_lcdnumber_ptr();
   QHBoxLayout* get_layout_ptr();

   void setup_layouts();
   void setup_signal_slot_connections();
   
  private: 

   QSlider* slider_ptr;
   QLCDNumber* lcdnumber_ptr;
   QHBoxLayout* Layout_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
//   void docopy(const InOut& p);

};

// ==========================================================================
// Inlined methods:
// ==========================================================================


#endif  // InOut.h



