#ifndef DRAW
#define DRAW

void drawDetails();
void drawProcesses();
void drawCPUInfo(int start, int tpy);
void drawMemInfo(int start);
void drawMemProgressBar(int y, int x, int start, int end, const std::vector<float>& data, int col, int typ);
void drawProgressBar(int y, int x, int start, int end, const std::vector<float>& data, int col, int typ);

#endif
