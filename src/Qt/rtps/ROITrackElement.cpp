#include "ROITrackElement.h"

QDataStream& operator << (QDataStream& s, const sTrackElement& x) {
	s << x.utmCoord;
	s << x.trackID;
	s << x.chipSize;
	if (x.chipSize > 0) {
		s.writeBytes(x.chipData.str().c_str(),x.chipSize);
		s << x.utmCorners;
	}
	return s;
}
QDataStream& operator >> (QDataStream& s, sTrackElement& x) {
	std::vector<char> vect_char;
	s >> x.utmCoord;
	s >> x.trackID;
	s >> x.chipSize;

	if (x.chipSize > 0) {
		vect_char.resize(x.chipSize);
		s.readRawData(reinterpret_cast<char*>(&vect_char[0]),x.chipSize);
		x.chipData.clear(); 
		x.chipData.str("");
		x.chipData.write(&vect_char[0], x.chipSize);
		s >> x.utmCorners;
	}
	return s;
}
stringstream& operator << (stringstream& s, const sTrackElement& x) {
	s << x.utmCoord;
	s.write(reinterpret_cast<const char*>(&(x.trackID)), sizeof(uint32_t));
	s.write(reinterpret_cast<const char*>(&(x.chipSize)), sizeof(uint32_t));
	if (x.chipSize > 0) {
		s.write(x.chipData.str().c_str(), x.chipSize); 
		s << x.utmCorners;
	}
	return s; 
}
stringstream& operator >> (stringstream& s, sTrackElement& x) {
	std::vector<char> vect_char;
	s >> x.utmCoord;
	s.read(reinterpret_cast<char*>(&(x.trackID)), sizeof(uint32_t));    
	s.read(reinterpret_cast<char*>(&(x.chipSize)), sizeof(uint32_t));    

	if (x.chipSize > 0) {
		vect_char.resize(x.chipSize);
		s.read(reinterpret_cast<char*>(&vect_char[0]), x.chipSize);
		x.chipData.clear(); 
		x.chipData.str("");
		x.chipData.write(&vect_char[0], x.chipSize);
		s >> x.utmCorners;
	}
	return s;
}

ostream& operator<< (ostream& s, const sTrackElement& x) {
	s << "  UTM Coord : " << x.utmCoord << "\n";
	s << "  Track ID  : " << x.trackID << "\n";
	s << "  Chip Size : " << x.chipSize << "\n";
	if (x.chipSize > 0) {
		s << "  Chip Data :\n" << x.chipData.str().c_str() << "\n";
		s << "  UTM Corners :\n" << x.utmCorners;
	}
	return s;
}