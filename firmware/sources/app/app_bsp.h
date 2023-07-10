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

typedef enum eEngineStatus {
    STOPPING = 0x00,
    SCROLLING,
} EngineState_t;

typedef enum eMOTORS_Position {
    MOTOR_FRONT = 0,
    MOTOR_REAR,
    DUOMOTORS,
} MOTORS_Pos_t;

class Engines {
private:
    uint8_t MOTOR1_StateCtl;
    uint8_t MOTOR2_StateCtl;

public:
    Engines();
    ~Engines();
    void initialize();
    void setMOTORS(MOTORS_Pos_t MOTORS, EngineState_t state);
    uint8_t readMOTORStateCtl(uint8_t MOTORS);
    bool isRun(MOTORS_Pos_t MOTORS);
};

extern Engines ENGINES;

#endif /* __APP_BSP_H */
