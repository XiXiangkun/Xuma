#include <vector>
#include <thread>
#include <numa.h>
#include <termios.h>
#include <ncurses.h>
#include <iostream>
#include "xuma.hpp"

bool exitFlag = false;
int maxNodes = 0;
int totalCores = 0;
int currentMode = 0;  // 0: 显示per Node的CPU占用 1: 显示 per Core的CPU占用 2: 显示具体每个Core上的进程
std::vector<float> cpuUtilizationPerNode;
std::vector<float> cpuUtilizationPerCore;
std::mutex mutexModeFlag;
std::mutex mutexStopFlag;
std::condition_variable cvModeFlag;
std::condition_variable cvStopFlag;


int main() {
		totalCores = numa_num_configured_cpus();
        maxNodes = numa_max_node(); 
        cpuUtilizationPerNode.resize(maxNodes + 1, 0);
		cpuUtilizationPerCore.resize(totalCores, 0);

        // ncurses相关的初始化
        initscr();
        curs_set(0);
        if (has_colors() == FALSE) {
                endwin();
                return 1;
        }
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);  // 进度条的颜色设置
        init_pair(2, COLOR_WHITE, COLOR_BLACK);  // 屏幕背景的颜色设置
        wbkgd(stdscr, COLOR_PAIR(2));
		cbreak();
		noecho();
        clear();
        refresh();

		// 按键监视，刷新数据 
        std::thread dataUpdateThread(updateData);
        std::thread inputCheckThread(checkInput);
        
		while (!exitFlag) {
                clear();
                drawDetails();
                switch (currentMode) {
						case 0:
								drawProgressBar(2, 0, 0, (maxNodes + 1) / 2, cpuUtilizationPerNode, 2);
				                drawProgressBar(2, getmaxx(stdscr) / 2, (maxNodes + 1) / 2, maxNodes + 1, cpuUtilizationPerNode, 2);
								refresh();
                				{
                        				std::unique_lock<std::mutex> lock(mutexModeFlag);
				                        if (cvModeFlag.wait_for(lock, std::chrono::milliseconds(1000)) == std::cv_status::no_timeout) {
                				                break; // 如果条件满足，退出循环
                		        		}
               					}
								break;
						case 1:
								drawProgressBar(2, 0, 0, totalCores / 4, cpuUtilizationPerCore, 4);
                                drawProgressBar(2, getmaxx(stdscr) / 4, totalCores / 4, totalCores / 2, cpuUtilizationPerCore, 4);
								drawProgressBar(2, getmaxx(stdscr) / 2, totalCores / 2, totalCores / 4 * 3, cpuUtilizationPerCore, 4);
                                drawProgressBar(2, getmaxx(stdscr) / 4 * 3, totalCores / 4 * 3, totalCores, cpuUtilizationPerCore, 4);
								refresh();
                                {
                                        std::unique_lock<std::mutex> lock(mutexModeFlag);
                                        if (cvModeFlag.wait_for(lock, std::chrono::milliseconds(1000)) == std::cv_status::no_timeout) {
                                                break; // 如果条件满足，退出循环
                                        }
                                }
								break;
						case 2:
						default:
								drawProgressBar(2, 0, 0, (maxNodes + 1) / 2, cpuUtilizationPerNode, 2);
                                drawProgressBar(2, getmaxx(stdscr) / 2, (maxNodes + 1) / 2, maxNodes + 1, cpuUtilizationPerNode, 2);
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
