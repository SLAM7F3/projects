#include "NFOVCommand.h"

QDataStream& operator << (QDataStream& s, const sNFOVCommand& x) {
	s << x.currentNFOVCommandID;
	s << x.enable;
	s << x.utmCoord;
	s << x.imageUpdatePriority;
	s << x.imagePeriod;
	s << x.jpegQuality;
	return s;
}
QDataStream& operator >> (QDataStream& s, sNFOVCommand& x) {
	s >> x.currentNFOVCommandID;
	s >> x.enable;
	s >> x.utmCoord;
	s >> x.imageUpdatePriority;
	s >> x.imagePeriod;
	s >> x.jpegQuality;
	return s;
}
stringstream& operator << (stringstream& s, const sNFOVCommand& x) {
	s.write(reinterpret_cast<const char*>(&(x.currentNFOVCommandID)), sizeof(uint32_t));
	s.write(reinterpret_cast<const char*>(&(x.enable)), sizeof(bool));
	s << x.utmCoord;
	s.write(reinterpret_cast<const char*>(&(x.imageUpdatePriority)), sizeof(uint32_t));
	s.write(reinterpret_cast<const char*>(&(x.imagePeriod)), sizeof(float));
	s.write(reinterpret_cast<const char*>(&(x.jpegQuality)), sizeof(uint32_t));
	return s;
}
stringstream& operator >> (stringstream& s, sNFOVCommand& x) {
	s.read(reinterpret_cast<char*>(&(x.currentNFOVCommandID)), sizeof(uint32_t));
	s.read(reinterpret_cast<char*>(&(x.enable)), sizeof(bool));
	s >> x.utmCoord;
	s.read(reinterpret_cast<char*>(&(x.imageUpdatePriority)), sizeof(uint32_t));
	s.read(reinterpret_cast<char*>(&(x.imagePeriod)), sizeof(float));
	s.read(reinterpret_cast<char*>(&(x.jpegQuality)), sizeof(uint32_t));
	return s;
}

ostream& operator<< (ostream& s, const sNFOVCommand& x) {
	s << "CMD ID    : " << x.currentNFOVCommandID << "\n";
	s << "Enable    : " << x.enable << "\n";
	s << "UTM Coord : " << x.utmCoord << "\n";
	s << "Im Upd Pri: " << x.imageUpdatePriority << "\n";
	s << "Im Period : " << x.imagePeriod << "\n";
	s << "Jpeg Qual : " << x.jpegQuality << "\n";
	return s;
}
