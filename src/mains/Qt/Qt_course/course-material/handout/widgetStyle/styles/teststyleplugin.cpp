#include <qstyleplugin.h>
#include "teststyle.h"

static const char styleName[] = "teststyle";

// Style Plugin Interface
class TestStylePlugin : public QStylePlugin
{
public:
    TestStylePlugin() {}
    ~TestStylePlugin() {}

    QStringList keys() const {
        return QStringList() << styleName;
    }

    QStyle* create( const QString& key ) {
        if ( key == styleName ) return new TestStyle();
        return 0;
    }
};

Q_EXPORT_PLUGIN( TestStylePlugin )
