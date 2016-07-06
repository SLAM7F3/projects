#include <QtTest/QtTest>
#include "myclass.h"

class Test: public QObject
{
    Q_OBJECT

private slots:
    void toUpper_data()
    {
        QTest::addColumn<QString>( "input" );
        QTest::addColumn<QString>( "result" );

        QTest::newRow( "all lower" ) << "hello" << "hello";
        QTest::newRow( "mixed" ) << "HellO" << "hello";
        QTest::newRow( "all upper" ) << "HELLO" << "hello";
        QTest::newRow( "empty input" ) << "" << "";
    }

    void toUpper()
    {
        QFETCH( QString, input );
        QFETCH( QString, result );
        QCOMPARE( input.toLower(), result );
    }

    void count_data()
    {
        QTest::addColumn<QString>( "data" );
        QTest::addColumn<int>( "result" );

        QTest::newRow( "empty string" ) << "" << 0;
        QTest::newRow( "normal string" ) << "hello" << 5;
        QTest::newRow( "expected error" ) << "hello" << 1;
        QTest::newRow( "uncaught error" ) << "hello" << 1;
    }

    void count()
    {
        QFETCH( QString, data );
        QFETCH( int, result );
        QEXPECT_FAIL( "expected error", "Errorneous test cases can be expected", Abort );
        QCOMPARE( data.count(), result );
    }

        void myclass_data()
    {
        QTest::addColumn<MyClass>( "input" );
        QTest::addColumn<int>( "result" );

        QTest::newRow( "possitive numbers" ) << MyClass( 20, 10 ) << 2;
        QTest::newRow( "zero" ) << MyClass( 0, 10 ) << 0;
    }

    void myclass()
    {
        QFETCH( MyClass, input );
        QFETCH( int, result );
        QCOMPARE( input.devide(), result );
    }

};

QTEST_MAIN(Test)

#include "main.moc"

