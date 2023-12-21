#include <vector>
#include <thread>
#include <numa.h>
#include <termios.h>
#include <ncurses.h>
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
        // init_pair(3, COLOR_MAGENTA, COLOR_BLACK);  // 进度条的颜色设置
        wbkgd(stdscr, COLOR_PAIR(2));
        clear();
        refresh();

        std::thread dataUpdateThread(updateData, std::ref(exitFlag), std::ref(cpuUtilizationPerNode));
        std::thread inputCheckThread(checkInput, std::ref(exitFlag));
        while (!exitFlag) {
                clear();
                drawDetails();
                drawProgressBar(2, 2, "Left Progress", 0, (maxNodes + 1) / 2, cpuUtilizationPerNode);
                drawProgressBar(2, getmaxx(stdscr) / 2 + 2, "Right Progress", (maxNodes + 1) / 2, maxNodes + 1, cpuUtilizationPerNode);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                refresh();
        }
        exitFlag = true;
        dataUpdateThread.join();
        inputCheckThread.join();
        timeout(-1);
        endwin();
        return 0;
}
