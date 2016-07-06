#include <SimpleImagePlugin.h>

#include <SimpleImageIOHandler.h>

SimpleImagePlugin::SimpleImagePlugin(QObject* parent) :
	QImageIOPlugin(parent)
{

}

SimpleImagePlugin::~SimpleImagePlugin()
{

}

QImageIOPlugin::Capabilities 
SimpleImagePlugin::capabilities(QIODevice * device, const QByteArray & format) const
{
	// First operate on the format
	if (format == "sif")
        return Capabilities(CanRead | CanWrite);
    if (!format.isEmpty())
        return 0;


	// ...but we might just have the device....
    if (!device->isOpen())
        return 0;

    Capabilities cap;
    if (device->isReadable() && SimpleImageIOHandler::canRead(device))
        cap |= CanRead;
    if (device->isWritable())
        cap |= CanWrite;
    return cap;

}

QImageIOHandler* 
SimpleImagePlugin::create(QIODevice * device, const QByteArray & format) const
{
	QImageIOHandler *handler = new SimpleImageIOHandler(device);

    handler->setFormat(format);
    return handler;
}

QStringList 
SimpleImagePlugin::keys() const
{
	QStringList list;
	list << "sif";
	return list;
}

Q_EXPORT_PLUGIN(SimpleImagePlugin)