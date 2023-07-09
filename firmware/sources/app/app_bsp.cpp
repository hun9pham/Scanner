#include "app_bsp.h"

#include "io_cfg.h"

Indicator LEDLIFE;
Indicator LEDFLASH;
Indicator LEDSTATUS;
Indicator LEDDIR;
Indicator LEDFAULT;

Engines ENGINES;

Indicator::Indicator() {

}

Indicator::~Indicator() {

}

void Indicator::initialize(pf_funcCtl ctrlInit, pf_funcCtl ctrlOn, pf_funcCtl ctrlOff) {
    assertParam(ctrlInit != NULL && ctrlOn != NULL && ctrlOff != NULL);
    this->ctrlInit = ctrlInit;
    this->ctrlOn = ctrlOn;
    this->ctrlOff = ctrlOff;

    this->ctrlInit();
    Indicator::OffState();
}

void Indicator::OnState() {
    this->ctrlOn();
    this->indicateState = INDICATOR_BRIGHT;
}

void Indicator::OffState() {
    this->ctrlOff();
    this->indicateState = INDICATOR_DARK;
}

void Indicator::Blinking() {
    if (INDICATOR_BRIGHT == this->indicateState) {
        Indicator::OffState();
    }
    else {
        Indicator::OnState();
    }
}

void Engines::standStill(uint8_t motorPos) {
    if (motorPos == MOTOR_FRONT) {
        this->motor1State = ENGINE_STANDSTILL;
        ctlMotor1State(STOPPING);
    }
    else if (motorPos == MOTOR_REAR) {
        this->motor2State = ENGINE_STANDSTILL;
        ctlMotor2State(STOPPING);
    }
}

void Engines::rotateForward(uint8_t motorPos) {
    if (motorPos == MOTOR_FRONT) {
        this->motor1State = ENGINE_FORWARD;
        ctlMotor1State(ROTATE_LEFT);
    }
    else if (motorPos == MOTOR_REAR) {
        this->motor2State = ENGINE_FORWARD;
        ctlMotor2State(ROTATE_LEFT);
    }
}

void Engines::rotateBackward(uint8_t motorPos) {
    if (motorPos == MOTOR_FRONT) {
        this->motor1State = ENGINE_BACKWARD;
        ctlMotor1State(ROTATE_RIGHT);
    }
    else if (motorPos == MOTOR_REAR) {
        this->motor2State = ENGINE_BACKWARD;
        ctlMotor2State(ROTATE_RIGHT);
    }
}

Engines::Engines() {
    this->motor1State = ENGINE_STANDSTILL;
    this->motor2State = ENGINE_STANDSTILL;
}

Engines::~Engines() {

}

void Engines::initialize() {
    motorsNorInit();
}

void Engines::setOperation(uint8_t oper) {
    switch (oper) {
    case ENGINE_STANDSTILL: {
        standStill(MOTOR_FRONT);
        standStill(MOTOR_REAR);
    }
    break;
    case ENGINE_FORWARD: {
        rotateForward(MOTOR_FRONT);
        rotateForward(MOTOR_REAR);
    }
    break;
    case ENGINE_BACKWARD: {
        rotateBackward(MOTOR_FRONT);
        rotateBackward(MOTOR_REAR);
    }
    break;
    default:
    break;
    }
}

void Engines::setMotorFront(uint8_t oper) {
    switch (oper) {
    case ENGINE_STANDSTILL: standStill(MOTOR_FRONT);
    break;
    case ENGINE_FORWARD: rotateForward(MOTOR_FRONT);
    break;
    case ENGINE_BACKWARD: rotateBackward(MOTOR_FRONT);
    break;
    default:
    break;
    }
}

void Engines::setMotorRear(uint8_t oper) {
    switch (oper) {
    case ENGINE_STANDSTILL: standStill(MOTOR_REAR);
    break;
    case ENGINE_FORWARD: rotateForward(MOTOR_REAR);
    break;
    case ENGINE_BACKWARD: rotateBackward(MOTOR_REAR);
    break;
    default:
    break;
    }
}
