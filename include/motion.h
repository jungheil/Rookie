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
        CARCONTROL_LINEAR_ZERO      =   0,
        CARCONTROL_LINEAR_FORWARD   =   1,
        CARCONTROL_LINEAR_BACKWARD  =   2
    };
    enum CARCONTROL_ANGULAR{
        CARCONTROL_ANGULAR_ZERO     =   0,
        CARCONTROL_ANGULAR_LEFT     =   1,
        CARCONTROL_ANGULAR_RIGHT    =   2
    };
    bool Move(CARCONTROL_LINEAR,unsigned char linear_velocity, CARCONTROL_ANGULAR,unsigned char angular_velocity);
    bool Move(unsigned char data1, unsigned char data2);
    void SetEnable(bool enable);
private:
    Usart usart;
    bool enable_ = true;
};

class Motion {
public:
    explicit Motion(CarController *controller):controller_(controller){};
    inline void set_target(int id){target_id_ = id;};
    void UpdatePerson(std::vector<Person> person){person_ = person; is_control_ = false;};
    virtual void Move(bool stop) = 0;

protected:
    Person* FindPerson();
protected:
    CarController *controller_;
    int target_id_ = -1;
    bool lost_target_ = true;
    std::vector<Person> person_;
    Person *tp_;
    bool is_control_ = false;
    int loss_delay_ = 0;
    float last_distance;

};

class Motion3D:public Motion{
public:
    Motion3D(CarController *controller):Motion(controller){};
    void Move(bool stop) override;


};




#endif //ROOKIE_MOTION_H
