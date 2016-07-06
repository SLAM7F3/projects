/********************************************************************************
** Form generated from reading ui file 'editor.ui'
**
** Created: Tue Jul 24 15:40:13 2007
**      by: Qt User Interface Compiler version 4.2.1
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_EDITOR_H
#define UI_EDITOR_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QTextEdit>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_Editor
{
public:
    QAction *actionLoad;
    QAction *actionSave;
    QAction *actionQuit;
    QAction *actionAbout;
    QAction *actionAboutQt;
    QWidget *centralwidget;
    QVBoxLayout *vboxLayout;
    QTextEdit *textEdit;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuHelp;
    QStatusBar *statusbar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *Editor)
    {
    Editor->setObjectName(QString::fromUtf8("Editor"));
    actionLoad = new QAction(Editor);
    actionLoad->setObjectName(QString::fromUtf8("actionLoad"));
    actionLoad->setIcon(QIcon(QString::fromUtf8("../../../../../../../../../../../usr/share/icons/Bluecurve/64x64/stock/gtk-open.png")));
    actionSave = new QAction(Editor);
    actionSave->setObjectName(QString::fromUtf8("actionSave"));
    actionSave->setIcon(QIcon(QString::fromUtf8("../../../../../../../../../../../usr/share/icons/Bluecurve/64x64/stock/gtk-save.png")));
    actionQuit = new QAction(Editor);
    actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
    actionAbout = new QAction(Editor);
    actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
    actionAboutQt = new QAction(Editor);
    actionAboutQt->setObjectName(QString::fromUtf8("actionAboutQt"));
    centralwidget = new QWidget(Editor);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    vboxLayout = new QVBoxLayout(centralwidget);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(9);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    textEdit = new QTextEdit(centralwidget);
    textEdit->setObjectName(QString::fromUtf8("textEdit"));

    vboxLayout->addWidget(textEdit);

    Editor->setCentralWidget(centralwidget);
    menubar = new QMenuBar(Editor);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menubar->setGeometry(QRect(0, 0, 800, 25));
    menuFile = new QMenu(menubar);
    menuFile->setObjectName(QString::fromUtf8("menuFile"));
    menuHelp = new QMenu(menubar);
    menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
    Editor->setMenuBar(menubar);
    statusbar = new QStatusBar(Editor);
    statusbar->setObjectName(QString::fromUtf8("statusbar"));
    Editor->setStatusBar(statusbar);
    toolBar = new QToolBar(Editor);
    toolBar->setObjectName(QString::fromUtf8("toolBar"));
    toolBar->setOrientation(Qt::Horizontal);
    Editor->addToolBar(static_cast<Qt::ToolBarArea>(4), toolBar);

    menubar->addAction(menuFile->menuAction());
    menubar->addAction(menuHelp->menuAction());
    menuFile->addAction(actionLoad);
    menuFile->addAction(actionSave);
    menuFile->addAction(actionQuit);
    menuHelp->addAction(actionAbout);
    menuHelp->addAction(actionAboutQt);
    toolBar->addAction(actionLoad);
    toolBar->addAction(actionSave);
    toolBar->addAction(actionQuit);

    retranslateUi(Editor);

    QSize size(800, 600);
    size = size.expandedTo(Editor->minimumSizeHint());
    Editor->resize(size);


    QMetaObject::connectSlotsByName(Editor);
    } // setupUi

    void retranslateUi(QMainWindow *Editor)
    {
    Editor->setWindowTitle(QApplication::translate("Editor", "MainWindow", 0, QApplication::UnicodeUTF8));
    actionLoad->setText(QApplication::translate("Editor", "Load", 0, QApplication::UnicodeUTF8));
    actionSave->setText(QApplication::translate("Editor", "Save", 0, QApplication::UnicodeUTF8));
    actionQuit->setText(QApplication::translate("Editor", "Quit", 0, QApplication::UnicodeUTF8));
    actionAbout->setText(QApplication::translate("Editor", "About", 0, QApplication::UnicodeUTF8));
    actionAboutQt->setText(QApplication::translate("Editor", "AboutQt", 0, QApplication::UnicodeUTF8));
    menuFile->setTitle(QApplication::translate("Editor", "File", 0, QApplication::UnicodeUTF8));
    menuHelp->setTitle(QApplication::translate("Editor", "Help", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Editor: public Ui_Editor {};
} // namespace Ui

#endif // UI_EDITOR_H
