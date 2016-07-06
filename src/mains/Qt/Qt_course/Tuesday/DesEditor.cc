// ==========================================================================
// Editor class member function definitions
// ==========================================================================
// Last modified on 7/24/07; 7/25/07
// ==========================================================================

#include "DesEditor.h"
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
}		       

void Editor::initialize_member_objects()
{
}

Editor::Editor(QWidget* parent_ptr):
   QMainWindow(parent_ptr)
{
   setupUi(this);
   setup_signal_slot_connections();
   set_tool_tips();
}

Editor::~Editor()
{
}

// ---------------------------------------------------------------------
void Editor::setup_signal_slot_connections()
{
   QObject::connect(actionLoad, SIGNAL(triggered()),
                    this, SLOT(load_file()));
   QObject::connect(actionSave, SIGNAL(triggered()),
                    this, SLOT(save_file()));
   QObject::connect(actionQuit, SIGNAL(triggered()),
                    this, SLOT(quit_app()));
   QObject::connect(actionAbout, SIGNAL(triggered()),
                    this, SLOT(about()));
   QObject::connect(actionAboutQt, SIGNAL(triggered()),
                    this, SLOT(aboutQt()));
}

// ---------------------------------------------------------------------
void Editor::set_tool_tips()
{
   actionLoad->setToolTip("Load a new file");
   actionSave->setToolTip("Save results into a file");
   actionQuit->setToolTip("Quit this program");
}

// ---------------------------------------------------------------------
void Editor::load_file()
{
   cout << "inside Editor::load_file()" << endl;

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

   textEdit->setText(mystring);
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
      outstream << textEdit->toPlainText();
      output_file.close();
   }
}

// ---------------------------------------------------------------------
void Editor::quit_app()
{
   cout << "inside Editor::quit_app()" << endl;

// qApp is a MACRO which returns a pointer to the main application

   qApp->quit();
}

// ---------------------------------------------------------------------
void Editor::about()
{
//   cout << "inside Editor::about()" << endl;

// qApp is a MACRO which returns a pointer to the main application

   QString title("Editor program");
   QString text_message("Written on 4/24/07");

//   QMessageBox::information(this,title,text_message);
   QMessageBox::about(this,title,text_message);
}

// ---------------------------------------------------------------------
void Editor::aboutQt()
{
//   cout << "inside Editor::aboutQt()" << endl;

// qApp is a MACRO which returns a pointer to the main application

   QMessageBox::aboutQt(this);
}
