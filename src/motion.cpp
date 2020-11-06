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

Person* Motion::FindPerson() {
    for (auto ap: person_){
        if(ap.get_id() == target_id_){
            return &ap;
        }
    }
    return nullptr;
}

void Motion2D::Start(std::vector<Person> person, bool is_used) {
    if(!is_used) person_ = person;
    tp_ = FindPerson();
    lost_target_ = tp_ == nullptr;

    if(lost_target_){

    }else{
        tp_.
    }
}