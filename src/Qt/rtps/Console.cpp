#include "Console.h"

/*
QDataStream& operator << (QDataStream& s, const sConsoleMessage& x) {
	s.writeBytes(&(x.source[0]), 32);
	s << x.length;
	s.writeBytes(x.body.str().c_str(),x.length);
	return s;
}

QDataStream& operator >> (QDataStream& s, sConsoleMessage& x) {
	std::vector<char> vect_char;
	s.readRawData(&(x.source[0]), 32);
	s >> x.length;

	vect_char.resize(x.length);
	s.readRawData(reinterpret_cast<char*>(&vect_char[0]),x.length);
	x.body.clear(); 
	x.body.str("");
	x.body.write(&vect_char[0], x.length);
	return s;
	return s;
}
*/

stringstream& operator << (stringstream& s, const sConsoleMessage& x) {
	s.write(reinterpret_cast<const char*>(&(x.source[0])), 32*sizeof(char));
	s.write(reinterpret_cast<const char*>(&(x.length)), sizeof(uint16_t));
	s.write(x.body.str().c_str(), x.length); 
	return s;
}

stringstream& operator >> (stringstream& s, sConsoleMessage& x) {
	std::vector<char> vect_char;
	s.read(reinterpret_cast<char*>(&(x.source[0])), 32*sizeof(char));    
	s.read(reinterpret_cast<char*>(&(x.length)), sizeof(uint16_t));    
	
	vect_char.resize(x.length);
	s.read(reinterpret_cast<char*>(&vect_char[0]), x.length);
	x.body.clear(); 
	x.body.str("");
	x.body.write(&vect_char[0], x.length);
	return s;
}

ostream& operator<< (ostream& s, const sConsoleMessage& x) {
	s << "  Source: " << x.source << "\n";
	s << "  Length: " << x.length << "\n";
	s << "  Body:   " << x.body.str().c_str() << "\n";
	return s;
}

/*
QDataStream& operator << (QDataStream& s, const sConsole& x) {
	uint32_t numMsg = min(x.numMsg, uint32_t(MAX_NUM_CONSOLE_MESSAGES));
	s << numMsg;
	for (int i=0; i<numMsg; i++)
		s << x.Msgs[i];
	return s;
}

QDataStream& operator >> (QDataStream& s, sConsole& x) {
	s >> x.numMsg;
	x.numMsg = min(x.numMsg, uint32_t(MAX_NUM_CONSOLE_MESSAGES));
	for (int i=0; i<x.numMsg; i++)
		s >> x.Msgs[i];
	return s;
}
*/

stringstream& operator << (stringstream& s, const sConsole& x) {
	uint32_t numMsg = min(x.numMsg, uint32_t(MAX_NUM_CONSOLE_MESSAGES));
	s.write(reinterpret_cast<const char*>(&numMsg), sizeof(uint32_t));
	for (int i=0; i<numMsg; i++)
		s << x.Msgs[i];
	return s;
}

stringstream& operator >> (stringstream& s, sConsole& x) {
	s.read(reinterpret_cast<char*>(&(x.numMsg)), sizeof(uint32_t));    
	x.numMsg = min(x.numMsg, uint32_t(MAX_NUM_CONSOLE_MESSAGES));
	for (int i=0; i<x.numMsg; i++)
		s >> x.Msgs[i];
	return s;
}

ostream& operator<< (ostream& s, const sConsole& x) {
	s << "Num Msgs: " << x.numMsg;
	for (int i=0; i<x.numMsg; i++) {
		s << " Message " << i << " :\n";
		s << x.Msgs[i] << "\n";
	}
	return s;
}
