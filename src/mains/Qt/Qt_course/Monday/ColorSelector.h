// ==========================================================================
// Header file for ColorSelector class
// ==========================================================================
// Last modified on 7/23/07
// ==========================================================================

#ifndef MYCLASS_H
#define MYCLASS_H

#include <iostream>
#include <string>
#include <vector>
#include <QtGui>

class ColorSelector: public QWidget
{

   Q_OBJECT

  public:

// Initialization, constructor and destructor functions:

   ColorSelector(QWidget* parent_ptr=NULL);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~ColorSelector();

// Set and get member functions:

   QVBoxLayout* get_layout_ptr();

   void setup_layouts();
   void setup_signal_slot_connections();
   
  private: 

   QLabel* descriptor_label_ptr;
   QLabel *red_label_ptr,*green_label_ptr,*blue_label_ptr;
   QPushButton* pushbutton_ptr;
   QVBoxLayout* Layout_ptr;
   
   QColor picked_color;

   void allocate_member_objects();
   void initialize_member_objects();
//   void docopy(const ColorSelector& p);

   private slots:

   void get_color();
   void set_color_label();

};

// ==========================================================================
// Inlined methods:
// ==========================================================================


#endif  // ColorSelector.h



