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

bool isPerNode = false;
int drawRow = 0;
int drawCol = 0;

struct CPUInfo {
		std::string cpuNum;	
		int user;			// 用户态nice优先级运行的时间ce优先级运行的时间
		int nice;			// nice优先级运行的时间内核态核态
		int system;			// 内核态空闲状态闲状态
		int idle;			// 空闲状态等待I/O操作的时间待I/O操作的时间
		int iowait;			// 等待I/O操作的时间处理硬件中断的时间理硬件中断的时间
		int irq;			// 处理硬件中断的时间处理软件中断的时间理软件中断的时间
		int softirq;		// 处理软件中断的时间虚拟化环境中处理器抢占时间拟化环境中处理器抢占时间
		int steal;			// 虚拟化环境中处理器抢占时间虚拟CPU运行时间拟CPU运行时间
		int guest;			// 虚拟CPU运行时间nice优先级运行的虚拟CPU时间ce优先级运行的虚拟CPU时间
		int guest_nice;		// nice优先级运行的虚拟CPU时间
};

void getNodeMemory(){
		float tmpRes;
		memUtilizationPerNode.clear();
		long long memorySize, memoryFree;
		struct bitmask *nodeMask = numa_allocate_nodemask();
        numa_bitmask_clearall(nodeMask);
        for (int node = 0; node <= maxNodes; ++node){
                memorySize = numa_node_size64(node, nullptr);
                numa_bitmask_setbit(nodeMask, node);
                numa_node_size64(node, &memoryFree);
				tmpRes = 1 - static_cast<float>(memoryFree) / memorySize;
				memUtilizationPerNode.push_back(tmpRes);
                // std::cout << "Node " << node << " size: " << memorySize / 1024 / 1024 / 1024 << " GB" << std::endl;
                // std::cout << "Node " << node << " available size: " << memoryFree / 1024 / 1024 / 1024 << " GB" << std::endl;
        }
        numa_bitmask_free(nodeMask);
		return ;
}

void getCpuTime(const std::string& statLine, std::vector<int>& cpuTotalTime, std::vector<int>& cpuIdleTime){
		// std::cout << statLine << std::endl;
		std::istringstream iss(statLine);
		CPUInfo cInfo;
		int sum = 0;
		iss >> cInfo.cpuNum >> cInfo.user >> cInfo.nice >> cInfo.system >> cInfo.idle >> cInfo.iowait >> cInfo.irq >> cInfo.softirq >> cInfo.steal >> cInfo.guest >> cInfo.guest_nice;
		sum = cInfo.user + cInfo.nice + cInfo.system + cInfo.idle + cInfo.iowait + cInfo.irq + cInfo.softirq + cInfo.steal + cInfo.guest + cInfo.guest_nice;
		cpuIdleTime.push_back(cInfo.idle);
		cpuTotalTime.push_back(sum);
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
		mvprintw(1, 1, "Cores Count is ");
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

void drawCPUInfo(int start, int tpy){
		mvprintw(start, 1, "-------------");
		attron(COLOR_PAIR(1));
		printw("CPU Utilization"); 
		attroff(COLOR_PAIR(1));
		switch (tpy) {
				case 2:
						printw("(Maximum : ");
						attron(COLOR_PAIR(1));
						printw("%d%%", totalCores / (maxNodes + 1) * 100);
						attroff(COLOR_PAIR(1));
						break;
				case 4:
						printw("(Maximum : ");
						attron(COLOR_PAIR(1));
						printw("100%%");
						attroff(COLOR_PAIR(1));
						break;
				default:
						printw("(Maximum : ");
						attron(COLOR_PAIR(1));
						printw("%d%%", totalCores / (maxNodes + 1) * 100);
						attroff(COLOR_PAIR(1));
						break;
		}
		printw(")-------------");
}

void drawMemInfo(int start){
		mvprintw(start, 1, "-------------");
        attron(COLOR_PAIR(3));
        printw("MEM Utilization");
        attroff(COLOR_PAIR(3));
		printw("(Maximum : ");
		attron(COLOR_PAIR(3));
        printw("100%%");
        attroff(COLOR_PAIR(3));
        printw(")-------------");
}

void drawMemProgressBar(int y, int x, int start, int end, const std::vector<float>& data, int col, int typ){
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
						if (typ == 3) {
								fillPercent = 1;
						}
						break;
				case 4:
						fillPercent = 1;
						break;
				default:
						fillPercent = totalCores / maxNodes;
						break;
		}
		int tmpBlock = 0;
		if (start != 0) tmpBlock = (maxNodes + 1) / 2;
		for (auto it = startIterator; it != endIterator; it++) {
				float percentage = *it;
				float fill_width = bar_width * (percentage / fillPercent);
				mvprintw(y + 1 + counter, x + 1, "Node %d", start);
		        attron(COLOR_PAIR(typ));
        		mvprintw(y + 1 + counter, x + 9, "[");
                for (int i = 0; i < fill_width; ++i) {
						mvprintw(y + 1 + counter, x + i + 10, "|");
		        }
        		mvprintw(y + 1 + counter, x + bar_width + 10, "]");
                attroff(COLOR_PAIR(typ));
		        mvprintw(y + 1 + counter, x + bar_width + 1 + 10, " %.1f%%", percentage * 100);
				counter++;
				start++;
				tmpBlock++;
		}
		return ;
}

void drawProgressBar(int y, int x, int start, int end, const std::vector<float>& data, int col, int typ){
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
						if (typ == 3) {
								fillPercent = 1;
						}
						break;
				case 4:
						fillPercent = 1;
						break;
				default:
						fillPercent = totalCores / maxNodes;
						break;
		}
		int tmpBlock = 0;
		if (start != 0) tmpBlock = (maxNodes + 1) / 2;
		for (auto it = startIterator; it != endIterator; it++) {
				float percentage = *it;
				float fill_width = bar_width * (percentage / fillPercent);
				if (isPerNode){
						if (tmpBlock == nodesImage[drawRow][drawCol]){
								attron(COLOR_PAIR(5));
								mvprintw(y + 1 + counter, x + 1, "Node %d", start);
								attroff(COLOR_PAIR(5)); 
                				attron(COLOR_PAIR(4));
		        		        mvprintw(y + 1 + counter, x + 9, "[");
				                for (int i = 0; i < fill_width; ++i) {
                		        		mvprintw(y + 1 + counter, x + i + 10, "|");
                				}
								for (int i = fill_width; i < bar_width; ++i) {
										mvprintw(y + 1 + counter, x + i + 10, " ");
								}
		                		mvprintw(y + 1 + counter, x + bar_width + 10, "]");
        				        attroff(COLOR_PAIR(4));
								attron(COLOR_PAIR(5));
		                		mvprintw(y + 1 + counter, x + bar_width + 1 + 10, " %.1f%%", percentage * 100);
								attroff(COLOR_PAIR(5));
						} else {
								mvprintw(y + 1 + counter, x + 1, "Node %d", start);
								attron(COLOR_PAIR(typ));
								mvprintw(y + 1 + counter, x + 9, "[");
								for (int i = 0; i < fill_width; ++i) {
										mvprintw(y + 1 + counter, x + i + 10, "|");
								}
								mvprintw(y + 1 + counter, x + bar_width + 10, "]");
								attroff(COLOR_PAIR(typ));
								mvprintw(y + 1 + counter, x + bar_width + 1 + 10, " %.1f%%", percentage * 100);
						}
				} else {
						mvprintw(y + 1 + counter, x + 1, "Node %d", start);
		                attron(COLOR_PAIR(typ));
        		        mvprintw(y + 1 + counter, x + 9, "[");
                		for (int i = 0; i < fill_width; ++i) {
                        		mvprintw(y + 1 + counter, x + i + 10, "|");
		                }
        		        mvprintw(y + 1 + counter, x + bar_width + 10, "]");
                		attroff(COLOR_PAIR(typ));
		                mvprintw(y + 1 + counter, x + bar_width + 1 + 10, " %.1f%%", percentage * 100);
				}
				counter++;
				start++;
				tmpBlock++;
		}
		return ;
}

void updateData(){
		std::vector<int> oldCpuTotalTime(totalCores, 0);
        std::vector<int> oldCpuIdleTime(totalCores, 0);
        std::vector<int> cpuTotalTime;
        std::vector<int> cpuIdleTime;
		while (!exitFlag) {
				getNodeMemory();
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
														isPerNode = false;
                                                        cvModeFlag.notify_all();
                                                        break;
                                                }
										case 10:  // 回车
												{
                                                        std::unique_lock<std::mutex> lock(mutexModeFlag);
                                                        currentMode = 2;
                                                        isPerNode = false;
                                                        cvModeFlag.notify_all();
                                                        break;
                                                }
										case 27:
												{
														if (currentMode == 1) break;
														timeout(100);  // 设置一个短暂的超时等待更多字符
														int nextChar = getch();  // 获取下一个字符
														timeout(-1);  // 恢复正常等待时间
														if (nextChar == '[') {
																int thirdChar = getch();
																if (!isPerNode) {
																		isPerNode = true;
																		drawRow = 0;
																		drawCol = 0;
																} else {
																		switch (thirdChar) {
																				case 'A':  // 上
																						if (drawRow > 0) drawRow--;
																						break;
																				case 'B':  // 下
																						if (drawRow < (maxNodes + 1) / 2 - 1) drawRow++;
																						break;
																				case 'C':  // 右
																						if (drawCol < 1) drawCol++;
																						break;
																				case 'D':  // 左
																						if (drawCol > 0) drawCol--;
																						break;
																		}
																}
																{
																		std::unique_lock<std::mutex> lock(mutexModeFlag);
																		currentMode = 0;
																		cvModeFlag.notify_all();
																}
														}
														break;
												}
										default:
												break;
								}
						}
        		}
		}
}
