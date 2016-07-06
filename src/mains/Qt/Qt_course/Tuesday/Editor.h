// ==========================================================================
// Header file for Editor class
// ==========================================================================
// Last modified on 7/24/07
// ==========================================================================

#ifndef EDITOR_H
#define EDITOR_H

#include <iostream>
#include <string>
#include <vector>
#include <QtGui>

class Editor: public QMainWindow
{

   Q_OBJECT

  public:

// Initialization, constructor and destructor functions:

   Editor(QWidget* parent_ptr=NULL);

// On 11/13/01, we learned from Tara Dennis that base class
// destructors ought to always be declared as virtual so that they
// will automatically be called by inherited class destructors:

   virtual ~Editor();

// Set and get member functions:

   void setup_signal_slot_connections();
   
  private: 

   QString filename;

   QTextEdit* textEdit_ptr;
   QAction *action_load_ptr,*action_save_ptr,*action_quit_ptr,
      *action_about_ptr,*action_aboutqt_ptr;

   void allocate_member_objects();
   void initialize_member_objects();
//   void docopy(const Editor& p);

   private slots:

      void load_file();
      void save_file();
      void quit_app();
   

};

// ==========================================================================
// Inlined methods:
// ==========================================================================


#endif  // Editor.h



