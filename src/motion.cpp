//
// Created by li on 26/10/2020.
//

#include "motion.h"
#include <algorithm>
//bool CarController::Move(CARCONTROL_LINEAR linear,
//                         unsigned char linear_velocity,
//                         CARCONTROL_ANGULAR angular,
//                         unsigned char angular_velocity) {
//    unsigned char data[2] = {128, 128};
//    if (enable_){
//        switch(linear){
//            case CARCONTROL_LINEAR_ZERO:
//                data[0] = 128;
//                break;
//            case CARCONTROL_LINEAR_FORWARD:
//                data[0] = 128 - linear_velocity/2;
//                break;
//            case CARCONTROL_LINEAR_BACKWARD:
//                data[0] = 128 + linear_velocity/2;
//        }
//        switch (angular) {
//            case CARCONTROL_ANGULAR_ZERO:
//                data[1] = 128;
//                break;
//            case CARCONTROL_ANGULAR_LEFT:
//                data[1] = 128 - angular_velocity/2;
//                break;
//            case CARCONTROL_ANGULAR_RIGHT:
//                data[1] = 128 + angular_velocity/2;
//                break;
//        }
//    }
//    int len = usart.UsartSend(data);
//    usleep(500);
//    return len != -1;
//}

//bool CarController::Move(unsigned char data1=128, unsigned char data2=128){
//    unsigned char data[2] = {data1, data2};
//    int len = usart.UsartSend(data);
//    usleep(500);
//    return len != -1;
//}

bool CarController::Move(uint16_t distance,uint16_t angle=9000){
    //cout<<endl<<"angle:" << (float)angle/100-90 << endl<<"distance: "<<(float)distance/100<<endl;
    //TODO:Pheyeon-Add binary shift     (finished)
    uint16_t data[2] = {distance, angle};
    unsigned char buff[30]={};
    buff[0]=0x01;
    buff[1]=(data[0]&0xFF00)>>8;
    buff[2]=data[0]&0x00FF;
    buff[3]=(data[1]&0xFF00)>>8;
    buff[4]=data[1]&0x00FF;
    buff[5]=0x00FF;
    int len=0;
    for(uint8_t loop_num=0;loop_num<6;loop_num++)len = usart.UsartSend(&buff[loop_num]);
//    int len = usart.UsartSend(data);
    usleep(500);
    return len != -1;
}

void CarController::SetEnable(bool enable) {
    enable_ = enable;
    if(!enable){
        unsigned char data[2] = {128,128};
        usart.UsartSend(data);
        usleep(500);
    }
}

Person* Motion::FindPerson() {
    for (Person &ap: person_){
        //cout<< "id: "<<ap.get_id()<<endl;
        if(ap.get_id() == target_id_){
            return &ap;
        }
//        return &ap;
    }
    return nullptr;
}

void Motion3D::Move(bool stop) {
    tp_ = FindPerson();
    lost_target_ = tp_ == nullptr;

    if(stop){
//        controller_->Move(CarController::CARCONTROL_LINEAR_ZERO,128,CarController::CARCONTROL_ANGULAR_ZERO,128);
        controller_->Move((uint16_t)0);//TODO:Pheyon-Send (0,0) to stop the machine(finished)
    }else if(lost_target_){
        if(loss_delay_++ > 100){
//            controller_->Move(CarController::CARCONTROL_LINEAR_ZERO,128,CarController::CARCONTROL_ANGULAR_ZERO,128);
            controller_->Move((uint16_t)0);//TODO:Pheyon-Send (0,0) to stop the machine(finished)
        }
        //cout << "lost_target " << target_id_<<endl;
    }else if(!is_control_ ){
        float distance = 0;
        float angle = 0;
        loss_delay_ = 0;
        distance = tp_->get_distance();
        angle = atan2(tp_->get_located().x,tp_->get_located().z);

        is_control_ = controller_->Move((uint16_t)(distance*100),(uint16_t)(angle*5729.577951+9000));//TODO:Pheyeon-add uart(finished)
//        CarController::CARCONTROL_LINEAR cc_linear;
//        CarController::CARCONTROL_ANGULAR cc_angular;
//        if(distance>1.7) cc_linear=CarController::CARCONTROL_LINEAR_FORWARD;
//        else if(distance < 1.2) cc_linear=CarController::CARCONTROL_LINEAR_BACKWARD;
//        else cc_linear = CarController::CARCONTROL_LINEAR_ZERO;
//
//        if(angle > 0.1) cc_angular = CarController::CARCONTROL_ANGULAR_RIGHT;
//        else if(angle < 0.1) cc_angular = CarController::CARCONTROL_ANGULAR_LEFT;
//        else cc_angular = CarController::CARCONTROL_ANGULAR_ZERO;
//
//       is_control_ = controller_->Move(cc_linear,
//                          100+(abs(distance - 1.45)/3<1 ? abs(distance - 1.45)/3 : 1)*155,
//                          cc_angular,
//                          50 + abs(angle)/3.14*200);
//TODO:Pheyeon-Start
//        unsigned char v_linear = 128;
//        unsigned char v_angular = 128;
//        float tar_distance = 0.8;//safety distance
//        if(distance >= tar_distance){
//            //if (abs(last_distance - distance) < 0.5){
//            if (false){
//                unsigned char temp_v = 128-((distance-1)/2*127) + (distance - last_distance)*50;
//                v_linear = 128 - (temp_v > 127 ? 127 : temp_v);
//            }else{
//                v_linear = distance > 3 ? 1:(128-((distance-tar_distance)/(3-tar_distance)*127));
//            }
//        }else{
//            v_linear = 128+77+(tar_distance-distance)/tar_distance*50;
//        }
//        int sign = angle > 0 ? 1:-1;
//        if (abs(angle) < 0.04){
//            v_angular = 128;
//        }else{
//            v_angular = 128 + (abs(angle)>0.3?sign:angle/0.3)*127;
//        }
//        //cout<<(int)v_linear<<", "<<(int)v_angular<<endl;
//        is_control_ = controller_->Move(v_linear,v_angular);
//TODO:Pheyeon-End,remove the control programme to the PLC
    }
}