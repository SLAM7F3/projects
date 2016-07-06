// ==========================================================================
// Header file for Designer version of Editor class
// ==========================================================================
// Last modified on 7/24/07; 7/25/07
// ==========================================================================

#ifndef DESEDITOR_H
#define DESEDITOR_H

#include <iostream>
#include <string>
#include <vector>
#include <QtGui>

#include "ui_editor.h"

class Editor: public QMainWindow , private Ui_Editor
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
   void set_tool_tips();
   
  private: 

   QString filename;


   void allocate_member_objects();
   void initialize_member_objects();
//   void docopy(const Editor& p);

   private slots:

      void load_file();
      void save_file();
      void quit_app();
      void about();
      void aboutQt();
};

// ==========================================================================
// Inlined methods:
// ==========================================================================


#endif  // Editor.h



