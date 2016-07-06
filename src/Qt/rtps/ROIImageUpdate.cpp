#include "ROIImageUpdate.h"

QDataStream& operator << (QDataStream& s, const sROIImageUpdate& x) {
	s << x.regionID;
	s << x.imageUpdatePriority;
	s << x.utmCorners;
	s << x.imageSize;
	s.writeBytes(x.imageData.str().c_str(),x.imageSize);
	return s;
}
QDataStream& operator >> (QDataStream& s, sROIImageUpdate& x) {
	std::vector<char> vect_char;
	s >> x.regionID;
	s >> x.imageUpdatePriority;
	s >> x.utmCorners;
	s >> x.imageSize;

	vect_char.resize(x.imageSize);
	s.readRawData(reinterpret_cast<char*>(&vect_char[0]),x.imageSize);
	x.imageData.clear(); 
	x.imageData.str("");
	x.imageData.write(&vect_char[0], x.imageSize);
	return s;
}
stringstream& operator << (stringstream& s, const sROIImageUpdate& x) {
	s.write(reinterpret_cast<const char*>(&(x.regionID)), sizeof(uint32_t));
	s.write(reinterpret_cast<const char*>(&(x.imageUpdatePriority)), sizeof(uint32_t));
	s << x.utmCorners;
	s.write(reinterpret_cast<const char*>(&(x.imageSize)), sizeof(uint32_t));
	s.write(x.imageData.str().c_str(), x.imageSize); 
	return s;
}
stringstream& operator >> (stringstream& s, sROIImageUpdate& x) {
	std::vector<char> vect_char;
	s.read(reinterpret_cast<char*>(&(x.regionID)), sizeof(uint32_t));    
	s.read(reinterpret_cast<char*>(&(x.imageUpdatePriority)), sizeof(uint32_t));    
	s >> x.utmCorners;
	s.read(reinterpret_cast<char*>(&(x.imageSize)), sizeof(uint32_t));    

	vect_char.resize(x.imageSize);
	s.read(reinterpret_cast<char*>(&vect_char[0]), x.imageSize);
	x.imageData.clear(); 
	x.imageData.str("");
	x.imageData.write(&vect_char[0], x.imageSize);
	return s;
}

ostream& operator<< (ostream& s, const sROIImageUpdate& x) {
	s << "Region ID   : " << x.regionID << "\n";
	s << "Img Upd Pri : " << x.imageUpdatePriority << "\n";
	s << "UTM Corners :\n" << x.utmCorners << "\n";
	s << "Im Size     : " <<  x.imageSize << "\n";	
	s << "Im Data     :\n" << x.imageData.str().c_str() << "\n";
	return s;
}
