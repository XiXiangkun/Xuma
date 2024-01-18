#include <global.hpp>
#include <process.hpp>
#include <cpumem.hpp>

void updateData(){
		std::vector<int> oldCpuTotalTime(totalCores, 0);
        std::vector<int> oldCpuIdleTime(totalCores, 0);
        std::vector<int> cpuTotalTime;
        std::vector<int> cpuIdleTime;
		while (!exitFlag) {
				if (currentMode == 0 || currentMode == 1) {
						getNodeMemory();
						catProcStat(oldCpuTotalTime, oldCpuIdleTime, cpuTotalTime, cpuIdleTime);
				} else if (currentMode == 2) {
						getProcessInfo();
				} else {
						getNodeMemory();
                        catProcStat(oldCpuTotalTime, oldCpuIdleTime, cpuTotalTime, cpuIdleTime);
				}
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
														if (currentMode == 1 || currentMode == 2) break;
                                                        std::unique_lock<std::mutex> lock(mutexModeFlag);
                                                        currentMode = 2;
                                                        isPerNode = false;
                                                        cvModeFlag.notify_all();
                                                        break;
                                                }
										case 27:
												{
														// if (currentMode == 1 || currentMode == 2) break;
														timeout(100);  // 设置一个短暂的超时等待更多字符
														int nextChar = getch();  // 获取下一个字符
														timeout(-1);  // 恢复正常等待时间
														if (nextChar == '[') {
																int thirdChar = getch();
																if (currentMode == 1 || currentMode == 2) break;
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
