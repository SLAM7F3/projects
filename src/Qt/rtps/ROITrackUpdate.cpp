#include "ROITrackUpdate.h"

QDataStream& operator << (QDataStream& s, const sROITrackUpdate& x) {
	uint32_t numTracks = min(x.numTracks, uint32_t(MAX_NUM_TRACKS));
	s << x.regionID;
	s << x.trackUpdatePriority;
	s << numTracks;
	s << x.numChips;
	for (int i=0; i<numTracks; i++)
		s << x.tracks[i];
	return s;
}
QDataStream& operator >> (QDataStream& s, sROITrackUpdate& x) {
	s >> x.regionID;
	s >> x.trackUpdatePriority;
	s >> x.numTracks;
	x.numTracks =  min(x.numTracks, uint32_t(MAX_NUM_TRACKS));
	s >> x.numChips;
	for (int i=0; i<x.numTracks; i++)
		s >> x.tracks[i];
	return s;
}
stringstream& operator << (stringstream& s, const sROITrackUpdate& x) {
	uint32_t numTracks = min(x.numTracks, uint32_t(MAX_NUM_TRACKS));
	s.write(reinterpret_cast<const char*>(&(x.regionID)), sizeof(uint32_t));
	s.write(reinterpret_cast<const char*>(&(x.trackUpdatePriority)), sizeof(uint32_t));
	s.write(reinterpret_cast<const char*>(&(numTracks)), sizeof(uint32_t));
	s.write(reinterpret_cast<const char*>(&(x.numChips)), sizeof(uint32_t));
	for (int i=0; i<numTracks; i++)
		s << x.tracks[i];
	return s; 
}
stringstream& operator >> (stringstream& s, sROITrackUpdate& x) {
	s.read(reinterpret_cast<char*>(&(x.regionID)), sizeof(uint32_t));
	s.read(reinterpret_cast<char*>(&(x.trackUpdatePriority)), sizeof(uint32_t));
	s.read(reinterpret_cast<char*>(&(x.numTracks)), sizeof(uint32_t));
	x.numTracks =  min(x.numTracks, uint32_t(MAX_NUM_TRACKS));
	s.read(reinterpret_cast<char*>(&(x.numChips)), sizeof(uint32_t));
	for (int i=0; i<x.numTracks; i++)
		s >> x.tracks[i];
	return s;
}

ostream& operator<< (ostream& s, const sROITrackUpdate& x) {
	s << "Region ID   : " << x.regionID << "\n";
	s << "Trk Upd Pri : " << x.trackUpdatePriority << "\n";
	s << "Num Tracks  : " << x.numTracks << "\n";
	s << "Num Chips   : " << x.numChips << "\n";
	for (int i =0; i<x.numTracks; i++) {
		s << "Track " << i << ":\n";
	    s << x.tracks[i] << "\n";
	}
	return s;
}