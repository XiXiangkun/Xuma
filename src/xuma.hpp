#ifndef XUMA
#define XUMA

#include <string>
#include <vector>

extern int maxNodes;
extern int totalCores;

void getNodeMemory();
void getCpuTime(const std::string& statLine, std::vector<int>& cpuTotalTime, std::vector<int>& cpuIdleTime);
void countCpuUtilization(std::vector<int>& oldCpuTotalTime, std::vector<int>& oldCpuIdleTime, std::vector<int>& cpuTotalTime, std::vector<int>& cpuIdleTime, std::vector<float>& cpuUtilizationPerNode);
void catProcStat(std::vector<int>& oldCpuTotalTime, std::vector<int>& oldCpuIdleTime, std::vector<int>& cpuTotalTime, std::vector<int>& cpuIdleTime, std::vector<float>& cpuUtilizationPerNode);
void drawDetails();
void drawProgressBar(int y, int x, const std::string& name, int start, int end, const std::vector<float>& data);
void updateData(volatile bool& exitFlag, std::vector<float>& cpuUtilizationPerNode);
void checkInput(volatile bool& exitFlag);


#endif
