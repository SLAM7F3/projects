#include "tagreader.h"
#if defined Q_WS_WIN
#include "qt_windows.h"
#endif

int main( int argc, char** argv )
{
  for ( int i=1; i < argc; i++ ) {
    TagReader reader;
    reader.parse( argv[i] );
  }
}
