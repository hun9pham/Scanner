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
#define MOTOR1_PWM_PERCENT  (75)
#define MOTOR2_PWM_PERCENT  (75)

Engines::Engines() {
    
}

Engines::~Engines() {

}

void Engines::initialize() {
    MOTORS_CtlInit();
    this->MOTOR1_StateCtl = STOPPING;
    this->MOTOR2_StateCtl = STOPPING;
}

void Engines::setMOTORS(MOTORS_Pos_t MOTORS, EngineState_t state) {
    if (MOTORS == MOTOR_FRONT) {
        if (state == STOPPING) {
            this->MOTOR1_StateCtl = STOPPING;
            MOTOR1_SetPWM(STOP);
        }
        else if (state == SCROLL_FORDWARD) {
            this->MOTOR1_StateCtl = SCROLL_FORDWARD;
            MOTOR1_SetPWM(SCROLLFORWARD);
        }
        else if (state == SCROLL_BACKWARD) {
            this->MOTOR1_StateCtl = SCROLL_BACKWARD;
            MOTOR1_SetPWM(SCROLLBACKWARD);
        }
    }
    else if (MOTORS == MOTOR_REAR) {
        if (state == STOPPING) {
            this->MOTOR2_StateCtl = STOPPING;
            MOTOR2_SetPWM(STOP);
        }
        else if (state == SCROLL_FORDWARD) {
            this->MOTOR2_StateCtl = SCROLL_FORDWARD;
            MOTOR2_SetPWM(SCROLLFORWARD);
        }
        else if (state == SCROLL_BACKWARD) {
            this->MOTOR2_StateCtl = SCROLL_BACKWARD;
            MOTOR2_SetPWM(SCROLLBACKWARD);
        }
    }
    else if (MOTORS == DUOMOTORS) {
        if (state == STOPPING) {
            this->MOTOR1_StateCtl = STOPPING;
            this->MOTOR2_StateCtl = STOPPING;
            MOTOR1_SetPWM(STOP);
            MOTOR2_SetPWM(STOP);
        }
        else if (state == SCROLL_FORDWARD) {
            this->MOTOR1_StateCtl = SCROLL_FORDWARD;
            this->MOTOR2_StateCtl = SCROLL_FORDWARD;
            MOTOR1_SetPWM(SCROLLFORWARD);
            MOTOR2_SetPWM(SCROLLFORWARD);
        }
        else if (state == SCROLL_BACKWARD) {
            this->MOTOR1_StateCtl = SCROLL_BACKWARD;
            this->MOTOR2_StateCtl = SCROLL_BACKWARD;
            MOTOR1_SetPWM(SCROLLBACKWARD);
            MOTOR2_SetPWM(SCROLLBACKWARD);
        }
    }
}

uint8_t Engines::readMOTORStateCtl(uint8_t MOTORS) {
    return ((MOTORS == MOTOR_FRONT) ? this->MOTOR1_StateCtl : this->MOTOR2_StateCtl);
}

bool Engines::isRun(MOTORS_Pos_t MOTORS) {
    bool ret = false;
    if (MOTORS == MOTOR_FRONT) {
        ret = MOTOR1_IsRun();
    }
    else if (MOTORS == MOTOR_REAR) {
        ret = MOTOR2_IsRun();
    }
    return ret;
}
