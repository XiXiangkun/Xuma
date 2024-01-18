#ifndef CPUMEM
#define CPUMEM

void getNodeMemory();
void getCpuTime(const std::string& statLine, std::vector<int>& cpuTotalTime, std::vector<int>& cpuIdleTime);
void countCpuUtilization(std::vector<int>& oldCpuTotalTime, std::vector<int>& oldCpuIdleTime, std::vector<int>& cpuTotalTime, std::vector<int>& cpuIdleTime);
void catProcStat(std::vector<int>& oldCpuTotalTime, std::vector<int>& oldCpuIdleTime, std::vector<int>& cpuTotalTime, std::vector<int>& cpuIdleTime);

#endif
