#include "NTracker.h"
#include "KM.h"
Scalar colors[] = { Scalar(255,0,0),
                    Scalar(0,0,255),
                    Scalar(255,255,0),
                    Scalar(0,255,255),
                    Scalar(255,0,255),
                    Scalar(255,127,255),
                    Scalar(127,0,255),
                    Scalar(127,0,127) };
NTracker::NTracker(Point2f pt, float dt, float Accel_noise_mag) {
    tracker_id=NextTrackID;
    NextTrackID++;
    KF = new TKalmanFilter(pt,dt,Accel_noise_mag);
    prediction=pt;
    skipped_frames=0;
    distribution = false;
}
NTracker::~NTracker() {
    delete KF;
}
void NTracker::compute(Person &person) {
    float left = person.get_box().x;
    float top = person.get_box().y;
    float right = person.get_box().x+person.get_box().width;
    float bottom = person.get_box().y+person.get_box().height;
    right_bottom=Point2f (right,bottom);
    left_top=Point2f (left,top);
}
Tracker_Set::Tracker_Set(float _dt, float _Accel_noise_mag, double _dist_thres, int _maximum_allowed_skipped_frames,int _max_trace_length) {
    dt=_dt;
    Accel_noise_mag=_Accel_noise_mag;
    dist_thres=_dist_thres;
    maximum_allowed_skipped_frames=_maximum_allowed_skipped_frames;
    max_trace_length=_max_trace_length;
    NextTrackID=0;
}
void Tracker_Set::Update(std::vector<Person> &person) {
    cv::Mat map(400,400,CV_8UC1,Scalar(0));
//    Draw_map(person);
    //无历史跟踪器则初始化
    if(tracks.size()==0){
        for(int i=0;i<person.size();i++)
        {
           NTracker* tr=new NTracker(person[i].get_located_xz(),dt,Accel_noise_mag);
           cout<<"111111"<<"\n";
            tr->hog.Init(person[i].get_mat());
            tr->hs.Init(person[i].get_mat());
            tracks.push_back(tr);
        }
    }else{
        for (int i = 0; i < tracks.size(); ++i) {
            tracks[i]->distribution= false;
        }
    }
    int T = tracks.size();
    int D = person.size();

    vector< vector<double> > Cost(D,vector<double>(T));
    vector< vector<double> > HOG(D,vector<double>(T));
    vector< vector<double> > HS(D,vector<double>(T));

    vector<int> assignment(D);//匹配结果
    double dist;
    for(int i=0;i<person.size();i++)
    {
        for(int j=0;j<tracks.size();j++)
        {
            Point2d diff=(tracks[j]->prediction-person[i].get_located_xz());
            //euclid distance
            dist=sqrtf(diff.x*diff.x+diff.y*diff.y);
            Cost[i][j]=dist;
            HOG[i][j]=tracks[j]->hog.GetLoss(person[i].get_mat());
            HS[i][j]=tracks[j]->hs.GetSimilarity(person[i].get_mat());
        }
    }
    normal_size(HOG);
    normal_size(HS);
    normal_size(Cost);
    data_fusion(Cost,HS,HOG);
    normal_size(Cost);


    KM km;
    km.solve(Cost,assignment);
    int D_matched=0;
    //对未匹配的跟踪器进行标记处理
    for (int i = 0; i < assignment.size(); ++i) {
        if(assignment[i]>=0&&assignment[i]<T){
            tracks[assignment[i]]->distribution=true;
            D_matched++;
            tracks[assignment[i]]->KF->GetPrediction();
            tracks[assignment[i]]->skipped_frames=0;
            tracks[assignment[i]]->prediction=tracks[assignment[i]]->KF->Update(person[i].get_located_xz(), 1);
            tracks[assignment[i]]->hog.Update(person[i].get_mat());
            tracks[assignment[i]]->hog.Update(person[i].get_mat());
            tracks[assignment[i]]->hs.Update(person[i].get_mat());
            tracks[assignment[i]]->compute(person[i]);
            person[i].set_id(tracks[assignment[i]]->tracker_id);
            person[i].set_tracked(true);
        }else{
            NTracker* tr=new NTracker(person[i].get_located_xz(),dt,Accel_noise_mag);
            tr->hog.Init(person[i].get_mat());//更新hog
            tr->hs.Init(person[i].get_mat());//更新hs
            tr->distribution=true;
            tr->existing=true;
            tr->prediction=person[i].get_located_xz();
            tr->compute(person[i]);
            person[i].set_id(tr->tracker_id);
            person[i].set_tracked(true);
            tracks.push_back(tr);

        }
    }
    //删除丢失帧超过上限的跟踪器
    for (int i = 0; i < tracks.size(); ++i) {
        if(!tracks[i]->distribution){
            tracks[i]->skipped_frames++;
            if(tracks[i]->skipped_frames > maximum_allowed_skipped_frames){
                delete tracks[i];
                tracks.erase(tracks.begin()+i);
                continue;
            }
            tracks[i]->KF->GetPrediction();
            tracks[i]->prediction=tracks[i]->KF->Update(Point2f(0,0),0);
        }
        if(tracks[i]->trace.size()>max_trace_length)
        {
            tracks[i]->trace.erase(tracks[i]->trace.begin(),tracks[i]->trace.end()-max_trace_length);
        }
        tracks[i]->trace.push_back(tracks[i]->prediction);
        tracks[i]->KF->LastResult=tracks[i]->prediction;
    }
    Draw_map_by_track(10);
}
Tracker_Set::~Tracker_Set(){
    for(int i=0;i<tracks.size();i++)
    {
        delete tracks[i];
    }
    tracks.clear();
};
void Tracker_Set::draw_person(cv::Mat &src){
    for (int i = 0; i < tracks.size(); ++i) {
        if(tracks[i]->distribution){
            cv::rectangle(src, tracks[i]->left_top, tracks[i]->right_bottom, cv::Scalar(255, 255, 0), 3);
            std::stringstream strStream;
            strStream<<tracks[i]->tracker_id;
            std::string label = strStream.str();
            putText(src, label, cv::Point(tracks[i]->left_top.x,tracks[i]->left_top.y+5 ), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,255,0),1);
        }
    }
}

void Tracker_Set::Draw_map_by_track(int num) {
    cv::Mat map(800,800,CV_8UC1,Scalar(0));
    double H = 800;
    double W = 800;
    for (int i = 0; i < tracks.size(); ++i) {
        if(tracks[i]->distribution){
            for (int j = 0; j < tracks[i]->trace.size(); ++j) {
                if(j==0){
                    cv::circle(map, cv::Point(W/2+(tracks[i]->trace[j].x/5)*W/2, H-(tracks[i]->trace[j].y/5)*H),3, Scalar(255,0,255),-1);
                    continue;
                }
                cv::circle(map, cv::Point(W/2+(tracks[i]->trace[j].x/5)*W/2, H-(tracks[i]->trace[j].y/5)*H),3, colors[0],-1);
                if(j>=1){
                    line(map,cv::Point(W/2+(tracks[i]->trace[j].x/5)*W/2, H-(tracks[i]->trace[j].y/5)*H),cv::Point(W/2+(tracks[i]->trace[j-1].x/5)*W/2, H-(tracks[i]->trace[j-1].y/5)*H),Scalar(255),1);
                }
            }
        }
    }
    line(map,Point2f(5,H-5),Point2f(W-5,H-5),Scalar(255),1);
    line(map,Point2f(W/2,H),Point2f(W/2,0),Scalar(255),1);
    imshow("2d",map);
}



