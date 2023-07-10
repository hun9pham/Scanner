#include "app_bsp.h"

#include "io_cfg.h"

Indicator LEDLIFE;
Indicator LEDFLASH;
Indicator LEDSTATUS;
Indicator LEDDIR;
Indicator LEDFAULT;

Engines ENGINES;

//////////////////////////////////////////////
// Class INDICATOR
//////////////////////////////////////////////
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

//////////////////////////////////////////////
// Class ENGINES
//////////////////////////////////////////////
#define MOTOR1_PWM_PERCENT  (70)
#define MOTOR2_PWM_PERCENT  (95)

Engines::Engines() {
    
}

Engines::~Engines() {

}

void Engines::initialize() {
    MOTORS_PWMInit();
    this->MOTOR1_StateCtl = STOPPING;
    this->MOTOR2_StateCtl = STOPPING;
}

void Engines::setMOTORS(MOTORS_Pos_t MOTORS, EngineState_t state) {
    if (MOTORS == MOTOR_FRONT) {
        if (state == STOPPING) {
            this->MOTOR1_StateCtl = STOPPING;
            MOTOR1_SetPWM(0);
        }
        else {
            this->MOTOR1_StateCtl = SCROLLING;
            MOTOR1_SetPWM(MOTOR1_PWM_PERCENT);
        }
    }
    else if (MOTORS == MOTOR_REAR) {
        if (state == STOPPING) {
            this->MOTOR2_StateCtl = STOPPING;
            MOTOR2_SetPWM(0);
        }
        else {
            this->MOTOR2_StateCtl = SCROLLING;
            MOTOR2_SetPWM(MOTOR2_PWM_PERCENT);
        }
    }
    else if (MOTORS == DUOMOTORS) {
        if (state == STOPPING) {
            this->MOTOR1_StateCtl = STOPPING;
            this->MOTOR2_StateCtl = STOPPING;
            MOTOR1_SetPWM(0);
            MOTOR2_SetPWM(0);
        }
        else {
            this->MOTOR1_StateCtl = SCROLLING;
            this->MOTOR2_StateCtl = SCROLLING;
            MOTOR1_SetPWM(MOTOR1_PWM_PERCENT);
            MOTOR2_SetPWM(MOTOR2_PWM_PERCENT);
        }
    }
}

uint8_t Engines::readMOTORStateCtl(uint8_t MOTORS) {
    return ((MOTORS == MOTOR_FRONT) ? this->MOTOR1_StateCtl : this->MOTOR2_StateCtl);
}

bool Engines::isRun(MOTORS_Pos_t MOTORS) {
    return ((MOTORS == MOTOR_FRONT) ? MOTOR1_IsRun() : MOTOR2_IsRun());
}
