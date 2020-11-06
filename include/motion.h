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
    virtual void Start(std::vector<Person> person, bool is_used) = 0;

protected:
    Person* FindPerson();
protected:
    CarController *controller_;
    int target_id_ = -1;
    bool lost_target_ = true;
    std::vector<Person> person_;
    Person *tp_;

};

class Motion2D:Motion{
    void Start(std::vector<Person> person, bool is_used) override;


};




#endif //ROOKIE_MOTION_H
