#include "SystemStatus.h"

QDataStream& operator << (QDataStream& s, const sSystemStatus& x){
	uint32_t numValidROIS = min(x.numValidROIS, uint32_t(MAX_NUM_ROIS));
	s << x.running;
	s << x.utmCoord;
	s << x.altitude;
	s << x.heading;

	s << x.NFOVAvailable;
	s << x.currentNFOVCommandID;

	s << numValidROIS;
	for (int i =0; i< numValidROIS; i++)
		s << x.roiId[i];
	for (int i =0; i< numValidROIS; i++)
		s << x.roiState[i];
	return s;
}
QDataStream& operator >> (QDataStream& s, sSystemStatus& x){
	s >> x.running;
	s >> x.utmCoord;
	s >> x.altitude;
	s >> x.heading;

	s >> x.NFOVAvailable;
	s >> x.currentNFOVCommandID;

	s >> x.numValidROIS;
	x.numValidROIS = min(x.numValidROIS, uint32_t(MAX_NUM_ROIS));
	for (int i =0; i< x.numValidROIS; i++)
		s >> x.roiId[i];
	for (int i =0; i< x.numValidROIS; i++)
		s >> x.roiState[i];
	return s;
}
stringstream& operator << (stringstream& s, const sSystemStatus& x){
	uint32_t numValidROIS = min(x.numValidROIS, uint32_t(MAX_NUM_ROIS));
	s.write(reinterpret_cast<const char*>(&(x.running)), sizeof(bool));
	s << x.utmCoord;
	s.write(reinterpret_cast<const char*>(&(x.altitude)), sizeof(float));
	s.write(reinterpret_cast<const char*>(&(x.heading)), sizeof(float));

	s.write(reinterpret_cast<const char*>(&(x.NFOVAvailable)), sizeof(bool));
	s.write(reinterpret_cast<const char*>(&(x.currentNFOVCommandID)), sizeof(uint32_t));

	s.write(reinterpret_cast<const char*>(&(numValidROIS)), sizeof(uint32_t));
	s.write(reinterpret_cast<const char*>(&(x.roiId[0])), numValidROIS*sizeof(uint32_t));
	s.write(reinterpret_cast<const char*>(&(x.roiState[0])), numValidROIS*sizeof(uint32_t));
	return s;
}
stringstream& operator >> (stringstream& s, sSystemStatus& x){
	s.read(reinterpret_cast<char*>(&(x.running)), sizeof(bool));
	s >> x.utmCoord;
	s.read(reinterpret_cast<char*>(&(x.altitude)), sizeof(float));
	s.read(reinterpret_cast<char*>(&(x.heading)), sizeof(float));

	s.read(reinterpret_cast<char*>(&(x.NFOVAvailable)), sizeof(bool));
	s.read(reinterpret_cast<char*>(&(x.currentNFOVCommandID)), sizeof(uint32_t));

	s.read(reinterpret_cast<char*>(&(x.numValidROIS)), sizeof(uint32_t));
	x.numValidROIS = min(x.numValidROIS, uint32_t(MAX_NUM_ROIS));
	s.read(reinterpret_cast<char*>(&(x.roiId[0])), x.numValidROIS*sizeof(uint32_t));
	s.read(reinterpret_cast<char*>(&(x.roiState[0])), x.numValidROIS*sizeof(uint32_t));
	return s;
}

ostream& operator<< (ostream& s, const sSystemStatus& x) {
	s << "Running        : " << x.running << "\n";
	s << "UTM Coord      : " << x.utmCoord << "\n"; 
	s << "Altitude       : " << x.altitude << "\n";

	s << "NFOV Active    : " << x.NFOVAvailable <<"\n";
	s << "Current Cmd ID : " << x.currentNFOVCommandID <<"\n";

	s << "NumValid Roi   : " << x.numValidROIS << "\n";
	s << "ROI ID         : ";
	for (uint32_t i =0; i< x.numValidROIS; i++)
		s << x.roiId[i] << " ";
	s << "\nROI ID State : ";
	for (uint32_t i =0; i< x.numValidROIS; i++)
		s << x.roiState[i] << " ";
	s << "\n";
	return s;
}
