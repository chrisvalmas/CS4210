#include "Process.h"

bool Process::completed() {
	return currBurst == nBurst;
}

// << operator overload
std::ostream& operator<<(std::ostream& os, const Process& p){
	os << "id: "<< p.pid << " arrival time: " << p.arrivalTime << " burst time: " << p.burstTime << " # bursts: " << p.nBurst << " io time: " << p.ioTime; 
	return os;
}