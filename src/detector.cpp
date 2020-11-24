////
//// Created by li on 2020/6/12.
////

#include <fstream>
#include "detector.h"
#include <math.h>


using namespace cv;
using namespace dnn;
using namespace std;

namespace XRDetector{
    MLCV::MLCV(){
        // Load the network
        net_ = readNetFromDarknet(modelConfiguration_, modelWeights_);
        net_.setPreferableBackend(DNN_BACKEND_OPENCV);
        net_.setPreferableTarget(DNN_TARGET_CPU);
    }

    void MLCV::Predictor(std::vector<cv::Rect> &boxes, cv::Mat src) {
        Mat frame;
        cv::cvtColor(src, frame, COLOR_BGR2RGB);
        src_size_ = Size(src.cols,src.rows);
        blobFromImage(frame, blob_, 1/255.0,
                      Size(inpWidth_, inpHeight_),
                      Scalar(0,0,0), true, false);

        net_.setInput(blob_);
        vector<Mat> outs;
        net_.forward(outs, GetOutputsNames(net_));
        Process(outs);
        boxes = boxes_;
        boxes_.clear();
    }

    void MLCV::Process(const std::vector<cv::Mat> &outs) {
        vector<float> confidences;
        vector<Rect> boxes;
        for (size_t i = 0; i < outs.size(); ++i)
        {
            // Scan through all the bounding boxes output from the network and keep only the
            // ones with high confidence scores. Assign the box's class label as the class
            // with the highest score for the box.
            float* data = (float*)outs[i].data;
            for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
            {
                Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
                Point classIdPoint;
                double confidence;
                // Get the value and location of the maximum score
                minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
//                if(classIdPoint.x != 0) continue;
                if (confidence > confThreshold_)
                {
                    int centerX = (int)(data[0] * src_size_.width);
                    int centerY = (int)(data[1] * src_size_.height);
                    int width = (int)(data[2] * src_size_.width);
                    int height = (int)(data[3] * src_size_.height);
                    int left = centerX - width / 2;
                    int top = centerY - height / 2;

                    confidences.push_back((float)confidence);
                    boxes.push_back(Rect(left, top, width, height));
                }
            }
        }

        // Perform non maximum suppression to eliminate redundant overlapping boxes with
        // lower confidences
        vector<int> indices;
        NMSBoxes(boxes, confidences, confThreshold_, nmsThreshold_, indices);
        for (size_t i = 0; i < indices.size(); ++i)
        {
            int idx = indices[i];
            boxes_.push_back(boxes[idx]);
            confidences_.push_back(confidences[idx]);
        }
    }

    vector<String> MLCV::GetOutputsNames(const Net& net)
    {
        static vector<String> names;
        if (names.empty())
        {
            //Get the indices of the output layers, i.e. the layers with unconnected outputs
            vector<int> outLayers = net.getUnconnectedOutLayers();

            //get the names of all the layers in the network
            vector<String> layersNames = net.getLayerNames();

            // Get the names of the output layers in names
            names.resize(outLayers.size());
            for (size_t i = 0; i < outLayers.size(); ++i)
                names[i] = layersNames[outLayers[i] - 1];
        }
        return names;
    }

    MLTorch::MLTorch() {
        module_ = torch::jit::load(modelWeights_);
    }

    void MLTorch::Predictor(std::vector<cv::Rect> &boxes, cv::Mat src){
        vector<Rect> outs;
        src_size_ = Size(src.cols,src.rows);
        Mat img;
        cv::resize(src, img, cv::Size(inpWidth_, inpHeight_));
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
        torch::Tensor imgTensor = torch::from_blob(img.data, {img.rows, img.cols,3},torch::kByte);
        imgTensor = imgTensor.permute({2,0,1});
        imgTensor = imgTensor.toType(torch::kFloat);
        imgTensor = imgTensor.div(255);
        imgTensor = imgTensor.unsqueeze(0);

        // preds: [?, 15120, 9]
        torch::Tensor preds = module_.forward({imgTensor}).toTuple()->elements()[0].toTensor();
        std::vector<torch::Tensor> dets = NonMaxSuppression(preds);
        if (dets.size() > 0)
        {
            // Visualize result
            for (size_t i=0; i < dets[0].sizes()[0]; ++ i)
            {
                float left = dets[0][i][0].item().toFloat() * src_size_.width / inpWidth_;
                float top = dets[0][i][1].item().toFloat() * src_size_.height / inpHeight_;
                float right = dets[0][i][2].item().toFloat() * src_size_.width / inpWidth_;
                float bottom = dets[0][i][3].item().toFloat() * src_size_.height / inpHeight_;
                float score = dets[0][i][4].item().toFloat();
                int classID = dets[0][i][5].item().toInt();
                if(classID != 0) continue;
                outs.push_back(cv::Rect(left, top, (right - left), (bottom - top)));

            }
        }
        boxes = outs;
    }

    std::vector<torch::Tensor> MLTorch::NonMaxSuppression(torch::Tensor preds){
        std::vector<torch::Tensor> output;
        for (size_t i=0; i < preds.sizes()[0]; ++i){
            torch::Tensor pred = preds.select(0, i);

            // Filter by scores
            torch::Tensor scores = pred.select(1, 4) * std::get<0>( torch::max(pred.slice(1, 5, pred.sizes()[1]), 1));
            pred = torch::index_select(pred, 0, torch::nonzero(scores > confThreshold_).select(1, 0));
            if (pred.sizes()[0] == 0) continue;

            // (center_x, center_y, w, h) to (left, top, right, bottom)
            pred.select(1, 0) = pred.select(1, 0) - pred.select(1, 2) / 2;
            pred.select(1, 1) = pred.select(1, 1) - pred.select(1, 3) / 2;
            pred.select(1, 2) = pred.select(1, 0) + pred.select(1, 2);
            pred.select(1, 3) = pred.select(1, 1) + pred.select(1, 3);

            // Computing scores and classes
            std::tuple<torch::Tensor, torch::Tensor> max_tuple = torch::max(pred.slice(1, 5, pred.sizes()[1]), 1);
            pred.select(1, 4) = pred.select(1, 4) * std::get<0>(max_tuple);
            pred.select(1, 5) = std::get<1>(max_tuple);

            torch::Tensor  dets = pred.slice(1, 0, 6);

            torch::Tensor keep = torch::empty({dets.sizes()[0]});
            torch::Tensor areas = (dets.select(1, 3) - dets.select(1, 1)) * (dets.select(1, 2) - dets.select(1, 0));
            std::tuple<torch::Tensor, torch::Tensor> indexes_tuple = torch::sort(dets.select(1, 4), 0, 1);
            torch::Tensor v = std::get<0>(indexes_tuple);
            torch::Tensor indexes = std::get<1>(indexes_tuple);
            int count = 0;
            while (indexes.sizes()[0] > 0){
                keep[count] = (indexes[0].item().toInt());
                count += 1;

                // Computing overlaps
                torch::Tensor lefts = torch::empty(indexes.sizes()[0] - 1);
                torch::Tensor tops = torch::empty(indexes.sizes()[0] - 1);
                torch::Tensor rights = torch::empty(indexes.sizes()[0] - 1);
                torch::Tensor bottoms = torch::empty(indexes.sizes()[0] - 1);
                torch::Tensor widths = torch::empty(indexes.sizes()[0] - 1);
                torch::Tensor heights = torch::empty(indexes.sizes()[0] - 1);
                for (size_t i=0; i<indexes.sizes()[0] - 1; ++i)
                {
                    lefts[i] = std::max(dets[indexes[0]][0].item().toFloat(), dets[indexes[i + 1]][0].item().toFloat());
                    tops[i] = std::max(dets[indexes[0]][1].item().toFloat(), dets[indexes[i + 1]][1].item().toFloat());
                    rights[i] = std::min(dets[indexes[0]][2].item().toFloat(), dets[indexes[i + 1]][2].item().toFloat());
                    bottoms[i] = std::min(dets[indexes[0]][3].item().toFloat(), dets[indexes[i + 1]][3].item().toFloat());
                    widths[i] = std::max(float(0), rights[i].item().toFloat() - lefts[i].item().toFloat());
                    heights[i] = std::max(float(0), bottoms[i].item().toFloat() - tops[i].item().toFloat());
                }
                torch::Tensor overlaps = widths * heights;

                // FIlter by IOUs
                torch::Tensor ious = overlaps / (areas.select(0, indexes[0].item().toInt()) + torch::index_select(areas, 0, indexes.slice(0, 1, indexes.sizes()[0])) - overlaps);
                indexes = torch::index_select(indexes, 0, torch::nonzero(ious <= nmsThreshold_).select(1, 0) + 1);
            }
            keep = keep.toType(torch::kInt64);
            output.push_back(torch::index_select(dets, 0, keep.slice(0, 0, count)));
        }
        return output;
    }

    Detector::Detector() {
        R_b_cam = Mat::eye(3,3,CV_32FC1);
        R_b_cam.at<float>(0,0) =cos(10/180*3.1415926);
        R_b_cam.at<float>(0,1) =-sin(10/180*3.1415926);
        R_b_cam.at<float>(1,0) =sin(10/180*3.1415926);
        R_b_cam.at<float>(1,1) =cos(10/180*3.1415926);


    }

    void Detector::UpdatePerson(Ximg img, std::vector<Person> &person){
        person.clear();
        std::vector<cv::Rect> boxes;
        ml.Predictor(boxes, img.get_cv_color());
        for (size_t i = 0; i<boxes.size(); i++){
            RectSafety(boxes[i],img.get_cv_color().rows,img.get_cv_color().cols);
            cv::Point3f point;
            if (img.cam_->GetCamType() == Camera::CAMERA_TYPE_REALSENSE){
                Mat mask;
                GetMask(img,boxes[i],mask);
                float distance=0;
                float dists[16];
                for (int j = 0; j<4;j++){
                    for(int k=0; k<4;k++){
                        if (mask.at<uchar>(mask.rows/5*(1+k),mask.cols/5*(1+j))==255){
                            dists[j*4+k]=img.get_rs_depth().get_distance(mask.cols/5*(1+j)+boxes[i].x,mask.rows/5*(1+k)+boxes[i].y);
                        }else{
                            dists[j*4+k]=0;
                        }
                    }
                }
                int count=0;
                float sum=0;
                for (int i=0; i<16;i++){
                    if (dists[i] !=0){
                        sum+=dists[i];
                        count++;
                    }
                }
                distance=sum/count;
                if(!isnormal(distance) || distance < 0.1 || distance > 8) continue;
//                cout<<"dist:"<<distance<<endl;
                Point pixel = Point(boxes[i].x+0.5*boxes[i].width,boxes[i].y+0.5*boxes[i].height);
                point = Pixel2Point(img, pixel,distance);
                Mat vec = R_b_cam*cv::Vec3f(point.x,point.y,point.z);

                point = Point3f(vec.at<float>(0,0),vec.at<float>(0,1),vec.at<float>(0,2));
                person.push_back(Person(distance,point, boxes[i]));
            }else{
                person.push_back(Person(boxes[i]));
            }
        }
        if(person.size()>0){
            for(int i =0;i<person.size();i++){
                Mat map = img.get_cv_color();
                Mat obj = map(person[i].get_box());
                obj = obj&
                resize(obj,obj,cv::Size(128, 128), CV_8UC1);//调整样本大小
//            imshow("debug",obj);
                person[i].compute_hog_feature(obj);
            }
            if(person.size()>1){
                double dis=0;
                for (int j = 0; j < person[0].descriptors.size(); j++)
                {
                    dis += pow(person[0].descriptors[j]-person[1].descriptors[j], 2);
                }
                cout<<"0和1"<<"相似度"<<dis<<"\n";
            }
        }
        std::vector<cv::Rect>().swap(boxes);
    }

    void Detector::GetMask(Ximg img, Rect box, Mat& out) {
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
}

