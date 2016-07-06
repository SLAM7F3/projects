#include "MessageWrapper.h"

QDataStream& operator << (QDataStream& s, const sMessageWrapper& x) {
	s << x.messageID;
	s << x.length;
	s.writeBytes(x.body.str().c_str(),x.length);
	return s;
}
QDataStream& operator >> (QDataStream& s, sMessageWrapper& x) {
	std::vector<char> vect_char;
	s >> x.messageID;
	s >> x.length;

	vect_char.resize(x.length);
	s.readRawData(reinterpret_cast<char*>(&vect_char[0]),x.length);
	x.body.clear(); 
	x.body.str("");
	x.body.write(&vect_char[0], x.length);
	return s;
}
stringstream& operator << (stringstream& s, const sMessageWrapper& x) {
	s.write(reinterpret_cast<const char*>(&(x.messageID)), sizeof(uint32_t));
	s.write(reinterpret_cast<const char*>(&(x.length)), sizeof(uint32_t));
	s.write(x.body.str().c_str(), x.length); 
	return s;
}
stringstream& operator >> (stringstream& s, sMessageWrapper& x) {
	std::vector<char> vect_char;
	s.read(reinterpret_cast<char*>(&(x.messageID)), sizeof(uint32_t));    
	s.read(reinterpret_cast<char*>(&(x.length)), sizeof(uint32_t));    

	vect_char.resize(x.length);
	s.read(reinterpret_cast<char*>(&vect_char[0]), x.length);
	x.body.clear(); 
	x.body.str("");
	x.body.write(&vect_char[0], x.length);
	return s;
}

ostream& operator<< (ostream& s, const sMessageWrapper& x) {
	s << "Msg ID : " << x.messageID << "\n";
	s << "Length : " << x.length << "\n";
	s << "Body :   " << x.body.str().c_str() << "\n";
	return s;
}


sMessageWrapper& operator << (sMessageWrapper& s, const sROICommand& x) {
	s.messageID = 0x00010001;
	s.body.clear();
	s.body.str("");
	s.body << x;
   	s.length = uint32_t(s.body.str().length());
	return s;	
}
sMessageWrapper& operator >> (sMessageWrapper& s, sROICommand& x) {
	s.body.clear();
	s.body.seekg(ios_base::beg);
	s.body >> x;
	return s;	
}

sMessageWrapper& operator << (sMessageWrapper& s, const sROITrackUpdate& x) {
	s.messageID = 0x00010002;
	s.body.clear();
	s.body.str("");
	s.body << x;
   	s.length = uint32_t(s.body.str().length());
	return s;	
}
sMessageWrapper& operator >> (sMessageWrapper& s, sROITrackUpdate& x) {
	s.body.clear();
	s.body.seekg(ios_base::beg);
	s.body >> x;
	return s;	
}


sMessageWrapper& operator << (sMessageWrapper& s, const sROIImageUpdate& x) {
	s.messageID = 0x00010003;
	s.body.clear();
	s.body.str("");
	s.body << x;
   	s.length = uint32_t(s.body.str().length());
	return s;	
}
sMessageWrapper& operator >> (sMessageWrapper& s, sROIImageUpdate& x) {
	s.body.clear();
	s.body.seekg(ios_base::beg);
	s.body >> x;
	return s;	
}

sMessageWrapper& operator << (sMessageWrapper& s, const sNFOVCommand& x) {
	s.messageID = 0x00020001;
	s.body.clear();
	s.body.str("");
	s.body << x;
   	s.length = uint32_t(s.body.str().length());
	return s;	
}
sMessageWrapper& operator >> (sMessageWrapper& s, sNFOVCommand& x) {
	s.body.clear();
	s.body.seekg(ios_base::beg);
	s.body >> x;
	return s;	
}

sMessageWrapper& operator << (sMessageWrapper& s, const sNFOVImageUpdate& x) {
	s.messageID = 0x00020002;
	s.body.clear();
	s.body.str("");
	s.body << x;
   	s.length = uint32_t(s.body.str().length());
	return s;	
}
sMessageWrapper& operator >> (sMessageWrapper& s, sNFOVImageUpdate& x) {
	s.body.clear();
	s.body.seekg(ios_base::beg);
	s.body >> x;
	return s;	
}

sMessageWrapper& operator << (sMessageWrapper& s, const sWFOVImageUpdate& x) {
	s.messageID = 0x00030001;
	s.body.clear();
	s.body.str("");
	s.body << x;
   	s.length = uint32_t(s.body.str().length());
	return s;	
}
sMessageWrapper& operator >> (sMessageWrapper& s, sWFOVImageUpdate& x) {
	s.body.clear();
	s.body.seekg(ios_base::beg);
	s.body >> x;
	return s;	
}

sMessageWrapper& operator << (sMessageWrapper& s, const sSystemStatus& x) {
	s.messageID = 0x000F0001;
	s.body.clear();
	s.body.str("");
	s.body << x;
   	s.length = uint32_t(s.body.str().length());
	return s;	
}
sMessageWrapper& operator >> (sMessageWrapper& s, sSystemStatus& x) {
	s.body.clear();
	s.body.seekg(ios_base::beg);
	s.body >> x;
	return s;	
}

sMessageWrapper& operator << (sMessageWrapper& s, const sConsole& x) {
	s.messageID = 0x000F0002;
	s.body.clear();
	s.body.str("");
	s.body << x;
   	s.length = uint32_t(s.body.str().length());
	return s;	
}
sMessageWrapper& operator >> (sMessageWrapper& s, sConsole& x) {
	s.body.clear();
	s.body.seekg(ios_base::beg);
	s.body >> x;
	return s;	
}

