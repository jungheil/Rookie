//
// Created by li on 26/10/2020.
//

#include "motion.h"
#include <algorithm>
bool CarController::Move(CARCONTROL_LINEAR linear,
                         unsigned char linear_velocity,
                         CARCONTROL_ANGULAR angular,
                         unsigned char angular_velocity) {
    unsigned char data[2] = {128, 128};
    if (enable_){
        switch(linear){
            case CARCONTROL_LINEAR_ZERO:
                data[0] = 128;
                break;
            case CARCONTROL_LINEAR_FORWARD:
                data[0] = 128 - linear_velocity/2;
                break;
            case CARCONTROL_LINEAR_BACKWARD:
                data[0] = 128 + linear_velocity/2;
        }
        switch (angular) {
            case CARCONTROL_ANGULAR_ZERO:
                data[1] = 128;
                break;
            case CARCONTROL_ANGULAR_LEFT:
                data[1] = 128 - angular_velocity/2;
                break;
            case CARCONTROL_ANGULAR_RIGHT:
                data[1] = 128 + angular_velocity/2;
                break;
        }
    }
    int len = usart.UsartSend(data);
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
    }
    return nullptr;
}

void Motion3D::Move() {
    tp_ = FindPerson();
    lost_target_ = tp_ == nullptr;

    if(lost_target_){
        if(loss_delay_++ > 250){
            controller_->Move(CarController::CARCONTROL_LINEAR_ZERO,0,CarController::CARCONTROL_ANGULAR_ZERO,0);
        }
        //cout << "lost_target " << target_id_<<endl;
    }else if(!is_control_){
        loss_delay_ = 0;
        float distance;
        float angle;
        distance = sqrt(pow(tp_->get_located().x,2)+pow(tp_->get_located().z,2));
        angle = atan2(tp_->get_located().x,tp_->get_located().z);
        //<<"distance: "<<distance<<endl;
        //cout<<"angle:" << angle << endl;
        CarController::CARCONTROL_LINEAR cc_linear;
        CarController::CARCONTROL_ANGULAR cc_angular;
        if(distance>1.7) cc_linear=CarController::CARCONTROL_LINEAR_FORWARD;
        else if(distance < 1.2) cc_linear=CarController::CARCONTROL_LINEAR_BACKWARD;
        else cc_linear = CarController::CARCONTROL_LINEAR_ZERO;

        if(angle > 0.1) cc_angular = CarController::CARCONTROL_ANGULAR_RIGHT;
        else if(angle < 0.1) cc_angular = CarController::CARCONTROL_ANGULAR_LEFT;
        else cc_angular = CarController::CARCONTROL_ANGULAR_ZERO;
        int v_l = (abs(distance - 1.45)/3);

       is_control_ = controller_->Move(cc_linear,
                          100+(abs(distance - 1.45)/3<1 ? abs(distance - 1.45)/3 : 1)*155,
                          cc_angular,
                          50 + abs(angle)/3.14*200);
    }
}