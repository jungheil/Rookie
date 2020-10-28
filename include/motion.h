//
// Created by li on 26/10/2020.
//
#ifndef ROOKIE_MOTION_H
#define ROOKIE_MOTION_H
#include "common.h"
#include "usart.h"

class CarController{
public:
    enum CARCONTROL_LINEAR{
        CARCONTROL_LINEAR_FORWARD   =   0,
        CARCONTROL_LINEAR_BACKWARD  =   1
    };
    enum CARCONTROL_ANGULAR{
        CARCONTROL_ANGULAR_LEFT     =   0,
        CARCONTROL_ANGULAR_RIGHT    =   1
    };
    void Move(CARCONTROL_LINEAR,unsigned char linear_velocity, CARCONTROL_ANGULAR,unsigned char angular_velocity);
    inline void set_enable_(bool enable){enable_=enable;};
private:
    Usart usart;
    bool enable_ = true;
};

class Motion {
public:
    explicit Motion(CarController *controller):controller_(controller){};
    inline void set_target(int id){target_id_ = id;};
    virtual void Update(std::vector<Person> person)=0;
private:
    Person* FindPerson();
private:
    CarController *controller_;
    int target_id_ = -1;
    bool lost_target_ = true;
};

class Motion2D:Motion{
    void Update(std::vector<Person> person) override;


};




#endif //ROOKIE_MOTION_H
