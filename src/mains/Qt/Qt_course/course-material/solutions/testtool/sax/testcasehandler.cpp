#include <QtGui>
#include "testcasehandler.h"
#include "testtool.h"

bool TestCaseHandler::startElement( const QString&, const QString&, const QString& tag,
                                    const QXmlAttributes& atts )
{
    // Every start-tag resets the buffer
    _buf = "";
    // Here we read the "name" attribute in the "test-case" element
    if( tag == "test-case" ) {
        _name = atts.value( "name" );
    }
    return true;
}

bool TestCaseHandler::endElement( const QString&, const QString&, const QString& tag )
{
    if( tag == "test-case" ) {
        // A test-case struct is finished, add it to the GUI:
        _testTool->addPage( _name, _objective, _input, _output );
        // Clear data:
        _name = _objective = _input = _output = "";
    } else if( tag == "input" ) {
        _input = _buf.trimmed();
    } else if( tag == "output" ) {
        _output = _buf.trimmed();
    } else if( tag == "objective" ) {
        _objective = _buf.trimmed();
    } else if( tag == "test-specification" ) {
        qDebug( "Parsing done" );
    } else {
        qWarning() << "Unknown tag: \"" << tag << "\"";
        return false;
    }
    return true;
}

bool TestCaseHandler::characters( const QString& ch )
{
    _buf += ch;
    return true;
}
