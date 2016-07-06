#ifndef TEST_WIDGET_H
#define TEST_WIDGET_H

#include <QWidget>
class HelpBrowser;

class TestWidget :public QWidget
{
public:
    TestWidget( QWidget* parent );

private:
    HelpBrowser* _helpBrowser;
};

#endif // TEST_WIDGET_H
