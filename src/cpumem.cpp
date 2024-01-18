#include <global.hpp>

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
        int coresCount = 0;
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


