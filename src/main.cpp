#include <vector>
#include <thread>
#include <numa.h>
#include <termios.h>
#include <ncurses.h>
#include <iostream>
#include "xuma.hpp"


int main() {
		totalCores = numa_num_configured_cpus();
        maxNodes = numa_max_node();
        
		volatile bool exitFlag = false;
        std::vector<float> cpuUtilizationPerNode(maxNodes + 1, 0);

        // ncurses相关的初始化
        initscr();
        curs_set(0);
        if ( has_colors() == FALSE) {
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
        std::thread dataUpdateThread(updateData, std::ref(exitFlag), std::ref(cpuUtilizationPerNode));
        std::thread inputCheckThread(checkInput, std::ref(exitFlag));
        
		while (!exitFlag) {
                clear();
                drawDetails();
                drawProgressBar(2, 2, "Left Progress", 0, (maxNodes + 1) / 2, cpuUtilizationPerNode);
                drawProgressBar(2, getmaxx(stdscr) / 2 + 2, "Right Progress", (maxNodes + 1) / 2, maxNodes + 1, cpuUtilizationPerNode);
				{
						std::unique_lock<std::mutex> lock(mutexExitFlag);
						if (cvExitFlag.wait_for(lock, std::chrono::milliseconds(1000)) == std::cv_status::no_timeout) {
								break; // 如果条件满足，退出循环
						}
				}
                refresh();
        }
        
		dataUpdateThread.join();
        inputCheckThread.join();
        endwin();
        
		return 0;
}
