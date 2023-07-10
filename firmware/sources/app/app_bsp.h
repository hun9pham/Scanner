#ifndef __APP_BSP_H
#define __APP_BSP_H

#include <stdint.h>
#include <stdbool.h>

#define INDICATOR_BRIGHT (0xFE)
#define INDICATOR_DARK   (0xFF)

typedef void (*pf_funcCtl)(void);

class Indicator {
private:
    uint8_t indicateState;
    pf_funcCtl ctrlInit;
    pf_funcCtl ctrlOn;
    pf_funcCtl ctrlOff;

    static void assertParam(bool isTRUE) {
        if (!isTRUE) {
            // TODO: Handle problems
            // -
            // -
        }
    }
public:
    Indicator();
    ~Indicator();

    void initialize(pf_funcCtl ctrlInit, pf_funcCtl ctrlOn, pf_funcCtl ctrlOff);
    void OnState();
    void OffState();
    void Blinking();
    uint8_t getState() {
        return this->indicateState;
    }
};

extern Indicator LEDLIFE;
extern Indicator LEDFLASH;
extern Indicator LEDSTATUS;
extern Indicator LEDDIR;
extern Indicator LEDFAULT;

enum eEngineState {
    ENGINE_STANDSTILL = 0x00,
    ENGINE_FORWARD,
    ENGINE_BACKWARD,
};

enum {
    MOTOR_FRONT = 0,
    MOTOR_REAR,
};

class Engines {
private:
    uint8_t motor1State;
    uint8_t motor2State;

    void standStill(uint8_t motorPos);
    void rotateForward(uint8_t motorPos);
    void rotateBackward(uint8_t motorPos);
public:
    Engines();
    ~Engines();
    void initialize();
    void setOperation(uint8_t oper);
    void setMotorFront(uint8_t oper);
    void setMotorRear(uint8_t oper);
    uint8_t getMotorFront() {
        return motor1State;
    }
    uint8_t getMotorRear() {
        return motor2State;
    }
    bool isMotorFrontRunning();
    bool isMotorRearRunning();
};

extern Engines ENGINES;

#endif /* __APP_BSP_H */
