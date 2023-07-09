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

class Engines {
private:
    uint8_t stateControl;
    bool viaPWM;
    uint8_t percentPWM;

    void standStill();
    void rotateForward();
    void rotateBackward();
public:
    Engines();
    ~Engines();
    void initialize(bool isPWM=false);
    bool setPWMSpeed(uint8_t percent); /* 0-100% */
    void setOperation(uint8_t oper);
    uint8_t getOperation();
};

extern Engines ENGINES;

#endif /* __APP_BSP_H */
