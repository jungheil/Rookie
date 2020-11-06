//
// Created by li on 26/10/2020.
//

#include "motion.h"
void CarController::Move(CARCONTROL_LINEAR linear,
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
    usart.UsartSend(data);
}

void CarController::SetEnable(bool enable) {
    enable_ = enable;
    if(!enable){
        unsigned char data[2] = {128,128};
        usart.UsartSend(data);
    }
}

Person* Motion::FindPerson() {
    for (Person &ap: person_){
        if(ap.get_id() == target_id_){
            return &ap;
        }
    }
    return nullptr;
}

void Motion3D::Start(std::vector<Person> person, bool is_used) {
    if(!is_used) person_ = person;
    tp_ = FindPerson();
    lost_target_ = tp_ == nullptr;

    if(lost_target_){
        controller_->SetEnable(false);
    }else{
        int distance;
        float angle;
        distance = sqrt(pow(tp_->get_located().x,2)+pow(tp_->get_located().z,2));
        angle = atan2(tp_->get_located().x,tp_->get_located().z);

        CarController::CARCONTROL_LINEAR cc_linear;
        CarController::CARCONTROL_ANGULAR cc_angular;
        if(distance>2) cc_linear=CarController::CARCONTROL_LINEAR_FORWARD;
        else if(distance < 1.5) cc_linear=CarController::CARCONTROL_LINEAR_BACKWARD;
        else cc_linear = CarController::CARCONTROL_LINEAR_ZERO;

        if(angle > 0.1) cc_angular = CarController::CARCONTROL_ANGULAR_RIGHT;
        else if(angle < 0.1) cc_angular = CarController::CARCONTROL_ANGULAR_LEFT;
        else cc_angular = CarController::CARCONTROL_ANGULAR_ZERO;
        controller_->SetEnable(true);
        controller_->Move(cc_linear,200,cc_angular,200);
    }
}