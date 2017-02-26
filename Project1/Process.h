#pragma once
#include <string>
#include <iostream>
 
class Process {
public:
    // constructor
    Process(char p, int a, int bt, int nb, int io)
        : pid(p), arrivalTime(a), burstTime(bt), nBurst(nb), ioTime(io), currBurst(1), timeRemaining(bt), waitTime(0) {}
	// default constructor
	Process() {};
	// copy constructor
	// Process(const Process& p);

    // getters
    char getID() const { return pid; }
	int getArrivalTime() const{ return arrivalTime; }
	int getIOTime() const{ return ioTime; }
	int getBurstTime() const{ return burstTime; }
    int getRemaining() const { return timeRemaining; }
    int getWaitTime() const { return waitTime; }
	int getEndIO() const { return endIO; }

    // setters
    void setRemaining(int t) { timeRemaining = t; }
    void setEndBurst(int t) { endBurst = t + burstTime; }
	void setEndIO(int t) { endIO = t + ioTime; }
    void setArrivalTime(int t) { arrivalTime = t; }

    // incrementors
    void incCurrBurst() { currBurst++; }
    void incWaitTime() { waitTime++; }
 
    // tests
	bool arrived(int t) const{ return t == arrivalTime; }
    bool burstDone(int t) { return t == endBurst; }
    bool ioDone(int t) { return t == endIO; }
    bool completed();   // test whether process has more cpu bursts to perform
 
    // overloaded operators
    friend std::ostream& operator<<(std::ostream& os, const Process& p);
 
private:
    char pid;
    int arrivalTime;
    int burstTime;
    int nBurst;
    int ioTime;
 
    int currBurst;
    int timeRemaining;
    int waitTime;
 
    int endIO;
    int endBurst;
 };