#include <QtGui>
#include <QtTest/QtTest>

class TestGui: public QObject
{
    Q_OBJECT

private slots:
    void simpleTest()
    {
        QString testString = "hello world";
        QLineEdit lineEdit;
        QTest::keyClicks(&lineEdit, testString );
        QCOMPARE(lineEdit.text(), testString );
    }

    void testGuiDataDriven_data()
    {
        QTest::addColumn<QTestEventList>("events");
        QTest::addColumn<QString>("expected");

        QTestEventList list1;
        list1.addKeyClick('a');
        QTest::newRow("char") << list1 << "a";

        QTestEventList list2;
        list2.addKeyClick('a');
        list2.addKeyClick(Qt::Key_Backspace);
        QTest::newRow("there and back again") << list2 << "";
    }

    void testGuiDataDriven()
    {
        QFETCH(QTestEventList, events);
        QFETCH(QString, expected);

        QLineEdit lineEdit;
        events.simulate(&lineEdit);
        QCOMPARE(lineEdit.text(), expected);
    }
};


QTEST_MAIN(TestGui)
#include "main.moc"

