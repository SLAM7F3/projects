#include <QtCore>

bool operator<( const QPoint& p1, const QPoint& p2 )
{
    return ( p1.x() < p2.x() ||
             (p1.x() == p2.x() && p1.y() < p2.y() ) );
}

uint qHash( const QPoint& p )
{
    return qHash(p.x()) ^ qHash(p.y());
}

int main( int /*argc*/, char** /*argv*/ ) {

    QMap<QPoint, int> map;
    map.insert( QPoint( 1,2 ), 42 );

    QHash<QPoint, int > hash;
    hash.insert( QPoint( 1,1 ), 42 );

    QSet<QPoint> set;
    set.insert( QPoint( 1,1 ) );


    // Check the location
    QStringList strings;
    strings << "abc" << "def" << "hij";

    QMutableListIterator<QString> it(strings);
    it.toBack();
    qDebug() << "Just iterated over: " << it.previous();
    it.insert( "XXX" );
    qDebug() << "Next item after the insert is: " << it.previous();

    it = QMutableListIterator<QString>(strings);
    qDebug() << "Just iterated over: " << it.next();
    it.insert( "YYY" );
    qDebug() << "Next item after the insert is: " << it.next();

    it = QMutableListIterator<QString>(strings);

    while ( it.hasNext() )
        qDebug() << it.next();
}
