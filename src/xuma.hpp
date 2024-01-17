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
void drawProcesses();
void drawCPUInfo(int start, int tpy);
void drawMemInfo(int start);
void drawProgressBar(int y, int x, int start, int end, const std::vector<float>& data, int col, int typ);
void drawMemProgressBar(int y, int x, int start, int end, const std::vector<float>& data, int col, int typ);
void updateData();
void checkInput();

struct CPUInfo {
        std::string cpuNum;
        int user;           // 用户态nice优先级运行的时间ce优先级运行的时间
        int nice;           // nice优先级运行的时间内核态核态
        int system;         // 内核态空闲状态闲状态
        int idle;           // 空闲状态等待I/O操作的时间待I/O操作的时间
        int iowait;         // 等待I/O操作的时间处理硬件中断的时间理硬件中断的时间
        int irq;            // 处理硬件中断的时间处理软件中断的时间理软件中断的时间
        int softirq;        // 处理软件中断的时间虚拟化环境中处理器抢占时间拟化环境中处理器抢占时间
        int steal;          // 虚拟化环境中处理器抢占时间虚拟CPU运行时间拟CPU运行时间
        int guest;          // 虚拟CPU运行时间nice优先级运行的虚拟CPU时间ce优先级运行的虚拟CPU时间
        int guest_nice;     // nice优先级运行的虚拟CPU时间
};

struct ProcessInfo {
        int pid;
        int virt;           // 虚拟内存 /proc/pid/statm的数值 * 64 KB
        int res;            // 物理内存 /proc/pid/statm的数值 * 64 KB
        int shr;            // 共享内存 /proc/pid/statm的数值 * 64 KB
        float cpu;          // ((utime2 - utime1) + (stime2 - stime1)) / 2
        float mem;          // res * 64 / totalMem
        std::string name;   // 进程名称
        int processor;      // 进程最后所在cpu
};

#endif
