#ifndef ROOKIE_KM_H
#define ROOKIE_KM_H
#include <vector>
#include <cmath>
#include <cstring>
using namespace std;
const int MAXN=100;//图最大数量
class KM{
private:
    const int INF = 0x3f3f3f3f;
    int flag=0;//0时为检测器少于等于跟踪器，1时为检测器大于跟踪器
    int cost[MAXN][MAXN];//检测器与跟踪器的期望
    int Detect[MAXN];//检测器期望
    int Track[MAXN];//跟踪器期望
    bool vis_Dectect[MAXN];//记录每一轮匹配匹配过的检测器
    bool vis_Track[MAXN];//记录每一轮匹配匹配过的跟踪器
    int match[MAXN];//记录每个跟踪器匹配的检测器，如果没有则为-1
    int TD_match[MAXN];//记录每个检测器匹配的跟踪器，如果没有则为-1
    int slack[MAXN];//记录每个跟踪器如果能被检测器匹配至少还需要多少期望值
    int D;//检测器数量
    int T;//跟踪器数量
public:
    KM(){};
    ~KM(){};
    void matrix_Transform(vector<vector<double>>& DistMatrix);//将double权值矩阵处理为int权值矩阵
    void solve(vector<vector<double>>& DistMatrix,vector<int>& Assignment);//匹配处理
    bool dfs(int Detect_id);//深度优先搜索
    void DT_solve(vector<int> &Assignment);//检测器-跟踪器匹配
    void TD_solve(vector<int> &Assignment);//跟踪器-检测器匹配
    bool DT_dfs(int Detect_id);//检测器-跟踪器dfs
    bool TD_dfs(int Track_id);//跟踪器－检测器dfs
};





#endif //ROOKIE_KM_H
