#include <iostream>
#include <fstream>
#include <queue>
#include <string>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <numeric>
#include <sstream>
#include "Process.h"

#define t_cs 6
#define t_slice 94
 
// Comparators
struct CmpSRT { // Shortest time remaining comp
	bool operator()(const Process& p1, const Process& p2) {
		return (p1.getRemaining() > p2.getRemaining() || (p1.getRemaining() == p2.getRemaining() && p1.getID() > p2.getID()));
	}
};

struct CmpIO { // I/O time comp
	bool operator()(const Process& p1, const Process& p2) {
		return (p1.getEndIO() > p2.getEndIO() || (p1.getEndIO() == p2.getEndIO() && p1.getID() > p2.getID()));
	}
};

typedef std::priority_queue<Process, std::deque<Process>, CmpSRT > PPQ;

// function prototypes
void readFile(std::deque<Process>& processVector, std::ifstream& infile);
void printEvent(int time, std::string detail);
void doFCFS(std::deque<Process> processList, std::ofstream& outfile);
void doSRT(const std::deque<Process>& processList, std::ofstream& outfile);
void doRR(const std::deque<Process>& processList, std::ofstream& outfile);
std::string printQueue(const std::deque<Process>& readyQueue);
std::string printQueue(PPQ readyQueue);


int main(int argc, char* argv[]) {
    // check # args
    if (argc != 3) {
        std::cerr << "ERROR: Invalid arguments" << std::endl;
        return 1;
    }
 
    // check filename
    std::ifstream input(argv[1]);
    if (!input.good()) {
        std::cerr << "ERROR: Invalid input file format" << std::endl;
		std::cerr << "USAGE: .//a.out <input-file> <stats-output-file>" << std::endl;
        return 1;
    }

	// check filename
	std::ofstream output(argv[2]);
	if (!output.good()) {
		std::cerr << "ERROR: Invalid input file format" << std::endl;
		std::cerr << "USAGE: .//a.out <input-file> <stats-output-file>" << std::endl;
		return 1;
	}
	
    std::deque<Process> processList;
    readFile(processList, input); 
		
    doFCFS(processList, output);
    //doSRT(processList, output);
    //doRR(processList, output);
 
    return 0;
}
 
void readFile(std::deque<Process>& processList, std::ifstream& infile) {
    std::string line;
 
    while (std::getline(infile, line))
    {
        // check for comment lines
        if (line[0] == '#') continue;
        // manage whitespace
        size_t tmp = line.find_first_not_of(" \t\n\r\v");
        if (tmp == std::string::npos || line.empty()) continue; // empty line
        if (tmp != 0) {                                         // strip leading whitespace
            line.erase(0, tmp);
        }
        // parse valid lines
        char a;
        int b, c, d, e;
        std::sscanf(line.c_str(), "%c|%d|%d|%d|%d", &a, &b, &c, &d, &e);
        processList.push_back(Process(a, b, c, d, e));
    }
}
 
void doFCFS(std::deque<Process> processList, std::ofstream& outfile) {
    std::deque<Process> readyQueue;
    std::vector<Process> ioList;
    std::make_heap(ioList.begin(), ioList.end(), CmpIO());
    
    Process currProcess;
    int t = 0;
    bool cpuBusy = false;
    bool doingCS = false;
	int csStart, csEnd;
    int numCS = 0;

    std::vector<int> waitTimes;
    std::vector<int> turnaroundTimes;
    std::vector<int> cpuBurstTimes;
 
    std::cout << "time " << t << "ms: Simulator started for FCFS "  << printQueue(readyQueue) << std::endl;
     
    while (1) {	
        // load arrived processes into ready queue
        while (!processList.empty() && processList.front().arrived(t)) {
            readyQueue.push_back(processList.front());
            std::cout << "time " << t << "ms: Process " << processList.front().getID() << " arrived and added to ready queue " << printQueue(readyQueue) << std::endl;
            processList.pop_front();
        }
        
        // check that context switch is done (end of process)
        if (doingCS && t == csEnd) {
            doingCS = false;
            cpuBusy = false;

			turnaroundTimes.push_back(csEnd - currProcess.getArrivalTime());
            // "send to IO"
			if(!currProcess.completed()){
				currProcess.setEndIO(t);
				ioList.push_back(currProcess);
				std::push_heap(ioList.begin(), ioList.end(), CmpIO());
			}
        }
		
		if (!ioList.empty()) {
			// check if items in heap are done with I/O
			while (!ioList.empty() && ioList.front().ioDone(t)) {
				// check if process has more bursts to perform
				if (!ioList.front().completed()) {
					// update arrival time and send to ready queue
					ioList.front().setArrivalTime(t);
					ioList.front().clearWaitTime();
					readyQueue.push_back(ioList.front());
					std::cout << "time " << t << "ms: Process " << ioList.front().getID() << " completed I/O; added to ready queue " << printQueue(readyQueue) << std::endl;
				}

				// remove finished process from io list
                std::pop_heap(ioList.begin(), ioList.end(), CmpIO());
				ioList.pop_back();
			}
		}
		
		// load next process into cpu (context switch)
        if (!readyQueue.empty() && !cpuBusy) {
            cpuBusy = true;
            doingCS = true;
            csStart = t + t_cs/2;
			numCS++;
            // remove process from ready queue
            currProcess = readyQueue.front();
            readyQueue.pop_front();
			waitTimes.push_back(currProcess.getWaitTime());
		}
		
		// check context switch is done (to cpu)
        if (doingCS && t == csStart) {
            doingCS = false;
			currProcess.setEndBurst(t);
			std::cout << "time " << t << "ms: Process " << currProcess.getID() << " started using the CPU " << printQueue(readyQueue) << std::endl;
        }
		
		// manage current process
        if (cpuBusy && !doingCS) {
            // is cpu burst done?
            if (currProcess.burstDone(t)) {
				currProcess.incCurrBurst();
				// begin context swtich (out of cpu) if there is something to be switched in
				doingCS = true;
				csEnd = t + t_cs/2;
				currProcess.setEndIO(csEnd);
				cpuBurstTimes.push_back(currProcess.getBurstTime());
				if(currProcess.completed()){
					std::cout << "time " << t << "ms: Process " << currProcess.getID() << " terminated "<< printQueue(readyQueue) << std::endl;
				}else{
					std::cout << "time " << t << "ms: Process " << currProcess.getID() << " completed a CPU burst; "<< currProcess.getBursts() << ((currProcess.getBursts()==1)? " burst":" bursts") <<" to go "<< printQueue(readyQueue) << std::endl;
					std::cout << "time " << t << "ms: Process " << currProcess.getID() << " switching out of CPU; will block on I/O until time "<< currProcess.getEndIO() << "ms " << printQueue(readyQueue) << std::endl;
				}
			}
        }
		
		if (readyQueue.empty() && processList.empty() && ioList.empty() && !cpuBusy) break;
		t++;
		// update wait times of all processes in ready queue
        if (!readyQueue.empty()) {
            for (unsigned int i = 0; i < readyQueue.size(); ++i) {
                readyQueue[i].incWaitTime();
            }
        }
    }
	
	outfile << "Algorithm FCFS" << std::endl;
	outfile << "--average CPU burst time : " << std::setprecision(2) << std::fixed <<(std::accumulate(cpuBurstTimes.begin(), cpuBurstTimes.end(), 0) / (double)cpuBurstTimes.size()) << std::endl;
	outfile << "--average wait time : " <<(std::accumulate(waitTimes.begin(), waitTimes.end(), 0) / (double)waitTimes.size()) << std::endl;
	outfile << "--average turnaround time : " << (std::accumulate(turnaroundTimes.begin(), turnaroundTimes.end(), 0) / (double)turnaroundTimes.size()) << std::endl;
	outfile << "--total number of context switches : " << numCS << std::endl;
	outfile << "--total number of premeptions : " << 0 << std::endl;
	
	
	std::cout << "time " << t << "ms: Simulator ended for FCFS" << std::endl;
}
 
void doSRT(const std::deque<Process>& processList, std::ofstream& outfile) {
    PPQ processQueue(processList.begin(), processList.end());
     
	std::cout << printQueue(processQueue);
    std::cout << std::endl;

	 // test print
    while (!processQueue.empty()) {
        std::cout << processQueue.top() << std::endl;
        processQueue.pop();
    }
}
 
void doRR(const std::deque<Process>& processList, std::ofstream& outfile) {
    std::queue<Process> processQueue(processList);

	// test print
	while (!processQueue.empty()) {
		std::cout << processQueue.front() << std::endl;
		processQueue.pop();
	}
	std::cout << std::endl;

}

std::string printQueue(const std::deque<Process>& readyQueue) {
	std::ostringstream os;
    if(readyQueue.empty()){
        os<< "[Q <empty>]";
        return os.str();
    }
	os << "[Q ";
	for (unsigned int i = 0; i < readyQueue.size();++i){
		os << readyQueue[i].getID();
		if (i != readyQueue.size() - 1) {
			os << " ";
		}
	}
	os << "]";
	return os.str();
}

std::string printQueue(PPQ readyQueue) {
	std::ostringstream os;
	if(readyQueue.empty()){
        os<< "[Q <empty>]";
        return os.str();
    }
	os << "[Q ";
	while(!readyQueue.empty()){
		os << readyQueue.top().getID();
		readyQueue.pop();
		if (readyQueue.size() > 0) {
			os << " ";
		}
    }
	os << "]";
	return os.str();
}