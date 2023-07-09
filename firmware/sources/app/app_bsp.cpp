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

void Engines::standStill() {
    this->stateControl = ENGINE_STANDSTILL;
    if (viaPWM) {
        ctlMotor1PWM(STOPPING, this->percentPWM);
        ctlMotor2PWM(STOPPING, this->percentPWM);
    }
    else {
        ctlMotor1State(STOPPING);
        ctlMotor2State(STOPPING);
    }
}

void Engines::rotateForward() {
    this->stateControl = ENGINE_FORWARD;
    if (viaPWM) {
        ctlMotor1PWM(ROTATE_LEFT, this->percentPWM);
        ctlMotor2PWM(ROTATE_LEFT, this->percentPWM);
    }
    else {
        ctlMotor1State(ROTATE_LEFT);
        ctlMotor2State(ROTATE_LEFT);
    }
}

void Engines::rotateBackward() {
    this->stateControl = ENGINE_BACKWARD;
    if (viaPWM) {
        ctlMotor1PWM(ROTATE_RIGHT, this->percentPWM);
        ctlMotor2PWM(ROTATE_RIGHT, this->percentPWM);
    }
    else {
        ctlMotor1State(ROTATE_RIGHT);
        ctlMotor2State(ROTATE_RIGHT);
    }
}

Engines::Engines() {
    this->stateControl = ENGINE_STANDSTILL;
}

Engines::~Engines() {

}

void Engines::initialize(bool viaPWM) {
    if (viaPWM) {
        motorsPWMInit();
        this->percentPWM = 0;
    }
    else {
        motorsNorInit();
    }
}

void Engines::setOperation(uint8_t oper) {
    switch (oper) {
    case ENGINE_STANDSTILL: standStill();
    break;
    case ENGINE_FORWARD: rotateForward();
    break;
    case ENGINE_BACKWARD: rotateBackward();
    break;
    default:
    break;
    }
}

uint8_t Engines::getOperation() {
    return this->stateControl;
}

bool Engines::setPWMSpeed(uint8_t percent) {
    if (percent < 0 || percent > 100) {
        return false;
    }
    this->percentPWM = percent;
    return true;
}
