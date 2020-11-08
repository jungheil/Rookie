#include "Ctracker.h"

size_t CTrack::NextTrackID=0;
Scalar Colors[] = { Scalar(255,0,0),
                    Scalar(0,0,255),
                    Scalar(255,255,0),
                    Scalar(0,255,255),
                    Scalar(255,0,255),
                    Scalar(255,127,255),
                    Scalar(127,0,255),
                    Scalar(127,0,127) };
// ---------------------------------------------------------------------------
// Track constructor.
// The track begins from initial point (pt)
// ---------------------------------------------------------------------------
CTrack::CTrack(Point2f pt, float dt, float Accel_noise_mag)
{
    track_id=NextTrackID;
    NextTrackID++;
    // Every track have its own Kalman filter,
    // it user for next point position prediction.
    KF = new TKalmanFilter(pt,dt,Accel_noise_mag);
    // Here stored points coordinates, used for next position prediction.
    prediction=pt;
    skipped_frames=0;
}
// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
CTrack::~CTrack()
{
    // Free resources.
    delete KF;
}

// ---------------------------------------------------------------------------
// Tracker. Manage tracks. Create, remove, update.
// ---------------------------------------------------------------------------
CTracker::CTracker(float _dt, float _Accel_noise_mag, double _dist_thres, int _maximum_allowed_skipped_frames,int _max_trace_length)
{
    dt=_dt;
    Accel_noise_mag=_Accel_noise_mag;
    dist_thres=_dist_thres;
    maximum_allowed_skipped_frames=_maximum_allowed_skipped_frames;
    max_trace_length=_max_trace_length;
}
// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
void CTracker::Update(std::vector<Person> &person)
{
    // -----------------------------------
    // If there is no tracks yet, then every point begins its own track.
    // -----------------------------------
    if(tracks.size()==0)
    {
        // If no tracks yet
        for(int i=0;i<person.size();i++)
        {
            CTrack* tr=new CTrack(person[i].get_box_center(),dt,Accel_noise_mag);
            tracks.push_back(tr);
        }
    }

    int N=tracks.size();		// the number of tracks
    int M=person.size();	// the number of points detected

    // Matrix distance from track N-th to point detected M-th
    vector< vector<double> > Cost(N,vector<double>(M));
    vector<int> assignment; // matrix used to determine N-th track will be join with point detected M-th based on Hungarian algorithm

    // matrix distance
    double dist;
    for(int i=0;i<tracks.size();i++)
    {
        for(int j=0;j<person.size();j++)
        {
            Point2d diff=(tracks[i]->prediction-person[j].get_box_center());
            //euclid distance
            dist=sqrtf(diff.x*diff.x+diff.y*diff.y);
            Cost[i][j]=dist;
        }
    }
    // -----------------------------------
    // Solving assignment problem (tracks and predictions of Kalman filter)
    // -----------------------------------
    //AssignmentProblemSolver APS;
    //APS.Solve(Cost,assignment,AssignmentProblemSolver::optimal);
    Hungarian APS;
    APS.Solve(Cost, assignment);
    // -----------------------------------
    // clean assignment from pairs with large distance
    // -----------------------------------
    // Not assigned tracks
    for(int i=0;i<assignment.size();i++)
    {
        if(assignment[i] >= 0 && assignment[i] < M)
        {
            if(Cost[i][assignment[i]] > dist_thres)
            {
                assignment[i]=-1;
                // Mark unassigned tracks, and increment skipped frames counter,
                // when skipped frames counter will be larger than threshold, track will be deleted.
                tracks[i]->skipped_frames++;
            }
        }
        else
        {
            // If track have no assigned detect, then increment skipped frames counter.
            tracks[i]->skipped_frames++;
        }
    }

    // -----------------------------------
    // If track didn't get detects long time, remove it.
    // -----------------------------------
    for(int i=0;i<tracks.size();i++)
    {
        if(tracks[i]->skipped_frames > maximum_allowed_skipped_frames)
        {
            delete tracks[i];
            tracks.erase(tracks.begin()+i);
            assignment.erase(assignment.begin()+i);
            i--;
        }
    }
    // -----------------------------------
    // Search for unassigned detects
    // -----------------------------------
    vector<int> not_assigned_detections;
    vector<int>::iterator it;
    for(int i=0;i<person.size();i++)
    {
        it=find(assignment.begin(), assignment.end(), i);
        if(it==assignment.end())
        {
            not_assigned_detections.push_back(i);
        }
    }

    // -----------------------------------
    // and start new tracks for them.
    // -----------------------------------
    if(not_assigned_detections.size()!=0)
    {
        for(int i=0;i<not_assigned_detections.size();i++)
        {
            CTrack* tr=new CTrack(person[not_assigned_detections[i]].get_box_center(),dt,Accel_noise_mag);
            tracks.push_back(tr);
        }
    }

    // Update Kalman Filters state

    for(int i=0;i<assignment.size();i++)
    {
        // If track updated less than one time, than filter state is not correct.

        tracks[i]->KF->GetPrediction();

        if(assignment[i] >= 0 && assignment[i] < M) // If we have assigned detect, then update using its coordinates,
        {
            tracks[i]->skipped_frames=0;
            tracks[i]->prediction=tracks[i]->KF->Update(person[assignment[i]].get_box_center(), 1);
            person[assignment[i]].set_id(i);
        }
        else				  // if not continue using predictions
        {
            tracks[i]->prediction=tracks[i]->KF->Update(Point2f(0,0),0);
        }

        if(tracks[i]->trace.size()>max_trace_length)
        {
            tracks[i]->trace.erase(tracks[i]->trace.begin(),tracks[i]->trace.end()-max_trace_length);
        }

        tracks[i]->trace.push_back(tracks[i]->prediction);
        tracks[i]->KF->LastResult=tracks[i]->prediction;
    }

}
// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
CTracker::~CTracker(void)
{
    for(int i=0;i<tracks.size();i++)
    {
        delete tracks[i];
    }
    tracks.clear();
};
void CTracker::draw_person(cv::Mat &src, std::vector<Person> &person){
    for(int i=0;i<this->tracks.size();i++){
        if(this->tracks[i]->trace.size()>1){
//            string str = to_string(i);
//            int num = this->tracks[i]->trace.size();
//            cv::putText(src, // 图像矩阵
//                        str,                  // string型文字内容
//                        Point(this->tracks[i]->trace[num - 1].x,
//                              this->tracks[i]->trace[num - 1].y),           // 文字坐标，以左下角为原点
//                        cv::FONT_HERSHEY_SIMPLEX,   // 字体类型
//                        3, // 字体大小
//                        cv::Scalar(0, 255, 255));
            for (int j = 0; j < this->tracks[i]->trace.size() - 1; j++) {
                Point p1 = Point(this->tracks[i]->trace[j].x, this->tracks[i]->trace[j].y);
                Point p2 = Point(this->tracks[i]->trace[j + 1].x, this->tracks[i]->trace[j + 1].y);
                cv::line(src,
                         p1,
                         p2,
                         Colors[this->tracks[i]->track_id % 8], 2, LINE_AA);
            }
        }
    }
    for(int i =0;i<person.size();i++){
        string str = to_string(person[i].get_id());
        cv::putText(src, // 图像矩阵
                    str,                  // string型文字内容
                    person[i].get_box_center(),           // 文字坐标，以左下角为原点
                    cv::FONT_HERSHEY_SIMPLEX,   // 字体类型
                    5, // 字体大小
                    cv::Scalar(255, 255, 255));
    }
}