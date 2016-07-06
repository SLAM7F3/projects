#include <QtGui>
#include <QDomDocument>

int main( int , char** )
{
  QDomDocument doc;
  doc.setContent( QString("<animals/>") );

  QDomElement animal  = doc.documentElement();

  QDomElement mammals = doc.createElement( "mammals" );
  animal.appendChild( mammals );

  QDomElement monkeys = doc.createElement( "monkeys" );
  monkeys.setAttribute( "size", "large" );

  mammals.appendChild( monkeys );
  monkeys.appendChild( doc.createElement("gorilla") );
  monkeys.appendChild( doc.createElement("orang-utan") );

  QDomElement human = doc.createElement("human");
  mammals.appendChild( human );

  QDomText text = doc.createTextNode("The most widespread mammal on earth");
  human.appendChild(text);

  animal.appendChild( doc.createElement( "birds" ) );

  QString str = doc.toString();
  qDebug() << str;
}
