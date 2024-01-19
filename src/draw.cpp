#include <global.hpp>

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

void drawProcesses() {
    // 定义各列宽度
    const int pidWidth = 10;
    const int virtWidth = 10;
    const int resWidth = 10;
    const int shrWidth = 10;
    const int cpuWidth = 10;
    const int memWidth = 10;
	const int spaceWidth = 5;
    const int nameWidth = 50;
    const int coreWidth = 6;

    attron(COLOR_PAIR(5));
	mvprintw(3, 0, "%*s %*s %*s %*s %*s %*s %*s %-*s %*s",
             pidWidth, "PID", virtWidth, "VIRT", resWidth, "RES",
             shrWidth, "SHR", cpuWidth, "%CPU", memWidth, "%MEM",
			 spaceWidth, "", nameWidth, "NAME", coreWidth, "CORE");
	attroff(COLOR_PAIR(5));
    int row = 4;
    for (const auto& process : processesDisplay) {
        mvprintw(row, 0, "%*d %*d %*d %*d %*.1f %*.1f %*s  %-*s %*d",
                 pidWidth, process.pid, virtWidth, process.virt, resWidth, process.res,
                 shrWidth, process.shr, cpuWidth, process.cpu, memWidth, process.mem,
                 spaceWidth, " ", nameWidth, process.name.c_str(), coreWidth, process.processor);
        row++;
        if (row >= LINES) break;  // 如果行数超过屏幕大小则停止
    }
    return;
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
