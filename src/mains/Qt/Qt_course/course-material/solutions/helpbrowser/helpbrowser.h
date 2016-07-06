#ifndef HELPBROWSER_H
#define HELPBROWSER_H

#include <QDialog>
#include <QMap>
class QTextBrowser;

class HelpBrowser :public QDialog
{
public:
  HelpBrowser( QWidget* parent );
  void setSource( const QString& name );
  void setHelpPath( const QString& path );
  void setHelp(QWidget* widget, QString page);

protected:
  virtual bool eventFilter( QObject* sender, QEvent* event );

private:
  QTextBrowser* _browser;
  QMap<QWidget*,QString> _helpMap;
};

#endif // HELPBROWSER_H
