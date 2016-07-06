#include "UTMdef.h"

QDataStream& operator << (QDataStream& s, const sUTMCoord& x) {
	s << x.Northing;
	s << x.Easting;
	s.writeBytes(x.Zone,4);
	return s;
}
QDataStream& operator >> (QDataStream& s, sUTMCoord& x) {
	s >> x.Northing;
	s >> x.Easting;
	s.readRawData(x.Zone,4);
	return s;
}

stringstream& operator << (stringstream& s, const sUTMCoord& x) {
	s.write(reinterpret_cast<const char*>(&(x.Northing)), sizeof(double));
	s.write(reinterpret_cast<const char*>(&(x.Easting)), sizeof(double));
	s.write(reinterpret_cast<const char*>(&(x.Zone)), 4);
	return s; 
}
stringstream& operator >> (stringstream& s, sUTMCoord& x) {
	s.read(reinterpret_cast<char*>(&(x.Northing)), sizeof(double));
	s.read(reinterpret_cast<char*>(&(x.Easting)), sizeof(double));
	s.read(reinterpret_cast<char*>(&(x.Zone)), 4);
	return s;
}

ostream& operator<< (ostream& s, const sUTMCoord& x) {
	s << "  East=" << x.Easting << " North=" << x.Northing << " Zone=" << x.Zone;
	return s;
}

QDataStream& operator << (QDataStream& s, const sUTMCorners& x) {
	s << x.Northing[0] << x.Northing[1] << x.Northing[2] << x.Northing[3];
	s << x.Easting[0] << x.Easting[1] << x.Easting[2] << x.Easting[3];
	s.writeBytes(x.Zone,4);
	return s;
}

QDataStream& operator >> (QDataStream& s, sUTMCorners& x) {
	s >> x.Northing[0] >> x.Northing[1] >> x.Northing[2] >> x.Northing[3];
	s >> x.Easting[0] >> x.Easting[1] >> x.Easting[2] >> x.Easting[3];
	s.readRawData(x.Zone,4);
	return s;
}

stringstream& operator << (stringstream& s, const sUTMCorners& x) {
	s.write(reinterpret_cast<const char*>(&(x.Northing)), 4*sizeof(double));
	s.write(reinterpret_cast<const char*>(&(x.Easting)), 4*sizeof(double));
	s.write(reinterpret_cast<const char*>(&(x.Zone)), 4);
	return s; 

}

stringstream& operator >> (stringstream& s, sUTMCorners& x) {
	s.read(reinterpret_cast<char*>(&(x.Northing)), 4*sizeof(double));
	s.read(reinterpret_cast<char*>(&(x.Easting)), 4*sizeof(double));
	s.read(reinterpret_cast<char*>(&(x.Zone)), 4);
	return s;

}

ostream& operator<< (ostream& s, const sUTMCorners& x) {
	s << "  East[0]=" << x.Easting[0] << " North[0]=" << x.Northing[0] << "\n"; 
	s << "  East[1]=" << x.Easting[1] << " North[1]=" << x.Northing[1] << "\n"; 
	s << "  East[2]=" << x.Easting[2] << " North[2]=" << x.Northing[2] << "\n"; 
	s << "  East[3]=" << x.Easting[3] << " North[3]=" << x.Northing[3] << "\n"; 
	s << "  Zone   =" << x.Zone;
	return s;
}
