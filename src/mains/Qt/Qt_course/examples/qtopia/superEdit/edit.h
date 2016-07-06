#ifndef EDIT_H
#define EDIT_H
#include <QMainWindow>
#include <QContent>
class QDocumentSelector;
class QTextEdit;
class QStackedWidget;

class Edit :public QMainWindow
{
Q_OBJECT

public:
  Edit(QWidget* parent = 0, Qt::WFlags f = 0);

protected slots:
  void slotLoad();
  void slotNew();
  void slotLoad( const QContent& lnk );
  void setDocument(const QString&);

protected:
  void closeEvent ( QCloseEvent * e );
  void save();

private:
  QStackedWidget* _stack;
  QTextEdit* _edit;
  QDocumentSelector* _selector;
  QContent _doc;
};



#endif /* EDIT_H */

