#ifndef SIMPLEIMAGEPLUGIN_H
#define SIMPLEIMAGEPLUGIN_H

#include <QImageIOPlugin>

class SimpleImagePlugin : public QImageIOPlugin
{
public:
	SimpleImagePlugin(QObject* parent=0);
	~SimpleImagePlugin();

	virtual Capabilities capabilities(QIODevice * device, const QByteArray & format) const;
	virtual QImageIOHandler* create(QIODevice * device, const QByteArray & format = QByteArray()) const;
	virtual QStringList keys() const;

};

#endif

