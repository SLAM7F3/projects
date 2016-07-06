#include "plugin.h"
#include <compass.h>
#include <compass2.h>

/* XPM */
static const char * icon1[] = {
"22 22 2 1",
"       c white",
".      c blue",
"       .......        ",
"     ..       ..      ",
"   ..   .  .    ..    ",
"  .     .. .      .   ",
"  .     . ..     ..   ",
" .      .  .    .  .  ",
" .             .   .  ",
".            ..     . ",
".           ..      . ",
". . . .    ..   ... . ",
". . . .  ....   .   . ",
". . . .  ..     ..  . ",
". . . .         .   . ",
".  . .          ... . ",
" .        ..       .  ",
" .       .  .      .  ",
"  .       .       .   ",
"  .        .      .   ",
"   ..    .  .   ..    ",
"     ..   ..  ..      ",
"       .......        ",
"                      "};

static const char * icon2[] = {
"22 22 2 1",
"       c white",
".      c red",
"       .......        ",
"     ..       ..      ",
"   ..   .  .    ..    ",
"  .     .. .      .   ",
"  .     . ..     ..   ",
" .      .  .    .  .  ",
" .             .   .  ",
".            ..     . ",
".           ..      . ",
". . . .    ..   ... . ",
". . . .  ....   .   . ",
". . . .  ..     ..  . ",
". . . .         .   . ",
".  . .          ... . ",
" .        ..       .  ",
" .       .  .      .  ",
"  .       .       .   ",
"  .        .      .   ",
"   ..    .  .   ..    ",
"     ..   ..  ..      ",
"       .......        ",
"                      "};

CompassPlugin::CompassPlugin()
{
}

QStringList CompassPlugin::keys() const
{
    QStringList list;
    list << "CompassWidget" << "CompassWidget2";
    return list;
}

QWidget* CompassPlugin::create( const QString &feature, QWidget* parent, const char* name )
{
  if ( feature == "CompassWidget" )
    return new CompassWidget( parent, name );
  if ( feature == "CompassWidget2" )
    return new CompassWidget2( parent, name );
  else
    return 0;
}

QString CompassPlugin::group( const QString&  ) const
{
  return "Input";
}

QIconSet CompassPlugin::iconSet( const QString& feature ) const
{
  if ( feature == "CompassWidget" )
    return QIconSet( QPixmap( icon1 ) );
  if ( feature == "CompassWidget2" )
    return QIconSet( QPixmap( icon2 ) );
  else
    return QIconSet();
}

QString CompassPlugin::includeFile( const QString& feature ) const
{
  if ( feature == "CompassWidget" )
    return "compass.h";
  if ( feature == "CompassWidget2" )
    return "compass2.h";
  else
    return QString::null;
}

QString CompassPlugin::toolTip( const QString& feature ) const
{
  if ( feature == "CompassWidget" )
    return tr("Compass Widget");
  else if ( feature == "CompassWidget2" )
    return tr("Another Compass Widget");
  else
    return QString::null;
}

QString CompassPlugin::whatsThis( const QString& feature ) const
{
  if ( feature == "CompassWidget" )
    return tr("A Compass Widget, what can I say?");
  else if ( feature == "CompassWidget2" )
    return tr("An other Compass Widget, what can I say?");
  else
    return QString::null;
}

bool CompassPlugin::isContainer( const QString& feature ) const
{
  if ( feature == "CompassWidget" )
    return false;
  if ( feature == "CompassWidget2" )
    return false;
  else
    return false;
}


Q_EXPORT_PLUGIN( CompassPlugin )
