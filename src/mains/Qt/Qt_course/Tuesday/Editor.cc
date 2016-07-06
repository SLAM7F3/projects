// ==========================================================================
// Editor class member function definitions
// ==========================================================================
// Last modified on 7/24/07
// ==========================================================================

#include "Editor.h"
#include <QtDebug>

using std::cout;
using std::endl;
using std::string;
using std::vector;

// ---------------------------------------------------------------------
// Initialization, constructor and destructor functions:
// ---------------------------------------------------------------------

void Editor::allocate_member_objects()
{
   textEdit_ptr=new QTextEdit;
//   textEdit_ptr=new QTextEdit("label goes here");
}		       

void Editor::initialize_member_objects()
{
   setCentralWidget(textEdit_ptr);

   setStatusBar(statusBar());
   setMenuBar(menuBar());

   QMenu* file_menu_ptr=menuBar()->addMenu("File");
   action_quit_ptr=menuBar()->addAction("Quit");
   QMenu* help_menu_ptr=menuBar()->addMenu("Help");

   action_load_ptr=file_menu_ptr->addAction("Load");
   action_save_ptr=file_menu_ptr->addAction("Save");

   action_about_ptr=help_menu_ptr->addAction("About");
   action_aboutqt_ptr=help_menu_ptr->addAction("About Qt");
}

Editor::Editor(QWidget* parent_ptr):
   QMainWindow(parent_ptr)
{
   allocate_member_objects();
   initialize_member_objects();

   setup_signal_slot_connections();
}

Editor::~Editor()
{
}

// ---------------------------------------------------------------------
void Editor::setup_signal_slot_connections()
{
   QObject::connect(action_load_ptr, SIGNAL(triggered()),
                    this, SLOT(load_file()));
   QObject::connect(action_save_ptr, SIGNAL(triggered()),
                    this, SLOT(save_file()));
   QObject::connect(action_quit_ptr, SIGNAL(triggered()),
                    this, SLOT(quit_app()));
}

// ---------------------------------------------------------------------
void Editor::load_file()
{
//   cout << "inside Editor::load_file()" << endl;

   filename=QFileDialog::getOpenFileName();
//   qDebug() << "filename = " << filename << endl;

   QFile input_file(filename);
   QString mystring;
   if (input_file.open(QIODevice::ReadOnly))
   {
      QTextStream stream(&input_file);
      mystring=stream.readAll();
      input_file.close();
   }

   textEdit_ptr->setText(mystring);
//   qDebug() << "mystring = " << mystring << endl;
}

// ---------------------------------------------------------------------
void Editor::save_file()
{
   cout << "inside Editor::save_file()" << endl;

   filename=QFileDialog::getSaveFileName();
   QFile output_file(filename);
   if (output_file.open(QIODevice::WriteOnly))
   {
      QTextStream outstream(&output_file);
      outstream << textEdit_ptr->toPlainText();
      output_file.close();
   }
}

// ---------------------------------------------------------------------
void Editor::quit_app()
{
//   cout << "inside Editor::quit_app()" << endl;

// qApp is a MACRO which returns a pointer to the main application

   qApp->quit();
}
