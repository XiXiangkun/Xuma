#include <xuma.hpp>
#include <iostream>
#include <numa.h>
#include <fstream>
#include <sstream>
#include <iterator>
#include <vector>
#include <ncurses.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <termios.h>
#include <condition_variable>
#include <mutex>


void getNodeMemory(){
		long long memorySize, memoryFreep;
		struct bitmask *nodeMask = numa_allocate_nodemask();
        numa_bitmask_clearall(nodeMask);
        for (int node = 0; node <= maxNodes; ++node){
                memorySize = numa_node_size64(node, nullptr);
                numa_bitmask_setbit(nodeMask, node);
                numa_node_size64(node, &memoryFreep);
                // std::cout << "Node " << node << " size: " << memorySize / 1024 / 1024 / 1024 << " GB" << std::endl;
                // std::cout << "Node " << node << " available size: " << memoryFreep / 1024 / 1024 / 1024 << " GB" << std::endl;
        }
        numa_bitmask_free(nodeMask);
		return ;
}

void getCpuTime(const std::string& statLine, std::vector<int>& cpuTotalTime, std::vector<int>& cpuIdleTime){
		// std::cout << statLine << std::endl;
		std::istringstream iss(statLine);
		std::vector<std::string> tokens(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
		int sum = 0;
		int value = 0;
		for (size_t i = 1; i <= 7; ++i){
				value = std::stoi(tokens[i]);
				sum += value;
				if (i == 4){
						cpuIdleTime.push_back(value);
						// std::cout << "idle: " << value << std::endl;
				}
		}
		cpuTotalTime.push_back(sum);
		// std::cout << "sum: " << sum << std::endl;
		return ;
}

void countCpuUtilization(std::vector<int>& oldCpuTotalTime, std::vector<int>& oldCpuIdleTime, std::vector<int>& cpuTotalTime, std::vector<int>& cpuIdleTime){
		int total, idle;
		int	coresCount = 0;
		int coresPerNode = totalCores / (maxNodes + 1);
		float cpuUsed;
		float cpuUsedPerNode = 0;
		cpuUtilizationPerNode.clear();
		cpuUtilizationPerCore.clear();
		for (int i = 0; i < totalCores; i++){
				total = cpuTotalTime[i] - oldCpuTotalTime[i];
				// std::cout << "nowTotal: " << cpuTotalTime[i] << " oldTotal: " << oldCpuTotalTime[i] << " total: " << total << std::endl;
				idle = cpuIdleTime[i] - oldCpuIdleTime[i];
				// std::cout << "nowIdle: " << cpuIdleTime[i] << " oldIdle: " << oldCpuIdleTime[i] << " idle: " << idle << std::endl;
				cpuUsed = 1 - static_cast<float>(idle) / total;
				// std::cout << "cpuUsed: " << cpuUsed << std::endl;
				cpuUtilizationPerCore.push_back(cpuUsed);
				if (coresCount < coresPerNode - 1){
						cpuUsedPerNode += cpuUsed;
						coresCount++;
				} else if (coresCount == coresPerNode - 1){
						cpuUsedPerNode += cpuUsed;
						cpuUtilizationPerNode.push_back(cpuUsedPerNode);
						coresCount = 0;
						cpuUsedPerNode = 0;
				}
		}
		return ;
}

void catProcStat(std::vector<int>& oldCpuTotalTime, std::vector<int>& oldCpuIdleTime, std::vector<int>& cpuTotalTime, std::vector<int>& cpuIdleTime){
		oldCpuTotalTime = cpuTotalTime;
        oldCpuIdleTime = cpuIdleTime;
		// std::vector<int>().swap(cpuTotalTime);
		// std::vector<int>().swap(cpuIdleTime);
		cpuTotalTime.clear();
		cpuIdleTime.clear();
		std::ifstream procStat("/proc/stat");
		std::string statLine;
		while (std::getline(procStat, statLine)){
				if (statLine.substr(0, 3) == "cpu" && std::isdigit(statLine[3])){
						// std::cout << statLine << std::endl;
						getCpuTime(statLine, cpuTotalTime, cpuIdleTime);
				} else{
						continue;
				}
		}
		countCpuUtilization(oldCpuTotalTime, oldCpuIdleTime, cpuTotalTime, cpuIdleTime);
		return ;
}

void drawDetails(){
		mvprintw(1, 2, "Cores Count is ");
        attron(A_BOLD);
        printw("%d", totalCores);
        attroff(A_BOLD);
        printw(", Nodes Count is ");
        attron(A_BOLD);
        printw("%d", maxNodes + 1);
        attroff(A_BOLD);
        printw(", per Node with ");
        attron(A_BOLD);
        printw("%d", totalCores / (maxNodes + 1));
        attroff(A_BOLD);
        printw(" Cores");
}

void drawProgressBar(int y, int x, int start, int end, const std::vector<float>& data, int col){
		int max_x, max_y;
        int counter = 0;
        auto startIterator = data.begin() + start;
        auto endIterator = data.begin() + end;
		getmaxyx(stdscr, max_y, max_x);
		int bar_width = max_x / (col + 1) - 10;
		float fillPercent = totalCores / maxNodes;
		switch (col) {
				case 2:
						fillPercent = totalCores / maxNodes;
						break;
				case 4:
						fillPercent = 1;
						break;
				default:
						fillPercent = totalCores / maxNodes;
						break;
		}
		for (auto it = startIterator; it != endIterator; it++) {
				float percentage = *it;
				float fill_width = bar_width * (percentage / fillPercent);
				mvprintw(y + 1 + counter, x + 1, "Node %d", start);
				attron(COLOR_PAIR(1));
				mvprintw(y + 1 + counter, x + 9, "[");
				for (int i = 0; i < fill_width; ++i) {
				 		mvprintw(y + 1 + counter, x + i + 10, "|");
				}
				mvprintw(y + 1 + counter, x + bar_width + 10, "]");
				attroff(COLOR_PAIR(1));
				mvprintw(y + 1 + counter, x + bar_width + 1 + 10, " %.1f%%", percentage * 100);
				counter++;
				start++;
		}
		return ;
}

void updateData(){
		std::vector<int> oldCpuTotalTime(totalCores, 0);
        std::vector<int> oldCpuIdleTime(totalCores, 0);
        std::vector<int> cpuTotalTime;
        std::vector<int> cpuIdleTime;
		while (!exitFlag) {
				// getNodeMemory();
				catProcStat(oldCpuTotalTime, oldCpuIdleTime, cpuTotalTime, cpuIdleTime);
				{
						std::unique_lock<std::mutex> lock(mutexStopFlag);
						if (cvStopFlag.wait_for(lock, std::chrono::seconds(2)) == std::cv_status::no_timeout) {
								break;
						}
				}
		}
}

void checkInput(){
		while (!exitFlag){
				struct timeval timeout;
        		timeout.tv_sec = 3;
		        timeout.tv_usec = 0;
		        fd_set fds;
        		FD_ZERO(&fds);
		        FD_SET(STDIN_FILENO, &fds);

        		int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout);
		        if (ret > 0) {
        			    int ch = getch();
		    	        if (ch != ERR) {
								switch (ch) {
										case 'q':
										case 'Q':
												{
														// ModeFlag不会改变currentMode，但是会给main信号，退出当前循环
														// exitFlag保证了循环不会再进入
														std::unique_lock<std::mutex> lock(mutexModeFlag);
														exitFlag = true;
														cvModeFlag.notify_all();
												}
												{
														// StopFlag用于updateData线程的终止
														std::unique_lock<std::mutex> lock(mutexStopFlag);
                                                        cvStopFlag.notify_all();
												}
														break;
										case 'n':
										case 'N':
												{
														std::unique_lock<std::mutex> lock(mutexModeFlag);
														currentMode = 0;
                                                        cvModeFlag.notify_all();
														break;
												}
										case 'c':
										case 'C':
												{
                                                        std::unique_lock<std::mutex> lock(mutexModeFlag);
                                                        currentMode = 1;
                                                        cvModeFlag.notify_all();
                                                        break;
                                                }
										case 'i':
										case 'I':
												currentMode = 2;
                                        		continue;
										default:
												break;
								}
						}
        		}
		}
}
