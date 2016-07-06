#include <QtGui>
#include <QtTest/QtTest>

class TestGui: public QObject
{
    Q_OBJECT

private slots:
    void signalEmission()
    {
        QLineEdit lineEdit;
        QSignalSpy spy( &lineEdit, SIGNAL( textChanged ( const QString & ) ) );
        QString text( "hello world" );
        QTest::keyClicks(&lineEdit, text);
        QCOMPARE( spy.count(), text.length() );
        for ( int i = 0; i < text.length(); ++i ) {
            QCOMPARE( spy[i][0].toString(), text.left(i+1) );
        }
    }
};


QTEST_MAIN(TestGui)
#include "main.moc"

