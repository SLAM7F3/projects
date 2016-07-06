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

    void slotInsertTable();
    void slotInsertList();
    void slotInsertTextBox();

    void slotBoldface( bool );
    void slotItalic( bool );
    void slotUnderline( bool );
    void slotSearchReplace();

    void slotAbout();
    void slotAboutQt();

    void slotTextChanged();

protected:
    void closeEvent( QCloseEvent* );

private:
    QAction* _fileSaveAction;
    QAction* _filePrintAction;
    QAction* _boldface;
    QAction* _italic;
    QAction* _underline;
    QTextEdit* _edit;
    QLabel* _fileNameLabel;
    QString _fileName;
};

#endif // EDITOR_H
