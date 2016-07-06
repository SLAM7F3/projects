#ifndef TESTCASEHANDLER_H
#define TESTCASEHANDLER_H
#include <QXmlDefaultHandler>

class TestTool;

/**
 * Simple SAX handler to read test-case.xml files. No error handling is done.
 */
class TestCaseHandler : public QXmlDefaultHandler {
public:
  TestCaseHandler( TestTool* testTool ) : _testTool( testTool ) {}

  bool startElement( const QString&, const QString&, const QString& tag, const QXmlAttributes& atts);
  bool endElement( const QString&, const QString&, const QString& tag);
  bool characters( const QString& ch );
private:
  TestTool* _testTool;

  QString _name;
  QString _objective;
  QString _input;
  QString _output;

  QString _buf;

  // Dont offer copy c'tor and assignment operator:
  TestCaseHandler( const TestCaseHandler & );
  TestCaseHandler &operator=( const TestCaseHandler& );
};


#endif // TESTCASEHANDLER_H
