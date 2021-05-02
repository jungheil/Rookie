#include "KM.h"

void KM::matrix_Transform(vector<vector<double>> &DistMatrix) {
    D = DistMatrix.size();
    T = DistMatrix[0].size();
    for (int i = 0; i < D; ++i) {
        for (int j = 0; j < T; ++j) {
            cost[i][j]=round((1-DistMatrix[i][j])*10);
        }
    }
}

bool KM::dfs(int Detect_id) {
    vis_Dectect[Detect_id]= true;
    for (int i = 0; i < T; ++i) {
        if(vis_Track[i]){
            continue;
        }
        int gap = Detect[Detect_id]+Track[i]-cost[Detect_id][i];
        if(gap==0){
            vis_Track[i]= true;
            if(match[i]==-1||dfs(match[i])){
                match[i]=Detect_id;
                return true;
            }
        }else{
            slack[i] = min(slack[i],gap);
        }
    }
    return false;
}
bool KM::DT_dfs(int Detect_id) {
    vis_Dectect[Detect_id]= true;
    for (int i = 0; i < T; ++i) {
        if(vis_Track[i]){
            continue;
        }
        int gap = Detect[Detect_id]+Track[i]-cost[Detect_id][i];
        if(gap==0){
            vis_Track[i]= true;
            if(match[i]==-1||DT_dfs(match[i])){
                match[i]=Detect_id;
                return true;
            }
        }else{
            slack[i] = min(slack[i],gap);
        }
    }
    return false;
}
bool KM::TD_dfs(int Track_id) {
    vis_Track[Track_id]= true;
    for (int i = 0; i < D; ++i) {
        if(vis_Dectect[i]){
            continue;
        }
        int gap = Track[Track_id]+Detect[i]-cost[i][Track_id];
        if(gap==0){
            vis_Dectect[i]= true;
            if(TD_match[i]==-1||TD_dfs(TD_match[i])){
                TD_match[i]=Track_id;
                return true;
            }
        }else{
            slack[i] = min(slack[i],gap);
        }
    }
    return false;
}
void KM::DT_solve(vector<int> &Assignment) {
    memset(match,-1,100);
    memset(Track,0,100);
    for (int & i : Assignment) {
        i=-1;
    }
    //检测器初始期望是与它相连的跟踪器的最大权值
    for (int i = 0; i < D; ++i) {
        Detect[i]=cost[i][0];
        for (int j = 0; j < T; ++j) {
            Detect[i]=max(Detect[i],cost[i][j]);
        }
    }
    for (int i = 0; i < D; ++i) {
        fill(slack,slack+T,INF);//初始化无穷大
        int count=0;
        while (1){
            memset(vis_Dectect,false,D);
            memset(vis_Track, false,T);
            if(DT_dfs(i)){
                count=0;
                break;
            }
            int d = INF;
            for (int j = 0; j < T; ++j) {
                if(!vis_Track[j]){
                    d = min(d,slack[j]);
                }
            }
            for (int j = 0; j < T; ++j) {
                if(vis_Dectect[j]){
                    Detect[j]-=d;
                }
                if(vis_Track[j]){
                    Track[j]+=d;
                }else{
                    slack[j]-=d;
                }
            }
            count++;
            if(count>=5){
                match[i]=-2;
                count=0;
                break;
            }
        }
    }
    for (int i = 0; i < T; ++i) {
        if(match[i]>=0){
            Assignment[match[i]]=i;
        }
    }
}
void KM::TD_solve(vector<int> &Assignment) {
    memset(TD_match,-1,100);
    memset(Detect,0,100);
    //跟踪器初始期望是与它相连的跟踪器的最大权值
    for (int i = 0; i < T; ++i) {
        Track[i]=cost[0][i];
        for (int j = 0; j < D; ++j) {
            Track[i]=max(Track[i],cost[j][i]);
        }
    }
    for (int i = 0; i < T; ++i) {
        fill(slack,slack+D,INF);//初始化无穷大
        int count=0;
        while (1){
            memset(vis_Dectect,false,D);
            memset(vis_Track, false,T);
            if(TD_dfs(i)){
                count=0;
                break;
            }
            int d = INF;
            for (int j = 0; j < T; ++j) {
                if(!vis_Track[j]){
                    d = min(d,slack[j]);
                }
            }
            for (int j = 0; j < T; ++j) {
                if(vis_Dectect[j]){
                    Detect[j]-=d;
                }
                if(vis_Track[j]){
                    Track[j]+=d;
                }else{
                    slack[j]-=d;
                }
            }
            count++;
            if(count>=5){
                TD_match[i]=-2;
                count=0;
                break;
            }
        }
    }
    for (int i = 0; i < D; ++i) {
        Assignment[i]=TD_match[i];
    }
}
void KM::solve(vector<vector<double>> &DistMatrix, vector<int> &Assignment) {
    matrix_Transform(DistMatrix);
    if (D <= T) {
        DT_solve(Assignment);
    } else {
        TD_solve(Assignment);
    }
}
