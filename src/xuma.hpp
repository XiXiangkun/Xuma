#ifndef XUMA
#define XUMA

#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

extern bool exitFlag;
extern int maxNodes;
extern int totalCores;
extern int currentMode;
extern std::vector<float> cpuUtilizationPerNode;
extern std::vector<float> cpuUtilizationPerCore;
extern std::vector<float> memUtilizationPerNode;
extern std::vector<std::vector<int>> nodesImage;
extern std::mutex mutexStopFlag;
extern std::mutex mutexModeFlag;
extern std::condition_variable cvModeFlag;
extern std::condition_variable cvStopFlag;

void getNodeMemory();
void getCpuTime(const std::string& statLine, std::vector<int>& cpuTotalTime, std::vector<int>& cpuIdleTime);
void countCpuUtilization(std::vector<int>& oldCpuTotalTime, std::vector<int>& oldCpuIdleTime, std::vector<int>& cpuTotalTime, std::vector<int>& cpuIdleTime);
void catProcStat(std::vector<int>& oldCpuTotalTime, std::vector<int>& oldCpuIdleTime, std::vector<int>& cpuTotalTime, std::vector<int>& cpuIdleTime);
void drawDetails();
void drawCPUInfo(int start, int tpy);
void drawMemInfo(int start);
void drawProgressBar(int y, int x, int start, int end, const std::vector<float>& data, int col, int typ);
void drawMemProgressBar(int y, int x, int start, int end, const std::vector<float>& data, int col, int typ);
void updateData();
void checkInput();


#endif
