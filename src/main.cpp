#include <global.hpp>
#include <xuma.hpp>
#include <process.hpp>
#include <cpumem.hpp>
#include <draw.hpp>

bool exitFlag = false;
int maxNodes = 0;
int totalCores = 0;
int currentMode = 0;  // 0: 显示per Node的CPU占用 1: 显示 per Core的CPU占用 2: 显示具体每个Core上的进程
bool isPerNode = false;
int drawRow = 0;
int drawCol = 0;
std::vector<float> memUtilizationPerNode;
std::vector<float> cpuUtilizationPerNode;
std::vector<float> cpuUtilizationPerCore;
std::vector<std::vector<int>> nodesImage;
std::mutex mutexModeFlag;
std::mutex mutexStopFlag;
std::condition_variable cvModeFlag;
std::condition_variable cvStopFlag;
std::map<int, std::pair<long, long>> previousTimes;
std::vector<ProcessInfo> processesDisplay;


int main() {
		totalCores = numa_num_configured_cpus();
        maxNodes = numa_max_node(); 
		memUtilizationPerNode.resize(maxNodes + 1, 0);
        cpuUtilizationPerNode.resize(maxNodes + 1, 0);
		cpuUtilizationPerCore.resize(totalCores, 0);
		nodesImage.resize((maxNodes + 1) / 2, std::vector<int>(2));
		int tmpRow = 0;
		for (; tmpRow < (maxNodes + 1) / 2; tmpRow++){
				nodesImage[tmpRow][0] = tmpRow;
				nodesImage[tmpRow][1] = tmpRow + (maxNodes + 1) / 2;
		}

        // ncurses相关的初始化
        initscr();
        curs_set(0);
        if (has_colors() == FALSE) {
                endwin();
                return 1;
        }
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);  // CPU进度条的颜色设置
        init_pair(2, COLOR_WHITE, COLOR_BLACK);  // 屏幕背景的颜色设置
        init_pair(3, COLOR_GREEN, COLOR_BLACK);  // MEM进度条的颜色设置
		init_pair(4, COLOR_CYAN, COLOR_WHITE); 
        init_pair(5, COLOR_BLACK, COLOR_WHITE);
        wbkgd(stdscr, COLOR_PAIR(2));
		cbreak();
		noecho();
        clear();
        refresh();

		// 按键监视，刷新数据 
        std::thread dataUpdateThread(updateData);
        std::thread inputCheckThread(checkInput);
       
	   	int realMaxNodes = maxNodes + 1;
		int realNodesDivide2 = realMaxNodes / 2;
		int realCoresDivide4 = totalCores / 4;
		int maxX = getmaxx(stdscr);
		int maxXDivide2 = maxX / 2;
		int maxXDivide4 = maxX / 4;
		int startPosition = 3;

		while (!exitFlag) {
                clear();
                drawDetails();
                switch (currentMode) {
						case 0:  // 只显示Node和Mem
								drawCPUInfo(startPosition, 2);
								drawProgressBar(startPosition, 0, 0, realNodesDivide2, cpuUtilizationPerNode, 2, 1);
				                drawProgressBar(startPosition, maxXDivide2, realNodesDivide2, realMaxNodes, cpuUtilizationPerNode, 2, 1);
								drawMemInfo(startPosition + 2 + realNodesDivide2);
								drawMemProgressBar(startPosition + 2 + realNodesDivide2, 0, 0, realNodesDivide2, memUtilizationPerNode, 2, 3);
								drawMemProgressBar(startPosition + 2 + realNodesDivide2, maxXDivide2, realNodesDivide2, realMaxNodes, memUtilizationPerNode, 2, 3);
								refresh();
                				{
                        				std::unique_lock<std::mutex> lock(mutexModeFlag);
				                        if (cvModeFlag.wait_for(lock, std::chrono::milliseconds(1000)) == std::cv_status::no_timeout) {
                				                break; // 如果条件满足，退出循环
                		        		}
               					}
								break;
						case 1:  // 显示Core和Mem
								drawCPUInfo(startPosition, 4);
								drawProgressBar(startPosition, 0, 0, realCoresDivide4, cpuUtilizationPerCore, 4, 1);
                                drawProgressBar(startPosition, maxXDivide4, realCoresDivide4, realCoresDivide4 * 2, cpuUtilizationPerCore, 4, 1);
								drawProgressBar(startPosition, maxXDivide2, realCoresDivide4 * 2, realCoresDivide4 * 3, cpuUtilizationPerCore, 4, 1);
                                drawProgressBar(startPosition, maxXDivide4 * 3, realCoresDivide4 * 3, totalCores, cpuUtilizationPerCore, 4, 1);
								drawMemInfo(startPosition + 2 + realCoresDivide4);
								drawMemProgressBar(startPosition + 2 + realCoresDivide4, 0, 0, realNodesDivide2, memUtilizationPerNode, 2, 3);
								drawMemProgressBar(startPosition + 2 + realCoresDivide4, maxXDivide2, realNodesDivide2, realMaxNodes, memUtilizationPerNode, 2, 3);
								refresh();
                                {
                                        std::unique_lock<std::mutex> lock(mutexModeFlag);
                                        if (cvModeFlag.wait_for(lock, std::chrono::milliseconds(1000)) == std::cv_status::no_timeout) {
                                                break; // 如果条件满足，退出循环
                                        }
                                }
								break;
						case 2:  // 显示具体的进程%CPU
								drawProcesses();
								refresh();
								{
                                        std::unique_lock<std::mutex> lock(mutexModeFlag);
                                        if (cvModeFlag.wait_for(lock, std::chrono::milliseconds(1000)) == std::cv_status::no_timeout) {
                                                break; // 如果条件满足，退出循环
                                        }
                                }
                                break;
						default:  // 默认情况，只显示Node和Mem
								drawCPUInfo(startPosition, 2);
                                drawProgressBar(startPosition, 0, 0, realNodesDivide2, cpuUtilizationPerNode, 2, 1);
                                drawProgressBar(startPosition, maxXDivide2, realNodesDivide2, realMaxNodes, cpuUtilizationPerNode, 2, 1);
                                drawMemInfo(startPosition + 2 + realNodesDivide2);
                                drawMemProgressBar(startPosition + 2 + realNodesDivide2, 0, 0, realNodesDivide2, memUtilizationPerNode, 2, 3);
                                drawMemProgressBar(startPosition + 2 + realNodesDivide2, maxXDivide2, realNodesDivide2, realMaxNodes, memUtilizationPerNode, 2, 3);
								refresh();
                                {
                                        std::unique_lock<std::mutex> lock(mutexModeFlag);
                                        if (cvModeFlag.wait_for(lock, std::chrono::milliseconds(1000)) == std::cv_status::no_timeout) {
                                                break; // 如果条件满足，退出循环
                                        }
                                }
								break;
				}
        }
        
		dataUpdateThread.join();
        inputCheckThread.join();
        endwin();
        
		return 0;
}
