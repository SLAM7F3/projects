#include "WFOVImageUpdate.h"

QDataStream& operator << (QDataStream& s, const sWFOVImageUpdate& x){
	s << x.utmCorners;
	s << x.imageSize;
	s.writeBytes(x.imageData.str().c_str(),x.imageSize);
	return s;
}
QDataStream& operator >> (QDataStream& s, sWFOVImageUpdate& x){
	std::vector<char> vect_char;
	s >> x.utmCorners;
	s >> x.imageSize;

	vect_char.resize(x.imageSize);
	s.readRawData(reinterpret_cast<char*>(&vect_char[0]),x.imageSize);
	x.imageData.clear(); 
	x.imageData.str("");
	x.imageData.write(&vect_char[0], x.imageSize);
	return s;
}
stringstream& operator << (stringstream& s, const sWFOVImageUpdate& x){
	s << x.utmCorners;
	s.write(reinterpret_cast<const char*>(&(x.imageSize)), sizeof(uint32_t));
	s.write(x.imageData.str().c_str(), x.imageSize); 
	return s; 
}
stringstream& operator >> (stringstream& s, sWFOVImageUpdate& x){
	std::vector<char> vect_char;
	s >> x.utmCorners;
	s.read(reinterpret_cast<char*>(&(x.imageSize)), sizeof(uint32_t));    

	vect_char.resize(x.imageSize);
	s.read(reinterpret_cast<char*>(&vect_char[0]), x.imageSize);
	x.imageData.clear(); 
	x.imageData.str("");
	x.imageData.write(&vect_char[0], x.imageSize);
	return s;
}

ostream& operator<< (ostream& s, const sWFOVImageUpdate& x) {
	s << "UTM Corners :\n" << x.utmCorners << "\n";
	s << "Im Size     : " <<  x.imageSize << "\n";
	s << "Im Data     :\n" << x.imageData.str().c_str() << "\n";
	return s;
}