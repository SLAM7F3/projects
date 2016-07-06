#include <QtTest/QtTest>

class TestQString: public QObject
{
    Q_OBJECT

private slots:
    void toLower()
    {
        QString str = "Hello";
        QVERIFY( str.toLower() == "hello" );
    }

    void toUpper()
    {
        QString str = "Hello";
        QCOMPARE(str.toUpper(), QString("HELLO"));
    }

    void failure()
    {
        QVERIFY( 1+1 == 3 );
    }

    void expectedFailure()
    {
        QEXPECT_FAIL( "", "The Universe is still sane", Continue );
        QVERIFY( 42 == 43 );

        QEXPECT_FAIL( "", "Just to show that it will fail if it doesn't", Continue );
        QVERIFY( 42 == 42 );
    }
};

QTEST_MAIN(TestQString)

#include "main.moc"

