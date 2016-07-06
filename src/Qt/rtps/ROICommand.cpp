#include "ROICommand.h"

// write to stream
QDataStream& operator << (QDataStream& s, const sROICommand& x) {
	s << x.regionID;
	s << x.enable;
	s << x.utmCoord;
	s << x.trackID;
	s << x.trackUpdatePriority;
	s << x.imageUpdatePriority;
	s << x.imagePeriod;
	s << x.jpegQuality;
	s << x.numChips;
	return s;
}
// read from stream
QDataStream& operator >> (QDataStream& s, sROICommand& x) {
	s >> x.regionID;
	s >> x.enable;
	s >> x.utmCoord;
	s >> x.trackID;
	s >> x.trackUpdatePriority;
	s >> x.imageUpdatePriority;
	s >> x.imagePeriod;
	s >> x.jpegQuality;
	s >> x.numChips;
	return s;
}
// write to string stream
stringstream& operator << (stringstream& s, const sROICommand& x) {
	s.write(reinterpret_cast<const char*>(&(x.regionID)), sizeof(uint32_t));
	s.write(reinterpret_cast<const char*>(&(x.enable)), sizeof(bool));
	s << x.utmCoord;
	s.write(reinterpret_cast<const char*>(&(x.trackID)), sizeof(uint32_t));
	s.write(reinterpret_cast<const char*>(&(x.trackUpdatePriority)), sizeof(uint32_t));
	s.write(reinterpret_cast<const char*>(&(x.imageUpdatePriority)), sizeof(uint32_t));
	s.write(reinterpret_cast<const char*>(&(x.imagePeriod)), sizeof(float));
	s.write(reinterpret_cast<const char*>(&(x.jpegQuality)), sizeof(uint32_t));
	return s;
}
// read from string stream
stringstream& operator >> (stringstream& s, sROICommand& x) {
	s.read(reinterpret_cast<char*>(&(x.regionID)), sizeof(uint32_t));
	s.read(reinterpret_cast<char*>(&(x.enable)), sizeof(bool));
	s >> x.utmCoord;
	s.read(reinterpret_cast<char*>(&(x.trackID)), sizeof(uint32_t));
	s.read(reinterpret_cast<char*>(&(x.trackUpdatePriority)), sizeof(uint32_t));
	s.read(reinterpret_cast<char*>(&(x.imageUpdatePriority)), sizeof(uint32_t));
	s.read(reinterpret_cast<char*>(&(x.imagePeriod)), sizeof(float));
	s.read(reinterpret_cast<char*>(&(x.jpegQuality)), sizeof(uint32_t));
	return s;
}

ostream& operator<< (ostream& s, const sROICommand& x) {
	s << "Region ID  : " << x.regionID << "\n";
	s << "Enable     : " << x.enable << "\n";
	s << "UTM Coord  : " << x.utmCoord << "\n";
	s << "Track ID   : " << x.trackID << "\n";
	s << "Trk Upd Pri: " << x.trackUpdatePriority << "\n";
	s << "Img Upd Pri: " << x.imageUpdatePriority << "\n";
	s << "Img Period : " << x.imagePeriod << "\n";
	s << "Jpeg Qual  : " << x.jpegQuality << "\n";
	return s;
}
