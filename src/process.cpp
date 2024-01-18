#include <global.hpp>

int getMemSize() {
        std::string unusedVariable;
        int memSize;
        std::string memPath = "/proc/meminfo";
        std::ifstream memFile(memPath);
        if (memFile.is_open()) {
                std::string memLine;
                std::getline(memFile, memLine);
                std::istringstream iss(memLine);
                iss >> unusedVariable >> memSize;
        }
        return memSize;
}

void getProcessInfo(){
        int totalMemSize = getMemSize();
        // std::map<int, std::pair<long, long>> previousTimes;
        int coresPerNode = totalCores / (maxNodes + 1);
        int targetNode = nodesImage[drawRow][drawCol];
        processesDisplay.clear();
        struct stat st;
        // std::vector<std::string> processes;
        if (stat("/proc", &st) == 0 && S_ISDIR(st.st_mode)) {
                DIR *dir = opendir("/proc");
                if (dir != nullptr) {
                        struct dirent *entry;
                        while ((entry = readdir(dir)) != nullptr) {
                                if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
                                        // processes.push_back(entry->d_name);
                                        std::string pid = entry->d_name;
                                        std::string path = "/proc/" + pid + "/stat";
                                        std::ifstream file(path);
                                        if (file.is_open()) {
                                                std::string line;
                                                std::getline(file, line);
                                                std::istringstream iss(line);
                                                std::vector<std::string> tokens;
                                                std::string token;
                                                while (std::getline(iss, token, ' ')) {
                                                        if (!token.empty()) {
                                                                tokens.push_back(token);
                                                        }
                                                }
                                                int processor = std::stoi(tokens[38]);
                                                long utime = std::stol(tokens[13]);
                                                long stime = std::stol(tokens[14]);
                                                if (previousTimes.find(std::stoi(pid)) == previousTimes.end() || previousTimes[std::stoi(pid)].second != processor) {
                                                        previousTimes[std::stoi(pid)] = {utime + stime, processor};
                                                } else {
                                                        long prevTotalTime = previousTimes[std::stoi(pid)].first;
                                                        float cpuUsage = (float)(utime + stime - prevTotalTime) / (2.0f * sysconf(_SC_CLK_TCK)) * 100.0f;
                                                        previousTimes[std::stoi(pid)] = {utime + stime, processor};
                                                        if (coresPerNode * targetNode <= processor && processor < coresPerNode * (targetNode + 1)) {
                                                        // if (80 <= processor && processor < 88) {
                                                                int virt, res, shr;
                                                                std::string statmPath = "/proc/" + pid + "/statm";
                                                                std::ifstream procStatm(statmPath);
                                                                std::string statmLine;
                                                                std::getline(procStatm, statmLine);
                                                                std::istringstream issM(statmLine);
                                                                issM >> virt >> res >> shr;
                                                                ProcessInfo tmpProcessInfo = {std::stoi(pid), virt * 64, res * 64, shr * 64, cpuUsage, static_cast<float>(res * 64) / totalMemSize, tokens[1], processor};
                                                                processesDisplay.push_back(tmpProcessInfo);
                                                        }
                                                }
                                        }
                                }
                        }
                }
                closedir(dir);
        }
        std::sort(processesDisplay.begin(), processesDisplay.end(), [](const ProcessInfo& a, const ProcessInfo& b){
                return a.cpu > b.cpu;
        });
}

