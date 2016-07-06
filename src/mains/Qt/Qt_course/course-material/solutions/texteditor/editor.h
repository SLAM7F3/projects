#ifndef EDITOR_H
#define EDITOR_H

#include <QMainWindow>
#include <QTextEdit>
class QAction;
class QLabel;

class Editor : public QMainWindow {
    Q_OBJECT
public:
    Editor( QWidget* parent = 0 );

protected slots:
    void slotFileOpen();
    void slotFileSave();
    void slotFilePrint();
    void slotQuit();

    void slotAbout();
    void slotAboutQt();

    void slotTextChanged();

protected:
    void closeEvent( QCloseEvent* );

private:
    QAction*        _fileSaveAction;
    QAction*        _filePrintAction;
    QTextEdit*      _edit;
    QLabel*         _fileNameLabel;

    QString         _fileName;
};

#endif // EDITOR_H
