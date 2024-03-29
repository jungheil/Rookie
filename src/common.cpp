//
// Created by li on 2020/6/12.
//

#include "common.h"
#include <sys/time.h>

using namespace std;
using namespace cv;

Ximg& Ximg::operator=(Ximg &ximg){
    cam_ = ximg.cam_;
    rs_intrinsics_ = ximg.rs_intrinsics_;
    is_used_ = ximg.is_used_;
    header_ = ximg.header_;
    cv_color_ = ximg.cv_color_.clone();
    cv_depth_ = ximg.cv_depth_.clone();
    rs_depth_ = ximg.rs_depth_;
    pitch_ = ximg.pitch_;
    ximg.is_used_ = true;
    return *this;
}

Ximg& Ximg::operator=(const Ximg &ximg){
    cam_ = ximg.cam_;
    rs_intrinsics_ = ximg.rs_intrinsics_;
    is_used_ = ximg.is_used_;
    header_ = ximg.header_;
    cv_color_ = ximg.cv_color_.clone();
    cv_depth_ = ximg.cv_depth_.clone();
    rs_depth_ = ximg.rs_depth_;
    pitch_ = ximg.pitch_;
    return *this;
}


cv::Point3f Pixel2Point(Ximg img, cv::Point pixel){
    float _point[3];
    float _pixel[2] = {float(pixel.x),float(pixel.y)};
    rs2_deproject_pixel_to_point(_point,&img.rs_intrinsics_,_pixel,img.get_rs_depth().get_distance(_pixel[0],_pixel[1]));
    return cv::Point3f(_point[0],_point[1],_point[2]);
}

cv::Point3f Pixel2Point(Ximg img, cv::Point pixel, float distance){
    float _point[3];
    float _pixel[2] = {float(pixel.x),float(pixel.y)};
    rs2_deproject_pixel_to_point(_point,&img.rs_intrinsics_,_pixel,distance);
    return cv::Point3f(_point[0],_point[1],_point[2]);
}



Header::Header(uint64_t seq):seq_(seq) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    stamp_=tv.tv_sec*1E6+tv.tv_usec;
}

void DrawPred(cv::Mat &src, std::vector<Person> person)
{
    for(size_t i = 0; i<person.size(); i++){
        int left = person[i].get_box().x;
        int top = person[i].get_box().y;
        int x = person[i].get_box().x+0.5*person[i].get_box().width;
        int y = person[i].get_box().y+0.5*person[i].get_box().height;
        int right = person[i].get_box().x+person[i].get_box().width;
        int bottom = person[i].get_box().y+person[i].get_box().height;
        //Draw a rectangle displaying the bounding box
        cv::circle(src, cv::Point(x, y),3, cv::Scalar(159, 237, 123),-1);
        cv::rectangle(src, cv::Point(left, top), cv::Point(right, bottom), cv::Scalar(159, 237, 123), 2);
        //Get the label for the class name and its confidence
        std::stringstream ss;
        int baseline;

        std::string temp_id = "";
        temp_id +=std::to_string(person[i].get_id());
        cv::Point temp_point = cv::Point(left, top);
        temp_point.y +=5;
        //行人检测时左上角标号绘制

        ss<<"ID: "<<temp_id;
        std::string id_text = ss.str();
        cv::Size id_size = cv::getTextSize(id_text, FONT_HERSHEY_PLAIN, 3, 2,&baseline);
        rectangle(src,Rect(person[i].get_box().x+2,person[i].get_box().y+2,id_size.width+20,id_size.height+10),cv::Scalar(87,71,255),-1);
        putText(src, id_text, cv::Point(person[i].get_box().x+12, person[i].get_box().y+id_size.height+7), cv::FONT_HERSHEY_PLAIN , 3, cv::Scalar(255,255,255),2);

        ss.str("");
        ss<<setiosflags(ios::fixed)<<setprecision(2);
        ss<<"Located: ("<<person[i].get_located().x<<", "<<person[i].get_located().y<<", "<<person[i].get_located().z<<")";
        std::string located_text = ss.str();
        cv::Size located_size = cv::getTextSize(located_text, FONT_HERSHEY_PLAIN, 1, 2,&baseline);
        rectangle(src,Rect(person[i].get_box().x+2,person[i].get_box().y+id_size.height+located_size.height+2,located_size.width+20,located_size.height+10),cv::Scalar(2, 165, 255),-1);
        putText(src, located_text, cv::Point(person[i].get_box().x+12, person[i].get_box().y+id_size.height+located_size.height+17), cv::FONT_HERSHEY_PLAIN , 1, cv::Scalar(255,255,255),2);
    }
}

bool RectSafety(cv::Rect &brect, int cols, int rows) {
    cv::Rect out_rect=cv::Rect(0,0,cols,rows);
    brect=brect&out_rect;
    return brect.width != 0 && brect.height != 0;
}

void Draw_map(std::vector<Person> &person){
    cv::Mat map(400,400,CV_8UC1,Scalar(0));
    for (int i = 0; i < person.size(); ++i) {
        cout<<person[i].get_located_xz().x<<","<<person[i].get_located_xz().y<<"\n";
        cv::circle(map, cv::Point(200+(person[i].get_located_xz().x/5)*200, 400-(person[i].get_located_xz().y/5)*400),3, cv::Scalar(159, 237, 123),-1);
    }
    line(map,Point2f(5,395),Point2f(395,395),Scalar(255),1);
    line(map,Point2f(200,0),Point2f(200,400),Scalar(255),1);


    imshow("2d",map);
}

cv::Mat To3Channels(const cv::Mat& binImg)
{
    cv::Mat three_channel = cv::Mat::zeros(binImg.rows,binImg.cols,CV_8UC3);
    std::vector<cv::Mat> channels;
    for (int i=0;i<3;i++)
    {
        channels.push_back(binImg);
    }
    merge(channels,three_channel);
    return three_channel;
}


Coordinate::Coordinate(Coordinate *found, cv::Mat &trans) {
    found_ = found;
    is_base_=false;
    trans_=trans.clone();
}

void GetTransfer(cv::Mat &matrix, float tx=0,float ty=0, float tz=0, float rx=0, float ry=0, float rz=0){
    rx = rx/180*3.1415926;
    ry = ry/180*3.1415926;
    rz = rz/180*3.1415926;

    cv::Mat trans = cv::Mat::zeros(cv::Size(4,4),CV_32FC1);
    trans.at<float>(0,0) = cos(rz)*cos(ry);
    trans.at<float>(0,1) = cos(rz)*sin(ry)*sin(rx)-sin(rz)*cos(rx);
    trans.at<float>(0,2) = cos(rz)*sin(ry)*cos(rx)+sin(rz)*sin(rx);
    trans.at<float>(0,3) = tx;

    trans.at<float>(1,0) = sin(rz)*cos(ry);
    trans.at<float>(1,1) = sin(rz)*sin(ry)*sin(rx)-cos(rz)*cos(rx);
    trans.at<float>(1,2) = sin(rz)*sin(ry)*cos(rx)-cos(rz)*sin(rx);
    trans.at<float>(1,3) = ty;

    trans.at<float>(2,0) = -sin(ry);
    trans.at<float>(2,1) = cos(ry)*sin(rx);
    trans.at<float>(2,2) = cos(ry)*cos(rx);
    trans.at<float>(2,3) = tz;

    trans.at<float>(3,0) = 0;
    trans.at<float>(3,1) = 0;
    trans.at<float>(3,2) = 0;
    trans.at<float>(3,3) = 1;
    trans.copyTo(matrix);
}

cv::Mat Coordinate::get_trans_ (){
    if(this->is_base_==false){
        return this->found_->get_trans_()*trans_;
    }else{
        return trans_;
    }
}
void Person::compute_hog_feature(cv::Mat &src) {
    cv::Mat data;
//    resize(src,data,cv::Size(128, 128), CV_8UC1);//调整样本大小
    cv::cvtColor(src,data,cv::COLOR_BGR2GRAY);
    threshold(data,data,0,255,cv::THRESH_OTSU);//大津法自适应二值化样本
    cv::HOGDescriptor *hog = new cv::HOGDescriptor(cv::Size(128, 128),
                                                   cv::Size(16, 16),
                                                   cv::Size(8, 8),
                                                   cv::Size(8, 8), 9);//创建HOG描述器
    descriptors.clear();
    hog->compute(src, descriptors, cv::Size(1, 1),
                 cv::Size(0, 0)); //Hog特征计算
}

void Person::Update(float distance, const cv::Point3f &located, const cv::Rect &box) {
    distance_ = distance;
    located_ = located;
    box_ = box;
    box_center_ = cv::Point2f(box_.x+0.5*box_.width,box_.y+0.5*box_.height);
    locater_x_z = cv::Point2f(located.x,located.z);
}

void GetMask(Ximg img, cv::Rect box, cv::Mat& out) {
//        rs2::colorizer color_map_(2);
//        cv::Mat img_d(cv::Size(img.get_rs_depth().get_width(), img.get_rs_depth().get_height()),
//                CV_8UC3, (void*)img.get_rs_depth().apply_filter(color_map_).get_data(), cv::Mat::AUTO_STEP);
////        img_d=img_d(person.box_);
//        Mat gray;
//        cvtColor(img_d, gray, COLOR_BGR2GRAY);
//        Mat dst;
//        imshow("1112",gray);
//        waitKey(0);


    float **distance = new float*[box.height];
    float max_dist=0.5;
    float min_dist=6;
    for(int i=0; i<box.height; i++){
        distance[i] = new float[box.width];
        for(int j=0; j<box.width; j++){
            distance[i][j] = img.get_rs_depth().
                    get_distance(box.x+j,box.y+i);
//                if (distance[i][j]<min_dist) min_dist=distance[i][j];
//                if (distance[i][j]>max_dist) max_dist=distance[i][j];
        }
    }

    float breadth = max_dist-min_dist;
    Mat roi(box.height,box.width,CV_8UC1);
    for(int i=0;i<box.height;i++){
        for(int j=0; j<box.width;j++){
            float size=distance[i][j]-min_dist;
            size = size < 0 ? 0 : size;
            size = size > 255 ? 255 : size;
            roi.ptr<uchar>(i)[j]=(distance[i][j]-min_dist)/breadth*255;
        }
    }
//        Mat gray;
//        cvtColor(img.get_cv_color(),gray,COLOR_BGR2GRAY);
//        gray=gray(person.box_);
//        Canny(gray,gray,50,100*2);
//        Canny(roi,roi,50,100*2);
    Mat element10 = getStructuringElement(MORPH_RECT, Size(25, 25));
    Mat element25 = getStructuringElement(MORPH_RECT, Size(25, 25));

//        dilate(gray, gray, element);
//        dilate(roi, roi, element);
//        erode(roi, roi, element);
//        threshold(roi,roi,0,255,THRESH_BINARY|THRESH_OTSU);
    erode(roi, roi, element10);
    dilate(roi, roi, element25);
    Mat dst;
    int reliable=0;
    for(int i=0;i<3;i++){
        Mat fill = To3Channels(roi);
        Rect ccomp;
        floodFill(fill,Point(roi.cols/2,roi.rows*(1+i)/4),Scalar(255,0,0),&ccomp, Scalar(1, 1, 1),Scalar(1, 1, 1));
        inRange(fill,Scalar(255,0,0),Scalar(255,0,0),fill);
        int pixel_sum = sum(fill)[0];
        if(pixel_sum>reliable){
            dst = fill;
            reliable = pixel_sum;
        }
    }
    erode(dst, dst, element25);
    vector<Mat> channels;
    split(dst,channels);
    channels[0].copyTo(out);
    for(int i=0; i<box.height; i++){
        delete [] distance[i];
    }
    delete distance;

}

bool RectSafety(cv::Rect2d &brect, int cols, int rows) {
    cv::Rect2d out_rect=cv::Rect(0,0,cols,rows);
    brect=brect&out_rect;
    return brect.area()!=0;
}
void normal_size(std::vector<std::vector<double>> &data ){
    if(data.size()==1){
        return;
    }
    double max_data=0;
    double min_data=0;
    for(const auto &s:data){
        for(const auto &i:s){
            if(i>max_data){
                max_data =i;
            }
            if(i<min_data){
                min_data =i;
            }
        }
    }
    for( auto &s:data){
        for( auto &i:s){
            i=(i-min_data)/(max_data-min_data);
        }
    }

}
void data_fusion(std::vector<std::vector<double>> &cost,std::vector<std::vector<double>> &hs,std::vector<std::vector<double>> &hog){
    if(cost.empty()){
        return;
    }
    for(int i = 0; i< cost.size();i++){
        for(int j = 0; j<cost[0].size(); j++){
            cost[i][j] = cost[i][j]+hs[i][j]+hog[i][j];
        }
    }
}

ZQRCodeDetector::ZQRCodeDetector() {
    using namespace zbar;
    scanner_.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
}

bool ZQRCodeDetector::Get(Mat &src, vector<QRCode> &objs) {
    using namespace zbar;

    objs.clear();

    // Convert image to grayscale
    Mat imGray;
    cvtColor(src, imGray,COLOR_BGR2GRAY);
    equalizeHist(imGray, imGray);

    // Wrap image data in a zbar image
    Image image(src.cols, src.rows, "Y800", (uchar *)imGray.data, src.cols * src.rows);

    // Scan the image for barcodes and QRCodes
    int n = scanner_.scan(image);

    // Print results
    for(Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol)
    {
        QRCode obj;

        obj.data = symbol->get_data();

        // Obtain location
        for(int i = 0; i< symbol->get_location_size(); i++)
        {
            obj.location.emplace_back(symbol->get_location_x(i),symbol->get_location_y(i));
            obj.center.x += symbol->get_location_x(i);
            obj.center.y += symbol->get_location_y(i);

        }
        obj.center.x /= symbol->get_location_size();
        obj.center.y /= symbol->get_location_size();

        objs.push_back(obj);
    }
    return n;
}
